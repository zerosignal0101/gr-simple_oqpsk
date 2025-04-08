/* -*- c++ -*- */
/*
 * Copyright 2025 zerosignal.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SIMPLE_OQPSK_OQPSKMOD_H
#define INCLUDED_SIMPLE_OQPSK_OQPSKMOD_H

#include <gnuradio/block.h>
#include <gnuradio/simple_oqpsk/api.h>

namespace gr {
namespace simple_oqpsk {

/*!
 * \brief <+description of block+>
 * \ingroup simple_oqpsk
 *
 */
class SIMPLE_OQPSK_API oqpskMod : virtual public gr::block
{
public:
    typedef std::shared_ptr<oqpskMod> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of simple_oqpsk::oqpskMod.
     *
     * To avoid accidental use of raw pointers, simple_oqpsk::oqpskMod's
     * constructor is in a private implementation
     * class. simple_oqpsk::oqpskMod::make is the public interface for
     * creating new instances.
     */
    static sptr
    make(bool debug = false, int samples_per_symbol = 4, float rolloff = 0.35);
};

} // namespace simple_oqpsk
} // namespace gr

#endif /* INCLUDED_SIMPLE_OQPSK_OQPSKMOD_H */
