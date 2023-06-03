// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

//不要加任何锁！
int page_ref_count[PHYSTOP/PGSIZE];

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

/*struct {
  struct spinlock lock;
  int count[PHYSTOP/PGSIZE];
} page_ref_count;*/

void
kinit()
{
  for (int i = 0; i < PHYSTOP/PGSIZE; i++) {
    page_ref_count[i] = 1;
  }
  initlock(&kmem.lock, "kmem");
  //initlock(&page_ref_count.lock, "page_ref_count");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");
  //注意：因为很多地方都需要用到kfree，所以这里直接在这里减去引用计数
  page_ref_count[(uint64)pa/PGSIZE]--;

  if (page_ref_count[(uint64)pa/PGSIZE] == 0) {
    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r) {
    kmem.freelist = r->next;
    page_ref_count[(uint64)r/PGSIZE] = 1;     
  }
  release(&kmem.lock);

  if(r){
     memset((char*)r, 5, PGSIZE); // fill with junk
  }

  return (void*)r;
}

//实验技巧：page_ref_count的+1封装
void increase1(uint64 pa) {
  page_ref_count[pa/PGSIZE] += 1;
  return;
}