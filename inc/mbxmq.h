  /**************************************************************************/
  /*                                                                        */
  /*        System:  Test Tool                                              */
  /*                                                                        */
  /*   Module name:  mbxmq.h                                                */
  /*                                                                        */
  /*   Description:                                                         */
  /*                                                                        */
  /*   (c) Copyright 2011, International Business Machines-Taiwan Corp      */
  /*                                                                        */
  /*  Modification history:                                                 */
  /*    Date     Ver   Name        Description                              */
  /*  ---------- ----- ----------  ---------------------------------------  */
  /*  2011/11/29 T0.00 Gavin Wu    Initial Release                          */
  /*                                                                        */
  /**************************************************************************/
  #ifndef INCLUDED_MBXMQ_H
  #define INCLUDED_MBXMQ_H

  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include "mqmsg.h"

  /*------------------------------------------------------------------------*/
  /*    For Configuration File                                              */
  /*------------------------------------------------------------------------*/
  #define CONF_FILE               "/usr/hsmsd/hsms/ini/hsms.ini"
  #define CONF_NOTE_SIGN          ';'
  #define CONF_MBX_SO_SEC         "MBX"
  #define CONF_MBX_SO             "MBX_SO"
  #define CONF_SECTION_LEN        100
  #define CONF_KEY_LEN            100
  #define CONF_VALUE_LEN          100
  /*------------------------------------------------------------------------*/
  /*    Common Constant                                                     */
  /*------------------------------------------------------------------------*/
  #define SPACE                   ' '
  #define _NORMAL                 0
  #define _ERROR                  1
  #define LOG_TYPE_INFO           'I'
  #define LOG_TYPE_ERR            'E'
  #define LOG_TYPE_WARN           'W'
  #define LOG_TYPE_DEBUG          'D'
  #define _MQ_NO_TIMEOUT          -1
  /*------------------------------------------------------------------------*/
  /*    Variable Length                                                     */
  /*------------------------------------------------------------------------*/
  #define CORR_ID_LEN             128
  #define PRG_NAME_LEN            10
  #define BUFF_LEN                256
  #define MQ_MSG_BUFF_LEN         100000

  /*------------------------------------------------------------------------*/
  /*    Common Function                                                     */
  /*------------------------------------------------------------------------*/
//#ifdef	__cplusplus
//extern "C" {
//#endif
//
//  long ComMQConnx     ( void        *pSoHandle       ,      //So Handle
//                        char       **error_msg       ,      //Error Message
//                        PMBXHMBX     Hcon            ,      //Connection Handle
//                        PMBXCHAR     pQMgrName       ,      //MQ Manager Name
//                        PMBXCHAR     pCnnName        );     //MQ Channel Name
//
//  long ComMQDisc      ( void        *pSoHandle       ,      //So Handle
//                        char       **error_msg       ,      //Error Message
//                        PMBXHMBX     Hcon            );     //Connection Handle 
//
//  long ComMQSend      ( void        *pSoHandle       ,      //So Handle
//                        char       **error_msg       ,      //Error Message
//                        MBXHMBX      Hcon            ,      //Connection Handle
//                        PMBXCHAR     pSendQueueName  ,      //Send Queue Name
//                        PMBXCHAR     pReplyQueueName ,      //Reply Queue Name
//                        PMBXMSGID    pCorrID         ,      //Correlation ID
//                        PMBXCHAR     pMsgBuf         ,      //Send Message Buffer
//                        MBXLONG      pBufLen         );     //Length of Message Buffer
//
//  long ComMQReceive   ( void        *pSoHandle       ,      //So Handle
//                        char       **error_msg       ,      //Error Message
//                        MBXHMBX      Hcon            ,      //Connection Handle
//                        PMBXCHAR     pRecvQueueName  ,      //Receive Queue Name
//                        PMBXCHAR     pReplyQueueName ,      //Reply Queue Name
//                        PMBXMSGID    pCorrID         ,      //Correlation ID
//                        PMBXCHAR     pMsgBuf         ,      //Receive Message Buffer
//                        MBXLONG      pBufLen         ,      //Length of Message Buffer
//                        MBXLONG      ptimeout        );     //Timeout for Receive Message(second, -1 means no timeout)
//
//  long ComTrxSendRecv ( MBXHMBX      Hcon            ,      //Connection Handle
//                        PMBXCHAR     pSendQueueName  ,      //Send Queue Name
//                        PMBXCHAR     pRecvQueueName  ,      //Reply Queue Name
//                        PMBXMSGID    pCorrID         ,      //Correlation ID
//                        PMBXCHAR     pMsgBuf         ,      //Send and Receive Message Buffer
//                        MBXLONG      pBufLen         ,      //Length of Message Buffer
//                        MBXLONG      ptimeout        );     //Timeout for Receive Message(second, -1 means no timeout)
//
//  long ComTrxRecvSend ( MBXHMBX      Hcon            ,      //Connection Handle
//                        PMBXCHAR     pSendQueueName  ,      //Send Queue Name
//                        PMBXCHAR     pRecvQueueName  ,      //Receive Queue Name
//                        PMBXMSGID    pCorrID         ,      //Correlation ID
//                        PMBXCHAR     pMsgBuf         ,      //Send and Receive Message Buffer
//                        MBXLONG      pBufLen         ,      //Length of Message Buffer
//                        MBXLONG      ptimeout        );     //Timeout for Receive Message(second, -1 is no timeout)
//
//  void ComGetConfig   ( char        *pOutBuf         ,      //Output Buffer
//                        char        *pSection        ,      //Section Name which using []
//                        char        *pSelectKey      );     //Key Name which using =
//
//#ifdef	__cplusplus
//}
//#endif

class MBXMQ
{
public:

	 MBXMQ();
   MBXMQ( char *pMbxSo );

   virtual ~MBXMQ();

   long Conn ( PMBXCHAR     pQMgrName       ,      //MQ Manager Name
               PMBXCHAR     pCnnName        );     //MQ Channel Name

   long Conn ( MBXHMBX     *hconn           ,
               PMBXCHAR     pQMgrName       ,      //MQ Manager Name
               PMBXCHAR     pCnnName        );     //MQ Channel Name

   long Disc ( void                         );

   long Disc ( MBXHMBX     *hconn           );

   long Open ( PMBXCHAR     pQueueName      ,      //Queue Name
               int          pMode           );     //Open Mode (0:INPUT,1:OUTPUT)

   long Open ( MBXHMBX      hconn           ,
               MBXHMB      *hobj            ,
               PMBXCHAR     pQueueName      ,
               int          pMode           );

   long Close( void                         );

   long Close( MBXHMBX      hconn           ,
               MBXHMB      *hobj            );

   long Recv ( PMBXCHAR     pRecvQueueName  ,      //Receive Queue Name
               PMBXCHAR     pReplyQueueName ,      //Reply Queue Name
               PMBXMSGID    pCorrID         ,      //Correlation ID
               PMBXCHAR     pMsgBuf         ,      //Receive Message Buffer
               MBXLONG      pBufLen         ,      //Length of Message Buffer
               MBXLONG      ptimeout        );     //Timeout for Receive Message(second, -1 means no timeout)

   long Recv ( MBXHMBX      hconn           ,
               MBXHMB       hobj            ,
               CMQMsg      *mqmsg           ,
               char        *msgid1          ,
               char        *msgid2          ,
               int          timer           );

   long Send ( PMBXCHAR     pSendQueueName  ,      //Send Queue Name
               PMBXCHAR     pReplyQueueName ,      //Reply Queue Name
               PMBXMSGID    pCorrID         ,      //Correlation ID
               PMBXCHAR     pMsgBuf         ,      //Send Message Buffer
               MBXLONG      pBufLen         );     //Length of Message Buffer

   long Send ( MBXHMBX      hconn           ,
               MBXHMB       hobj            ,
               CMQMsg      *mqmsg           );

private:

   PMBXHMBX   con_handle;  //Connection Handle
   PMBXHMB    que_handle;  //Queue Handle
   void      *so_handle;   //mbx.so handle
   char      *error_msg;   //Error Message

   long SetSO( char *pMbxSo );

};

  #endif
