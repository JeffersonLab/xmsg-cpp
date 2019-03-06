#include <xmsg/util.h>
#include <xmsg/xmsg.h>

#include "helper/proxy_wrapper.h"
#include "helper/utils.h"

#include <gmock/gmock.h>

#include <atomic>

using namespace testing;


TEST(Subscription, UnsubscribeStopsThread)
{
    xmsg::test::ProxyThread proxy_thread{};

    auto actor = xmsg::xMsg{"test"};
    auto con = actor.connect();

    auto cb = [](xmsg::Message&) {};
    auto sub = actor.subscribe(xmsg::Topic::build("test"), std::move(con), cb);
    xmsg::util::sleep(1000);

    actor.unsubscribe(std::move(sub));
}


TEST(Subscription, SuscribeReceivesAllMessages)
{
    struct Check
    {
        std::atomic_int counter{0};
        std::atomic_long sum{0};

        const int N = 10000;
        const long SUM_N = 49995000L;
    } check;

    xmsg::test::ProxyThread proxy_thread;

    SimpleCondition sub_ready;
    SimpleCondition all_msg;

    auto sub_thread = std::thread{[&]() {
        try {
            auto actor = xmsg::xMsg{"test_subscriber"};
            auto connection = actor.connect();

            auto topic = xmsg::Topic::raw("test_topic");
            auto cb = [&](xmsg::Message& msg) {
                auto i = xmsg::parse_message<int>(msg);
                auto n = check.counter.fetch_add(1);
                check.sum.fetch_add(i);
                if (++n == check.N) {
                    all_msg.notify_one();
                }
            };

            auto sub = actor.subscribe(topic, std::move(connection), cb);
            sub_ready.notify_one();

            all_msg.wait_for(4000);
            actor.unsubscribe(std::move(sub));
        } catch (std::exception& e) {
            std::cerr << "Subscriber error: " << e.what() << std::endl;
        }
    }};

    auto pub_thread = std::thread{[&]() {
        try {
            sub_ready.wait_for(1000);
            auto actor = xmsg::xMsg{"test_publisher"};
            auto connection = actor.connect();
            auto topic = xmsg::Topic::raw("test_topic");
            for (int i = 0; i < check.N; i++) {
                auto msg = xmsg::make_message(topic, i);
                actor.publish(connection, msg);
            }
        } catch (std::exception& e) {
            std::cerr << "Publisher error: " << e.what() << std::endl;
        }
    }};

    pub_thread.join();
    sub_thread.join();

    ASSERT_THAT(check.counter.load(), Eq(check.N));
    ASSERT_THAT(check.sum.load(), Eq(check.SUM_N));
}


TEST(Subscription, SyncPublishReceivesAllResponses)
{
    struct Check
    {
        std::atomic_int counter{0};
        std::atomic_long sum{0};

        const int N = 100;
        const long SUM_N = 4950L;
    } check;

    xmsg::test::ProxyThread proxy_thread;

    SimpleCondition sub_ready;
    SimpleCondition all_msg;

    auto sub_thread = std::thread{[&]() {
        try {
            auto sub_actor = xmsg::xMsg{"test_subscriber"};
            auto sub_con = sub_actor.connect();
            auto rep_con = sub_actor.connect();

            auto sub_topic = xmsg::Topic::raw("test_topic");
            auto sub_cb = [&](xmsg::Message& m) {
                auto r_topic = m.meta()->replyto();
                auto r_data = xmsg::parse_message<int>(m);
                auto r_msg = xmsg::make_message(xmsg::Topic::raw(r_topic), r_data);
                sub_actor.publish(rep_con, r_msg);
            };

            auto sub = sub_actor.subscribe(sub_topic, std::move(sub_con), sub_cb);
            sub_ready.notify_one();

            all_msg.wait_for(8000);
            sub_actor.unsubscribe(std::move(sub));
        } catch (std::exception& e) {
            std::cerr << "Subscriber error: " << e.what() << std::endl;
        }
    }};

    auto pub_thread = std::thread{[&]() {
        try {
            sub_ready.wait_for(1000);
            auto pub_actor = xmsg::xMsg{"test_publisher"};
            auto pub_con = pub_actor.connect();
            auto pub_topic = xmsg::Topic::raw("test_topic");
            for (int i = 0; i < check.N; i++) {
                auto msg = xmsg::make_message(pub_topic, i);
                auto r_msg = pub_actor.sync_publish(pub_con, msg, 1000);
                auto r_data = xmsg::parse_message<int>(r_msg);
                check.counter.fetch_add(1);
                check.sum.fetch_add(r_data);
            }
        } catch (std::exception& e) {
            std::cerr << "Publisher error: " << e.what() << std::endl;
        }
    }};

    pub_thread.join();
    all_msg.notify_one();
    sub_thread.join();

    ASSERT_THAT(check.counter.load(), Eq(check.N));
    ASSERT_THAT(check.sum.load(), Eq(check.SUM_N));
}



int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
