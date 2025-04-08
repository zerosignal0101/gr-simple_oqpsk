find_package(PkgConfig)

PKG_CHECK_MODULES(PC_GR_SIMPLE_OQPSK gnuradio-simple_oqpsk)

FIND_PATH(
    GR_SIMPLE_OQPSK_INCLUDE_DIRS
    NAMES gnuradio/simple_oqpsk/api.h
    HINTS $ENV{SIMPLE_OQPSK_DIR}/include
        ${PC_SIMPLE_OQPSK_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    GR_SIMPLE_OQPSK_LIBRARIES
    NAMES gnuradio-simple_oqpsk
    HINTS $ENV{SIMPLE_OQPSK_DIR}/lib
        ${PC_SIMPLE_OQPSK_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          )

include("${CMAKE_CURRENT_LIST_DIR}/gnuradio-simple_oqpskTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GR_SIMPLE_OQPSK DEFAULT_MSG GR_SIMPLE_OQPSK_LIBRARIES GR_SIMPLE_OQPSK_INCLUDE_DIRS)
MARK_AS_ADVANCED(GR_SIMPLE_OQPSK_LIBRARIES GR_SIMPLE_OQPSK_INCLUDE_DIRS)
