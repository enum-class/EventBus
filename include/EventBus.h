#pragma once

#include <SuperFactory.h>
#include <SuperQueue.h>

template<std::size_t Q_SIZE, std::size_t POOL_SIZE = Q_SIZE,
         std::size_t BLOCK = 64>
class EventBus final {
  superqueue::SuperQueue *queue = nullptr;
  superfactory::SuperFactory<POOL_SIZE, BLOCK> factory;

 public:
  EventBus() : queue(superqueue::create(Q_SIZE)) {}

  template<class TEvent, class... Args>
  inline auto send(Args... args) noexcept -> bool {
    auto *event =
        factory.template create<TEvent>(std::forward<Args>(args)...);
    if (event) { [[likely]]
      return superqueue::enqueue<superqueue::SyncType::MULTI_THREAD,
                                 superqueue::Behavior::FIXED>(queue, event);
}
    return false;
  }

  ~EventBus() {
    superqueue::free(queue);
    queue = nullptr;
  }

  template<class TEvent> inline auto process_next() noexcept -> bool {
    auto *data = superqueue::dequeue<superqueue::SyncType::SINGLE_THREAD,
                                     superqueue::Behavior::FIXED>(queue);

    if (data) [[likely]] {
      auto *ev = reinterpret_cast<TEvent *>(data);
      // TODO(saman): pass process function
      ev->process();
      factory.template recycle<TEvent>(ev);
      return true;
    }

    return false;
  }
};
