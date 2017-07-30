# GeoTiffApply

`map` and `reduce` functions for geotiff datasets.

GeoTiffApply provides functionality for processing large raster datasets with user-defined functions using `map` and `reduce` philosophies. 
It can be used to efficiently process single datasets which are large than the available memory. 

## Status
The core library for mapping a function across a raster is in a usable state, if not well documented yet. 
The function library is essentially just an example at this point - you will need to implement your own processing functions!

## Project Goals
- Provide `map` and `reduce` functions for user-defined processes against (geospatial) raster datasets of any size. These functions can handle any kind of iteration (different block sizes, overlaps/buffers, 'mosaic' iterators) and can also chain user-defined processes to be applied to each block.

GeoTiffApply aims to make it easy to define raster processing without having to worry about the boilerplate of actually applying it to the raster, no matter the size of the raster. 

## Building

GeoTiffApply uses CMake for building on all platforms.
GeoTiffApply is split into two libraries: `core` and `alg`.
The `core` library is dependent upon GDAL (trunk) and `alg` is dependent upon OpenCV.
To build the tests, googletest is required.

### Building on Windows

To build on windows, ensure that OpenCV and GDAL are built and the binary location is added to your system PATH.
For CMake to be able to locate GDAL, declare an environment variable called `GDAL_ROOT` pointing to the GDAL folder. 
For building tests, an environment variable called `GTEST_ROOT` pointing to the googletest installation folder may also be necessary.
For OpenCV, add the OpenCV binary folder to CMAKE_PREFIX_PATH. This can be added on the command line like:

   cmake -DCMAKE_PREFIX_PATH=C:\path\to\opencv\build CMakeLists.txt
 
Or declared in the GUI. 
  
### Usage

`galg.h` defines the `RasterProcess` class which provides the `map` and `mapMany` functions for mapping a user defined function across the dataset.

The processing function is created by implementing the `IProcessImage` interface. This function receives an input array, output array, the X window size and the Y window size. The arrays a single dimension, with a size of X * Y. The value representing the input no data value and output no data values are also passed in. 

When calling `RasterProcess::map` you pass a pointer to your `IProcessImage` object, the path of the input raster, the desired path of the output raster, the desired window X and Y sizes (optional) a desired pixel buffer and a boolean stating whether to skip holes and create a sparse output dataset.  
The pixel buffer defines an overlap between windows, which is useful for implementing functions which rely on accessing pixel neighbours. 

For an example of implementing `IProcessImage`, see `alg/threshold.cpp`.
For an example of using `RasterProcess`, see the tests.




