#include "thread_wiz.skel.h"
#include <iostream>

int
main()
{
  thread_wiz_bpf* skel = thread_wiz_bpf::open();
  if (skel == nullptr) {
    std::cerr << "Failed to open and load BPF object\n";
    return 1;
  }
  thread_wiz_bpf::load(skel);
  auto err = thread_wiz_bpf::load(skel);
  if (err != 0) {
    std::cerr << "Failed to load BPF object\n";
    return 1;
  }
  std::cout << "Successfully loaded BPF object\n";
  err = thread_wiz_bpf::attach(skel);
  if (err != 0) {
    std::cerr << "Failed to attach BPF handlers\n";
    return 1;
  }
  std::cout << "Successfully attached BPF handlers\n";
  for (;;) {
  }
  thread_wiz_bpf::destroy(skel);
}
