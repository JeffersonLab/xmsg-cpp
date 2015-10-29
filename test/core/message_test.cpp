/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim: set ts=8 sts=4 et sw=4 tw=80: */

#include "message.h"

#include "helper/utils.h"
#include "gmock/gmock.h"

using namespace testing;

namespace {

const xmsg::Topic topic = xmsg::Topic::raw("test/topic");

}


TEST(Message, CreateWithBytes)
{
    auto data = std::vector<std::uint8_t>{ 0x0, 0x1, 0x2, 0x3, 0xa, 0xb };
    auto msg = xmsg::Message{topic, "test/binary", data};

    EXPECT_THAT(msg.data(), ContainerEq(data));
    EXPECT_THAT(msg.meta()->datatype(), StrEq("test/binary"));
}


TEST(Message, PassingNullMetadataThrows)
{
    auto data = std::vector<std::uint8_t>{ 0x0, 0x1, 0x2, 0x3, 0xa, 0xb };
    auto meta = std::unique_ptr<xmsg::proto::Meta>{};

    EXPECT_EXCEPTION(xmsg::Message(topic, std::move(meta), data),
                     std::invalid_argument,  "null metadata");
}


TEST(Message, PassingNullMimeTypeThrows)
{
    auto data = std::vector<std::uint8_t>{ 0x0, 0x1, 0x2, 0x3, 0xa, 0xb };
    EXPECT_EXCEPTION(xmsg::Message(topic, nullptr, data),
                     std::invalid_argument,  "null mime-type");
}


TEST(Message, EqualMessages)
{
    auto data = std::vector<std::uint8_t>{ 0x0, 0x1, 0x2, 0x3, 0xa, 0xb };

    auto msg1 = xmsg::Message{topic, "test/binary", data};
    auto msg2 = xmsg::Message{topic, "test/binary", data};

    ASSERT_TRUE(msg1 == msg2);
}


int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
