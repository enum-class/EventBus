#include <EventBus.h>
#include <IEvent.h>

#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include <iostream>

typedef std::chrono::high_resolution_clock Clock;

std::vector<std::string> results;

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
        results.push_back(std::to_string(val1 * val2 * val3));
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
        results.push_back(std::to_string(val1.size() * val2));
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
        results.push_back(std::to_string(val1 * val1));
    }
};

auto benchmark_run(auto& bus, std::size_t number_of_events_per_producer,
        std::size_t number_of_producers)
{
    results.clear();
    results.reserve(number_of_events_per_producer * number_of_producers);

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1,100);

    std::vector<std::jthread> producers;
    auto start_time = Clock::now();
    
    for (int i = 0; i < number_of_producers; i++) {
        producers.push_back(std::jthread([&]() {
            for (int i = 0; i < number_of_events_per_producer; i++) {
                switch(dist(rng) % 3){
                    case 0:
                        while (!bus.template send<Ev1>(i, dist(rng), dist(rng) / (i + 1))) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(5));
                        }
                        break;
                    case 1:
                        while (!bus.template send<Ev2>(std::to_string(i), dist(rng) / (i + 1))) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(5));
                        }
                        break;
                    case 2:
                        while (!bus.template send<Ev3>(dist(rng) / (i + 1))) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(5));
                        }
                        break;
                }
            }
        }));
    }

    int counter = 0;
    while (counter < number_of_events_per_producer * number_of_producers) {
        if (bus.template process_next<IEvent>()) {
            counter++;
        }
    }

    auto end_time = Clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
}

int main() {
    EventBus<1000> bus;
    auto time1 = benchmark_run(bus, 1000000, 1);
    auto time2 = benchmark_run(bus, 1000000 / 2, 2);
    auto time3 = benchmark_run(bus, 1000000 / 4, 4);

    std::cout << time1 << "\t" << time2 << "\t" << time3 << "\n";
}
