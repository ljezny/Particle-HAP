#include "HKConnection.h"
#include "HKServer.h"
#include "HKAccessory.h"
#include "HKLog.h"

int connectionID = 0;

void generateAccessoryKey(ed25519_key *key) {
    int r = wc_ed25519_init(key);
    Logger.printf("wc_ed25519_init key: r:%d\n",r);
    WC_RNG rng;
    r = wc_ed25519_make_key(&rng, ED25519_KEY_SIZE, key);
    Logger.printf("wc_ed25519_make_key key: r:%d\n",r);
    print_hex_memory(key,sizeof(ed25519_key));
}

HKConnection::HKConnection(HKServer *s,TCPClient c) {
    client =  c;
    server = s;
    //generateAccessoryKey(&accessoryKey);
    c_ID = new char[32];
    memset(c_ID,0,32);
    snprintf(c_ID,32,"%d/%d.%d.%d.%d",connectionID,client.remoteIP()[0],client.remoteIP()[1],client.remoteIP()[2],client.remoteIP()[3]);
    
    connectionID++;
    
}

void HKConnection::writeEncryptedData(uint8_t* payload,size_t size) {
    Logger.println("BEGIN: writeEncryptedData");
    byte nonce[12];
    memset(nonce, 0, sizeof(nonce));
    
    byte outputBuffer[(size / 1024 + 1) * (1024 + 18)];
    
    int output_offset = 0;
    int payload_offset = 0;
    int part = 0;
    while (payload_offset < size) {
        
        size_t chunk_size = size - payload_offset;
        if (chunk_size > 1024)
            chunk_size = 1024;
        byte aead[2] = {(byte) (chunk_size % 256), (byte)(chunk_size / 256)};
        
        memcpy(outputBuffer + output_offset, aead, 2);
        
        byte i = 4;
        int x = readsCount++;
        while (x) {
            nonce[i++] = x % 256;
            x /= 256;
        }
        int r = wc_ChaCha20Poly1305_Encrypt(
                                            (const byte *) readKey,
                                            nonce,
                                            aead, 2,
                                            (const byte *)payload+payload_offset, chunk_size,
                                            (byte *) outputBuffer + output_offset +2,
                                            (byte *) (outputBuffer + output_offset + chunk_size + 2)
                                            );
        if (r) {
            Logger.printf("Failed to chacha encrypt payload (code %d)\n", r);
            client.stop();
            return;
        }
        payload_offset += chunk_size;
        output_offset += chunk_size + 16 + 2;
        
        part++;
    }
    
    if(isConnected()){
        client.write(outputBuffer,output_offset);
    }
    Logger.println("END: writeEncryptedData");
}

void HKConnection::decryptData(uint8_t* payload,size_t *size) {
    Logger.println("BEGIN: decryptData");
    uint8_t *decryptedData =(uint8_t *) malloc((*size) * sizeof(uint8_t));
    size_t decryptedTotalSize = 0;
    size_t payload_size = *size;
    size_t *decrypted_size = size;
    
    const size_t block_size = 1024 + 16 + 2;
    size_t required_decrypted_size =
    payload_size / block_size * 1024 + payload_size % block_size - 16 - 2;
    if (*decrypted_size < required_decrypted_size) {
        *decrypted_size = required_decrypted_size;
    }
    
    decryptedTotalSize = required_decrypted_size;
    
    byte nonce[12];
    memset(nonce, 0, sizeof(nonce));
    
    int payload_offset = 0;
    int decrypted_offset = 0;
    while (payload_offset < payload_size) {
        size_t chunk_size = payload[payload_offset] + payload[payload_offset+1]*256;
        if (chunk_size+18 > payload_size-payload_offset) {
            // Unfinished chunk
            break;
        }
        
        byte i = 4;
        int x = writesCount++;
        while (x) {
            nonce[i++] = x % 256;
            x /= 256;
        }
        size_t decrypted_len = *decrypted_size - decrypted_offset;
        
        int r= wc_ChaCha20Poly1305_Decrypt(
                                           (const byte *) writeKey,
                                           nonce,
                                           payload+payload_offset, 2,
                                           (const byte *)payload+payload_offset+2, chunk_size,
                                           (const byte *)payload+payload_offset+2 + chunk_size, decryptedData + decrypted_offset
                                           );
        if (r) {
            Logger.printf("Failed to chacha decrypt payload (code %d)\n", r);
            //Once session security has been established, if the accessory encounters a decryption failure then it must immediately close the connection used for the session.
            //client.stop();
            *size = 0;
            return;
        }
        
        decrypted_offset += chunk_size;
        payload_offset += chunk_size + 0x12; // 0x10 is for some auth bytes
    }
    memset(payload,0,*size);
    memcpy(payload,decryptedData,decryptedTotalSize);
    *size = decryptedTotalSize;
    free(decryptedData);
    Logger.println("END: decryptData");
}

void HKConnection::readData(uint8_t* buffer,size_t *size) {
    int total = 0;
    int tempBufferSize = 4096;
    uint8_t *tempBuffer =(uint8_t *) malloc(tempBufferSize * sizeof(uint8_t));
    while(isConnected() && client.available()) {
        int len = client.read(tempBuffer,tempBufferSize);
        memcpy((buffer+total), tempBuffer,len);
        total += len;
    }
    
    *size = total;
    
    if(isEncrypted && total > 0) {
        decryptData(buffer,size);
    }
    
    free(tempBuffer);
}

void HKConnection::writeData(uint8_t* responseBuffer,size_t responseLen) {
    Logger.println("BEGIN: writeData");
    Logger.printf("writeData responseLen = %d\n", responseLen);
    if(isConnected()){
        if(isEncrypted) {
            writeEncryptedData((uint8_t *)responseBuffer,responseLen);
        } else {
            client.write((uint8_t *)responseBuffer, (size_t)responseLen);
        }
    }
    Logger.println("END: writeData");
}

void HKConnection::handleConnection() {
    /*if (!isConnected()) {
     return;
     }*/
    
    int input_buffer_size = 4096;
    uint8_t *inputBuffer =(uint8_t *) malloc(input_buffer_size * sizeof(uint8_t));
    memset(inputBuffer,0,input_buffer_size);
    size_t len = 0;
    readData(inputBuffer,&len);
    Logger.printf("Request Message read length: %d \n", len);
    
    while (len > 0) {
        lastKeepAliveMs = millis();
        HKNetworkMessage msg((const char *)inputBuffer);
        if (!strcmp(msg.directory, "pair-setup")){
            Logger.printf("Handling Pair Setup...\n");
            handlePairSetup((const char *)inputBuffer);
        }
        else if (!strcmp(msg.directory, "pair-verify")){
            Logger.printf("Handling Pair Varify...\n");
            if(handlePairVerify((const char *)inputBuffer)){
                isEncrypted = true;
                server->setPaired(true);
            }
        } else if (!strcmp(msg.directory, "identify")){
            client.stop();
        } else if(isEncrypted) { //connection is secured
            Logger.printf("Handling message request: %s\n",msg.directory);
            handleAccessoryRequest((const char *)inputBuffer, len);
        }
        readData(inputBuffer,&len);
    }
    processPostedCharacteristics();
    
    free(inputBuffer);
    
}

void HKConnection::announce(char* desc){
    char *reply = (char*)malloc(4096);
    memset(reply,0,4096);
    int len = snprintf(reply, 4096, "EVENT/1.0 200 OK\r\nContent-Type: application/hap+json\r\nContent-Length: %lu\r\n\r\n%s", strlen(desc), desc);
    
    Logger.printf("--------BEGIN ANNOUNCE: %s--------\n",clientID());
    Logger.printf("%s\n",reply);
    Logger.printf("--------END ANNOUNCE: %s--------\n",clientID());
    Logger.printf("--------ANNOUNCE: %s--------\n",clientID());
    writeData((byte*)reply,len);
    free(reply);
}

void HKConnection::keepAlive() {
    if((millis() - lastKeepAliveMs) > 2000) {
        lastKeepAliveMs = millis();
        if(isConnected()) {
            if(isEncrypted && readsCount > 0) {
                Logger.printf("Keeping alive..\n");
                
                char *aliveMsg = new char[32];
                memset(aliveMsg,0,32);
                strncpy(aliveMsg, "{\"characteristics\": []}", 32);
                announce(aliveMsg);
                free(aliveMsg);
                
            }
        }
    }
}

int wc_SrpSetKeyH(Srp *srp, byte *secret, word32 size) {
    SrpHash hash;
    int r = BAD_FUNC_ARG;
    
    srp->key = (byte*) XMALLOC(SHA512_DIGEST_SIZE, NULL, DYNAMIC_TYPE_SRP);
    if (!srp->key)
        return MEMORY_E;
    
    srp->keySz = SHA512_DIGEST_SIZE;
    
    r = wc_InitSha512(&hash.data.sha512);
    if (!r) r = wc_Sha512Update(&hash.data.sha512, secret, size);
    if (!r) r = wc_Sha512Final(&hash.data.sha512, srp->key);
    
    // clean up hash data from stack for security
    memset(&hash, 0, sizeof(hash));
    
    return r;
}
bool HKConnection::handlePairVerify(const char *buffer) {
    bool completed = false;
    char state = State_Pair_Verify_M1;
    
    HKNetworkMessage msg(buffer);
    HKNetworkResponse response = HKNetworkResponse(200);
    bcopy(msg.data.dataPtrForIndex(6), &state, 1);
    
    switch (state) {
        case State_Pair_Verify_M1: {
            Logger.printf("Pair Verify M1\n");
            curve25519_key controllerKey;
            int r = wc_curve25519_init(&controllerKey);
            Logger.printf("wc_curve25519_init key: r:%d\n",r);
            r = wc_curve25519_import_public_ex((const byte *) msg.data.dataPtrForIndex(3) , 32, &controllerKey,EC25519_LITTLE_ENDIAN);
            Logger.printf("wc_curve25519_import_public_ex: r:%d\n",r);
            memcpy(&controllerKeyData,msg.data.dataPtrForIndex(3) , 32);
            
            curve25519_key secretKey;
            r = wc_curve25519_init(&secretKey);
            Logger.printf("wc_curve25519_init: r:%d\n",r);
            WC_RNG rng;
            wc_curve25519_make_key(&rng,CURVE25519_KEYSIZE,&secretKey);
            Logger.printf("wc_curve25519_make_key: r:%d\n",r);
            
            word32 publicSecretKeySize = CURVE25519_KEYSIZE;
            r = wc_curve25519_export_public_ex(&secretKey, publicSecretKeyData, &publicSecretKeySize,EC25519_LITTLE_ENDIAN);
            Logger.printf("wc_curve25519_export_public_ex: r:%d\n",r);
            word32 sharedKeySize = CURVE25519_KEYSIZE;
            
            
            r = wc_curve25519_shared_secret_ex(&secretKey,&controllerKey,sharedKey,&sharedKeySize,EC25519_LITTLE_ENDIAN);
            Logger.printf("crypto_curve25519_shared_secret: %d\n", r);
            
            int accessoryInfoSize = CURVE25519_KEYSIZE+CURVE25519_KEYSIZE+strlen(deviceIdentity);
            byte accessoryInfo[accessoryInfoSize];
            memcpy(accessoryInfo,publicSecretKeyData, CURVE25519_KEYSIZE);
            memcpy(&accessoryInfo[CURVE25519_KEYSIZE],deviceIdentity, strlen(deviceIdentity));
            memcpy(&accessoryInfo[CURVE25519_KEYSIZE+strlen(deviceIdentity)],msg.data.dataPtrForIndex(3), CURVE25519_KEYSIZE);
            
            word32 accessorySignSize = ED25519_SIG_SIZE;
            byte accesorySign[accessorySignSize];
            r = wc_ed25519_sign_msg(accessoryInfo, accessoryInfoSize, accesorySign, &accessorySignSize,accessoryKey);
            Logger.printf("wc_ed25519_sign_msg: r:%d\n",r);
            
            HKNetworkMessageDataRecord signRecord;
            signRecord.activate = true;
            signRecord.data = new char[accessorySignSize];
            signRecord.index = 10;
            signRecord.length = accessorySignSize;
            memcpy(signRecord.data,accesorySign,accessorySignSize);
            
            HKNetworkMessageDataRecord idRecord;
            idRecord.index = 1;
            idRecord.activate = true;
            idRecord.length = strlen(deviceIdentity);
            idRecord.data = new char[idRecord.length];
            memcpy(idRecord.data,deviceIdentity, idRecord.length);
            
            HKNetworkMessageData data;
            data.addRecord(signRecord);
            data.addRecord(idRecord);
            
            char salt[] = "Pair-Verify-Encrypt-Salt";
            char info[] = "Pair-Verify-Encrypt-Info";
            size_t sessionKeySize = CHACHA20_POLY1305_AEAD_KEYSIZE;
            r = wc_HKDF(SHA512,(const byte*) sharedKey, sharedKeySize,(const byte*) salt, strlen(salt),(const byte*) info, strlen(info),sessionKeyData, CHACHA20_POLY1305_AEAD_KEYSIZE);
            Logger.printf("wc_HKDF: r:%d\n",r);
            
            const char *plainMsg = 0;   unsigned short msgLen = 0;
            data.rawData(&plainMsg, &msgLen);
            
            size_t encryptMsgSize = 0;
            byte encryptMsg[msgLen+16];
            r = wc_ChaCha20Poly1305_Encrypt(
                                            (const byte *) sessionKeyData,
                                            (const byte *)"\x0\x0\x0\x0PV-Msg02",
                                            NULL, 0,
                                            (const byte *)plainMsg, msgLen,
                                            (byte *) encryptMsg,
                                            (byte *) (encryptMsg+msgLen)
                                            );
            Logger.printf("wc_ChaCha20Poly1305_Encrypt: r:%d\n",r);
            HKNetworkMessageDataRecord stage;
            stage.activate = true;
            stage.data = new char;
            stage.data[0] = State_Pair_Verify_M2;
            stage.index = 6;
            stage.length = 1;
            
            
            HKNetworkMessageDataRecord encryptRecord;
            encryptRecord.activate = true;
            encryptRecord.index = 5;
            encryptRecord.length = msgLen+16;
            encryptRecord.data = new char[encryptRecord.length];
            memcpy(encryptRecord.data,encryptMsg,encryptRecord.length);
            
            HKNetworkMessageDataRecord pubKeyRecord;
            pubKeyRecord.activate = true;
            pubKeyRecord.data = new char[publicSecretKeySize];
            pubKeyRecord.index = 3;
            pubKeyRecord.length = publicSecretKeySize;
            memcpy(pubKeyRecord.data, publicSecretKeyData, publicSecretKeySize);
            
            response.data.addRecord(stage);
            response.data.addRecord(pubKeyRecord);
            response.data.addRecord(encryptRecord);
            
            delete [] plainMsg;
        }
            break;
        case State_Pair_Verify_M3: {
            Logger.printf("Pair Verify M3\n");
            char *encryptedData = msg.data.dataPtrForIndex(5);
            short packageLen = msg.data.lengthForIndex(5);
            byte decryptedData[packageLen-16];
            int r= wc_ChaCha20Poly1305_Decrypt(
                                               (const byte *) sessionKeyData,
                                               (const byte *)"\x0\x0\x0\x0PV-Msg03",
                                               NULL, 0,
                                               (const byte *)encryptedData, packageLen-16,
                                               (const byte *)encryptedData+packageLen-16, decryptedData
                                               );
            Logger.printf("wc_ChaCha20Poly1305_Decrypt: r:%d\n",r);
            HKNetworkMessageData subData = HKNetworkMessageData((char *)decryptedData, packageLen-16);
            HKKeyRecord rec = server->persistor->getKey(subData.dataPtrForIndex(1));
            
            int controllerInfoSize = CURVE25519_KEYSIZE+CURVE25519_KEYSIZE+subData.lengthForIndex(1);
            byte controllerInfo[controllerInfoSize];
            memcpy(controllerInfo,controllerKeyData, CURVE25519_KEYSIZE);
            memcpy(&controllerInfo[CURVE25519_KEYSIZE],subData.dataPtrForIndex(1), subData.lengthForIndex(1));
            memcpy(&controllerInfo[CURVE25519_KEYSIZE+subData.lengthForIndex(1)],publicSecretKeyData, CURVE25519_KEYSIZE);
            
            ed25519_key clKey;
            r = wc_ed25519_init(&clKey);
            Logger.printf("wc_ed25519_init: r:%d\n",r);
            r = wc_ed25519_import_public((const byte*) rec.publicKey, ED25519_PUB_KEY_SIZE, &clKey);
            Logger.printf("wc_ed25519_import_public: r:%d\n",r);
            int verified = 0;
            r = wc_ed25519_verify_msg((byte*) subData.dataPtrForIndex(10), subData.lengthForIndex(10),(const byte*)  controllerInfo,controllerInfoSize, &verified, &clKey);
            Logger.printf("wc_ed25519_verify_msg: r:%d\n",r);
            if(verified) {
                completed = true;
                
                HKNetworkMessageDataRecord stage;
                stage.activate = true;
                stage.data = new char;
                stage.data[0] = State_Pair_Verify_M4;
                stage.index = 6;
                stage.length = 1;
                response.data.addRecord(stage);
                
                //hkdf
                const char salt[] = "Control-Salt";
                const char read_info[] = "Control-Read-Encryption-Key";
                const char write_info[] = "Control-Write-Encryption-Key";
                r = wc_HKDF(SHA512,(const byte*) sharedKey, CHACHA20_POLY1305_AEAD_KEYSIZE,(const byte*) salt, strlen(salt),(const byte*) read_info, strlen(read_info),readKey, CHACHA20_POLY1305_AEAD_KEYSIZE);
                Logger.printf("wc_HKDF: r:%d\n",r);
                r = wc_HKDF(SHA512,(const byte*) sharedKey, CHACHA20_POLY1305_AEAD_KEYSIZE,(const byte*) salt, strlen(salt),(const byte*) write_info, strlen(write_info),writeKey, CHACHA20_POLY1305_AEAD_KEYSIZE);
                Logger.printf("wc_HKDF: r:%d\n",r);
                Logger.println("Pair verified, secure connection established");
            }
            else{
                HKNetworkMessageDataRecord error;
                error.activate = true;
                error.data = new char[1];
                error.data[0] = 2;
                error.index = 7;
                error.length = 1;
                response.data.addRecord(error);
                Logger.println("Pair NOT verified.");
            }
        }
    }
    
    char *responseBuffer = 0;  int responseLen = 0;
    response.getBinaryPtr(&responseBuffer, &responseLen);
    if (responseBuffer) {
        writeData((byte *)responseBuffer,responseLen);
        
        delete [] responseBuffer;
        
    } else {
        Logger.printf("Why empty response\n");
    }
    return completed;
}

void HKConnection::handlePairSetup(const char *buffer) {
    bool completed = false;
    
    HKNetworkMessageDataRecord stateRecord;
    stateRecord.activate = true;
    stateRecord.data = new char[1];
    stateRecord.length = 1;
    stateRecord.index = 6;
    
    PairSetupState_t state = State_M1_SRPStartRequest;
    
    char *responseBuffer = 0; int responseLen = 0;
    
    HKNetworkMessage msg = HKNetworkMessage(buffer);
    HKNetworkResponse mResponse(200);
    
    
    state = (PairSetupState_t)(*msg.data.dataPtrForIndex(6));
    Logger.printf("State: %d\n", state);
    *stateRecord.data = (char)state+1;
    switch (state) {
        case State_M1_SRPStartRequest: {
            Logger.println("State_M1_SRPStartRequest");
            stateRecord.data[0] = State_M2_SRPStartRespond;
            HKNetworkMessageDataRecord saltRec;
            HKNetworkMessageDataRecord publicKeyRec;
            
            byte salt[16];
            for (int i = 0; i < 16; i++) {
                salt[i] = rand();
            }
            
            int r = wc_SrpInit(&srp,SRP_TYPE_SHA512,SRP_CLIENT_SIDE);
            srp.keyGenFunc_cb = wc_SrpSetKeyH;
            if (!r) r = wc_SrpSetUsername(&srp,(const byte *)"Pair-Setup",strlen("Pair-Setup"));
            Logger.printf("wc_SrpSetUsername: r:%d\n",r);
            if (!r) r = wc_SrpSetParams(&srp,(const byte *)N, sizeof(N),(const byte *)generator, 1,salt,16);
            Logger.printf("wc_SrpSetParams: r:%d\n",r);
            if (!r) r = wc_SrpSetPassword(&srp,(const byte *)devicePassword,strlen(devicePassword));
            Logger.printf("wc_SrpSetPassword: r:%d\n",r);
            if (!r) r = wc_SrpGetVerifier(&srp, (byte *)publicKey, &publicKeyLength); //use publicKey to store v
            Logger.printf("wc_SrpGetVerifier: r:%d\n",r);
            srp.side=SRP_SERVER_SIDE; //switch to server mode
            if (!r) r = wc_SrpSetVerifier(&srp, (byte *)publicKey, publicKeyLength);
            Logger.printf("wc_SrpSetVerifier: r:%d\n",r);
            if (!r) r = wc_SrpGetPublic(&srp, (byte *)publicKey, &publicKeyLength);
            Logger.printf("wc_SrpGetPublic: r:%d\n",r);
            saltRec.index = 2;
            saltRec.activate = true;
            saltRec.length = sizeof(salt);
            saltRec.data = new char[saltRec.length];
            memcpy(saltRec.data,salt, saltRec.length);
            publicKeyRec.index = 3;
            publicKeyRec.activate = true;
            publicKeyRec.length = publicKeyLength;
            publicKeyRec.data = new char[publicKeyRec.length];
            memcpy(publicKeyRec.data,publicKey, publicKeyRec.length);
            
            mResponse.data.addRecord(stateRecord);
            mResponse.data.addRecord(publicKeyRec);
            mResponse.data.addRecord(saltRec);
            
        }
            break;
            
        case State_M3_SRPVerifyRequest: {
            Logger.println("State_M3_SRPVerifyRequest");
            stateRecord.data[0] = State_M4_SRPVerifyRespond;
            const char *keyStr = 0;
            int keyLen = 0;
            const char *proofStr;
            int proofLen = 0;
            keyStr = msg.data.dataPtrForIndex(3);
            keyLen = msg.data.lengthForIndex(3);
            char *temp = msg.data.dataPtrForIndex(4);
            if (temp != NULL) {
                proofStr = temp;
                proofLen = msg.data.lengthForIndex(4);
            } else {
                Logger.println("no proof sent!");
            }
            int r = wc_SrpComputeKey(&srp,(byte*) keyStr,keyLen,(byte*) publicKey,publicKeyLength);
            Logger.printf("wc_SrpComputeKey: r:%d\n",r);
            r = wc_SrpVerifyPeersProof(&srp, (byte*) proofStr, proofLen);
            Logger.printf("wc_SrpVerifyPeersProof: r:%d\n",r);
            if (r != 0) { //failed
                HKNetworkMessageDataRecord responseRecord;
                responseRecord.activate = true;
                responseRecord.data = new char[1];
                responseRecord.data[0] = 2;
                responseRecord.index = 7;
                responseRecord.length = 1;
                mResponse.data.addRecord(stateRecord);
                mResponse.data.addRecord(responseRecord);
                
                Logger.println("INCORRECT PASSWORD");
            } else { //success
                wc_SrpGetProof(&srp, (byte *)response,&responseLength);
                //SRP_respond(srp, &response);
                HKNetworkMessageDataRecord responseRecord;
                responseRecord.activate = true;
                responseRecord.index = 4;
                responseRecord.length = responseLength;
                responseRecord.data = new char[responseRecord.length];
                memcpy(responseRecord.data,response, responseRecord.length);
                
                mResponse.data.addRecord(stateRecord);
                mResponse.data.addRecord(responseRecord);
                Logger.println("PASSWORD OK");
            }
        }
            break;
        case State_M5_ExchangeRequest: {
            Logger.println("State_M5_ExchangeRequest");
            stateRecord.data[0] = State_M6_ExchangeRespond;
            const char *encryptedPackage = NULL;int packageLen = 0;
            encryptedPackage = msg.data.dataPtrForIndex(5);
            packageLen = msg.data.lengthForIndex(5);
            char encryptedData[packageLen];
            memcpy(encryptedData,encryptedPackage, packageLen-16);
            char mac[16];
            memcpy(mac,&encryptedPackage[packageLen-16], 16);
            
            const char salt1[] = "Pair-Setup-Encrypt-Salt";
            const char info1[] = "Pair-Setup-Encrypt-Info";
            uint8_t sharedKey[100];
            int r = wc_HKDF(SHA512,(const byte*) srp.key, srp.keySz,(const byte*) salt1, strlen(salt1),(const byte*) info1, strlen(info1),sharedKey, CHACHA20_POLY1305_AEAD_KEYSIZE);
            Logger.printf("wc_HKDF: r:%d\n",r);
            uint8_t decryptedData[packageLen-16];
            bzero(decryptedData, packageLen-16);
            r= wc_ChaCha20Poly1305_Decrypt(
                                           (const byte *)sharedKey,
                                           (const byte *)"\x0\x0\x0\x0PS-Msg05",
                                           NULL, 0,
                                           (const byte *)encryptedData, packageLen-16,
                                           (const byte *)mac, decryptedData
                                           );
            Logger.printf("wc_ChaCha20Poly1305_Decrypt: r:%d\n",r);
            HKNetworkMessageData *subTLV8 = new HKNetworkMessageData((char *)decryptedData, packageLen-16);
            char *controllerIdentifier = subTLV8->dataPtrForIndex(1);
            char *controllerPublicKey = subTLV8->dataPtrForIndex(3);
            size_t controllerPublicKeySize = subTLV8->lengthForIndex(3);
            char *controllerSignature = subTLV8->dataPtrForIndex(10);
            size_t controllerSignatureSize = subTLV8->lengthForIndex(10);
            char controllerHash[100];
            
            server->persistor->resetPersistor();
            
            HKKeyRecord newRecord;
            memcpy(newRecord.controllerID,controllerIdentifier, 36);
            memcpy(newRecord.publicKey,controllerPublicKey, 32);
            server->persistor->addKey(newRecord);
            
            
            const char salt2[] = "Pair-Setup-Controller-Sign-Salt";
            const char info2[] = "Pair-Setup-Controller-Sign-Info";
            r = wc_HKDF(SHA512,(const byte*) srp.key, srp.keySz,(const byte*) salt2, strlen(salt2),(const byte*) info2, strlen(info2),(byte*)controllerHash, CHACHA20_POLY1305_AEAD_KEYSIZE);
            Logger.printf("wc_HKDF: r:%d\n",r);
            memcpy(&controllerHash[32],controllerIdentifier, 36);
            memcpy(&controllerHash[68],controllerPublicKey, 32);
            
            ed25519_key clKey;
            r = wc_ed25519_init(&clKey);
            Logger.printf("wc_ed25519_init: r:%d\n",r);
            r = wc_ed25519_import_public((const byte*) controllerPublicKey, controllerPublicKeySize, &clKey);
            Logger.printf("wc_ed25519_import_public: r:%d\n",r);
            int verified = 0;
            r = wc_ed25519_verify_msg((byte*) controllerSignature, controllerSignatureSize,(const byte*)  controllerHash,100, &verified, &clKey);
            Logger.printf("wc_ed25519_verify_msg: r:%d\n",r);
            if(verified) {
                HKNetworkMessageData *returnTLV8 = new HKNetworkMessageData();
                
                HKNetworkMessageDataRecord usernameRecord;
                usernameRecord.activate = true;
                usernameRecord.index = 1;
                usernameRecord.length = strlen(deviceIdentity);
                usernameRecord.data = new char[usernameRecord.length];
                memcpy(usernameRecord.data,deviceIdentity,usernameRecord.length);
                returnTLV8->addRecord(usernameRecord);
                
                // Generate Signature
                const char salt3[] = "Pair-Setup-Accessory-Sign-Salt";
                const char info3[] = "Pair-Setup-Accessory-Sign-Info";
                size_t outputSize = 64+strlen(deviceIdentity);
                uint8_t output[outputSize];
                r = wc_HKDF(SHA512,(const byte*) srp.key, srp.keySz,(const byte*) salt3, strlen(salt3),(const byte*) info3, strlen(info3),(byte*)output, CHACHA20_POLY1305_AEAD_KEYSIZE);
                Logger.printf("wc_HKDF: r:%d\n",r);
                word32 accessoryPubKeySize = ED25519_PUB_KEY_SIZE;
                uint8_t accessoryPubKey[accessoryPubKeySize];
                r = wc_ed25519_export_public(accessoryKey, accessoryPubKey, &accessoryPubKeySize);
                memcpy(&output[32],deviceIdentity,strlen(deviceIdentity));
                memcpy(&output[32+strlen(deviceIdentity)],accessoryPubKey,accessoryPubKeySize);
                word32 signatureSize = 64;
                uint8_t signature[signatureSize];
                
                r = wc_ed25519_sign_msg(output,outputSize,signature,&signatureSize,accessoryKey);
                Logger.printf("wc_ed25519_sign_msg: r:%d\n",r);
                
                HKNetworkMessageDataRecord signatureRecord;
                signatureRecord.activate = true;
                signatureRecord.index = 10;
                signatureRecord.length = 64;
                signatureRecord.data = new char[64];
                memcpy(signatureRecord.data,signature,signatureSize);
                returnTLV8->addRecord(signatureRecord);
                
                HKNetworkMessageDataRecord publicKeyRecord;
                publicKeyRecord.activate = true;
                publicKeyRecord.index = 3;
                publicKeyRecord.length = accessoryPubKeySize;
                publicKeyRecord.data = new char[accessoryPubKeySize];
                memcpy(publicKeyRecord.data,accessoryPubKey,accessoryPubKeySize);
                returnTLV8->addRecord(publicKeyRecord);
                char *tlv8Data;unsigned short tlv8Len;
                returnTLV8->rawData((const char **) &tlv8Data, &tlv8Len);
                
                HKNetworkMessageDataRecord tlv8Record;
                tlv8Record.data = new char[tlv8Len+16];
                tlv8Record.length = tlv8Len+16;
                bzero(tlv8Record.data, tlv8Record.length);
                
                
                r = wc_ChaCha20Poly1305_Encrypt(
                                                (const byte *)sharedKey,
                                                (const byte *)"\x0\x0\x0\x0PS-Msg06",
                                                NULL, 0,
                                                (const byte *)tlv8Data, tlv8Len,
                                                (byte*) tlv8Record.data,
                                                (byte*) (tlv8Record.data + tlv8Len)
                                                );
                Logger.printf("wc_ChaCha20Poly1305_Encrypt: r:%d\n",r);
                
                tlv8Record.activate = true;
                tlv8Record.index = 5;//5
                
                mResponse.data.addRecord(stateRecord);
                mResponse.data.addRecord(tlv8Record);
                completed = true;
                delete returnTLV8;
                free(tlv8Data);
            }
            
            delete subTLV8;
        }
            break;
    }
    mResponse.getBinaryPtr(&responseBuffer, &responseLen);
    if (responseBuffer) {
        writeData((byte*)responseBuffer,responseLen);
        
        delete [] responseBuffer;
    } else {
        Logger.printf("Why empty response\n");
    }
    if(completed){
        server->setPaired(1);
        //client.stop();
        Logger.println("Pairing completed.");
        wc_SrpTerm(&srp);
    }
    
}

void HKConnection::handleAccessoryRequest(const char *buffer,size_t size){
    char *resultData = 0; unsigned int resultLen = 0;
    Logger.printf("--------REQUEST %s--------\n",clientID());
    Logger.printf("--------BEGIN REQUEST: %s--------\n",clientID());
    Logger.printf("%s\n",buffer);
    Logger.printf("--------BEGIN REQUEST: %s--------\n",clientID());
    handleAccessory(buffer, size, &resultData, &resultLen, this);
    if(resultLen > 0) {
        Logger.printf("--------BEGIN RESPONSE: %s--------\n",clientID());
        Logger.printf("%s\n",resultData);
        Logger.printf("--------END RESPONSE: %s--------\n",clientID());
        
        writeData((byte*)resultData,resultLen);
        Logger.printf("--------RESPONSE %s--------\n",clientID());
    }
    if(resultData) {
        free(resultData);
    }
}

void HKConnection::processPostedCharacteristics() {
    for(int i = 0; i < postedCharacteristics.size(); i++) {
        characteristics *c = postedCharacteristics.at(i);
        
        char broadcastTemp[1024];
        memset(broadcastTemp,0,1024);
        snprintf(broadcastTemp, 1024, "{\"characteristics\":[{\"aid\": %d, \"iid\": %d, \"value\": %s}]}", c->accessory->aid, c->iid, c->value(NULL).c_str());
        announce(broadcastTemp);
        
    }
    postedCharacteristics.clear();
    
}

void HKConnection::postCharacteristicsValue(characteristics *c){
    postedCharacteristics.push_back(c);
}
