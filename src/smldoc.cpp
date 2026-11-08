#include <stdexcept>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <ctime>
#include "smldoc.h"

CSMLDoc::CSMLDoc( char *name )
{
   memset( m_sfname, 0x00, sizeof( m_sfname ) );
   if (name && strlen(name) < sizeof(m_sfname)) strcpy(m_sfname, name);
   else throw std::invalid_argument("Invalid SML name");
}

CSMLDoc::~CSMLDoc()
{
   m_smlname.Clear();
   m_smlitem.Clear();
}

int CSMLDoc::LoadSML( char *file, char *name )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CSMLDoc::Parse( CHSMSMsg *hsmsmsg )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CSMLDoc::Build( CHSMSMsg *hsmsmsg )
{
   if (!hsmsmsg) return -1;
   //Copy Stream Function Name
   if( memcmp( hsmsmsg -> m_sfname, m_sfname, sizeof( hsmsmsg -> m_sfname ) ) != 0 ){
      memcpy( hsmsmsg -> m_sfname, m_sfname, sizeof( hsmsmsg -> m_sfname ) );
   }

   struct ml_item *tmpMEMItem = ( struct ml_item * )m_smlitem.Top( NULL );
   hsmsmsg -> m_bodylen = 0;

   delete[] hsmsmsg->m_bodybuf;
   hsmsmsg->m_bodybuf = NULL;

   //Get Message Length
   for( int ix = 0 ; ix < m_smlitem.Count() ; ix++ ){
      const SMLITEM* item = reinterpret_cast<const SMLITEM*>(tmpMEMItem->mbr);
      if (item->size > 255) return -1; // This legacy builder supports one length byte.
      hsmsmsg -> m_bodylen += item->size + 2;

      if( tmpMEMItem -> nxt == NULL ){
         break;
      }
      tmpMEMItem = tmpMEMItem -> nxt;
   }

   //Set Message Data
   if( hsmsmsg -> m_bodylen > 0 ){
      hsmsmsg -> m_bodybuf = new unsigned char[ hsmsmsg -> m_bodylen ];

      unsigned char *ptr = hsmsmsg -> m_bodybuf;

      tmpMEMItem = ( struct ml_item * )m_smlitem.Top( NULL );

      for( int ix = 0 ; ix < m_smlitem.Count() ; ix++ ){
         struct SMLITEM *tmpSMLItem = ( struct SMLITEM * )tmpMEMItem -> mbr;

         *ptr = tmpSMLItem -> type;
         ptr++;

         *ptr = ( unsigned char )tmpSMLItem -> size;
         ptr++;

         memcpy( ptr, tmpSMLItem -> data, tmpSMLItem -> size );
         ptr += tmpSMLItem -> size;

         if( tmpMEMItem -> nxt == NULL ){
            break;
         }
         tmpMEMItem = tmpMEMItem -> nxt;
      }
   }

   return 0;
}

int CSMLDoc::GetString( char *buffer, int size, char *name, ... )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CSMLDoc::GetByte( char *buffer, int size, char *name, ... )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CSMLDoc::GetSize( char *name, ... )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CSMLDoc::SetString( char *buffer, char *name, ... )
{
   if (!buffer || strlen(buffer) > 255) return -1;
   std::unique_ptr<char[]> storage(new char[sizeof(SMLITEM) + strlen(buffer)]{});
   char* tmpBuff = storage.get();
   
   struct SMLITEM *tmpItem = ( struct SMLITEM * )tmpBuff;

   tmpItem -> type = 0x41;

   tmpItem -> size = strlen( buffer );

   tmpItem -> name = NULL;
   // Name ownership was dangling after shallow copy; unused in this partial builder.

   memcpy( tmpItem -> data, buffer, strlen( buffer ) );

   m_smlitem.Add( tmpItem, sizeof( struct SMLITEM ) + strlen( buffer ), NULL );

   if( tmpItem -> name != NULL ){
   	  delete[] tmpItem -> name;
   }
   // storage releases the temporary even if m_smlitem.Add throws.

   return 0;
}

int CSMLDoc::SetByte( char *buffer, int size, char *name, ... )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CSMLDoc::SetSize( int size, char *name, ... )
{
   return -1; // Original implementation absent; explicitly unsupported.
}


int CSMLDoc::GetData( char *buffer, int size, char *itemname )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CSMLDoc::SetData( char *buffer, int size, char *itemname )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CSMLDoc::LoadSMLRoot( char *name )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CSMLDoc::LoadSMLList( CMemList *p_smlname )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CSMLDoc::ParseSECS( CMemList *smllist, char *pname, unsigned char **secsmsg )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CSMLDoc::BuildSECS( CMemList *smllist, char *pname, unsigned char **secsmsg, int *secslen )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

unsigned char CSMLDoc::GetSECSType( unsigned char **secsmsg, int *size )
{
   return 0x00;
}

unsigned long CSMLDoc::GetSECSSize( unsigned char **secsmsg, int size )
{
   return 0x00;
}

int CSMLDoc::SetSECSType( unsigned char **secsmsg, int *secslen, unsigned char type, unsigned long size )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CSMLDoc::SetSECSSize( unsigned char **secsmsg, int *secslen, unsigned long size )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

void CSMLDoc::FreeName( CMemList *smllist )
{
   return;
}

void CSMLDoc::FreeItem( CMemList *smllist )
{
   return;
}

char* CSMLDoc::CopyWord( char *dstbuf, char *orgbuf )
{
   return NULL;
}

void CSMLDoc::CBuffer2SECS( int type, unsigned char *secs, unsigned char *data, long size )
{
   return;
}

void CSMLDoc::SECS2CBuffer( int type, unsigned char *data, unsigned char *secs, long size )
{
   return;
}
