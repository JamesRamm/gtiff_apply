cmake_minimum_required (VERSION 3.0)
project (GeoTiffMap)

# configure a header file to pass some of the CMake settings
# to the source code
# The version number.
set (GeoAlg_VERSION_MAJOR 0)
set (GeoAlg_VERSION_MINOR 1)
configure_file (
  "${PROJECT_SOURCE_DIR}/src/core/GeoTiffMapConfig.h.in"
  "${PROJECT_SOURCE_DIR}/src/core/GeoTiffMapConfig.h"
  )
  
# Requires GDAL to proceed
include(FindGDAL)
find_package(GDAL REQUIRED)
if (NOT GDAL_FOUND)
    message( FATAL_ERROR "GDAL package not found." )
endif (NOT GDAL_FOUND)

include_directories ("${PROJECT_SOURCE_DIR}/src/core")
include_directories ("${GDAL_INCLUDE_DIR}")

# glob all the sources
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/core/*.cpp")
add_library(galgcore SHARED ${SOURCES})
target_link_libraries(galgcore ${GDAL_LIBRARY})

include (GenerateExportHeader)
GENERATE_EXPORT_HEADER( galgcore
             BASE_NAME GeoTiffMap
             EXPORT_MACRO_NAME GALGCORE_DLL
             EXPORT_FILE_NAME ${PROJECT_SOURCE_DIR}/src/core/core_exp.h
             STATIC_DEFINE GeoTiffMap_BUILT_AS_STATIC
)

### ALG library
include_directories ("${PROJECT_SOURCE_DIR}/src/alg")
find_package(OpenCV REQUIRED)
add_library(galgfunc SHARED "src/alg/threshold.cpp")
target_link_libraries(galgfunc galgcore ${OpenCV_LIBS})

GENERATE_EXPORT_HEADER( galgfunc
             BASE_NAME GeoTiffMap
             EXPORT_MACRO_NAME GALGFUNC_DLL
             EXPORT_FILE_NAME ${PROJECT_SOURCE_DIR}/src/alg/func_exp.h
             STATIC_DEFINE GeoTiffMap_BUILT_AS_STATIC
)

### TESTS
# Tests require googletest
include(FindGTest)
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})


# If *nix, pthread must be specified *after* the googletest libs
if(WIN32)
  set (PTHREAD "")
else(WIN32)
  set (PTHREAD pthread)
endif(WIN32)

add_executable(galgtest test/galg_unittest.cpp)
target_link_libraries(galgtest ${GTEST_BOTH_LIBRARIES} galgcore galgfunc ${PTHREAD})
add_test(AllTestsInGalg galgtest "${CMAKE_CURRENT_LIST_DIR}/test/10_12_1.tif")


### SWIG
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

find_package(PythonLibs)
include_directories(${PYTHON_INCLUDE_PATH})

file(GLOB galg_HEADERS "${PROJECT_SOURCE_DIR}/src/core/*.h")
if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4127")
  add_definitions(-DSWIG_PYTHON_INTERPRETER_NO_DEBUG)
  set(CMAKE_SWIG_FLAGS "-D_SWIG_WIN32")
endif()
set_source_files_properties("${PROJECT_SOURCE_DIR}/python/galg.i" PROPERTIES CPLUSPLUS ON)
set_property(SOURCE "${PROJECT_SOURCE_DIR}/python/galg.i" PROPERTY SWIG_FLAGS "-builtin;-threads")
swig_add_module(galg python "${PROJECT_SOURCE_DIR}/python/galg.i" ${galg_HEADERS})
swig_link_libraries(galg galgcore ${PYTHON_LIBRARIES})

###installations
file(GLOB HEADERS "${PROJECT_SOURCE_DIR}/src/core/*.h" "${PROJECT_SOURCE_DIR}/src/alg/*.h")
install(TARGETS galgcore galgfunc galgtest EXPORT GeoTiffMap DESTINATION bin)
install(FILES ${HEADERS} DESTINATION include)
