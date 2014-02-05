/* This file is part of RGBDSLAM.
 *
 * RGBDSLAM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RGBDSLAM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RGBDSLAM.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef USE_SIFT_GPU
#define NDEBUG 1

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <iostream>
#include <stdio.h>

#include "photogram.h"
#include "sift_gpu_wrapper.h"

SiftGPUWrapper* SiftGPUWrapper::instance = NULL;

SiftGPUWrapper::SiftGPUWrapper() {
    boost::mutex::scoped_lock lock(gpu_mutex);
    data = NULL;
    error = false;
    imageHeight = 0;
    imageWidth = 0;
    siftgpu = new SiftGPU();

#if defined(SIFT_GPU_MODE) and SIFT_GPU_MODE == 1
    char method[] = {"-cuda"};
#elif defined(SIFT_GPU_MODE) and SIFT_GPU_MODE == 2
    char method[] = {"-glsl"};
#endif

    // XX (mtourne): max features determined by
    // SiftGpu, eventually set to a value
    int max_features = -1;
    char max_feat_char[10];
    sprintf(max_feat_char, "%d", max_features);
    char subpixelKey[] = {"-s"};
    char subpixelValue[] = {"1"};
    char max_flag[] = {"-tc2"};
    //char resize_storage_on_demand[] = {"-tight"};
    char unnormalized_descriptors[] = {"-unn"};
    char verbosity[] = {"-v"};//nothing but errors
    char verbosity_val[] = {"0"};//nothing but errors
    //char * argv[] = {method, "-t", "10", subpixelKey, subpixelValue, max_flag, max_feat_char};
    char first_octave[] = {"-fo"};
    char first_octave_val[] = {"0"};
    char * argv[] = {method,  subpixelKey, subpixelValue, \
                     max_flag, max_feat_char, first_octave,  \
                     first_octave_val, verbosity, verbosity_val }; /*, resize_storage_on_demand};
                     unnormalized_descriptors, \
                     "-e", "50.0" };//, "-t", "0.005"};*/
    siftgpu->ParseParam(9, argv);

    if (siftgpu->CreateContextGL() != SiftGPU::SIFTGPU_FULL_SUPPORTED) {
        LOG(ERROR) << "Can't create OpenGL context! SiftGPU cannot be used.";
        error = true;
    }


    isMatcherInitialized = false;
}

SiftGPUWrapper::~SiftGPUWrapper() {
    boost::mutex::scoped_lock lock(gpu_mutex);
    delete siftgpu;
    if (isMatcherInitialized) delete matcher;
    instance = NULL;
    if (data != NULL) {
        free(data);
        data = NULL;
    }
}

void SiftGPUWrapper::destroyInstance() {
    delete instance;
}
SiftGPUWrapper* SiftGPUWrapper::getInstance() {
    if (instance == NULL) {
        LOG(INFO) << "Create Instance";
        instance = new SiftGPUWrapper();
    }
    return instance;
}

void SiftGPUWrapper::detect(const cv::Mat& image, cv::vector<cv::KeyPoint>& keypoints,
                            std::vector<float>& descriptors, const Mat& mask) const {
    // TODO (mtourne): timer is a good idea
    //ScopedTimer s(__FUNCTION__);
    if (error) {
        keypoints.clear();
        LOG(FATAL) << "SiftGPU cannot be used. Detection of keypoints failed";
    }

    //get image
    if(image.rows != imageHeight || image.cols != imageWidth){
      imageHeight = image.rows;
      imageWidth = image.cols;
      free(data);
      data = (unsigned char*) malloc(imageWidth * imageHeight);
    }
    cvMatToSiftGPU(image, data);

    int num_features = 0;
    SiftGPU::SiftKeypoint* keys = 0;

    boost::mutex::scoped_lock lock(gpu_mutex);
    LOG(DEBUG) << "SIFTGPU: cols: " << image.cols << ", rows: " << image.rows;
    if (siftgpu->RunSIFT(image.cols, image.rows, data, GL_LUMINANCE, GL_UNSIGNED_BYTE)) {
        num_features = siftgpu->GetFeatureNum();
        LOG(INFO) << "Number of features found: " << num_features;
        keys = new SiftGPU::SiftKeypoint[num_features];
        descriptors.resize(128 * num_features);
        //descriptors = new float[128 * num_features];
        siftgpu->GetFeatureVector(&keys[0], &descriptors[0]);
    } else {
        LOG(WARNING) << "SIFTGPU->RunSIFT() failed!";
    }

    //copy to opencv structure
    keypoints.clear();
    for (int i = 0; i < num_features; ++i) {
        KeyPoint key(keys[i].x, keys[i].y, 6.0 * keys[i].s, keys[i].o); // 6 x scale is the conversion to pixels, according to changchang wu (the author of siftgpu)
        keypoints.push_back(key);
    }
}

int SiftGPUWrapper::match(
        const std::vector<float>& descriptors1,
        int num1,
        const std::vector<float>& descriptors2,
        int num2,
        std::vector<cv::DMatch>* matches) {
    boost::mutex::scoped_lock lock(gpu_mutex);

    if (!isMatcherInitialized)
        initializeMatcher();

    float sumDistances = 0;

    LOG(DEBUG) << "descriptor1 size: " << num1 << ", descriptor2 size: " << num2;

    matcher->SetDescriptors(0, num1, &descriptors1[0]);
    matcher->SetDescriptors(1, num2, &descriptors2[0]);

    int match_buf[4096][2];
    int number = matcher->GetSiftMatch(4096, match_buf, 0.9, 0.9);

    LOG(DEBUG) << "matches: " << number;

    if (matches->size() != 0) {
        LOG(WARNING) << "Clearing matches vector!";
        matches->clear();
    }

    cv::DMatch match;
    int counter = 0;

    for (int i = 0; i < number; i++) {
        match.queryIdx = match_buf[i][0];
        match.trainIdx = match_buf[i][1];

        //only use matches with indices != 0 (opengl context problem may sometimes happen)
        if (match.queryIdx == 0 || match.trainIdx == 0) {
            counter++;
        }

        if (counter > 0.5 * number) {
            matches->clear();
            sumDistances = 0;
            LOG(ERROR) << "Matches bad due to context error";
            break;
        }

        float sum = 0;
        for (int j = 0; j < 128; j++) {
            float a = descriptors1[match.queryIdx * 128 + j] - descriptors2[match.trainIdx * 128 + j];
            sum += a * a;
        }

        match.distance = sqrt(sum);
        sumDistances += match.distance;
        matches->push_back(match);
        LOG(DEBUG) << "Matched Features " << match.queryIdx << " and " <<  match.trainIdx << " with distance of " << match.distance <<". Sum:" << sumDistances;
    }

    return sumDistances;
}

void SiftGPUWrapper::initializeMatcher() {
    matcher = CreateNewSiftMatchGPU(4096);
    if (!matcher->VerifyContextGL()) {
        LOG(FATAL) << "Can't create OpenGL context! SiftGPU Matcher cannot be used.\nYou probably have no OpenGL support. You can use a different type of matcher though, e.g. FLANN.";
        error = true;
        return;
    }
    LOG(INFO) << "SiftGPU matcher initialized successfully.";
    isMatcherInitialized = true;
}

void SiftGPUWrapper::cvMatToSiftGPU(const Mat& image, unsigned char* siftImage) const {
    // TODO (mtourne): timer is a good idea
    // ScopedTimer s(__FUNCTION__);
    Mat tmp;
    image.convertTo(tmp, CV_8U);
    for (int y = 0; y < tmp.rows; ++y) {
        for (int x = 0; x < tmp.cols; ++x) {
            siftImage[y * tmp.cols + x] = tmp.at<unsigned char> (y, x);
        }
    }
}

void SiftGPUWrapper::writePGM(FILE *fp, unsigned char* data, int width, int height)
{
    int val;
    fprintf(fp, "P5\n%d %d\n255\n", width, height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            val = (int) (/*255.0 */data[y * width + x]);
            if (x == 0) val = 255;
            if (y == 0) val = 255;
            fputc(MAX(0, MIN(255, val)), fp);
        }
    }
}
#endif
