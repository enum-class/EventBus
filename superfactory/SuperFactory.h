#pragma once

#include <SuperQueue.h>

namespace {

struct SuperMemPool {
  uint8_t *data = nullptr;
  superqueue::SuperQueue *pool = nullptr;
};

inline auto create_pool(std::size_t size, std::size_t block) -> SuperMemPool * {
  auto *mempool = new SuperMemPool;
  mempool->pool = superqueue::create(size);

  mempool->data = new uint8_t[size * block];

  for (int i = 0; i < size; i++) {
    if (!superqueue::enqueue<superqueue::SyncType::SINGLE_THREAD,
                             superqueue::Behavior::FIXED>(
            mempool->pool, &mempool->data[i * block])) {
      throw;
}
  }

  return mempool;
}

inline void free_pool(SuperMemPool *pool) {
  superqueue::free(pool->pool);
  delete[] pool->data;
  pool->data = nullptr;
  pool->pool = nullptr;
}

inline auto acquire(SuperMemPool *mempool) noexcept -> uint8_t * {
  return static_cast<uint8_t *>(superqueue::dequeue<superqueue::SyncType::MULTI_THREAD,
                                        superqueue::Behavior::FIXED>(
      mempool->pool, 1));
}

inline void release(SuperMemPool *mempool, uint8_t *buffer) noexcept {
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
  inline auto create(Args... args) noexcept -> TEvent * {
    uint8_t *buf = acquire(mempool);
    if (buf) { [[likely]]
      return new (buf) TEvent(std::forward<Args>(args)...);
}

    return nullptr;
  }

  template<class TEvent> inline void recycle(TEvent *event) noexcept {
    event->~TEvent();
    release(mempool, reinterpret_cast<uint8_t *>(event));
  }
};

}  // namespace superfactory
