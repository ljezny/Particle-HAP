#ifndef  HKStringUtils_H
#define HKStringUtils_H

#include <stdio.h>
#include <Particle.h>
const char *copyLine(const char *rawData, char *destination);
const char *skipTillChar(const char *ptr, const char target);
inline void int2str(int i, char *s) { sprintf(s,"%d",i); }
void print_hex_memory(void *mem, int count);
#endif
