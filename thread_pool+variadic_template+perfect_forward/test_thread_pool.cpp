#include <iostream>

#include "thread_pool.hpp"

int main() {
  ThreadPool pool(10);
  std::vector<std::future<int>> results;

  for (int i = 0; i < 1000; ++i) {
    results.emplace_back(pool.enque(
        [](int n) {
          int sum = 0;
          for (int i = 0; i < n; ++i)
            sum += i;
          return sum;
        },
        i));
  }

  for (auto &result : results) {
    std::cout << result.get() << std::endl;
  }

  return 0;
}