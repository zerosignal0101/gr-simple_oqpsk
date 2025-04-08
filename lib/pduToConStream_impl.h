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
    bool d_debug;
    std::string d_tag_name;
    float d_sample_rate;

    std::deque<std::vector<uint8_t>> d_pdu_queue;
    std::vector<uint8_t> d_current_pdu;
    size_t d_pdu_offset;
    
    std::chrono::time_point<std::chrono::steady_clock> d_last_time;
    double d_items_per_second;
    double d_items_per_us;
    uint64_t d_next_tag_offset;
    
    gr::thread::mutex d_mutex;

    void handle_pdu(pmt::pmt_t msg);

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
