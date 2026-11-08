#include <cstddef>
  /**************************************************************************/
  /*                                                                        */
  /*        System:  TCS PROJECT                                            */
  /*                                                                        */
  /*   Module name:  memlist.h                                              */
  /*                                                                        */
  /*   Description:  Memory List Class                                      */
  /*                                                                        */
  /*   (c) Copyright 2005, International Business Machines-Japan Corp       */
  /*                                                                        */
  /*  Modification history:                                                 */
  /*    Date     Ver   Name        Description                              */
  /*  ---------- ----- ----------  ---------------------------------------  */
  /*  2005/01/20 L0.00 Y.KUBO      Initial release                          */
  /*                                                                        */
  /**************************************************************************/
  #ifndef INCLUDED_MEMLIST_H
  #define INCLUDED_MEMLIST_H

  struct ml_item { // item of memory chain
      struct ml_item *prv;
      struct ml_item *nxt;
      int siz;
      alignas(std::max_align_t) char mbr[1];
  };

  struct ml_root { // root of memory chain
      struct ml_item *top;
      struct ml_item *las;
      void *lst;
      int cnt;
  };

  class CMemList
  {

  public:
      CMemList();
      CMemList(const CMemList&) = delete;
      CMemList& operator=(const CMemList&) = delete;
      virtual ~CMemList();
      void Clear( void );
      void * Add( void *cpymbr, long siz, void **retmbr = NULL );
      int Delete( void *handle );
      void * Make( void );
      void * List( void );
      int Count( void );
      void * Top( void **member );
      void * Next( void *handle, void **member );
      void * Member( void *handle );

  private:
      struct ml_root m_root;

  };

  #endif
