#ifndef  HKNetworkResponse_H
#define HKNetworkResponse_H

#include "HKConfig.h"
#include "HKNetworkMessageData.h"

class HKNetworkResponse {
    unsigned short responseCode;
    string responseType();
public:
    HKNetworkMessageData data;
    HKNetworkResponse(unsigned short _responseCode);
    void getBinaryPtr(char **buffer, int *contentLength);
};
#endif
