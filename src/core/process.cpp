/*
 * process.cpp
 *
 *  Created on: 14 Jul 2016
 *      Author: jamesramm
 */

#include <iostream>
#include "galg.h"
#include "iterator.h"

#include "gdal_priv.h"
#include "cpl_error.h"

GALGError createOutputDataset(GDALDataset *srcDataset,
		const char *outputPathStr, GDALDataset *&dstDataset, bool skipHoles) {
	GALGError errResult = { 0, NULL };

	const char *formatStr = "GTiff";
	GDALDriver *gdalDriver;
	gdalDriver = GetGDALDriverManager()->GetDriverByName(formatStr);

	RETURNIF(gdalDriver == NULL, 1, "Could not initialise Geotiff driver");

	char **optionStrArray = NULL;
	optionStrArray = CSLSetNameValue(optionStrArray, "TILED", "YES");
	optionStrArray = CSLSetNameValue(optionStrArray, "COMPRESS", "LZW");
	if (skipHoles) {
		optionStrArray = CSLSetNameValue(optionStrArray, "SPARSE_OK", "TRUE");
	}

	dstDataset = gdalDriver->Create(outputPathStr, srcDataset->GetRasterXSize(),
			srcDataset->GetRasterYSize(), srcDataset->GetRasterCount(),
			srcDataset->GetRasterBand(1)->GetRasterDataType(), optionStrArray);

	RETURNIF(dstDataset == NULL, 1, "Could not create output dataset");

	double geotransform[6];
	srcDataset->GetGeoTransform(geotransform);
	dstDataset->SetGeoTransform(geotransform);
	dstDataset->SetProjection(srcDataset->GetProjectionRef());

	GDALRasterBand *srcBand, *dstBand;
	for (int ixBand = 0; ixBand < srcDataset->GetRasterCount(); ++ixBand) {
		srcBand = srcDataset->GetRasterBand(ixBand + 1);
		dstBand = dstDataset->GetRasterBand(ixBand + 1);
		dstBand->SetNoDataValue(srcBand->GetNoDataValue());
	}
	return errResult;
}

// Default implementation of IProcessImage
IProcessImage::IProcessImage() {
}
IProcessImage::~IProcessImage() {
}
GALGError IProcessImage::processImage(float *inputArray, float *outputArray,
		int nWindowXSize, int nWindowYSize, double *inNoDataValue,
		double *outNoDataValue) {
	GALGError err = { 0, NULL };
	memcpy(outputArray, inputArray, (size_t) nWindowXSize * nWindowYSize);
	return err;
}

RasterProcess::RasterProcess() {

}

GALGError RasterProcess::map(IProcessImage &processor, const char *inputPathStr,
		const char *outputPathStr, int *windowXSize, int *windowYSize,
		int *nPixelBuffer, bool skipHoles) {

	GALGError result = { 0, NULL };
	GDALDataset *srcDataset;
	GDALDataset *dstDataset;
	// Open the input dataset and verify
	srcDataset = (GDALDataset *) GDALOpen(inputPathStr, GA_ReadOnly);

	// If the assesrtion is TRUE, exit the function with a suitable error
	RETURNIF(srcDataset == NULL, 1, "Could not open source dataset");

	// Create output dataset and verify
	result = createOutputDataset(srcDataset, outputPathStr, dstDataset,
			skipHoles);
	RETURNIF(result.errnum != 0, result.errnum, result.msg);

	// Setup the iterator. If pixelBuffer was passed, we created a buffered iterator,
	// otherwise use a standard BlockIterator
	BlockIterator *iterator = NULL;
	if (nPixelBuffer != NULL && *nPixelBuffer > 0) {
		iterator = new BufferedIterator(dstDataset, *nPixelBuffer);
	} else {
		iterator = new BlockIterator(dstDataset);
	}
	RETURNIF(iterator == NULL, 1,
			"Unable to allocate memory for BlockIterator");
	iterator->setBlockSize(*windowXSize, *windowYSize);

	// Prepare the data buffers
	float *bufInputData = NULL, *bufOutputData = NULL;

	size_t nxBytes = *windowXSize * sizeof(float);
	size_t nyBytes = *windowYSize * sizeof(float);
	bufInputData = (float *) VSIMalloc2(nxBytes, nyBytes);
	bufOutputData = (float *) VSIMalloc2(nxBytes, nyBytes);
	RETURNIF(bufInputData == NULL || bufOutputData == NULL, 1,
			"Unable to allocate data arrays");

	int nBands = srcDataset->GetRasterCount();
	int xOff, yOff, xSize, ySize;
	GDALRasterBand *srcBand, *dstBand;
	double inNoDataValue, outNoDataValue;
	int bSuccess;

	// Apply the process function to each sub window of each band
	// in the dataset
	for (int iBand = 0; iBand < nBands; ++iBand) {

		srcBand = srcDataset->GetRasterBand(iBand + 1);
		dstBand = dstDataset->GetRasterBand(iBand + 1);
		inNoDataValue = srcBand->GetNoDataValue(&bSuccess);
		outNoDataValue = dstBand->GetNoDataValue(&bSuccess);

		while (iterator->next(&xSize, &ySize, &xOff, &yOff)) {

			// Read input data. TODO:: Verify function ran
			srcBand->RasterIO(GF_Read, xOff, yOff, xSize, ySize, bufInputData,
					xSize, ySize, GDT_Float32, 0, 0);

			// Call the process function. TODO: Verify output
			processor.processImage(bufInputData, bufOutputData, xSize, ySize,
					&inNoDataValue, &outNoDataValue);

			// Write out the result. TODO: Verify function ran
			dstBand->RasterIO(GF_Write, xOff, yOff, xSize, ySize, bufOutputData,
					xSize, ySize, GDT_Float32, 0, 0);
		}
	}
	dstDataset->FlushCache();
	GDALClose(dstDataset);
	GDALClose(srcDataset);
	return result;
}

GALGError RasterProcess::mapMany(std::vector<IProcessImage *> &processorArray,
		const char *inputPathStr, const char *outputPathStr, int *windowXSize,
		int *windowYSize, int *nPixelBuffer, bool skipHoles) {

	GALGError result = { 1, "Not Implemented" };

	// TODO

	return result;
}

GALGError RasterProcess::reduce(IProcessImage &processor,
		const char **inputPathStrArray, const char *outputPathStr,
		int *windowXSize, int *windowYSize, int *nPixelBuffer, bool skipHoles) {
	GALGError result = { 1, "Not Implemented" };

	// TODO

	return result;
}

