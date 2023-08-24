#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

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
 * @param mask:int 一个二进制数据，希望追踪哪个系统调用，
 * 就把这个参数的某个比特位设置成1，其它比特位是0
 * 例如：追踪系统调用的需要为1和3，SYS_fork和SYS_wait
 * 则这个参数(mask)应该是....1010(比特位从0开始)
 * 即(1 << SYS_fork) | (1 << SYS_wait)
 * @return 根据xv6系统调用函数的规定，失败返回-1，成功
 * 则返回各种对应的数值
*/
uint64
sys_trace(void)
{
  // 获取user mode中输入的参数
  // 如何获取？user界面的interface中接收了一个int参数
  // 这会储存到寄存器a0中，我们把a0寄存器中的数据读出来即可
  int mask;
  if (argint(0, &mask) < 0) {
    return -1;
  }
  //将获取到的mask参数存入该进程的相关变量中
  struct proc* p = myproc();
  p->tracemask = mask;
  //成功则返回非负值
  return 0;
}

/**
 * @param sysinfo:struct-sysinfo* 一个sysinfo结构体指针
 * 表示userspace中该结构体的虚拟地址
 * @note 本函数需要获取系统中空闲的内存数量以及正在运行的
 * 进程数目，然后拷贝到用户的sysinfo地址空间，这是一个虚拟地址
*/
uint64
sys_sysinfo(void) 
{
  // 获取用户空间的虚拟地址参数传入
  // va代表用户空间的虚拟地址
  uint64 va;
  if (argaddr(0, &va) < 0) return -1;
  
  //创建一个sysinfo结构体用于保存对应数据
  struct sysinfo info;

  //获取空闲内存和活跃进程
  info.freemem = getFreeMemo();
  info.nproc = getProcNum();

  struct proc* p = myproc();
  //然后把它们分别存储到用户空间的结构体中
  if (copyout(p->pagetable, va, (char*)&info, sizeof(info)) < 0) return -1;
  return 0; 
}
