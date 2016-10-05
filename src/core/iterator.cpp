#include "iterator.h"
#include <iostream>
#include "cpl_conv.h"

/*****************
 * BLOCK ITERATOR
 *****************/

BlockIterator::BlockIterator(GDALDataset *dataset) {
	GDALRasterBand *band;
	band = dataset->GetRasterBand(1);
	band->GetBlockSize(&blockWidth, &blockHeight);
	rasterXSize = dataset->GetRasterXSize();
	rasterYSize = dataset->GetRasterYSize();
	xOff = -1;
	yOff = 0;
}

GALGError BlockIterator::setBlockSize(int blockWidth, int blockHeight) {

	GALGError error = { 0, NULL };
	if (blockWidth > this->rasterXSize || blockHeight > this->rasterYSize) {

		error.errnum = 1;
		error.msg = "Requested block size is greater than the raster size";
	} else {
		this->blockWidth = blockWidth;
		this->blockHeight = blockHeight;
	}
	return error;
}

void BlockIterator::nextXOff() {
	this->xOff += this->blockWidth;
}

void BlockIterator::nextYOff() {
	this->yOff += this->blockHeight;
}

void BlockIterator::calcBlockSize(int *xSize, int *ySize) {
	*xSize = this->blockWidth;
	*ySize = this->blockHeight;
}

bool BlockIterator::next(int *xSize, int *ySize, int *xOff, int *yOff) {
	// Logic for moving the read window across raster blocks. Exact calculation
	// of read offsets and sizes is delegated to other functions
	// which can be overridden for different behaviour
	if (this->xOff < 0) {
		this->xOff = 0;
	} else {
		this->nextXOff();
	}
	if (this->xOff >= this->rasterXSize - 1) {
		this->xOff = 0;
		this->nextYOff();
		if (this->yOff >= this->rasterYSize - 1) {
			// Return FALSE to indicate we have reached the end
			return false;
		}
	}
	*xOff = this->xOff;
	*yOff = this->yOff;
	this->calcBlockSize(xSize, ySize);
	this->getActualBlockSize(xSize, ySize);
	return true;
}

void BlockIterator::getActualBlockSize(int *xSize, int *ySize) {
	// If the read window overflows the edges of the raster
	// it is reduced.
	if (this->xOff + *xSize >= this->rasterXSize) {
		*xSize = this->rasterXSize - this->xOff;
	}
	if (this->yOff + *ySize >= this->rasterYSize) {
		*ySize = this->rasterYSize - this->yOff;
	}
}

/********************
 * BUFFERED ITERATOR
 ********************/

BufferedIterator::BufferedIterator(GDALDataset *dataset, int bufferSize) :
		BlockIterator(dataset) {
	this->bufferSize = 0;
	this->setBufferSize(bufferSize);
}

bool BufferedIterator::next(int *xSize, int *ySize, int *xOff, int *yOff) {

	// Calculate the x and y offsets for this window
	if (this->xOff < 0) {
		this->xOff = 0;
	} else {
		this->nextXOff();
	}
	if ((this->xOff + this->bufferSize) >= this->rasterXSize) {
		this->xOff = 0;
		this->nextYOff();
		if ((this->yOff + this->bufferSize) >= this->rasterYSize) {
			// Return FALSE to indicate we have reached the end
			return false;
		}
	}
	*xOff = this->xOff;
	*yOff = this->yOff;

	// Calculate the x and y window size for this window
	this->calcBlockSize(xSize, ySize);
	this->getActualBlockSize(xSize, ySize);
	return true;
}

void BufferedIterator::nextXOff() {
	// We subtract the desired buffer size from the blockwidth
	// in order to read an overlapping buffer - we will
	// add the buffer size on to the block size to get the total number of pixels to read
	if (this->xOff == 0) {
		this->xOff += this->blockWidth;
	} else {
		this->xOff += this->blockWidth + this->bufferSize;
	}
}

void BufferedIterator::nextYOff() {
	if (this->yOff == 0) {
		this->yOff += this->blockHeight;
	} else {
		this->yOff += this->blockHeight + this->bufferSize;
	}
}

void BufferedIterator::calcBlockSize(int *xSize, int *ySize) {
	if (this->xOff == 0) {
		*xSize = this->blockWidth + this->bufferSize;
	} else {
		*xSize = this->blockWidth + 2 * this->bufferSize;
	}

	if (this->yOff == 0) {
		*ySize = this->blockHeight + this->bufferSize;
	} else {
		*ySize = this->blockHeight + 2 * this->bufferSize;
	}

}

GALGError BufferedIterator::setBufferSize(int bufferSize) {
	GALGError err = { 0, NULL };
	if (bufferSize >= (this->rasterXSize + this->blockWidth)
			|| bufferSize >= (this->rasterYSize + this->blockHeight)) {

		err.msg = "Requested buffer size is greater than the raster size";
		err.errnum = 1;
	} else {
		this->bufferSize = bufferSize;
	}
	return err;
}

