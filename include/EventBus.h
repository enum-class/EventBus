#pragma once

#include <SuperFactory.h>
#include <SuperQueue.h>

template<std::size_t Q_SIZE, std::size_t POOL_SIZE = Q_SIZE,
         std::size_t BLOCK = 64>
class EventBus final {
  superfactory::SuperFactory<POOL_SIZE, BLOCK> factory;
  superqueue::SuperQueue *queue = nullptr;

 public:
  EventBus() : queue(superqueue::create(Q_SIZE)) {}

  template<class TEvent, class... Args> bool send(Args... args) noexcept {
    TEvent *event =
        factory.template create<TEvent>(std::forward<Args>(args)...);
    if (event)
      return superqueue::enqueue<superqueue::SyncType::MULTI_THREAD,
                                 superqueue::Behavior::FIXED>(queue, event);
    return false;
  }

  template<class TEvent> bool process_next() noexcept {
    auto *data = superqueue::dequeue<superqueue::SyncType::SINGLE_THREAD,
                                     superqueue::Behavior::FIXED>(queue);

    if (data) {
      TEvent *ev = reinterpret_cast<TEvent *>(data);
      // TODO: pass process function
      ev->process();
      factory.template recycle<TEvent>(ev);
      return true;
    }

    return false;
  }
};
