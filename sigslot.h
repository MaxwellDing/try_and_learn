// Connect.hpp
#ifndef CONNECT_H_
#define CONNECT_H_

#include <functional>
#include <memory>
#include <sstream>
#include <vector>

#include "thread_pool.h"

#define emit
#define signals public
#define connect(sender, signal, slot) ((sender)->signal.Bind(slot))

enum class SignalPolicy {
  SYNC,
  ASYNC
};

template <typename Derived>
class SlotBase {
 public:
  template <typename... RArgs>
  void Run(RArgs&&... args) {
    static_cast<Derived*>(this)->Exec(std::forward<RArgs>(args)...);
  }
  virtual ~SlotBase() = default;
};

template<SignalPolicy policy, typename... Args>
class Slot : public SlotBase<Slot<policy, Args...>> {
 public:
  using OnFunc = std::function<void(Args...)>;
  Slot(OnFunc&& func) noexcept : func_(std::forward<OnFunc>(func)) {}

  template <typename... RArgs, typename = std::result_of<OnFunc(RArgs...)>>
  void Exec(RArgs&&... args) {
    func_(std::forward<RArgs>(args)...);
  }

 private:
  OnFunc func_;
};

template<typename... Args>
class Slot<SignalPolicy::ASYNC, Args...> : public SlotBase<Slot<SignalPolicy::ASYNC, Args...>> {
 public:
  using OnFunc = std::function<void(Args...)>;
  Slot(const OnFunc& func) noexcept : func_(func) { tp_.Resize(1); }

  template <typename... RArgs, typename = std::result_of<OnFunc(RArgs...)>>
  void Exec(RArgs&&... args) {
    tp_.VoidPush(0, func_, std::forward<RArgs>(args)...);
  }

 private:
  EqualityThreadPool tp_;
  OnFunc func_;
};

template<SignalPolicy policy, typename... Args>
class Signal {
 public:
  using SlotPtr = std::shared_ptr<SlotBase<Slot<policy, Args...>>>;
  using OnFunc = std::function<void(Args...)>;

  void Bind(OnFunc&& func) {
    slots_.push_back(SlotPtr(new Slot<policy, Args...>(std::forward<OnFunc>(func))));
  }

  template <typename... RArgs, typename = std::result_of<OnFunc(RArgs...)>>
  void operator()(RArgs&&... args) {
    for (auto& iter : slots_) {
      iter->Run(std::forward<RArgs>(args)...);
    }
  }

 private:
  std::vector<SlotPtr> slots_;
};

template<typename... Args>
using SyncSignal = Signal<SignalPolicy::SYNC, Args...>;

template<typename... Args>
using AsyncSignal = Signal<SignalPolicy::ASYNC, Args...>;

#endif

