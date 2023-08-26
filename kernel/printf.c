//
// formatted console output -- printf, panic.
//

#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

volatile int panicked = 0;

// lock to avoid interleaving concurrent printf's.
static struct {
  struct spinlock lock;
  int locking;
} pr;

static char digits[] = "0123456789abcdef";

static void
printint(int xx, int base, int sign)
{
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i]);
}

static void
printptr(uint64 x)
{
  int i;
  consputc('0');
  consputc('x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    consputc(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

// Print to the console. only understands %d, %x, %p, %s.
void
printf(char *fmt, ...)
{
  va_list ap;
  int i, c, locking;
  char *s;

  locking = pr.locking;
  if(locking)
    acquire(&pr.lock);

  if (fmt == 0)
    panic("null fmt");

  va_start(ap, fmt);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printint(va_arg(ap, int), 10, 1);
      break;
    case 'x':
      printint(va_arg(ap, int), 16, 1);
      break;
    case 'p':
      printptr(va_arg(ap, uint64));
      break;
    case 's':
      if((s = va_arg(ap, char*)) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }

  if(locking)
    release(&pr.lock);
}

void
panic(char *s)
{
  pr.locking = 0;
  printf("panic: ");
  printf(s);
  printf("\n");
  panicked = 1; // freeze uart output from other CPUs
  for(;;)
    ;
}

void
printfinit(void)
{
  initlock(&pr.lock, "pr");
  pr.locking = 1;
}

<<<<<<< HEAD
/**
 * @note 此函数的作用在于发生错误时，能够从栈帧的错误点
 * @note 向上追溯，自底向上(从小地址到大地址)打印各个函
 * @note 数栈帧的return address，反映函数调用情况
 * @note 例如：sum_then_double [call] sum_to，则当sum_to
 * @note 出现错误时，能够打印出：
 * @note ①sum_to到sum_then_double的return address
 * @note ②还能够打印出sum_then_double到main的return address
*/
void
backtrace(void)
{
  //一个用户进程的栈大小为PGSIZE = 4096
  //获得当前函数的frame pointer，它指向栈的顶部(在os中栈顶是数据结构中的“栈底”)
  uint64 fp = r_fp();
  //获得这个栈的地址最大值(位于栈所在地址空间的最上方)
  //注意：栈在该地址空间中是向下增长的
  uint64 stack_end = PGROUNDUP(fp); 
  printf("backtrace:\n");
  //! \bug 此处fp <= stack_end 会多一次循环：即fp == stack_end的情况
  //! \bug scause = 0x000000000000000d，会导致painc错误，或许不需要获得main的调用情况
  //! \bug 错误的返回地址 0x0000000000000012, 18
  while (fp < stack_end) {
    printf("%p\n", *(uint64*)(fp - 8));
    fp = *(uint64*)(fp - 16);
  }
}
=======
void 
backtrace(void)
{
  uint64 cur_fp = r_fp();
  uint64 stack_bottom = PGROUNDUP(cur_fp);
  printf("backtrace\n");

  while (cur_fp < stack_bottom) {
    printf("%p\n",*(uint64 *)(cur_fp-8));
    cur_fp = *(uint64 *)(cur_fp-16);
  }
  return;
}
>>>>>>> 2badc23d7ebd4f43062cce51b62ed204d9746f59
