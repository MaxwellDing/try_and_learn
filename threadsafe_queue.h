/*************************************************************************
 * Copyright (C) [2020] by Cambricon, Inc. All rights reserved
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *************************************************************************/

#ifndef THREADSAFE_QUEUE_H_
#define THREADSAFE_QUEUE_H_

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T, typename Q = std::queue<T>>
class ThreadSafeQueue {
 public:
  using queue_type = typename std::enable_if<std::is_same<typename Q::value_type, T>::value, Q>::type;
  using value_type = T;
  using size_type = typename Q::size_type;

  ThreadSafeQueue() = default;

  bool TryPop(T& value);

  bool WaitAndTryPop(T& value, const std::chrono::microseconds rel_time);

  void Push(T const& new_value);

  template <typename... Arguments>
  void Emplace(Arguments&&... args) {
    std::lock_guard<std::mutex> lk(data_m_);
    q_.emplace(std::forward<Arguments>(args)...);
    notempty_cond_.notify_one();
  }

  bool Empty() {
    std::lock_guard<std::mutex> lk(data_m_);
    return q_.empty();
  }

  size_type Size() {
    std::lock_guard<std::mutex> lk(data_m_);
    return q_.size();
  }

 private:
  ThreadSafeQueue(const ThreadSafeQueue& other) = delete;
  ThreadSafeQueue& operator=(const ThreadSafeQueue& other) = delete;

  std::mutex data_m_;
  queue_type q_;
  std::condition_variable notempty_cond_;
};  // class ThreadSafeQueue

namespace detail {
template <typename T>
inline void GetFrontAndPop(std::queue<T>* q_, T& value) {
  value = q_->front();
  q_->pop();
}

template <typename T>
inline void GetFrontAndPop(std::priority_queue<T>* q_, T& value) {
  value = q_->top();
  q_->pop();
}
}  // namespace detail

template <typename T, typename Q>
bool ThreadSafeQueue<T, Q>::TryPop(T& value) {
  std::lock_guard<std::mutex> lk(data_m_);
  if (q_.empty()) {
    return false;
  } else {
    detail::GetFrontAndPop<T>(&q_, value);
    return true;
  }
}

template <typename T, typename Q>
bool ThreadSafeQueue<T, Q>::WaitAndTryPop(T& value, const std::chrono::microseconds rel_time) {
  std::unique_lock<std::mutex> lk(data_m_);
  if (notempty_cond_.wait_for(lk, rel_time, [&] { return !q_.empty(); })) {
    detail::GetFrontAndPop<T>(&q_, value);
    return true;
  } else {
    return false;
  }
}

template <typename T, typename Q>
void ThreadSafeQueue<T, Q>::Push(const T& new_value) {
  std::lock_guard<std::mutex> lk(data_m_);
  q_.push(new_value);
  notempty_cond_.notify_one();
}

template <typename T>
using TSQueue = ThreadSafeQueue<T>;

template <typename T>
using TSPriorityQueue = ThreadSafeQueue<T, std::priority_queue<T>>;

#endif  // THREADSAFE_QUEUE_H_
