//
//  PhotoSystemLEDLightBulbAccessory.hpp
//  HKTester
//
//  Created by Lukas Jezny on 13/03/2019.
//  Copyright © 2019 Lukas Jezny. All rights reserved.
//

#ifndef PhotoSystemLEDLightBulbAccessory_hpp
#define PhotoSystemLEDLightBulbAccessory_hpp

#include "LightBulbAccessoryBase.h"
#include <stdio.h>

class PhotonSystemLEDLightBulbAccessory: public LightBulbAccessoryBase {
private:
public:
    virtual void updateColor(RgbColor color);
    
};

#endif /* PhotoSystemLEDLightBulbAccessory_hpp */
