#include "HKNetworkResponse.h"
#include "HKStringUtils.h"
HKNetworkResponse::HKNetworkResponse(unsigned short _responseCode) {
    responseCode = _responseCode;
}

string HKNetworkResponse::responseType() {
    switch (responseCode) {
        case 200:
            return "OK";
        default:
            return "";
    }
}

void HKNetworkResponse::getBinaryPtr(char **buffer, int *contentLength) {
    char tempCstr[5];   int2str(responseCode, tempCstr);
    string temp = "HTTP/1.1 ";
    temp += tempCstr+(" "+responseType());
    temp += "\r\nContent-Type: application/pairing+tlv8\r\nContent-Length: ";
    const char *dataPtr;    unsigned short dataLen;
    data.rawData(&dataPtr, &dataLen);
    int2str(dataLen, tempCstr);
    temp += tempCstr;
    temp += "\r\n\r\n";
    *buffer = new char[temp.length()+dataLen];
    bcopy(temp.c_str(), *buffer, temp.length());
    bcopy(dataPtr, &((*buffer)[temp.length()]), dataLen);
    *contentLength = temp.length()+dataLen;
    delete [] dataPtr;
}
