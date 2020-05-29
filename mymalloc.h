// malloc.h
#ifndef MYMALLOC_H
#define MYMALLOC_H

void* mymalloc(size_t size);

void myfree(void* ptr);

void* mycalloc(size_t nmemb, size_t size);

void* myrealloc(void* ptr, size_t size);

#endif /* MYMALLOC_H */
