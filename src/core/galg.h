/*
 * galg.h
 *
 *  Created on: 14 Jul 2016
 *      Author: jamesramm
 */

#ifndef GALG_H_
#define GALG_H_

#include "core_exp.h"
#include "common.h"
#include <vector>
#include <memory>

// Include processing functions

/**
 * \brief Definition of a raster processing function.
 *
 * A GALGRasterProcessFn accepts an array of data as input, applies custom logic and writes the output to padfOutArray.
 * Such a function can be passed to GALGRunRasterProcess to apply custom processing to a GDALDataset in chunks and create
 * a new GDALDataset.
 *
 * @param padfInArray The input array of data.
 *
 * @param padfOutArray The output array of data. On first call (via GDALRunRasterProcess) this will be an empty, initialised array,
 *    which should be populated with the result of calculations on padfInArray. In subsequent calls it will contain the result of the
 *    previous window.
 *
 * @param nWindowXSize the actual x size (width) of the read window.
 *
 * @param nWindowYSize the actual y size (height) of the read window. The length of padfInArray == padfOutArray == nWindowXSize * nWindowYSize
 *
 * @param pData Process-specific data. This data is passed straight through to the GDALRasterProcessFn and may contain e.g user defined parameters.
 *     The GDALRasterProcessFn definition would define the structure/type of such data.
 *
 * @param pdfNoDataValue The no data value of the dataset
 */
class GALGCORE_DLL IProcessImage {

public:
    IProcessImage();
    virtual ~IProcessImage();
    virtual GALGError processImage(float *inputArray, float *outputArray, int nWindowXSize, int nWindowYSize,
            double *inNoDataValue, double *outNoDataValue);

};

class GALGCORE_DLL RasterProcess {

public:
    RasterProcess();
    ~RasterProcess(){};

    /**
     * \brief Apply a raster processing function to each sub-window of a raster.
     *
     * The input raster dataset is read in chunks of nWindowXSize * nWindowYSize and each chunk is passed to the processing
     * function. The output array from the function is written to the destination dataset.
     * An optional 'pixel buffer' can be specified to allow overlaps between successive windows. This is useful for
     * some algorithms, e.g. blob extraction, watershed/stream flow analysis, convolution etc.
     * Process specific data can be passed (e.g. configuration parameters). This data is simply passed straight through to the processing
     * function on each call.
     *
     * @param processFn A GALGRasterProcessFn to apply to each sub window of the raster.
     *
     * @param inputPathStr Path to the source raster dataset from which pixel values are read
     *
     * @param outputPathStr Path to the desired output GeoTiff dataset
     *
     * @param dataObject Process-specific data. This is passed straight through to the GDALRasterProcessFn on each call.
     *
     * @param windowXSize The desired width of each read window. If NULL it defaults to the 'natural' block size of the raster
     *
     * @param windowYSize The desired height of each read window. If NULL it defaults to the 'natural' block size.
     *
     * @param nPixelBuffer A pixel buffer to apply to the read window. The read window is expanded by pnPixelBuffer pixels in all directions such that
     *    each window overlaps by pnPixelBuffer pixels.
     *
     * @param skipHoles If true, will skip processing blocks which contain only no data values and create a sparse geotiff. Only available for geotiff inputs
     *
     * @return a GALGError struct indicating whether the process succeeded.
     */
    GALGError map(IProcessImage &processor, const char *inputPathStr,
            const char *outputPathStr, int *windowXSize,
            int *windowYSize, int *nPixelBuffer, bool skipHoles);

    /**
     * \brief Apply multiple raster processing functions to each sub-window of a raster
     *
     * For each window, the functions defined by the paProcessFn array are called in turn, with the array output of the previous function forming the input
     * to the next function. This allows processing 'toolchains' to be built without having to create intermediate datasets, which can be less efficient in time and space.
     *
     *
     * @param processFnArray An array of GDALRasterProcessFn to apply to each sub window of the raster
     *
     * @param nProcesses The size of paProcessFn
     *
     * @param inputPathStr The path to the source raster dataset from which pixel values are read
     *
     * @param outputPathStr The path to the destination raster dataset to which pixel values are written. Must support RasterIO in write mode.
     *
     * @param dataObjectArray an array of process-specific data objects of size nProcesses. Each data object will be passed to the corresponding GDALRasterProcessFn
     *
     * @param windowXSize The desired width of each read window. If NULL it defaults to the 'natural' block size of the raster
     *
     * @param windowYSize The desired height of each read window. If NULL it defaults to the 'natural' block size.
     *
     * @param nPixelBuffer A pixel buffer to apply to the read window. The read window is expanded by pnPixelBuffer pixels in all directions such that
     *    each window overlaps by pnPixelBuffer pixels.
     *
     *    @param skipHoles If true, will skip processing blocks which contain only no data values and create a sparse geotiff. Only available for geotiff inputs
     *
     * @return a GALGError struct indicating whether the process succeeded.
     */
    GALGError mapMany(std::vector<IProcessImage *> &processorArray,
            const char *inputPathStr, const char *outputPathStr,
            int *windowXSize, int *windowYSize, int *nPixelBuffer, bool skipHoles);

    /**
     * \brief Apply a raster processing 'reduction' function to each sub-window of multiple raster datasets.
     *
     * TODO: Complete
     */
    GALGError reduce(IProcessImage &processor, const char **inputPathStrArray,
            const char *outputPathStr, int *windowXSize,
            int *windowYSize, int *nPixelBuffer, bool skipHoles);

};

#endif /* GALG_H_ */
