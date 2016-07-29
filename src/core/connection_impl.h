/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim: set ts=8 sts=4 et sw=4 tw=80: */
/*
 * Copyright (C) 2015. Jefferson Lab, xMsg framework (JLAB). All Rights Reserved.
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research, and not-for-profit purposes,
 * without fee and without a signed licensing agreement.
 *
 * Contact Vardan Gyurjyan
 * Department of Experimental Nuclear Physics, Jefferson Lab.
 *
 * IN NO EVENT SHALL JLAB BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL,
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF
 * THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF JLAB HAS BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * JLAB SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE. THE CLARA SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
 * HEREUNDER IS PROVIDED "AS IS". JLAB HAS NO OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef XMSG_CORE_PRIVATE_H_
#define XMSG_CORE_PRIVATE_H_

#include "connection_driver.h"

#include <xmsg/connection_setup.h>

#include "zhelper.h"

namespace xmsg {
namespace detail {

struct ProxyDriver::Impl {
    Impl(zmq::context_t& ctx,
         ProxyAddress addr,
         std::shared_ptr<ConnectionSetup>&& setup)
      : addr{addr},
        setup{std::move(setup)},
        pub{ctx, zmq::socket_type::pub},
        sub{ctx, zmq::socket_type::sub},
        control{ctx, zmq::socket_type::dealer},
        id{core::get_random_id()}
    { }

    ProxyAddress addr;
    std::shared_ptr<ConnectionSetup> setup;
    zmq::socket_t pub;
    zmq::socket_t sub;
    zmq::socket_t control;
    std::string id;
};

} // end namespace detail
} // end namespace xmsg

#endif // XMSG_CORE_PRIVATE_H_
