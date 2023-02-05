#pragma once

class IEvent {
 public:
  virtual ~IEvent() = default;
  virtual void process() const noexcept = 0;
};
