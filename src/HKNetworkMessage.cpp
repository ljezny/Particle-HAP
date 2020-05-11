#include "HKNetworkMessage.h"
#include "HKStringUtils.h"
HKNetworkMessage::HKNetworkMessage(const char *rawData) {
    strcpy(method, "POST");

    const char *ptr = rawData;
    for (int i = 0; (*ptr)!=0 && (*ptr)!=' '; ptr++, i++) {
        method[i] = (*ptr);
        method[i+1] = 0;
    } ptr+=2;

    //Copy message directory
    for (int i = 0; (*ptr)!=0 && (*ptr)!=' '; ptr++, i++) {
        directory[i] = (*ptr);
        directory[i+1] = 0;
    }

    ptr = skipTillChar(ptr, '\n');

    char buffer[1024];
    const char *dptr = ptr;
    for (int i = 0; i < 19; i++) {
        bzero(buffer, 1024);
        dptr = copyLine(dptr, buffer);
    }

    //Reject host
    if (strncmp(ptr, "Host", 4) == 0) {
        ptr = skipTillChar(ptr, '\n');
    }

    //Get the length of content
    //Skip to the content-type
    ptr = skipTillChar(ptr, ':'); ptr++;
    unsigned int dataSize = (unsigned int)strtol(ptr, (char **)&ptr, 10);

    //Get the type of content
    //Skip to the content-length
    ptr = skipTillChar(ptr, ':');  ptr+=2;
    for (int i = 0; (*ptr)!=0 && (*ptr)!='\r'; ptr++, i++) {
        type[i] = (*ptr);
        type[i+1] = 0;
    }
    ptr+=4;

    //Data
    data = HKNetworkMessageData(ptr, dataSize);
}

void HKNetworkMessage::getBinaryPtr(char **buffer, int *contentLength) {
    const char *_data; unsigned short dataSize;
    data.rawData(&_data, &dataSize);
    (*buffer) = new char[1024];
    (*contentLength) = snprintf((*buffer), 1024, "%s /%s HTTP/1.1\r\nContent-Length: %hu\r\nContent-Type: %s\r\n\r\n", method, directory, dataSize, type);
    for (int i = 0; i < dataSize; i++) {
        (*buffer)[*contentLength+i] = _data[i];
    }
    (*contentLength)+=dataSize;
    (*buffer)[*contentLength] = 0;
}
