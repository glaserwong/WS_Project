  /**************************************************************************/
  /*                                                                        */
  /*        System:  TCS PROJECT                                            */
  /*                                                                        */
  /*   Module name:  waitmsg.h                                              */
  /*                                                                        */
  /*   Description:  Waiting 2nd Message Class                              */
  /*                                                                        */
  /*   (c) Copyright 2005, International Business Machines-Japan Corp       */
  /*                                                                        */
  /*  Modification history:                                                 */
  /*    Date     Ver   Name        Description                              */
  /*  ---------- ----- ----------  ---------------------------------------  */
  /*  2005/01/20 L0.00 Y.KUBO      Initial release                          */
  /*                                                                        */
  /**************************************************************************/
  #ifndef INCLUDED_WAITMSG_H
  #define INCLUDED_WAITMSG_H

  #define WAITMSG_SIGNAL_NORMAL  0
  #define WAITMSG_SIGNAL_STOP    1

  #define WAITMSG_STATUS_NORMAL  1
  #define WAITMSG_STATUS_TIMEOUT 2

  #include "hsmsmsg.h"
  #include <mutex>
  #include <condition_variable>

  class CWaitMsg
  {

  public:
      CWaitMsg( CHSMSMsg *msg );
      virtual ~CWaitMsg();
      int Wait( CHSMSMsg **msg );
      int Set( int status, CHSMSMsg *msg = NULL );
      int Signal( int signal );

  public:
      char m_sfname[32];
      unsigned long m_systembyte;
      int m_status;
      CHSMSMsg * m_msg;
      int m_locked;
      struct timeval m_crttime;

  private:
      std::mutex m_mutex;
      std::condition_variable m_cond;
      int m_signal;

  };

  #endif
