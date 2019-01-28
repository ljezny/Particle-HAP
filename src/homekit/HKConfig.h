#ifndef HKConfig_H
#define HKConfig_H
#include "crypto/types.h"
#include "HKConsts.h"

#define TCP_SERVER_PORT 5115
#define MAX_PAIRINGS 16
#define EEPROM_STORAGE_ADDRESS_OFFSET 512 //if you are using EEPROM, you can set offset for Homekit to store pairings

#define deviceIdentity "5D:C6:5E:50:CE:38"  //ID
#define deviceName "particl8" //deviceName
#define devicePassword "523-12-643" //Password

#define CUSTOM_RAND_GENERATE_BLOCK customRngFunc
extern int customRngFunc(byte* output, word32 sz);



#endif
