#pragma once
#include <SuperQueueCore.h>
#include <SuperQueueImpl.h>

#include <cstddef>
#include <cstring>

namespace {
auto calculate_memsize_element(unsigned int size,
                                      unsigned int count) noexcept -> std::size_t {
  std::size_t sz = sizeof(superqueue::SuperQueue) + count * size;
  sz = ALIGN(sz, superqueue::CACHE_LINE_SIZE);
  return sz;
}

auto combine32ms1b(uint32_t x) noexcept -> uint32_t {
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;

  return x;
}

auto align32pow2(uint32_t x) noexcept -> uint32_t {
  x--;
  x = combine32ms1b(x);

  return x + 1;
}
} // namespace

namespace superqueue {
static inline auto create(std::size_t size) noexcept -> SuperQueue * {
  superqueue::SuperQueue *tr = nullptr;
  const unsigned int req_count = size;

  size = align32pow2(size + 1);
  std::size_t real_size = calculate_memsize_element(sizeof(uint8_t), size);
  if (real_size == 0) {
    return nullptr;
}

  auto *mz = new uint64_t[real_size];
  tr = reinterpret_cast<superqueue::SuperQueue *>(mz);
  std::memset(tr, 0, sizeof(*tr));
  tr->size = size;
  tr->mask = tr->size - 1;
  tr->capacity = req_count;
  tr->memzone = mz;

  return tr;
}

static inline void free(SuperQueue *tr) noexcept { delete[] tr->memzone; }

static inline void reset(SuperQueue *tr) noexcept {
  tr->consumer.head = 0;
  tr->consumer.tail = 0;
  tr->producer.head = 0;
  tr->producer.tail = 0;
}

template <SyncType sync, Behavior behavior>
static inline auto enqueue(SuperQueue *tr, void *obj,
                           unsigned int n = 1) noexcept -> bool {
  unsigned int free_space = 0;
  return core::do_enqueue<sync, behavior>(tr, &obj, n, &free_space);
}

template <SyncType sync, Behavior behavior>
static inline auto dequeue(SuperQueue *tr, unsigned int n = 1) noexcept -> void * {
  unsigned int free_space = 0;
  void *obj = nullptr;
  auto ret = core::do_dequeue<sync, behavior>(tr, &obj, n, &free_space);
  return obj;
}

static inline auto count_entries(const SuperQueue *tr) noexcept -> unsigned int {
  uint32_t prod_tail = tr->producer.tail;
  uint32_t cons_tail = tr->consumer.tail;
  uint32_t count = (prod_tail - cons_tail) & tr->mask;
  return (count > tr->capacity) ? tr->capacity : count;
}

static inline auto count_free_entries(const SuperQueue *tr) noexcept -> std::size_t {
  return tr->capacity - superqueue::count_entries(tr);
}

static inline auto is_full(const SuperQueue *tr) noexcept -> bool {
  return superqueue::count_free_entries(tr) == 0;
}

static inline auto is_empty(const SuperQueue *tr) noexcept -> bool {
  uint32_t prod_tail = tr->producer.tail;
  uint32_t cons_tail = tr->consumer.tail;
  return cons_tail == prod_tail;
}
} // namespace superqueue
