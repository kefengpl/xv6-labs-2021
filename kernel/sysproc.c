#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
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
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;
  backtrace();

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
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

  if(argint(0, &pid) < 0)
    return -1;
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

/**
 * @param interval:int 表示有interval个tick(或者参数是n，有n个tick)
 * @param handler:funcpointer 是函数指针
 * @note tick代表时间的流逝，可以作为一个“计时单位”
 * @note 调用此函数表明，在一个process经过interval个tick之后，将会调用handler
 * @note 例如sigalarm(2, periodic)表示该process每经过两个tick，将执行一次periodic函数
 * @note 当handler函数返回后，这个process需要从它被打断的地方继续执行
 * @note sigalarm(0, 0)代表该进程将结束周期性地call handler
*/
uint64
sys_sigalarm(void)
{
  // 首先获取上述注释中的两个参数
  int interval;
  if (argint(0, &interval) < 0) return -1;
  // 注意：此处获得的是函数指针的虚拟地址
  // 位于用户页表下的handler函数指针虚拟地址
  uint64 handler_va;
  if (argaddr(1, &handler_va) < 0) return -1;
  struct proc *p = myproc();
  // 如果函数是sigalarm(0, 0)，就停止alarm calls
  if (interval == 0 && handler_va == 0) {
    p->handler = 0;
    p->alarm_interval = 0;
    p->ticks_count = 0;
    p->has_handler = 0;
    return 0;
  }
  // 其它情况下，需要赋值给线程p的有关属性
  p->has_handler = 1;
  p->alarm_interval = interval;
  p->handler = handler_va;
  p->ticks_count = 0;
  return 0;
}

/**
 * @note tick alarm调用handler函数之后需要
 * @note 恢复调用handler之前该进程的寄存器
*/
uint64
sys_sigreturn(void)
{
  struct proc* p = myproc();
  *p->trapframe = *p->trapframe_backup;
  p->in_handler = 0;
  return 0;
}
