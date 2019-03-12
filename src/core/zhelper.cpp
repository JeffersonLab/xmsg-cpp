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

#include "zhelper.h"

#include "likely.h"

#include <xmsg/util.h>

#include <atomic>
#include <mutex>
#include <random>
#include <sstream>

namespace {

class RandomGenerator
{
public:
    template<typename D>
    auto operator()(D& dist)
    {
        std::lock_guard<std::mutex> guard{mtx_};
        return dist(rng_);
    }

private:
    std::mutex mtx_;

    std::mt19937_64 rng_{[]{
        std::random_device rd;
        return rd();
    }()};
} rng;


// language identifier (Java:1, C++:2, Python:3)
const auto cpp_id = 2;

std::atomic_uint_fast32_t rt_seq{[]{
    auto d = std::uniform_int_distribution<std::uint_fast32_t>{0, 999'999};
    return rng(d);
}()};

}


namespace xmsg {
namespace detail {

// Read up to 3-part messages. Any message with more parts is unexpected and
// invalid.
RawMessage::RawMessage(zmq::socket_t& socket)
{
    while (true) {
        auto& msg = parts[counter];
        socket.recv(&msg);
        ++counter;
        if (counter == msg_size || !msg.more()) {
            break;
        }
    }

    if (XMSG_UNLIKELY(parts.back().more())) {
        while (true) {
            zmq::message_t msg;
            socket.recv(&msg);
            if (!msg.more()) {
                break;
            }
        }
        throw std::runtime_error{"Invalid multi-part message"};
    }
}


// replyTo generation: format is "ret:<id>:2[dddddd]"
std::string get_unique_replyto(const std::string& subject)
{
    static constexpr auto rt_seq_size = 1'000'000;
    static constexpr auto rt_seq_base = cpp_id * rt_seq_size;

    auto id = ++rt_seq % rt_seq_size + rt_seq_base;

    return "ret:" + subject + ":" + std::to_string(id);
}


void set_unique_replyto(std::uint_fast32_t value)
{
    rt_seq = value;
}


// actor unique ID: format is 8 digits: [dddddddd]
std::string encode_identity(const std::string& address, const std::string& name)
{
    std::uniform_int_distribution<std::uint_fast8_t> dist{0, 99};
    std::string id = address + "#" + name + "#" + std::to_string(rng(dist));
    std::stringstream ss;
    ss << std::hex << std::hash<std::string>{}(id);
    return ss.str().substr(0, 8);
}


// control ID generation: format is 9 digits: 2[ppp][ddddd]
std::string get_random_id()
{
    static const auto id_prefix = []{
        auto ip_hash = std::hash<std::string>{}(util::localhost());
        return cpp_id * 100'000'000 + (ip_hash % 1000) * 100'000;
    }();

    std::uniform_int_distribution<std::uint_fast32_t> id_dist{0, 99'999};

    return std::to_string(id_prefix + rng(id_dist));
}

} // end namespace util

} // end namespace xmsg
