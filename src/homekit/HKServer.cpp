#include "HKServer.h"
#include "HKConnection.h"
#include "HKLog.h"

#ifndef PARTICLE_COMPAT
#include "spark_wiring_tcpclient.h"
#include "spark_wiring_tcpserver.h"
#include "spark_wiring_network.h"
#include "spark_wiring_thread.h"
#endif

//TCP for handling server port
static int TCP_SERVER_PORT = 5556;



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

HKServer::~HKServer() {
    delete persistor;
    hkLog.info("Server destructed");
}

void HKServer::start () {
    TCP_SERVER_PORT++;
    server = new TCPServer(TCP_SERVER_PORT);
    server->begin();
    hkLog.info("Server started at port %d", TCP_SERVER_PORT);
    bonjour.setUDP( &udp );
    setPaired(false);

    Particle.variable("connections", &this->connections, INT);
}

void HKServer::stop () {
    for(int i = 0; i < clients.size(); i++) {
      HKConnection *conn = clients.at(i);
      conn->close();
      delete conn;
    }
    clients.clear();

    server->stop();
    delete server;
    server = NULL;

    hkLog.info("Server stopped");
    paired = false;
}

void HKServer::setPaired(bool p) {
    if(paired) {
        return;
    }
    unsigned short configNumber = persistor->getAndUpdateConfigurationVersion();
    paired = p;

    bonjour.removeAllServiceRecords();

    char* configNumberStr = new char[32];
    memset(configNumberStr, 0, 6);
    int configNumberLen = sprintf(configNumberStr, "%d",configNumber);

    char* deviceTypeStr = new char[6];
    memset(deviceTypeStr, 0, 6);
    sprintf(deviceTypeStr, "%d",deviceType);

    char* recordTxt = new char[512];
    memset(recordTxt, 0, 512);
    int len = sprintf(recordTxt, "%csf=%d%cid=%s%cpv=1.0%cc#=%s%cs#=1%cff=0%cmd=%s%cci=%s",4, p ? 0 : 1,(char)deviceIdentity.length()+3,deviceIdentity.c_str(),6,3+configNumberLen,configNumberStr,4,4,(char)hapName.length() + 3,hapName.c_str(),3 + strlen(deviceTypeStr),deviceTypeStr);

    char* bonjourName = new char[128];
    memset(bonjourName, 0, 128);

    sprintf(bonjourName, "%s._hap",hapName.c_str());

    bonjour.addServiceRecord(bonjourName,
                             TCP_SERVER_PORT,
                             MDNSServiceTCP,
                             recordTxt);

    free(deviceTypeStr);
    free(recordTxt);
    free(bonjourName);
}

bool HKServer::handle() {
    bool result = false;
    bonjour.begin(hapName.c_str());
    result |= bonjour.run();
    bonjour.stop();

    TCPClient newClient = server->available();
    if(newClient) {
        hkLog.info("Client connected.");
        clients.insert(clients.begin(),new HKConnection(this,newClient));
        Particle.publish("homekit/accept", String(newClient.remoteIP()), PUBLIC);
        result |= true;
    }

    int i = clients.size() - 1;
    while(i >= 0) {
        HKConnection *conn = clients.at(i);

        result |= conn->handleConnection();
        if(!conn->isConnected()) {
            hkLog.info("Client removed.");
            Particle.publish("homekit/close", String(conn->remoteIP()), PUBLIC);
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
