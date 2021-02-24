/*************************************************************************
 * Copyright (C) [2019] by Cambricon, Inc. All rights reserved
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
#ifndef EDK_CXXUTIL_RWLOCK_H_
#define EDK_CXXUTIL_RWLOCK_H_

#include <pthread.h>

#include <memory>
#include <utility>

class RwLock {
 public:
  RwLock() { pthread_rwlock_init(&rwlock_, NULL); }
  ~RwLock() { pthread_rwlock_destroy(&rwlock_); }
  void WriteLock() { pthread_rwlock_wrlock(&rwlock_); }
  void ReadLock() { pthread_rwlock_rdlock(&rwlock_); }
  void Unlock() { pthread_rwlock_unlock(&rwlock_); }

 private:
  pthread_rwlock_t rwlock_;
};

class RwLockWriteGuard {
 public:
  explicit RwLockWriteGuard(RwLock& lock) : lock_(lock) { lock_.WriteLock(); }
  ~RwLockWriteGuard() { lock_.Unlock(); }

 private:
  RwLock& lock_;
};

class RwLockReadGuard {
 public:
  explicit RwLockReadGuard(RwLock& lock) : lock_(lock) { lock_.ReadLock(); }
  ~RwLockReadGuard() { lock_.Unlock(); }

 private:
  RwLock& lock_;
};

#endif  // EDK_CXXUTIL_RWLOCK_H_
