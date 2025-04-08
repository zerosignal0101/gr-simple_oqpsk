/* -*- c++ -*- */
/*
 * Copyright 2025 zerosignal.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "oqpskMod_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace simple_oqpsk {

using input_type = uint8_t;
using output_type = gr_complex;
oqpskMod::sptr oqpskMod::make(bool debug, int samples_per_symbol, float rolloff)
{
    return gnuradio::make_block_sptr<oqpskMod_impl>(debug, samples_per_symbol, rolloff);
}


/*
 * The private constructor
 */
oqpskMod_impl::oqpskMod_impl(bool debug, int samples_per_symbol, float rolloff)
    : gr::block("oqpskMod",
                gr::io_signature::make(
                    1 /* min inputs */, 1 /* max inputs */, sizeof(input_type)),
                gr::io_signature::make(
                    1 /* min outputs */, 1 /*max outputs */, sizeof(output_type))),
    d_debug(debug),
    d_samples_per_symbol(samples_per_symbol),
    d_rolloff(rolloff),
    d_symbol_count(0),
    d_current_byte(0),
    d_bit_count(0),
    d_i_phase(0.0),
    d_q_phase(0.0)
{
    // Generate RRC filter taps
    generate_rrc_taps();
      
    if(d_debug) {
      std::cout << "OQPSK Modulator Initialized:" << std::endl;
      std::cout << "  Samples per symbol: " << d_samples_per_symbol << std::endl;
      std::cout << "  Roll-off factor: " << d_rolloff << std::endl;
    }
}

/*
 * Our virtual destructor.
 */
oqpskMod_impl::~oqpskMod_impl() {}

void oqpskMod_impl::generate_rrc_taps()
{
    // Generate root-raised cosine filter taps
    int ntaps = 11 * d_samples_per_symbol;
    d_rrc_taps = gr::filter::firdes::root_raised_cosine(
    1.0,                     // gain
    d_samples_per_symbol,     // sampling rate
    1.0,                     // symbol rate
    d_rolloff,               // rolloff factor
    ntaps);                  // number of taps
}

void oqpskMod_impl::map_bits_to_symbol(unsigned char bits, float* i, float* q)
{
    // Gray coded OQPSK mapping
    switch(bits) {
    case 0: // 00
        *i = 1.0/sqrt(2.0);
        *q = 1.0/sqrt(2.0);
        break;
    case 1: // 01
        *i = -1.0/sqrt(2.0);
        *q = 1.0/sqrt(2.0);
        break;
    case 2: // 10
        *i = -1.0/sqrt(2.0);
        *q = -1.0/sqrt(2.0);
        break;
    case 3: // 11
        *i = 1.0/sqrt(2.0);
        *q = -1.0/sqrt(2.0);
        break;
    }
    
    if(d_debug) {
    std::cout << "Symbol mapped - bits: " << (int)bits 
                << " -> I: " << *i << ", Q: " << *q << std::endl;
    }
}

void oqpskMod_impl::forecast(int noutput_items, gr_vector_int& ninput_items_required)
{
#pragma message( \
    "implement a forecast that fills in how many items on each input you need to produce noutput_items and remove this warning")
    /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
}

int oqpskMod_impl::general_work(int noutput_items,
                                gr_vector_int& ninput_items,
                                gr_vector_const_void_star& input_items,
                                gr_vector_void_star& output_items)
{
    const unsigned char *in = (const unsigned char *)input_items[0];
    gr_complex *out = (gr_complex *)output_items[0];
    
    int consumed = 0;
    int produced = 0;
    
    while(produced < noutput_items) {
    if(d_bit_count == 0) {
        // Get new byte if we've processed all bits
        if(consumed < ninput_items[0]) {
        d_current_byte = in[consumed++];
        d_bit_count = 8;
        } else {
        break; // No more input
        }
    }
    
    // Process two bits at a time (one symbol)
    unsigned char bits = (d_current_byte >> (d_bit_count - 2)) & 0x03;
    d_bit_count -= 2;
    
    float i_symbol, q_symbol;
    map_bits_to_symbol(bits, &i_symbol, &q_symbol);
    
    // Apply pulse shaping with half-symbol offset for Q
    int i_offset = d_symbol_count * d_samples_per_symbol;
    int q_offset = i_offset + d_samples_per_symbol / 2;
    
    // Apply RRC filter to I and Q components
    for(int i = 0; i < d_samples_per_symbol; i++) {
        if(produced + i < noutput_items) {
        // I component (no offset)
        float i_sample = i_symbol * d_rrc_taps[i];
        
        // Q component (half-symbol offset)
        float q_sample = 0.0;
        if(i + d_samples_per_symbol/2 < d_rrc_taps.size()) {
            q_sample = q_symbol * d_rrc_taps[i + d_samples_per_symbol/2];
        }
        
        out[produced + i] = gr_complex(i_sample, q_sample);
        }
    }
    
    produced += d_samples_per_symbol;
    d_symbol_count++;
    }
    
    if(d_debug) {
    std::cout << "Processed " << consumed << " input bytes, produced " 
                << produced << " output samples" << std::endl;
    }
    
    return produced;
}

} /* namespace simple_oqpsk */
} /* namespace gr */
