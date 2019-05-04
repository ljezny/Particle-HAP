#ifndef HKConfig_H
#define HKConfig_H
#include "crypto/types.h"
#include "HKConsts.h"


#define TCP_SERVER_PORT 5556

//Maximum pairings available. Apple recommends 16. But be aware, one pairings needs (36+32)B of your EEPROM
#define MAX_PAIRINGS 16

//Apple requires 8 connections at least and must accomodate new one. Photon can handle up to 10 connections, but one is cloud connection.
//So allow 8 connections, every 9th will be closed send HTTP 503 and closed.
#define MAX_CONNECTIONS 8

//if you are using EEPROM, you can set offset for Homekit to store pairings
#define EEPROM_STORAGE_ADDRESS_OFFSET 512

#define SHARED_REQUEST_BUFFER_LEN 1024 //buffer for incomming requests from Homekit controllers, should be enough
#define SHARED_RESPONSE_BUFFER_LEN 5000 //buffer for generating response JSON,TLV - can overflow when there're a lot of services and accessories
#define SHARED_TEMP_CRYPTO_BUFFER_LEN 1024+18+2 //1024 - is crypto chunk size, 18+2 is crypto headers

#define CUSTOM_RAND_GENERATE_BLOCK customRngFunc
extern int customRngFunc(byte* output, word32 sz);

#endif
