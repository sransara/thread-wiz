#include "vmlinux.h"

#include <bpf/bpf_core_read.h>
#include <bpf/bpf_helpers.h>

char LICENSE[] SEC("license") = "Dual BSD/GPL";

const volatile pid_t pid_for_observation = 0;
const int max_task_traversal = 10;

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

SEC("tp/sched/sched_switch")
int handle_exec(struct trace_event_raw_sched_switch *ctx) {
  struct task_struct *task = (struct task_struct *)bpf_get_current_task();
  if (!is_process_under_observation(task)) {
    return 0;
  }
  bpf_printk("PID %d PPID: %d CTXPPID %d CTXNPID %d.\n",
             BPF_CORE_READ(task, pid), BPF_CORE_READ(task, parent, pid),
             ctx->prev_pid, ctx->next_pid);
  return 0;
}
