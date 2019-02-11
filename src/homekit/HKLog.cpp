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
    char output[512];

    va_list argptr;
    va_start(argptr, format);
    vsprintf(output,format,argptr);
    Serial.println(output);
    va_end(argptr);
    return 0;

}

int HKLog::println(const char *value) {
    Serial.println(value);
    return 0;
}
