#include <future>
#include <iostream>
#include <mutex>

#include "rwlock.h"
#include "rw_mutex.h"
#include "spinlock.h"
using Clock = std::chrono::steady_clock;
using Duration = std::chrono::duration<double, std::milli>;
using TimePoint = std::chrono::time_point<Clock, Duration>;
void TestRwlock() {
  TimePoint start, mid, end;
  Duration dura;

  constexpr int test_time = 100000;
  int t = 0;
  int factor = 1;

  std::cout << "--------------------------------------------------\n";
  RwLock pthread_rw_mutex;
  t = test_time;
  start = Clock::now();
  while (t--) {
    RwLockReadGuard lk(pthread_rw_mutex);
  }
  end = Clock::now();
  dura = end - start;
  std::cout << "pthread read " << test_time << " times: " << dura.count() << "ms\n";
t = test_time;
  start = Clock::now();
  while (t--) {
    RwLockWriteGuard lk(pthread_rw_mutex);
  }
  end = Clock::now();
  dura = end - start;
  std::cout << "pthread write " << test_time << " times: " << dura.count() << "ms\n";

  std::future<double> ret;
  t = test_time;
  start = Clock::now();
  auto read_process = [&pthread_rw_mutex, test_time, factor]() -> double {
    int t = factor * test_time;
    TimePoint s = Clock::now();
    while (t--) {
      RwLockReadGuard lk(pthread_rw_mutex);
    }
    TimePoint e = Clock::now();
    return (e - s).count();
  };
  ret = std::async(std::launch::async, read_process);
  auto ret2 = std::async(std::launch::async, read_process);
  while (t--) {
    RwLockWriteGuard lk(pthread_rw_mutex);
  }
  mid = Clock::now();
  double read_tmp = ret.get();
  ret2.get();
  end = Clock::now();
  dura = end - start;
  std::cout << "pthread read " << factor * test_time << " times, "
            << "write " << test_time << ": " << dura.count() << "ms\n";
  std::cout << "\tpthread write " << test_time << " times: " << (mid - start).count() << "ms\n";
  std::cout << "\tpthread read " << test_time << " times: " << read_tmp << "ms\n";

  std::cout << "--------------------------------------------------\n";
  RwMutex rw_mutex;
  t = test_time;
  start = Clock::now();
  while (t--) {
    UniqueReadLock lk(rw_mutex);
  }
  end = Clock::now();
  dura = end - start;
  std::cout << "read " << test_time << " times: " << dura.count() << "ms\n";

  t = test_time;
  start = Clock::now();
  while (t--) {
    UniqueWriteLock lk(rw_mutex);
  }
  end = Clock::now();
  dura = end - start;
  std::cout << "write " << test_time << " times: " << dura.count() << "ms\n";

  t = test_time;
  start = Clock::now();
  auto read_process2 = [&rw_mutex, test_time, factor]() -> double {
    int t = factor * test_time;
    TimePoint s = Clock::now();
    while (t--) {
      UniqueReadLock lk(rw_mutex);
    }
    TimePoint e = Clock::now();
    return (e - s).count();
  };
  ret = std::async(std::launch::async, read_process2);
  ret2 = std::async(std::launch::async, read_process2);
  while (t--) {
    UniqueWriteLock lk(rw_mutex);
  }
  mid = Clock::now();
  read_tmp = ret.get();
  ret2.get();
  end = Clock::now();
  dura = end - start;
  std::cout << "read " << factor * test_time << " times, "
            << "write " << test_time << ": " << dura.count() << "ms\n";
  std::cout << "\twrite " << test_time << " times: " << (mid - start).count() << "ms\n";
  std::cout << "\tread " << test_time << " times: " << read_tmp << "ms\n";

  std::cout << "--------------------------------------------------\n";
  std::mutex mut;
  t = test_time;
  start = Clock::now();
  while (t--) {
    std::lock_guard<std::mutex> lk(mut);
  }
  end = Clock::now();
  dura = end - start;
  std::cout << "std::mutex lock " << test_time << " times: " << dura.count() << "ms\n";

  t = test_time;
  start = Clock::now();
  auto read_process3 = [&mut, test_time, factor]() -> double {
    int t = factor * test_time;
    TimePoint s = Clock::now();
    while (t--) {
      std::lock_guard<std::mutex> lk(mut);
    }
    TimePoint e = Clock::now();
    return (e - s).count();
  };
  ret = std::async(std::launch::async, read_process3);
  ret2 = std::async(std::launch::async, read_process3);
  while (t--) {
    std::lock_guard<std::mutex> lk(mut);
  }
  mid = Clock::now();
  read_tmp = ret.get();
  ret2.get();
  end = Clock::now();
  dura = end - start;
  std::cout << "one thread lock " << factor * test_time << " times, "
            << "another thread lock " << test_time << ": " << dura.count() << "ms\n";
  std::cout << "\twrite " << test_time << " times: " << (mid - start).count() << "ms\n";
  std::cout << "\tread " << test_time << " times: " << read_tmp << "ms\n";


  std::cout << "--------------------------------------------------\n";
  SpinLock sp_lk;
  t = test_time;
  start = Clock::now();
  while (t--) {
    SpinLockGuard lk(sp_lk);
  }
  end = Clock::now();
  dura = end - start;
  std::cout << "spin lock " << test_time << " times: " << dura.count() << "ms\n";

  t = test_time;
  start = Clock::now();
  auto read_process4 = [&sp_lk, test_time, factor]() -> double {
    int t = factor * test_time;
    TimePoint s = Clock::now();
    while (t--) {
      SpinLockGuard lk(sp_lk);
    }
    TimePoint e = Clock::now();
    return (e - s).count();
  };
  ret = std::async(std::launch::async, read_process4);
  ret2 = std::async(std::launch::async, read_process4);
  while (t--) {
    SpinLockGuard lk(sp_lk);
  }
  mid = Clock::now();
  read_tmp = ret.get();
  ret2.get();
  end = Clock::now();
  dura = end - start;
  std::cout << "one thread spinlock " << factor * test_time << " times, "
            << "another thread spinlock " << test_time << ": " << dura.count() << "ms\n";
  std::cout << "\twrite " << test_time << " times: " << (mid - start).count() << "ms\n";
  std::cout << "\tread " << test_time << " times: " << read_tmp << "ms\n";


  std::cout << "--------------------------------------------------\n";
  pthread_spinlock_t pthread_sp_lk;
  pthread_spin_init(&pthread_sp_lk, PTHREAD_PROCESS_PRIVATE);
  t = test_time;
  start = Clock::now();
  while (t--) {
    pthread_spin_lock(&pthread_sp_lk);
    pthread_spin_unlock(&pthread_sp_lk);
  }
  end = Clock::now();
  dura = end - start;
  std::cout << "pthread spin lock " << test_time << " times: " << dura.count() << "ms\n";

  t = test_time;
  start = Clock::now();
  auto read_process5 = [&pthread_sp_lk, test_time, factor]() -> double {
    int t = factor * test_time;
    TimePoint s = Clock::now();
    while (t--) {
      pthread_spin_lock(&pthread_sp_lk);
      pthread_spin_unlock(&pthread_sp_lk);
    }
    TimePoint e = Clock::now();
    return (e - s).count();
  };
  ret = std::async(std::launch::async, read_process5);
  ret2 = std::async(std::launch::async, read_process5);
  while (t--) {
    pthread_spin_lock(&pthread_sp_lk);
    pthread_spin_unlock(&pthread_sp_lk);
  }
  mid = Clock::now();
  read_tmp = ret.get();
  ret2.get();
  end = Clock::now();
  dura = end - start;
  std::cout << "pthread spinlock " << factor * test_time << " times, "
            << "another pthread spinlock " << test_time << ": " << dura.count() << "ms\n";
  std::cout << "\twrite " << test_time << " times: " << (mid - start).count() << "ms\n";
  std::cout << "\tread " << test_time << " times: " << read_tmp << "ms\n";
  pthread_spin_destroy(&pthread_sp_lk);
}
