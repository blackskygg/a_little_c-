#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
  ThreadPool(int worker_size) : stop_(false) {
    for (int i = 0; i < worker_size; ++i) {
      workers_.emplace_back([this] {
        std::cout << "worker started\n";
        for (;;) {
          std::function<void()> task;
          {
            // mutex + condvar is a good replacement for semaphore.
            std::unique_lock<std::mutex> lock(mutex_); // RIIA for mutex_

            // Unlock and check. If the predicate is false, sleep and wait util
            // cond_.notify_*().
            cond_.wait(lock, [this] { return stop_ || !tasks_.empty(); });

            if (stop_) {
              std::cout << "worker stopped\n";
              return;
            }

            task = std::move(tasks_.front());
            tasks_.pop();
          }
          task();
        }
      });
    }
  }

  template <class F, class... Args>
  auto enque(F &&f, Args &&... args)
      -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    // Must use pointer because we want to share the same future with wokers and
    // the caller.
    auto async_task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    {
      std::unique_lock<std::mutex> lock(mutex_);

      if (stop_) {
        throw std::runtime_error("can't enque when thread pool is stopped.");
      }
      tasks_.emplace([async_task] { (*async_task)(); });
    }

    cond_.notify_one();
    return async_task->get_future();
  }

  ~ThreadPool() {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      stop_ = true;
    }
    cond_.notify_all();
    for (auto &worker : workers_) {
      worker.join();
    }
  }

private:
  std::queue<std::function<void()>> tasks_;
  std::vector<std::thread> workers_;

  std::mutex mutex_;
  std::condition_variable cond_;

  bool stop_;
};