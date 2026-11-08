#include <stdio.h>
#include <cstring>
#include <string>
#include <cstdarg>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <cstdlib>
#include "export.h"
#include "mbxmq.h"

MBXMQ gtMBXMQ;

using namespace std;

extern "C" int GetGlobal( int itemno, void *buffer )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

/**************************************************************************/
/*                                                                        */
/*  Name        :  GetConfig( )                                           */
/*                                                                        */
/*  Outline     :  Get Value from Configuration file                      */
/*                                                                        */
/*  Function    :                                                         */
/*                                                                        */
/*  Parameter   : (I) none                                                */
/*                (O) none                                                */
/*                                                                        */
/**************************************************************************/
static std::string trim(const std::string& s) {
   const size_t a = s.find_first_not_of(" \t\r\n");
   return a == std::string::npos ? "" : s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
}
extern "C" int GetConfigSized(const char* section, const char* key, char* buffer, size_t capacity) {
   if (!section || !key || !buffer || !capacity) return -1;
   buffer[0] = 0;
   const char* file = std::getenv("HSMS_CONFIG");
   std::ifstream in(file ? file : "ini/hsms.ini");
   std::string line, current;
   while (std::getline(in, line)) {
      line = trim(line);
      if (line.empty() || line[0] == ';' || line[0] == '#') continue;
      if (line.front() == '[' && line.back() == ']') { current = trim(line.substr(1, line.size()-2)); continue; }
      size_t eq = line.find('=');
      if (current == section && eq != std::string::npos && trim(line.substr(0, eq)) == key) {
         std::string value = trim(line.substr(eq+1));
         if (value.size() >= capacity) return -1;
         std::memcpy(buffer, value.c_str(), value.size()+1); return 0;
      }
   }
   return -1;
}

extern "C" int GetConfig(char* section, char* key, char* buffer) {
   return GetConfigSized(section, key, buffer, 1024);
}

extern "C" void SetLocal( void *gmem )
{
   return;
}

extern "C" void *GetLocal( void )
{
   return NULL;
}

extern "C" void SetBankData( char *name, void *data, int size )
{
   return;
}

extern "C" int GetBankData( char *name, void *data, int size )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

extern "C" void PrintLog( char *fmt, ... )
{
   va_list vaptr;

   return;
}

extern "C" void WarningLog( char *fmt, ... )
{
   va_list vaptr;

   return;
}

extern "C" void ErrorLog( char *fmt, ... )
{
   va_list vaptr;

   return;
}

extern "C" int SendHSMS( CHSMSMsg *snedmsg )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

extern "C" int RecvHSMS( CHSMSMsg *sendmsg, CHSMSMsg *recvmsg )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

extern "C" int SendRecvHSMS( CHSMSMsg *sendmsg, CHSMSMsg *recvmsg )
{
   if (!sendmsg || (sendmsg->m_bodylen && !sendmsg->m_bodybuf) || sendmsg->m_bodylen > 16777206) return -1;
   if (!std::memchr(sendmsg->m_sfname, 0, sizeof(sendmsg->m_sfname))) return -1;
   const char* name = sendmsg->m_sfname;
   int lStreamNo = 0, lFunctionNo = 0;
   if (*name++ != 'S') return -1;
   int digits = 0;
   while (*name >= '0' && *name <= '9') {
      if (++digits > 3) return -1;
      lStreamNo = lStreamNo * 10 + (*name++ - '0');
   }
   if (!digits || *name++ != 'F') return -1;
   digits = 0;
   while (*name >= '0' && *name <= '9') {
      if (++digits > 3) return -1;
      lFunctionNo = lFunctionNo * 10 + (*name++ - '0');
   }
   if (!digits || *name || lStreamNo > 127 || lFunctionNo > 255) return -1;

   //Set Message Header
   memset( sendmsg -> m_hsmshdr, 0x00, HSMS_HEADER_SIZE );
   sendmsg -> m_hsmshdr[ HSMS_HDR_BYTE_2 ] = lStreamNo + 0x80;
   sendmsg -> m_hsmshdr[ HSMS_HDR_BYTE_3 ] = lFunctionNo;

   //Set System Byte
   sendmsg -> m_hsmshdr[ HSMS_HDR_SBYT_1 ] = 0x00;
   sendmsg -> m_hsmshdr[ HSMS_HDR_SBYT_2 ] = 0x00;
   sendmsg -> m_hsmshdr[ HSMS_HDR_SBYT_3 ] = 0x00;
   sendmsg -> m_hsmshdr[ HSMS_HDR_SBYT_4 ] = 0x10;


   //Set Message Length
   const uint32_t lMsgLen = static_cast<uint32_t>(sendmsg->m_bodylen + HSMS_HDR_LEN);
   for (int ix = 0; ix < HSMS_MSG_LEN; ++ix) sendmsg->m_hsmshdr[ix] = static_cast<unsigned char>(lMsgLen >> (8 * (3-ix)));

   //Output Message
   cout.write( ( char* )sendmsg -> m_hsmshdr, HSMS_HEADER_SIZE );
   if( sendmsg -> m_bodylen > 0 && sendmsg -> m_bodybuf != NULL ){
      cout.write( ( char* )sendmsg -> m_bodybuf, sendmsg -> m_bodylen );
   }
   cout << flush;

   return cout.good() ? 0 : -1;
}

extern "C" int ReplyHSMS( CHSMSMsg *recvmsg, CHSMSMsg *sendmsg )
{
   if (!recvmsg || !sendmsg || (sendmsg->m_bodylen && !sendmsg->m_bodybuf) || sendmsg->m_bodylen > 16777206) return -1;
   if (recvmsg->m_hsmshdr[HSMS_HDR_BYTE_3] == 255) return -1;
   //Set Message Header
   memcpy( sendmsg -> m_hsmshdr, recvmsg -> m_hsmshdr, HSMS_HEADER_SIZE );
   sendmsg -> m_hsmshdr[ HSMS_HDR_BYTE_2 ] &= 0x7F;
   sendmsg -> m_hsmshdr[ HSMS_HDR_BYTE_3 ] += 0x01;

   //Set Message Length
   const uint32_t lMsgLen = static_cast<uint32_t>(sendmsg->m_bodylen + HSMS_HDR_LEN);
   for (int ix = 0; ix < HSMS_MSG_LEN; ++ix) sendmsg->m_hsmshdr[ix] = static_cast<unsigned char>(lMsgLen >> (8 * (3-ix)));

   //Output Message
   cout.write( ( char* )sendmsg -> m_hsmshdr, HSMS_HEADER_SIZE );
   if( sendmsg -> m_bodylen > 0 && sendmsg -> m_bodybuf != NULL ){
      cout.write( ( char* )sendmsg -> m_bodybuf, sendmsg -> m_bodylen );
   }
   cout << flush;

   return cout.good() ? 0 : -1;
}

extern "C" int SendMQ( CMQMsg *sendmsg, char *quename )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

extern "C" int PostMQ( CMQMsg *sendmsg, char *quename )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

extern "C" int RecvMQ( CMQMsg *sendmsg, CMQMsg *recvmsg )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

extern "C" int SendRecvMQ( CMQMsg *sendmsg, CMQMsg *recvmsg, char *quename )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

extern "C" int ReplyMQ( CMQMsg *recvmsg, CMQMsg *sendmsg )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

extern "C" int PostIM( CMQMsg *sendmsg, int timer )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

extern "C" int DBCommit( void )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

extern "C" int DBRollback( void )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

//extern "C" int ConnectMQ( MBXHMBX *hconn, char *mqmname, char *conname )
//{
//	
//	int            lRc;
//	
//	MBXMQ *tMBXMQ = new MBXMQ();
//	
//  lRc = tMBXMQ->Conn( mqmname, conname );
//  hconn = tMBXMQ->con_handle;
//  
//   return lRc;
//}
//
//
//extern "C" int DisconnectMQ( MBXHMBX *hconn )
//{
//   return 0;
//}
//
//extern "C" int OpenMQ( MBXHMBX hconn, MBXHMB *hobj, char *quename, int openmode )
//{
//   return 0;
//}
//
//extern "C" int CloseMQ( MBXHMBX hconn, MBXHMB *hobj )
//{
//   return 0;
//}
//
//extern "C" int PutMQ( MBXHMBX hconn, MBXHMB hobj, CMQMsg *mqmsg )
//{
//   return 0;
//}
//
//extern "C" int GetMQ( MBXHMBX hconn, MBXHMB hobj, CMQMsg *mqmsg, char *msgid1, char *msgid2, int timer )
//{
//   return 0;
//}



extern "C" int ConnectMQ( MBXHMBX *hconn, char *mqmname, char *conname )
{
   return gtMBXMQ.Conn( hconn, mqmname, conname );
}

extern "C" int DisconnectMQ( MBXHMBX *hconn )
{
   return gtMBXMQ.Disc( hconn );
}

extern "C" int OpenMQ( MBXHMBX hconn, MBXHMB *hobj, char *quename, int openmode )
{
	 long lRc;
   lRc= gtMBXMQ.Open( hconn, hobj, quename, openmode );
   return lRc;
}

extern "C" int CloseMQ( MBXHMBX hconn, MBXHMB *hobj )
{
   return gtMBXMQ.Close( hconn, hobj );
}

extern "C" int PutMQ( MBXHMBX hconn, MBXHMB hobj, CMQMsg *mqmsg )
{
   return gtMBXMQ.Send( hconn, hobj, mqmsg );
}

extern "C" int GetMQ( MBXHMBX hconn, MBXHMB hobj, CMQMsg *mqmsg, char *msgid1, char *msgid2, int timer )
{
   return gtMBXMQ.Recv( hconn, hobj, mqmsg, msgid1, msgid2, timer );
}

