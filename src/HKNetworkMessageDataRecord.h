#ifndef  HKNetworkMessageDataRecord_H
#define HKNetworkMessageDataRecord_H

#include "HKConfig.h"

class HKNetworkMessageDataRecord {
private:
public:
    unsigned char index = 0;
    char *data = 0;
    unsigned int length = 0;
    bool activate = false;
    ~HKNetworkMessageDataRecord();
    HKNetworkMessageDataRecord &operator=(const HKNetworkMessageDataRecord&);
};

#endif
