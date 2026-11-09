  /**************************************************************************/
  /*                                                                        */
  /*        System:  TCS PROJECT                                            */
  /*                                                                        */
  /*   Module name:  mbx.h                                                  */
  /*                                                                        */
  /*   Description:  MQ Wrap Class                                          */
  /*                                                                        */
  /*   (c) Copyright 2009, International Business Machines-Japan Corp       */
  /*                                                                        */
  /*  Modification history:                                                 */
  /*    Date     Ver   Name        Description                              */
  /*  ---------- ----- ----------  ---------------------------------------  */
  /*  2009/06/13 L0.00 ISCJ        Initial Release                          */
  /*                                                                        */
  /**************************************************************************/
  #ifndef INCLUDED_MBX_H
  #define INCLUDED_MBX_H

  /*********************************************************************/
  /*  MBX Define Type                                                  */
  /*********************************************************************/

  typedef char MBXCHAR, *PMBXCHAR;
  typedef long MBXLONG, *PMBXLONG;

  typedef MBXLONG MBXHMBX, *PMBXHMBX;
  typedef MBXLONG MBXHMB , *PMBXHMB ;

  typedef MBXCHAR MBXCHAR128[128];

  typedef MBXCHAR128 MBXMSGID, *PMBXMSGID;

  typedef struct MBXMD {
    MBXCHAR128 rmbm;   // Reply Message Box Connection
    MBXCHAR128 rmb;    // Reply Message Box
    MBXMSGID   msgid1; // Message ID 1
    MBXMSGID   msgid2; // Message ID 2
    MBXLONG    type;   // MessageType (0:None,1:Request,2:Reply)
    MBXLONG    exp;    // Message Expiry Time(ms)
    MBXLONG    len;    // Message Length
  } MBXMD, *PMBXMD;

  /*********************************************************************/
  /*  MBX Define Length                                                */
  /*********************************************************************/

  #define MBX_DEF_NAME_LENGTH            sizeof(MBXCHAR128)
  #define MBX_MSG_ID_LENGTH              sizeof(MBXMSGID)

  /*********************************************************************/
  /*  MBX Define Constant                                               */
  /*********************************************************************/

  #define MBXOO_INPUT_SHARED             0           // Message Box Open Mode Input
  #define MBXOO_OUTPUT                   1           // Message Box Open Mode Output
  #define MBXOO_BROWSE                   2           // Message Box Open Mode Browse

  #define MBXPMO_BROWSE_FIRST            0           // Message Peek Mode First
  #define MBXPMO_BROWSE_NEXT             1           // Message Peek Mode Next

  #define MBXMT_NONE                     0           // Message Type None
  #define MBXMT_REQUEST                  1           // Message Type Request
  #define MBXMT_REPLY                    2           // Message Type Reply

  /*********************************************************************/
  /*  MBX Define Error                                                 */
  /*********************************************************************/

  #define MBXRC_NO_MSG_AVAILABLE         1           // no message arrival within timeout
  #define MBXRC_TRUNCATED_MSG_ACCEPTED   2           // buffer too small

  /*********************************************************************/
  /*  MBXCONNX Function -- Connect Message Box                         */
  /*********************************************************************/

  extern "C" MBXLONG MBXCONNX ( PMBXHMBX hmbm ,      // Message Box Connection Handle
                                PMBXCHAR mbm  ,      // Massage Box Connection Name
                                PMBXCHAR cnn  );     // Connection String like 'localhost(1414)'

  /*********************************************************************/
  /*  MBXDISC Function -- Disconnect Message Box                       */
  /*********************************************************************/

  extern "C" MBXLONG MBXDISC ( PMBXHMBX hmbm );      // Message Box Connection Handle

  /*********************************************************************/
  /*  MBXOPEN Function -- Open Message Box                             */
  /*********************************************************************/

  extern "C" MBXLONG MBXOPEN ( MBXHMBX  hmbm ,       // Message Box Connection Handle
                               PMBXHMB  hmb  ,       // Message Box Handle
                               PMBXCHAR mb   ,       // Message Box Name
                               MBXLONG  mode );      // Open Mode (0:INPUT,1:OUTPUT)

  /*********************************************************************/
  /*  MBXCLOSE Function -- Close Message Box                           */
  /*********************************************************************/

  extern "C" MBXLONG MBXCLOSE ( MBXHMBX  hmbm ,      // Message Box Connection Handle
                                PMBXHMB  hmb  );     // Message Box Handle

  /*********************************************************************/
  /*  MBXPEEK Function -- Peek Message                                 */
  /*********************************************************************/

  extern "C" MBXLONG MBXPEEK ( MBXHMBX   hmbm   ,    // Message Box Connection Handle
                               MBXHMB    hmb    ,    // Message Box Handle
                               PMBXMSGID msgid1 ,    // Message Key 1
                               PMBXMSGID msgid2 ,    // Message Key 2
                               PMBXCHAR  buf    ,    // Message Buffer
                               MBXLONG   len    ,    // Message Buffer Length
                               MBXLONG   mode   ,    // Peek Mode (0:FIRST,1:NEXT)
                               MBXLONG   timer  ,    // Wait Timer for Message Arrival(ms)
                               PMBXMD    md     );   // Message Description

  /*********************************************************************/
  /*  MBXGET Function -- Get Message                                   */
  /*********************************************************************/

  extern "C" MBXLONG MBXGET ( MBXHMBX   hmbm   ,     // Message Box Connection Handle
                              MBXHMB    hmb    ,     // Message Box Handle
                              PMBXMSGID msgid1 ,     // Message Key 1
                              PMBXMSGID msgid2 ,     // Message Key 2
                              PMBXCHAR  buf    ,     // Message Buffer
                              MBXLONG   len    ,     // Message Buffer Length
                              MBXLONG   timer  ,     // Wait Timer for Message Arrival(ms)
                              PMBXMD    md     );    // Message Description

  /*********************************************************************/
  /*  MBXPUT Function -- Put Message                                   */
  /*********************************************************************/

  extern "C" MBXLONG MBXPUT ( MBXHMBX  hmbm ,        // Message Box Connection Handle
                              MBXHMB   hmb  ,        // Message Box Handle
                              PMBXCHAR buf  ,        // Message Buffer
                              MBXLONG  len  ,        // Message Buffer Length
                              PMBXMD   md   );       // Message Description

  #endif

