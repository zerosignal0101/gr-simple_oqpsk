/* -*- c++ -*- */
/*
 * Copyright 2025 zerosignal.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "pduToConStream_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace simple_oqpsk {

using output_type = uint8_t;
pduToConStream::sptr
pduToConStream::make(bool debug, const std::string& tag_name, float sample_rate)
{
    return gnuradio::make_block_sptr<pduToConStream_impl>(debug, tag_name, sample_rate);
}


/*
 * The private constructor
 */
pduToConStream_impl::pduToConStream_impl(bool debug,
                                         const std::string& tag_name,
                                         float sample_rate)
    : gr::block("pduToConStream",
                gr::io_signature::make(0, 0, 0), // No streaming input
                gr::io_signature::make(1, 1, sizeof(output_type))),
                d_debug(debug),
                d_tag_name(tag_name),
                d_sample_rate(sample_rate),
                d_pdu_queue(),
                d_current_pdu(),
                d_pdu_offset(0),
                d_last_time(std::chrono::steady_clock::now()),
                d_items_per_second(sample_rate),
                d_items_per_us(sample_rate / 1e6),
                d_next_tag_offset(0)
{
    message_port_register_in(pmt::mp("pdu"));
    set_msg_handler(pmt::mp("pdu"), [this](pmt::pmt_t msg) {
        this->handle_pdu(msg);
    });
    if (debug == true) {
        std::cout << "PDU to Stream block initialized with:" << std::endl;
        std::cout << "  Tag name: " << d_tag_name << std::endl;
        std::cout << "  Sample rate: " << d_sample_rate << " samples/sec" << std::endl;
    }
}

/*
 * Our virtual destructor.
 */
pduToConStream_impl::~pduToConStream_impl() {}

/*
 * PDU input handler.
 */
void pduToConStream_impl::handle_pdu(pmt::pmt_t msg) {
    gr::thread::scoped_lock guard(d_mutex);
    
    // Check if this is a valid PDU
    if (!pmt::is_pair(msg)) {
        std::cerr << "Received invalid PDU (not a pair)" << std::endl;
        return;
    }
    
    pmt::pmt_t metadata = pmt::car(msg);
    pmt::pmt_t vector = pmt::cdr(msg);
    
    if (!pmt::is_u8vector(vector)) {
        std::cerr << "Received invalid PDU (data not u8vector)" << std::endl;
        return;
    }
    
    size_t pdu_len = pmt::length(vector);
    const uint8_t* pdu_data = (const uint8_t*)pmt::uniform_vector_elements(vector, pdu_len);
    
    // Copy the PDU data to our queue
    std::vector<uint8_t> new_pdu(pdu_data, pdu_data + pdu_len);
    d_pdu_queue.push_back(new_pdu);
    
    if (d_debug == true) {
        std::cout << "Received new PDU with length " << pdu_len << " bytes" << std::endl;
        std::cout << "Current PDU queue size: " << d_pdu_queue.size() << std::endl;
    }
}

void pduToConStream_impl::forecast(int noutput_items,
                                   gr_vector_int& ninput_items_required)
{
#pragma message( \
    "implement a forecast that fills in how many items on each input you need to produce noutput_items and remove this warning")
    /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
}

int pduToConStream_impl::general_work(int noutput_items,
                                      gr_vector_int& ninput_items,
                                      gr_vector_const_void_star& input_items,
                                      gr_vector_void_star& output_items)
{
    gr::thread::scoped_lock guard(d_mutex);
    uint8_t *out = (uint8_t *)output_items[0];

    // Calculate how many items we should produce based on time elapsed
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - d_last_time);
    double elapsed_seconds = elapsed.count() / 1e6;
    int target_items = static_cast<int>(std::round(elapsed_seconds * d_sample_rate));

    // Limit to the requested noutput_items
    int items_to_produce = std::min(target_items, noutput_items);
    if (items_to_produce <= 0) {
        return 0; // Not enough time has passed to produce any items
    }
    
    if (d_debug == true) {
        std::cout << "Work called: noutput_items=" << noutput_items 
                << ", target_items=" << target_items
                << ", producing=" << items_to_produce << std::endl;
    }

    int produced = 0;
    while (produced < items_to_produce) {
        // Check if we need to start a new PDU
        if (!d_current_pdu.empty() && d_pdu_offset >= d_current_pdu.size()) {
            if (d_debug == true) {
                std::cout << "Finished processing current PDU (length=" 
                        << d_current_pdu.size() << ")" << std::endl;
            }
            d_current_pdu.clear();
            d_pdu_offset = 0;
        }
        
        if (d_current_pdu.empty() && !d_pdu_queue.empty()) {
            d_current_pdu = d_pdu_queue.front();
            d_pdu_queue.pop_front();
            d_pdu_offset = 0;
            
            // The next tag will be placed at the current produced count
            d_next_tag_offset = produced;
            if (d_debug == true) {
                std::cout << "Starting new PDU (length=" << d_current_pdu.size() 
                            << "), queue size now=" << d_pdu_queue.size() << std::endl;
            }
        }
        
        // Determine how many items we can produce in this iteration
        int remaining_request = items_to_produce - produced;
        int remaining_pdu = d_current_pdu.empty() ? 0 : (d_current_pdu.size() - d_pdu_offset);
        int chunk_size = std::min(remaining_request, remaining_pdu);
        
        if (chunk_size > 0) {
            // Copy data from current PDU
            memcpy(out + produced, d_current_pdu.data() + d_pdu_offset, chunk_size);
            d_pdu_offset += chunk_size;
            produced += chunk_size;
            
            // Add packet length tag at the start of the PDU
            if (d_pdu_offset == chunk_size) { // Just started this PDU
                add_item_tag(0, // Port number
                            nitems_written(0) + d_next_tag_offset, // Offset
                            pmt::string_to_symbol(d_tag_name),
                            pmt::from_long(d_current_pdu.size()));
                if (d_debug == true) {
                    std::cout << "Added tag '" << d_tag_name << "'=" << d_current_pdu.size()
                            << " at offset " << (nitems_written(0) + d_next_tag_offset) << std::endl;
                }
            }
        } else {
            // No PDU data available - insert padding
            int padding_size = remaining_request;
            memset(out + produced, 1, padding_size);
            produced += padding_size;
            if (d_debug == true) {
                std::cout << "Inserted " << padding_size << ")" << std::endl;
            }
        }
    }
    
    // Update timing
    d_last_time = now;
    if (d_debug == true) {
        std::cout << "Produced " << produced << " items in this work call" << std::endl;
    }
    return produced;
}

} /* namespace simple_oqpsk */
} /* namespace gr */
