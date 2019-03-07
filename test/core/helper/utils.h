#ifndef XMSG_TEST_HELPER_UTILS_H_
#define XMSG_TEST_HELPER_UTILS_H_

#include <condition_variable>
#include <chrono>
#include <mutex>

namespace testing {

class SimpleCondition
{
public:
    void notify_one()
    {
        std::unique_lock<std::mutex> lk{mtx_};
        ready_ = true;
        cond_.notify_one();
    }

    void notify_all()
    {
        std::unique_lock<std::mutex> lk{mtx_};
        ready_ = true;
        cond_.notify_all();
    }

    void wait()
    {
        std::unique_lock<std::mutex> lk{mtx_};
        cond_.wait(lk, [&]{ return ready_; });
    }

    void wait_for(int timeout)
    {
        std::chrono::milliseconds ms{timeout};
        std::unique_lock<std::mutex> lk{mtx_};
        cond_.wait_for(lk, ms, [&]{ return ready_; });
    }

private:
    std::mutex mtx_;
    std::condition_variable cond_;
    bool ready_ = false;
};


#define EXPECT_EXCEPTION(statement, expected_exception, expected_message)      \
    ASSERT_THROW(statement, expected_exception);                               \
    try {                                                                      \
        statement;                                                             \
    } catch (const expected_exception& e) {                                    \
        ASSERT_THAT(std::string{e.what()}, StrEq(expected_message));           \
    }

}

#endif
