/* static rfc_char_t sccsid[] = "@(#) $Id: //bas/640_REL/src/krn/rfc/trfcserv.c#3 $ SAP"; */

/*====================================================================*/
/*                                                                    */
/*     PROGRAM     :   trfcserv.c (running on Windows_NT, Windows_95, */
/*                                 OS/2 and R/3-platforms)            */
/*                                                                    */
/*                                                                    */
/*     DESCRIPTION :   Sample Server-Program for transactional RFC    */
/*                     Following function is available:               */
/*                                                                    */
/*                       - STFC_WRITE_TO_TCPIC                        */
/*                                                                    */
/*                     and can only be called from R/3 >= 3.0         */
/*                                                                    */
/*                                                                    */
/*                     trfcserv.tid:  TID-Management                  */
/*                     trfcserv.lck:  Lock file for access TID-Mgm.   */
/*                     trnn...n.dat:  Received data from R/3-ABAP     */
/*                     trnn...n.trc:  Trace file for RFC-Protocol     */
/*                                    (nn...n: Process ID)            */
/*                                                                    */
/*                     These files will be created/appended in the    */
/*                     directory defined via TRFC_WORK_DIR. If this   */
/*                     Environment Variable is not defined these files*/
/*                     will be in the working directory of the user   */
/*                     who started this program (mostly user from     */
/*                     SAP Gateway).                                  */
/*                                                                    */
/*                     Trace files will only be written if the        */
/*                     Environment Variable TRFC_TRACE is defined and */
/*                     not equal 0.                                   */
/*                                                                    */
/*                     On UNIX this program can be started via a      */
/*                     sript with setting the required Environment    */
/*                     Variable as follow:                            */
/*                                                                    */
/*                     transrfc:                                      */
/*                     #!/bin/csh                                     */
/*                     setenv TRFC_WORK_DIR /usr/sap/rfctest          */
/*                     setenv TRFC_TRACE    1                         */
/*                     /usr/sap/rfctest/trfcserv $*                   */
/*                                                                    */
/*                     Please do the same for other platforms.        */
/*                                                                    */
/*                     The entry in sm59 must contain the name of     */
/*                     this script (transrfc) as name of program.     */
/*                                                                    */
/*                     Because of working with file system it is      */
/*                     recommended to choose the working directory as */
/*                     a local directory and NOT mounted via NFS.     */
/*                                                                    */
/*                                                                    */
/*     SAP AG Walldorf                                                */
/*     Systeme, Anwendungen und Produkte in der Datenverarbeitung     */
/*                                                                    */
/*     Copyright (C) SAP AG 1995                                      */
/*                                                                    */
/*====================================================================*/

#if defined(SAPonOS400)
#include "saptype.h"
#endif

#define _POSIX_SOURCE  /* open(), O_*, fcntl(), close(), ... */

/*--------------------------------------------------------------------*/
/* Set up includes                                                    */
/*--------------------------------------------------------------------*/

#include "trfcserv.h"


/*====================================================================*/
/*  Function:  main                                                   */
/*                                                                    */
/*    Accept RFC-Connection                                           */
/*    Install all offering functions                                  */
/*    Loop in waiting for Remote Function Call                        */
/*      do function                                                   */
/*    until client disconnects the RFC-Connection                     */
/*                                                                    */
/*====================================================================*/
mainU (int argc, rfc_char_t **argv)
{
  setbuf(stderr, NULL);

  if (argc == 1)
  {
    help();
    return 0;
  }

  /*------------------------------------------------------------------*/
  /* Output argument list                                             */
  /*------------------------------------------------------------------*/
  fprintfU (stderr, cU("\nargc    = %d"), argc);
  for (i=0; i<argc; i++)
    fprintfU (stderr, cU("\nargv[%d] = '%s'"), i, argv[i]);
  fflush (stderr);

  /*------------------------------------------------------------------*/
  /* Get working directory and names of TID- and LOCK-file            */
  /*------------------------------------------------------------------*/
  memsetU (working_dir, cU('\0'), 64);

  if ((ptr = getenvU (cU("TRFC_WORK_DIR"))) != NULL)
    sprintfU (working_dir, cU("%s/"), ptr);

  strcpyU (tid_file,   working_dir);
  strcpyU (lock_file,  working_dir);
  strcpyU (tid_file+strlenU (tid_file),   std_tid_file);
  strcpyU (lock_file+strlenU (lock_file), std_lock_file);

  if (((ptr = getenvU (cU("TRFC_TRACE"))) != NULL) && (ptr[0] != cU('0')))
  {
    create_file_name(trace_file);
    strcpyU (trace_file+strlenU (trace_file), cU(".trc"));
    trace_fp = fopenU (trace_file, cU("a"));
  }
  else
    trace_fp = NULL;

  strcpyU (tbuf, cU("\n********************************\n"));
  actutime = time(NULL);
  /*CCQ_CLIB_LOCTIME_OK*/
  time_ptr = localtime(&actutime);
  memcpyU (datetime, asctimeU (time_ptr), 24);
  sprintfU (tbuf+strlenU (tbuf), cU("*   %s   *\n"), datetime);
  strcpyU (tbuf+strlenU (tbuf), cU("********************************\n"));
  TRFC_trace(tbuf);

  /*------------------------------------------------------------------*/
  /* Initialize the TID management                                    */
  /*------------------------------------------------------------------*/
  init_TID();

  /*------------------------------------------------------------------*/
  /* Accept RFC-Connection                                            */
  /*------------------------------------------------------------------*/
  sprintfU (tbuf, cU("\n<==  RfcAccept                           rfc_handle = "));
  TRFC_trace(tbuf);

  rfc_handle = RfcAccept(argv);

  if (rfc_handle == RFC_HANDLE_NULL)
  {
    sprintfU (tbuf, cU("RFC_HANDLE_NULL"));
    TRFC_trace(tbuf);
    rfc_error (cU("RfcAccept"));
  }
  sprintfU (tbuf, cU("%u"), rfc_handle);
  TRFC_trace(tbuf);

  /*------------------------------------------------------------------*
   * attach a context to the handle, just to demonstrate this API
   *------------------------------------------------------------------*/
  rfc_rc = RfcAttachContext(rfc_handle, NULL, NULL, cU("MySampleContext"));
  if (rfc_rc != RFC_OK)
  {
      sprintfU (tbuf, cU("RfcAttachContext failed %d\n"), rfc_rc);
      TRFC_trace(tbuf);
      exit(1);
  }
  else
  {
      /* Demonstrate a call to RfcGetAttachedContext on a accept handle
       */
      int      fFromCarrier;
      void     *pContext;
      SAP_CHAR *pCtx;

      sprintfU (tbuf, cU("\n<==  RfcGetAttachedContext    rfc_rc = "));
      TRFC_trace(tbuf);

      rfc_rc = RfcGetAttachedContext(rfc_handle, &pContext, &fFromCarrier);

      sprintfU (tbuf, cU("%d\n"), rfc_rc);
      TRFC_trace(tbuf);

      if (rfc_rc == RFC_OK)
      {
          pCtx = (SAP_CHAR*)pContext;
          sprintfU (tbuf, cU("User context %s\n"), pCtx);
          TRFC_trace(tbuf);
          sprintfU (tbuf, cU("From Carrier %d\n\n"), fFromCarrier);
          TRFC_trace(tbuf);
      }

  }

  /*------------------------------------------------------------------*/
  /* Install offering functions                                       */
  /*------------------------------------------------------------------*/
  rfc_rc = install();
  if( rfc_rc != RFC_OK )
    return 1;

  /*------------------------------------------------------------------*/
  /* Wait for Remote Function Call                                    */
  /*------------------------------------------------------------------*/
  do
  {
    sprintfU (tbuf, cU("\n\nWait for next RFC call ....."));
    TRFC_trace(tbuf);

#ifdef SAP_RFC_LISTEN
    for (rfc_rc = RFC_RETRY; rfc_rc == RFC_RETRY;)
    {
      sprintfU (tbuf, cU("\n<==  RfcListen                rfc_rc = "));
      TRFC_trace(tbuf);

      rfc_rc = RfcListen(rfc_handle);

      sprintfU (tbuf, cU("%d"), rfc_rc);
      TRFC_trace(tbuf);
      if (rfc_rc == RFC_RETRY)
      {
        /* do something else while waiting for the next RFC call */
#if defined(SAPonUNIX) || defined (SAPonOS400) || defined(SAPonOS390)
        sleep(2);
#elif defined(SAPonNT) || defined(OS2)
        Sleep(2000);
#else
#error "Error Message > Please update this function for your OS"
#endif
      }
    }
    if (rfc_rc != RFC_OK)
    {
      if (rfc_rc != RFC_CLOSED)
        rfc_error(cU("RfcListen"));
      else
	break;
    }
#endif

    rfc_rc = RfcDispatch(rfc_handle);

    sprintfU (tbuf, cU("\n<==  RfcDispatch              rfc_rc = %d"), rfc_rc);
    TRFC_trace(tbuf);

  } while (rfc_rc == RFC_OK);

  if (trace_fp != NULL)
    fclose(trace_fp);

  return 0;
}


/*====================================================================*/
/*                                                                    */
/* Install the offered functions for RFC                              */
/*                                                                    */
/*====================================================================*/
static RFC_RC install(void)
{
  sprintfU (tbuf, cU("\n<==  RfcInstallTransactionControl"));
  TRFC_trace(tbuf);

  RfcInstallTransactionControl ((RFC_ON_CHECK_TID)   TID_check,
                                (RFC_ON_COMMIT)      TID_commit,
                                (RFC_ON_ROLLBACK)    TID_rollback,
                                (RFC_ON_CONFIRM_TID) TID_confirm);

#ifdef SAP_RFC_GETNAME
  /* Install USER_GLOBAL_SERVER for working with RfcGetName */
  sprintfU (tbuf, cU("\n<==  RfcInstallFunction '%s' rfc_rc = "), name_user_global_server);
  TRFC_trace(tbuf);

  rfc_rc = RfcInstallFunction(name_user_global_server,
                              (RFC_ONCALL) user_global_server,
                              user_global_server_docu());

  sprintfU (tbuf, cU("%d"), rfc_rc);
  TRFC_trace(tbuf);

  if( rfc_rc != RFC_OK )
  {
    sprintfU (abort_text, cU("\nERROR: Install %s     rfc_rc = %d"),
              name_user_global_server, rfc_rc);
    function_abort(abort_text);
  }

  /* Necessary for access docu_function by sm59 (system information) */
  sprintfU (tbuf, cU("\n<==  RfcInstallFunction '%s'  rfc_rc = "), name_transactional_test);
  TRFC_trace(tbuf);

  rfc_rc = RfcInstallFunction(name_transactional_test,
                              (RFC_ONCALL) NULL,
                              transactional_test_docu());

#else
  sprintfU (tbuf, cU("\n<==  RfcInstallFunction 'STFC_WRITE_TO_TCPIC'  rfc_rc = "));
  TRFC_trace(tbuf);

  rfc_rc = RfcInstallFunction (name_transactional_test,
                               (RFC_ONCALL) transactional_test,
                               transactional_test_docu());
#endif

  sprintfU (tbuf, cU("%d"), rfc_rc);
  TRFC_trace(tbuf);

  if( rfc_rc != RFC_OK )
  {
    sprintfU (abort_text, cU("\nERROR: Install %s     rfc_rc = %d"),
              name_transactional_test, rfc_rc);
    function_abort (abort_text);
  }

  return RFC_OK;
}


#ifdef SAP_RFC_GETNAME
/*====================================================================*/
/*                                                                    */
/* RFC-FUNCTION: %%USER_GLOBAL_SERVER                                 */
/*                                                                    */
/* Global server function for working with RfcGetName                 */
/*                                                                    */
/*====================================================================*/
static RFC_RC DLL_CALL_BACK_FUNCTION user_global_server(RFC_HANDLE rfc_handle)
{
  sprintfU (tbuf, cU("\n\nStart Function %s"), name_user_global_server);
  TRFC_trace(tbuf);

  sprintfU (tbuf, cU("\n<==  RfcGetName               rfc_rc = "));
  TRFC_trace(tbuf);

  rfc_rc = RfcGetName(rfc_handle, function_name);

  sprintfU (tbuf, cU("%d"), rfc_rc);
  TRFC_trace(tbuf);

  if (rfc_rc == RFC_OK)
  {
    sprintfU (tbuf, cU("   Function Name: '%s'"), function_name);
    if (strcmpU (function_name, name_transactional_test) == 0)
      rfc_rc = transactional_test(rfc_handle);
    else
    {
      sprintfU (abort_text,
                cU("'%s' ist not implemented in program trfcserv.c"),
                function_name);
      function_abort(abort_text);
    }
  }
  else
    rfc_error(cU("RfcGetName"));

  return rfc_rc;
}
#endif


/*====================================================================*/
/*                                                                    */
/* RFC-FUNCTION: STFC_WRITE_TO_TCPIC                                  */
/*                                                                    */
/* Received data from internal table will be write in a file          */
/*                                                                    */
/*====================================================================*/
static RFC_RC DLL_CALL_BACK_FUNCTION transactional_test(RFC_HANDLE rfc_handle)
{
    sprintfU (tbuf, cU("\n\nStart Function %s"), name_transactional_test);
    
    TRFC_trace(tbuf);

    memsetR (&parameters[0], 0, sizeofR(parameters));

    /* Prepare internal table TCPICDAT */
    tables[0].name     = cU("TCPICDAT");
    tables[0].nlen     = 8;
    tables[0].type     = TYPC;
    tables[0].leng     = TCPICDAT_ENTRY_SIZE*sizeofR(SAP_UC);
    tables[0].itmode   = RFC_ITMODE_BYREFERENCE;

    /* terminate array */
    tables[1].name = NULL;

    
    /* receive data */
    sprintfU (tbuf, cU("\n<==  RfcGetData               rfc_rc = "));
    TRFC_trace(tbuf);

    rfc_rc = RfcGetData (rfc_handle, parameters, tables);

    sprintfU (tbuf, cU("%d"), rfc_rc);
    TRFC_trace(tbuf);

    if (rfc_rc != RFC_OK)
     rfc_error(cU("RfcGetData"));

    
    sprintfU (tbuf, cU("\n<==  RfcGetAttributes         rc = "));
    
    TRFC_trace(tbuf);
    
    rc = RfcGetAttributes(rfc_handle, &rfc_attributes);
    
    sprintfU (tbuf, cU("%d"), rc);
    
    TRFC_trace(tbuf);
    
    if (rc)
     rfc_error(cU("RfcGetAttributes"));

    
    sprintfU (tbuf, cU("\n\nAttributes of this RFC connection"));
    sprintfU (tbuf+strlenU (tbuf), cU("\n---------------------------------"));
    sprintfU (tbuf+strlenU (tbuf), cU("\nDestination            :  %s"), rfc_attributes.dest);
    sprintfU (tbuf+strlenU (tbuf), cU("\nMy Host                :  %s"), rfc_attributes.own_host);

    if (rfc_attributes.rfc_role == RFC_ROLE_CLIENT)
    {
        if (rfc_attributes.partner_type == RFC_SERVER_EXT)
         sprintfU (tbuf+strlenU (tbuf), cU("\nServer Program Name    :  %s"), rfc_attributes.partner_host);
        else if (rfc_attributes.partner_type == RFC_SERVER_EXT_REG)
         sprintfU (tbuf+strlenU (tbuf), cU("\nServer Program ID      :  %s"), rfc_attributes.partner_host);
        else
         sprintfU (tbuf+strlenU (tbuf), cU("\nPartner Host           :  %s"), rfc_attributes.partner_host);
    }
    else
    {
        sprintfU (tbuf+strlenU (tbuf), cU("\nPartner Host           :  %s"), rfc_attributes.partner_host);
    }

    sprintfU (tbuf+strlenU (tbuf), cU("\nSystem No.             :  %s"), rfc_attributes.systnr);
    sprintfU (tbuf+strlenU (tbuf), cU("\nSystem Name            :  %s"), rfc_attributes.sysid);
    sprintfU (tbuf+strlenU (tbuf), cU("\nClient                 :  %s"), rfc_attributes.client);
    sprintfU (tbuf+strlenU (tbuf), cU("\nUser                   :  %s"), rfc_attributes.user);
    sprintfU (tbuf+strlenU (tbuf), cU("\nLanguage               :  %s"), rfc_attributes.language);
    sprintfU (tbuf+strlenU (tbuf), cU("\nISO-Language           :  %s"), rfc_attributes.ISO_language);

    if (rfc_attributes.trace == cU('X'))
     sprintfU (tbuf+strlenU (tbuf), cU("\nRFC Trace              :  ON"));
    else
     sprintfU (tbuf+strlenU (tbuf), cU("\nRFC Trace              :  OFF"));

    sprintfU (tbuf+strlenU (tbuf), cU("\nMy Codepage            :  %s"), rfc_attributes.own_codepage);
    sprintfU (tbuf+strlenU (tbuf), cU("\nPartner Codepage       :  %s"), rfc_attributes.partner_codepage);
    sprintfU (tbuf+strlenU (tbuf), cU("\nReal Partner Codepage  :  %s"), rfc_attributes.real_partner_codepage);

    if (rfc_attributes.rfc_role == RFC_ROLE_CLIENT)
     sprintfU (tbuf+strlenU (tbuf), cU("\nRFC Role               :  External RFC Client"));
    else if (rfc_attributes.own_type == RFC_SERVER_EXT)
     sprintfU (tbuf+strlenU (tbuf), cU("\nRFC Role               :  External RFC Server, started by SAP gateway"));
    else
     sprintfU (tbuf+strlenU (tbuf), cU("\nRFC Role               :  External RFC Server, registered at SAP gateway"));

    sprintfU (tbuf+strlenU (tbuf), cU("\nRFC Library Release    :  %s"), rfc_attributes.own_rel);

    if (rfc_attributes.partner_type == RFC_SERVER_R3)
     sprintfU (tbuf+strlenU (tbuf), cU("\nRFC Partner            :  SAP R/3"));
    else if (rfc_attributes.partner_type == RFC_SERVER_R2)
     sprintfU (tbuf+strlenU (tbuf), cU("\nRFC Partner            :  SAP R/2"));
    else if (rfc_attributes.rfc_role == RFC_ROLE_CLIENT)
    {
        if (rfc_attributes.partner_type == RFC_SERVER_EXT)
         sprintfU (tbuf+strlenU (tbuf), cU("\nRFC Partner            :  External RFC Server, started by SAP gateway"));
        else
         sprintfU (tbuf+strlenU (tbuf), cU("\nRFC Partner            :  External RFC Server, registered at SAP gateway"));
    }
    else
    {
        sprintfU (tbuf+strlenU (tbuf), cU("\nRFC Partner            :  External RFC Client"));
    }
    
    sprintfU (tbuf+strlenU (tbuf), cU("\nPartner System Release :  %s"),   rfc_attributes.partner_rel);
    sprintfU (tbuf+strlenU (tbuf), cU("\nR/3 Kernel Release     :  %s"),   rfc_attributes.kernel_rel);
    sprintfU (tbuf+strlenU (tbuf), cU("\nCPI-C Conversation ID  :  %s\n"), rfc_attributes.CPIC_convid);
    sprintfU (tbuf+strlenU (tbuf), cU("\nProgram Name           :  %s\n"), rfc_attributes.progname);

    TRFC_trace(tbuf);


    /* get call info for this trfc connection */
    rfc_call_info.call_type = cU('T');

    rfc_rc = RfcGetCallInfo (rfc_handle, &rfc_call_info);
    
    if (rfc_rc != RFC_OK)
     rfc_error (cU("RfcGetCallInfo"));

    fprintfU (stderr, cU("\nCall Info: function name:    %s\n"), rfc_call_info.function_name);
    fprintfU (stderr, cU("            call type :       %c\n"),  rfc_call_info.call_type);
    fprintfU (stderr, cU("            function counter: %s\n"),  rfc_call_info.function_counter);
    fprintfU (stderr, cU("            tid:              %s\n"),  rfc_call_info.tid);


    {
      /* Demonstrate a call to RfcGetAttachedContext on a play back handle
       */
      int      fFromCarrier;
      void     *pContext;
      SAP_CHAR *pCtx;

      sprintfU (tbuf, cU("\n<==  RfcGetAttachedContext    rfc_rc = "));
      TRFC_trace(tbuf);

      rfc_rc = RfcGetAttachedContext(rfc_handle, &pContext, &fFromCarrier);

      sprintfU (tbuf, cU("%d\n"), rfc_rc);
      TRFC_trace(tbuf);

      if (rfc_rc == RFC_OK)
      {
          pCtx = (SAP_CHAR*)pContext;
          sprintfU (tbuf, cU("User context %s\n"), pCtx);
          TRFC_trace(tbuf);
          sprintfU (tbuf, cU("From Carrier %d\n\n"), fFromCarrier);
          TRFC_trace(tbuf);
      }
    }



    /* Put the name of data file in trace file */
    sprintfU (tbuf, cU("\nDATA FILE:  '%s'"), data_file);
    TRFC_trace(tbuf);

    rc = write_itab_to_file(tables[0].ithandle, data_file, (rfc_char_t*)tables[0].name);

    if (rc)
    {
        sprintfU (abort_text, cU("Can not write user data in file system"));
    
        function_abort(abort_text);
    }

    
    /*------------------------------------------------------------------*/
    /* Return parameters and tables                                     */
    /*------------------------------------------------------------------*/
    memsetR (&parameters[0], 0, sizeofR(parameters));

    /* return to the caller */
    sprintfU (tbuf, cU("\n<==  RfcSendData              rfc_rc = "));
    TRFC_trace(tbuf);

    rfc_rc = RfcSendData (rfc_handle, parameters, tables);

    
    sprintfU (tbuf, cU("%d"), rfc_rc);
    TRFC_trace(tbuf);

    if (rfc_rc != RFC_OK)
     rfc_error (cU("RfcSendData"));

    
    return rfc_rc;
}


/*--------------------------------------------------------------------*/
/* TID_CHECK-Function for transactional RFC                           */
/*--------------------------------------------------------------------*/
static int DLL_CALL_BACK_FUNCTION TID_check(RFC_TID tid)
{
  sprintfU (tbuf, cU("\n\nStart Function TID_CHECK      TID = %s"), tid);
  TRFC_trace(tbuf);
  strcpyU (TransID, tid);

  /* Check TID from TID Management */
  tid_rc = check_TID(tid, data_file);

  if (tid_rc == TID_OK)
  {
    create_file_name(data_file);
    strcpyU (data_file+strlenU (data_file), cU(".dat"));
    return 0;
  }

  if (tid_rc == TID_FOUND)
    return 1;

  if (tid_rc == TID_ERROR_LOCK)
    function_abort(tid_etext01);
  else
    function_abort(tid_etext02);

  return 1;
}


/*--------------------------------------------------------------------*/
/* TID_COMMIT-Function for transactional RFC                          */
/*--------------------------------------------------------------------*/
static void DLL_CALL_BACK_FUNCTION TID_commit(RFC_TID tid)
{
  sprintfU (tbuf, cU("\n\nStart Function TID_COMMIT     TID = %s"), tid);
  TRFC_trace(tbuf);
  strcpyU (TransID, tid);

  /* Commit TID from TID Management */
  tid_rc = update_TID(tid, EXECUTED, data_file);

  if (tid_rc == TID_OK)
    return;

  if (tid_rc == TID_ERROR_LOCK)
    function_abort(tid_etext03);
  else
    function_abort(tid_etext04);

  return;
}


/*--------------------------------------------------------------------*/
/* CONFIRM-Function for transactional RFC                             */
/*--------------------------------------------------------------------*/
static void DLL_CALL_BACK_FUNCTION TID_confirm(RFC_TID tid)
{
  sprintfU (tbuf, cU("\n\nStart Function TID_CONFIRM    TID = %s"), tid);
  TRFC_trace(tbuf);
  strcpyU (TransID, tid);

  /* Confirm TID from TID Management */
  tid_rc = update_TID(tid, CONFIRMED, data_file);

  if (tid_rc == TID_OK)
    return;

  if (tid_rc == TID_ERROR_LOCK)
    function_abort(tid_etext05);
  else
    function_abort(tid_etext06);

  return;
}


/*--------------------------------------------------------------------*/
/* TID_ROLLBACK-Function for transactional RFC                        */
/*--------------------------------------------------------------------*/
static void DLL_CALL_BACK_FUNCTION TID_rollback(RFC_TID tid)
{
  sprintfU (tbuf, cU("\n\nStart Function TID_ROLLBACK   TID = %s"), tid);
  TRFC_trace(tbuf);
  strcpyU (TransID, tid);

  /* Rollback TID from TID Management */
  tid_rc = update_TID(tid, ROLLBACK, data_file);

  if (tid_rc == TID_OK)
    return;

  if (tid_rc == TID_ERROR_LOCK)
    function_abort(tid_etext07);
  else
    function_abort(tid_etext08);

  return;
}


/*--------------------------------------------------------------------*/
/* Initialize the TID-Management                                      */
/*--------------------------------------------------------------------*/
static void init_TID(void)
{
  FILE          *fp;

  /* Read TID-file for check and update */
  fp = fopenU (tid_file, cU("rb+"));
  if (fp == NULL)
  {
    /* TID-Management not existent */
    fp = fopenU (tid_file, cU("ab"));
    if (fp == NULL)
    {
      /* ERROR: Create file for TID-Management failed */
      unlock_TID(lock_file);
      strcpyU (tbuf, tid_etext00);
      TRFC_trace(tbuf);
      exit(1);
    }

    memsetU (tbuf, cU('\0'), LINE_SIZE_CHAR);
    strcpyU (tbuf, cU("***   TID-MANAGEMENT FOR TRANSACTIONAL RFC (Server Program)   ***\n"));
    if (fputsU (tbuf, fp) < 0)
    {
      /* ERROR: Write in TID-Management failed */
      unlock_TID(lock_file);
      fclose(fp);
      strcpyU (tbuf, tid_etext02);
      TRFC_trace(tbuf);
      exit(1);
    }
  }
  else
  {
    while ((ptr = fgetsU (tbuf, LINE_SIZE_CHAR+1, fp)) != NULL)
    {
      if (memcmpU (tbuf+52, cU("CREATED"), 7) == 0)
      {
        /* Auf ROLLBACK setzen */
        memcpyU (TransID, tbuf+26, 24);
        TransID[24] = 0;
        tid_rc = update_TID(TransID, ROLLBACK, data_file);
        if (tid_rc == TID_OK)
          continue;
        if (tid_rc == TID_ERROR_LOCK)
          strcpyU (tbuf, tid_etext01);
        else
          strcpyU (tbuf, tid_etext02);
        TRFC_trace(tbuf);
        exit(1);
      }
    }
  }
  return;
}



/*--------------------------------------------------------------------*/
/* Check and Update TID in the TID-Management                         */
/*--------------------------------------------------------------------*/
static TID_RC check_TID(RFC_TID tid, rfc_char_t *datafile)
{
  TID_RC        tidrc;
  FILE          *fp;

  /* Try to lock TID-Management */
  if (lock_TID(lock_file, tid))
    return TID_ERROR_UPDATE;

  /* Read TID-file for check and update */
  fp = fopenU (tid_file, cU("rb+"));
  if (fp == NULL)
    tidrc = TID_ERROR_UPDATE;

  while ((ptr = fgetsU (tbuf, LINE_SIZE_CHAR, fp)) != NULL)
  {
    if ((memcmpU (tbuf+26, tid, 24) == 0) &&
        (memcmpU (tbuf+52, cU("ROLLBACK"), 8) != 0))
      break;
  }

  if (ptr != NULL)
  {
    /* Previous TID found: Inform the RFC-Library about EXECUTED RFC-functions */
    unlock_TID(lock_file);
    fclose(fp);
    return TID_FOUND;
  }

  /* Write new TID at the end of the tid_file */
  tid_state = CREATED;
  if (write_TID(fp, tid, tid_state, datafile))
    tidrc = TID_ERROR_UPDATE;
  else
    tidrc = TID_OK;

  /* Unlock TID-Management */
  unlock_TID(lock_file);
  fclose(fp);
  return tidrc;
}


/*--------------------------------------------------------------------*/
/* Update TID in the TID-Management                                   */
/*--------------------------------------------------------------------*/
static TID_RC update_TID(RFC_TID tid, TID_STATE tid_state, rfc_char_t *datafile)
{
  TID_RC        tidrc;
  long          offset = 0;
  FILE          *fp;

  /* Try to lock TID-Management */
  if (lock_TID(lock_file, tid))
    return TID_ERROR_UPDATE;

  /* Open TID-file for update */
  fp = fopenU (tid_file, cU("rb+"));
  if (fp == NULL)
  {
    /* ERROR: Open TID-Management failed */
    unlock_TID(lock_file);
    return TID_ERROR_UPDATE;
  }

  while ((ptr = fgetsU (tbuf, LINE_SIZE_CHAR+1, fp)) != NULL)
  {
    if ((memcmpU (tbuf+26, tid, 24) == 0) &&
        (memcmpU (tbuf+52, cU("ROLLBACK"), 8) != 0))
      break;
    offset = offset + strlenU (tbuf);
  }

  if ((ptr != NULL) &&
      (fseek(fp, offset, SEEK_SET) == 0) &&
      (write_TID(fp, tid, tid_state, datafile) == 0))
    tidrc = TID_OK;
  else
    /* ERROR: Update TID-Management failed */
    tidrc = TID_ERROR_UPDATE;

  /* Unlock TID-Management */
  unlock_TID(lock_file);
  fclose(fp);
  return tidrc;
}


/*--------------------------------------------------------------------*/
/* Write TID in the TID-Management                                    */
/*--------------------------------------------------------------------*/
static int write_TID(FILE *fp, RFC_TID tid, TID_STATE tid_state, rfc_char_t *datafile)
{
  memsetU (tbuf, cU('\0'), LINE_SIZE_CHAR);
  actutime = time(NULL);
  /*CCQ_CLIB_LOCTIME_OK*/
  time_ptr = localtime(&actutime);
  memcpyU (datetime, asctimeU (time_ptr), 24);

  switch(tid_state)
  {
    case CREATED:
      sprintfU (tbuf, cU("%s  %s  CREATED    %s\n"), datetime, tid, datafile);
      break;

    case EXECUTED:
      sprintfU (tbuf, cU("%s  %s  EXECUTED   %s\n"), datetime, tid, datafile);
      break;

    case CONFIRMED:
      sprintfU (tbuf, cU("%s  %s  CONFIRMED  %s\n"), datetime, tid, datafile);
      break;

    case ROLLBACK:
      sprintfU (tbuf, cU("%s  %s  ROLLBACK   %s\n"), datetime, tid, datafile);
      break;
  }

  if (fputsU (tbuf, fp) < 0)
    return 1;
  else
    return 0;
}


/*--------------------------------------------------------------------*/
/* Lock the TID-Management                                            */
/*--------------------------------------------------------------------*/
static int lock_TID (rfc_char_t *lockfile, RFC_TID tid)
{
  int try_lock = 60;

#ifdef SAPonUNIX
  struct flock lck;

  while (try_lock)
  {
    if ((lock_fd = openU (lockfile, O_CREAT|O_RDWR, 0777)) == -1)
      return 1;

    lck.l_type   = F_WRLCK;
    lck.l_whence = 0;
    lck.l_start  = 0l;
    lck.l_len    = 0l;

    if (fcntl(lock_fd, F_SETLK, &lck) < 0)
    {
      close(lock_fd);
      sleep(1);
      try_lock--;
    }
    else
    {
      memsetU (tbuf, cU('\0'), LINE_SIZE_CHAR);
      actutime = time(NULL);
      /*CCQ_CLIB_LOCTIME_OK*/
      time_ptr = localtime(&actutime);
      memcpyU (datetime, asctimeU (time_ptr), 24);

      sprintfU (tbuf, cU("%s  %s  EXECUTING  \n"), datetime, tid);
      write(lock_fd, tbuf, LINE_SIZE_RAW);
      return 0;
    }
  }
  return 1;

#elif defined(SAPonNT)
  while (try_lock)
  {
    lock_fd = openU (lockfile, O_CREAT|O_TEMPORARY|O_RDWR, 0777);
    if (lock_fd == -1)
    {
      Sleep(1000);
      try_lock--;
    }
    else
    {
      memsetU (tbuf, cU('\0'), LINE_SIZE_CHAR);
      actutime = time(NULL);
      /*CCQ_CLIB_LOCTIME_OK*/
      time_ptr = localtime(&actutime);
      memcpyU (datetime, asctimeU (time_ptr), 24);

      sprintfU (tbuf, cU("%s  %s  EXECUTING  \n"), datetime, tid);
      write(lock_fd, tbuf, LINE_SIZE_RAW);
      return 0;
    }
  }
  return 1;

#elif defined(SAPonVMS)
  while (try_lock)
  {
    lock_fd = openU (lockfile, O_CREAT|O_RDWR, 0777, cU("fop=dlt"), cU("shr=nil"));
    if (lock_fd == -1)
    {
      sleep(1);
      try_lock--;
    }
    else
    {
      memsetU (tbuf, cU('\0'), LINE_SIZE_CHAR);
      actutime = time(NULL);
      /*CCQ_CLIB_LOCTIME_OK*/
      time_ptr = localtime(&actutime);
      memcpyU (datetime, asctimeU (time_ptr), 24);

      sprintfU (tbuf, cU("%s  %s  EXECUTING  \n"), datetime, tid);
      write(lock_fd, tbuf, LINE_SIZE_RAW);
      return 0;
    }
  }
  return 1;

#elif defined(SAPonOS2_2x)
  while (try_lock)
  {
    lock_fd = openU (lockfile, O_CREAT|O_RDWR, S_IREAD|S_IWRITE );
    if (lock_fd == -1)
    {
      DosSleep(1000);
      try_lock--;
    }
    else
    {
      memsetU (tbuf, cU('\0'), LINE_SIZE_CHAR);
      actutime = time(NULL);
      /*CCQ_CLIB_LOCTIME_OK*/
      time_ptr = localtime(&actutime);
      memcpyU (datetime, asctimeU (time_ptr), 24);

      sprintfU (tbuf, cU("%s  %s  EXECUTING  \n"), datetime, tid);
      write(lock_fd, tbuf, LINE_SIZE_RAW);
      return 0;
    }
  }
  return 1;

#elif defined(SAPonOS400)
 struct flock lck;

 while (try_lock)
 {
   if ((lock_fd = openU (lockfile, O_CREAT|O_RDWR)) == -1)
     return 1;

   lck.l_type   = F_WRLCK;
   lck.l_whence = 0;
   lck.l_start  = 0l;
   lck.l_len    = 0l;

   if (fcntl(lock_fd, F_SETLK, &lck) < 0)
   {
     close(lock_fd);
     sleep(1);
     try_lock--;
   }
   else
   {
     memsetU (tbuf, cU('\0'), LINE_SIZE_CHAR);
     actutime = time(NULL);
     /*CCQ_CLIB_LOCTIME_OK*/
     time_ptr = localtime(&actutime);
     memcpyU (datetime, asctimeU (time_ptr), 24);

     sprintfU (tbuf, cU("%s  %s  EXECUTING  \n"), datetime, tid);
     write(lock_fd, (void*)tbuf, LINE_SIZE_RAW);
     return 0;
   }
 }
 return 1;

#elif SAPonOS390
  struct flock lck;

  while (try_lock)
  {
    if ((lock_fd = openU (lockfile, O_CREAT|O_RDWR, 0777)) == -1)
      return 1;

    lck.l_type   = F_WRLCK;
    lck.l_whence = 0;
    lck.l_start  = 0l;
    lck.l_len    = 0l;

    if (fcntl(lock_fd, F_SETLK, &lck) < 0)
    {
      close(lock_fd);
      sleep(1);
      try_lock--;
    }
    else
    {
      memsetU (tbuf, cU('\0'), LINE_SIZE_CHAR);
      actutime = time(NULL);
      /*CCQ_CLIB_LOCTIME_OK*/
      time_ptr = localtime(&actutime);
      memcpyU (datetime, asctimeU (time_ptr), 24);

      sprintfU (tbuf, cU("%s  %s  EXECUTING  \n"), datetime, tid);
      write(lock_fd, tbuf, LINE_SIZE_RAW);
      return 0;
    }
  }
  return 1;

#else
#error "Error Message > Please update this function for your OS"
#endif
}


/*--------------------------------------------------------------------*/
/* Unlock the TID-Management                                          */
/*--------------------------------------------------------------------*/
static int unlock_TID(rfc_char_t *lockfile)
{
#if defined(SAPonUNIX) || defined(SAPonOS400) || defined(SAPonOS390)
  close (lock_fd);
  unlinkU (lockfile);
  return 1;

#elif defined(SAPonNT) || defined(SAPonVMS) || defined(SAPonOS2_2x)
  close (lock_fd);
  return 0;

#else
#error "Error Message > Please update this function for your OS"
#endif
}


/*--------------------------------------------------------------------*/
/* Move internal table to file                                        */
/*--------------------------------------------------------------------*/
int write_itab_to_file(ITAB_H itab_h, rfc_char_t *datafile, rfc_char_t *itab_name)
{
  FILE          *fp;
  unsigned      k,
                it_leng;

  if (itab_h == NULL)
    return 0;

  /* Open data_file */
  fp = fopenU (datafile, cU("a"));
  if (fp == NULL)
    return 1;

  it_leng = ItLeng(itab_h); /* it_leng is number of bytes */

  sprintfU (tbuf, cU("Table Name=%s\nTable Line=%u\nTable Fill=%u\n"),
            itab_name, it_leng, ItFill(itab_h));
  if (fwriteU (tbuf, 1, strlenU (tbuf), fp) < strlenU (tbuf))
    function_abort(tid_etext10);

  it_leng = it_leng/sizeofR(SAP_UC); /* it_leng is number of chars now */

  for (k = 1; ; k++)
  {
    ptr = (rfc_char_t *) ItGetLine(itab_h, k);
    if (ptr == NULL) 
     break;

    /* write into file with end of line */
    memcpyU (tbuf, ptr, it_leng);
    tbuf[it_leng] = cU('\n');
    if (fwriteU (tbuf, 1, it_leng+1, fp) < it_leng+1)
      function_abort(tid_etext10);
  }

  /* Close data_file */
  fclose(fp);

  return 0;
}


/*====================================================================*/
/*                                                                    */
/* Documentation for function STFC_WRITE_TO_TCPIC                     */
/*                                                                    */
/*====================================================================*/
static rfc_char_t *transactional_test_docu(void)
{
  static rfc_char_t docu[] =
  iU("RFC-Client can send data via tRFC in one internal table.")          NL_AR
  iU("")                                                                  NL_AR
  iU("TABLES")                                                            NL_AR
  iU("  TCPICDAT        C(0072)")                                         NL_AR
   ;
  return docu;
}

#ifdef SAP_RFC_GETNAME
/*====================================================================*/
/*                                                                    */
/* Documentation for function %%USER_GLOBAL_SERVER                    */
/*                                                                    */
/*====================================================================*/
static rfc_char_t *user_global_server_docu(void)
{
  static rfc_char_t docu[] =
  cU("The RFC library will call this function if any unknown")            NL_AR
  cU("RFC function should be executed in this RFC server program.")       NL_AR
   ;
  return docu;
}
#endif


/*--------------------------------------------------------------------*/
/* set up RFC parameters                                              */
/*--------------------------------------------------------------------*/
void rfc_param (RFC_PARAMETER *rfc_param,
                rfc_char_t          *abap_field_name,
                unsigned      data_type,
                void          *data_addr,
                unsigned      data_len)
{
  RFC_PARAMETER *p = rfc_param;

  p->name = abap_field_name;
  p->nlen = abap_field_name == NULL ? 0 : strlenU (abap_field_name);
  p->type = data_type;
  p->addr = data_addr;
  p->leng = data_len;

  /* init next parameter for no more parameter */

  p++;
  p->name = NULL;
  p->nlen = 0;
  p->type = 0;
  p->addr = NULL;
  p->leng = 0;

  return;
}


/*--------------------------------------------------------------------*/
/* Error Cleanup because of an RFC-Error                              */
/* Because of Windows DLL function must not be defined as static      */
/*--------------------------------------------------------------------*/
void DLL_CALL_BACK_FUNCTION rfc_error (rfc_char_t *operation )
{
  RFC_ERROR_INFO_EX  error_info;

  sprintfU (tbuf, cU("\n<==  RfcLastErrorEx\n"));

  sprintfU (tbuf+strlenU (tbuf), cU("\nRFC Call/Exception: %s\n"), operation);
  TRFC_trace(tbuf);

  RfcLastErrorEx(&error_info);

  sprintfU (tbuf+strlenU (tbuf), cU("Group       Error group %d\n"), error_info.group);
  sprintfU (tbuf+strlenU (tbuf), cU("Key         %s\n"), error_info.key);
  sprintfU (tbuf+strlenU (tbuf), cU("Message     %s\n"), error_info.message);
  TRFC_trace(tbuf);

  sprintfU (tbuf, cU("\n<==  RfcClose\n"));
  TRFC_trace(tbuf);

  RfcClose(RFC_HANDLE_NULL);

  if (trace_fp != NULL)
    fclose(trace_fp);

  exit(1);
}


/*--------------------------------------------------------------------*/
/* Issue RfcAbort with Abort Text because of an Application Error     */
/*--------------------------------------------------------------------*/
void function_abort (rfc_char_t *atext)
{
  sprintfU (tbuf, cU("\nRfcAbort   '%s'\n"), atext);
  TRFC_trace(tbuf);

  RfcAbort(rfc_handle, atext);

  if (trace_fp != NULL)
    fclose(trace_fp);

  exit(1);
}


/*--------------------------------------------------------------------*/
/* Write Trace Info into trace_file                                   */
/*--------------------------------------------------------------------*/
void TRFC_trace (rfc_char_t *text)
{
  if (trace_fp != NULL)
  {
    fputsU (text, trace_fp);
    fflush (trace_fp);
  }
  fprintfU (stderr, cU("%s"), text);
  fflush (stderr);

  return;
}


/*--------------------------------------------------------------------*/
/* Create file name                                                   */
/*--------------------------------------------------------------------*/
static void create_file_name (rfc_char_t *filename)
{
  if (file_nr == 0)
  {
    file_nr = time(NULL);
    znr = (unsigned int) file_nr/2;
    srand(znr);
    file_nr = getpid() + rand();
  }
  else
    file_nr++;

  strcpyU (filename,  working_dir);

#if defined(SAPonUNIX) || defined(SAPonNT) || defined(SAPonOS2_2x) || defined(SAPonOS400) || defined(SAPonOS390)
  sprintfU (filename+strlenU (filename),   cU("tr%u"), file_nr);

#elif defined(SAPonVMS)
  sprintfU (filename+strlenU (filename),   cU("tr_%X"), file_nr);

#else
#error "Error Message > Please update this function for your OS"
#endif

  return;
}


/*--------------------------------------------------------------------*/
/* Output help for starting program                                   */
/*--------------------------------------------------------------------*/
static void help(void)
{
  printfU ( NL                                                             );
  printfU (cU("Syntax for start and run in register mode:")                NL );
  printfU (cU(" ")                                                         NL );
  printfU (cU("  trfcserv [options]")                                      NL );
  printfU (cU(" ")                                                         NL );
  printfU (cU("  with")                                                    NL );
  printfU (cU("  options = -D<destination with type 'R' in saprfc.ini>")   NL );
  printfU (cU("          = -t             RFC-Trace on")                   NL );
  printfU (cU("  or")                                                      NL );
  printfU (cU("  options = -a<program ID> e.g.  <own host name>.trfcserv") NL );
  printfU (cU("          = -g<SAP gateway host name>        e.g.  hs0311") NL );
  printfU (cU("          = -x<SAP gateway service>          e.g. sapgw53") NL );
  printfU (cU("          = -t             RFC-Trace on")                   NL );
  printfU (cU("          = -L<SNC library> e.g./krb5/hpux/lib/libkrb5.sl") NL );
  printfU (cU("          = -S<SNC own name>         e.g. s:sample@hw1145") NL );
  printfU (cU(" ")                                                         NL );
  printfU (cU(" Option L and S are only necessary if working with SNC")    NL );
  printfU (cU(" (Secure Network Communication).")                          NL );
  printfU (cU(" ")                                                         NL );
  return;
}
