#include "event_handler.h"
#include "thread_wiz.bpf.h"
#include "web_child.h"
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int event_handler(void *ctx, void *data, size_t data_sz) {
  (void)ctx;
  (void)data_sz;

  const struct event *event = static_cast<const struct event *>(data);
  json jsono;
  switch (event->type) {
  case THREAD_TRACE_NEW:
    jsono = {{"type", "THREAD_TRACE_NEW"},
             {"comm", event->event.event_trace_new.comm},
             {"pid", event->event.event_trace_new.pid},
             {"target_cpu", event->event.event_trace_new.target_cpu}};
    break;
  case THREAD_TRACE_SWITCH:
    jsono = {
        {"type", "THREAD_TRACE_SWITCH"},
        {"timestamp_ns", event->event.event_trace_switch.timestamp_ns},
        {"prev_state", event->event.event_trace_switch.prev_state},
        {"cpu", event->event.event_trace_switch.cpu},
        {"prev_pid", event->event.event_trace_switch.prev_pid},
        {"prev_tgid", event->event.event_trace_switch.prev_tgid},
        {"next_pid", event->event.event_trace_switch.next_pid},
        {"next_tgid", event->event.event_trace_switch.next_tgid},
        {"prev_is_observed", event->event.event_trace_switch.prev_is_observed},
        {"next_is_observed", event->event.event_trace_switch.next_is_observed}};
    break;
  case THREAD_TRACE_TERMINATE:
    jsono = {{"type", "THREAD_TRACE_TERMINATE"},
             {"pid", event->event.event_trace_terminate.pid}};
    break;
  default:
    jsono = {{"type", "UNKNOWN"}, {"event_type", event->type}};
    std::cerr << "Unknown event type: " << event->type << "\n";
    break;
  }

  write_to_web_process(jsono);
  return 0;
}
