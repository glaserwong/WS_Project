#include <string.h>
#include <ctime>
#include "mqmsg.h"


CMQMsg::CMQMsg( )
{
   m_msgdesc = MBXMD{};
   m_msgbuf = NULL;
   m_msglen = 0;
   memset(m_mqmname, 0, sizeof(m_mqmname));
   memset(m_conname, 0, sizeof(m_conname));
   memset(m_quename, 0, sizeof(m_quename));
   m_reltime = timeval{};
}

CMQMsg::~CMQMsg()
{
   if( m_msgbuf != NULL ){
   	  delete[] m_msgbuf;
   	  m_msgbuf = NULL;
   }
}

int CMQMsg::Alloc( int msglen, char *msgbuf )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

char* CMQMsg::GetFunction( char *trxid )
{
   return NULL;
}

void CMQMsg::Dump( void )
{
   return ;
}

int CMQMsg::TreeDump( char* &, int&, int& )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

void CMQMsg::SetReleaseTime( int delay )
{
   return ;
}
