/* -*- c++ -*- */
/*
 * Copyright 2025 zerosignal.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SIMPLE_OQPSK_PDUTOCONSTREAM_H
#define INCLUDED_SIMPLE_OQPSK_PDUTOCONSTREAM_H

#include <gnuradio/block.h>
#include <gnuradio/simple_oqpsk/api.h>

namespace gr {
namespace simple_oqpsk {

/*!
 * \brief <+description of block+>
 * \ingroup simple_oqpsk
 *
 */
class SIMPLE_OQPSK_API pduToConStream : virtual public gr::block
{
public:
    typedef std::shared_ptr<pduToConStream> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of simple_oqpsk::pduToConStream.
     *
     * To avoid accidental use of raw pointers, simple_oqpsk::pduToConStream's
     * constructor is in a private implementation
     * class. simple_oqpsk::pduToConStream::make is the public interface for
     * creating new instances.
     */
    static sptr make();
};

} // namespace simple_oqpsk
} // namespace gr

#endif /* INCLUDED_SIMPLE_OQPSK_PDUTOCONSTREAM_H */
