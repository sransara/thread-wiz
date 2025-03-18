#include "event_handler.h"
#include "thread_wiz.bpf.h"
#include <cstddef>
#include <iostream>

int event_handler(void *ctx, void *data, size_t data_sz) {
  (void)ctx;
  (void)data_sz;

  const struct event *event = static_cast<const struct event *>(data);
  switch (event->type) {
  case THREAD_TRACE_NEW:
    std::cout << "New thread: " << event->event.event_trace_new.comm
              << " (pid: " << event->event.event_trace_new.pid << ") on CPU "
              << event->event.event_trace_new.target_cpu << "\n";
    break;
  case THREAD_TRACE_SWITCH:
    std::cout << "Thread switch: " << event->event.event_trace_switch.prev_pid
              << " (tgid: " << event->event.event_trace_switch.prev_tgid
              << ") -> " << event->event.event_trace_switch.next_pid
              << " (tgid: " << event->event.event_trace_switch.next_tgid
              << ") on CPU " << event->event.event_trace_switch.cpu << "\n";
    break;
  case THREAD_TRACE_TERMINATE:
    std::cout << "Thread terminate: " << event->event.event_trace_terminate.pid
              << "\n";
    break;
  default:
    std::cerr << "Unknown event type: " << event->type << "\n";
    break;
  }
  return 0;
}
