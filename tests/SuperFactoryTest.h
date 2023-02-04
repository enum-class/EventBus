#include <gtest/gtest.h>
#include <SuperFactory.h>


static inline bool super_factory_logic_test() {
    superfactory::SuperFactory<2, 32> factory;
    
    class Event
    {
     public:
        int8_t data;
        int8_t value;

        Event(int8_t d, int8_t v)
        : data(d)
        , value(v)
        {
        }
    };

    Event* ev1 = factory.create<Event>(1 , 2);
    EXPECT_NE(ev1, nullptr);
    EXPECT_EQ(ev1->data, 1);
    EXPECT_EQ(ev1->value, 2);

    Event* ev2 = factory.create<Event>(3 , 4);
    EXPECT_NE(ev2, nullptr);
    EXPECT_EQ(ev2->data, 3);
    EXPECT_EQ(ev2->value, 4);

    Event* ev3 = factory.create<Event>(5 , 6);
    EXPECT_EQ(ev3, nullptr);
    
    factory.recycle<Event>(ev1);

    ev3 = factory.create<Event>(5 , 6);
    EXPECT_NE(ev3, nullptr);
    EXPECT_EQ(ev3->data, 5);
    EXPECT_EQ(ev3->value, 6);

    return true;
}
