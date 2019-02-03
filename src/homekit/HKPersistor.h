#ifndef HKPersistor_H
#define HKPersistor_H

#ifdef PARTICLE_COMPAT
#include "../../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif

#include "HKConfig.h"
#include "HKNetworkMessageDataRecord.h"

#include <stdio.h>
#include <vector>

struct HKKeyRecord {
    char controllerID[36];
    char publicKey[32];
};

struct HKStorage {
    unsigned char pairingsCount;
    HKKeyRecord pairings[MAX_PAIRINGS];
};

class HKPersistor {
private:
  HKStorage *storage = new HKStorage();
public:
  void resetPersistor();
  bool addKey(HKKeyRecord record);
  void removeKey(HKKeyRecord record);
  int keyIndex(HKKeyRecord record);
  HKKeyRecord getKey(char key[32]);

  void loadRecordStorage();
  void saveSaveStorage();
};
#endif
