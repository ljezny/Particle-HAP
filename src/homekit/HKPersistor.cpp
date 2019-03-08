#include "HKPersistor.h"
#include "HKStringUtils.h"
void HKPersistor::loadRecordStorage() {
    Serial.println("Persistor: load");
    
    storage = EEPROM.get(EEPROM_STORAGE_ADDRESS_OFFSET, storage);
    if(((unsigned char *)&storage)[0] == 0xFF) { //particle eeprom default value
        resetAll();
    }
}

void HKPersistor::saveSaveStorage(){
    Serial.println("Persistor: save");
    EEPROM.put(EEPROM_STORAGE_ADDRESS_OFFSET, storage);
}

void HKPersistor::resetAll() {
    Serial.println("Persistor: resetAll");
    for(int i = 0; i < 6; i++){
        storage.deviceId[i] = random(256);
    }
    resetPairings();
}

void HKPersistor::resetPairings() {
    Serial.println("Persistor: resetPairings");
    for(int i = 0; i<MAX_PAIRINGS; i++) { //find first empty slot
       memset(&storage.pairings[i],0,sizeof(HKKeyRecord));
    }
    saveSaveStorage();
}

bool HKPersistor::addKey(HKKeyRecord record) {
    byte emptyRecord[32];
    memset(emptyRecord,0,32);
    Serial.println("Persistor: adding key");
    for(int i = 0; i<MAX_PAIRINGS; i++) { //find first empty slot
        if (bcmp(storage.pairings[i].controllerID, emptyRecord, 32) == 0) {
            memcpy(&storage.pairings[i],&record,sizeof(HKKeyRecord));
            saveSaveStorage();
            Serial.println("Persistor: key added");
            return true;
        }
    }
    return false;
}

void HKPersistor::removeKey(HKKeyRecord record) {
    int index = keyIndex(record);
    if (index != -1) {
        memset(&storage.pairings[index],0,sizeof(HKKeyRecord));
        saveSaveStorage();
    }
}

int HKPersistor::keyIndex(HKKeyRecord record) {
    Serial.println("Persistor: key exists");
    for (unsigned char i = 0; i < MAX_PAIRINGS; i++) {
        if (bcmp(storage.pairings[i].controllerID, record.controllerID, 32) == 0) {
            Serial.println("Persistor: key found");
            return i;
        }
    }
    Serial.println("Persistor: key not found");
    return -1;
}

HKKeyRecord HKPersistor::getKey(char controllerID[32]) {
    Serial.println("Persistor: key get");
    for (unsigned char i = 0; i < MAX_PAIRINGS; i++) {
        if (bcmp(storage.pairings[i].controllerID, controllerID, 32) == 0) {
            Serial.println("Persistor: key found");
            return storage.pairings[i];
        }
    }
    Serial.println("Persistor: key not found");
    HKKeyRecord emptyRecord;
    bzero(emptyRecord.controllerID, 32);
    return emptyRecord;
}
