#include <EventBus.h>
#include <IEvent.h>
#include <gtest/gtest.h>

#include <thread>
#include <vector>

std::vector<int> result;

class Event1 : public IEvent {
 public:
  int16_t id;
  uint16_t value;

  Event1(auto id_, auto value_) : id(id_), value(value_) {}

  ~Event1() {}

  virtual void process() override { result.push_back(id * value); }
};

class Event2 : public IEvent {
 public:
  int id;
  int value;

  Event2(auto id_, auto value_) : id(id_), value(value_) {}

  ~Event2() {}

  virtual void process() override { result.push_back(id + value); }
};

static inline bool event_bus_logic_test() {
  EventBus<2> bus;

  EXPECT_TRUE(bus.send<Event1>(1, 2));
  EXPECT_TRUE(bus.send<Event2>(333, 444));
  EXPECT_FALSE(bus.send<Event2>(444, 666));

  bus.process_next<IEvent>();
  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result.at(0), 1 * 2);

  EXPECT_TRUE(bus.send<Event2>(444, 666));

  bus.process_next<IEvent>();
  bus.process_next<IEvent>();

  EXPECT_EQ(result.size(), 3);
  EXPECT_EQ(result.at(1), 333 + 444);
  EXPECT_EQ(result.at(2), 444 + 666);

  return true;
}

static inline bool event_bus_thread_test(std::size_t producers_count) {
  result.clear();
  static constexpr size_t GEN_SIZE = 1000000;
  EventBus<150> bus;

  std::vector<std::jthread> producers;
  for (int i = 0; i < producers_count; i++) {
    producers.push_back(std::jthread([&]() {
      for (int i = 0; i < GEN_SIZE; i++) {
        while (!bus.send<Event1>(i, i * i)) {
          std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
      }
    }));
  }

  int counter = 0;
  while (counter < GEN_SIZE * producers_count) {
    if (bus.process_next<IEvent>()) counter++;
  }

  EXPECT_EQ(result.size(), GEN_SIZE * producers_count);
  return true;
}
