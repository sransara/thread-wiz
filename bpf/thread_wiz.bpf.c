#include "vmlinux.h"

#include "thread_wiz.bpf.h"
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_helpers.h>

char LICENSE[] SEC("license") = "Dual BSD/GPL";

const int max_task_traversal = 10;

struct {
  __uint(type, BPF_MAP_TYPE_RINGBUF);
  __uint(max_entries, sizeof(struct event) * 1024);
} thread_wiz_bus SEC(".maps");

// taken as input from the userspace program
const volatile pid_t pid_for_observation = 0;

bool is_process_under_observation(struct task_struct *task) {
  // BPF only allows bounded loops, so we limit the number of tasks to traverse
  for (int i = 0; task != NULL && i < max_task_traversal; i++) {
    pid_t this_pid = BPF_CORE_READ(task, tgid);
    if (this_pid == pid_for_observation) {
      return true;
    }
    task = BPF_CORE_READ(task, parent);
  }
  return false;
}

SEC("tp/sched/sched_wakeup_new")
int handle_wakeup_new(struct trace_event_raw_sched_wakeup_template *ctx) {
  struct task_struct *task = (struct task_struct *)bpf_get_current_task();
  if (!is_process_under_observation(task)) {
    return 0;
  }
  struct event *event =
      bpf_ringbuf_reserve(&thread_wiz_bus, sizeof(struct event), 0);
  if (event == NULL) {
    return 0;
  }
  event->type = THREAD_TRACE_NEW;
  struct event_trace_new *event_trace_new = &event->event.event_trace_new;
  // read null terminated string from trace context
  bpf_probe_read_str(event_trace_new->comm, sizeof(event_trace_new->comm),
                     ctx->comm);
  event_trace_new->pid = ctx->pid;
  event_trace_new->target_cpu = ctx->target_cpu;
  bpf_ringbuf_submit(event, 0);
  return 0;
}

SEC("tp/sched/sched_switch")
int handle_switch(struct trace_event_raw_sched_switch *ctx) {
  // if neither the previous nor the next task is under observation, return
  struct task_struct *ptask = bpf_task_from_pid(ctx->prev_pid);
  struct task_struct *ntask = bpf_task_from_pid(ctx->next_pid);
  if (!is_process_under_observation(ptask) &&
      !is_process_under_observation(ntask)) {
    if (ptask != NULL) {
      bpf_task_release(ptask);
    }
    if (ntask != NULL) {
      bpf_task_release(ntask);
    }
    return 0;
  }
  pid_t prev_tgid;
  pid_t next_tgid;
  if (ptask != NULL) {
    prev_tgid = BPF_CORE_READ(ptask, tgid);
    bpf_task_release(ptask);
  }
  if (ntask != NULL) {
    next_tgid = BPF_CORE_READ(ntask, tgid);
    bpf_task_release(ntask);
  }

  struct event *event =
      bpf_ringbuf_reserve(&thread_wiz_bus, sizeof(struct event), 0);
  if (event == NULL) {
    return 0;
  }
  event->type = THREAD_TRACE_SWITCH;
  struct event_trace_switch *event_trace_switch =
      &event->event.event_trace_switch;

  const cpuid_t cpu = bpf_get_smp_processor_id(); // get the active CPU

  event_trace_switch->timestamp_ns = bpf_ktime_get_ns(); // get boot timestamp
  event_trace_switch->prev_state = ctx->prev_state;
  event_trace_switch->cpu = cpu;
  event_trace_switch->prev_pid = ctx->prev_pid;
  event_trace_switch->prev_tgid = prev_tgid;
  event_trace_switch->next_pid = ctx->next_pid;
  event_trace_switch->next_tgid = next_tgid;

  bpf_ringbuf_submit(event, 0);
  return 0;
}

SEC("tp/sched/sched_process_exit")
int handle_exit(struct trace_event_raw_sched_process_template *ctx) {
  struct task_struct *task = (struct task_struct *)bpf_get_current_task();
  if (!is_process_under_observation(task)) {
    return 0;
  }
  struct event *event =
      bpf_ringbuf_reserve(&thread_wiz_bus, sizeof(struct event), 0);
  if (event == NULL) {
    return 0;
  }
  event->type = THREAD_TRACE_TERMINATE;
  struct event_trace_terminate *event_trace_terminate =
      &event->event.event_trace_terminate;
  event_trace_terminate->pid = ctx->pid;
  bpf_ringbuf_submit(event, 0);
  return 0;
}
