#ifndef  HKNetworkMessageData_H
#define HKNetworkMessageData_H


#include "HKConfig.h"
#include "HKNetworkMessageDataRecord.h"

#include <string>

using std::string;

class HKNetworkMessageData {
private:
    HKNetworkMessageDataRecord records[10];
public:
    unsigned char count = 0;
    HKNetworkMessageData() {}
    HKNetworkMessageData(const char *rawData, unsigned short len);
    HKNetworkMessageData(const HKNetworkMessageData &data);
    HKNetworkMessageData & operator=(const HKNetworkMessageData &);
    void rawData(const char **dataPtr, unsigned short *len);
    void addRecord(HKNetworkMessageDataRecord& record);
    int recordIndex(unsigned char index);
    char *dataPtrForIndex(unsigned char index);
    unsigned int lengthForIndex(unsigned char index);
};
#endif
