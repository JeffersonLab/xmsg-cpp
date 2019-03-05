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

#include <xmsg/util.h>

#include <chrono>
#include <regex>
#include <shared_mutex>
#include <system_error>
#include <thread>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>


namespace {

// clang-format off
const auto ip_regex = std::regex{"^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
                                     "(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"};
// clang-format on


std::vector<std::string> get_addresses()
{
    struct AddrList {
        AddrList() { getifaddrs(&ifl); }

        ~AddrList() { if (ifl != nullptr) freeifaddrs(ifl); }

        struct ifaddrs* ifl = nullptr;
    } ifl;

    std::vector<std::string> all_addrs;

    for (struct ifaddrs* ifa = ifl.ifl; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) {
            void* s_addr = &((struct sockaddr_in*) ifa->ifa_addr)->sin_addr;
            char addr_buf[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, s_addr, addr_buf, INET_ADDRSTRLEN);

            auto addr = std::string{addr_buf};
            if (addr.compare(0, 3, "127") != 0) {
                all_addrs.push_back(std::move(addr));
            }
        }
    }

    return all_addrs;
}


std::string get_host_address(const std::string& hostname)
{
    struct hostent* h = gethostbyname(hostname.data());
    if (h == nullptr) {
        throw std::system_error{EFAULT, std::system_category()};
    }
    return { inet_ntoa(*((struct in_addr*) h->h_addr)) };
}


class LocalAddrs
{
public:
    void update_addrs()
    {
        auto updated = get_addresses();
        std::lock_guard<std::shared_timed_mutex> lk{mtx_};
        std::swap(addr_, updated);
    }

    std::string get_first()
    {
        std::shared_lock<std::shared_timed_mutex> lk{mtx_};
        return addr_[0];
    }

    std::vector<std::string> get_addrs()
    {
        std::shared_lock<std::shared_timed_mutex> lk{mtx_};
        return addr_;
    }

private:
    std::shared_timed_mutex mtx_;
    std::vector<std::string> addr_ = get_addresses();
};


LocalAddrs& local_addrs()
{
    static auto* addrs = new LocalAddrs{};
    return *addrs;
}

}


namespace xmsg {

namespace util {

std::string localhost()
{
    return local_addrs().get_first();
}


std::vector<std::string> get_localhost_addrs()
{
    return local_addrs().get_addrs();
}


void update_localhost_addrs()
{
    local_addrs().update_addrs();
}


std::string to_host_addr(const std::string& hostname)
{
    if (is_ipaddr(hostname)) {
        return { hostname };
    }
    if (hostname == "localhost") {
        return local_addrs().get_first();
    }
    return get_host_address(hostname);
}


bool is_ipaddr(const std::string& hostname)
{
    return std::regex_match(hostname, ip_regex);
}


void validate_ipaddr(const std::string& address)
{
    if (!is_ipaddr(address)) {
        throw std::invalid_argument{"invalid IP address: " + address};
    }
}


void sleep(long millis)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(millis));
}

} // end namespace util

} // end namespace xmsg
