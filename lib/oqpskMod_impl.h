/* -*- c++ -*- */
/*
 * Copyright 2025 zerosignal.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SIMPLE_OQPSK_OQPSKMOD_IMPL_H
#define INCLUDED_SIMPLE_OQPSK_OQPSKMOD_IMPL_H

#include <gnuradio/simple_oqpsk/oqpskMod.h>
#include <vector>
#include <cmath>
#include <stdexcept>

namespace gr {
namespace simple_oqpsk {

class oqpskMod_impl : public oqpskMod
{
private:
    bool d_debug;
    int d_samples_per_symbol;
    float d_rolloff;
    
    // Pulse shaping filter
    std::vector<float> d_rrc_taps;
    
    // State variables
    int d_symbol_count;
    unsigned char d_current_byte;
    int d_bit_count;
    float d_i_phase, d_q_phase;
    
    // Generate RRC filter taps
    void generate_rrc_taps();
    
    // Map bits to symbol
    void map_bits_to_symbol(unsigned char bits, float* i, float* q);

public:
    oqpskMod_impl(bool debug, int samples_per_symbol, float rolloff);
    ~oqpskMod_impl();

    // Where all the action really happens
    void forecast(int noutput_items, gr_vector_int& ninput_items_required);

    int general_work(int noutput_items,
                     gr_vector_int& ninput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items);
};

} // namespace simple_oqpsk
} // namespace gr

#endif /* INCLUDED_SIMPLE_OQPSK_OQPSKMOD_IMPL_H */
