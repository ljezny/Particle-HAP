#ifndef  HKNetworkMessage_H
#define HKNetworkMessage_H

#include "HKConfig.h"
#include "HKNetworkMessageData.h"

class HKNetworkMessage {
    char method[15];
    char type[64];
public:
    char directory[128];
    HKNetworkMessageData data;
    HKNetworkMessage(const char *rawData);
    void getBinaryPtr(char **buffer, int *contentLength);
};
#endif
