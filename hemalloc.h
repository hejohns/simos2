// malloc.h
#ifndef HEMALLOC_H
#define HEMALLOC_H

#include <stddef.h>

void* malloc(size_t size) __attribute__((malloc));

void free(void* ptr);

void* calloc(size_t nmemb, size_t size) __attribute__((malloc));

void* realloc(void* ptr, size_t size) __attribute__((malloc));

#endif /* HEMALLOC_H */
