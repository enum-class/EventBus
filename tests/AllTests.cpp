#include <gtest/gtest.h>

#include "SuperFactoryTest.h"
#include "SuperQueueTest.h"

TEST(SuperQueueTest, logic) { super_queue_logic_test(); }
TEST(SuperFactoryTest, logic) { super_factory_logic_test(); }
