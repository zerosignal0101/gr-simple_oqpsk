# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Check if there is C++ code at all
########################################################################
if(NOT simple_oqpsk_sources)
    message(STATUS "No C++ sources... skipping python bindings")
    return()
endif(NOT simple_oqpsk_sources)

########################################################################
# Check for pygccxml
########################################################################
gr_python_check_module_raw("pygccxml" "import pygccxml" PYGCCXML_FOUND)

include(GrPybind)

########################################################################
# Python Bindings
########################################################################

list(APPEND simple_oqpsk_python_files
    pduToConStream_python.cc python_bindings.cc)

gr_pybind_make_oot(simple_oqpsk ../../.. gr::simple_oqpsk "${simple_oqpsk_python_files}")

# copy bindings extension for use in QA test module
add_custom_command(
    TARGET simple_oqpsk_python
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:simple_oqpsk_python>
            ${PROJECT_BINARY_DIR}/test_modules/gnuradio/simple_oqpsk/)

install(
    TARGETS simple_oqpsk_python
    DESTINATION ${GR_PYTHON_DIR}/gnuradio/simple_oqpsk
    COMPONENT pythonapi)
