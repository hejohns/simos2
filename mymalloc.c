// mymalloc.c
/* dynamic memory allocation interface for user tasks */
#include <stddef.h>
#include "mymalloc.h"

/* A very simple buddy? allocator
 *
 * Partially to reduce extreme fragmentation--
 * but mostly just cause this is an educational project
 */

// if I'm correct, __bss_end is a dummy compiler defined global variable 
// to serve as the address of the end of the .bss section. 
// We will start heap directly after &__bss_end. 
// If you wanted to move the heap to external ram, either 
// 1) move the bss section as well, or 2) change some stuff around here
// (The prior seems most logical)
extern char __bss_end;
static volatile char* myheap_start = (&__bss_end)+1;
static volatile char* myheap_end = mymalloc_start;
// __heap_start/__heap_end are also compiler defined... but I couldn't find it
// in the elf dump for this commit
// (granted I'm not using avr libc's malloc headers)

void* malloc(size_t size){
    return (void*)mymalloc_start;
}

void free(void* ptr){
}

void* calloc(size_t nmemb, size_t size){
    return (void*)0;
}

void* realloc(void* ptr, size_t size){
    return (void*)0;
}
