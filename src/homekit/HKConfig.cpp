#include "HKConfig.h"
#include <Particle.h>

int customRngFunc(byte* output, word32 sz) {
  for(int i = 0; i<sz; i++) {
    output[i] = rand();
  }
  return 0;
}
