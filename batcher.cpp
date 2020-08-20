#include "batcher.h"

#include <future>
#include <iostream>

namespace edk {

template <class in_type>
void Batcher<in_type>::Emit() {
  if (notifier_) {
    std::async(std::launch::async, notifier_, cache_);
  } else {
    std::cout << "Batcher donot have notifier, do nothing" << std::endl;
  }
  cache_.clear();
}

}  // namespace edk

