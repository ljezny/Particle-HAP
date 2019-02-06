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
    u_int8_t deviceId[6];
    HKKeyRecord pairings[MAX_PAIRINGS];
};

class HKPersistor {
private:
    HKStorage storage = HKStorage();
public:
    void resetAll();
    void resetPairings();
    
    bool addKey(HKKeyRecord record);
    void removeKey(HKKeyRecord record);
    int keyIndex(HKKeyRecord record);
    HKKeyRecord getKey(char key[32]);
    
    void loadRecordStorage();
    void saveSaveStorage();
    
    const uint8_t* getDeviceId() {
        return storage.deviceId;
    }
    
};
#endif
