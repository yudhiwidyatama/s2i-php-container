/* static rfc_char_t sccsid[] = "@(#) $Id: //bas/640_REL/src/krn/rfc/srfcserv.c#4 $ SAP"; */

/*====================================================================*/
/*                                                                    */
/*     PROGRAM     :   srfcserv.c (running on Windows, Windows_NT,    */
/*                                 Windows_95, OS/2 and R/3-Platforms)*/
/*                                                                    */
/*                                                                    */
/*     DESCRIPTION :   Server-Program for RFC Test                    */
/*                                                                    */
/*                     Following functions are available:             */
/*                                                                    */
/*                       - STFC_CONNECTION                            */
/*                                                                    */
/*                       - STFC_PERFORMANCE                           */
/*                                                                    */
/*                       - STFC_STRUCTURE                             */
/*                                                                    */
/*                       - STFC_CONNECTION_BACK                       */
/*                                                                    */
/*                       - RFC_RAISE_ERROR                            */
/*                                                                    */
/*                       - STFC_STRING                                */
/*                                                                    */
/*                     and can be called from R/2, R/3 or External    */
/*                                                                    */
/*                                                                    */
/*     SAP AG Walldorf                                                */
/*     Systeme, Anwendungen und Produkte in der Datenverarbeitung     */
/*                                                                    */
/*     Copyright (C) SAP AG 1994                                      */
/*                                                                    */
/*====================================================================*/

/*CCQ_FILE_OFF*/

/*--------------------------------------------------------------------*/
/* Set up includes                                                    */
/*--------------------------------------------------------------------*/

#if defined(SAPonOS400) 
#include "saptype.h"
#endif

#include "srfcserv.h"


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
    setbuf (stderr, NULL);

    if (argc == 1)
    {
        help();
    
        return 0;
    }


    /*------------------------------------------------------------------*/
    /* Output argument list                                             */
    /*------------------------------------------------------------------*/
    fprintfU(stderr, cU("\nargc    = %d"), argc);
    
    for (i=0; i<argc; i++)
    {
        fprintfU(stderr, cU("\nargv[%d] = '%s'"), i, argv[i]);
    }
    
    
    /*------------------------------------------------------------------*/
    /* Check working in register mode for SNC check                     */
    /*------------------------------------------------------------------*/
    SNC_checked = 0;

    register_mode = 0;
    
    for (i=1; i<argc; i++)
    {
        if ((memcmpU(argv[i], cU("-D"), 2) == 0) || (memcmpU(argv[i], cU("-a"), 2) == 0))
        {
            register_mode = 1;
            
            break;
        }
    }


    /*------------------------------------------------------------------*/
    /* Accept RFC-Connection                                            */
    /*------------------------------------------------------------------*/
    fprintfU(stderr, cU("\n<==  RfcAccept                rfc_handle = "));

    rfc_handle = RfcAccept(argv);
    
    fprintfU(stderr, cU("%u"), rfc_handle);

    if (rfc_handle == RFC_HANDLE_NULL)
    {
        rfc_error( cU("RfcAccept"));
    }
    
    
    /*------------------------------------------------------------------*/
    /* Install offering functions                                       */
    /*------------------------------------------------------------------*/
    rfc_rc = install();
    
    if (rfc_rc != RFC_OK)
    {
        return 1;
    }


#ifdef SAP_RFC_GETNAME
    /*------------------------------------------------------------------*/
    /* Get function name directly and and start it                      */
    /*------------------------------------------------------------------*/
    for (;;)
    {
        fprintfU(stderr, cU("\n\nWait for next RFC call with RfcGetNameEx"));

        fprintfU(stderr, cU("\n<==  RfcGetNameEx             rfc_rc = "));
    
        rfc_rc = RfcGetNameEx(rfc_handle, function_name);
    
        fprintfU(stderr, cU("%d"), rfc_rc);

        if (rfc_rc == RFC_OK)
        {
            fprintfU(stderr, cU("   Function Name: '%s'"), function_name);

            /* Check working with SNC and if yes, SNC name and ACL Key ok */
            SNC_check();

            if ((strcmpU (function_name, name_connection_test))  &&
                (strcmpU (function_name, name_performance_test)) &&
                (strcmpU (function_name, name_structure_test))   &&
                (strcmpU (function_name, name_callback_test))    &&
                (strcmpU (function_name, name_raise_error))    &&
                (strcmpU (function_name, name_string_test)))
            {
                sprintfU(abort_text, cU("RFC function %s is not implemented in this server program"), function_name);
    
                function_abort(abort_text);
            }
            else
            {
                if (strcmpU(function_name, name_connection_test) == 0)
                {
                    rfc_rc = connection_test(rfc_handle);
                }
                else if (strcmpU(function_name, name_performance_test) == 0)
                {
                    rfc_rc = performance_test(rfc_handle);
                }
                else if (strcmpU(function_name, name_structure_test) == 0)
                {
                    rfc_rc = structure_test(rfc_handle);
                }
                else if (strcmpU(function_name, name_callback_test) == 0)
                {
                    rfc_rc = callback_test(rfc_handle);
                }
                else if (strcmpU(function_name, name_raise_error) == 0)
                {
                    rfc_rc = rfc_raise_error(rfc_handle);
                }
                else if (strcmpU(function_name, name_string_test) == 0)
                {
                    rfc_rc = string_test(rfc_handle);
                }

                if (rfc_rc != RFC_OK)
                {
                    break;
                }
            }
        }
        else
        {
            if (rfc_rc == RFC_SYSTEM_CALLED)
            {
                continue;
            }
    
            if (rfc_rc != RFC_CLOSED)
            {
                rfc_error(cU("RfcGetNameEx"));
            }
    
            break;
        }
    }
#else
    /*------------------------------------------------------------------*/
    /* Wait for RFC call                                                */
    /*------------------------------------------------------------------*/
    do
    {
        fprintfU(stderr, cU("\n\nWait for next RFC call with or without RfcListen/RfcWaitForRequest"));

#if defined(SAP_RFC_WAIT) || defined(SAP_RFC_LISTEN)
        for (rfc_rc = RFC_RETRY; rfc_rc == RFC_RETRY;)
        {
#ifdef SAP_RFC_WAIT
            RFC_INT wtime = 60;     /* 60 sec. */
    
            fprintfU(stderr, cU("\n<==  RfcWaitForRequest        rfc_rc = "));
    
            rfc_rc = RfcWaitForRequest (rfc_handle, wtime);
#else
            fprintfU(stderr, cU("\n<==  RfcListen                rfc_rc = "));
    
            rfc_rc = RfcListen(rfc_handle);
#endif
            fprintfU(stderr, cU("%d"), rfc_rc);
    
            if (rfc_rc == RFC_RETRY)
            {
                /* do something else while waiting for next RFC calls */
#ifdef SAPonUNIX
                sleep(1);
#elif defined(SAPonNT) || defined(OS2)
                Sleep(1000);
#else
                rfc_rc = rfc_rc;
#endif
            }
        }

        if (rfc_rc != RFC_OK)
        {
            if (rfc_rc != RFC_CLOSED)
            {
                rfc_error(cU("RfcListen"));
            }
            else
            {
                break;
            }
        }

        /* Check working with SNC and if yes, SNC name and ACL Key ok */
        SNC_check();
#endif

        fprintfU(stderr, cU("\n<==  RfcDispatch ....."));
    
        rfc_rc = RfcDispatch(rfc_handle);
    
        fprintfU(stderr, cU("\n<==  RfcDispatch              rfc_rc = %d"), rfc_rc);
    } while (rfc_rc == RFC_OK);

    if (rfc_rc != RFC_CLOSED)
    {
        rfc_error(cU("RfcDispatch"));
    }
#endif

    
    return 0;
}


/*====================================================================*/
/*                                                                    */
/* Install the RFC-functions offered in this program                  */
/*                                                                    */
/*====================================================================*/
static RFC_RC install(void)
{
  /* Install STFC_CONNECTION */
  fprintfU(stderr, cU("\n<==  RfcInstallFunction (%s)      rfc_rc = "), name_connection_test);

#ifdef SAP_RFC_GETNAME
  /* Necessary for access docu_function by sm59 (system information) */
  rfc_rc = RfcInstallFunction(name_connection_test,
                              (RFC_ONCALL) NULL,
                              connection_test_docu());
#else
  rfc_rc = RfcInstallFunction(name_connection_test,
                              (RFC_ONCALL) connection_test,
                              connection_test_docu());
#endif

  fprintfU(stderr, cU("%d"), rfc_rc);

  if( rfc_rc != RFC_OK )
  {
    sprintfU(abort_text, cU("\nERROR: Install (%s)      rfc_rc = %d"), name_connection_test, rfc_rc);
    function_abort(abort_text);
  }

  /* Install STFC_PERFORMANCE */
  fprintfU(stderr, cU("\n<==  RfcInstallFunction (%s)     rfc_rc = "), name_performance_test);

#ifdef SAP_RFC_GETNAME
  /* Necessary for access docu_function by sm59 (system information) */
  rfc_rc = RfcInstallFunction(name_performance_test,
                              (RFC_ONCALL) NULL,
                              performance_test_docu());
#else
  rfc_rc = RfcInstallFunction(name_performance_test,
                              (RFC_ONCALL) performance_test,
                              performance_test_docu());
#endif

  fprintfU(stderr, cU("%d"), rfc_rc);

  if( rfc_rc != RFC_OK )
  {
    sprintfU (abort_text, cU("\nERROR: Install (%s)     rfc_rc = %d"),
              name_performance_test, rfc_rc);
    function_abort(abort_text);
  }

  /* Install STFC_STRUCTURE */
  fprintfU(stderr, cU("\n<==  RfcInstallFunction (%s)       rfc_rc = "),
           name_structure_test);

#ifdef SAP_RFC_GETNAME
  /* Necessary for access docu_function by sm59 (system information) */
  rfc_rc = RfcInstallFunction(name_structure_test,
                              (RFC_ONCALL) NULL,
                              structure_test_docu());
#else
  rfc_rc = RfcInstallFunction(name_structure_test,
                              (RFC_ONCALL) structure_test,
                              structure_test_docu());
#endif

  fprintfU(stderr, cU("%d"), rfc_rc);

  if( rfc_rc != RFC_OK )
  {
    sprintfU (abort_text, cU("\nERROR: Install (%s)     rfc_rc = %d"),
              name_structure_test, rfc_rc);
    function_abort(abort_text);
  }

  /* Install STFC_CONNECTION_BACK */
  fprintfU (stderr, cU("\n<==  RfcInstallFunction (%s) rfc_rc = "),
            name_callback_test);

#ifdef SAP_RFC_GETNAME
  /* Necessary for access docu_function by sm59 (system information) */
  rfc_rc = RfcInstallFunction(name_callback_test,
                              (RFC_ONCALL) NULL,
                              callback_test_docu());
#else
  rfc_rc = RfcInstallFunction(name_callback_test,
                              (RFC_ONCALL) callback_test,
                              callback_test_docu());
#endif

  fprintfU(stderr, cU("%d"), rfc_rc);

  if( rfc_rc != RFC_OK )
  {
    sprintfU (abort_text, cU("\nERROR: Install (%s)      rfc_rc = %d"),
              name_connection_test, rfc_rc);
    function_abort(abort_text);
  }

  /* Install STFC_RAISE_ERROR */
  fprintfU(stderr, cU("\n<==  RfcInstallFunction (%s)      rfc_rc = "), name_raise_error);

#ifdef SAP_RFC_GETNAME
  /* Necessary for access docu_function by sm59 (system information) */
  rfc_rc = RfcInstallFunction(name_raise_error,
                              (RFC_ONCALL) NULL,
                              rfc_raise_error_docu());
#else
  rfc_rc = RfcInstallFunction(name_raise_error,
                              (RFC_ONCALL) rfc_raise_error,
                              rfc_raise_error_docu());
#endif

  fprintfU(stderr, cU("%d"), rfc_rc);

  if( rfc_rc != RFC_OK )
  {
    sprintfU(abort_text, cU("\nERROR: Install (%s)      rfc_rc = %d"), name_raise_error, rfc_rc);
    function_abort(abort_text);
  }

  /* Install STFC_STRING */
  fprintfU (stderr, cU("\n<==  RfcInstallFunction (%s) rfc_rc = "),
            name_string_test);

#ifdef SAP_RFC_GETNAME
  /* Necessary for access docu_function by sm59 (system information) */
  rfc_rc = RfcInstallFunction(name_string_test,
                              (RFC_ONCALL) NULL,
                              string_test_docu());
#else
  rfc_rc = RfcInstallFunction(name_string_test,
                              (RFC_ONCALL) string_test,
                              string_test_docu());
#endif

  fprintfU(stderr, cU("%d"), rfc_rc);

  if( rfc_rc != RFC_OK )
  {
    sprintfU (abort_text, cU("\nERROR: Install (%s)      rfc_rc = %d"),
              name_string_test, rfc_rc);
    function_abort(abort_text);
  }


  return RFC_OK;
}


/*====================================================================*/
/*                                                                    */
/* Get specific info about an RFC connection                          */
/*                                                                    */
/*====================================================================*/
void get_attributes(RFC_HANDLE rfc_handle)
{
    RFC_RC          rfc_rc = RFC_OK;
    RFC_CODE_PAGE   cpOwn, 
                    cpRemote, 
                    cpComm;
    SAP_CHAR        cRepl;
    int             iIgnoreErrors = 0, 
                    iPcs = 0;


    fprintfU(stderr, cU("\n<==  RfcGetAttributes         rc = "));
    
    rc = RfcGetAttributes (rfc_handle, &rfc_attributes);
    
    fprintfU(stderr, cU("%d"), rc);
    
    
    if (rc)
    {
        rfc_error (cU("RfcGetAttributes"));
    }


    fprintfU (stderr, cU("\n\nAttributes of this RFC connection"));
    fprintfU (stderr, cU("\n---------------------------------"));
    fprintfU (stderr, cU("\nDestination            :  %s"), rfc_attributes.dest);
    fprintfU (stderr, cU("\nMy Host                :  %s"), rfc_attributes.own_host);

    
    if (rfc_attributes.rfc_role == RFC_ROLE_CLIENT)
    {
        if (rfc_attributes.partner_type == RFC_SERVER_EXT)
        {
            fprintfU (stderr, cU("\nServer Program Name    :  %s"), rfc_attributes.partner_host);
        }
        else if (rfc_attributes.partner_type == RFC_SERVER_EXT_REG)
        {
            fprintfU (stderr, cU("\nServer Program ID      :  %s"), rfc_attributes.partner_host);
        }
        else
        {
            fprintfU (stderr, cU("\nPartner Host           :  %s"), rfc_attributes.partner_host);
        }
    }
    else
    {
        fprintfU (stderr, cU("\nPartner Host           :  %s"), rfc_attributes.partner_host);
    }


    fprintfU (stderr, cU("\nSystem No.             :  %s"), rfc_attributes.systnr);
    fprintfU (stderr, cU("\nSystem Name            :  %s"), rfc_attributes.sysid);
    fprintfU (stderr, cU("\nClient                 :  %s"), rfc_attributes.client);
    fprintfU (stderr, cU("\nUser                   :  %s"), rfc_attributes.user);
    fprintfU (stderr, cU("\nLanguage               :  %s"), rfc_attributes.language);
    fprintfU (stderr, cU("\nISO-Language           :  %s"), rfc_attributes.ISO_language);

    if (rfc_attributes.trace == cU('X'))
    {
        fprintfU (stderr, cU("\nRFC Trace              :  ON"));
    }
    else
    {
        fprintfU (stderr, cU("\nRFC Trace              :  OFF"));
    }

    fprintfU (stderr, cU("\nMy Codepage            :  %s"), rfc_attributes.own_codepage);
    fprintfU (stderr, cU("\nPartner Codepage       :  %s"), rfc_attributes.partner_codepage);
    fprintfU (stderr, cU("\nReal Partner Codepage  :  %s"), rfc_attributes.real_partner_codepage);

    if (rfc_attributes.rfc_role == RFC_ROLE_CLIENT)
    {
        fprintfU (stderr, cU("\nRFC Role               :  External RFC Client"));
    }
    else if (rfc_attributes.own_type == RFC_SERVER_EXT)
    {
        fprintfU (stderr, cU("\nRFC Role               :  External RFC Server, started by SAP gateway"));
    }
    else
    {
        fprintfU (stderr, cU("\nRFC Role               :  External RFC Server, registered at SAP gateway"));
    }


    fprintfU (stderr, cU("\nRFC Library Release    :  %s"), rfc_attributes.own_rel);

    if (rfc_attributes.partner_type == RFC_SERVER_R3)
    {
        fprintfU (stderr, cU("\nRFC Partner            :  SAP R/3"));
    }
    else if (rfc_attributes.partner_type == RFC_SERVER_R2)
    {
        fprintfU (stderr, cU("\nRFC Partner            :  SAP R/2"));
    }
    else if (rfc_attributes.rfc_role == RFC_ROLE_CLIENT)
    {
        if (rfc_attributes.partner_type == RFC_SERVER_EXT)
        {
            fprintfU (stderr, cU("\nRFC Partner            :  External RFC Server, started by SAP gateway"));
        }
        else
        {
            fprintfU (stderr, cU("\nRFC Partner            :  External RFC Server, registered at SAP gateway"));
        }
    }
    else
    {
        fprintfU (stderr, cU("\nRFC Partner            :  External RFC Client"));
    }


    fprintfU (stderr, cU("\nPartner System Release :  %s"),   rfc_attributes.partner_rel);
    fprintfU (stderr, cU("\nR/3 Kernel Release     :  %s"),   rfc_attributes.kernel_rel);
    fprintfU (stderr, cU("\nCPI-C Conversation ID  :  %s\n"), rfc_attributes.CPIC_convid);
    fprintfU (stderr, cU("\nProgram Name           :  %s\n"), rfc_attributes.progname);

    
    printfU (cU("\n<==  RfcGetCodePagesInfo      rc = "));    
    
    rfc_rc = RfcGetCodePagesInfo (rfc_handle, cpOwn, cpRemote, cpComm, &cRepl, &iIgnoreErrors, &iPcs);

    printfU (cU("%d\n"), rc);

    
    if (rfc_rc)
    {
        rfc_error(cU("RfcGetCodePagesInfo"));
    }

    printfU (cU("\nOwn code page            :  %s"), cpOwn);
    printfU (cU("\nRemote code page         :  %s"), cpRemote);
    printfU (cU("\nCommunication code page  :  %s"), cpComm);
    printfU (cU("\nActual PCS               :  %d\n"), iPcs);
    printfU (cU("\nReplacement character    :  0x%x"), (unsigned int)cRepl);
    printfU (cU("\nIgnore Errors            :  %d\n"), iIgnoreErrors);


    return;
}


/*====================================================================*/
/*                                                                    */
/* RFC-FUNCTION:  STFC_CONNECTION                                     */
/*                                                                    */
/* Send an echo and a response message to the RFC client program      */
/*                                                                    */
/*====================================================================*/
static RFC_RC DLL_CALL_BACK_FUNCTION connection_test(RFC_HANDLE rfc_handle)
{
    fprintfU (stderr, cU("\n\nStart Function '%s'"), name_connection_test);


    /* Check working with SNC and if yes, SNC name and ACL Key ok */
    SNC_check();

    
    memsetR (&parameters[0], 0, sizeofR(parameters));
    
    /* parameters */
    rfc_param (&parameters[0], cU("REQUTEXT"), TYPC, requtext, 255*sizeofR(SAP_UC));

    /* no table ---> terminate array */
    tables[0].name = NULL;

    
    /* receive data */
    fprintfU (stderr, cU("\n<==  RfcGetData               rfc_rc = "));
    
    rfc_rc = RfcGetData (rfc_handle, parameters, tables);
    
    fprintfU (stderr, cU("%d"), rfc_rc);
    
    if (rfc_rc != RFC_OK)
     rfc_error (cU("RfcGetData"));

    
    /* Get specific info of this RFC connection */
    get_attributes (rfc_handle);

    
    /* Get call info for an synchronous rfc call */
    rfc_call_info.call_type = cU('S');

    rfc_rc = RfcGetCallInfo (rfc_handle, &rfc_call_info);

    if (rfc_rc != RFC_OK)
     rfc_error (cU("RfcGetCallInfo"));

    fprintfU (stderr, cU("\nCall Info: function name: %s\n"),   rfc_call_info.function_name);
    fprintfU (stderr, cU("            call type :    %c\n\n"), rfc_call_info.call_type);
    

    /* Echo text */
    strcpyU (echotext, requtext);

    
    /* Info text */
    memsetU (resptext, BLANK, 255);
    
    memcpyU(resptext, cU("Sysid:"), 6);
    
    ptr = getenvU(cU("HOSTNAME"));
    
    if (ptr == NULL)
     ptr = getenvU(cU("USER"));
    
    if (ptr == NULL)
     ptr = getenvU(cU("LOGNAME"));
    
    if (ptr != NULL)
     strcpyU(resptext+7, ptr);
    else
     strcpyU(resptext+7, cU("????????"));
    
    resptext[strlenU(resptext)] = cU(' ');
    
    memcpyU (resptext+18, cU("Date & Time:"), 12);
    
    actutime = time(NULL);
    
    /*CCQ_CLIB_LOCTIME_OK*/
    time_ptr = localtime(&actutime);
    
    memcpyU(&resptext[31], asctimeU (time_ptr), 24);

    
    /*------------------------------------------------------------------*/
    /* Return parameters and tables                                     */
    /*------------------------------------------------------------------*/
    memsetR (&parameters[0], 0, sizeofR(parameters));

    /* parameters */
    rfc_param(&parameters[0], cU("ECHOTEXT"), TYPC, echotext, 255*sizeofR(SAP_UC));
    rfc_param(&parameters[1], cU("RESPTEXT"), TYPC, resptext, 255*sizeofR(SAP_UC));

    /* no table ---> terminate array */
    tables[0].name = NULL;

    
    /* return to the caller */
    fprintfU(stderr, cU("\n<==  RfcSendData              rfc_rc = "));
    
    rfc_rc = RfcSendData (rfc_handle, parameters, tables);
    
    fprintfU(stderr, cU("%d"), rfc_rc);

    if (rfc_rc != RFC_OK)
     rfc_error (cU("RfcSendData"));

    
    /* In register mode SNC check by calling each RFC function */
    if (register_mode)
     SNC_checked = 0;

    
    return rfc_rc;
}


/*====================================================================*/
/*                                                                    */
/* RFC-FUNCTION:  STFC_PERFORMANCE                                    */
/*                                                                    */
/* Send and Receive internal tables as required                       */
/*                                                                    */
/*====================================================================*/
static RFC_RC DLL_CALL_BACK_FUNCTION performance_test(RFC_HANDLE rfc_handle)
{
  fprintfU(stderr, cU("\n\nStart Function '%s'"), name_performance_test);

  /* Check working with SNC and if yes, SNC name and ACL Key ok */
  SNC_check();

  memsetR (&parameters[0], 0, sizeofR(parameters));

  /* parameter */
  rfc_param(&parameters[0], cU("CHECKTAB"), TYPC, checktab,       1*sizeofR(SAP_UC));
  rfc_param(&parameters[1], cU("LGIT0332"), TYPC, lgit0332_field, 5*sizeofR(SAP_UC));
  rfc_param(&parameters[2], cU("LGIT1000"), TYPC, lgit1000_field, 5*sizeofR(SAP_UC));

  rfc_param(&parameters[3], cU("LGET0332"), TYPC, lget0332_field, 5*sizeofR(SAP_UC));
  rfc_param(&parameters[4], cU("LGET1000"), TYPC, lget1000_field, 5*sizeofR(SAP_UC));

  /* Prepare internal table ITAB0332 */
  tables[0].name     = cU("ITAB0332");
  tables[0].nlen     = 8;
  tables[0].type     = TYPC;
  tables[0].leng     = ITAB0332_ENTRY_SIZE;
  tables[0].itmode   = RFC_ITMODE_BYREFERENCE;

  /* Prepare internal table ITAB1000 */
  tables[1].name     = cU("ITAB1000");
  tables[1].nlen     = 8;
  tables[1].type     = TYPC;
  tables[1].leng     = ITAB1000_ENTRY_SIZE;
  tables[1].itmode   = RFC_ITMODE_BYREFERENCE;

  /* Prepare internal table ETAB0332 */
  tables[2].name     = cU("ETAB0332");
  tables[2].nlen     = 8;
  tables[2].type     = TYPC;
  tables[2].leng     = ETAB0332_ENTRY_SIZE;
  tables[2].itmode   = RFC_ITMODE_BYREFERENCE;

  /* Prepare internal table ETAB1000 */
  tables[3].name     = cU("ETAB1000");
  tables[3].nlen     = 8;
  tables[3].type     = TYPC;
  tables[3].leng     = ETAB1000_ENTRY_SIZE;
  tables[3].itmode   = RFC_ITMODE_BYREFERENCE;

  /* terminate array */
  tables[4].name = NULL;

  /* receive data */
  fprintfU (stderr, cU("\n<==  RfcGetData               rfc_rc = "));
  rfc_rc = RfcGetData (rfc_handle,
                       parameters,
                       tables);
  fprintfU(stderr, cU("%d"), rfc_rc);

  if (rfc_rc != RFC_OK)
    rfc_error (cU("RfcGetData"));

  /* Get specific info of this RFC connection */
  get_attributes(rfc_handle);

  /* Receive length of all ITABnnnn and ETABnnnn */
  lgit0332 = atoiU (lgit0332_field);
  lgit1000 = atoiU (lgit1000_field);
  lget0332 = atoiU (lget0332_field);
  lget1000 = atoiU (lget1000_field);

  /* Check receive length and real length of ITAB0332 */
  check_table (tables[0].ithandle, cU("ITAB0332"), lgit0332, checktab, text0332, retcode);
  exitcode[0] = retcode[0];

  /* Check receive length and real length of ITAB1000 */
  check_table (tables[1].ithandle, cU("ITAB1000"), lgit1000, checktab, text1000, retcode);
  if (exitcode[0] == cU('O'))
    exitcode[0] = retcode[0];

  if (exitcode[0] == cU('O'))
  {
    /* Fill table ETAB0332 as required */
    if (fill_table(tables[2].ithandle, lget0332))
    {
      fprintfU (stderr, cU("\nERROR: Fill table etab0332\n"));
      exitcode[0] = cU('E');
    }
    else
    {
      /* Fill table ETAB1000 as required */
      if (fill_table(tables[3].ithandle, lget1000))
      {
        fprintfU (stderr, cU("\nERROR: Fill table etab1000\n"));
        exitcode[0] = cU('E');
      }
    }
  }

  /*------------------------------------------------------------------*/
  /* Return parameters and tables                                     */
  /*------------------------------------------------------------------*/

  memsetR (&parameters[0], 0, sizeofR(parameters));

  /* parameters */
  rfc_param (&parameters[0], cU("EXITCODE"), TYPC, exitcode, 1*sizeofR(SAP_UC));
  rfc_param (&parameters[1], cU("TEXT0332"), TYPC, text0332, 72*sizeofR(SAP_UC));
  rfc_param (&parameters[2], cU("TEXT1000"), TYPC, text1000, 72*sizeofR(SAP_UC));

  /* Do not refresh the importing tables for better performance */
  /* Be carreful, this is necessary because the RFC             */
  /* client does take care of the contents of this              */
  /* tables. (They will also be refreshed.)                     */ 
  /* ItFree(tables[0].ithandle);                                */
  /* ItFree(tables[1].ithandle);                                */

  /* return to the caller */
  fprintfU (stderr, cU("\n<==  RfcSendData              rfc_rc = "));
  rfc_rc = RfcSendData (rfc_handle,
                        parameters,
                        tables);
  fprintfU(stderr, cU("%d"), rfc_rc);

  if (rfc_rc != RFC_OK)
    rfc_error (cU("RfcSendData"));

  /* In register mode SNC check by calling each RFC function */
  if (register_mode)
    SNC_checked = 0;

  return rfc_rc;
}


/*====================================================================*/
/*                                                                    */
/* RFC-FUNCTION:  STFC_STRUCTURE                                      */
/*                                                                    */
/* Send and Receive inhomogeneous structure/table                     */
/*                                                                    */
/*====================================================================*/
static RFC_RC DLL_CALL_BACK_FUNCTION structure_test(RFC_HANDLE rfc_handle)
{
  const SAP_RAW hex3_value[] = { 0xf1, 0xf2, 0xf3 };

  fprintfU (stderr, cU("\n\nStart Function '%s'"), name_structure_test);

  /* Check working with SNC and if yes, SNC name and ACL Key ok */
  SNC_check();

  memsetR (&parameters[0], 0, sizeofR(parameters));

  /* Install structure 'RFCTEST' */
  rfc_rc = RfcInstallUnicodeStructure (cU("RFCTEST"),
                                       typeOfRfcTest,
                                       ENTRIES(typeOfRfcTest),
                                       0, NULL,
                                       &handleOfRfcTest );

  if (rfc_rc != RFC_OK)
    rfc_error (cU("RfcInstallStructure"));

  /* Importing Parameters */
  parameters[0].name = cU("IMPORTSTRUCT");
  parameters[0].nlen = 12;
  parameters[0].addr = &imstruct;
  parameters[0].leng = RFCTEST_LN;
  parameters[0].type = handleOfRfcTest;
  parameters[1].name = NULL;

  /* Prepare internal table RFCTABLE */
  tables[0].name     = cU("RFCTABLE");
  tables[0].nlen     = 8;
  tables[0].type     = handleOfRfcTest;
  tables[0].leng     = RFCTEST_LN;
  tables[0].itmode   = RFC_ITMODE_BYREFERENCE;
  tables[1].name     = NULL;

  /* receive data */
  fprintfU (stderr, cU("\n<==  RfcGetData               rfc_rc = "));
  rfc_rc = RfcGetData (rfc_handle,
                       parameters,
                       tables);

  fprintfU (stderr, cU("%d"), rfc_rc);

  if (rfc_rc != RFC_OK)
    rfc_error (cU("RfcGetData"));

  /* Get specific info of this RFC connection */
  get_attributes (rfc_handle);

  /* Echo structure */
  exstruct = imstruct;

  /* Info text */
  memsetU (resptext, BLANK, 255);
  memcpyU (resptext, cU("Sysid:"), 6);
  ptr = getenvU (cU("HOSTNAME"));
  if (ptr == NULL)
    ptr = getenvU (cU("USER"));
  if (ptr == NULL)
    ptr = getenvU (cU("LOGNAME"));
  if (ptr != NULL)
    strcpyU (resptext+7, ptr);
  else
    strcpyU (resptext+7, cU("????????"));
  resptext[strlenU(resptext)] = cU(' ');

  memcpyU (resptext+18, cU("Date & Time:"), 12);
  actutime = time(NULL);
  /*CCQ_CLIB_LOCTIME_OK*/
  time_ptr = localtime(&actutime);
  memcpyU (&resptext[31], asctimeU (time_ptr), 24);

  /* New structure value */
  imstruct.rfcfloat = imstruct.rfcfloat +1;
  imstruct.rfcchar1[0] = cU('X');
  imstruct.rfcint2  = imstruct.rfcint2  +1;
  imstruct.rfcint1  = imstruct.rfcint1  +1;
  memcpyU (imstruct.rfcchar4, cU("EXT."), 4);
  memcpyR (imstruct.rfchex3,  hex3_value, 3);
  memcpyU (imstruct.rfcchar2, cU("YZ"),       2);
  memcpyU (imstruct.rfctime,  cU("124500"),   6);
  memcpyU (imstruct.rfcdate,  cU("19960924"), 8);

  /* Append into table 'RFCTABLE' */
  lineleng = ItLeng(tables[0].ithandle);
  ptr = (rfc_char_t *) ItAppLine(tables[0].ithandle);
  if (ptr == NULL)
    rfc_error(cU("ItAppLine"));
  memcpyR ((void *)ptr, &imstruct, lineleng);

  /* Exporting Parameters */
  parameters[0].name = cU("RESPTEXT");
  parameters[0].nlen = 8;
  parameters[0].addr = resptext;
  parameters[0].leng = sizeofR (resptext);
  parameters[0].type = TYPC;

  parameters[1].name = cU("ECHOSTRUCT");
  parameters[1].nlen = 10;
  parameters[1].addr = &exstruct;
  parameters[1].leng = RFCTEST_LN;
  parameters[1].type = handleOfRfcTest;

  parameters[2].name = NULL;

  /* return to the caller */
  fprintfU (stderr, cU("\n<==  RfcSendData              rfc_rc = "));
  rfc_rc = RfcSendData (rfc_handle,
                        parameters,
                        tables);
  fprintfU (stderr, cU("%d"), rfc_rc);

  if (rfc_rc != RFC_OK)
    rfc_error (cU("RfcSendData"));

  /* In register mode SNC check by calling each RFC function */
  if (register_mode)
    SNC_checked = 0;

  return rfc_rc;
}


/*====================================================================*/
/*                                                                    */
/* RFC-FUNCTION:  STFC_CONNECTION_BACK                                */
/*                                                                    */
/* Callback the function STFC_CONNECTION "NRBACK"-times on the        */
/* RFC client before send an echo and a response message to the       */
/* RFC client program.                                                */
/*                                                                    */
/*====================================================================*/
static RFC_RC DLL_CALL_BACK_FUNCTION callback_test(RFC_HANDLE rfc_handle)
{
  fprintfU (stderr, cU("\n\nStart Function '%s'"), name_callback_test);

  /* Check working with SNC and if yes, SNC name and ACL Key ok */
  SNC_check();

  memsetR (&parameters[0], 0, sizeofR(parameters));

  /* parameters */
  memsetU (nrback_field, BLANK, 5);
  rfc_param (&parameters[0], cU("NRBACK"),   TYPC, nrback_field, 5*sizeofR(SAP_UC));
  rfc_param (&parameters[1], cU("REQUTEXT"), TYPC, requtext,     255*sizeofR(SAP_UC));

  /* no table ---> terminate array */
  tables[0].name = NULL;

  /* receive data */
  fprintfU (stderr, cU("\n<==  RfcGetData               rfc_rc = "));
  rfc_rc = RfcGetData (rfc_handle,
                       parameters,
                       tables);
  fprintfU (stderr, cU("%d"), rfc_rc);

  if (rfc_rc != RFC_OK)
    rfc_error(cU("RfcGetData"));

  /* Get specific info of this RFC connection */
  get_attributes(rfc_handle);

  /*------------------------------------------------------------------*/
  /* Depend on request: Call STFC_CONNECTION as callback              */
  /*------------------------------------------------------------------*/

  if (nrback_field[0] == BLANK)
    nrback = 0;
  else
    nrback = atoiU (nrback_field);

  noback = 0;
  for (nrcount = 0; nrcount < nrback; nrcount++)
  {
    function = name_connection_test;

    memsetR (&exporting[0], 0, sizeofR(exporting));
    memsetR (&importing[0], 0, sizeofR(importing));

    rfc_param (&exporting[0], cU("REQUTEXT"), TYPC, requtext, 255*sizeofR(SAP_UC));
    rfc_param (&importing[0], cU("ECHOTEXT"), TYPC, echotext, 255*sizeofR(SAP_UC));
    rfc_param (&importing[1], cU("RESPTEXT"), TYPC, resptext, 255*sizeofR(SAP_UC));

    /* no table ---> terminate array */
    tables[0].name = NULL;

    fprintfU (stderr, cU("\n<==  RfcCallReceive (%s)     rfc_rc = "), function);

    rfc_rc = RfcCallReceive (rfc_handle,
                             function,
                             exporting,
                             importing,
                             tables,
                             &exception);
    fprintfU (stderr, cU("%d\n"), rfc_rc);

    /* check return code */
    switch(rfc_rc)
    {
      case RFC_OK:
        break;
      case RFC_EXCEPTION:
      case RFC_SYS_EXCEPTION:
        rfc_error (exception);
        break;
      default:
        rfc_error (cU("RfcCallReceive"));
        break;
    }
    noback = noback + 1;
  }

  /* No. of executed callback's */
  sprintfU (noback_field, cU("%05d"), noback);

  /* Echo text */
  strcpyU (echotext, requtext);

  /* Info text */
  memsetU (resptext, BLANK, 255);
  memcpyU (resptext, cU("Sysid:"), 6);
  ptr = getenvU (cU("HOSTNAME"));
  if (ptr == NULL)
    ptr = getenvU (cU("USER"));
  if (ptr == NULL)
    ptr = getenvU (cU("LOGNAME"));
  if (ptr != NULL)
    strcpyU (resptext+7, ptr);
  else
    strcpyU (resptext+7, cU("????????"));
  resptext[strlenU(resptext)] = cU(' ');

  memcpyU(resptext+18, cU("Date & Time:"), 12);
  actutime = time(NULL);
  /*CCQ_CLIB_LOCTIME_OK*/
  time_ptr = localtime(&actutime);
  memcpyU (&resptext[31], asctimeU (time_ptr), 24);

  /*------------------------------------------------------------------*/
  /* Return parameters and tables                                     */
  /*------------------------------------------------------------------*/

  memsetR (&parameters[0], 0, sizeofR(parameters));

  /* parameters */
  rfc_param (&parameters[0], cU("NOBACK"),   TYPC, noback_field, 5*sizeofR(SAP_UC));
  rfc_param (&parameters[1], cU("ECHOTEXT"), TYPC, echotext,     255*sizeofR(SAP_UC));
  rfc_param (&parameters[2], cU("RESPTEXT"), TYPC, resptext,     255*sizeofR(SAP_UC));

  /* no table ---> terminate array */
  tables[0].name = NULL;

  /* return to the caller */
  fprintfU (stderr, cU("\n<==  RfcSendData              rfc_rc = "));
  rfc_rc = RfcSendData (rfc_handle,
                        parameters,
                        tables);
  fprintfU (stderr, cU("%d"), rfc_rc);

  if (rfc_rc != RFC_OK)
    rfc_error (cU("RfcSendData"));

  /* In register mode SNC check by calling each RFC function */
  if (register_mode)
    SNC_checked = 0;

  return rfc_rc;
}


/*====================================================================*/
/*                                                                    */
/* RFC-FUNCTION:  RFC_RAISE_ERROR                                     */
/*                                                                    */
/*  raises an error	                                                  */
/*                                                                    */
/*====================================================================*/
static RFC_RC DLL_CALL_BACK_FUNCTION rfc_raise_error( RFC_HANDLE handle )
{
    /* variables declaration */
    int div_by_zero = 0;
    int i_method    = 0; /* Error method as int */
    int raised      = 1; /* if an error is raised */
    
    RFC_PARAMETER    import[4];
    RFC_PARAMETER    export[3];
    RFC_TABLE        table[1];
    RFC_RC           rc;

    RFC_INT          counter = 0;
    RFC_CHAR         method[4];
    RFC_CHAR         messagetype[2] = iU("S");
    RFC_CHAR         parameter[250];
    RFC_CHAR         return_value[250] = iU(" ");
    
    /* variables for call back */
    rfc_char_t          *exception  = NULL;
    RFC_ERROR_INFO_EX   error_info;
    RFC_PARAMETER       cbexport[2];
    RFC_CHAR            cbmethod[4] = iU("0");
    RFC_CHAR            msg006[] = iU("Function not supported");
    /* end of variables declaration */
    
    memsetU (parameter, cU(' '), 250);
    memsetU (method, cU(' '), 4);
    
    
    memsetR (&export[0], 0, sizeofR (export));
    memsetR (&import[0], 0, sizeofR (import));
    memsetR (&table[0], 0, sizeofR (table));
    

    /* declaration of meta data  */
    import[0].name = cU("METHOD");
    import[0].nlen = 6;
    import[0].addr = &method;
    import[0].leng = sizeofR(method);
    import[0].type = RFCTYPE_CHAR;
    
    import[1].name = cU("MESSAGETYPE");
    import[1].nlen = 11;
    import[1].addr = &messagetype;
    import[1].leng = sizeofR(messagetype);
    import[1].type = RFCTYPE_CHAR;
    
    import[2].name = cU("PARAMETER");
    import[2].nlen = 9;
    import[2].addr = &parameter;
    import[2].leng = sizeofR(parameter);
    import[2].type = RFCTYPE_CHAR;
	
	import[3].name = NULL;

    export[0].name = cU("COUNTER");
    export[0].nlen = 7;
    export[0].addr = &counter;
    export[0].leng = sizeofR(counter);
    export[0].type = RFCTYPE_INT;
    
    export[1].name = cU("RETURN_VALUE");
    export[1].nlen = 12;
    export[1].addr = &return_value;
    export[1].leng = sizeofR(return_value);
    export[1].type = RFCTYPE_CHAR;
    
	export[2].name = NULL;

    table[0].name = NULL;
    /* end of declaration of meta data  */

    rc = RfcGetData( handle, import, NULL );
    
    if( rc != RFC_OK ) return rc;
    
    /* print out METHOD and PARAMETER */
    fprintfU (stderr, cU("\nRAISE ERROR No. \"%.4s\"\n"), method);
    fprintfU (stderr, cU("\nPARAMETER VALUE \"%.4s\"\n"), parameter);
    
    /* which error should be raised */
    i_method = atoiU (method);
    
    /* abort the connection */
    if (i_method == 1)
    {
        raised = 0;
        
        fprintfU (stderr, cU("\n<=== ABORT THE CONNECTION\n"));
        
        RfcAbort ( handle, msg006 );
    }
    
    /* division by zero */
    if (i_method == 4)
    {
        raised = 0;
        fprintfU (stderr, cU("\n<=== DIVISION BY ZERO\n"));
        div_by_zero = 1/div_by_zero; 
    }
    
    /* call back RFCPING */
    if (i_method == 11)
    {
        raised = 0;
        
        fprintfU (stderr, cU("\n<=== CALL BACK \"RFC_PING\"\n"));
        
        rc = RfcCallReceive(handle, cU("RFCPING"), NULL, NULL, NULL, &exception);
        
        if (rc != RFC_OK)
        {
            RfcLastErrorEx(&error_info);

            return rc;
        }
    }
    
    /* call back RFC_RAISE_ERROR */
    if (i_method == 20)
    {
        raised = 0;
        
        cbexport[0].name = cU("METHOD");
        cbexport[0].nlen = 6;
        cbexport[0].addr = &parameter;
        cbexport[0].leng = sizeofR(cbmethod);
        cbexport[0].type = RFCTYPE_CHAR;
        cbexport[1].name = NULL;
        
        fprintfU (stderr, cU("\n<=== CALL BACK \"RFC_RAISE_ERROR\" with method %.4s\n"), parameter);
        
        rc = RfcCallReceive(handle, cU("RFC_RAISE_ERROR"), cbexport, NULL, NULL, &exception);
        
        if (rc != RFC_OK)
        {
            RfcLastErrorEx(&error_info);

            return rc;
        }
        
    }   
    if (raised)
    {
        fprintfU (stderr, cU("\n<=== %s \n"), msg006);
    }                 

    rc = RfcSendData( handle, export, table );

    return rc;
} /* rfc_raise_error */


/*====================================================================*/
/*                                                                    */
/* RFC-FUNCTION:  STFC_STRING                                         */
/*                                                                    */
/*  Send and receive strings	                                      */
/*                                                                    */
/*====================================================================*/
static RFC_RC DLL_CALL_BACK_FUNCTION string_test(RFC_HANDLE rfc_handle)
{
    fprintfU (stderr, cU("\n\nStart Function '%s'"), name_performance_test);


    /* Check working with SNC and if yes, SNC name and ACL Key ok */
    SNC_check();

    
    memsetR (&parameters[0], 0, sizeofR(parameters));


    /* Importing Parameters */
    parameters[0].name = cU("QUESTION");
    parameters[0].nlen = strlenU ((rfc_char_t*) parameters[0].name);
    parameters[0].addr = &questions;
    parameters[0].leng = 0; /* You do not know how long is the received string */ 
    parameters[0].type = RFCTYPE_STRING;

    parameters[1].name = NULL;

    tables[0].name = NULL; /* no tables */

    /* receive data */
    fprintfU (stderr, cU("\n<==  RfcGetData               rfc_rc = "));
              rfc_rc = RfcGetData (rfc_handle,
              parameters,
              tables);
    
    fprintfU (stderr, cU("%d"), rfc_rc);

    
    if (rfc_rc != RFC_OK)
     rfc_error (cU("RfcGetData"));

    
    /* Get specific info of this RFC connection */
    get_attributes(rfc_handle);

    
    fprintfU (stderr, cU(" The received import parameter 'QUESTION' has content: %s\n"), questions);

    
    /* Initialize answer string */  
    answer = RfcAllocString (12); /* RfcAllocStrings allocs automatically  13 bytes */
    
    /*SAPUNICODEOK_LIBFCT*//*SAPUNICODEOK_CHARTYPE*/
    strcpy ((char*) answer, cR("Fine, thanks")); /* strings are UTF8 not UTF16 */

    /* Exporting Parameters */
    parameters[0].name = cU("MYANSWER");
    parameters[0].nlen = strlenU ((rfc_char_t *)parameters[0].name);
    parameters[0].addr = &answer;
    /*SAPUNICODEOK_LIBFCT*//*SAPUNICODEOK_CHARTYPE*/
    parameters[0].leng = strlen ((char*) answer); /* the value does not matter, 
                                                     because rfc lib calculates 
                                                     it using strlen automatically
                                                   */
    parameters[0].type = RFCTYPE_STRING;

    parameters[1].name = NULL;  

    tables[0].name = NULL;    /* no tables */


    /* return to the caller */
    fprintfU (stderr, cU("\n<==  RfcSendData              rfc_rc = "));
    
    
    rfc_rc = RfcSendData (rfc_handle,
                          parameters,
                          tables);
    
    
    fprintfU (stderr, cU("%d"), rfc_rc);

    if (rfc_rc != RFC_OK)
     rfc_error (cU("RfcSendData"));

    
    /* In register mode SNC check by calling each RFC function */
    if (register_mode)
    {
        SNC_checked = 0;
    }


    /* release both strings */
    RfcFreeString (questions);
    RfcFreeString (answer);

    
    return rfc_rc;
}


/*====================================================================*/
/*                                                                    */
/* Documentation for function STFC_CONNECTION                         */
/*                                                                    */
/*====================================================================*/
static rfc_char_t *connection_test_docu (void)
{
  static rfc_char_t docu[] =
  iU("RFC-Client has to send a message to this function and wait   ")     NL_AR
  iU("for the echo and a response message (max. 255 B).            ")     NL_AR
  iU("")                                                                  NL_AR
  iU("IMPORTING")                                                         NL_AR
  iU("  REQUTEXT        C(255)")                                          NL_AR
  iU("EXPORTING")                                                         NL_AR
  iU("  ECHOTEXT        C(255)")                                          NL_AR
  iU("  RESPTEXT        C(255)")                                          NL_AR
   ;
  return docu;
}


/*====================================================================*/
/*                                                                    */
/* Documentation for function STFC_CONNECTION_BACK                    */
/*                                                                    */
/*====================================================================*/
static rfc_char_t *callback_test_docu(void)
{
  static rfc_char_t docu[] =
  iU("RFC-Client has to send a message to this function and wait   ")     NL_AR
  iU("for the echo and a response message (max. 255 B).            ")     NL_AR
  iU("During the execution of this function, the RFC function      ")     NL_AR
  iU("named STFC_CONNECTION will be called 'NRBACK'-times.         ")     NL_AR
  iU("")                                                                  NL_AR
  iU("IMPORTING")                                                         NL_AR
  iU("  NRBACK          C(5)")                                            NL_AR
  iU("  REQUTEXT        C(255)")                                          NL_AR
  iU("EXPORTING")                                                         NL_AR
  iU("  ECHOTEXT        C(255)")                                          NL_AR
  iU("  RESPTEXT        C(255)")                                          NL_AR
   ;
  return docu;
}


/*====================================================================*/
/*                                                                    */
/* Documentation for function STFC_PERFORMANCE                        */
/*                                                                    */
/*====================================================================*/
static rfc_char_t *performance_test_docu(void)
{
  static rfc_char_t docu[] =
  iU("RFC-Client can send data in max. 2 different tables and then ")     NL_AR
  iU("wait for response (data in max. 2 different tables).         ")     NL_AR
  iU("")                                                                  NL_AR
  iU("RFC-Client must define:                                      ")     NL_AR
  iU("     - the number of lines in 4 tables as importing parameter")     NL_AR
  iU("     - the parameter checktab (Y/N) for checking the contents")     NL_AR
  iU("       of the 2 importing tables (ITAB0332 and ITAB1000)     ")     NL_AR
  iU("")                                                                  NL_AR
  iU("IMPORTING")                                                         NL_AR
  iU("  CHECKTAB        C(1)")                                            NL_AR
  iU("  LGIT0332        C(5)")                                            NL_AR
  iU("  LGIT1000        C(5)")                                            NL_AR
  iU("  LGET0332        C(5)")                                            NL_AR
  iU("  LGET1000        C(5)")                                            NL_AR
  iU("EXPORTING")                                                         NL_AR
  iU("  EXITCODE        C(1)")                                            NL_AR
  iU("  TEXT0332        C(72)")                                           NL_AR
  iU("  TEXT1000        C(72)")                                           NL_AR
  iU("TABLES")                                                            NL_AR
  iU("  ITAB0332        C(0332)")                                         NL_AR
  iU("  ITAB1000        C(1000)")                                         NL_AR
  iU("  ETAB0332        C(0332)")                                         NL_AR
  iU("  ETAB1000        C(1000)")                                         NL_AR
   ;
  return docu;
}


/*====================================================================*/
/*                                                                    */
/* Documentation for function STFC_STRUCTURE                          */
/*                                                                    */
/*====================================================================*/
static rfc_char_t *structure_test_docu(void)
{
  static rfc_char_t docu[] =
  iU("Send/Receive data in homogeneous structure/table             ")     NL_AR
  iU("  NOT available with 16-bit RFC library on Windows           ")     NL_AR
  iU("")                                                                  NL_AR
  iU("IMPORTING")                                                         NL_AR
  iU("  IMPORTSTRUCT    Structure RFCTEST(48)")                           NL_AR
  iU("EXPORTING")                                                         NL_AR
  iU("  RESPTEXT        C(255)               ")                           NL_AR
  iU("  ECHOSTRUCT      Structure RFCTEST(48)")                           NL_AR
  iU("TABLES")                                                            NL_AR
  iU("  RFCTABLE        Structure RFCTEST(48)")                           NL_AR
   ;
  return docu;
}


/*====================================================================*/
/*                                                                    */
/* Documentation for function STFC_RAISE_ERROR                        */
/*                                                                    */
/*====================================================================*/
static rfc_char_t * rfc_raise_error_docu( void )
{
   static rfc_char_t docu[] =
 iU("!!! FOR INTERNAL USE ONLY !!!");

   return docu;
} /* rfc_raise_error_docu */



/*====================================================================*/
/*                                                                    */
/* Documentation for function STFC_STRING                             */
/*                                                                    */
/*====================================================================*/
static rfc_char_t *string_test_docu(void)
{
 static rfc_char_t docu[] =
 iU("String handling example")                                           NL_AR
 iU("")                                                                  NL_AR
 iU("IMPORTING")                                                         NL_AR
 iU("    QUESTION	string")                                             NL_AR
 iU("EXPORTING")                                                         NL_AR
 iU("    MYANSWER  string")                                              NL_AR
 ;

 return docu;
}


/*--------------------------------------------------------------------*/
/* set up RFC parameters                                              */
/*--------------------------------------------------------------------*/
void rfc_param(RFC_PARAMETER *rfc_param,
               rfc_char_t    *abap_field_name,
               unsigned       data_type,
               void          *data_addr,
               unsigned       data_len)
{
  RFC_PARAMETER *p = rfc_param;

  p->name = abap_field_name;
  p->nlen = abap_field_name == NULL ? 0 : strlenU(abap_field_name);
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
void DLL_CALL_BACK_FUNCTION rfc_error(rfc_char_t *operation )
{
  RFC_ERROR_INFO_EX  error_info;

  fprintfU(stderr, cU("\n<==  RfcLastErrorEx\n"));

  RfcLastErrorEx(&error_info);

  fprintfU(stderr, cU("\nRFC Call/Exception: %s\n"), operation);
  fprintfU(stderr, cU("Group       Error group %d\n"), error_info.group);
  fprintfU(stderr, cU("Key         %s\n"), error_info.key);
  fprintfU(stderr, cU("Message     %s\n"), error_info.message);

  exit(1);
}


/*--------------------------------------------------------------------*/
/* Issue RfcAbort with Abort Text because of an Application Error     */
/*--------------------------------------------------------------------*/
void function_abort(rfc_char_t *atext)
{
  fprintfU(stderr, cU("\n<==  RfcAbort       %s"), atext);

  RfcAbort(rfc_handle, atext);
  exit(1);
}


/*--------------------------------------------------------------------*/
/* Fill internal table as required                                    */
/*--------------------------------------------------------------------*/
int fill_table(ITAB_H itab_h,
               int    leng)
{
  int 	      position,
              linenr;
  rfc_char_t  etext[1001];


  if (leng == 0)
    return 0;


  lineleng = ItLeng(itab_h)/sizeofR(SAP_UC);
  ulen     = ItLeng(itab_h)/sizeofR(SAP_UC);

  memsetU (etext, BLANK, 1000);

  for (position=40; position<lineleng; position++)
    sprintfU (etext+position, cU("%01d"), position % 10);

  sprintfU (etext, cU("Line Length: %04d   Line No.: "), lineleng);

  for (linenr=1; linenr<=leng; linenr++)
  {
    sprintfU(etext+30, cU("%05d"), linenr);
    etext[35] = cU(' ');

    ptr = (rfc_char_t *) ItAppLine(itab_h);
    if (ptr == NULL) 
     break;
    memcpyU(ptr, etext, ulen);
  }

  if (linenr <= leng)
    return 1;
  else
    return 0;
}


/*--------------------------------------------------------------------*/
/* Check ETABnnnn                                                     */
/*--------------------------------------------------------------------*/
int check_table(ITAB_H itab_h,
                rfc_char_t   *itabname,
                int    recvleng,
                rfc_char_t   *checktab,
                rfc_char_t   *rtext,
                rfc_char_t   *retcode)
{
  int   position;

  rfc_char_t  value[2],
              tbuff[1001],
              etext[73]= iU(" "),
              errline[40]= iU("ERROR:  #Lines = nnnnn  Expected: nnnnn"),
              errcont[40]= iU("ERROR at line nnnnn position nnnn:  n/n"),
              mtext[73]  = iU("Import table (length nnnn):  #Lines = nnnnn: OK,  Contents: OK");


  memsetU (etext, cU(' '), 72);

  itableng = ItFill(itab_h);
  lineleng = ItLeng(itab_h)/sizeofR(SAP_UC);
  ulen     = ItLeng(itab_h)/sizeofR(SAP_UC);

  strcpyU(etext, mtext);
  etext[strlenU(etext)] = cU(' ');

  if (itabname[0] == cU('I'))
    memcpyU(etext, cU("Ex"), 2);

  memcpyU(etext+21, itabname+4, 4);

  if (itableng != recvleng)
  {
    memcpyU(etext+28, errline, 39);
    sprintfU(etext+45, cU("%05d"), itableng);
    etext[50] = cU(' ');
    sprintfU(etext+62, cU("%05d"), recvleng);
    etext[67] = cU(' ');
    retcode[0] = cU('E');
  }
  else
  {
    sprintfU(etext+38, cU("%05d"), itableng);
    etext[43] = cU(':');
    retcode[0] = cU('O');

    if (checktab[0] != cU('Y'))
      memcpyU(etext+60, cU("NO CHECK"), 8);
    else
    {
      for (j = 1; ; j++)
      {
        ptr = (rfc_char_t *) ItGetLine(itab_h, j);
        if (ptr == NULL) 
         break;

        memcpyU(tbuff, ptr, ulen);
        tbuff[lineleng] = cU('\0');

        for (position=40; position<lineleng; position++)
        {
          sprintfU (value, cU("%01d"), position % 10);
          if (value[0] != tbuff[position])
          {
            retcode[0] = cU('E');
            break;
          }
          position = position + 1;
        }
        if (retcode[0] == cU('E'))
        {
          memcpyU(etext+28, errcont, 39);
          sprintfU(etext+42, cU("%05d"), j);
          etext[47] = cU(' ');
          sprintfU(etext+57, cU("%05d"), position);
          etext[62] = cU(' ');
          etext[64] = tbuff[position];
          etext[66] = value[0];
          break;
        }
      }
    }
  }

  strcpyU(rtext, etext);
  return 0;
}


/*--------------------------------------------------------------------*/
/* SNC check: - Working with SNC                                      */
/*            - If yes, check SNC name or SNC ACL key                 */
/*                                                                    */
/* SNC check should be done                                           */
/*   - after an successful RfcGetName                                 */
/*   - after an successful RfcListen for waiting for RFC request      */
/*   - after an successful RfcWaitForRequest                          */
/*   - if working with RfcDispatch, in each RFC function and before   */
/*     RfcGetData                                                     */
/*--------------------------------------------------------------------*/
void SNC_check(void)
{
  if (SNC_checked)
    return;

  SNC_checked = 1;

  fprintfU(stderr, cU("\n<==  RfcSncMode               rfc_rc = "));
  rfc_rc = RfcSncMode(rfc_handle, &snc_mode);
  fprintfU(stderr, cU("%d"), rfc_rc);

  if (rfc_rc != RFC_OK)
    rfc_error(cU("RfcSncMode"));

  if (snc_mode == RFC_SNC_MODE_OFF)
  {
    fprintfU(stderr, cU("       RFC_SNC_MODE_OFF"));
    return;
  }

  fprintfU(stderr, cU("       RFC_SNC_MODE_ON"));

  fprintfU(stderr, cU("\n<==  RfcSncPartnerName        rfc_rc = "));
  rfc_rc = RfcSncPartnerName ( rfc_handle,
                               snc_partner_name,
                               SNC_NAME_LN);
  fprintfU(stderr, cU("%d"), rfc_rc);

  if (rfc_rc != RFC_OK)
    rfc_error(cU("RfcSncPartnerName"));

  fprintfU (stderr, cU("\nSNC-PartnerName: Length = %u"), strlenU(snc_partner_name));
  fprintfU (stderr, cU("\n                 Value  = %s"), snc_partner_name);
  fprintfU (stderr, cU("\n<==  RfcSncPartnerAclKey      rfc_rc = "));

  memsetR (snc_partner_aclkey, cR(' '), SNC_NAME_LN);

  rfc_rc = RfcSncPartnerAclKey (rfc_handle,
                                snc_partner_aclkey,
                                SNC_NAME_LN,
                                &aclkey_len);

  fprintfU(stderr, cU("%d"), rfc_rc);

  if (rfc_rc != RFC_OK)
    rfc_error(cU("RfcSncPartnerAclKey"));

  fprintfU(stderr, cU("\nSNC-PartnerAclKey: Length = %d"), aclkey_len);

  fprintfU(stderr, cU("\n                   Value  = "));

  for (i=0; i<aclkey_len; i++)
    fprintfU (stderr, cU("%02X"), (unsigned) snc_partner_aclkey[i]);

  /* Sample coding for check and refuse */
  if (strcmpU (snc_partner_name, cU("not_authorized_snc_partner_name")) == 0)
  {
    sprintfU (abort_text,
              cU("SNC partner name '%s' is not authorized for calling this program"),
              snc_partner_name);
    function_abort(abort_text);
  }

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
  printfU (cU("  srfcserv [options]")                                      NL );
  printfU (cU(" ")                                                         NL );
  printfU (cU("  with")                                                    NL );
  printfU (cU("  options = -D<destination with type 'R' in saprfc.ini>")   NL );
  printfU (cU("          = -t             RFC-Trace on")                   NL );
  printfU (cU("  or")                                                      NL );
  printfU (cU("  options = -a<program ID> e.g.  <own host name>.srfcserv") NL );
  printfU (cU("          = -g<SAP gateway host name>        e.g.  hs0311") NL );
  printfU (cU("          = -x<SAP gateway service>          e.g. sapgw53") NL );
  printfU (cU("          = -t             RFC-Trace on")                   NL );
  printfU (cU("          = -L<SNC library, optional>")                     NL );
  printfU (cU("          = -S<SNC myname, optional>")                      NL );
  printfU (cU("          = -Q<SNC quality of protection, optional>")       NL );
  printfU (cU(" ")                                                         NL );
  printfU (cU(" Option L, S and Q can be set if working with SNC" )        NL );
  printfU (cU(" (Secure Network Communication).")                          NL );
  printfU (cU(" ")                                                         NL );
  return;
}

