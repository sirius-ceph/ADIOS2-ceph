#------------------------------------------------------------------------------#
# Distributed under the OSI-approved Apache License, Version 2.0.  See
# accompanying file Copyright.txt for details.
#------------------------------------------------------------------------------#

cmake_minimum_required(VERSION 3.6)

# Fail immediately if not using an out-of-source build
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR
    "In-source builds are not supported.  Please create a build directory "
    "separate from the source directory")
endif()

project(ADIOS2 VERSION 2.1.0)

#------------------------------------------------------------------------------#
# Some boilerplate to setup nice output directories
#------------------------------------------------------------------------------#
set(CMAKE_INSTALL_BINDIR bin)
set(CMAKE_INSTALL_LIBDIR lib)
set(CMAKE_INSTALL_INCLUDEDIR include)
set(CMAKE_INSTALL_CMAKEDIR lib/cmake/adios2)

list(INSERT CMAKE_MODULE_PATH 0 "${ADIOS2_SOURCE_DIR}/cmake")
if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    ${ADIOS2_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
endif()
if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${ADIOS2_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
endif()
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${ADIOS2_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
endif()

# Let windows builds auto-export dll symbols
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

#------------------------------------------------------------------------------#
# Silence MSVC warnings
#------------------------------------------------------------------------------#

if(MSVC)
  add_definitions(
    -D_CRT_SECURE_NO_DEPRECATE
    -D_CRT_SECURE_NO_WARNINGS
    -D_SCL_SECURE_NO_DEPRECATE
    -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
  )
endif()

#------------------------------------------------------------------------------#
# Top level options
#------------------------------------------------------------------------------#
include(ADIOSFunctions)

# Default to a debug build if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

# Force C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(CMakeDependentOption)

# Setup shared library defaults.  If explicitly specified somehow, then default 
# to that.  Otherwise base the default on whether or not shared libs are even
# supported.
get_property(SHARED_LIBS_SUPPORTED GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)
cmake_dependent_option(BUILD_SHARED_LIBS
  "Build shared libraries (so/dylib/dll)." ${SHARED_LIBS_SUPPORTED}
  "SHARED_LIBS_SUPPORTED" OFF
)
mark_as_advanced(BUILD_SHARED_LIBS)

# Setup PIC defaults.  If explicitly specified somehow, then default 
# to that.  Otherwise base the default on whether or not shared libs are even
# supported.
if(DEFINED ADIOS2_ENABLE_PIC)
  set(ADIOS2_ENABLE_PIC_DEFAULT ${ADIOS2_ENABLE_PIC})
elseif(DEFINED CMAKE_POSITION_INDEPENDENT_CODE)
  set(ADIOS2_ENABLE_PIC_DEFAULT ${CMAKE_POSITION_INDEPENDENT_CODE})
else()
  set(ADIOS2_ENABLE_PIC_DEFAULT ${SHARED_LIBS_SUPPORTED})
endif()
cmake_dependent_option(ADIOS2_ENABLE_PIC
  "Build with Position Independent Code" ${ADIOS2_ENABLE_PIC_DEFAULT}
  "SHARED_LIBS_SUPPORTED" OFF
)
set(CMAKE_POSITION_INDEPENDENT_CODE ${ADIOS2_ENABLE_PIC})
mark_as_advanced(CMAKE_POSITION_INDEPENDENT_CODE)

adios_option(BZip2     "Enable support for BZip2 transforms" AUTO)
adios_option(ZFP       "Enable support for ZFP transforms" AUTO)
adios_option(MPI       "Enable support for MPI" AUTO)
adios_option(DataMan   "Enable support for DataMan" AUTO)
adios_option(SST       "Enable support for SST" AUTO)
adios_option(ZeroMQ    "Enable support for ZeroMQ" AUTO)
adios_option(HDF5      "Enable support for the HDF5 engine" AUTO)
adios_option(ADIOS1    "Enable support for the ADIOS 1.x engine" AUTO)
adios_option(Python    "Enable support for Python bindings" AUTO)
adios_option(Fortran   "Enable support for Fortran bindings" OFF)
adios_option(SysVShMem "Enable support for SysV Shared Memory IPC on *NIX" AUTO)
include(${ADIOS2_SOURCE_DIR}/cmake/DetectOptions.cmake)

if(ADIOS2_HAVE_MPI)
  # Workaround for MPI forcing the link of C++ bindings
  add_definitions(-DOMPI_SKIP_MPICXX -DMPICH_SKIP_MPICXX)
endif()

set(ADIOS2_CONFIG_OPTS
  BZip2 ZFP MPI DataMan SST ZeroMQ HDF5 ADIOS1 Python Fortran SysVShMem
)
GenerateADIOSHeaderConfig(${ADIOS2_CONFIG_OPTS})
configure_file(
  ${ADIOS2_SOURCE_DIR}/CTestCustom.cmake.in
  ${ADIOS2_BINARY_DIR}/CTestCustom.cmake
  @ONLY
)

install(FILES ${ADIOS2_BINARY_DIR}/source/adios2/ADIOSConfig.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/adios2
)

#------------------------------------------------------------------------------#
# Third party libraries
#------------------------------------------------------------------------------#
include(CTest)
mark_as_advanced(BUILD_TESTING)
add_subdirectory(thirdparty)

#------------------------------------------------------------------------------#
# Main library source
#------------------------------------------------------------------------------#
add_subdirectory(source)

#------------------------------------------------------------------------------#
# Language bindings
#------------------------------------------------------------------------------#
add_subdirectory(bindings)

#------------------------------------------------------------------------------#
# Examples
#------------------------------------------------------------------------------#
option(ADIOS2_BUILD_EXAMPLES "Build examples" ON)
option(ADIOS2_BUILD_EXAMPLES_EXPERIMENTAL "Build experimental examples" OFF)
if(ADIOS2_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

#------------------------------------------------------------------------------#
# Testing
#------------------------------------------------------------------------------#
if(DEFINED BUILD_TESTING)
  set(ADIOS2_BUILD_TESTING_DEFAULT ${BUILD_TESTING})
else()
  set(ADIOS2_BUILD_TESTING_DEFAULT ON)
endif()
unset(BUILD_TESTING)
option(ADIOS2_BUILD_TESTING "Build the ADIOS2 testing tree" 
       ${ADIOS2_BUILD_TESTING_DEFAULT})
       
include(CTest)
set(BUILD_TESTING ${ADIOS2_BUILD_TESTING})
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(testing)
endif()

#------------------------------------------------------------------------------#
# Generating package configs
#------------------------------------------------------------------------------#
GenerateADIOSPackageConfig()

#------------------------------------------------------------------------------#
# Configuration summary
#------------------------------------------------------------------------------#
message("")
message("ADIOS2 build configuration:")
message("  ADIOS Version: ${ADIOS2_VERSION}")
message("  C++ Compiler : ${CMAKE_CXX_COMPILER_ID} " 
                         "${CMAKE_CXX_COMPILER_VERSION} " 
                         "${CMAKE_CXX_COMPILER_WRAPPER}")
message("    ${CMAKE_CXX_COMPILER}")
message("")
if(ADIOS2_HAVE_Fortran)
  message("  Fortran Compiler : ${CMAKE_Fortran_COMPILER_ID} "
                               "${CMAKE_Fortran_COMPILER_VERSION} "
                               "${CMAKE_Fortran_COMPILER_WRAPPER}")
  message("    ${CMAKE_Fortran_COMPILER}")
  message("")
endif()
message("  Installation prefix: ${CMAKE_INSTALL_PREFIX}")
message("        bin: ${CMAKE_INSTALL_BINDIR}")
message("        lib: ${CMAKE_INSTALL_LIBDIR}")
message("    include: ${CMAKE_INSTALL_INCLUDEDIR}")
message("      cmake: ${CMAKE_INSTALL_CMAKEDIR}")
if(ADIOS2_HAVE_Python)
  message("     python: ${CMAKE_INSTALL_PYTHONDIR}")
endif()
message("")
message("  Features:")
if(BUILD_SHARED_LIBS)
  set(msg_lib_type "shared")
elseif(CMAKE_POSITION_INDEPENDENT_CODE)
  set(msg_lib_type "static (with PIC)")
else()
  set(msg_lib_type "static (without PIC)")
endif()
message("    Library Type: ${msg_lib_type}")
message("    Build Type:   ${CMAKE_BUILD_TYPE}")
message("    Testing: ${BUILD_TESTING}")
message("    Build Options:")

foreach(opt IN LISTS ADIOS2_CONFIG_OPTS)
  message_pad("      ${opt}" 15 label)
  if(${ADIOS2_HAVE_${opt}})
    message("${label}: ON")
  else()
    message("${label}: OFF")
  endif()
endforeach()
message("")
