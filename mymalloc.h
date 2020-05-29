// malloc.h
#ifndef MYMALLOC_H
#define MYMALLOC_H

void* malloc(size_t size);

void free(void* ptr);

void* calloc(size_t nmemb, size_t size);

void* realloc(void* ptr, size_t size);

#endif /* MYMALLOC_H */
