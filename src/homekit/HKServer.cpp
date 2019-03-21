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

HKServer::~HKServer() {
    delete persistor;
    hkLog.info("Server destructed");
}

void HKServer::start () {
    server.begin();
    hkLog.info("Server started at port %d", TCP_SERVER_PORT);

    bonjour.setUDP( &udp );
    bonjour.begin(hapName.c_str());
    setPaired(false);
}

void HKServer::stop () {
    server.stop();
    hkLog.info("Server stopped");
    bonjour.stop();
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

void HKServer::handle() {

    bonjour.run();

    TCPClient newClient = server.available();
    if(newClient) {
        hkLog.info("Client connected.");
        clients.insert(clients.begin(),new HKConnection(this,newClient));
    }

    int i = clients.size() - 1;
    while(i >= 0) {
        HKConnection *conn = clients.at(i);

        conn->handleConnection();
        if(!conn->isConnected()) {
            hkLog.info("Client removed.");

            conn->close();
            clients.erase(clients.begin() + i);
            delete conn;
        }

        i--;
    }


}
