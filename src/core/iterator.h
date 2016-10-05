/*
 * ITERATOR API
 *
 *
 */
#ifndef ITERATOR_H_
#define ITERATOR_H_

#include "gdal_priv.h"

#include "core_exp.h"
#include "common.h"

/*
 * \brief Iterate a raster dataset in blocks.
 * Iterates over the pixels of a raster band in blocks.
 * Each call to ``next`` returns the pixel offsets and block size
 * which can be pass directly to RasterIO
 */
class GALGCORE_DLL BlockIterator {

public:
	BlockIterator(GDALDataset *dataset);
	virtual ~BlockIterator() {};
	virtual bool next(int *xSize, int *ySize, int *xOff, int *yOff);
	virtual GALGError setBlockSize(int blockWidth, int blockHeight);

protected:
	virtual void calcBlockSize(int *xSize, int *ySize);
	virtual void nextXOff();
	virtual void nextYOff();
	/*
	 * Adjust the given x and y sizes of a block to ensure it fits within the raster limits
	 */
	virtual void getActualBlockSize(int *xSize, int *ySize);
	int blockWidth, blockHeight, rasterXSize, rasterYSize;
	int xOff, yOff;

};

/*
 * Iterates over the pixels of a raster band in buffered blocks.
 * Same as ``BlockIterator`` except each block overlaps the previous
 * by ``N`` pixels. ``N`` is the buffer size set by calling ``setBufferSize``
 */
class GALGCORE_DLL BufferedIterator: public BlockIterator {

public:
	BufferedIterator(GDALDataset *dataset, int bufferSize);
	GALGError setBufferSize(int bufferSize);
	bool next(int *xSize, int *ySize, int *xOff, int *yOff);

protected:
	void calcBlockSize(int *xSize, int *ySize);
	void nextXOff();
	void nextYOff();

private:
	int bufferSize;

};

#endif // ITERATOR_H_

