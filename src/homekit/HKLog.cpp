//
//  HKLog.cpp
//  HKTester
//
//  Created by Lukas Jezny on 06/02/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#include "HKLog.h"
#include <stdlib.h>
#include "HKStringUtils.h"
HKLog HKLogger;

int HKLog::printf(const std::string fmt, ...){
    return 0;

    int size = ((int)fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
    std::string str;
    va_list ap;
    while (1) {     // Maximum two passes on a POSIX system...
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {  // Everything worked
            str.resize(n);
            break;
        }
        if (n > -1)  // Needed size returned
            size = n + 1;   // For null char
        else
            size *= 2;      // Guess at a larger size (OS specific)
    }
    Serial.println(str.c_str());
    return 0;
}

int HKLog::println(const std::string value) {
    return 0;
    Serial.println(value.c_str());
    return 0;
}
