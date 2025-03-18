#ifndef THREAD_WIZ_H
#define THREAD_WIZ_H

#define THREAD_COMM_LEN 16

typedef int pid_t;
typedef unsigned int cpuid_t;
typedef unsigned long long timestamp_t;
typedef long procstate_t;

enum event_type { // NOLINT: C doesn't allow base types in enums
  THREAD_TRACE_NEW,
  THREAD_TRACE_SWITCH,
  THREAD_TRACE_TERMINATE
};

struct event_trace_new {
  char comm[THREAD_COMM_LEN]; // name of the thread
  pid_t pid;                  // pid of the new thread
  cpuid_t target_cpu;         // cpu the thread will be running on
};

struct event_trace_switch {
  timestamp_t timestamp_ns; // system boot time in nanoseconds
  procstate_t prev_state;   // previous state
  cpuid_t cpu;              // origin cpu
  pid_t prev_pid;           // previous pid
  pid_t prev_tgid;          // previous tgid
  pid_t next_pid;           // next scheduled pid
};

struct event_trace_terminate {
  pid_t pid; // terminating pid
};

union _event {
  struct event_trace_new event_trace_new;
  struct event_trace_switch event_trace_switch;
  struct event_trace_terminate event_trace_terminate;
};

struct event {
  enum event_type type;
  union _event event;
};

#endif
