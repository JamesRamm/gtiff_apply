# GeoTiffApply

`map` and `reduce` functions for geotiff datasets.

GeoTiffApply provides functionality for processing large raster datasets with user-defined functions using `map` and `reduce` philosophies. 

## Status
GeoTiffApply is currently in its' infancy - more of the start of a collection of ideas than something releasable yet!

## Project Goals
- Provide `map` and `reduce` functions for user-defined processes against (geospatial) raster datasets of any size. These functions can handle any kind of iteration (different block sizes, overlaps/buffers, 'mosaic' iterators) and can also chain user-defined processes to be applied to each block.

GeoTiffApply aims to make it easy to define raster processing without having to worry about the boilerplate of actually applying it to the raster, no matter the size of the raster. 

## Building

GeoTiffApply uses CMake for building on all platforms.
GeoTiffApply is split into two libraries: `galgcore` and `galgfunc`.
The `galgcore` library is dependent upon GDAL (trunk) and `galgfunc` is dependent upon OpenCV.
To build the tests, googletest is required.

### Building on Windows

To build on windows, ensure that OpenCV and GDAL are built and the binary location is added to your system PATH.
For CMake to be able to locate GDAL, declare an environment variable called `GDAL_ROOT` pointing to the GDAL folder. 
For building tests, an environment variable called `GTEST_ROOT` pointing to the googletest installation folder may also be necessary.
For OpenCV, add the OpenCV binary folder to CMAKE_PREFIX_PATH. This can be added on the command line like:

   cmake -DCMAKE_PREFIX_PATH=C:\path\to\opencv\build CMakeLists.txt
 
Or declared in the GUI. 
  


