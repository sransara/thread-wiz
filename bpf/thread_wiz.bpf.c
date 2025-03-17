#include "vmlinux.h"

#include <bpf/bpf_helpers.h>

SEC("tp/sched/sched_switch")
int
handle_exec(struct trace_event_raw_sched_switch* ctx)
{
  struct task_struct* task = (struct task_struct*)bpf_get_current_task();
  bpf_printk("PID %d PPID: %d CTXPPID %d CTXNPID %d.\n",
             task->pid,
             task->parent->pid,
             ctx->prev_pid,
             ctx->next_pid);
  return 0;
}
