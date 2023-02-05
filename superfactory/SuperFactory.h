#pragma once

#include <SuperQueue.h>

namespace {

struct SuperMemPool {
  uint8_t *data = nullptr;
  superqueue::SuperQueue *pool = nullptr;
};

static inline SuperMemPool *create_pool(std::size_t size, std::size_t block) {
  SuperMemPool *mempool = new SuperMemPool;
  mempool->pool = superqueue::create(size);

  mempool->data = new uint8_t[size * block];

  for (int i = 0; i < size; i++) {
    if (!superqueue::enqueue<superqueue::SyncType::SINGLE_THREAD,
                             superqueue::Behavior::FIXED>(
            mempool->pool, &mempool->data[i * block]))
      throw;
  }

  return mempool;
}

static inline void free_pool(SuperMemPool *pool) {
  superqueue::free(pool->pool);
  delete[] pool->data;
  pool->data = nullptr;
  pool->pool = nullptr;
}

static inline uint8_t *acquire(SuperMemPool *mempool) noexcept {
  return (uint8_t *)superqueue::dequeue<superqueue::SyncType::MULTI_THREAD,
                                        superqueue::Behavior::FIXED>(
      mempool->pool, 1);
}

static inline void release(SuperMemPool *mempool, uint8_t *buffer) noexcept {
  superqueue::enqueue<superqueue::SyncType::SINGLE_THREAD,
                      superqueue::Behavior::FIXED>(mempool->pool, buffer);
}

}  // namespace

namespace superfactory {

template<std::size_t SIZE, std::size_t BLOCK> class SuperFactory final {
  SuperMemPool *mempool = nullptr;

 public:
  SuperFactory() : mempool(create_pool(SIZE, BLOCK)) {}
  ~SuperFactory() { free_pool(mempool); delete mempool;}
  template<class TEvent, class... Args>
  inline TEvent *create(Args... args) noexcept {
    uint8_t *buf = acquire(mempool);
    if (buf) [[likely]]
      return new (buf) TEvent(std::forward<Args>(args)...);

    return nullptr;
  }

  template<class TEvent> inline void recycle(TEvent *event) noexcept {
    event->~TEvent();
    release(mempool, reinterpret_cast<uint8_t *>(event));
  }
};

}  // namespace superfactory
