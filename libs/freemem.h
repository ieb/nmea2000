
#ifndef __FREEMEM_H__
#define __FREEMEM_H__

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif


#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>


extern char _end;
extern "C" char *sbrk(int i);
char *ramstart=(char *)0x20070000;
char *ramend=(char *)0x20088000;

typedef struct {
    int32_t dynamic_ram;
    int32_t static_ram;
    int32_t stack_used;
    int32_t free;
} meminfo_t;


int freeMemory(meminfo_t *info)
{
    char *heapend=sbrk(0);
    register char * stack_ptr asm ("sp");
    struct mallinfo mi=mallinfo();
    info->dynamic_ram = mi.uordblks;
    info->static_ram = &_end - ramstart;
    info->stack_used = ramend - stack_ptr;
    info->free = (stack_ptr - heapend) + mi.fordblks;
//    printf("\nDynamic ram used: %d\n",mi.uordblks);
//    printf("Program static ram used %d\n",&_end - ramstart); 
//    printf("Stack ram used %d\n\n",ramend - stack_ptr); 
//    printf("My guess at free mem: %d\n",stack_ptr - heapend + mi.fordblks);
    return info->free;
}


#endif