#include "HKConfig.h"

#ifdef PARTICLE_COMPAT
#include "../../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif


int customRngFunc(byte* output, word32 sz) {
  for(int i = 0; i<sz; i++) {
    output[i] = rand();
  }
  return 0;
}
