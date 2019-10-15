#include "LightBulbAccessoryBase.h"


#include "HKConnection.h"
#include "HKLog.h"


#include <set>


#include <Particle.h>


std::string LightBulbAccessoryBase::getPower (HKConnection *sender) {
    return (color.r > 0 || color.g > 0 || color.b > 0) ? "true" : "false";
}


void LightBulbAccessoryBase::powerTrackable (bool oldValue, bool newValue, HKConnection *sender) {
    color.r = newValue ? (color.r > 0 ? color.r : 255) : 0;
    color.g = newValue ? (color.g > 0 ? color.g : 255) : 0;
    color.b = newValue ? (color.b > 0 ? color.b : 255) : 0;

    updateColor(color);

    if(brightnessStateChar) {
      brightnessStateChar->notify(NULL);
    }
}


std::string LightBulbAccessoryBase::getLedHue (HKConnection *sender) {
    return format("%d",((RgbToHsv(color).h) * 360) / 255);
}

void LightBulbAccessoryBase::setLedHue (int oldValue, int newValue, HKConnection *sender) {
    HsvColor hsv = RgbToHsv(color);
    hsv.h = (255 * newValue) / 360;
    color = HsvToRgb(hsv);
    updateColor(color);
}

std::string LightBulbAccessoryBase::getLedBrightness (HKConnection *sender) {
    return format("%d",((RgbToHsv(color).v) * 100) / 255);
}

void LightBulbAccessoryBase::setLedBrightness (int oldValue, int newValue, HKConnection *sender) {
    HsvColor hsv = RgbToHsv(color);
    hsv.v = (255 * newValue) / 100;
    color = HsvToRgb(hsv);
    updateColor(color);
}

std::string LightBulbAccessoryBase::getLedSaturation (HKConnection *sender) {
    return format("%d",((RgbToHsv(color).s) * 100) / 255);
}

void LightBulbAccessoryBase::setLedSaturation (int oldValue, int newValue, HKConnection *sender) {
    HsvColor hsv = RgbToHsv(color);
    hsv.s = (255 * newValue) / 100;
    color = HsvToRgb(hsv);
    updateColor(color);
}

void LightBulbAccessoryBase::lightIdentify(bool oldValue, bool newValue, HKConnection *sender) {
    hkLog.info("Start Identify Light\n");
}

void LightBulbAccessoryBase::initAccessorySet() {
    Accessory *lightAcc1 = new Accessory();

    //Add Light
    AccessorySet *accSet = &AccessorySet::getInstance();
    addInfoServiceToAccessory(lightAcc1, "Bulb name", "Vendor name", "Model name", "1","1.0.0", std::bind(&LightBulbAccessoryBase::lightIdentify, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3));
    accSet->addAccessory(lightAcc1);

    Service *lightService1 = new Service(serviceType_lightBulb);
    lightAcc1->addService(lightService1);

    stringCharacteristics *lightServiceName1 = new stringCharacteristics(charType_serviceName, permission_read, 0);
    lightServiceName1->characteristics::setValue("Bulb name");
    lightAcc1->addCharacteristics(lightService1, lightServiceName1);

    boolCharacteristics *powerState1 = new boolCharacteristics(charType_on, permission_read|permission_write|permission_notify);
    powerState1->perUserQuery = std::bind(&LightBulbAccessoryBase::getPower, this, std::placeholders::_1);
    powerState1->valueChangeFunctionCall = std::bind(&LightBulbAccessoryBase::powerTrackable, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
    lightAcc1->addCharacteristics(lightService1, powerState1);

    brightnessStateChar = new intCharacteristics(charType_brightness, permission_read|permission_write, 0, 100, 1, unit_percentage);
    brightnessStateChar->perUserQuery = std::bind(&LightBulbAccessoryBase::getLedBrightness, this, std::placeholders::_1);
    brightnessStateChar->valueChangeFunctionCall = std::bind(&LightBulbAccessoryBase::setLedBrightness, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
    lightAcc1->addCharacteristics(lightService1, brightnessStateChar);

    intCharacteristics *ledSaturationStateChar = new intCharacteristics(charType_saturation, permission_read|permission_write|permission_notify, 0, 100, 1, unit_percentage);
    ledSaturationStateChar->perUserQuery = std::bind(&LightBulbAccessoryBase::getLedSaturation, this, std::placeholders::_1);
    ledSaturationStateChar->valueChangeFunctionCall = std::bind(&LightBulbAccessoryBase::setLedSaturation, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
    lightAcc1->addCharacteristics(lightService1, ledSaturationStateChar);

    intCharacteristics *ledHueStateChar = new intCharacteristics(charType_hue, permission_read|permission_write|permission_notify, 0, 360, 1, unit_arcDegree);
    ledHueStateChar->perUserQuery = std::bind(&LightBulbAccessoryBase::getLedHue, this, std::placeholders::_1);
    ledHueStateChar->valueChangeFunctionCall = std::bind(&LightBulbAccessoryBase::setLedHue, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);
    lightAcc1->addCharacteristics(lightService1, ledHueStateChar);

    updateColor(color);
};

RgbColor LightBulbAccessoryBase::HsvToRgb(HsvColor hsv)
{
    RgbColor rgb;
    unsigned char region, remainder, p, q, t;

    if (hsv.s == 0)
    {
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
        return rgb;
    }

    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6;

    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            rgb.r = hsv.v; rgb.g = t; rgb.b = p;
            break;
        case 1:
            rgb.r = q; rgb.g = hsv.v; rgb.b = p;
            break;
        case 2:
            rgb.r = p; rgb.g = hsv.v; rgb.b = t;
            break;
        case 3:
            rgb.r = p; rgb.g = q; rgb.b = hsv.v;
            break;
        case 4:
            rgb.r = t; rgb.g = p; rgb.b = hsv.v;
            break;
        default:
            rgb.r = hsv.v; rgb.g = p; rgb.b = q;
            break;
    }

    return rgb;
}

HsvColor LightBulbAccessoryBase::RgbToHsv(RgbColor rgb)
{
    HsvColor hsv;
    unsigned char rgbMin, rgbMax;

    rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
    rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

    hsv.v = rgbMax;
    if (hsv.v == 0)
    {
        hsv.h = 0;
        hsv.s = 0;
        return hsv;
    }

    hsv.s = 255 * long(rgbMax - rgbMin) / hsv.v;
    if (hsv.s == 0)
    {
        hsv.h = 0;
        return hsv;
    }

    if (rgbMax == rgb.r)
        hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
    else if (rgbMax == rgb.g)
        hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
    else
        hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

    return hsv;
}
