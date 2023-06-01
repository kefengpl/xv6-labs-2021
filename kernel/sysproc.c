#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
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


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  struct proc* p = myproc(); //get this process
  pagetable_t pagetable = p->pagetable;

  uint64 va, dstva;
  int npages;
  if(argaddr(0, &va)<0 || argint(1, &npages)<0 || argaddr(2, &dstva)<0){
    return -1;
  }

  int MAX_PAGES = 64;
  if(npages > MAX_PAGES){
    panic("Only support less than or equal to 64 npages.");
  }

  va = PGROUNDDOWN(va);
  uint64 bitmask = 0;
  uint64 left_i = 1;
  pte_t* the_pte;
  for(int i=0; i<npages; i++){
    left_i = 1 << i;
    if((the_pte = walk(pagetable, va, 0)) == 0){
      return -1;
    }
    if((*the_pte & PTE_A)){
      bitmask = bitmask | left_i;
      *the_pte = *the_pte & ~PTE_A;
    }
    va += PGSIZE;
  }
  //错误点在于：(char*)&bitmask这个参数传递有问题
  if(copyout(pagetable, dstva, (char*)&bitmask, sizeof(the_pte)) < 0){
    return -1;
  }
  return 0;
}
#endif

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