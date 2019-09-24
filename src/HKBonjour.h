#ifndef  HKBonjour_H
#define HKBonjour_H

#ifdef PARTICLE_COMPAT
    #include "../utils/HKTester/HKTester/Particle_Compat/particle_compat.h"
#else
    #include <Particle.h>
    extern "C" {
        #include <inttypes.h>
    }
#endif




extern uint16_t ethutil_htons(unsigned short hostshort);
extern uint32_t ethutil_htonl(unsigned long hostlong);
extern uint16_t ethutil_ntohs(unsigned short netshort);
extern uint32_t ethutil_ntohl(unsigned long netlong);

typedef uint8_t byte;

typedef enum _MDNSState_t {
   MDNSStateIdle,
   MDNSStateQuerySent
} MDNSState_t;

typedef enum _MDNSError_t {
   MDNSTryLater = 3,
   MDNSNothingToDo = 2,
   MDNSSuccess  = 1,
   MDNSInvalidArgument = -1,
   MDNSOutOfMemory = -2,
   MDNSSocketError = -3,
   MDNSAlreadyProcessingQuery = -4,
   MDNSNotFound = -5,
   MDNSServerError = -6,
   MDNSTimedOut = -7
} MDNSError_t;

typedef struct _MDNSDataInternal_t {
   uint32_t xid;
   uint32_t lastQueryFirstXid;
} MDNSDataInternal_t;

typedef enum _MDNSServiceProtocol_t {
   MDNSServiceTCP,
   MDNSServiceUDP
} MDNSServiceProtocol_t;

typedef MDNSServiceProtocol_t MDNSServiceProtocol;

#define MNDS_NAME_MAX_LENGTH 64
#define MNDS_SRV_NAME_MAX_LENGTH 64
#define MNDS_TEXT_CONTENT_MAX_LENGTH 512
typedef struct _MDNSServiceRecord_t {
   uint16_t                port;
   MDNSServiceProtocol_t   proto;
   uint8_t                name[MNDS_NAME_MAX_LENGTH];
   uint8_t                servName[MNDS_SRV_NAME_MAX_LENGTH];
   uint8_t                textContent[MNDS_TEXT_CONTENT_MAX_LENGTH];
} MDNSServiceRecord_t;


#define BONJOUR_NAME_MAX_LENGTH 128

class HKBonjour {
private:
    MDNSDataInternal_t    _mdnsData;
    MDNSState_t           _state;
    uint8_t            _bonjourName[BONJOUR_NAME_MAX_LENGTH];
    bool hasServiceRecord = false;
    MDNSServiceRecord_t _serviceRecord;
    unsigned long        _lastAnnounceMillis;

    uint8_t*             _resolveNames[2];
    unsigned long        _resolveLastSendMillis[2];
    unsigned long        _resolveTimeouts[2];

    MDNSServiceProtocol_t _resolveServiceProto;

    MDNSError_t _processMDNSQuery();
    MDNSError_t _sendMDNSMessage(uint32_t peerAddress, uint32_t xid, int type);


    void _writeDNSName(const uint8_t* name, uint16_t* pPtr, uint8_t* buf, int bufSize,
                      int zeroTerminate);
    void _writeMyIPAnswerRecord(uint16_t* pPtr, uint8_t* buf, int bufSize);
    void _writeServiceRecordName(uint16_t* pPtr, uint8_t* buf, int bufSize, int tld);
    void _writeServiceRecordPTR(uint16_t* pPtr, uint8_t* buf, int bufSize,
                               uint32_t ttl);
    uint8_t* _findFirstDotFromRight(const uint8_t* str);

    int _matchStringPart(const uint8_t** pCmpStr, int* pCmpLen, const uint8_t* buf,
                        int dataLen);

    const uint8_t* _postfixForProtocol(MDNSServiceProtocol_t proto);

    void _finishedResolvingName(char* name, const byte ipAddr[4]);
public:
    HKBonjour();
    ~HKBonjour();

    void begin();
    void begin(const char* bonjourName);
    bool run();

    void setBonjourName(const char* bonjourName);

    void setServiceRecord(const char* name, uint16_t port, MDNSServiceProtocol_t proto,
                    const char* textContent);

    void removeServiceRecord();

    // Particle compilation errors
    UDP* _localUDP;
    int setUDP( UDP * localUDP );
    int stop();
    int beginMulticast(uint8_t *IPAddr, uint16_t port);
    int write(uint8_t *buf, int size);
    int read(uint8_t *buf, int size);
    int beginPacket(uint8_t * IPAddr, uint16_t port);
    int endPacket();
    int parsePacket();
    int flush();
    int remotePort();
    int remoteIP();

};

#endif
