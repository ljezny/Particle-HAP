#ifndef  HKConnection_H
#define HKConnection_H

#include "HKConfig.h"
#include "HKBonjour.h"
#include "HKNetworkMessage.h"
#include "HKNetworkMessageData.h"
#include "HKStringUtils.h"
#include "HKConsts.h"
#include "HKNetworkResponse.h"
#include "HKPersistor.h"

#ifdef PARTICLE_COMPAT
#include "../../example/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
#include <Particle.h>
#endif

#include <vector>

extern "C" {
  #include "crypto/types.h"
  #include "crypto/srp.h"
  #include "crypto/error-crypt.h"
  #include "crypto/ed25519.h"
  #include "crypto/chacha.h"
  #include "crypto/chacha20_poly1305.h"
  #include "crypto/curve25519.h"
  #include "crypto/hmac.h"
}

class HKServer;
class characteristics;

class HKConnection {

private:
  TCPClient client;
  Srp srp;

  byte controllerKeyData[CHACHA20_POLY1305_AEAD_KEYSIZE];
  byte sessionKeyData[CHACHA20_POLY1305_AEAD_KEYSIZE];
  byte publicSecretKeyData[CHACHA20_POLY1305_AEAD_KEYSIZE];

  ed25519_key *accessoryKey = (ed25519_key*) ACESSORY_KEY;

  word32 publicKeyLength = 384;
  char* publicKey = new char[publicKeyLength];

  word32 responseLength = 128;
  char* response = new char[responseLength];

  byte sharedKey[CURVE25519_KEYSIZE];
  byte readKey[CHACHA20_POLY1305_AEAD_KEYSIZE];
  int readsCount = 0;
  byte writeKey[CHACHA20_POLY1305_AEAD_KEYSIZE];
  int writesCount = 0;
  bool isEncrypted = false;

  std::vector<characteristics *> notifiableCharacteristics;
  std::vector<characteristics *> postedCharacteristics;

  void writeEncryptedData(uint8_t* payload,size_t size);
  void decryptData(uint8_t* buffer,size_t *size);
  void readData(uint8_t** buffer,size_t *size);

  void handlePairSetup(const char *buffer);
  bool handlePairVerify(const char *buffer);
  void handleAccessoryRequest(const char *buffer,size_t size);
  void processPostedCharacteristics();

  char *c_ID;
  int lastKeepAliveMs = 0;
public:
  HKServer *server;
  bool relay = false;

  HKConnection(HKServer *s,TCPClient c);
  ~HKConnection();
  bool handleConnection();
  void keepAlive();
  void announce(char* buffer);
  void writeData(uint8_t* buffer,size_t size);

  bool isConnected(){
    return client.status();
  }

  char* clientID(){
    return c_ID;
  }

  void close(){
    client.stop();
  }
  void postCharacteristicsValue(characteristics *c);
  void addNotifiedCharacteristics(characteristics *c);
  IPAddress remoteIP() {
    return client.remoteIP();
  }
};
#endif
