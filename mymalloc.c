// mymalloc.c
/* dynamic memory allocation interface for user tasks */
#include <stddef.h>
#include <string.h>
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

static struct header{
    size_t size; // size includes header; thus size is always a power of 2
    struct header* next;
};
static struct header free_list_head = {.size=0x8000, .next=NULL}; // size_t is 2 bytes

static inline size_t pow_of_2_ceiling(size_t size){
    // this definitely could be written better...
    unsigned char power = 0;
    size_t size_cpy = size;
    while(1){
        size_cpy >>= 1;
        if(size_cpy){
            power++;
        }
        else{
            break;
        }
    }
    if(1<<power == size){
        // was already perfect power of two
        return size;
    }
    else{
        return ((size_t)1)<<(power+1);
    }
}

void* malloc(size_t size){
    // round up size to power of 2
    size_t fixedSize = pow_of_2_ceiling(sizeof(header) + size);
    for(unsigned char n=0; /* NOTICE HOW THERE ARE NO CHECKS */; n++){
        if((struct header)(myheap_start + n*fixedSize).size == 0){
            // struct header.size == 0 means block not used
            (struct header)(myheap_start + n*fixedSize).size = fixedSize;
            return myheap_start + n*fixedSize + sizeof(struct header);
        }
    }
}

void free(void* ptr){
    char* realPtr = (char*)ptr - sizeof(struct header);
    size_t size = ((struct header)realPtr).size;
    char* buddyPtr = ((realPtr-myheap_start)^size)+myheap_start;
    ((struct header)realPtr).size = 0;
}

void* calloc(size_t nmemb, size_t size){
    if(nmemb == 0 || size == 0){
        return NULL;
    }
    else{
        size_t totalSize = nmemb*size;
        void* ret = malloc(totalSize);
        // would really prefer bzero :(
        memset(ret, 0, totalSize);
        return ret;
    }
}

void* realloc(void* ptr, size_t size){
    if(ptr == NULL){
        return malloc(size);
    }
    else if(ptr != NULL && size == 0){
        free(ptr);
        return NULL;
    }
    else{
        return NULL;
    }
}
