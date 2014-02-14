#include "image_pairs.h"
#include "image.h"

#include <opencv2/calib3d/calib3d.hpp>

// inspired from persitence.cpp:write( .., vector<Keypoint>& keypoints)
static void write_matches(FileStorage& fs, const std::string& name, const Matches& matches) {
    WriteStructContext ws(fs, name, CV_NODE_SEQ + CV_NODE_FLOW);

    for (DMatch match : matches) {
        write(fs, match.queryIdx);
        write(fs, match.trainIdx);
        write(fs, match.imgIdx);
        write(fs, match.distance);
    }
}

// inspired from persitence.cpp:read( .., vector<Keypoint>& keypoints)
static void read_matches(const FileNode& node, Matches& matches) {
    matches.resize(0);

    FileNodeIterator it = node.begin(), it_end = node.end();
    for( ; it != it_end; ) {
        DMatch match;

        it >> match.queryIdx >> match.trainIdx >> match.imgIdx >> match.distance;
        matches.push_back(match);
    }
}

void ImagePair::write(FileStorage& fs) const {
    LOG(DEBUG) << "Serializing Image Pair";

    fs << "{"
       << "F" << F;

    write_matches(fs, "matches", matches);

    fs << "inliers_count" << (int) inliers_count
       << "inliers" << keypointsInliers
       << "image1" << *image1
       << "image2" << *image2
       << "error" << error
       << "}";
}

void ImagePair::read(const FileNode& node) {
    int count;

    image1.reset(new Image());
    image2.reset(new Image());

    LOG(DEBUG) << "De-serializing Image Pair";

    node["F"] >> F;

    read_matches(node["matches"], matches);

    node["inliers_count"] >> count;
    inliers_count = count;

    node["inliers"] >> keypointsInliers;
    node["image1"] >> *image1;
    node["image2"] >> *image2;
    node["error"] >> error;
}


bool ImagePair::compute_matches() {
    ImageFeaturesPtr features1 = image1->get_image_features();
    ImageFeaturesPtr features2 = image2->get_image_features();

    if (!features1 || !features2) {
        LOG(ERROR) << "Unable to load features from images";
        return false;
    }

    match_features(*features1, *features2, matches);

    if (matches.size() < MIN_FEATURE_MATCHES) {
        LOG(DEBUG) << "Not enough matches: " << matches.size()
                   << ", at least " << MIN_FEATURE_MATCHES << "needed.";
        return false;
    }

    return true;
}

bool ImagePair::compute_F_mat() {
    // use RANSAC to get F matrix
    // return better_matches for all the matches that make sense
    ImageFeaturesPtr features1 = image1->get_image_features();
    ImageFeaturesPtr features2 = image2->get_image_features();
    vector<Point2f> pts1, pts2;
    vector<unsigned char> status;

    if (!features1 || !features2) {
        LOG(ERROR) << "Unable to load features from images";
        return false;
    }

    matches2points(matches, *features1, *features2, pts1, pts2);

    F = findFundamentalMat(pts1, pts2, FM_RANSAC, 1, 0.99, status);

    // XX (mtourne): stupid vector<unsigned char> to vector<char> conversion
    keypointsInliers = vector<char> (status.begin(), status.end());

    inliers_count = countNonZero(keypointsInliers);

    LOG(DEBUG) << "F matrix: " << endl << F;

    LOG(DEBUG) << "Fundamental mat is keeping " << inliers_count << " / "
               << keypointsInliers.size();

    if (inliers_count < MIN_INLIERS) {
        LOG(DEBUG) << "Not enough inliers: " << inliers_count
                   << ", at least " << MIN_INLIERS << "needed.";
        return false;
    }

    return true;
}

// decompose E into Rotation and Translation
// two methods of choice here : Horn90 and Hartley & Zisserman
// implement HZ using SVD decomposition
static int getRT(Mat& E, vector<Mat> &R, vector<Mat> &T) {
	SVD svd(E,SVD::MODIFY_A);

    LOG(DEBUG) << "U: " << svd.u << endl
               << "Vt:" << svd.vt << endl
               << "W:" << svd.w << endl;

    return 0;
}

int ImagePair::compute_camera_mat() {
    Mat K1 = image1->get_camera_matrix();
    Mat K2 = image2->get_camera_matrix();

    // essential matrix
    Mat E = K1.t() * F * K2;

    //according to http://en.wikipedia.org/wiki/Essential_matrix#Properties_of_the_essential_matrix
    // det(E) == 0
    if(fabsf(determinant(E)) > 1e-07) {
        LOG(DEBUG) << "det(E) != 0: " << determinant(E);
        return 1;
    }

    // For a given essential matrix E = U diag(1, 1, 0)V'
    //  and ﬁrst camera matrix P = [I | 0], there are four
    // possible choices for the second camera matrix P

    vector<Mat> R(4);
    vector<Mat> T(4);

    getRT(E, R, T);

    return 0;
}

void ImagePair::print_matches() const {
    Mat img_gray1 = image1->get_image_gray();
    Mat img_gray2 = image2->get_image_gray();
    ImageFeaturesPtr features1 = image1->get_image_features();
    ImageFeaturesPtr features2 = image2->get_image_features();

    if (!features1 || !features2) {
        LOG(ERROR) << "Unable to load features from images";
        return;
    }

    ostringstream ss;

    ss << "matches_" << image1->get_name() << "_" << image2->get_name();
    if (keypointsInliers.size() > 0) {
        ss << "_RANSAC";
    } else {
        ss << "_ALL_MATCHES";
    }
    ss << ".jpg";

    write_matches_image(img_gray1, *features1,
                        img_gray2, *features2,
                        matches, keypointsInliers, ss.str());
}
