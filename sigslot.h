// Connect.hpp
#ifndef CONNECT_H_
#define CONNECT_H_

#include <functional>
#include <memory>
#include <sstream>
#include <vector>

#include "thread_pool.h"

#define STRINGIFY(obj) #obj
#define emit
#define slots
#define signals public
#define connect_sync(sender, signal, slot) ((sender)->signal.Bind<SignalPolicy::SYNC>(slot))
#define connect_async(sender, signal, slot) ((sender)->signal.Bind<SignalPolicy::ASYNC>(slot))

inline std::string Stringify(void* ptr) {
  thread_local std::ostringstream ss;
  ss << ptr;
  return ss.str();
}

enum class SignalPolicy {
  SYNC,
  ASYNC
};

template <typename... Args>
class SlotBase {
 public:
  virtual void Exec(Args&&... args) = 0;
  virtual ~SlotBase() {}
};

template<SignalPolicy policy, typename... Args>
class Slot : public SlotBase<Args...> {
 public:
  using OnFunc = std::function<void(Args...)>;

  Slot(const OnFunc& func) : func_(func) {
      // Do nothing
  }

  void Exec(Args&&... args) override;

 private:
  OnFunc func_;
};

template<SignalPolicy policy, typename... Args>
void Slot<policy, Args...>::Exec(Args&&... args) {
  func_(std::forward<Args>(args)...);
}

template<typename... Args>
class Slot<SignalPolicy::ASYNC, Args...> : public SlotBase<Args...> {
 public:
  using OnFunc = std::function<void(Args...)>;
  Slot(const OnFunc& func) : func_(func) {
    tp_.Resize(1);
  }

  void Exec(Args&&... args) override;
 
 private:
  EqualityThreadPool tp_;
  OnFunc func_;
};

template<typename... Args>
void Slot<SignalPolicy::ASYNC, Args...>::Exec(Args&&... args) {
  tp_.VoidPush(0, func_, std::forward<Args>(args)...);
}

template<typename... Args>
class Signal {
 public:
  using SlotPtr = std::shared_ptr<SlotBase<Args...>>; 
  using OnFunc = std::function<void(Args...)>;

  template <SignalPolicy policy>
  void Bind(OnFunc&& func) {
    slots_.push_back(SlotPtr(new Slot<policy, Args...>(std::forward<OnFunc>(func))));
  }

  void operator()(Args&&... args);

 private:
  std::vector<SlotPtr> slots_;
};

template<typename... Args>
void Signal<Args...>::operator()(Args&&... args) {
  for (auto& iter : slots_) {
    iter->Exec(std::forward<Args>(args)...);
  }
}

#endif

