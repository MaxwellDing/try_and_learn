#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <glog/logging.h>
#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

#include "threadsafe_queue.h"

// thread pool to run user's functors with signature
//      ret func(int id, other_params)
// where id is the index of the thread that runs the functor
// ret is some return type

struct Task {
  void operator()() {
    if (func) {
      (func)();
    } else {
      LOG(WARNING) << "No task function";
    }
  }

  std::function<void()> func = nullptr;
  int64_t priority = 0;
  Task() = default;

  friend bool operator<(const Task &lhs, const Task &rhs) { return lhs.priority < rhs.priority; }
};

template <typename Q = TSQueue<Task>,
          typename T = typename std::enable_if<std::is_same<typename Q::value_type, Task>::value, Task>::type>
class ThreadPool {
 public:
  using queue_type = Q;
  using task_type = T;
  explicit ThreadPool(std::function<bool()> thread_init_func) : thread_init_func_(thread_init_func) {}
  ThreadPool(std::function<bool()> thread_init_func, int n_threads) : thread_init_func_(thread_init_func) {
    Resize(n_threads);
  }

  // the destructor waits for all the functions in the queue to be finished
  ~ThreadPool() { Stop(true); }

  // get the number of running threads in the pool
  size_t Size() const noexcept { return threads_.size(); }

  // number of idle threads
  int IdleNumber() const noexcept { return n_waiting_; }
  std::thread &GetThread(int i) { return *threads_[i]; }

  // change the number of threads in the pool
  // should be called from one thread, otherwise be careful to not interleave, also with this->stop()
  void Resize(size_t n_threads) {
    if (!is_stop_ && !is_done_) {
      size_t old_n_threads = threads_.size();
      if (old_n_threads <= n_threads) {
        // if the number of threads is increased
        VLOG(3) << "add " << n_threads - old_n_threads << " threads into threadpool";
        threads_.resize(n_threads);
        flags_.resize(n_threads);

        for (size_t i = old_n_threads; i < n_threads; ++i) {
          flags_[i] = std::make_shared<std::atomic<bool>>(false);
          SetThread(i);
        }
      } else {
        // the number of threads is decreased
        VLOG(3) << "stop " << old_n_threads - n_threads << " threads in threadpool";
        for (size_t i = old_n_threads - 1; i >= n_threads; --i) {
          // this thread will finish
          *flags_[i] = true;
          threads_[i]->detach();
        }

        {
          // stop the detached threads that were waiting
          std::unique_lock<std::mutex> lock(mutex_);
          cv_.notify_all();
        }

        // safe to delete because the threads are detached
        threads_.resize(n_threads);
        // safe to delete because the threads have copies of shared_ptr of the flags, not originals
        flags_.resize(n_threads);
      }
    }
  }

  // empty the queue
  void ClearQueue() {
    task_type t;
    // empty the queue
    while (task_q_.TryPop(t)) {}
  }

  // pops a functional wrapper to the original function
  task_type Pop() {
    task_type t;
    task_q_.TryPop(t);
    return t;
  }

  // wait for all computing threads to finish and stop all threads
  // may be called asynchronously to not pause the calling thread while waiting
  // if wait_all_task_done == true, all the functions in the queue are run, otherwise the queue is cleared without
  // running the functions
  void Stop(bool wait_all_task_done = false) {
    if (!wait_all_task_done) {
      VLOG(3) << "stop all the thread without waiting for remained task done";
      if (is_stop_) return;
      is_stop_ = true;
      for (size_t i = 0, n = this->Size(); i < n; ++i) {
        // command the threads to stop
        flags_[i]->store(true);
      }

      // empty the queue
      this->ClearQueue();
    } else {
      VLOG(3) << "waiting for remained task done before stop all the thread";
      if (is_done_ || is_stop_) return;
      // give the waiting threads a command to finish
      is_done_ = true;
    }

    cv_.notify_all();  // stop all waiting threads

    // wait for the computing threads to finish
    for (size_t i = 0; i < threads_.size(); ++i) {
      if (threads_[i]->joinable()) threads_[i]->join();
    }

    // if there were no threads in the pool but some functors in the queue, the functors are not deleted by the threads
    // therefore delete them here
    this->ClearQueue();
    threads_.clear();
    flags_.clear();
  }

  // run the user's function, returned value is templatized
  // operator returns std::future, where the user can get the result and rethrow the catched exceptins
  template <typename callable, typename... arguments>
  auto Push(int64_t priority, callable &&f, arguments &&... args) -> std::future<decltype(f(args...))> {
    VLOG(6) << "Sumbit one task to threadpool, priority: " << priority;
    VLOG(6) << "thread pool (idle/total): " << IdleNumber() << " / " << Size();
    auto pck = std::make_shared<std::packaged_task<decltype(f(args...))()>>(
        std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));
    task_type t;
    t.func = [pck]() { (*pck)(); };
    t.priority = priority;
    task_q_.Push(t);
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.notify_one();
    return pck->get_future();
  }

  // run the user's function, no future so that user cannot get return of task
  // there's no future, therefore user should guarantee that task won't throw,
  // otherwise the program may be corrupted
  template <typename callable, typename... arguments>
  void VoidPush(int64_t priority, callable &&f, arguments &&... args) {
    VLOG(6) << "Sumbit one task to threadpool, priority: " << priority;
    VLOG(6) << "thread pool (idle/total): " << IdleNumber() << " / " << Size();
    task_type t;
    t.func = std::bind(std::forward<callable>(f), std::forward<arguments>(args)...);
    t.priority = priority;
    task_q_.Push(t);
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.notify_one();
  }

 private:
  // deleted
  ThreadPool(const ThreadPool &) = delete;
  ThreadPool(ThreadPool &&) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;
  ThreadPool &operator=(ThreadPool &&) = delete;

  void SetThread(int i) {
    std::shared_ptr<std::atomic<bool>> tmp_flag(flags_[i]);
    auto f = [this, i, tmp_flag]() {
      std::atomic<bool> &flag = *tmp_flag;
      // init params that bind with thread
      if (thread_init_func_) {
        if (thread_init_func_()) {
          VLOG(5) << "Init thread context success, thread index: " << i;
        } else {
          LOG(ERROR) << "Init thread context failed, but program will continue. "
                        "Program cannot work correctly maybe.";
        }
      }
      task_type t;
      bool have_task = task_q_.TryPop(t);
      while (true) {
        // if there is anything in the queue
        while (have_task) {
          t();
          // params encapsulated in std::function need destruct at once
          t.func = nullptr;
          if (flag) {
            // the thread is wanted to stop, return even if the queue is not empty yet
            return;
          } else {
            have_task = task_q_.TryPop(t);
          }
        }

        // the queue is empty here, wait for the next command
        std::unique_lock<std::mutex> lock(mutex_);
        ++n_waiting_;
        cv_.wait(lock, [this, &t, &have_task, &flag]() {
          have_task = task_q_.TryPop(t);
          return have_task || is_done_ || flag;
        });
        --n_waiting_;

        // if the queue is empty and is_done_ == true or *flag then return
        if (!have_task) return;
      }
    };

    threads_[i].reset(new std::thread(f));
  }

  std::vector<std::unique_ptr<std::thread>> threads_;
  std::vector<std::shared_ptr<std::atomic<bool>>> flags_;
  queue_type task_q_;
  std::atomic<bool> is_done_{false};
  std::atomic<bool> is_stop_{false};
  // how many threads are waiting
  std::atomic<int> n_waiting_{0};

  std::mutex mutex_;
  std::condition_variable cv_;

  std::function<bool()> thread_init_func_{nullptr};
};  // class ThreadPool

using EqualityThreadPool = ThreadPool<TSQueue<Task>>;
using PriorityThreadPool = ThreadPool<TSPriorityQueue<Task>>;

#endif  // THREAD_POOL_H_
