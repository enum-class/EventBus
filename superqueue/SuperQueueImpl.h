#pragma once
#include <SuperQueueCore.h>

namespace {
template<superqueue::SyncType sync, superqueue::Behavior behavior>
static inline auto move_consumer_head(superqueue::SuperQueue *tr,
                                              unsigned int n,
                                              uint32_t *old_head,
                                              uint32_t *new_head,
                                              uint32_t *entries) noexcept -> unsigned int {
  unsigned int max = n;
  uint32_t prod_tail = 0;
  int success = 0;

  *old_head = tr->consumer.head.load(std::memory_order_relaxed);
  do {
    n = max;

    std::atomic_thread_fence(std::memory_order_acquire);

    prod_tail = tr->producer.tail.load(std::memory_order_acquire);

    *entries = (prod_tail - *old_head);

    if (n > *entries) [[unlikely]] {
      if constexpr (behavior == superqueue::Behavior::FIXED) {
        return 0;
      } else {
        n = *entries;
        if (n == 0) { [[unlikely]]
          return 0;
}
      }
    }

    *new_head = *old_head + n;
    if constexpr (sync == superqueue::SyncType::SINGLE_THREAD) {
      tr->consumer.head = *new_head;
      return n;
    } else {
      success = atomic_compare_exchange_strong_explicit(
          &tr->consumer.head, old_head, *new_head, std::memory_order_relaxed,
          std::memory_order_relaxed);
    }
  } while (success == 0);
  return n;
}

inline void dequeue_memory(superqueue::SuperQueue *tr,
                                  uint32_t cons_head, void *obj_table,
                                  uint32_t n) noexcept {
  unsigned int i = 0;
  const uint32_t size = tr->size;
  uint32_t idx = cons_head & tr->mask;
  auto *ring = reinterpret_cast<uint64_t *>(&tr[1]);
  auto *obj = static_cast<uint64_t *>(obj_table);
  obj[i] = ring[idx];
}

template<superqueue::SyncType sync, superqueue::Behavior behavior>
static inline auto move_producer_head(superqueue::SuperQueue *tr,
                                              unsigned int n,
                                              uint32_t *old_head,
                                              uint32_t *new_head,
                                              uint32_t *free_entries) noexcept -> unsigned int {
  const uint32_t capacity = tr->capacity;
  uint32_t cons_tail = 0;
  unsigned int max = n;
  int success = 0;

  *old_head = tr->producer.head.load(std::memory_order_relaxed);
  do {
    n = max;
    std::atomic_thread_fence(std::memory_order_acquire);
    cons_tail = tr->consumer.tail.load(std::memory_order_acquire);

    *free_entries = (capacity + cons_tail - *old_head);

    if (n > *free_entries) [[unlikely]] {
      if constexpr (behavior == superqueue::Behavior::FIXED) {
        return 0;
      } else {
        n = *free_entries;
        if (n == 0) {
          return 0;
}
      }
    }

    *new_head = *old_head + n;
    if constexpr (sync == superqueue::SyncType::SINGLE_THREAD) {
      tr->producer.head = *new_head;
      return n;
    } else {
      success = atomic_compare_exchange_strong_explicit(
          &tr->producer.head, old_head, *new_head, std::memory_order_relaxed,
          std::memory_order_relaxed);
    }
  } while (success == 0);
  return n;
}

template<superqueue::SyncType sync>
static inline void update_tail(superqueue::HeadTail *ht, uint32_t old_val,
                               uint32_t new_val) noexcept {
  if constexpr (sync == superqueue::SyncType::MULTI_THREAD) {
    while (ht->tail.load(std::memory_order_relaxed) != old_val) {
      //_mm_pause();
    }
  }

  ht->tail.store(new_val, std::memory_order_release);
}

inline void enqueue_memory(superqueue::SuperQueue *tr,
                                  uint32_t prod_head, const void *obj_table,
                                  uint32_t n) noexcept {
  unsigned int i = 0;
  const uint32_t size = tr->size;
  uint32_t idx = prod_head & tr->mask;
  auto *ring = reinterpret_cast<uint64_t *>(&tr[1]);
  const auto *obj = static_cast<const uint64_t *>(obj_table);

  ring[idx] = obj[i];
}
} // namespace

namespace core {
template<superqueue::SyncType sync, superqueue::Behavior behavior>
static inline auto do_enqueue(superqueue::SuperQueue *tr, const void *obj,
                              unsigned int n,
                              unsigned int *free_space) noexcept -> bool {
  uint32_t prod_head = 0;
  uint32_t prod_next = 0;
  uint32_t free_entries = 0;

  n = move_producer_head<sync, behavior>(tr, n, &prod_head, &prod_next,
                                         &free_entries);
  if (n == 0) {
    *free_space = free_entries - n;
    return false;
  }

  enqueue_memory(tr, prod_head, obj, n);
  update_tail<sync>(&tr->producer, prod_head, prod_next);

  *free_space = free_entries - n;
  return true;
}

template<superqueue::SyncType sync, superqueue::Behavior behavior>
static inline auto do_dequeue(superqueue::SuperQueue *tr, void *obj,
                              unsigned int n,
                              unsigned int *available) noexcept -> bool {
  uint32_t cons_head = 0;
  uint32_t cons_next = 0;
  uint32_t entries = 0;

  n = move_consumer_head<sync, behavior>(tr, n, &cons_head, &cons_next,
                                         &entries);
  if (n == 0) {
    *available = entries - n;
    return false;
  }

  dequeue_memory(tr, cons_head, obj, 1);
  update_tail<sync>(&tr->consumer, cons_head, cons_next);

  *available = entries - n;
  return true;
}

} // namespace core
