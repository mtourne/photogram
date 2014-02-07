#ifndef PHOTOGRAM_H_
#define PHOTOGRAM_H_

#include <memory>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <Eigen/Dense>
#include <math.h>

#include "logging.hpp"

using namespace std;
using namespace cv;

typedef Eigen::Matrix<double, 3, 3> Mat33;

#endif // !PHOTOGRAM_H_
