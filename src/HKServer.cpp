#include "HKServer.h"
#include "HKConnection.h"
#include "HKLog.h"

#ifndef PARTICLE_COMPAT
#include "spark_wiring_tcpclient.h"
#include "spark_wiring_tcpserver.h"
#include "spark_wiring_network.h"
#include "spark_wiring_thread.h"
#endif

HKServer::HKServer(int deviceType, std::string hapName,std::string passcode,void (*progressPtr)(Progress_t)) {
    this->hapName = hapName;
    this->deviceType = deviceType;
    this->passcode = passcode;
    this->progressPtr = progressPtr;
    persistor = new HKPersistor();
    persistor->loadRecordStorage();

    char *deviceIdentity = new char[12+5];
    const unsigned char *deviceId = persistor->getDeviceId();
    sprintf(deviceIdentity, "%02X:%02X:%02X:%02X:%02X:%02X",deviceId[0],deviceId[1],deviceId[2],deviceId[3],deviceId[4],deviceId[5]);

    this->deviceIdentity = deviceIdentity; //std::string will copy
    free(deviceIdentity);
}

int getPort() {
    return TCP_SERVER_PORT;
}

HKServer::~HKServer() {
    delete persistor;
    hkLog.info("Server destructed");
}

void HKServer::start () {
    server.begin();
    hkLog.info("Server started at port %d", TCP_SERVER_PORT);
    bonjour.setUDP( &udp );
    setPaired(false);

    Particle.variable("connections", &this->connections, INT);
}

void HKServer::stop () {
    for(uint i = 0; i < clients.size(); i++) {
      HKConnection *conn = clients.at(i);
      conn->close();
      delete conn;
    }
    clients.clear();

    server.stop();

    hkLog.info("Server stopped");
    paired = false;
}

void HKServer::setPaired(bool p) {
    if(paired) {
        return;
    }
    unsigned short configNumber = persistor->getAndUpdateConfigurationVersion();
    paired = p;

    bonjour.removeServiceRecord();

    char configNumberStr[32] = {0};
    int configNumberLen = sprintf(configNumberStr, "%d",configNumber);

    char deviceTypeStr[6] = {0};
    sprintf(deviceTypeStr, "%d",deviceType);

    char recordTxt[160] = {0};
    sprintf(recordTxt, "%csf=%d%cid=%s%cpv=1.0%cc#=%s%cs#=1%cff=0%cmd=%s%cci=%s",4, p ? 0 : 1,(char)deviceIdentity.length()+3,deviceIdentity.c_str(),6,3+configNumberLen,configNumberStr,4,4,(char)hapName.length() + 3,hapName.c_str(),3 + strlen(deviceTypeStr),deviceTypeStr);

    char bonjourName[128] = {0};

    sprintf(bonjourName, "%s._hap",hapName.c_str());
    bonjour.setBonjourName(bonjourName);
    bonjour.setServiceRecord(bonjourName,
                             TCP_SERVER_PORT,
                             MDNSServiceTCP,
                             recordTxt);
}

bool HKServer::handle() {
    bool result = false;
    bonjour.begin(hapName.c_str());
    result |= bonjour.run();
    bonjour.stop();

    TCPClient newClient = server.available();
    if(newClient) {
        hkLog.info("Client connected.");
        HKConnection *c = new HKConnection(this,newClient);
        int i = clients.size() - 1;
        while(i>=0) //delete all previously connected clients with same IP with current
        {
            HKConnection *conn = clients.at(i);
            if (!strcmp(conn->clientID(),c->clientID()))
            {
                conn->close();
                clients.erase(clients.begin() + i);
                delete conn;
            }
            i--;
        }
        clients.insert(clients.end(),c);
#ifdef DEBUG_PARTICLE_EVENTS
        Particle.publish("homekit/accept", c->clientID(), PRIVATE);
#endif
        result = true;
    }

    int i = clients.size() - 1;
    while(i >= 0) {
        HKConnection *conn = clients.at(i);

        result |= conn->handleConnection(i >= MAX_CONNECTIONS);

        if(!conn->isConnected()) {
            hkLog.info("Client removed.");
#ifdef DEBUG_PARTICLE_EVENTS
            Particle.publish("homekit/close", conn->clientID(), PRIVATE);
#endif
            conn->close();
            clients.erase(clients.begin() + i);
            delete conn;
            result |= true;
        }

        i--;
    }

    this->connections = clients.size();

    return result;
}
