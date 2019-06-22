#include "HKNetworkMessageDataRecord.h"
#include <strings.h>
HKNetworkMessageDataRecord & HKNetworkMessageDataRecord::operator=(const HKNetworkMessageDataRecord& r) {
    index = r.index;
    activate = r.activate;
    length = r.length;
    if (data)
        free(data);
    data = new char[length];
    bcopy(r.data, data, length);
    return *this;
}

HKNetworkMessageDataRecord::~HKNetworkMessageDataRecord() {
    if (length)
        free(data);
}
