#include "event_handler.h"
#include "thread_wiz.skel.h"
#include "web_child.h"
#include <bpf/libbpf.h>
#include <csignal>
#include <errno.h>
#include <iostream>
#include <string>

namespace {
volatile bool exiting = false;

void sig_handler(int sig) {
  (void)sig;
  exiting = true;
}

} // namespace

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <pid>\n";
    return 1;
  }
  const int pid_for_observation = std::stoi(argv[1]);

  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);

  thread_wiz_bpf *skel = thread_wiz_bpf::open();
  if (skel == nullptr) {
    std::cerr << "Failed to open and load BPF object\n";
    return 1;
  }

  skel->rodata->pid_for_observation = pid_for_observation;

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

  struct ring_buffer *data_bus = ring_buffer__new(
      bpf_map__fd(skel->maps.thread_wiz_bus), event_handler, nullptr, nullptr);
  if (data_bus == nullptr) {
    std::cerr << "Failed to create ring buffer\n";
    return 1;
  }
  err = open_web_process();
  if (err != 0) {
    return 1;
  }

  const int epoll_timeout = 100; // ms
  while (!exiting) {
    err = ring_buffer__poll(data_bus, epoll_timeout);
    if (err == -EINTR) {
      // Interrupted by signal, exit
      break;
    }
    if (err < 0) {
      std::cerr << "Error polling ring buffer\n";
      break;
    }
  }

  ring_buffer__free(data_bus);
  close_web_process();
  thread_wiz_bpf::destroy(skel);

  return 0;
}
