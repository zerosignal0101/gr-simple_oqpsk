/* -*- c++ -*- */
/*
 * Copyright 2025 zerosignal.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SIMPLE_OQPSK_PDUTOCONSTREAM_IMPL_H
#define INCLUDED_SIMPLE_OQPSK_PDUTOCONSTREAM_IMPL_H

#include <gnuradio/simple_oqpsk/pduToConStream.h>

namespace gr {
namespace simple_oqpsk {

class pduToConStream_impl : public pduToConStream
{
private:
    // Nothing to declare in this block.

public:
    pduToConStream_impl(bool debug,
        const std::string& tag_name,
        float sample_rate);
    ~pduToConStream_impl();

    // Where all the action really happens
    void forecast(int noutput_items, gr_vector_int& ninput_items_required);

    int general_work(int noutput_items,
                     gr_vector_int& ninput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items);
};

} // namespace simple_oqpsk
} // namespace gr

#endif /* INCLUDED_SIMPLE_OQPSK_PDUTOCONSTREAM_IMPL_H */
