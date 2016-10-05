#include "gtest/gtest.h"
#include "gdal.h"
#include "gdal_priv.h"
#include "../src/core/iterator.h"
#include "../src/core/galg.h"
#include "../src/alg/threshold.h"
#include <cstdio>

char *file_name;

namespace {
/*
 * Convienience function to create an in-memory, empty dataset
 */
void get_empty_dataset(GDALDataset *ds, int xSize, int ySize) {
	GDALDriver *poDriver;
	poDriver = GetGDALDriverManager()->GetDriverByName("MEM");
	if (poDriver == NULL) {
		printf("Driver is NULL");
		exit(1);
	}
	ds = poDriver->Create("Test", xSize, ySize, 1, GDT_Byte, NULL);
	ds->FlushCache();
}

void ones_dataset(GDALDataset *ds) {
	GByte abyRaster[10 * 12];
	memset(abyRaster, 1, 10 * 12);
	GDALRasterBand *band = ds->GetRasterBand(1);
	band->RasterIO(GF_Write, 0, 0, 10, 12, abyRaster, 10, 12, GDT_Byte, 0, 0);
	double gt[6] = { 0, 1, 0, 0, 0, 1 };
	ds->SetGeoTransform(gt);
	ds->FlushCache();

}

class IteratorTest: public testing::Test {

protected:
	virtual void SetUp() {
		GDALAllRegister();
		ds = (GDALDataset *) GDALOpen((const char *) file_name, GA_ReadOnly);
		it = new BlockIterator(ds);
	}

	virtual void TearDown() {
		delete it;
		GDALClose(ds);
	}

	BlockIterator *it;
	GDALDataset *ds;
	int xSize, ySize, xOff, yOff;
	bool isMore;
};

TEST_F(IteratorTest, YieldsWholeRaster) {
	/*
	 * In the case where block size is
	 * the exact same size as the raster,
	 * iterator should yield the whole raster
	 */
	it->setBlockSize(10, 12);
	isMore = it->next(&xSize, &ySize, &xOff, &yOff);

	EXPECT_TRUE(isMore);
	EXPECT_EQ(10, xSize);
	EXPECT_EQ(12, ySize);
	EXPECT_EQ(0, xOff);
	EXPECT_EQ(0, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_FALSE(isMore);

}

TEST_F(IteratorTest, YieldsCorrectOffsets) {

	/*
	 * A 10 x 12 raster with a block
	 * size of 5 x 5 should yield the
	 * following blocks
	 *   | 0 1 2 3 4 5 6 7 8 9
	 * --|---------------------
	 * 0 | 0 0 0 0 0 1 1 1 1 1
	 * 1 | 0 0 0 0 0 1 1 1 1 1
	 * 2 | 0 0 0 0 0 1 1 1 1 1
	 * 3 | 0 0 0 0 0 1 1 1 1 1
	 * 4 | 0 0 0 0 0 1 1 1 1 1
	 * 5 | 2 2 2 2 2 3 3 3 3 3
	 * 6 | 2 2 2 2 2 3 3 3 3 3
	 * 7 | 2 2 2 2 2 3 3 3 3 3
	 * 8 | 2 2 2 2 2 3 3 3 3 3
	 * 9 | 2 2 2 2 2 3 3 3 3 3
	 * 10| 4 4 4 4 4 5 5 5 5 5
	 * 11| 4 4 4 4 4 5 5 5 5 5
	 */
	it->setBlockSize(5, 5);
	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_TRUE(isMore);
	EXPECT_EQ(5, xSize);
	EXPECT_EQ(5, ySize);
	EXPECT_EQ(0, xOff);
	EXPECT_EQ(0, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_TRUE(isMore);
	EXPECT_EQ(5, xSize);
	EXPECT_EQ(5, ySize);
	EXPECT_EQ(5, xOff);
	EXPECT_EQ(0, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_TRUE(isMore);
	EXPECT_EQ(5, xSize);
	EXPECT_EQ(5, ySize);
	EXPECT_EQ(0, xOff);
	EXPECT_EQ(5, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_TRUE(isMore);
	EXPECT_EQ(5, xSize);
	EXPECT_EQ(5, ySize);
	EXPECT_EQ(5, xOff);
	EXPECT_EQ(5, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_TRUE(isMore);
	EXPECT_EQ(5, xSize);
	EXPECT_EQ(2, ySize);
	EXPECT_EQ(0, xOff);
	EXPECT_EQ(10, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_TRUE(isMore);
	EXPECT_EQ(5, xSize);
	EXPECT_EQ(2, ySize);
	EXPECT_EQ(5, xOff);
	EXPECT_EQ(10, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_FALSE(isMore);
}

TEST_F(IteratorTest, YieldsCorrectPixelBuffer) {
	/*
	 * A 10 x 12 raster with a block
	 * size of 5 x 5 and a buffer of 1
	 * should yield the following blocks
	 *   | 0 1 2 3 4 5 6 7 8 9
	 * --|---------------------
	 * 0 | 0 0 0 0 0 0
	 * 1 | 0 0 0 0 0 0
	 * 2 | 0 0 0 0 0 0
	 * 3 | 0 0 0 0 0 0
	 * 4 | 0 0 0 0 0 0
	 * 5 | 0 0 0 0 0 0
	 * 6 |
	 * 7 |
	 * 8 |
	 * 9 |
	 * 10|
	 * 11|
	 *   | 0 1 2 3 4 5 6 7 8 9
	 * --|---------------------
	 * 0 |         1 1 1 1 1 1
	 * 1 |         1 1 1 1 1 1
	 * 2 |         1 1 1 1 1 1
	 * 3 |         1 1 1 1 1 1
	 * 4 |         1 1 1 1 1 1
	 * 5 |         1 1 1 1 1 1
	 * 6 |
	 * 7 |
	 * 8 |
	 * 9 |
	 * 10|
	 * 11|
	 *   | 0 1 2 3 4 5 6 7 8 9
	 * --|---------------------
	 * 0 |
	 * 1 |
	 * 2 |
	 * 3 |
	 * 4 |
	 * 5 | 2 2 2 2 2 2
	 * 6 | 2 2 2 2 2 2
	 * 7 | 2 2 2 2 2 2
	 * 8 | 2 2 2 2 2 2
	 * 9 | 2 2 2 2 2 2
	 * 10| 2 2 2 2 2 2
	 * 11|
	 *   | 0 1 2 3 4 5 6 7 8 9
	 * --|---------------------
	 * 3 |
	 * 4 |         3 3 3 3 3 3
	 * 5 |         3 3 3 3 3 3
	 * 6 |         3 3 3 3 3 3
	 * 7 |         3 3 3 3 3 3
	 * 8 |         3 3 3 3 3 3
	 * 9 |         3 3 3 3 3 3
	 * 10|         3 3 3 3 3 3
	 * 11|
	 *   | 0 1 2 3 4 5 6 7 8 9
	 * --|---------------------
	 * 8 |
	 * 9 | 4 4 4 4 4 4
	 * 10| 4 4 4 4 4 4
	 * 11| 4 4 4 4 4 4
	 *
	 *   | 0 1 2 3 4 5 6 7 8 9
	 * --|---------------------
	 * 8 |
	 * 9 |         5 5 5 5 5 5
	 * 10|         5 5 5 5 5 5
	 * 11|         5 5 5 5 5 5

	 */
	delete it;
	it = new BufferedIterator(ds, 1);
	it->setBlockSize(5, 5);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_TRUE(isMore);
	EXPECT_EQ(6, xSize);
	EXPECT_EQ(6, ySize);
	EXPECT_EQ(0, xOff);
	EXPECT_EQ(0, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_TRUE(isMore);
	EXPECT_EQ(5, xSize);
	EXPECT_EQ(6, ySize);
	EXPECT_EQ(5, xOff);
	EXPECT_EQ(0, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_TRUE(isMore);
	EXPECT_EQ(6, xSize);
	EXPECT_EQ(7, ySize);
	EXPECT_EQ(0, xOff);
	EXPECT_EQ(5, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_TRUE(isMore);
	EXPECT_EQ(5, xSize);
	EXPECT_EQ(7, ySize);
	EXPECT_EQ(5, xOff);
	EXPECT_EQ(5, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_FALSE(isMore);
}

TEST_F(IteratorTest, YieldsCorrectPixelBuffer2) {
	/*
	 * As the above test, yet chooses a block size of 3
	 */
	delete it;
	it = new BufferedIterator(ds, 1);
	it->setBlockSize(3, 3);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_TRUE(isMore);
	EXPECT_EQ(4, xSize);
	EXPECT_EQ(4, ySize);
	EXPECT_EQ(0, xOff);
	EXPECT_EQ(0, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_TRUE(isMore);
	EXPECT_EQ(5, xSize);
	EXPECT_EQ(4, ySize);
	EXPECT_EQ(3, xOff);
	EXPECT_EQ(0, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_TRUE(isMore);
	EXPECT_EQ(3, xSize);
	EXPECT_EQ(4, ySize);
	EXPECT_EQ(7, xOff);
	EXPECT_EQ(0, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_TRUE(isMore);
	EXPECT_EQ(4, xSize);
	EXPECT_EQ(5, ySize);
	EXPECT_EQ(0, xOff);
	EXPECT_EQ(3, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_TRUE(isMore);
	EXPECT_EQ(5, xSize);
	EXPECT_EQ(5, ySize);
	EXPECT_EQ(3, xOff);
	EXPECT_EQ(3, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_TRUE(isMore);
	EXPECT_EQ(3, xSize);
	EXPECT_EQ(5, ySize);
	EXPECT_EQ(7, xOff);
	EXPECT_EQ(3, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_TRUE(isMore);
	EXPECT_EQ(4, xSize);
	EXPECT_EQ(5, ySize);
	EXPECT_EQ(0, xOff);
	EXPECT_EQ(7, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_TRUE(isMore);
	EXPECT_EQ(5, xSize);
	EXPECT_EQ(5, ySize);
	EXPECT_EQ(3, xOff);
	EXPECT_EQ(7, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_TRUE(isMore);
	EXPECT_EQ(3, xSize);
	EXPECT_EQ(5, ySize);
	EXPECT_EQ(7, xOff);
	EXPECT_EQ(7, yOff);

	isMore = it->next(&xSize, &ySize, &xOff, &yOff);
	EXPECT_FALSE(isMore);
}

class ProcessTest: public testing::Test {

protected:
	virtual void SetUp() {
	}

	virtual void TearDown() {
		std::remove("temp.tif");
	}
};

TEST_F(ProcessTest, BaseProcess) {
	RasterProcess process;
	IProcessImage baseproc;
	int xsize = 5, ysize = 5, buffer = 0;
	GALGError err = process.map(baseproc, file_name, "temp.tif", &xsize, &ysize, &buffer, false);
	EXPECT_EQ(err.errnum, 0);
	std::remove("temp.tif");
}

TEST_F(ProcessTest, Threshold) {
	RasterProcess process;
	Threshold threshold;
	double thresh = 10.0, maxV = 10.0;
	threshold.setThresholdParams(thresh, maxV, (int)THRESH_TRUNC);
	int xsize = 5, ysize = 5, buffer = 0;
	GALGError err = process.map(threshold, file_name, "temp.tif", &xsize, &ysize, &buffer, false);
	EXPECT_EQ(err.errnum, 0);

	// Check the maximum value is 10.0
	GDALDataset *src = (GDALDataset *)GDALOpen("temp.tif", GA_ReadOnly);
	double minVal, maxVal, meanVal, stdDev;
	src->GetRasterBand(1)->GetStatistics(0, 1, &minVal, &maxVal, &meanVal, &stdDev);
	EXPECT_EQ(maxVal, maxV);
	GDALClose(src);
}

}

int main(int argc, char** argv) {
::testing::InitGoogleTest(&argc, argv);
assert(argc == 2); // gtest leaved unparsed arguments for you

file_name = argv[1];
return RUN_ALL_TESTS();

}
