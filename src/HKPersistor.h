#ifndef HKPersistor_H
#define HKPersistor_H

#ifdef PARTICLE_COMPAT
#include "../utils/HKTester/HKTester/Particle_Compat/particle_compat.h"
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
    unsigned char deviceId[6];
    HKKeyRecord pairings[MAX_PAIRINGS];
    unsigned short configurationVersion = 1;
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

    const unsigned char* getDeviceId() {
        return storage.deviceId;
    }

    unsigned short getAndUpdateConfigurationVersion() {
        unsigned short v = storage.configurationVersion++;
        saveSaveStorage();
        return v;
    }

};
#endif
