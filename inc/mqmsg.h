#include "platform_time.h"
  /**************************************************************************/
  /*                                                                        */
  /*        System:  TCS PROJECT                                            */
  /*                                                                        */
  /*   Module name:  mqmsg.h                                                */
  /*                                                                        */
  /*   Description:  MQ Message Class                                       */
  /*                                                                        */
  /*   (c) Copyright 2005, International Business Machines-Japan Corp       */
  /*                                                                        */
  /*  Modification history:                                                 */
  /*    Date     Ver   Name        Description                              */
  /*  ---------- ----- ----------  ---------------------------------------  */
  /*  2005/01/20 L0.00 Y.KUBO      Initial release                          */
  /*  2005/06/28 L0.27 N.Hattori   Support Tree log                         */
  /*  2005/08/08 L0.36 Y.KUBO      Support Delaied Immidiate Message        */
  /*  2009/06/15 L2.15 Y.KUBO      CHG MQ->MBX                              */
  /*                                                                        */
  /**************************************************************************/
  #ifndef INCLUDED_MQMSG_H
  #define INCLUDED_MQMSG_H

//L2.15  #include <cmqc.h>
//L2.15  #include <cmqxc.h>
  #include "mbx.h" //L2.15

  class CMQMsg
  {

  public:
      CMQMsg();
      virtual ~CMQMsg();
      CMQMsg(const CMQMsg&) = delete;
      CMQMsg& operator=(const CMQMsg&) = delete;
      int Alloc( int msglen, char *msgbuf = NULL );
      char * GetFunction( char *trxid );
      void Dump( void );
      int TreeDump( char* &, int&, int& ); //L0.27
      void SetReleaseTime( int delay = 0 ); //L0.36

  public:
//L2.15      MQMD m_msgdesc;
//L2.15      MQCHAR m_mqmname[128];
//L2.15      MQCHAR m_conname[128];
//L2.15      MQCHAR m_quename[128];
//L2.15      MQCHAR *m_msgbuf;
//L2.15      MQLONG m_msglen;
      MBXMD m_msgdesc;        //L2.15
      MBXCHAR m_mqmname[128]; //L2.15
      MBXCHAR m_conname[128]; //L2.15
      MBXCHAR m_quename[128]; //L2.15
      MBXCHAR *m_msgbuf;      //L2.15
      MBXLONG m_msglen;       //L2.15
      struct timeval m_reltime; //L0.36

  };

  #endif
