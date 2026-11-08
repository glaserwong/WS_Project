  /**************************************************************************/
  /*                                                                        */
  /*        System:  TCS PROJECT                                            */
  /*                                                                        */
  /*   Module name:  smldoc.h                                               */
  /*                                                                        */
  /*   Description:  SML Document Class                                     */
  /*                                                                        */
  /*   (c) Copyright 2005, International Business Machines-Japan Corp       */
  /*                                                                        */
  /*  Modification history:                                                 */
  /*    Date     Ver   Name        Description                              */
  /*  ---------- ----- ----------  ---------------------------------------  */
  /*  2005/01/20 L0.00 Y.KUBO      Initial release                          */
  /*  2005/04/15 L0.01 Y.KUBO      Setup Default Value ASCII = SPACE        */
  /*  2005/04/18 L0.03 Y.KUBO      Load Configuration                       */
  /*  2005/06/16 L0.24 Y.KUBO      Support Other SML Data Type              */
  /*                                                                        */
  /**************************************************************************/
  #ifndef INCLUDED_SMLDOC_H
  #define INCLUDED_SMLDOC_H

  #include "memlist.h"
  #include "hsmsmsg.h"

  // --- Define SMLTYPE
  #define SMLTYPE_ROOT    0
  #define SMLTYPE_LIST    1
  #define SMLTYPE_ASCII   2
  #define SMLTYPE_BINARY  3
  #define SMLTYPE_BOOLEAN 4  //L0.24
  #define SMLTYPE_INT1    5  //L0.24
  #define SMLTYPE_INT2    6  //L0.24
  #define SMLTYPE_INT4    7  //L0.24
  #define SMLTYPE_FLOAT8  8  //L0.24
  #define SMLTYPE_FLOAT4  9  //L0.24
  #define SMLTYPE_UINT1   10 //L0.24
  #define SMLTYPE_UINT2   11 //L0.24
  #define SMLTYPE_UINT4   12 //L0.24

  // --- SML Name List (SMLNAME)
  struct SMLNAME {
      unsigned char type;
      unsigned char fix;
      unsigned char str;
      unsigned char noc; //L0.05
      unsigned long size;
      char *def;
      CMemList *list;
      char name[1];
  };

  // --- SML Item List (SMLITEM)
  struct SMLITEM {
      unsigned char type;
      unsigned long size;
      char *name;
      char data[1];
  };

  class CSMLDoc
  {

  public:
//L0.03      CSMLDoc( char *file, char *name );
      CSMLDoc( char *name ); //L0.03
      virtual ~CSMLDoc();
      int LoadSML( char *file, char *name );
      int Parse( CHSMSMsg *hsmsmsg );
      int Build( CHSMSMsg *hsmsmsg );
//L0.01      int GetData( char *buffer, int size, char *name, ... );
      int GetString( char *buffer, int size, char *name, ... ); //L0.01
      int GetByte( char *buffer, int size, char *name, ... );   //L0.01
      int GetSize( char *name, ... );
//L0.01      int SetData( char *buffer, int size, char *name, ... );
      int SetString( char *buffer, char *name, ... );           //L0.01
      int SetByte( char *buffer, int size, char *name, ... );   //L0.01
      int SetSize( int size, char *name, ... );

  private:
      int GetData( char *buffer, int size, char *itemname ); //L0.01
      int SetData( char *buffer, int size, char *itemname ); //L0.01
      int LoadSMLRoot( char *name );
      int LoadSMLList( CMemList *p_smlname );
      int ParseSECS( CMemList *smllist, char *pname, unsigned char **secsmsg );
      int BuildSECS( CMemList *smllist, char *pname, unsigned char **secsmsg, int *secslen );
      unsigned char GetSECSType( unsigned char **secsmsg, int *size );
      unsigned long GetSECSSize( unsigned char **secsmsg, int size );
      int SetSECSType( unsigned char **secsmsg, int *secslen, unsigned char type, unsigned long size );
      int SetSECSSize( unsigned char **secsmsg, int *secslen, unsigned long size );
      void FreeName( CMemList *smllist );
      void FreeItem( CMemList *smllist );
      char * CopyWord( char *dstbuf, char *orgbuf );
      void CBuffer2SECS( int type, unsigned char *secs, unsigned char *data, long size ); //L0.24
      void SECS2CBuffer( int type, unsigned char *data, unsigned char *secs, long size ); //L0.24

  private:
      char * m_sepwd[32];
      char m_sepch[32];
      FILE *m_fp;
      int m_line;
      char m_sfname[32];
      int m_waitbit;
      int m_noitem;
      CMemList m_smlname;
      CMemList m_smlitem;

  };

  #endif
