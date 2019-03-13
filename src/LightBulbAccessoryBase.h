
#ifndef LightBulbAccessoryBase_hpp
#define LightBulbAccessoryBase_hpp

#include "homekit/HKAccessory.h"
#include "HAPAccessoryDescriptor.h"


typedef struct RgbColor
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RgbColor;

typedef struct HsvColor
{
    unsigned char h;
    unsigned char s;
    unsigned char v;
} HsvColor;

class LightBulbAccessoryBase: public HAPAccessoryDescriptor {
private:
    RgbColor color;
    RgbColor HsvToRgb(HsvColor hsv);
    HsvColor RgbToHsv(RgbColor rgb);
    
    void powerTrackable (bool oldValue, bool newValue, HKConnection *sender);
    void brightTrackable (int oldValue, int newValue, HKConnection *sender);
    std::string getLedHue (HKConnection *sender);
    void setLedHue (int oldValue, int newValue, HKConnection *sender);
    std::string getLedBrightness (HKConnection *sender);
    void setLedBrightness (int oldValue, int newValue, HKConnection *sender);
    std::string getLedSaturation (HKConnection *sender);
    void setLedSaturation (int oldValue, int newValue, HKConnection *sender);
    void lightIdentify(bool oldValue, bool newValue, HKConnection *sender);
    virtual void updateColor(RgbColor color) = 0;
public:
    virtual void initAccessorySet();
    
    virtual int getDeviceType(){
        return deviceType_lightBulb;
    }
    virtual void handle(){
        
    }
};
#endif
