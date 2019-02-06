//
//  HKLog.cpp
//  HKTester
//
//  Created by Lukas Jezny on 06/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "HKLog.h"

int HKLog::printf(const char *format, ...){
    va_list argptr;
    va_start(argptr, format);
    Serial.printf(format,argptr);
    va_end(argptr);
    return 0;
}

int HKLog::println(const char *value) {
    Serial.println(value);
    return 0;
}
