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

#include <xmsg/proxy.h>

#include <xmsg/constants.h>
#include <xmsg/util.h>

#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#include <unistd.h>

static volatile sig_atomic_t signal_value = 0;


static void signal_handler(int signal)
{
    signal_value = signal;
}


static void wait_signals()
{
    struct sigaction action;

    action.sa_handler = signal_handler;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);

    sigaction(SIGINT, &action, nullptr);
    sigaction(SIGTERM, &action, nullptr);

    pause();

    if (signal_value == SIGINT) {
        std::cout << std::endl;
    }
    std::cout << "Exiting..." << std::endl;
}


std::string current_time()
{
    time_t now;
    std::time(&now);
    char buf[sizeof "2001-01-01 00:00:00"];
    std::strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return buf;
}


// Based on <https://stackoverflow.com/a/868894/401753> by
// <https://stackoverflow.com/users/85381/iain>.
char* get_option(char** begin, char** end, const std::string& option)
{
    char** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end) {
        return *itr;
    }
    return nullptr;
}


xmsg::ProxyAddress get_address(int argc, char** argv)
{
    std::string host = xmsg::util::localhost();
    int port = xmsg::constants::default_port;

    char* arg;
    if ((arg = get_option(argv, argv + argc, "--host")) != nullptr) {
        host = xmsg::util::to_host_addr(arg);
    }
    if ((arg = get_option(argv, argv + argc, "--port")) != nullptr) {
        port = std::stoi(arg);
    }

    return {host, port};
}


int main(int argc, char** argv)
{
    try {
        xmsg::ProxyAddress addr = get_address(argc, argv);
        xmsg::sys::Proxy proxy{addr};
        proxy.start();

        printf("[%s] xMsgProxy INFO: running on host = %s  port = %d\n",
               current_time().c_str(), addr.host().c_str(), addr.pub_port());

        wait_signals();
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
