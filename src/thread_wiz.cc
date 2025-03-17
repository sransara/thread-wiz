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
  for (;;) {
  }
  thread_wiz_bpf::destroy(skel);
}
