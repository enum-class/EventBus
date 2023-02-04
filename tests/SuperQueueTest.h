#include <gtest/gtest.h>
#include <SuperQueue.h>

static inline bool super_queue_logic_test() {
    superqueue::SuperQueue* tr = superqueue::create(5);
    EXPECT_EQ(superqueue::is_empty(tr), true);
    EXPECT_EQ(superqueue::count_entries(tr), 0);
    EXPECT_EQ(superqueue::count_free_entries(tr), 5);

    uint64_t data = 419;
    auto res = superqueue::enqueue<superqueue::SyncType::SINGLE_THREAD, superqueue::Behavior::VARIABLE>(tr,
                    reinterpret_cast<void*>(&data));

    EXPECT_EQ(res, true);
    EXPECT_EQ(superqueue::is_empty(tr), false);
    EXPECT_EQ(superqueue::count_entries(tr), 1);
    EXPECT_EQ(superqueue::count_free_entries(tr), 4);

    uint64_t data_2 = 813;
    res = superqueue::enqueue<superqueue::SyncType::SINGLE_THREAD, superqueue::Behavior::VARIABLE>(tr,
                    reinterpret_cast<void*>(&data_2));
    void* read_data = superqueue::dequeue<superqueue::SyncType::SINGLE_THREAD, superqueue::Behavior::VARIABLE>(tr);

    EXPECT_EQ(*(uint64_t*)read_data, data);
    
    read_data = superqueue::dequeue<superqueue::SyncType::SINGLE_THREAD, superqueue::Behavior::VARIABLE>(tr);

    EXPECT_EQ(*(uint64_t*)read_data, data_2);

    superqueue::free(tr);

    return true;
}
