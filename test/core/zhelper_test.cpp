#include "zhelper.h"

#include <gmock/gmock.h>

#include <limits>
#include <thread>
#include <vector>

using namespace testing;
using namespace xmsg;

TEST(UniqueReplyTo, GenerateUniqueReplyTo)
{
    detail::set_unique_replyto(-1);

    ASSERT_THAT(detail::get_unique_replyto("id"), StrEq("ret:id:2000000"));
    ASSERT_THAT(detail::get_unique_replyto("id"), StrEq("ret:id:2000001"));
    ASSERT_THAT(detail::get_unique_replyto("id"), StrEq("ret:id:2000002"));

    detail::set_unique_replyto(900000);

    auto t1 = std::thread{[]() {
        for (int i = 1; i < 50000; i++) {
            detail::get_unique_replyto("id");
        }
    }};
    auto t2 = std::thread{[]() {
        for (int i = 0; i < 40000; i++) {
            detail::get_unique_replyto("id");
        }
    }};

    t1.join();
    t2.join();

    ASSERT_THAT(detail::get_unique_replyto("id"), StrEq("ret:id:2990000"));

    for (int i = 1; i < 10000; i++) {
        detail::get_unique_replyto("id");
    }

    ASSERT_THAT(detail::get_unique_replyto("id"), StrEq("ret:id:2000000"));
    ASSERT_THAT(detail::get_unique_replyto("id"), StrEq("ret:id:2000001"));
    ASSERT_THAT(detail::get_unique_replyto("id"), StrEq("ret:id:2000002"));
}


TEST(UniqueReplyTo, OverflowUniqueReplyTo)
{
    detail::set_unique_replyto(std::numeric_limits<std::uint_fast32_t>::max());

    ASSERT_THAT(detail::get_unique_replyto("id"), StrEq("ret:id:2000000"));
    ASSERT_THAT(detail::get_unique_replyto("id"), StrEq("ret:id:2000001"));
    ASSERT_THAT(detail::get_unique_replyto("id"), StrEq("ret:id:2000002"));
}


TEST(UniqueReplyTo, EncodeIdentity)
{
    auto encode = detail::encode_identity("10.2.9.1", "test_actor");

    ASSERT_EQ(8u, encode.size());
}


TEST(RandomControlId, NineDigitsLong)
{
    auto id1 = detail::get_random_id();
    auto id2 = detail::get_random_id();
    auto id3 = detail::get_random_id();

    ASSERT_EQ(9u, id1.size());
    ASSERT_EQ(9u, id2.size());
    ASSERT_EQ(9u, id3.size());
}


TEST(RandomControlId, SameThreeDigitsPrefix)
{
    auto pre1 = detail::get_random_id().substr(1, 3);
    auto pre2 = detail::get_random_id().substr(1, 3);
    auto pre3 = detail::get_random_id().substr(1, 3);

    ASSERT_EQ(pre1, pre2);
    ASSERT_EQ(pre2, pre3);
}


TEST(RandomControlId, FirstDigitIsCppIdentifier)
{
    auto id1 = detail::get_random_id();
    auto id2 = detail::get_random_id();
    auto id3 = detail::get_random_id();

    ASSERT_EQ('2', id1[0]);
    ASSERT_EQ('2', id2[0]);
    ASSERT_EQ('2', id3[0]);
}


TEST(RandomControlId, RamdomLastFiveDigits)
{
    auto suff1 = detail::get_random_id().substr(4);
    auto suff2 = detail::get_random_id().substr(4);
    auto suff3 = detail::get_random_id().substr(4);

    ASSERT_NE(suff1, suff2);
    ASSERT_NE(suff1, suff3);
    ASSERT_NE(suff2, suff3);
}


#if THREAD_SANITIZER
TEST(RandomControlId, ThreadSafeGeneration)
{
    std::vector<std::thread> threads;

    for (int i = 0; i < 8; ++i) {
        threads.emplace_back([]() {
            for (int i = 0; i < 25; i++) {
                detail::get_random_id();
            }
        });
    }

    for (auto& t: threads) {
        t.join();
    }
}
#endif


int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
