#include <chrono>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <vector>

// Function for each thread to execute
void sleepForDuration(int threadId, int sleepTime) {
  std::cout << "Thread " << threadId << " sleeping for " << sleepTime
            << " milliseconds.\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
  std::cout << "Thread " << threadId << " finished sleeping.\n";
}

int main() {
  std::cout << "Process ID: " << getpid() << std::endl;
  std::cout << "Press Enter to continue...\n";
  std::cin.get();

  const int numThreads{5};
  const int sleepTime[]{1000, 2000, 3000, 4000, 5000};

  std::vector<std::thread> threads;

  // Start threads
  for (int i = 0; i < numThreads; ++i) {
    threads.emplace_back(std::thread(sleepForDuration, i, sleepTime[i]));
  }

  // Join threads
  for (auto &thread : threads) {
    thread.join();
  }

  std::cout << "All threads have completed.\n";
  return 0;
}
