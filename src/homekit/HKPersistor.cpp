#include "HKPersistor.h"
#include "HKStringUtils.h"
void HKPersistor::loadRecordStorage() {
  Serial.println("Persistor: load");
  EEPROM.get(EEPROM_STORAGE_ADDRESS_OFFSET, *storage);
  if(storage->pairingsCount == 0xFF) {
    resetPersistor();
  }
}

void HKPersistor::saveSaveStorage(){
  Serial.println("Persistor: save");
  EEPROM.put(EEPROM_STORAGE_ADDRESS_OFFSET, *storage);
}

void HKPersistor::resetPersistor() {
  Serial.println("Persistor: reset");
  memset(storage,0,sizeof(HKStorage));
  saveSaveStorage();
}

bool HKPersistor::addKey(HKKeyRecord record) {
    byte emptyRecord[32];
    memset(emptyRecord,0,32);
    Serial.println("Persistor: adding key");
    for(int i = 0; i<MAX_PAIRINGS; i++) { //find first empty slot
      if (bcmp(storage->pairings[i].controllerID, emptyRecord, 32) == 0) {
        memcpy(&storage->pairings[i],&record,sizeof(HKKeyRecord));
        storage->pairingsCount += 1;
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
        memset(&storage->pairings[index],0,sizeof(HKKeyRecord));
        saveSaveStorage();
    }
}

int HKPersistor::keyIndex(HKKeyRecord record) {
  Serial.println("Persistor: key exists");
  for (unsigned char i = 0; i < MAX_PAIRINGS; i++) {
    if (bcmp(storage->pairings[i].controllerID, record.controllerID, 32) == 0) {
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
    if (bcmp(storage->pairings[i].controllerID, controllerID, 32) == 0) {
      Serial.println("Persistor: key found");
      return storage->pairings[i];
    }
  }
  Serial.println("Persistor: key not found");
  HKKeyRecord emptyRecord;
  bzero(emptyRecord.controllerID, 32);
  return emptyRecord;
}
