
#ifndef LightBulbAccessoryBase_hpp
#define LightBulbAccessoryBase_hpp

#include "HKAccessory.h"
#include "HAPAccessoryDescriptor.h"


typedef struct RgbColor
{
    unsigned char r = 255;
    unsigned char g = 255;
    unsigned char b = 255;
} RgbColor;

typedef struct HsvColor
{
    unsigned char h;
    unsigned char s;
    unsigned char v;
} HsvColor;

class LightBulbAccessoryBase: public HAPAccessoryDescriptor {
private:
    RgbColor color; //initially turned ON
    RgbColor HsvToRgb(HsvColor hsv);
    HsvColor RgbToHsv(RgbColor rgb);

    intCharacteristics *brightnessStateChar = NULL;

    std::string getPower (HKConnection *sender);
    void powerTrackable (bool oldValue, bool newValue, HKConnection *sender);
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
    virtual bool handle(){
        return false;
    }
};
#endif
