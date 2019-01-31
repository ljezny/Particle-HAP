#include "HKServer.h"
#include "HKConnection.h"

#include "spark_wiring_tcpclient.h"
#include "spark_wiring_tcpserver.h"
#include "spark_wiring_network.h"
#include "spark_wiring_thread.h"

HKServer::HKServer() {
  persistor = new HKPersistor();

}
void HKServer::setup () {
  persistor->loadRecordStorage();
  server.begin();

  bonjour.setUDP( &udp );
  bonjour.begin(deviceName);
  setPaired(false);
}

void HKServer::setPaired(bool p) {
  paired = p;
  bonjour.removeAllServiceRecords();
  int r = bonjour.addServiceRecord(deviceName "._hap",
                          TCP_SERVER_PORT,
                          MDNSServiceTCP,
                          "\x4sf=1\x14id=" deviceIdentity "\x6pv=1.0\x04\c#=1\x04s#=1\x04\ff=1\x0Bmd=" deviceName "\x4\ci=5"); //ci=5-lightbulb, ci=2 bridge
  Serial.printf("Bonjour paired %d, r: %d\n", paired,r);
}

TCPClient client;
HKConnection *connection = NULL;
void HKServer::handle() {
  bonjour.run();

  //Serial.println(WiFi.localIP());

  if(!connection) {
    TCPClient newClient = server.available();
    if(newClient) {
      Serial.println("Client connected.");
      //clients.insert(clients.begin(),new HKConnection(this,newClient));
      connection = new HKConnection(this,newClient);
    }

  }
  if(connection) {
    if(connection->isConnected()) {
      connection->handleConnection();
    } else {
      //connection.close();
      free(connection);
      connection = NULL;
    }
  }

  /*
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
  */


}
