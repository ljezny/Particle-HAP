#include "HKConfig.h"

#ifdef PARTICLE_COMPAT
#include "../utils/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif


int customRngFunc(byte* output, word32 sz) {
  for(uint i = 0; i<sz; i++) {
    output[i] = rand();
  }
  return 0;
}
