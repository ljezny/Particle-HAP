#ifndef HKConfig_H
#define HKConfig_H
#include "crypto/types.h"
#include "HKConsts.h"

//TCP for handling server port
#define TCP_SERVER_PORT 5556
//Maximum pairings available. Apple recommends 16. But be aware, one pairings needs (36+32)B of your EEPROM
#define MAX_PAIRINGS 16
//if you are using EEPROM, you can set offset for Homekit to store pairings
#define EEPROM_STORAGE_ADDRESS_OFFSET 512

//ID of device, must be unique per device
#define deviceIdentity "5D:C6:5E:50:CE:38"
//Device name, which will be visible during pairing. Should be unique per device, so user can distinguish devices.
#define deviceName "particle"
//Pairing code, should be unique per device
#define devicePassword "523-12-643"


#define CUSTOM_RAND_GENERATE_BLOCK customRngFunc
extern int customRngFunc(byte* output, word32 sz);

#endif
