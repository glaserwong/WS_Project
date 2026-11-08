#include "platform_time.h"
  /**************************************************************************/
  /*                                                                        */
  /*        System:  TCS PROJECT                                            */
  /*                                                                        */
  /*   Module name:  hsmsmsg.h                                              */
  /*                                                                        */
  /*   Description:  HSMS Message Class                                     */
  /*                                                                        */
  /*   (c) Copyright 2005, International Business Machines-Japan Corp       */
  /*                                                                        */
  /*  Modification history:                                                 */
  /*    Date     Ver   Name        Description                              */
  /*  ---------- ----- ----------  ---------------------------------------  */
  /*  2005/01/20 L0.00 Y.KUBO      Initial release                          */
  /*  2005/06/16 L0.24 Y.KUBO      Support Other SML Data Type              */
  /*  2005/06/28 L0.27 N.Hattori   Support Tree log                         */
  /*  2008/07/08 L2.10 Y.KUBO      Support EOS Function (AUO)               */
  /*                                                                        */
  /**************************************************************************/
  #ifndef INCLUDED_HSMSMSG_H
  #define INCLUDED_HSMSMSG_H

  // --- Define HSMSSIZE
  #define HSMS_HEADER_SIZE   (HSMS_MSG_LEN+HSMS_HDR_LEN)

  #define HSMS_MSG_LEN       4  // Message Length
  #define HSMS_HDR_LEN       10 // Header Length

  // --- Define HSMSBYTE
  #define HSMS_HDR_MSGL_1    0  // Message Length 1
  #define HSMS_HDR_MSGL_2    1  //                2
  #define HSMS_HDR_MSGL_3    2  //                3
  #define HSMS_HDR_MSGL_4    3  //                4
  #define HSMS_HDR_SSID_1    4  // Session ID 1
  #define HSMS_HDR_SSID_2    5  //            2
  #define HSMS_HDR_BYTE_2    6  // Header Byte 1
  #define HSMS_HDR_BYTE_3    7  //             2
  #define HSMS_HDR_TYPE_P    8  // P Type
  #define HSMS_HDR_TYPE_S    9  // S Type
  #define HSMS_HDR_SBYT_1    10 // System Byte 1
  #define HSMS_HDR_SBYT_2    11 //             2
  #define HSMS_HDR_SBYT_3    12 //             3
  #define HSMS_HDR_SBYT_4    13 //             4

  // --- Define CTRLTYPE
  #define HSMS_CTL_DATMSG    0  // Data     Message
  #define HSMS_CTL_SELREQ    1  // Select   Request
  #define HSMS_CTL_SELREP    2  //          Response
  #define HSMS_CTL_LNKREQ    5  // LinkTest Request
  #define HSMS_CTL_LNKREP    6  //          Response
  #define HSMS_CTL_SEPREQ    9  // Separate Request

  // --- Define SECSTYPE
  #define SECS_LIST    000 // List (SECS format)
  #define SECS_BINARY  010 // Binary
  #define SECS_BOOLEAN 011 // Boolean
  #define SECS_ASCII   020 // Ascii
  #define SECS_JIS8    021 // JIS-8
  #define SECS_CHAR2   022 // 2 byte character
  #define SECS_INT8    030 // 8 byte integer
  #define SECS_INT1    031 // 1 byte integer
  #define SECS_INT2    032 // 2 byte integer
  #define SECS_INT4    034 // 4 byte integer
  #define SECS_FLOAT8  040 // 8 byte float
  #define SECS_FLOAT4  044 // 4 byte float
  #define SECS_UINT8   050 // 8 byte unsigned integer
  #define SECS_UINT1   051 // 1 byte unsigned integer
  #define SECS_UINT2   052 // 2 byte unsigned integer
  #define SECS_UINT4   054 // 4 byte unsigned integer

  class CHSMSMsg
  {

  public:
      CHSMSMsg( int mode = HSMS_CTL_DATMSG );
      virtual ~CHSMSMsg();
      CHSMSMsg(const CHSMSMsg&) = delete;
      CHSMSMsg& operator=(const CHSMSMsg&) = delete;
      int Alloc( char *hsmshdr = NULL );
      int Copy( CHSMSMsg *org );
      unsigned long GetMessageLen( void );
      void SetMessageLen( unsigned long hsmslen );
      unsigned short GetSessionID( void );
      void SetSessionID( unsigned short sessionid );
      void SetSessionID( void );
      unsigned long GetSystemByte( void );
      void SetSystemByte( unsigned long sysbyte );
      void SetSystemByte( void );
      void SetStreamFunction( char *sfname );
      void SetWaitBit( void );
      unsigned char GetSelectStatus( void );
      void SetSelectStatus( unsigned char status ); //L2.10
      int IsCtrlMsg( void );
      int IsPrimMsg( void );
      int IsWaitBit( void );
      int IsDeviceID( void );
      int CtlType( void );
      void SetTime( void );
      int IsTimeout( void );
      void Dump( void );
      int TreeDump( char* &, int&, int& ); //L0.27

  private:
      int Dump2Str( char *name, int cnt, char **str, char **msg );
      int Dump2Str_Type( int type, int size, char **str, char **msg ); //L0.24
      int Dump2StrLen( char *name, int cnt, int *str, char **msg );
      int Dump2StrLen_Type( int type, int size, int *str, char **msg ); //L0.24
      int SECSType( char **msg, int *size );
      int SECSSize( char **msg, int size );

  public:
      unsigned char m_hsmshdr[HSMS_HEADER_SIZE];
      unsigned long m_bodylen;
      unsigned char * m_bodybuf;
      char m_sfname[32];
      struct timeval m_recvtime;

  };

  #endif

