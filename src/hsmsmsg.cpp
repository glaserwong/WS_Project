#include <string.h>
#include <ctime>
#include "hsmsmsg.h"

CHSMSMsg::CHSMSMsg( int mode )
{
   memset(m_hsmshdr, 0, sizeof(m_hsmshdr));
   memset(m_sfname, 0, sizeof(m_sfname));
   m_bodybuf = NULL;
   m_bodylen = 0;
   m_recvtime = timeval{};
}

CHSMSMsg::~CHSMSMsg()
{
   if( m_bodybuf != NULL ){
      delete[] m_bodybuf;
   }
}

int CHSMSMsg::Alloc( char *hsmshdr )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CHSMSMsg::Copy( CHSMSMsg *org )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

unsigned long CHSMSMsg::GetMessageLen( void )
{
   return m_bodylen;
}

void CHSMSMsg::SetMessageLen( unsigned long hsmslen )
{
   m_bodylen = hsmslen;
   return;
}

unsigned short CHSMSMsg::GetSessionID( void )
{
   return static_cast<unsigned short>((unsigned(m_hsmshdr[4]) << 8) | m_hsmshdr[5]);
}

void CHSMSMsg::SetSessionID( unsigned short sessionid )
{
   m_hsmshdr[4] = static_cast<unsigned char>(sessionid >> 8);
   m_hsmshdr[5] = static_cast<unsigned char>(sessionid);
   return;
}

void CHSMSMsg::SetSessionID( void )
{
   return;
}

unsigned long CHSMSMsg::GetSystemByte( void )
{
   return (uint32_t(m_hsmshdr[10]) << 24) | (uint32_t(m_hsmshdr[11]) << 16) | (uint32_t(m_hsmshdr[12]) << 8) | m_hsmshdr[13];
}

void CHSMSMsg::SetSystemByte( unsigned long sysbyte )
{
   for (int i = 0; i < 4; ++i) m_hsmshdr[13-i] = static_cast<unsigned char>(sysbyte >> (8*i));
   return;
}

void CHSMSMsg::SetSystemByte( void )
{
   return;
}

void CHSMSMsg::SetStreamFunction( char *sfname )
{
   return;
}

void CHSMSMsg::SetWaitBit( void )
{
   m_hsmshdr[ HSMS_HDR_BYTE_2 ] |= 0x80;
   return;
}

unsigned char CHSMSMsg::GetSelectStatus( void )
{
   return 0x00;
}

void CHSMSMsg::SetSelectStatus( unsigned char status )
{
   return;
}

int CHSMSMsg::IsCtrlMsg( void )
{
   return m_hsmshdr[9] != 0;
}

int CHSMSMsg::IsPrimMsg( void )
{
   return m_hsmshdr[9] == 0 && (m_hsmshdr[7] & 1);
}

int CHSMSMsg::IsWaitBit( void )
{
   return (m_hsmshdr[6] & 0x80) != 0;
}

int CHSMSMsg::IsDeviceID( void )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CHSMSMsg::CtlType( void )
{
   return m_hsmshdr[9];
}

void CHSMSMsg::SetTime( void )
{
   return;
}

int CHSMSMsg::IsTimeout( void )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

void CHSMSMsg::Dump( void )
{
   return;
}

int CHSMSMsg::TreeDump( char* &, int&, int& )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CHSMSMsg::Dump2Str( char *name, int cnt, char **str, char **msg )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CHSMSMsg::Dump2Str_Type( int type, int size, char **str, char **msg )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CHSMSMsg::Dump2StrLen( char *name, int cnt, int *str, char **msg )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CHSMSMsg::Dump2StrLen_Type( int type, int size, int *str, char **msg )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CHSMSMsg::SECSType( char **msg, int *size )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CHSMSMsg::SECSSize( char **msg, int size )
{
   return -1; // Original implementation absent; explicitly unsupported.
}
