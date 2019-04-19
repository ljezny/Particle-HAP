//  Copyright (C) 2010 Georg Kaindl
//  http://gkaindl.com
//
//  Particle.io Port Copyright (c) 2017 Mobile FLow LLC
//  http://github/moflome
//
//  This file is part of Arduino EthernetBonjour.
//
//  EthernetBonjour is free software: you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public License
//  as published by the Free Software Foundation, either version 3 of
//  the License, or (at your option) any later version.
//
//  EthernetBonjour is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with EthernetBonjour. If not, see
//  <http://www.gnu.org/licenses/>.
//

#define  HAS_SERVICE_REGISTRATION      1  // disabling saves about 1.25 kilobytes
#define  HAS_NAME_BROWSING             1  // disable together with above, additionally saves about 4.3 kilobytes

#include <string.h>
#include <stdlib.h>

#include "HKBonjour.h"
#include "HKLog.h"
#define  MDNS_DEFAULT_NAME       "arduino"
#define  MDNS_TLD                ".local"
#define  DNS_SD_SERVICE          "_services._dns-sd._udp.local"
#define  MDNS_SERVER_PORT        5353
#define  MDNS_NQUERY_RESEND_TIME 1000   // 1 second, name query resend timeout
#define  MDNS_SQUERY_RESEND_TIME 1000  // 10 seconds, service query resend timeout
#define  MDNS_RESPONSE_TTL       120    //120 two minutes (in seconds)

#define  MDNS_MAX_SERVICES_PER_PACKET  6
#define  MSNS_ANNOUNCE_TIME_SEC  10 //Send announce packet every 5 seconds

static uint8_t mdnsMulticastIPAddr[] = { 224, 0, 0, 251 };
//static uint8_t mdnsHWAddr[] = { 0x01, 0x00, 0x5e, 0x00, 0x00, 0xfb };



typedef enum _MDNSPacketType_t {
   MDNSPacketTypeMyIPAnswer,
   MDNSPacketTypeNoIPv6AddrAvailable,
   MDNSPacketTypeServiceRecord,
   MDNSPacketTypeServiceRecordRelease,
   MDNSPacketTypeNameQuery,
   MDNSPacketTypeServiceQuery,
} MDNSPacketType_t;

typedef struct _DNSHeader_t {
   uint16_t    xid;
   uint8_t     recursionDesired:1;
   uint8_t     truncated:1;
   uint8_t     authoritiveAnswer:1;
   uint8_t     opCode:4;
   uint8_t     queryResponse:1;
   uint8_t     responseCode:4;
   uint8_t     checkingDisabled:1;
   uint8_t     authenticatedData:1;
   uint8_t     zReserved:1;
   uint8_t     recursionAvailable:1;
   uint16_t    queryCount;
   uint16_t    answerCount;
   uint16_t    authorityCount;
   uint16_t    additionalCount;
} __attribute__((__packed__)) DNSHeader_t;

typedef enum _DNSOpCode_t {
   DNSOpQuery     = 0,
   DNSOpIQuery    = 1,
   DNSOpStatus    = 2,
   DNSOpNotify    = 4,
   DNSOpUpdate    = 5
} DNSOpCode_t;


HKBonjour::HKBonjour()
{
   memset(&this->_mdnsData, 0, sizeof(MDNSDataInternal_t));
   memset(&this->_serviceRecords, 0, sizeof(this->_serviceRecords));

   this->_state = MDNSStateIdle;
//   this->_sock = -1;

   this->_resolveNames[0] = NULL;
   this->_resolveNames[1] = NULL;

   this->_lastAnnounceMillis = 0;
}

HKBonjour::~HKBonjour()
{
	this->stop();
}


int HKBonjour::setUDP( UDP * localUDP )
{
      this->_localUDP = localUDP;
      return 0;
}
int HKBonjour::stop()
{
    UDP *udp = this->_localUDP;

	udp->stop();

      return 0;
}
int HKBonjour::beginMulticast(uint8_t *IPAddr, uint16_t port)
{
    UDP *udp = this->_localUDP;

	udp->begin(5353);    // was... 4097
	udp->beginPacket(IPAddr, 5353);

      return 0;
}
int HKBonjour::write(uint8_t *buf, int size)
{
    UDP *udp = this->_localUDP;

	udp->write(buf, size);

      return 0;
}
int HKBonjour::read(uint8_t *buf, int size)
{
    UDP *udp = this->_localUDP;

      udp->setBuffer(size, buf); // application provided buffer

      return udp->read(buf, size);

}
int HKBonjour::beginPacket(uint8_t * IPAddr, uint16_t port)
{
    UDP *udp = this->_localUDP;
    udp->beginPacket(IPAddr, 5353);
    return 0;
}
int HKBonjour::endPacket()
{
    UDP *udp = this->_localUDP;

	udp->endPacket();

      return 0;
}
int HKBonjour::parsePacket()
{
    UDP *udp = this->_localUDP;

    return udp->parsePacket();
}
int HKBonjour::flush()
{
    UDP *udp = this->_localUDP;

    udp->flush();

      return 0;
}
int HKBonjour::remotePort()
{
    UDP *udp = this->_localUDP;

    return udp->remotePort();
}
int HKBonjour::remoteIP()
{
    UDP *udp = this->_localUDP;

    return udp->remoteIP();
}
/*unsigned long HKBonjour::localIP()
{
  IPAddress localIP = WiFi.localIP();

  return (unsigned long)localIP;
}*/

// return values:
// 1 on success
// 0 otherwise
int HKBonjour::begin(const char* bonjourName)
{
	int statusCode = 0;
	statusCode = this->setBonjourName(bonjourName);
	if (statusCode)
	statusCode = this->beginMulticast(mdnsMulticastIPAddr, MDNS_SERVER_PORT);

	return statusCode;
}

// return values:
// 1 on success
// 0 otherwise
int HKBonjour::begin()
{
   return this->begin(MDNS_DEFAULT_NAME);
}

// return value:
// A DNSError_t (DNSSuccess on success, something else otherwise)
// in "int" mode: positive on success, negative on error
MDNSError_t HKBonjour::_sendMDNSMessage(uint32_t peerAddress, uint32_t xid, int type,
                                                   int serviceRecord)
{
   MDNSError_t statusCode = MDNSSuccess;
   uint16_t ptr = 0;
   DNSHeader_t dnsHeader;
   uint8_t* buf;

   memset(&dnsHeader, 0, sizeof(DNSHeader_t));

   dnsHeader.xid = ethutil_htons(xid);
   dnsHeader.opCode = DNSOpQuery;

   switch (type) {
      case MDNSPacketTypeServiceRecordRelease:
      case MDNSPacketTypeMyIPAnswer:
         dnsHeader.answerCount = ethutil_htons(1);
         dnsHeader.queryResponse = 1;
         dnsHeader.authoritiveAnswer = 1;
         break;
      case MDNSPacketTypeServiceRecord:
         dnsHeader.answerCount = ethutil_htons(4);
         dnsHeader.additionalCount = ethutil_htons(1);
         dnsHeader.queryResponse = 1;
         dnsHeader.authoritiveAnswer = 1;
         break;
      case MDNSPacketTypeNameQuery:
      case MDNSPacketTypeServiceQuery:
         dnsHeader.queryCount = ethutil_htons(1);
         break;
      case MDNSPacketTypeNoIPv6AddrAvailable:
         dnsHeader.queryCount = ethutil_htons(1);
         dnsHeader.additionalCount = ethutil_htons(1);
         dnsHeader.responseCode = 0x03;
         dnsHeader.authoritiveAnswer = 1;
         dnsHeader.queryResponse = 1;
         break;
   }




   this->beginPacket(mdnsMulticastIPAddr,MDNS_SERVER_PORT);
   this->write((uint8_t*)&dnsHeader,sizeof(DNSHeader_t));

   ptr += sizeof(DNSHeader_t);
   buf = (uint8_t*)&dnsHeader;

   // construct the answer section
   switch (type) {
      case MDNSPacketTypeMyIPAnswer: {
         this->_writeMyIPAnswerRecord(&ptr, buf, sizeof(DNSHeader_t));
         break;
      }

#if defined(HAS_SERVICE_REGISTRATION) && HAS_SERVICE_REGISTRATION

      case MDNSPacketTypeServiceRecord: {

         // SRV location record
         this->_writeServiceRecordName(serviceRecord, &ptr, buf, sizeof(DNSHeader_t), 0);

         buf[0] = 0x00;
         buf[1] = 0x21;    // SRV record
         buf[2] = 0x80;    // cache flush
         buf[3] = 0x01;    // class IN

         // ttl
         *((uint32_t*)&buf[4]) = ethutil_htonl(MDNS_RESPONSE_TTL);

         // data length
         *((uint16_t*)&buf[8]) = ethutil_htons(8 + strlen((char*)this->_bonjourName));

         this->write((uint8_t*)buf,10);
         ptr += 10;
         // priority and weight
         buf[0] = buf[1] = buf[2] = buf[3] = 0;

         // port
         *((uint16_t*)&buf[4]) = ethutil_htons(this->_serviceRecords[serviceRecord]->port);

         this->write((uint8_t*)buf,6);
         ptr += 6;
         // target
         this->_writeDNSName(this->_bonjourName, &ptr, buf, sizeof(DNSHeader_t), 1);

         // TXT record
         this->_writeServiceRecordName(serviceRecord, &ptr, buf, sizeof(DNSHeader_t), 0);

         buf[0] = 0x00;
         buf[1] = 0x10;    // TXT record
         buf[2] = 0x80;    // cache flush
         buf[3] = 0x01;    // class IN

         // ttl
         *((uint32_t*)&buf[4]) = ethutil_htonl(MDNS_RESPONSE_TTL);

         this->write((uint8_t*)buf,8);
         ptr += 8;

         // data length && text
         if (NULL == this->_serviceRecords[serviceRecord]->textContent) {
            buf[0] = 0x00;
            buf[1] = 0x01;
            buf[2] = 0x00;

            this->write((uint8_t*)buf,3);
            ptr += 3;
         } else {
            int slen = strlen((char*)this->_serviceRecords[serviceRecord]->textContent);
            *((uint16_t*)buf) = ethutil_htons(slen);
            this->write((uint8_t*)buf,2);
            ptr += 2;

            this->write((uint8_t*)this->_serviceRecords[serviceRecord]->textContent,slen);
            ptr += slen;
         }

         // PTR record (for the dns-sd service in general)
         this->_writeDNSName((const uint8_t*)DNS_SD_SERVICE, &ptr, buf,
                                          sizeof(DNSHeader_t), 1);

         buf[0] = 0x00;
         buf[1] = 0x0c;    // PTR record
         buf[2] = 0x00;    // no cache flush
         buf[3] = 0x01;    // class IN

         // ttl
         *((uint32_t*)&buf[4]) = ethutil_htonl(MDNS_RESPONSE_TTL);

         // data length.
         uint16_t dlen = strlen((char*)this->_serviceRecords[serviceRecord]->servName) + 2;
         *((uint16_t*)&buf[8]) = ethutil_htons(dlen);

         this->write((uint8_t*)buf, 10);
         ptr += 10;

         this->_writeServiceRecordName(serviceRecord, &ptr, buf, sizeof(DNSHeader_t), 1);

         // PTR record (our service)
         this->_writeServiceRecordPTR(serviceRecord, &ptr, buf, sizeof(DNSHeader_t),
                                      MDNS_RESPONSE_TTL);

         // finally, our IP address as additional record
         this->_writeMyIPAnswerRecord(&ptr, buf, sizeof(DNSHeader_t));

         break;
      }

      case MDNSPacketTypeServiceRecordRelease: {
         // just send our service PTR with a TTL of zero
         this->_writeServiceRecordPTR(serviceRecord, &ptr, buf, sizeof(DNSHeader_t), 0);
         break;
      }

#endif // defined(HAS_SERVICE_REGISTRATION) && HAS_SERVICE_REGISTRATION

#if defined(HAS_NAME_BROWSING) && HAS_NAME_BROWSING

      case MDNSPacketTypeNameQuery:
      case MDNSPacketTypeServiceQuery:
      {
         // construct a query for the currently set _resolveNames[0]
         this->_writeDNSName(
               (type == MDNSPacketTypeServiceQuery) ? this->_resolveNames[1] :
                                                      this->_resolveNames[0],
               &ptr, buf, sizeof(DNSHeader_t), 1);

         buf[0] = buf[2] = 0x0;
         buf[1] = (type == MDNSPacketTypeServiceQuery) ? 0x0c : 0x01;
         buf[3] = 0x1;

         this->write((uint8_t*)buf, sizeof(DNSHeader_t));
         ptr += sizeof(DNSHeader_t);

         this->_resolveLastSendMillis[(type == MDNSPacketTypeServiceQuery) ? 1 : 0] = millis();

         break;
      }

#endif // defined(HAS_NAME_BROWSING) && HAS_NAME_BROWSING

      case MDNSPacketTypeNoIPv6AddrAvailable: {
         // since the WIZnet doesn't have IPv6, we will respond with a Not Found message
         this->_writeDNSName(this->_bonjourName, &ptr, buf, sizeof(DNSHeader_t), 1);

         buf[0] = buf[2] = 0x0;
         buf[1] = 0x1c; // AAAA record
         buf[3] = 0x01;

         this->write((uint8_t*)buf, 4);
         ptr += 4;

         // send our IPv4 address record as additional record, in case the peer wants it.
         this->_writeMyIPAnswerRecord(&ptr, buf, sizeof(DNSHeader_t));

         break;
      }
   }


   this->endPacket();

errorReturn:

   hkLog.info("Bonjour advertised");
   return statusCode;
}

bool HKBonjour::run()
{
   bool result = false;
   uint8_t i;
   unsigned long now = millis();

   // are we querying a name or service? if so, should we resend the packet or time out?
   for (i=0; i<2; i++) {
      if (NULL != this->_resolveNames[i]) {
         // Hint: _resolveLastSendMillis is updated in _sendMDNSMessage
         if (now - this->_resolveLastSendMillis[i] > ((i == 0) ? (uint32_t)MDNS_NQUERY_RESEND_TIME :
                                                                 (uint32_t)MDNS_SQUERY_RESEND_TIME))
            (void)this->_sendMDNSMessage(0,
                                         0,
                                         (0 == i) ? MDNSPacketTypeNameQuery :
                                                    MDNSPacketTypeServiceQuery,
                                         0);

         if (this->_resolveTimeouts[i] > 0 && now > this->_resolveTimeouts[i]) {
            if (i == 0)
               this->_finishedResolvingName((char*)this->_resolveNames[0], NULL);
            else if (i == 1) {
               if (this->_serviceFoundCallback) {
                  char* typeName = (char*)this->_resolveNames[1];
                  char* p = (char*)this->_resolveNames[1];
                  while(*p && *p != '.')
                     p++;
                  *p = '\0';

                  this->_serviceFoundCallback(typeName,
                                              this->_resolveServiceProto,
                                              NULL,
                                              NULL,
                                              0,
                                              NULL);
               }
            }

            if (NULL != this->_resolveNames[i]) {
               free(this->_resolveNames[i]);
               this->_resolveNames[i] = NULL;
            }
         }
      }
   }

   // now, should we re-announce our services again?
   if ((now - this->_lastAnnounceMillis) > 1000*MSNS_ANNOUNCE_TIME_SEC) {
      for (i=0; i<NumMDNSServiceRecords; i++) {
         if (NULL != this->_serviceRecords[i])
            (void)this->_sendMDNSMessage(0, 0, (int)MDNSPacketTypeServiceRecord, i);
      }

      this->_lastAnnounceMillis = now;
      result = true;
   }
   return result;
}

// return values:
// 1 on success
// 0 otherwise
int HKBonjour::setBonjourName(const char* bonjourName)
{
   if (NULL == bonjourName)
      return 0;

   memset(this->_bonjourName, 0, BONJOUR_NAME_MAX_LENGTH);
   strcpy((char*)this->_bonjourName, bonjourName);
   strcpy((char*)this->_bonjourName+strlen(bonjourName), MDNS_TLD);

   return 1;
}

// return values:
// 1 on success
// 0 otherwise
int HKBonjour::addServiceRecord(const char* name, uint16_t port,
                                           MDNSServiceProtocol_t proto)
{
#if defined(__MK20DX128__) || defined(__MK20DX256__)
	 return this->addServiceRecord(name, port, proto, NULL); //works for Teensy 3 (32-bit Arm Cortex)
#else
   return this->addServiceRecord(name, port, proto, ""); //works for Teensy 2 (8-bit Atmel)
#endif
}

// return values:
// 1 on success
// 0 otherwise
int HKBonjour::addServiceRecord(const char* name, uint16_t port,
                                           MDNSServiceProtocol_t proto, const char* textContent)
{
   int i, status = 0;
   MDNSServiceRecord_t* record = NULL;

   if (NULL != name && 0 != port) {
      for (i=0; i < NumMDNSServiceRecords; i++) {
         if (NULL == this->_serviceRecords[i]) {
            record = (MDNSServiceRecord_t*)malloc(sizeof(MDNSServiceRecord_t));
            if (NULL != record) {
               record->name = record->textContent = NULL;

               record->name = (uint8_t*)malloc(strlen((char*)name) + 1);
               memset(record->name, 0, strlen((char*)name) + 1);
               if (NULL == record->name)
                  goto errorReturn;

               if (NULL != textContent) {
                  record->textContent = (uint8_t*)malloc(strlen((char*)textContent) + 1);
                  memset(record->textContent, 0, strlen((char*)textContent) + 1);
                  if (NULL == record->textContent)
                     goto errorReturn;

                  strcpy((char*)record->textContent, textContent);
               }

               record->port = port;
               record->proto = proto;
               strcpy((char*)record->name, name);

               uint8_t* s = this->_findFirstDotFromRight(record->name);
               record->servName = (uint8_t*)malloc(strlen((char*)s) + 12 + 1);
               memset(record->servName, 0, strlen((char*)s) + 12 + 1);
               if (record->servName) {
                  strcpy((char*)record->servName, (const char*)s);

                  const uint8_t* srv_type = this->_postfixForProtocol(proto);
                  if (srv_type)
                     strcat((char*)record->servName, (const char*)srv_type);
               }

               this->_serviceRecords[i] = record;

               status = (MDNSSuccess ==
                           this->_sendMDNSMessage(0, 0, (int)MDNSPacketTypeServiceRecord, i));

               break;
            }
         }
      }
   }

   return status;

errorReturn:
   if (NULL != record) {
      if (NULL != record->name)
         free(record->name);
      if (NULL != record->servName)
         free(record->servName);
      if (NULL != record->textContent)
         free(record->textContent);

      free(record);
   }

   return 0;
}

void HKBonjour::_removeServiceRecord(int idx)
{
   if (NULL != this->_serviceRecords[idx]) {
      (void)this->_sendMDNSMessage(0, 0, (int)MDNSPacketTypeServiceRecordRelease, idx);

      if (NULL != this->_serviceRecords[idx]->textContent)
         free(this->_serviceRecords[idx]->textContent);

      if (NULL != this->_serviceRecords[idx]->servName)
         free(this->_serviceRecords[idx]->servName);

      free(this->_serviceRecords[idx]->name);
      free(this->_serviceRecords[idx]);

      this->_serviceRecords[idx] = NULL;
   }
}

void HKBonjour::removeServiceRecord(uint16_t port, MDNSServiceProtocol_t proto)
{
   this->removeServiceRecord(NULL, port, proto);
}

void HKBonjour::removeServiceRecord(const char* name, uint16_t port,
                                               MDNSServiceProtocol_t proto)
{
   int i;
   for (i=0; i<NumMDNSServiceRecords; i++)
      if (port == this->_serviceRecords[i]->port &&
          proto == this->_serviceRecords[i]->proto &&
          (NULL == name || 0 == strcmp((char*)this->_serviceRecords[i]->name, name))) {
             this->_removeServiceRecord(i);
             break;
          }
}

void HKBonjour::removeAllServiceRecords()
{
   int i;
   for (i=0; i<NumMDNSServiceRecords; i++)
      this->_removeServiceRecord(i);
}

void HKBonjour::_writeDNSName(const uint8_t* name, uint16_t* pPtr,
                                         uint8_t* buf, int bufSize, int zeroTerminate)
{
   uint16_t ptr = *pPtr;
   uint8_t* p1 = (uint8_t*)name, *p2, *p3;
   int i, c, len;

   while(*p1) {
      c = 1;
      p2 = p1;
      while (0 != *p2 && '.' != *p2) { p2++; c++; };

      p3 = buf;
      i = c;
      len = bufSize-1;
      *p3++ = (uint8_t)--i;
      while (i-- > 0) {
         *p3++ = *p1++;

         if (--len <= 0) {
            this->write((uint8_t*)buf, bufSize);
            ptr += bufSize;
            len = bufSize;
            p3 = buf;
         }
      }

      while ('.' == *p1)
         ++p1;

      if (len != bufSize) {
    	  this->write((uint8_t*)buf, bufSize-len);
         ptr += bufSize-len;
      }
   }

   if (zeroTerminate) {
      buf[0] = 0;
      this->write((uint8_t*)buf, 1);
      ptr += 1;
   }

   *pPtr = ptr;
}

void HKBonjour::_writeMyIPAnswerRecord(uint16_t* pPtr, uint8_t* buf, int bufSize)
{
   uint16_t ptr = *pPtr;

   this->_writeDNSName(this->_bonjourName, &ptr, buf, bufSize, 1);

   buf[0] = 0x00;
   buf[1] = 0x01;
   buf[2] = 0x80; // cache flush: true
   buf[3] = 0x01;
   this->write((uint8_t*)buf, 4);
   ptr += 4;

   *((uint32_t*)buf) = ethutil_htonl(MDNS_RESPONSE_TTL);
   *((uint16_t*)&buf[4]) = ethutil_htons(4);      // data length

   uint8_t myIp[4];
   IPAddress myIpBuf;
#if !defined(PARTICLE)
   myIpBuf = Ethernet.localIP();
#else
   myIpBuf = WiFi.localIP();
#endif
   myIp[0] = myIpBuf [0];
   myIp[1] = myIpBuf [1];
   myIp[2] = myIpBuf [2];
   myIp[3] = myIpBuf [3];

   memcpy(&buf[6], &myIp, 4);              // our IP address

   this->write((uint8_t*)buf, 10);
   ptr += 10;

   *pPtr = ptr;
}

void HKBonjour::_writeServiceRecordName(int recordIndex, uint16_t* pPtr, uint8_t* buf,
                                                   int bufSize, int tld)
{
   uint16_t ptr = *pPtr;

   uint8_t* name = tld ? this->_serviceRecords[recordIndex]->servName :
                         this->_serviceRecords[recordIndex]->name;

   this->_writeDNSName(name, &ptr, buf, bufSize, tld);

   if (0 == tld) {
      const uint8_t* srv_type =
         this->_postfixForProtocol(this->_serviceRecords[recordIndex]->proto);

      if (NULL != srv_type) {
         srv_type++; // eat the dot at the beginning
         this->_writeDNSName(srv_type, &ptr, buf, bufSize, 1);
      }
   }

   *pPtr = ptr;
}

void HKBonjour::_writeServiceRecordPTR(int recordIndex, uint16_t* pPtr, uint8_t* buf,
                                                  int bufSize, uint32_t ttl)
{
   uint16_t ptr = *pPtr;

   this->_writeServiceRecordName(recordIndex, &ptr, buf, bufSize, 1);

   buf[0] = 0x00;
   buf[1] = 0x0c;    // PTR record
   buf[2] = 0x00;    // no cache flush
   buf[3] = 0x01;    // class IN

   // ttl
   *((uint32_t*)&buf[4]) = ethutil_htonl(ttl);

   // data length (+13 = "._tcp.local" or "._udp.local" + 1  byte zero termination)
   *((uint16_t*)&buf[8]) =
         ethutil_htons(strlen((char*)this->_serviceRecords[recordIndex]->name) + 13);

   this->write((uint8_t*)buf, 10);
   ptr += 10;

   this->_writeServiceRecordName(recordIndex, &ptr, buf, bufSize, 0);

   *pPtr = ptr;
}

uint8_t* HKBonjour::_findFirstDotFromRight(const uint8_t* str)
{
   const uint8_t* p = str + strlen((char*)str);
   while (p > str && '.' != *p--);
   return (uint8_t*)&p[2];
}

int HKBonjour::_matchStringPart(const uint8_t** pCmpStr, int* pCmpLen, const uint8_t* buf,
                                           int dataLen)
{
   int matches = 1;

   if (*pCmpLen >= dataLen)
      matches &= (0 == memcmp(*pCmpStr, buf, dataLen));
   else
      matches = 0;

   *pCmpStr += dataLen;
   *pCmpLen -= dataLen;
   if ('.' == **pCmpStr)
      (*pCmpStr)++, (*pCmpLen)--;

   return matches;
}

const uint8_t* HKBonjour::_postfixForProtocol(MDNSServiceProtocol_t proto)
{
   const uint8_t* srv_type = NULL;
   switch(proto) {
      case MDNSServiceTCP:
         srv_type = (uint8_t*)"._tcp" MDNS_TLD;
         break;
      case MDNSServiceUDP:
         srv_type = (uint8_t*)"._udp" MDNS_TLD;
         break;
   }

   return srv_type;
}

void HKBonjour::_finishedResolvingName(char* name, const byte ipAddr[4])
{
   if (NULL != this->_nameFoundCallback) {
      if (NULL != name) {
         uint8_t* n = this->_findFirstDotFromRight((const uint8_t*)name);
         *(n-1) = '\0';
      }

      this->_nameFoundCallback((const char*)name, ipAddr);
   }

   free(this->_resolveNames[0]);
   this->_resolveNames[0] = NULL;
}


uint16_t ethutil_swaps(uint16_t i);
uint32_t ethutil_swapl(uint32_t l);

extern uint16_t ethutil_htons(unsigned short hostshort)
{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
	return ethutil_swaps(hostshort);
#else
	return hostshort;
#endif
}

extern uint32_t ethutil_htonl(unsigned long hostlong)
{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
	return ethutil_swapl(hostlong);
#else
	return hostlong;
#endif
}

extern uint16_t ethutil_ntohs(unsigned short netshort)
{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
	return ethutil_swaps(netshort);
#else
	return netshort;
#endif
}

extern uint32_t ethutil_ntohl(unsigned long netlong)
{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
	return ethutil_swapl(netlong);
#else
	return netlong;
#endif
}

uint16_t ethutil_swaps(uint16_t i)
{
	uint16_t ret=0;
	ret = (i & 0xFF) << 8;
	ret |= ((i >> 8)& 0xFF);
	return ret;
}

uint32_t ethutil_swapl(uint32_t l)
{
	uint32_t ret=0;
	ret = (l & 0xFF) << 24;
	ret |= ((l >> 8) & 0xFF) << 16;
	ret |= ((l >> 16) & 0xFF) << 8;
	ret |= ((l >> 24) & 0xFF);
	return ret;
}
