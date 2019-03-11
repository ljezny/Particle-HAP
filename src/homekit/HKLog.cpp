//
//  HKLog.cpp
//  HKTester
//
//  Created by Lukas Jezny on 06/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "HKLog.h"

HKLog HKLogger;

int HKLog::printf(const char *format, ...){
  return 0;
    char* output = new char[4096];
    memset(output,0,4096);
    va_list argptr;
    va_start(argptr, format);
    vsprintf(output,format,argptr);
    Serial.println(output);
    va_end(argptr);

    free(output);

    return 0;

}

int HKLog::println(const char *value) {
  return 0;
    Serial.println(value);
    return 0;
}
