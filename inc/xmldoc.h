  /**************************************************************************/
  /*                                                                        */
  /*        System:  TCS PROJECT                                            */
  /*                                                                        */
  /*   Module name:  xmldoc.h                                               */
  /*                                                                        */
  /*   Description:  XML Document Class                                     */
  /*                                                                        */
  /*   (c) Copyright 2005, International Business Machines-Japan Corp       */
  /*                                                                        */
  /*  Modification history:                                                 */
  /*    Date     Ver   Name        Description                              */
  /*  ---------- ----- ----------  ---------------------------------------  */
  /*  2005/01/20 L0.00 Y.KUBO      Initial release                          */
  /*  2005/04/15 L0.01 Y.KUBO      Setup Default Value ASCII = SPACE        */
  /*  2005/11/09 L0.46 Y.KUBO      Performance Tune                         */
  /*  2008/07/24 L2.12 Y.KUBO      Support XML Property (AUO)               */
  /*  2008/12/22 L2.14 Y.KUBO      CHG Support CDATA Section                */
  /*                                                                        */
  /**************************************************************************/
  #ifndef INCLUDED_XMLDOC_H
  #define INCLUDED_XMLDOC_H

  #include "memlist.h"
  #include "mqmsg.h"

  // --- XML Item List (XMLITEM)
  struct XMLITEM {
      char *itemname;
      CMemList *nodelist;
      CMemList *attrlist;
      int length;
      char data[1];
  };

  // --- Build Mode (Packing)
  #define BM_COMPRESS_DATA 0
  #define BM_EXPAND_DATA   1

  class CXMLDoc
  {

  public:
      CXMLDoc( char *doctype = NULL );
      virtual ~CXMLDoc();
      int Parse( CMQMsg *mqmsg );
      int Build( CMQMsg *mqmsg );
//L0.01      int GetData( char *buffer, int size, char *name, ... );
//L0.01      int SetData( char *buffer, int size, char *name, ... );
      void *SetNode( void *nodelist, char *itemname );               //L0.46
      int SetString( void *node, char *buffer, char *name );         //L0.46
      int SetByte( void *node, char *buffer, int size, char *name ); //L0.46
      int SetProperty( void *node, char *buffer, char *name ); //L2.12
      int GetString( char *buffer, int size, char *name, ... ); //L0.01
      int GetByte( char *buffer, int size, char *name, ... );   //L0.01
      int SetString( char *buffer, char *name, ... );           //L0.01
      int SetByte( char *buffer, int size, char *name, ... );   //L0.01
      int SetProperty( char *buffer, char *name, ... ); //L2.12

  private:
      int GetData( char *buffer, int size, char *itemname ); //L0.01
//L2.12      int SetData( char *buffer, int size, char *itemname ); //L0.01
      int SetData( char *buffer, int size, char *itemname, int property = 0 ); //L2.12
      int FreeItem( CMemList *xmllist );
      int ParseNode( CMemList *xmllist, char **buffer );
      int ParseComment( char **buffer );
      int ParseDoctype( char **buffer );
      int ParseSpec( char **buffer );
      int ParseItem( CMemList *xmlitem, char **buffer );
      int ParseItemStart( char **buffer, char *itemname, CMemList **attrlist );
      int ParseTagData( CMemList *xmllist, char **buffer, char *itemname, CMemList *attrlist );
      int ParseItemEnd( char **buffer, char *itemname );
      int ParseTagEmpty( CMemList *xmllist, char *itemname, CMemList *attrlist );
      int ParseAttr( CMemList *attrlist, char **buffer );
      int ParseAttrName( char **buffer, char *itemname, char *blockmark );
      int ParseAttrData( CMemList *attrlist, char **buffer, char *itemname, char blockmark );
//L2.14      int ParseData( char *orig, char *dest, char separater );
      int ParseData( char *orig, char *dest, char separater, char **nextp ); //L2.14
      int ParseDataLen( char *orig, int *dest, char separater );
      int DumpRoot( char **xmlstr );
      int DumpNode( CMemList *xmllist, char **xmlstr );
      int DumpAttr( CMemList *attrlist, char **xmlstr );
      int DumpData( char *data, char **xmlstr );
      int DumpRootLen( int *xmlstr );
      int DumpNodeLen( CMemList *xmllist, int *xmlstr );
      int DumpAttrLen( CMemList *attrlist, int *xmlstr );
      int DumpDataLen( char *data, int *xmlstr );
      XMLITEM *GetItem( CMemList *nodelist, CMemList *attrlist, char *name );
//L2.12      XMLITEM *SetItem( CMemList *nodelist, char *name, int length );
      XMLITEM *SetItem( XMLITEM *nodeitem, char *name, int length, int property ); //L2.12
      XMLITEM *SearchItem( CMemList *xmllist, char *itemname, int index );
      XMLITEM *BuildItem( CMemList *xmllist, char *itemname, int length );
      int DataCompress( char *data, int length );
      int DataExpand( char *data, int length );

  private:
      CMemList m_xmlitem;
      char m_doctype[128];
      char m_dtdname[128];
      char m_specval[2][128];
      int m_bmmode;

  };

  #endif

