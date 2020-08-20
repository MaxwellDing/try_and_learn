#include <functional>
#include <memory>
#include <mutex>
#include <vector>
#include <future>
#include <iostream>

namespace edk {

template <class in_type>
class Batcher {
 public:
  using notifier_type = std::function<void(std::vector<in_type>)>;

  void AddItem(in_type&& item) {
    std::lock_guard<std::mutex> lk(m_);
    cache_.emplace_back(std::forward<in_type>(item));
    if (cache_.size() >= batch_size_) {
      Emit();
    }
  }
  void SetTimeOut(int64_t t) {
    timeout_ = t;
  }
  void SetNotifier(notifier_type notifier) {
    notifier_ = notifier;
  }
  void SetBatchSize(uint32_t bs) {
    batch_size_ = bs;
    cache_.reserve(bs);
  }
 private:
  void Emit() {
    if (notifier_) {
      std::async(std::launch::async, notifier_, cache_);
    } else {
      std::cout << "Batcher donot have notifier, do nothing" << std::endl;
    }
    cache_.clear();
  }
  std::vector<in_type> cache_;
  notifier_type notifier_ = nullptr;
  int64_t timeout_ = -1;
  uint32_t batch_size_ = 0;
  std::mutex m_;
};

}  // namespace edk

