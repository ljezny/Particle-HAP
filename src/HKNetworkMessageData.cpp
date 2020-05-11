#include "HKNetworkMessageData.h"
#include "HKStringUtils.h"

HKNetworkMessageData & HKNetworkMessageData::operator=(const HKNetworkMessageData &data) {
    count = data.count;
    for (int i = 0; i < 10; i++) {
        if (data.records[i].length) {
            records[i] = data.records[i];
            //records[i].data = new char[records[i].length];
            //bcopy(data.records[i].data, records[i].data, data.records[i].length);
        }
    }
    return *this;
}

HKNetworkMessageData::HKNetworkMessageData(const char *rawData, unsigned short len) {
    unsigned short delta = 0;
    while (delta < len) {
        int index = recordIndex(rawData[delta+0]);
        if (index < 0) {
            records[count].index = (rawData)[delta+0];
            records[count].length = (unsigned char)(rawData)[delta+1];
            records[count].data = new char[records[count].length];
            records[count].activate = true;
            bcopy(&rawData[delta+2], records[count].data, records[count].length);
            delta += (records[count].length+2);
            count++;
        } else {
            int newLen = ((unsigned char*)(rawData))[delta+1];
            newLen += records[index].length;
            char *ptr = new char[newLen];
            bcopy(records[index].data, ptr, records[index].length);
            bcopy(&rawData[delta+2], &ptr[records[index].length], newLen-records[index].length);
            delete [] records[index].data;
            records[index].data = ptr;

            delta += (newLen-records[index].length+2);
            records[index].length = newLen;

        }
    }
}
char *HKNetworkMessageData::dataPtrForIndex(unsigned char index) {
    int _index = recordIndex(index);
    if (_index >= 0)
        return records[_index].data;
    return 0;
}
unsigned int HKNetworkMessageData::lengthForIndex(unsigned char index) {
    int _index = recordIndex(index);
    if (_index >= 0)
        return records[_index].length;
    return 0;
}
void HKNetworkMessageData::rawData(const char **dataPtr, unsigned short *len) {
    string buffer = "";
    for (int i = 0; i < 10; i++) {
        if (records[i].activate) {
            for (uint j = 0; j != records[i].length;) {
                unsigned char len = records[i].length-j>255?255:records[i].length-j;
                string temp(&records[i].data[j], len);
                temp = (char)records[i].index+((char)len+temp);
                buffer += temp;
                j += (unsigned int)len;
            }
        }
    }
    *len = buffer.length();
    *dataPtr = new char[*len];
    bcopy(buffer.c_str(), (void *)*dataPtr, *len);
}

int HKNetworkMessageData::recordIndex(unsigned char index) {
    for (int i = 0; i < count; i++) {
        if (records[i].activate&&records[i].index==index) return i;
    }
    return -1;
}

void HKNetworkMessageData::addRecord(HKNetworkMessageDataRecord& record) {
    records[count] = record;
    count++;
}
