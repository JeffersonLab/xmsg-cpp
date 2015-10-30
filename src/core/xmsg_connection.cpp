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

#include "xmsg_connection.h"

#include "xmsg_address.h"
#include "xmsg_constants.h"
#include "xmsg_connection_impl.h"
#include "xmsg_util.h"

#include "zmq.hpp"

#include <array>

namespace xmsg {

Connection::Connection(std::unique_ptr<Connection::Impl>&& impl)
  : con_{std::move(impl)}
{ }

Connection::Connection(Connection&&) = default;
Connection& Connection::operator=(Connection&&) = default;

Connection::~Connection() = default;


void Connection::connect()
{
    con_->setup->pre_connection(con_->pub);
    con_->setup->pre_connection(con_->sub);

    auto pub_port = std::to_string(con_->addr.pub_port);
    auto sub_port = std::to_string(con_->addr.sub_port);

    con_->pub.connect("tcp://" + con_->addr.host + ":" + pub_port);
    con_->sub.connect("tcp://" + con_->addr.host + ":" + sub_port);

    con_->setup->post_connection();
}


void Connection::send(Message& msg)
{
    const auto& t = msg.topic().str();
    const auto& m = msg.meta()->SerializeAsString();
    const auto& d = msg.data();

    con_->pub.send(t.data(), t.size(), ZMQ_SNDMORE);
    con_->pub.send(m.data(), m.size(), ZMQ_SNDMORE);
    con_->pub.send(d.data(), d.size(), 0);
}


Message Connection::recv()
{
    using MultiMessage = std::array<zmq::message_t, 3>;

    auto multi_msg = MultiMessage{};
    auto counter = size_t{0};
    for (auto& msg : multi_msg) {
        con_->sub.recv(&msg);
        ++counter;
        if (!msg.more()) {
            break;
        }
    }
    if (counter != multi_msg.size() || multi_msg.end()->more()) {
        throw std::runtime_error{"Invalid multi-part message"};
    }

    auto topic = Topic::raw(multi_msg[0].data<const char>());

    auto meta = std::make_unique<xmsg::proto::Meta>();
    meta->ParseFromArray(multi_msg[1].data(), multi_msg[1].size());

    auto data_ptr = multi_msg[2].data<std::uint8_t>();
    auto data_size = multi_msg[2].size();
    auto data = std::vector<std::uint8_t>(data_ptr, data_ptr + data_size);

    return {topic, std::move(meta), std::move(data)};
}


void Connection::subscribe(const Topic& topic)
{
    auto& str = topic.str();
    con_->sub.setsockopt(ZMQ_SUBSCRIBE, str.data(), str.size());
}


void Connection::unsubscribe(const Topic& topic)
{
    auto& str = topic.str();
    con_->sub.setsockopt(ZMQ_UNSUBSCRIBE, str.data(), str.size());
}


const ProxyAddress& Connection::address()
{
    return con_->addr;
}

} // end namespace xmsg
