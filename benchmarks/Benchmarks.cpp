#include <EventBus.h>
#include <IEvent.h>
#include <pthread.h>

#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

typedef std::chrono::high_resolution_clock Clock;

std::string results;

int set_thread_core(int core_id) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);

  pthread_t current_thread = pthread_self();
  return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

class Ev1 : public IEvent {
public:
    int val1;
    double val2;
    float val3;

    Ev1(auto v1, auto v2, auto v3)
    : val1(v1)
    , val2(v2)
    , val3(v3)
    {
    }
    
    ~Ev1() {}
    virtual void process() const noexcept override {
      results = std::to_string(val1 * val2 * val3);
    }
};

class Ev2 : public IEvent {
public:
    std::string val1;
    double val2;

    Ev2(auto v1, auto v2)
    : val1(v1)
    , val2(v2)
    {
    }

    ~Ev2() {}

    virtual void process() const noexcept override {
      results = std::to_string(val1.size() * val2);
    }
};

class Ev3 : public IEvent {
public:
    uint64_t val1;

    Ev3(auto v1)
    : val1(v1)
    {
    }

    ~Ev3() {}

    virtual void process() const noexcept override {
      results = std::to_string(val1 * val1);
    }
};

auto benchmark_run(auto &bus, std::size_t number_of_events_per_producer,
                   std::size_t number_of_producers) {
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<std::mt19937::result_type> dist(1, 100);

  std::vector<std::jthread> producers;
  auto start_time = Clock::now();

  for (int i = 0; i < number_of_producers; i++) {
    producers.push_back(std::jthread(
        [&](int id) {
          set_thread_core(id + 1);
          for (int i = 0; i < number_of_events_per_producer; i++) {
            switch (dist(rng) % 3) {
              case 0:
                while (!bus.template send<Ev1>(i, dist(rng),
                                               dist(rng) / (i + 1))) {}
                break;
              case 1:
                while (!bus.template send<Ev2>(std::to_string(i),
                                               dist(rng) / (i + 1))) {}
                break;
              case 2:
                while (!bus.template send<Ev3>(dist(rng) / (i + 1))) {}
                break;
            }
          }
        },
        i));
  }

  int counter = 0;
  while (counter < number_of_events_per_producer * number_of_producers) {
    if (bus.template process_next<IEvent>()) { counter++; }
  }

  auto end_time = Clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                               start_time)
      .count();
}

int main() {
  int repetition = 3;
  std::size_t event_count = 100000000;
  set_thread_core(0);
  EventBus<1000> bus;

  int thread_1_t = 0;
  int thread_2_t = 0;
  int thread_4_t = 0;
  int thread_8_t = 0;

  for (int i = 0; i < repetition; i++) {
    thread_1_t += benchmark_run(bus, event_count, 1);
    thread_2_t += benchmark_run(bus, event_count, 2);
    thread_4_t += benchmark_run(bus, event_count, 4);
    thread_8_t += benchmark_run(bus, event_count, 8);
  }

  std::cout << "| Queue size 1000 with 64 byte blocks                      |\n";
  std::cout << "|--------------------------------------|-------------------|\n";
  std::cout << "|     Million  operation per second    | Number of threads |\n";
  std::cout << "| " << (event_count * 1 * 2) / (thread_1_t / repetition)
            << "                                  | 1                 |\n";
  std::cout << "| " << (event_count * 2 * 2) / (thread_2_t / repetition)
            << "                                   | 2                 |\n";
  std::cout << "| " << (event_count * 4 * 2) / (thread_4_t / repetition)
            << "                                   | 4                 |\n";
  std::cout << "| " << (event_count * 8 * 2) / (thread_8_t / repetition)
            << "                                   | 8                 |\n";
}
