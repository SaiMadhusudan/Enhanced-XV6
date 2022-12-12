#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_set_priority(void)
{

#ifndef PBS
  printf("Wrong scheduler\n");
  return 0;
#endif

#ifdef PBS
  int priority, pid;
  if (argint(0, &priority) < 0)
    return -1;
  if (argint(1, &pid) < 0)
    return -1;
  int val = set_priority(priority, pid);
  return val;
#endif
}

uint64
sys_set_tickets(void)
{
#ifndef LBS
  printf("Wrong scheduler\n");
  return 0;
#endif

#ifdef LBS
  int number;

  argint(0, &number);
  if (number <= 0)
    return -1;

  int i = set_tickets(number);

  return i;
#endif
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_waitx(void)
{
  uint64 p, raddr, waddr;
  int rtime, wtime;
  if (argaddr(0, &p) < 0)
    return -1;
  if (argaddr(1, &raddr) < 0)
    return -1;
  if (argaddr(2, &waddr) < 0)
    return -1;
  int ret = waitx(p, &rtime, &wtime);
  struct proc *proc = myproc();
  if (copyout(proc->pagetable, raddr, (char *)&rtime, sizeof(int)) < 0)
    return -1;
  if (copyout(proc->pagetable, waddr, (char *)&wtime, sizeof(int)) < 0)
    return -1;
  return ret;
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (killed(myproc()))
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_trace(void)
{
  int n;
  if (argint(0, &n) < 0)
    return -1;
  myproc()->trace_mask = n;
  return 0;
}

uint64
sys_sigalarm(void)
{
  int ticks;
  if (argint(0, &ticks) < 0)
    return -1;
  uint64 handler;
  if (argaddr(1, &handler) < 0)
    return -1;
  myproc()->alarm_on = 0;
  myproc()->ticks = ticks;
  myproc()->cur_ticks = 0;
  myproc()->handler = handler;
  return 0;
}
void restore()
{
  struct proc *p = myproc();
  p->alarm_tf->kernel_satp = p->trapframe->kernel_satp;
  p->alarm_tf->kernel_sp = p->trapframe->kernel_sp;
  p->alarm_tf->kernel_trap = p->trapframe->kernel_trap;
  p->alarm_tf->kernel_hartid = p->trapframe->kernel_hartid;
  *(p->trapframe) = *(p->alarm_tf);
}

uint64
sys_sigreturn(void)
{
  restore();
  myproc()->alarm_on = 0;
  return myproc()->trapframe->a0;
}