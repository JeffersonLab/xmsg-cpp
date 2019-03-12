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

#ifndef XMSG_CORE_ZHELPER_H
#define XMSG_CORE_ZHELPER_H

#include <zmq.hpp>

#include <array>
#include <string>

namespace xmsg {
namespace detail {

class Context
{
public:
    zmq::socket_t create_socket(zmq::socket_type type)
    {
        auto out = zmq::socket_t{ctx_, type};
        out.setsockopt(ZMQ_RCVHWM, 0);
        out.setsockopt(ZMQ_SNDHWM, 0);
        return out;
    }

    void set_option(int opt, int val)
    {
        int rc = zmq_ctx_set(static_cast<void*>(ctx_), opt, val);
        if (rc < 0) {
            throw zmq::error_t{};
        }
    }

    int get_option(int opt)
    {
        int rc = zmq_ctx_get(static_cast<void*>(ctx_), opt);
        if (rc < 0) {
            throw zmq::error_t{};
        }
        return rc;
    }

    void close()
    {
        ctx_.close();
    }

private:
    zmq::context_t ctx_;
};


class BasicPoller final
{
public:
    explicit BasicPoller(zmq::socket_t& socket)
      : items_{{static_cast<void*>(socket), 0, ZMQ_POLLIN, 0}}
    { }

    bool poll(int timeout)
    {
        zmq::poll(items_.data(), 1, timeout);
        return (items_[0].revents & ZMQ_POLLIN) != 0;
    }

private:
    using Items = std::array<zmq::pollitem_t, 1>;
    Items items_;
};


class RawMessage {
public:
    RawMessage(zmq::socket_t& socket);

    auto& operator[](std::size_t idx)
    {
        return parts[idx];
    }

    std::size_t size()
    {
        return counter;
    }

private:
    static const size_t msg_size = 3;

    std::size_t counter = 0;
    std::array<zmq::message_t, msg_size> parts;
};


inline
void bind(zmq::socket_t& socket, int port)
{
    socket.bind("tcp://*:" + std::to_string(port));
}


inline
void connect(zmq::socket_t& socket, const std::string& host, int port)
{
    socket.connect("tcp://" + host + ":" + std::to_string(port));
}


inline
std::string to_string(const zmq::message_t& msg)
{
    return {msg.data<const char>(), msg.size()};
}


inline
std::vector<std::uint8_t> to_bytes(const zmq::message_t& msg)
{
    auto ptr = msg.data<std::uint8_t>();
    return {ptr, ptr + msg.size()};
}


std::string get_unique_replyto(const std::string& subject);

void set_unique_replyto(std::uint_fast32_t value);

std::string encode_identity(const std::string& address, const std::string& name);

std::string get_random_id();

} // end namespace detail
} // end namespace xmsg

#endif // XMSG_CORE_ZHELPER_H
