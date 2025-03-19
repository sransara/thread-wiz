#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <unistd.h>
#include <vector>

std::mutex mutex;

int fibonacci(int n) {
  if (n <= 0) {
    return 0;
  } else if (n == 1) {
    return 1;
  } else {
    return fibonacci(n - 1) + fibonacci(n - 2);
  }
}

void calculate_fibonacci(int n) {
  const std::lock_guard<std::mutex> lock(mutex);
  auto start_time = std::chrono::high_resolution_clock::now();
  const int result = fibonacci(n);
  auto end_time = std::chrono::high_resolution_clock::now();
  const std::chrono::duration<double> elapsed = end_time - start_time;
  std::cout << "Fibonacci(" << n << ") = " << result << ", calculated in "
            << elapsed.count() << " seconds\n";
}

int main() {
  std::cout << "Process ID: " << getpid() << std::endl;
  std::cout << "Press Enter to continue...\n";
  std::cin.get();

  const std::vector<int> numbers = {43, 44, 45, 46};
  std::vector<std::thread> threads;
  threads.reserve(numbers.size());

  for (const int number : numbers) {
    threads.emplace_back(std::thread(calculate_fibonacci, number));
  }

  for (auto &thread : threads) {
    thread.join();
  }

  return 0;
}
