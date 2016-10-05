
#include "threshold.h"
#include "opencv2/core/core.hpp"

void Threshold::setThresholdParams(double maxVal, double threshold, int thresholdType){
    maxVal_ = maxVal;
    threshold_ = threshold;
    thresholdType_ = thresholdType;
}

GALGError Threshold::processImage(float *inputArray, float *outputArray, int nWindowXSize, int nWindowYSize,
    double *inNoDataValue, double *outNoDataValue) {

    err.errnum = 0;
    err.msg = NULL;
    cv::Mat inMat(nWindowXSize, nWindowYSize, CV_32F, (void *)inputArray);
    cv::Mat outMat(nWindowXSize, nWindowYSize, CV_32F, (void *)outputArray);
    cv::threshold(inMat, outMat, threshold_, maxVal_, thresholdType_);

    return err;
}