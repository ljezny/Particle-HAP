#ifndef  HKStringUtils_H
#define HKStringUtils_H

#include "stdio.h"
#include <string>
#include <vector>
#include <stdlib.h>
#include <cstdio>

const char *copyLine(const char *rawData, char *destination);
const char *skipTillChar(const char *ptr, const char target);
inline void int2str(int i, char *s) { sprintf(s,"%d",i); }
void print_hex_memory(void *mem, int count);
std::string format(const std::string format, ...);
#endif
