#include "HKStringUtils.h"
#include "HKLog.h"

#include <stdio.h>
#ifdef PARTICLE_COMPAT
#include "../utils/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif

const char *copyLine(const char *rawData, char *destination) {
    int i;
    for (i = 0; rawData[i] != '\r' && rawData[i] != 0; i++) {
        destination[i] = rawData[i];
    }
    i++;
    if (rawData[i] == '\n')
        return &rawData[i+1];
    else return &rawData[i];
}

const char *skipTillChar(const char *ptr, const char target) {
    for (; (*ptr)!=0&&(*ptr)!=target; ptr++);  ptr++;
    return ptr;
}

void print_hex_memory(void *mem, int count) {
  int i;
  unsigned char *p = (unsigned char *)mem;
  for (i=0; i < count; i++) {
    hkLog.info("0x%02x, ", p[i]);
    if ((i%16==0) && i)
      hkLog.info("\n");
  }
  hkLog.info("\n");
}

std::string format(const std::string fmt, ...) {
    int size = ((int)fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
    std::string str;
    va_list ap;
    while (1) {     // Maximum two passes on a POSIX system...
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {  // Everything worked
            str.resize(n);
            return str;
        }
        if (n > -1)  // Needed size returned
            size = n + 1;   // For null char
        else
            size *= 2;      // Guess at a larger size (OS specific)
    }
    return str;
}


