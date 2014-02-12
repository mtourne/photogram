#ifndef PHOTOGRAM_H_
#define PHOTOGRAM_H_

#include <iostream>
#include <string>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <math.h>

// TODO (mtourne): eventually replace
// with log4cxx (compiled library)
#if defined(__clang__) || defined(GCC47)
#include "logging.hpp"
#else
#include "logging_dummy.hpp"
#endif

using namespace std;
using namespace cv;

#endif // !PHOTOGRAM_H_
