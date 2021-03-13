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
#include "../utils/HKTester/HKTester/Particle_Compat/particle_compat.h"
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
#define CLIENT_ID_MAX_LENGTH 15
#define PUBLIC_KEY_LENGTH 384
#define SRP_RESPONSE_LENGTH 128
class HKServer;
class characteristics;

class HKConnection {

private:
    TCPClient client = TCPClient();
    Srp srp;
    bool hasSrp = false;

    byte controllerKeyData[CHACHA20_POLY1305_AEAD_KEYSIZE];
    byte sessionKeyData[CHACHA20_POLY1305_AEAD_KEYSIZE];
    byte publicSecretKeyData[CHACHA20_POLY1305_AEAD_KEYSIZE];

    ed25519_key *accessoryKey = (ed25519_key*) ACCESSORY_KEY;

    word32 publicKeyLength = PUBLIC_KEY_LENGTH;
    char publicKey[PUBLIC_KEY_LENGTH] = {0};

    byte sharedKey[CURVE25519_KEYSIZE];
    byte readKey[CHACHA20_POLY1305_AEAD_KEYSIZE];
    int readsCount = 0;
    byte writeKey[CHACHA20_POLY1305_AEAD_KEYSIZE];
    int writesCount = 0;
    bool isEncrypted = false;

    std::vector<characteristics *> notifiableCharacteristics;
    std::vector<characteristics *> postedCharacteristics;

    void doWriteData(uint8_t *data, int size);
    void writeEncryptedData(uint8_t* payload,size_t size);
    void decryptData(uint8_t* buffer,size_t *size);
    void readData(uint8_t* buffer,size_t *size);

    void handlePairSetup(const char *buffer);
    bool handlePairVerify(const char *buffer);
    void handleAccessoryRequest(const char *buffer,size_t size);
    void processPostedCharacteristics();

    char c_ID[CLIENT_ID_MAX_LENGTH] = {0};
    int lastKeepAliveMs = 0;
public:
    HKServer *server = NULL;
    bool relay = false;

    HKConnection(HKServer *s,TCPClient c);
    HKConnection() {

    }
    ~HKConnection();
    bool handleConnection(bool maxConnectionsVictim);
    bool keepAlive();
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

    operator bool();
};
#endif
