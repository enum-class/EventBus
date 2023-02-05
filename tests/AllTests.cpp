#include <gtest/gtest.h>

#include "EventBusTest.h"
#include "SuperFactoryTest.h"
#include "SuperQueueTest.h"

TEST(SuperQueueTest, logic) { super_queue_logic_test(); }
TEST(SuperFactoryTest, logic) { super_factory_logic_test(); }
TEST(EventBusTest, logic) { event_bus_logic_test(); }
TEST(EventBusTest, spsc) { event_bus_thread_test(1); }
TEST(EventBusTest, mpsc) { event_bus_thread_test(2); }
TEST(EventBusTest, highly_concurrent_mpsc) { event_bus_thread_test(4); }
