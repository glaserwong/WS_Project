#include <stdexcept>
//#include <stdio.h>
#include <string.h>
#include <ctime>
#include "xmldoc.h"

CXMLDoc::CXMLDoc( char *doctype )
{
   memset(m_doctype, 0, sizeof(m_doctype));
   memset(m_dtdname, 0, sizeof(m_dtdname));
   memset(m_specval, 0, sizeof(m_specval));
   m_bmmode = BM_COMPRESS_DATA;
   if (doctype) {
      if (strlen(doctype) >= sizeof(m_doctype)) throw std::length_error("Document type too long");
      strcpy(m_doctype, doctype);
   }
}

CXMLDoc::~CXMLDoc()
{
//   m_xmlitem.Clear();
}

int CXMLDoc::Parse( CMQMsg *mqmsg )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::Build( CMQMsg *mqmsg )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

void* CXMLDoc::SetNode( void *nodelist, char *itemname )
{
   return NULL;
}

int CXMLDoc::SetString( void *node, char *buffer, char *name )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::SetByte( void *node, char *buffer, int size, char *name )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::SetProperty( void *node, char *buffer, char *name )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::GetString( char *buffer, int size, char *name, ... )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::GetByte( char *buffer, int size, char *name, ... )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::SetString( char *buffer, char *name, ... )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::SetByte( char *buffer, int size, char *name, ... )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::SetProperty( char *buffer, char *name, ... )
{
   return -1; // Original implementation absent; explicitly unsupported.
}



int CXMLDoc::GetData( char *buffer, int size, char *itemname )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::SetData( char *buffer, int size, char *itemname, int property )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::FreeItem( CMemList *xmllist )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::ParseNode( CMemList *xmllist, char **buffer )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::ParseComment( char **buffer )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::ParseDoctype( char **buffer )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::ParseSpec( char **buffer )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::ParseItem( CMemList *xmlitem, char **buffer )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::ParseItemStart( char **buffer, char *itemname, CMemList **attrlist )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::ParseTagData( CMemList *xmllist, char **buffer, char *itemname, CMemList *attrlist )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::ParseItemEnd( char **buffer, char *itemname )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::ParseTagEmpty( CMemList *xmllist, char *itemname, CMemList *attrlist )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::ParseAttr( CMemList *attrlist, char **buffer )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::ParseAttrName( char **buffer, char *itemname, char *blockmark )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::ParseAttrData( CMemList *attrlist, char **buffer, char *itemname, char blockmark )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::ParseData( char *orig, char *dest, char separater, char **nextp )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::ParseDataLen( char *orig, int *dest, char separater )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::DumpRoot( char **xmlstr )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::DumpNode( CMemList *xmllist, char **xmlstr )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::DumpAttr( CMemList *attrlist, char **xmlstr )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::DumpData( char *data, char **xmlstr )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::DumpRootLen( int *xmlstr )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::DumpNodeLen( CMemList *xmllist, int *xmlstr )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::DumpAttrLen( CMemList *attrlist, int *xmlstr )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int CXMLDoc::DumpDataLen( char *data, int *xmlstr )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

XMLITEM* CXMLDoc::GetItem( CMemList *nodelist, CMemList *attrlist, char *name )
{
   return NULL;
}

XMLITEM* CXMLDoc::SetItem( XMLITEM *nodeitem, char *name, int length, int property )
{
   return NULL;
}

XMLITEM* CXMLDoc::SearchItem( CMemList *xmllist, char *itemname, int index )
{
   return NULL;
}

XMLITEM* CXMLDoc::BuildItem( CMemList *xmllist, char *itemname, int length )
{
   return NULL;
}

int DataCompress( char *data, int length )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

int DataExpand( char *data, int length )
{
   return -1; // Original implementation absent; explicitly unsupported.
}

