#ifndef  HKNetworkMessage_H
#define HKNetworkMessage_H

#include "HKConfig.h"
#include "HKNetworkMessageData.h"

class HKNetworkMessage {
    char method[5];
    char type[40];
public:
    char directory[20];
    HKNetworkMessageData data;
    HKNetworkMessage(const char *rawData);
    void getBinaryPtr(char **buffer, int *contentLength);
};
#endif
