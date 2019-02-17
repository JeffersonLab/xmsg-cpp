#ifndef XMSG_TEST_HELPER_UTILS_H_
#define XMSG_TEST_HELPER_UTILS_H_

namespace testing {

#define EXPECT_EXCEPTION(statement, expected_exception, expected_message)      \
    ASSERT_THROW(statement, expected_exception);                               \
    try {                                                                      \
        statement;                                                             \
    } catch (const expected_exception& e) {                                    \
        ASSERT_THAT(std::string{e.what()}, StrEq(expected_message));           \
    }

}

#endif
