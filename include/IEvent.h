#pragma once

class IEvent {
 public:
  virtual ~IEvent() = default;
  virtual void process() = 0;
};
