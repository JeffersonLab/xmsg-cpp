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

#include "xmsg.h"

#include "xmsg_connection_setup.h"
#include "xmsg_connection_pool.h"
#include "xmsg_regdis.h"
#include "xmsg_util.h"

#include <random>

namespace {

std::random_device rd;
std::mt19937_64 rng{rd()};
std::uniform_int_distribution<int> gen{1, 100};

}

namespace xmsg {

struct xMsg::Impl {
    Impl(const std::string& name,
         const ProxyAddress& proxy_addr,
         const RegAddress& reg_addr,
         std::shared_ptr<ConnectionPool> con_pool)
      : name{name},
        default_proxy_addr{proxy_addr},
        default_reg_addr{reg_addr},
        con_pool{std::move(con_pool)}
    { }

    std::string name;
    ProxyAddress default_proxy_addr;
    RegAddress default_reg_addr;
    std::shared_ptr<ConnectionPool> con_pool;
};


xMsg::xMsg(const std::string& name)
  : xMsg{name, {}, {}}
{ }


xMsg::xMsg(const std::string& name,
           const RegAddress& default_registrar)
  : xMsg{name, {}, default_registrar}
{ }


xMsg::xMsg(const std::string& name,
           const ProxyAddress& default_proxy,
           const RegAddress& default_registrar)
  : xMsg{name, default_proxy, default_registrar,
         std::make_shared<ConnectionPool>()}
{ }

xMsg::xMsg(const std::string& name,
           const ProxyAddress& default_proxy,
           const RegAddress& default_registrar,
           std::shared_ptr<ConnectionPool> connection_pool)
  : xmsg_{new Impl{name, default_proxy, default_registrar,
                   std::move(connection_pool)}}
{ }


xMsg::xMsg(xMsg &&) = default;
xMsg& xMsg::operator=(xMsg &&) = default;

xMsg::~xMsg() = default;


std::unique_ptr<Connection> xMsg::connect()
{
    return xmsg_->con_pool->get_connection(xmsg_->default_proxy_addr);
}


std::unique_ptr<Connection> xMsg::connect(const ProxyAddress& addr)
{
    return xmsg_->con_pool->get_connection(addr);
}


void xMsg::set_connection_setup(std::unique_ptr<ConnectionSetup> setup)
{
    return xmsg_->con_pool->set_default_setup(std::move(setup));
}


void xMsg::release(std::unique_ptr<Connection>&& connection)
{
    xmsg_->con_pool->release_connection(std::move(connection));
}


void xMsg::publish(std::unique_ptr<Connection>& connection, Message& msg)
{
    connection->send(msg);
}


Message xMsg::sync_publish(std::unique_ptr<Connection>& connection,
                           Message& msg,
                           int timeout)
{
    using ConnectionView = Subscription::ConnectionWrapperPtr;

    auto return_addr = "return:" + std::to_string(gen(rng));
    msg.meta_->set_replyto(return_addr);

    std::atomic_bool response{false};
    auto rmsg = Message{Topic::raw(""), "", std::vector<std::uint8_t>{}};

    auto cb = [&](Message& m) {
        rmsg = std::move(m);
        response.store(true);
    };
    auto ptr = ConnectionView(connection.get(), [](Connection* c){});

    auto sub = std::unique_ptr<Subscription>{
            new Subscription{Topic::raw(return_addr), std::move(ptr), cb}
    };
    connection->send(msg);

    auto t = 0;
    while (t <= timeout && !response.load()) {
        ++t;
        util::sleep(1);
    }
    if (t >= timeout) {
        throw std::runtime_error("xMsg-Error: no response for time_out = " + std::to_string(timeout) + " milli sec.");
    }
    return rmsg;
}


std::unique_ptr<Subscription>
xMsg::subscribe(const Topic& topic,
                std::unique_ptr<Connection> connection,
                std::function<void(Message&)> callback)
{
    return std::unique_ptr<Subscription>{
            new Subscription{topic, std::move(connection), std::move(callback)}
    };
}


void xMsg::unsubscribe(std::unique_ptr<Subscription> handler)
{
    handler->stop();
}


void xMsg::register_as_publisher(const Topic& topic,
                                 const std::string& description)
{
    register_as_publisher(xmsg_->default_reg_addr, topic, description);
}


void xMsg::register_as_publisher(const RegAddress& addr,
                                 const Topic& topic,
                                 const std::string& description)
{
    auto driver = xmsg_->con_pool->get_connection(addr);
    auto proxy = xmsg_->default_proxy_addr;
    auto data = registration::create(xmsg_->name, description,
                                     proxy.host, proxy.pub_port,
                                     topic, true);
    data.set_description(description);

    driver->add(data, true);
}


void xMsg::register_as_subscriber(const Topic& topic,
                                  const std::string& description)
{
    register_as_subscriber(xmsg_->default_reg_addr, topic, description);
}


void xMsg::register_as_subscriber(const RegAddress& addr,
                                  const Topic& topic,
                                  const std::string& description)
{
    auto driver = xmsg_->con_pool->get_connection(addr);
    auto proxy = xmsg_->default_proxy_addr;
    auto data = registration::create(xmsg_->name, description,
                                     proxy.host, proxy.sub_port,
                                     topic, false);
    data.set_description(description);

    driver->add(data, false);
}


void xMsg::remove_as_publisher(const Topic& topic)
{
    remove_as_publisher(xmsg_->default_reg_addr, topic);
}


void xMsg::remove_as_publisher(const RegAddress& addr, const Topic& topic)
{
    auto driver = xmsg_->con_pool->get_connection(addr);
    auto proxy = xmsg_->default_proxy_addr;
    auto data = registration::create(xmsg_->name, "",
                                     proxy.host, proxy.pub_port,
                                     topic, true);
    driver->remove(data, true);
}



void xMsg::remove_as_subscriber(const Topic& topic)
{
    remove_as_subscriber(xmsg_->default_reg_addr, topic);
}


void xMsg::remove_as_subscriber(const RegAddress& addr, const Topic& topic)
{
    auto driver = xmsg_->con_pool->get_connection(addr);
    auto proxy = xmsg_->default_proxy_addr;
    auto data = registration::create(xmsg_->name, "",
                                     proxy.host, proxy.sub_port,
                                     topic, false);
    driver->remove(data, false);
}


RegDataSet xMsg::find_publishers(const Topic& topic)
{
    return find_publishers(xmsg_->default_reg_addr, topic);
}


RegDataSet xMsg::find_publishers(const RegAddress& addr, const Topic& topic)
{
    auto driver = xmsg_->con_pool->get_connection(addr);
    auto proxy = xmsg_->default_proxy_addr;
    auto data = registration::create(xmsg_->name, "",
                                     proxy.host, proxy.pub_port,
                                     topic, true);
    return driver->find(data, true);
}


RegDataSet xMsg::find_subscribers(const Topic& topic)
{
    return find_subscribers(xmsg_->default_reg_addr, topic);
}


RegDataSet xMsg::find_subscribers(const RegAddress& addr, const Topic& topic)
{
    auto driver = xmsg_->con_pool->get_connection(addr);
    auto proxy = xmsg_->default_proxy_addr;
    auto data = registration::create(xmsg_->name, "",
                                     proxy.host, proxy.sub_port,
                                     topic, false);
    return driver->find(data, false);
}


const std::string& xMsg::name()
{
    return xmsg_->name;
}


const RegAddress& xMsg::default_registrar()
{
    return xmsg_->default_reg_addr;
}


const ProxyAddress& xMsg::default_proxy()
{
    return xmsg_->default_proxy_addr;
}

} // end namespace xmsg
