#ifndef THRESHOLD_H_
#define THRESHOLD_H_
#include "../core/galg.h"
#include "func_exp.h"
#include "opencv2/imgproc/imgproc.hpp"


enum GALGFUNC_DLL ThresholdType {THRESH_BINARY = cv::THRESH_BINARY, THRESH_TRUNC = cv::THRESH_TRUNC,
                    THRESH_TOZERO = cv::THRESH_TOZERO, THRESH_BINARY_INV = cv::THRESH_BINARY_INV,
                    THRESH_TOZERO_INV = cv::THRESH_TOZERO_INV};

/*
* Fixed level thresholding
*
* A fixed level threshold is applied to each array element using OpenCV threshold
*/
class GALGFUNC_DLL Threshold: public IProcessImage {

public:
    void setThresholdParams(double maxVal, double threshold, int thresholdType);
    GALGError processImage(float *inputArray, float *outputArray, int nWindowXSize, int nWindowYSize,
        double *inNoDataValue, double *outNoDataValue);
private:
    double maxVal_ = 0;
    double threshold_ = 0;
    int thresholdType_ = THRESH_TOZERO;
    GALGError err;
};

#endif /* THRESHOLD_H_ */
