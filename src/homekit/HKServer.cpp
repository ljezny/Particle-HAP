#include "HKServer.h"
#include "HKConnection.h"

#ifndef PARTICLE_COMPAT
#include "spark_wiring_tcpclient.h"
#include "spark_wiring_tcpserver.h"
#include "spark_wiring_network.h"
#include "spark_wiring_thread.h"
#endif

HKServer::HKServer() {
  persistor = new HKPersistor();

}
void HKServer::setup () {
  persistor->loadRecordStorage();
  server.begin();

  bonjour.setUDP( &udp );
  bonjour.begin(hapName);
  setPaired(false);

  Serial.println(WiFi.localIP());
}

void HKServer::setPaired(bool p) {
    paired = p;
    bonjour.removeAllServiceRecords();
    char recordTxt[512];
    memset(recordTxt, 0, 512);
    sprintf(recordTxt, "%csf=1%cid=%s%cpv=1.0%cc#=1%cs#=1%cs#=1%cff=0%cmd=%s%cci=%d",4,(char)strlen(deviceIdentity)+3,deviceIdentity,6,4,4,4,4,(char)(strlen(hapName) + 3),hapName,4,5);
    bonjour.addServiceRecord(hapName "._hap",
                             TCP_SERVER_PORT,
                             MDNSServiceTCP,
                             recordTxt); //ci=5-lightbulb, ci=2 bridge
}

void HKServer::handle() {
  bonjour.run();

  if(clients.size() < MAX_CONNECTIONS) {
    TCPClient newClient = server.available();
    if(newClient) {
      Serial.println("Client connected.");
      clients.insert(clients.begin(),new HKConnection(this,newClient));
    }
  }

  int i = clients.size() - 1;
  while(i >= 0) {
    HKConnection *conn = clients.at(i);

    conn->handleConnection();
    if(!conn->isConnected()) {
      conn->close();
      Serial.println("Client removed.");
      clients.erase(clients.begin() + i);
    }

    i--;
  }


}
