#include <cstddef>
  /**************************************************************************/
  /*                                                                        */
  /*        System:  TCS PROJECT                                            */
  /*                                                                        */
  /*   Module name:  export.h                                               */
  /*                                                                        */
  /*   Description:  Export Function                                        */
  /*                                                                        */
  /*   (c) Copyright 2005, International Business Machines-Japan Corp       */
  /*                                                                        */
  /*  Modification history:                                                 */
  /*    Date     Ver   Name        Description                              */
  /*  ---------- ----- ----------  ---------------------------------------  */
  /*  2005/01/20 L0.00 Y.KUBO      Initial release                          */
  /*  2005/04/19 L0.04 Y.KUBO      Provide Glaobal Values                   */
  /*  2005/04/20 L0.06 Y.KUBO      Add Function ErrorLog                    */
  /*  2005/01/21 L0.08 Y.KUBO      Get Local Host Name                      */
  /*  2005/06/14 L0.16 Y.KUBO      Unique Correlation Id & Link Check       */
  /*  2005/06/14 L0.21 Y.KUBO      Expand MQ Send Function                  */
  /*  2005/06/15 L0.23 Y.KUBO      Internal(Interleave) Transaction         */
  /*  2005/07/28 L0.33 N.Hattori   New Function VErrorMsg, [V]WarningMsg    */
  /*  2005/07/29 L0.35 Y.KUBO      Support Unknown Function                 */
  /*  2005/08/08 L0.36 Y.KUBO      Support Delaied Immidiate Message        */
  /*  2005/09/24 L0.43 Y.KUBO      Support Error Function for HSMS          */
  /*  2006/06/16 L0.55 Y.KUBO      Add New Class CNamedBank, CLargeData     */
  /*  2008/07/08 L2.10 Y.KUBO      Support EOS Function (AUO)               */
  /*  2009/06/15 L2.15 Y.KUBO      CHG MQ->MBX                              */
  /*  2010/07/07 L2.16 ISCJ        FIX correct wrong definition             */
  /*                                                                        */
  /**************************************************************************/
  #ifndef INCLUDED_EXPORT_H
  #define INCLUDED_EXPORT_H

  #include "smldoc.h"
  #include "xmldoc.h"
  #ifndef __NODBSUPPORT__   //L0.34
  #  include "db2sql.h"
  #endif                    //L0.34

  #include "waitmsg.h" //L0.43

  #define HSMSCTL_ERROR_DUPLICATE_SYSBYTE 1 //L0.43
  #define HSMSCTL_ERROR_INVALID_DEVICEID  2 //L0.43
  #define HSMSCTL_ERROR_TIMEOUT_T3        3 //L0.43
  #define HSMSCTL_ERROR_UNKNOWN_1STMSG    4 //L0.43
  #define HSMSCTL_ERROR_UNKNOWN_2NDMSG    5 //L0.43

  // --- External Definitions
  #define ITEM_EQPTNAME   1 //L0.04
  #define ITEM_PEERADDR   2 //L0.08
  #define ITEM_PEERNAME   3 //L0.08
  #define ITEM_HOSTADDR   4 //L0.08
  #define ITEM_HOSTNAME   5 //L0.08
  #define ITEM_CORRELID   6 //L0.16
  #define ITEM_APPLVERS   7 //L0.35
  #define ITEM_CONFNAME   8 //L0.35
  #define ITEM_PFILENAM   9 //L0.35
  #define ITEM_APLOGNAM  10 //L0.35
  #define ITEM_CMLOGNAM  11 //L0.35

  // --- External Functions
  extern "C" int GetGlobal( int itemno, void *buffer );               //L0.04
  // Legacy buffer must have capacity >= 1024 bytes. New callers should use GetConfigSized.
  extern "C" int GetConfig( char *section, char *key, char *buffer );
  extern "C" int GetConfigSized(const char* section, const char* key, char* buffer, size_t capacity);
//L0.04  extern "C" void SetGlobal( void *gmem );
//L0.04  extern "C" void *GetGlobal( void );
  extern "C" void SetLocal( void *gmem ); //L0.04
  extern "C" void *GetLocal( void );      //L0.04
  extern "C" void SetBankData( char *name, void *data, int size ); //L0.55
  extern "C" int GetBankData( char *name, void *data, int size ); //L0.55
  extern "C" void PrintLog( char *fmt, ... );
  extern "C" void WarningLog( char *fmt, ... ); //L0.33
  extern "C" void ErrorLog( char *fmt, ... ); //L0.06
  extern "C" int SendHSMS( CHSMSMsg *snedmsg );
  extern "C" int RecvHSMS( CHSMSMsg *sendmsg, CHSMSMsg *recvmsg );
  extern "C" int SendRecvHSMS( CHSMSMsg *sendmsg, CHSMSMsg *recvmsg );
  extern "C" int ReplyHSMS( CHSMSMsg *recvmsg, CHSMSMsg *sendmsg );
//L0.21  extern "C" int SendMQ( CMQMsg *sendmsg );
  extern "C" int SendMQ( CMQMsg *sendmsg, char *quename = NULL ); //L0.21
  extern "C" int PostMQ( CMQMsg *sendmsg, char *quename ); //L0.21
  extern "C" int RecvMQ( CMQMsg *sendmsg, CMQMsg *recvmsg );
//L0.21  extern "C" int SendRecvMQ( CMQMsg *sendmsg, CMQMsg *recvmsg );
  extern "C" int SendRecvMQ( CMQMsg *sendmsg, CMQMsg *recvmsg, char *quename = NULL ); //L0.21
  extern "C" int ReplyMQ( CMQMsg *recvmsg, CMQMsg *sendmsg );
//L0.36  extern "C" int PostIM( CMQMsg *sendmsg ); //L0.23
  extern "C" int PostIM( CMQMsg *sendmsg, int timer = 0 ); //L0.36
  extern "C" int DBCommit( void );
  extern "C" int DBRollback( void );

//L2.15  extern "C" int ConnectMQ( MQHCONN *hconn, char *mqmname, char *conname );          //L2.10
//L2.15  extern "C" int DisconnectMQ( MQHCONN *hconn );                                     //L2.10
//L2.15  extern "C" int OpenMQ( MQHCONN hconn, MQHOBJ *hobj, char *quename, int openmode ); //L2.10
//L2.15  extern "C" int CloseMQ( MQHCONN hconn, MQHOBJ *hobj );                             //L2.10
//L2.15  extern "C" int PutMQ( MQHCONN hconn, MQHOBJ hobj, CMQMsg *mqmsg );                 //L2.10
//L2.15  extern "C" int GetMQ( MQHCONN hconn, MQHOBJ hobj, CMQMsg *mqmsg, int timer );      //L2.10
//L2.16  extern "C" int ConnectMBX( MBXHMBX *hconn, char *mqmname, char *conname );          //L2.15
//L2.16  extern "C" int DisconnectMBX( MBXHMBX *hconn );                                     //L2.15
//L2.16  extern "C" int OpenMBX( MBXHMBX hconn, MBXHMB *hobj, char *quename, int openmode ); //L2.15
//L2.16  extern "C" int CloseMBX( MBXHMBX hconn, MBXHMB *hobj );                             //L2.15
//L2.16  extern "C" int PutMBX( MBXHMBX hconn, MBXHMB hobj, CMQMsg *mqmsg );                 //L2.15
//L2.16  extern "C" int GetMBX( MBXHMBX hconn, MBXHMB hobj, CMQMsg *mqmsg, int timer );      //L2.15

  extern "C" int ConnectMQ( MBXHMBX *hconn, char *mqmname, char *conname );                                 //L2.16
  extern "C" int DisconnectMQ( MBXHMBX *hconn );                                                            //L2.16
  extern "C" int OpenMQ( MBXHMBX hconn, MBXHMB *hobj, char *quename, int openmode );                        //L2.16
  extern "C" int CloseMQ( MBXHMBX hconn, MBXHMB *hobj );                                                    //L2.16
  extern "C" int PutMQ( MBXHMBX hconn, MBXHMB hobj, CMQMsg *mqmsg );                                        //L2.16
  extern "C" int GetMQ( MBXHMBX hconn, MBXHMB hobj, CMQMsg *mqmsg, char *msgid1, char *msgid2, int timer ); //L2.16

  #endif

