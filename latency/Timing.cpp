
#include <iostream>
#include "Timing.h"

// Define the static members
std::chrono::high_resolution_clock::time_point start_time;
std::chrono::high_resolution_clock::time_point end_time;

void startTimer() {
  start_time = std::chrono::high_resolution_clock::now();
}

void stopTimer() {
  end_time = std::chrono::high_resolution_clock::now();
}

void calculateLatency() {
  std::cout << "Start time: " << std::chrono::duration_cast<std::chrono::microseconds>(start_time.time_since_epoch()).count() << " microseconds" << std::endl;
  std::cout << "End time: " << std::chrono::duration_cast<std::chrono::microseconds>(end_time.time_since_epoch()).count() << " microseconds" << std::endl;
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
  std::cout << "Latency: " << duration.count() << " microseconds" << std::endl;
}
