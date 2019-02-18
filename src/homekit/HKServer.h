#ifndef  HKServer_H
#define HKServer_H
#include "HKConfig.h"
#include "HKBonjour.h"
#include "HKNetworkMessage.h"
#include "HKNetworkMessageData.h"
#include "HKPersistor.h"
#include "HKConfig.h"

#ifdef PARTICLE_COMPAT
#include "../../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif

#include <vector>

class HKConnection;

class HKServer {
private:
    TCPServer server = TCPServer(TCP_SERVER_PORT);
    HKBonjour bonjour;
    UDP udp;
    bool paired = false;
    std::vector<HKConnection*> clients;
    int deviceType;
    const char *hapName;
    const char *deviceIdentity;
    const char *passcode;
    
public:
    HKPersistor *persistor;
    void (*progressPtr)(Progress_t);
    HKServer(int deviceType, const char* hapName, const char *passcode, void (*progressPtr)(Progress_t));
    void setup ();
    void handle();
    const char* getDeviceIdentity() {
        return deviceIdentity;
    }
    const char* getPasscode() {
        return passcode;
    }
};
#endif /* end of include guard: HKServer */
