/* static rfc_char_t sccsid[] = "@(#) $Id: //bas/640_REL/src/krn/rfc/srfctest.c#4 $ SAP"; */

/*====================================================================*/
/*                                                                    */
/*     PROGRAM     :   srfctest.c (running on Windows, Windows_NT,    */
/*                                 Windows_95, OS/2 and R/3-Platforms)*/
/*                                                                    */
/*                                                                    */
/*     DESCRIPTION :   Test program for Remote Function Calls         */
/*                                                                    */
/*                     - GET SYSTEM INFO:                             */
/*                        Get system info of a RFC server             */
/*                                                                    */
/*                     - CONNECTION TEST:                             */
/*                        Send a request and wait for echo and info   */
/*                                                                    */
/*                     - PERFORMANCE TEST:                            */
/*                        Send and receive internal tables            */
/*                        Number of entries of internal tables will   */
/*                        be required via dialog.                     */
/*                                                                    */
/*                     - RFC WITH SAPGUI:                             */
/*                        + Execute any transaction in R/3.           */
/*                        + Execute any function module in R/3.       */
/*                          Restriction: No import/export parameters  */
/*                                       are supported in this progr. */
/*                                                                    */
/*                     - TEST WITH INHOMOGENEOUS STRUCTURE/TABLE      */
/*                        + Only with R/3 >= 3.1 and NOT with         */
/*                          16-bit Windows                            */
/*                                                                    */
/*                     - CONVERSION SNC Name <==> SNC ACL Key         */
/*                                                                    */
/*                     - CALLBACK TEST:                               */
/*                        Send a request and wait for echo and info   */
/*                        and callback during the execution in server */
/*                                                                    */
/*                     - Check/Cancel registered RFC server           */
/*                                                                    */
/*                     - CONVERSION BCD Numbers <==> rfc_char_tacter string */
/*                                                                    */
/*                     - Start and Connect to a registered server     */
/*                        during the execution of a FM in R/3         */
/*                        + Only for R/3 >= 4.0C and NOT with         */
/*                          16-bit Windows                            */
/*                                                                    */
/*                     - Test the error handling                      */
/*                                                                    */
/*                     The RFC-Server-System can be R/2 or R/3 for    */
/*                     CONNECTION TEST or PERFORMANCE TEST.           */
/*                                                                    */
/*                     RFC WITH SAPGUI is only for R/3 >= 3.0 and not */
/*                     available with 16-bit RFC Library, OS/400      */
/*                     and OS/390                                     */
/*                                                                    */
/*                                                                    */
/*     SAP AG Walldorf                                                */
/*     Systeme, Anwendungen und Produkte in der Datenverarbeitung     */
/*                                                                    */
/*     Copyright (C) SAP AG 1994                                      */
/*                                                                    */
/*====================================================================*/

/*--------------------------------------------------------------------*/
/* Setup includes                                                     */
/*--------------------------------------------------------------------*/
#if defined(SAPonOS400)
#include "saptype.h"
#endif

#include "srfctest.h"


/*--------------------------------------------------------------------*/
/* main                                                               */
/*--------------------------------------------------------------------*/

int mainU (int argc, rfc_char_t **argv)
{
  setbuf(stdout, NULL);

  strcpyU (work_file, std_data_file);

  if (argc == 1)
    syntax_OK = 1;
  else
    syntax_OK = 0;

  for (i = 1; i < argc; i++)
  {
    if (memcmpU (argv[i], cU("file="), 5) == 0)
    {
      syntax_OK = 1;
      znr = strlenU (argv[i]) - 5;
      if (znr)
      {
        if (znr > 128)
          argv[i][5+128] = 0;
        strcpyU (work_file, &argv[i][5]);
      }
    }
  }

  if (syntax_OK == 0)
  {
    printfU (cU("\nSyntax:   srfctest [options]\n"));
    printfU (cU("\n          options:  file=<test data file>\n\n"));
    exit(1);
  }

  for (;;)
  {
    printfU (cU("\n\n"));
    printfU (cU("               **********************************************\n"));
    printfU (cU("               *                                            *\n"));
    printfU (cU("               *              SAP RFC-API TEST              *\n"));
    printfU (cU("               *                                            *\n"));
    printfU (cU("               *                    OS/2,                   *\n"));
    printfU (cU("               *       Windows, Windows_NT, Windows_95      *\n"));
    printfU (cU("               *                R/3-Platforms               *\n"));
    printfU (cU("               *                                            *\n"));
    printfU (cU("               *                                            *\n"));
    printfU (cU("               **********************************************\n\n\n"));

    /*----------------------------------------------------------------*/
    /*  Keyboard Input:  - ACTION                                     */
    /*----------------------------------------------------------------*/

    printfU (cU("\n                            Possible RFC-Tests\n"));
    printfU (cU("\n                            1. RFC_SYSTEM_INFO"));
    printfU (cU("\n                            2. CONNECTION"));
    printfU (cU("\n                            3. PERFORMANCE"));
    printfU (cU("\n                            4. RFC with SAPGUI"));
    printfU (cU("\n                            5. INHOMOGENEOUS STRUCTURE/TABLE"));
    printfU (cU("\n                            6. CONVERSION SNC Name <==> SNC ACL Key"));
    printfU (cU("\n                            7. RFC with CALLBACK"));
    printfU (cU("\n                            8. Check Registered Server"));
    printfU (cU("\n                            9. Cancel Registered Server"));
    printfU (cU("\n                           10. CONVERSION BCD Numbers <==> rfc_char_t string"));
    printfU (cU("\n                           11. Start/Connect to a reg. server from within R/3"));
    printfU (cU("\n                           12. Cancel running ABAP-Function"));
    printfU (cU("\n                           13. RFC with changing parameters"));
    printfU (cU("\n                           14. Calls RFC_RAISE_ERROR (for internal use only!)"));
    printfU (cU("\n                           99. Exit\n\n"));
    
    
    do
    {
        printfU (cU("                            Your Choice.......: "));
      
        fflush(stdout);
      
        getsU (ips);
      
        action = atoiU (ips);
      
        if (action == 99)
	     break;
    } while ((action < 1) || (action > 14));

#ifdef SAPonOS400
    if (action == 4)
    {
        printfU (cU("\n\nThis feature is not available on AS/400 !!!!!\n\n"));
      
        exit(1);
    }
#endif

#ifdef SAPonOS390
    if (action == 4)
    {
        printfU (cU("\n\nThis function is not available on OS/390 !\n\n"));
      
        exit(1);
    }
#endif

    if (action == 99)
      break;

    
    switch(action)
    {
        case 1:
            printfU (cU("\n\n\n"));
            printfU (cU("               **********************************************\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *          RFC-API:  RFC SYSTEM INFO         *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *       Get system info of an RFC server     *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               **********************************************\n"));
            srfcinfo();
            break;

        
        case 2:
            printfU (cU("\n\n\n"));
            printfU (cU("               **********************************************\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *          RFC-API:  CONNECTION TEST         *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *     Send message and wait for ECHO/INFO    *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               **********************************************\n"));
            srfcconn();
            break;

        
        case 3:
            printfU (cU("\n\n\n"));
            printfU (cU("               **********************************************\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *         RFC-API:  PERFORMANCE TEST         *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *        Send/Receive internal tables        *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               **********************************************\n"));
            srfcload();
            break;

        
        case 4:
            printfU (cU("\n\n\n"));
            printfU (cU("               **********************************************\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *         RFC-API:  RFC with SAPGUI          *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *    Start transaction or function module    *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               **********************************************\n"));
            srfcdiag();
            break;


        case 5:
            printfU (cU("\n\n\n"));
            printfU (cU("               **********************************************\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *   RFC-API:  INHOMOGENEOUS STRUCTURE/TABLE  *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               * Send/Receive inhomogeneous structure/table *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               **********************************************\n"));
            srfctruc();
            break;


        case 6:
            printfU (cU("\n\n\n"));
            printfU (cU("               **********************************************\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *    CONVERSION SNC Name <==> SNC ACL Key    *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *   The required SNC libary name can be set  *\n"));
            printfU (cU("               *   via the environment variable 'SNC_LIB'   *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               **********************************************\n"));
            ssncconv();
            break;


        case 7:
            printfU (cU("\n\n\n"));
            printfU (cU("               **********************************************\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *      RFC-API:  RFC WITH CALLBACK TEST      *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               **********************************************\n"));
            srfcback();
            break;


        case 8:
            printfU (cU("\n\n\n"));
            printfU (cU("               **********************************************\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *      RFC-API:  CHECK REGISTERED SERVER     *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               **********************************************\n"));
            srfcchek ();
            break;


        case 9:
            printfU (cU("\n\n\n"));
            printfU (cU("               **********************************************\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *     RFC-API:  CANCEL REGISTERED SERVER     *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               **********************************************\n"));
            srfccanc ();
            break;


        case 10:
            printfU (cU("\n\n\n"));
            printfU (cU("               **********************************************\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *  CONVERSION BCD numbers <==> rfc_char_t string   *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               **********************************************\n"));
            srfcconv ();
            break;

        case 11:
            printfU (cU("\n\n\n"));
            printfU (cU("               **********************************************\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *   RFC-API:  START/CONNECT TO REG. SERVER   *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *  Start and connect to a registered server  *\n"));
            printfU (cU("               *  during the execution of a function module *\n"));
            printfU (cU("               *  from within an R/3 system                 *\n"));
            printfU (cU("               *  This registered server will be started    *\n"));
            printfU (cU("               *  automatically by the RFC library using in *\n"));
            printfU (cU("               *  the running RFC client.                   *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               **********************************************\n"));
            srfcsreg();
            break;

        case 12:
            printfU (cU("\n\n\n"));
            printfU (cU("               **********************************************\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *  RFC-API:  CANCEL RUNNING ABAP FUNCTION    *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               **********************************************\n"));
            srfccancel();
            break;

        case 13:
            printfU (cU("\n\n\n"));
            printfU (cU("               **********************************************\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *  RFC-API:  Changing parameters in use      *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               **********************************************\n"));
            srfcchn ();
            break;

        case 14:
            printfU (cU("\n\n\n"));
            printfU (cU("               **********************************************\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               *  Calls function module RFC_RAISE_ERROR     *\n"));
            printfU (cU("               *                                            *\n"));
            printfU (cU("               **********************************************\n"));
            srfcraiseerror ();
            break;
    }

    printfU (cU("\n\n"));

    
    do
    {
        printfU (cU("                    Continue (y/n)..................:  "));
      
        fflush (stdout);
      
        getsU (ips);
      
        ips[0] = toupperU (ips[0]);
    } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));

    
    if (ips[0] == cU('N'))
     break;
  }

  
  printfU (cU("\n"));
  
  
  return 0;
}


/*====================================================================*/
/*                                                                    */
/* Check for start SAPGUI if needed                                   */
/*                                                                    */
/*====================================================================*/
void check_gui_for_start(void)
{
  int                   rc,
                        gui_option;
  rfc_char_t            register_info[513];
  RFC_INT               remote_gui = 0,
                        debug      = 0,
                        sapgui     = 0;
  RFC_GUI_START_PARAM   start_param;
  RFC_GUI_RETURN_PARAM  return_param;
  RFC_GUI_ERROR         error_message;


  /* Check SAPGUI required */
  if ((action != 4) && (abap_debug[0] != cU('Y')))
    return;

  /* Check SAPGUI is already running */
  rc = RfcGuiCheck(rfc_handle);

  if (rc)
    return;

  memsetR (register_info, 0, sizeofR (register_info));

  printfU (cU("\n\n          !!! SAPGUI IS NOT ALREADY RUNNING !!!\n\n"));
  printfU (cU("\n            On which computer do you want to run SAPGUI ?\n"));
  printfU (cU("\n              1. On this computer"));
  printfU (cU("\n              2. On a remote computer using a registered RFC server"));
  printfU (cU("\n              3. On a remote computer using RfcGuiStart\n\n"));

  do
  {
    printfU (cU("              Your Choice.......: "));
    fflush(stdout);
    getsU (ips);
    gui_option = atoiU (ips);
  } while ((gui_option < 1) || (gui_option > 3));

  switch(gui_option)
  {
    case 1:
      remote_gui = 0;
      break;

    case 2:
      remote_gui = 1;
      strcpyU (gwhost, rfc_attributes.partner_host);
      sprintfU (gwserv, cU("sapgw%s"), rfc_attributes.systnr);
      get_parameter_for_reg_server();
      sprintfU (register_info, cU("GWHOST=%s GWSERV=%s PROGID=%s"),
                gwhost, gwserv, tpid);
      printfU (cU("\n     Please start 'srfcserv' on the remote computer as follows:"));
      printfU (cU("\n\n        srfcserv -g%s -x%s -a%s"), gwhost, gwserv, tpid);
      printfU (cU("\n\n     Hit Return Key To Continue"));
      getcharU();
      break;

    case 3:
      remote_gui = 1;
      break;
  }

  if (abap_debug[0] == cU('Y'))
    debug = 1;
  else
    debug = 0;

  if (use_sapgui[0] == cU('Y'))
  {
    if (invisible[0] == cU('Y'))
      sapgui = 2;
    else
      sapgui = 1;
  }
  else
    sapgui = 0;

  printfU (cU("\n<==  RfcGuiPrepare            rc = "));
  rfc_rc = RfcGuiPrepare(rfc_handle,
                         remote_gui,
                         debug,
                         sapgui,
                         register_info,
                         start_param,
                         &error_info);

  printfU (cU("%d\n"), rfc_rc);

  if (rfc_rc != RFC_OK)
  {
    printfU (cU("\nGroup       Error group %d\n"), error_info.group);
    printfU (cU("Key         %s\n"), error_info.key);
    printfU (cU("Message     %s\n\n"), error_info.message);
    exit(1);
  }

  if ((remote_gui) && (strlenU (register_info) == 0))
  {
    printfU (cU("\n     *************************************************************"));
    printfU (cU("\n     *"));
    printfU (cU("\n     *   SIMULATION: This part should run on the remote computer"));
    printfU (cU("\n     *"));
    printfU (cU("\n     *   Start parameters for SAPGUI returned by RfcGuiPrepare:"));
    printfU (cU("\n     *      '%s'"), start_param);
    printfU (cU("\n     *"));
    printfU (cU("\n     *   <==  RfcGuiStart"));
    RfcGuiStart(start_param,
                error_message,
                return_param);

    printfU (cU("\n     *        Error Message returned by RfcGuiStart:"));
    printfU (cU("\n     *           '%s'"), error_message);
    printfU (cU("\n     *        Return information returned by RfcGuiStart"));
    printfU (cU("\n     *           '%s'"), return_param);
    printfU (cU("\n     *"));
    printfU (cU("\n     *************************************************************"));

    printfU (cU("\n<==  RfcGuiReturn             rc = "));
    rfc_rc = RfcGuiReturn(rfc_handle,
                          error_message,
                          return_param,
                          &error_info);

    printfU (cU("%d\n"), rfc_rc);

    if (rfc_rc != RFC_OK)
    {
      printfU (cU("\nGroup       Error group %d\n"), error_info.group);
      printfU (cU("Key         %s\n"), error_info.key);
      printfU (cU("Message     %s\n\n"), error_info.message);
      exit(1);
    }
  }

  return;
}


/*====================================================================*/
/*                                                                    */
/* Call Function RFC_SYSTEM_INFO                                      */
/*                                                                    */
/* Get system info of a RFC server                                    */
/*                                                                    */
/*====================================================================*/
int srfcinfo()
{
  get_parameter_for_conn_test();

  RFC_connect(cU("srfcinfo"));

  check_gui_for_start();

  for (nrcount=0; nrcount<nrcall; nrcount++)
  {
    /*----------------------------------------------------------------*/
    /* Call RFC_SYSTEM_INFO                                           */
    /*----------------------------------------------------------------*/

    function = name_systeminfo_test;

    memsetR (&exporting[0], 0, sizeofR (exporting));
    memsetR (&importing[0], 0, sizeofR (importing));

    rfc_param (&importing[0], cU("RFCSI_EXPORT"), TYPC, &rfcsi, sizeofR(rfcsi));

    /* no table ---> terminate array */
    tables[0].name = NULL;

    printfU (cU("\n<==  RfcCallReceive (%s)     rfc_rc = "), function);

#if defined(SAPonUNIX)
    gettimeofday(&scallmsec, &tzp);
#elif defined(SAPonOS400)
    scallmsec = time(NULL);
#else
    ftime(&scallmsec);
#endif

    rfc_rc = RfcCallReceive (rfc_handle,
                             function,
                             exporting,
                             importing,
                             tables,
                             &exception);
    printfU (cU("%d\n"), rfc_rc);

    /* check return code */
    switch(rfc_rc)
    {
      case RFC_OK:
        break;
      case RFC_EXCEPTION:
      case RFC_SYS_EXCEPTION:
        rfc_error(exception);
        break;
      default:
        rfc_error( cU("RfcCallReceive"));
        break;
    }

#if defined(SAPonUNIX)
    gettimeofday(&ecallmsec, &tzp);
#elif defined(SAPonOS400)
    ecallmsec = time(NULL);
#else
    ftime(&ecallmsec);
#endif

    /* Compute required times */
#if defined(SAPonUNIX)
    msecopen = diffmsec(&eopenmsec, &sopenmsec);
    mseccall = diffmsec(&ecallmsec, &scallmsec);
#elif defined(SAPonOS400)
    msecopen = diffmsec_AS400(eopenmsec, sopenmsec);
    mseccall = diffmsec_AS400(ecallmsec, scallmsec);
#else
    msecopen = diffmsec_NON_UNIX(&eopenmsec, &sopenmsec);
    mseccall = diffmsec_NON_UNIX(&ecallmsec, &scallmsec);
#endif

    printfU (cU("\n\n"));
    printfU (cU("*******************************************************************************\n"));
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*  Required time for connect and SAP logon                   %10u msec  *\n"), msecopen);
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*  Required time for RFC without connect and logon           %10u msec  *\n"), mseccall);
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*******************************************************************************\n"));
    display_rfcsi(&rfcsi);

    if (nrcount < nrcall-1)
    {
      if (use_close[0] == cU('Y'))
      {
        printfU (cU("\n<==  RfcClose\n"));
        RfcClose(rfc_handle);
      }
      printfU (cU("\n\nHit return key for next RFC call\n"));
      fflush(stdout);
      getcharU();

      /* If explizit or RFC server is NON-SAP and use the    */
      /* register mode:                                      */
      /*    ===> Close and Open the RFC connection           */
      if (use_close[0] == cU('Y'))
      {
        /* LOAD BALANCING only for the first time */
        if ((sapsys[0] == RFC_SERVER_R3) && (load_balancing[0] == cU('Y')))
        {
          saprfc_ini[0]     = cU('N');
          load_balancing[0] = cU('N');
          strcpyU (ashost, rfc_attributes.partner_host);
          strcpyU (sysnr,  rfc_attributes.systnr);
        }

        RFC_connect(cU("srfcinfo_N"));
      }
    }
  }

  /* Get specific info of this RFC connection */
  get_attributes(rfc_handle);

  /*----------------------------------------------------------------*/
  /* Close RFC connection                                           */
  /*----------------------------------------------------------------*/
  printfU (cU("\n<==  RfcClose\n"));
  RfcClose(rfc_handle);

  return 0;
}


/*====================================================================*/
/*                                                                    */
/* Call Function STFC_CONNECTION                                      */
/*                                                                    */
/* Send message and wait for echo and info                            */
/*                                                                    */
/*====================================================================*/
int srfcconn()
{
  get_parameter_for_conn_test();

  RFC_connect(cU("srfcconn"));

  check_gui_for_start();

  for (nrcount=0; nrcount<nrcall; nrcount++)
  {
    /*----------------------------------------------------------------*/
    /* Call Connection Test                                           */
    /*----------------------------------------------------------------*/

    function = name_connection_test;

    memsetR (&exporting[0], 0, sizeofR (exporting));
    memsetR (&importing[0], 0, sizeofR (importing));

    memsetU (requtext, cU(' '), 255);
    strcpyU (requtext, cU("abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890"));
    requtext[strlenU (requtext)] = cU(' ');

    rfc_param (&exporting[0], cU("REQUTEXT"), TYPC, requtext, 255*sizeofR(SAP_UC));
    rfc_param (&importing[0], cU("ECHOTEXT"), TYPC, echotext, 255*sizeofR(SAP_UC));
    rfc_param (&importing[1], cU("RESPTEXT"), TYPC, resptext, 255*sizeofR(SAP_UC));

    /* no table ---> terminate array */
    tables[0].name = NULL;

    printfU (cU("\n<==  RfcCallReceive (%s)     rfc_rc = "), function);

#if defined(SAPonUNIX)
    gettimeofday(&scallmsec, &tzp);
#elif defined(SAPonOS400)
    scallmsec = time(NULL);
#else
    ftime(&scallmsec);
#endif

    rfc_rc = RfcCallReceive(rfc_handle,
	  		    function,
			    exporting,
			    importing,
			    tables,
			    &exception);
    printfU (cU("%d\n"), rfc_rc);

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

#if defined(SAPonUNIX)
    gettimeofday(&ecallmsec, &tzp);
#elif defined(SAPonOS400)
    ecallmsec = time(NULL);
#else
    ftime(&ecallmsec);
#endif

    display(cU("Export Parameter"), exporting);

    display(cU("Import Parameter"), importing);

    /* Compute required times */
#if defined(SAPonUNIX)
    msecopen = diffmsec(&eopenmsec, &sopenmsec);
    mseccall = diffmsec(&ecallmsec, &scallmsec);
#elif defined(SAPonOS400)
    msecopen = diffmsec_AS400(eopenmsec, sopenmsec);
    mseccall = diffmsec_AS400(ecallmsec, scallmsec);
#else
    msecopen = diffmsec_NON_UNIX(&eopenmsec, &sopenmsec);
    mseccall = diffmsec_NON_UNIX(&ecallmsec, &scallmsec);
#endif

    printfU (cU("\n\n"));
    printfU (cU("*******************************************************************************\n"));
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*  Required time for connect and SAP logon                   %10u msec  *\n"), msecopen);
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*  Required time for RFC without connect and logon           %10u msec  *\n"), mseccall);
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*******************************************************************************\n"));

    if (nrcount < nrcall-1)
    {
      if (use_close[0] == cU('Y'))
      {
        printfU (cU("\n<==  RfcClose\n"));
        RfcClose(rfc_handle);
      }
      printfU (cU("\n\nHit return key for next RFC call\n"));
      fflush(stdout);
      getcharU ();

      /* If explizit or RFC server is NON-SAP and use the    */
      /* register mode:                                      */
      /*    ===> Close and Open the RFC connection           */
      if (use_close[0] == cU('Y'))
      {
	/* LOAD BALANCING only for the first time */
	if ((sapsys[0] == RFC_SERVER_R3) && (load_balancing[0] == cU('Y')))
        {
          saprfc_ini[0]     = cU('N');
          load_balancing[0] = cU('N');
          strcpyU (ashost, rfc_attributes.partner_host);
          strcpyU (sysnr,  rfc_attributes.systnr);
        }

        RFC_connect(cU("srfcconn_N"));
      }
    }
  }

  /* Get specific info of this RFC connection if server is External */
  if (sapsys[0] == RFC_SERVER_EXT)
    get_attributes(rfc_handle);

  /*----------------------------------------------------------------*/
  /* Close RFC connection                                           */
  /*----------------------------------------------------------------*/
  printfU (cU("\n<==  RfcClose\n"));
  RfcClose(rfc_handle);

  return 0;
}


/*====================================================================*/
/*                                                                    */
/* Call Function STFC_PERFORMANCE                                     */
/*                                                                    */
/* Send/receive internal tables                                       */
/* Number of entries will be required via dialog                      */
/*                                                                    */
/*====================================================================*/
int srfcload()
{
  get_parameter_for_load_test();

  RFC_connect(cU("srfcload"));

  check_gui_for_start();

  /* Prepare internal table ITAB0332 */
  tables[0].name     = cU("ITAB0332");
  tables[0].nlen     = 8;
  tables[0].type     = TYPC;
  tables[0].leng     = ITAB0332_ENTRY_SIZE;
  tables[0].itmode   = RFC_ITMODE_BYREFERENCE;

  printfU (cU("\n<==  ItCreate (ITAB0332)\n"));
  tables[0].ithandle = ItCreate (cU("ITAB0332"),
                                 ITAB0332_ENTRY_SIZE,
                                 0,
                                 0);

  if (tables[0].ithandle == NULL )
    rfc_error (cU("ItCreate ITAB0332"));

  /* Prepare internal table ITAB1000 */
  tables[1].name     = cU("ITAB1000");
  tables[1].nlen     = 8;
  tables[1].type     = TYPC;
  tables[1].leng     = ITAB1000_ENTRY_SIZE;
  tables[1].itmode   = RFC_ITMODE_BYREFERENCE;

  printfU (cU("\n<==  ItCreate (ITAB1000)\n"));
  tables[1].ithandle = ItCreate (cU("ITAB1000"),
                                 ITAB1000_ENTRY_SIZE,
                                 0,
                                 0);

  if (tables[1].ithandle == NULL )
    rfc_error (cU("ItCreate ITAB1000"));

  /* Prepare internal table ETAB0332 */
  tables[2].name     = cU("ETAB0332");
  tables[2].nlen     = 8;
  tables[2].type     = TYPC;
  tables[2].leng     = ETAB0332_ENTRY_SIZE;
  tables[2].itmode   = RFC_ITMODE_BYREFERENCE;

  printfU (cU("\n<==  ItCreate (ETAB0332)\n"));
  tables[2].ithandle = ItCreate (cU("ETAB0332"),
                                 ETAB0332_ENTRY_SIZE,
                                 0,
                                 0);

  if (tables[2].ithandle == NULL )
    rfc_error (cU("ItCreate ETAB0332"));

  /* Prepare internal table ETAB1000 */
  tables[3].name     = cU("ETAB1000");
  tables[3].nlen     = 8;
  tables[3].type     = TYPC;
  tables[3].leng     = ETAB1000_ENTRY_SIZE;
  tables[3].itmode   = RFC_ITMODE_BYREFERENCE;

  printfU (cU("\n<==  ItCreate (ETAB1000)\n"));
  tables[3].ithandle = ItCreate (cU("ETAB1000"),
                                 ETAB1000_ENTRY_SIZE,
                                 0,
                                 0);

  if (tables[3].ithandle == NULL )
    rfc_error (cU("ItCreate ETAB1000"));

  for (nrcount=0; nrcount<nrcall; nrcount++)
  {
    /*----------------------------------------------------------------*/
    /* Call Performance Test                                          */
    /*----------------------------------------------------------------*/

    function = name_performance_test;

    memsetR (&exporting[0], 0, sizeofR (exporting));
    memsetR (&importing[0], 0, sizeofR (importing));

    for (i=0; i<255; i++)
      sprintfU (&requtext[i], cU("%01d"), (i+1) % 10);

    rfc_param(&exporting[0], cU("CHECKTAB"), TYPC, checktab,       1*sizeofR(SAP_UC));
    rfc_param(&exporting[1], cU("LGIT0332"), TYPC, lgit0332_field, 5*sizeofR(SAP_UC));
    rfc_param(&exporting[2], cU("LGIT1000"), TYPC, lgit1000_field, 5*sizeofR(SAP_UC));
    rfc_param(&exporting[3], cU("LGET0332"), TYPC, lget0332_field, 5*sizeofR(SAP_UC));
    rfc_param(&exporting[4], cU("LGET1000"), TYPC, lget1000_field, 5*sizeofR(SAP_UC));

    rfc_param(&importing[0], cU("EXITCODE"), TYPC, exitcode, 1*sizeofR(SAP_UC));
    rfc_param(&importing[1], cU("TEXT0332"), TYPC, text0332, 72*sizeofR(SAP_UC));
    rfc_param(&importing[2], cU("TEXT1000"), TYPC, text1000, 72*sizeofR(SAP_UC));

    /* terminate array */
    tables[4].name = NULL;

#if defined(SAPonUNIX)
    gettimeofday(&sfillmsec, &tzp);
#elif defined(SAPonOS400)
    sfillmsec = time(NULL);
#else
    ftime(&sfillmsec);
#endif

    /* Fill table ITAB0332 as required */
    if (fill_table(tables[0].ithandle, lgit0332))
      printfU (cU("\nERROR: Fill table itab0332\n"));

    /* Fill table ITAB1000 as required */
    if (fill_table(tables[1].ithandle, lgit1000))
      printfU (cU("\nERROR: Fill table itab1000\n"));

#if defined(SAPonUNIX)
    gettimeofday(&efillmsec, &tzp);
#elif defined(SAPonOS400)
    efillmsec = time(NULL);
#else
    ftime(&efillmsec);
#endif

    printfU (cU("\n<==  RfcCallReceive (%s)     rfc_rc = "), function);

#if defined(SAPonUNIX)
    gettimeofday(&scallmsec, &tzp);
#elif defined(SAPonOS400)
    scallmsec = time(NULL);
#else
    ftime(&scallmsec);
#endif

    rfc_rc = RfcCallReceive (rfc_handle,
                             function,
                             exporting,
                             importing,
                             tables,
                             &exception);
    printfU (cU("%d\n"), rfc_rc);

    /* check return code */
    switch(rfc_rc)
    {
      case RFC_OK:
        break;
      case RFC_EXCEPTION:
      case RFC_SYS_EXCEPTION:
        rfc_error(exception);
        break;
      default:
        rfc_error (cU("RfcCallReceive"));
        break;
    }

#if defined(SAPonUNIX)
    gettimeofday(&ecallmsec, &tzp);
#elif defined(SAPonOS400)
    ecallmsec = time(NULL);
#else
    ftime(&ecallmsec);
#endif

    display (cU("Export Parameter"), exporting);

    display (cU("Import Parameter"), importing);

    /* Compute required times */
#if defined(SAPonUNIX)
    msecopen = diffmsec(&eopenmsec, &sopenmsec);
    mseccall = diffmsec(&ecallmsec, &scallmsec);
    msecfill = diffmsec(&efillmsec, &sfillmsec);
#elif defined(SAPonOS400)
    msecopen = diffmsec_AS400(eopenmsec, sopenmsec);
    mseccall = diffmsec_AS400(ecallmsec, scallmsec);
    msecfill = diffmsec_AS400(efillmsec, sfillmsec);
#else
    msecopen = diffmsec_NON_UNIX(&eopenmsec, &sopenmsec);
    mseccall = diffmsec_NON_UNIX(&ecallmsec, &scallmsec);
    msecfill = diffmsec_NON_UNIX(&efillmsec, &sfillmsec);
#endif

    totalsnd = (lgit0332 * 332) + (lgit1000 * 1000);
    totalrcv = (lget0332 * 332) + (lget1000 * 1000);
    totaldat = totalsnd + totalrcv;

    printfU (cU("\n\n%s"), text0332);
    printfU (cU("\n%s"), text1000);

    /*----------------------------------------------------------------*/
    /* Check and output result of test                                */
    /*----------------------------------------------------------------*/

    if (exitcode[0] != cU('O'))
    {
      printfU (cU("\n\n ERROR OCCURS !!!!!\n"));
      return 0;
    }

    if (checktab[0] == cU('Y'))
    {
#if defined(SAPonUNIX)
      gettimeofday(&schekmsec, &tzp);
#elif defined(SAPonOS400)
      schekmsec = time(NULL);
#else
      ftime(&schekmsec);
#endif
    }

    /* Check receive length, real length and contents of ETAB0332 */
    check_table(tables[2].ithandle,
                cU("ETAB0332"),
                lget0332,
                checktab,
                texi0332,
                exitcode);
		
    /* Check receive length and real length of ETAB1000 */
    check_table(tables[3].ithandle,
                cU("ETAB1000"),
                lget1000,
                checktab,
                texi1000,
                retcode);

    printfU (cU("\n%s"), texi0332);
    printfU (cU("\n%s"), texi1000);

    if (checktab[0] == cU('Y'))
    {
#if defined(SAPonUNIX)
      gettimeofday(&echekmsec, &tzp);
      msecchek = diffmsec(&echekmsec, &schekmsec);
#elif defined(SAPonOS400)
      echekmsec = time(NULL);
      msecchek = diffmsec_AS400(echekmsec, schekmsec);
#else
      ftime(&echekmsec);
      msecchek = diffmsec_NON_UNIX(&echekmsec, &schekmsec);
#endif
    }

    /*----------------------------------------------------------------*/
    /* Output result of test if everything OK                         */
    /*----------------------------------------------------------------*/

    if ((exitcode[0] != cU('O')) || (retcode[0] != cU('O')))
    {
      printfU (cU("\n\n ERROR OCCURS !!!!!\n"));
      return 0;
    }

    printfU (cU("\n\n"));
    printfU (cU("*******************************************************************************\n"));
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*  Required time for connect and SAP logon                   %10u msec  *\n"), msecopen);
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*  Time for fill export tables before RFC                    %10u msec  *\n"), msecfill);
    printfU (cU("*                                                                             *\n"));

    if (checktab[0] == cU('Y'))
    {
      printfU (cU("*  Time for RFC (cU(+ remotely check contents of export tables) %10u msec  *\n"), mseccall);
      printfU (cU("*                                                                             *\n"));
      printfU (cU("*  Time for check contents of import tables (locally)        %10u msec  *\n"), msecchek);
    }
    else
      printfU (cU("*  Time for RFC (No check contents of all tables)            %10u msec  *\n"), mseccall);

    printfU (cU("*                                                                             *\n"));
    printfU (cU("*  Total send data                                           %10u bytes *\n"), totalsnd);
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*  Total receive data                                        %10u bytes *\n"), totalrcv);
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*  Total transfer data                                       %10u bytes *\n"), totaldat);
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*******************************************************************************\n"));

    /*  Output internal tables on screen if required */
    if (printtab[0] == cU('Y'))
    {
      printfU (cU("\n\nHit return key for output internal tables on screen"));
      fflush (stdout);
      getcharU ();
      output_table (tables[0].ithandle, cU("ITAB0332"));
      output_table (tables[1].ithandle, cU("ITAB1000"));

      output_table (tables[2].ithandle, cU("ETAB0332"));
      output_table (tables[3].ithandle, cU("ETAB1000"));
    }

    if (nrcount < nrcall-1)
    {
      /* Free all internal tables */
      printfU (cU("\n<==  ItFree (ITAB0332)"));
      ItFree(tables[0].ithandle);

      printfU (cU("\n<==  ItFree (ITAB1000)"));
      ItFree(tables[1].ithandle);

      printfU (cU("\n<==  ItFree (ETAB0332)"));
      ItFree(tables[2].ithandle);

      printfU (cU("\n<==  ItFree (ETAB1000)"));
      ItFree(tables[3].ithandle);

      if (use_close[0] == cU('Y'))
      {
        printfU (cU("\n<==  RfcClose\n"));
        RfcClose(rfc_handle);
      }

      printfU (cU("\n\nHit return key for next RFC call\n"));
      fflush(stdout);
      getcharU ();

      /* If explizit or RFC server is NON-SAP and use the    */
      /* register mode:                                      */
      /*    ===> Close and Open the RFC connection           */
      if (use_close[0] == cU('Y'))
      {
	/* LOAD BALANCING only for the first time */
	if ((sapsys[0] == RFC_SERVER_R3) && (load_balancing[0] == cU('Y')))
        {
          saprfc_ini[0]     = cU('N');
          load_balancing[0] = cU('N');
          strcpyU (ashost, rfc_attributes.partner_host);
          strcpyU (sysnr,  rfc_attributes.systnr);
        }

        RFC_connect(cU("srfcload_N"));
      }
    }
  }

  /* Get specific info of this RFC connection if server is External */
  if (sapsys[0] == RFC_SERVER_EXT)
    get_attributes(rfc_handle);

  /* read call statistic */
  RfcStatistic (rfc_handle, &stats);


  /*----------------------------------------------------------------*/
  /* Close RFC connection                                           */
  /*----------------------------------------------------------------*/
  printfU (cU("\n<==  RfcClose\n"));
  RfcClose(rfc_handle);

  /*----------------------------------------------------------------*/
  /* Delete all used internal tables                                */
  /*----------------------------------------------------------------*/
  printfU (cU("\n<==  ItDelete (ITAB0332)"));
  ItDelete(tables[0].ithandle);

  printfU (cU("\n<==  ItDelete (ITAB1000)"));
  ItDelete(tables[1].ithandle);

  printfU (cU("\n<==  ItDelete (ETAB0332)"));
  ItDelete(tables[2].ithandle);

  printfU (cU("\n<==  ItDelete (ETAB1000)"));
  ItDelete(tables[3].ithandle);

  return 0;
}


/*====================================================================*/
/*                                                                    */
/* Call any Function Module in SAP R/3                                */
/*                                                                    */
/* Restriction: NO export or import parameter are supported           */
/*              in this program                                       */
/*                                                                    */
/*====================================================================*/
int srfcdiag()
{
  get_parameter_for_diag_test();

  RFC_connect(cU("srfcdiag"));

  check_gui_for_start();

  for (nrcount=0; nrcount<nrcall; nrcount++)
  {
    /*----------------------------------------------------------------*/
    /* Call a function module or a transaction in R/3                 */
    /*----------------------------------------------------------------*/

    if (transaction_test[0] == cU('T'))
      function = cU("ABAP4_CALL_TRANSACTION");
    else
      function = fname;

    memsetR (&exporting[0], 0, sizeofR (exporting));
    memsetR (&importing[0], 0, sizeofR (importing));

    if (transaction_test[0] == cU('T'))
      rfc_param (&exporting[0], cU("TCODE"), TYPC, tcode, 4*sizeofR(SAP_UC));

    tables[0].name = NULL;

    printfU (cU("\n<==  RfcCallReceive (%s)     rfc_rc = "), function);
    rfc_rc = RfcCallReceive(rfc_handle,
                            function,
                            exporting,
                            importing,
                            tables,
                            &exception);
    printfU (cU("%d\n"), rfc_rc);

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

    if (nrcount < nrcall-1)
    {
      if (use_close[0] == cU('Y'))
      {
        printfU (cU("\n<==  RfcClose\n"));
        RfcClose(rfc_handle);
      }
      printfU (cU("\n\nHit return key for next RFC call\n"));
      fflush (stdout);
      getcharU ();

      /* If explizit or RFC server is NON-SAP and use the    */
      /* register mode:                                      */
      /*    ===> Close and Open the RFC connection           */
      if (use_close[0] == cU('Y'))
      {
	/* LOAD BALANCING only for the first time */
	if ((sapsys[0] == RFC_SERVER_R3) && (load_balancing[0] == cU('Y')))
        {
          saprfc_ini[0]     = cU('N');
          load_balancing[0] = cU('N');
          strcpyU (ashost, rfc_attributes.partner_host);
          strcpyU (sysnr,  rfc_attributes.systnr);
        }

        RFC_connect(cU("srfcdiag_N"));
      }
    }
  }

  /* Compute required times */
#if defined(SAPonUNIX)
  msecopen = diffmsec(&eopenmsec, &sopenmsec);
#elif defined(SAPonOS400)
  msecopen = diffmsec_AS400(eopenmsec, sopenmsec);
#else
  msecopen = diffmsec_NON_UNIX(&eopenmsec, &sopenmsec);
#endif

  printfU (cU("\n\n"));
  printfU (cU("*******************************************************************************\n"));
  printfU (cU("*                                                                             *\n"));
  printfU (cU("*  Required time for connect and SAP logon                   %10u msec  *\n"), msecopen);
  printfU (cU("*                                                                             *\n"));
  printfU (cU("*******************************************************************************\n"));

  /*----------------------------------------------------------------*/
  /* Close RFC connection                                           */
  /*----------------------------------------------------------------*/
  printfU (cU("\n<==  RfcClose\n"));
  RfcClose(rfc_handle);

  return 0;
}


/*====================================================================*/
/*                                                                    */
/* Call Function STFC_STRUCTURE                                       */
/*                                                                    */
/* Send/receive inhomogeneous structures and table                    */
/*                                                                    */
/*====================================================================*/
int srfctruc()
{
  SAP_RAW         * ptr_raw;
  const SAP_RAW     hex3_value[] = { 0x31, 0x32, 0x33 };

  get_parameter_for_conn_test();

  RFC_connect(cU("srfcstruc"));

  check_gui_for_start();

  /* Install structure 'RFCTEST' */
  /* starting with release 610 you may use either 
   * RfcInstallUnicodeStructure or RfcInstallStructure
   * when building an ASCII/MBCS program
   * you must use RfcInstallUnicodeStructure
   * when building an UNICODE program
   */
  rfc_rc = RfcInstallUnicodeStructure (cU("RFCTEST"),
                                       typeOfRfcTest,
                                       ENTRIES(typeOfRfcTest),
                                       0, NULL,
                                       &handleOfRfcTest );

  if (rfc_rc != RFC_OK)
    rfc_error (cU("RfcInstallStructure"));

  /* Prepare internal table RFCTABLE */
  tables[0].name     = cU("RFCTABLE");
  tables[0].nlen     = 8;
  tables[0].type     = handleOfRfcTest;
  tables[0].leng     = RFCTEST_LN;
  tables[0].itmode   = RFC_ITMODE_BYREFERENCE;

  printfU (cU("\n<==  ItCreate (RFCTABLE)\n"));
  tables[0].ithandle = ItCreate (cU("RFCTABLE"),
                                 RFCTEST_LN,
                                 0,
                                 0);

  if (tables[0].ithandle == NULL )
    rfc_error (cU("ItCreate RFCTABLE"));

  for (nrcount=0; nrcount<nrcall; nrcount++)
  {
    /*----------------------------------------------------------------*/
    /* Call inhomogeneous structure/table test                        */
    /*----------------------------------------------------------------*/

    function = name_structure_test;

    exporting[0].name = cU("IMPORTSTRUCT");
    exporting[0].nlen = 12;
    exporting[0].addr = &imstruct;
    exporting[0].leng = RFCTEST_LN;
    exporting[0].type = handleOfRfcTest;

    exporting[1].name = NULL;

    importing[0].name = cU("RESPTEXT");
    importing[0].nlen = 8;
    importing[0].addr = resptext;
    importing[0].leng = sizeofR (resptext);
    importing[0].type = TYPC;

    importing[1].name = cU("ECHOSTRUCT");
    importing[1].nlen = 10;
    importing[1].addr = &exstruct;
    importing[1].leng = RFCTEST_LN;
    importing[1].type = handleOfRfcTest;

    importing[2].name = NULL;

    /* terminate array */
    tables[1].name = NULL;

    /* Fill structure 'IMPORTSTRUCT' */
    imstruct.rfcfloat = 1000000.0;
    imstruct.rfcchar1[0] = cU('A');
    imstruct.rfcint2  = 1000;
    imstruct.rfcint1  = 10;
    memcpyU (imstruct.rfcchar4, cU("EXT."), 4);
    imstruct.rfcint4  = 100000;
    memcpyR (imstruct.rfchex3,  hex3_value, 3);
    memcpyU (imstruct.rfcchar2, cU("BC"),       2);
    memcpyU (imstruct.rfctime,  cU("124500"),   6);
    memcpyU (imstruct.rfcdate,  cU("19960924"), 8);

    /* Fill table 'RFCTABLE' */
    lineleng = ItLeng(tables[0].ithandle);
    ptr_raw = (SAP_RAW *) ItAppLine(tables[0].ithandle);
    if (ptr_raw == NULL)
      rfc_error (cU("ItAppLine"));
    memcpyR (ptr_raw, &imstruct, lineleng);

#if defined(SAPonUNIX)
    gettimeofday(&efillmsec, &tzp);
#elif defined(SAPonOS400)
    efillmsec = time(NULL);
#else
    ftime(&efillmsec);
#endif

    printfU (cU("\n<==  RfcCallReceive (%s)     rfc_rc = "), function);

#if defined(SAPonUNIX)
    gettimeofday(&scallmsec, &tzp);
#elif defined(SAPonOS400)
    scallmsec = time(NULL);
#else
    ftime(&scallmsec);
#endif

    rfc_rc = RfcCallReceive(rfc_handle,
			    function,
			    exporting,
			    importing,
			    tables,
			    &exception);
    printfU (cU("%d\n"), rfc_rc);

    /* check return code */
    switch(rfc_rc)
    {
      case RFC_OK:
        break;
      case RFC_EXCEPTION:
      case RFC_SYS_EXCEPTION:
        rfc_error(exception);
        break;
      default:
        rfc_error (cU("RfcCallReceive"));
        break;
    }

#if defined(SAPonUNIX)
    gettimeofday(&ecallmsec, &tzp);
#elif defined(SAPonOS400)
    ecallmsec = time(NULL);
#else
    ftime(&ecallmsec);
#endif

    printfU (cU("\nExporting Parameter: IMPORTSTRUCT"));
    print_mystruct (&imstruct);

    printfU (cU("\n\nImporting Parameter RESPTEXT:"));
    printfU (cU("    length = 255    type = TYPC"));
    resptext[255] = 0;
    printfU (cU("\n      value = '%s'"), resptext);

    printfU (cU("\nImporting Parameter ECHOSTRUCT:"));
    print_mystruct (&exstruct);

    printfU (cU("\n\nInternal Table: RFCTABLE"));
    print_mytable (tables[0].ithandle);

    /* Compute required times */
#if defined(SAPonUNIX)
    msecopen = diffmsec(&eopenmsec, &sopenmsec);
    mseccall = diffmsec(&ecallmsec, &scallmsec);
    msecfill = diffmsec(&efillmsec, &sfillmsec);
#elif defined(SAPonOS400)
    msecopen = diffmsec_AS400(eopenmsec, sopenmsec);
    mseccall = diffmsec_AS400(ecallmsec, scallmsec);
    msecfill = diffmsec_AS400(efillmsec, sfillmsec);
#else
    msecopen = diffmsec_NON_UNIX(&eopenmsec, &sopenmsec);
    mseccall = diffmsec_NON_UNIX(&ecallmsec, &scallmsec);
    msecfill = diffmsec_NON_UNIX(&efillmsec, &sfillmsec);
#endif

    printfU (cU("\n\n"));
    printfU (cU("*******************************************************************************\n"));
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*  Required time for connect and SAP logon                   %10u msec  *\n"), msecopen);
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*  Required time for RFC without connect and logon           %10u msec  *\n"), mseccall);
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*******************************************************************************\n"));

    if (nrcount < nrcall-1)
    {
      /* Free internal table */
      printfU (cU("\n<==  ItFree (RFCTABLE)"));
      ItFree (tables[0].ithandle);

      if (use_close[0] == cU('Y'))
      {
        printfU (cU("\n<==  RfcClose\n"));
        RfcClose(rfc_handle);
      }

      printfU (cU("\n\nHit return key for next RFC call\n"));
      fflush (stdout);
      getcharU ();

      /* If explizit or RFC server is NON-SAP and use the    */
      /* register mode:                                      */
      /*    ===> Close and Open the RFC connection           */
      if (use_close[0] == cU('Y'))
      {
          /* LOAD BALANCING only for the first time */
            if ((sapsys[0] == RFC_SERVER_R3) && (load_balancing[0] == cU('Y')))
            {
                saprfc_ini[0]     = cU('N');
                load_balancing[0] = cU('N');
                strcpyU (ashost, rfc_attributes.partner_host);
                strcpyU (sysnr,  rfc_attributes.systnr);
            }

        
            RFC_connect(cU("srfcstruc_N"));
      }
    }
  }

  /* Get specific info of this RFC connection if server is External */
  if (sapsys[0] == RFC_SERVER_EXT)
    get_attributes(rfc_handle);

  /*----------------------------------------------------------------*/
  /* Close RFC connection                                           */
  /*----------------------------------------------------------------*/
  printfU (cU("\n<==  RfcClose\n"));
  RfcClose(rfc_handle);

  /*----------------------------------------------------------------*/
  /* Delete used internal table                                     */
  /*----------------------------------------------------------------*/
  printfU (cU("\n<==  ItDelete (RFCTABLE)"));
  ItDelete (tables[0].ithandle);

  return 0;
}


/*====================================================================*/
/*                                                                    */
/* Conversion SNC Name <===> SNC ACL Key                              */
/*                                                                    */
/*====================================================================*/
int ssncconv()
{
  memsetR (snc_aclkey, 0, sizeofR(snc_aclkey));
  strcpyU (snc_name,  std_snc_partnername);
  strcpyU (snc_lib,   std_snc_lib);

  printfU (cU("\n\n"));

  ptr = getenvU (cU("SNC_LIB"));
  if (ptr != NULL)
    strcpyU (snc_lib, ptr);

  printfU ( cU("               SNC Library Name...: (%s): "), snc_lib);
  getsU (ips);
  if (strlenU (ips) != 0)
  {
    ips[256] = 0;
    strcpyU (snc_lib, ips);
  }

  printfU (cU("               SNC Name...........: (%s): "), snc_name);
  getsU (ips);
  if (strlenU (ips) != 0)
  {
    ips[256] = 0;
    strcpyU (snc_name, ips);
  }

  printfU (cU("\n\n<==  RfcSncNameToAclKey       rfc_rc = "));
  rfc_rc = RfcSncNameToAclKey(snc_lib,
			      snc_name,
			      snc_aclkey,
			      SNC_NAME_LN,
			      &aclkey_len,
			      &error_info);
  printfU (cU("%d\n"), rfc_rc);

  if (rfc_rc != RFC_OK)
  {
    printfU (cU("\nGroup       Error group %d\n"), error_info.group);
    printfU (cU("Key         %s\n"), error_info.key);
    printfU (cU("Message     %s\n\n"), error_info.message);
    exit(1);
  }

  printfU (cU("SNC ACL Key: Length = %d\n"), aclkey_len);

  printfU (cU("             Value  = "));

  for (i=0; i<aclkey_len; i++)
    printfU (cU("%02X"), (unsigned) snc_aclkey[i]);

  snc_name[0] = 0;
  printfU (cU("\n\n<==  RfcSncAclKeyToName       rfc_rc = "));
  rfc_rc = RfcSncAclKeyToName ( snc_lib,
                                snc_aclkey,
                                aclkey_len,
                                snc_name,
                                SNC_NAME_LN,
                                &error_info);
  printfU (cU("%d\n"), rfc_rc);

  if (rfc_rc != RFC_OK)
  {
    printfU (cU("\nGroup       Error group %d\n"), error_info.group);
    printfU (cU("Key         %s\n"), error_info.key);
    printfU (cU("Message     %s\n\n"), error_info.message);
    exit(1);
  }

  printfU (cU("SNC Name   : Length = %u\n"), strlenU (snc_name));

  printfU (cU("             Value  = %s\n\n"), snc_name);

  return 0;
}


/*====================================================================*/
/*                                                                    */
/* Call Function STFC_CONNECTION_BACK                                 */
/*                                                                    */
/* Send message and wait for echo and info with callback              */
/*                                                                    */
/*====================================================================*/
int srfcback()
{
  get_parameter_for_back_test();

  RFC_connect(cU("srfcback"));

  check_gui_for_start();

  /*------------------------------------------------------------------*/
  /* Install RFC Function STFC_CONNECTION for Callback Test           */
  /*------------------------------------------------------------------*/

  printfU (cU("\n<==  RfcInstallFunction (%s)      rfc_rc = "),
           name_connection_test);

  rfc_rc = RfcInstallFunction(name_connection_test,
                              (RFC_ONCALL) connection_test,
                              connection_test_docu());
  printfU (cU("%d\n"), rfc_rc);

  if (rfc_rc != RFC_OK)
    exit(1);

  for (nrcount=0; nrcount<nrcall; nrcount++)
  {
    /*----------------------------------------------------------------*/
    /* Call Callback Test                                             */
    /*----------------------------------------------------------------*/

    function = name_callback_test;

    memsetR (&exporting[0], 0, sizeofR(exporting));
    memsetR (&importing[0], 0, sizeofR(importing));

    memsetU (requtext, cU(' '), 255);
    strcpyU (requtext, cU("abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890"));
    requtext[strlenU (requtext)] = cU(' ');

    rfc_param(&exporting[0], cU("NRBACK"),   TYPC, nrback_field, 5*sizeofR(SAP_UC));
    rfc_param(&exporting[1], cU("REQUTEXT"), TYPC, requtext,     255*sizeofR(SAP_UC));

    rfc_param(&importing[0], cU("NOBACK"),   TYPC, noback_field, 5*sizeofR(SAP_UC));
    rfc_param(&importing[1], cU("ECHOTEXT"), TYPC, echotext,     255*sizeofR(SAP_UC));
    rfc_param(&importing[2], cU("RESPTEXT"), TYPC, resptext,     255*sizeofR(SAP_UC));

    /* no table ---> terminate array */
    tables[0].name = NULL;

    printfU (cU("\n<==  RfcCallReceive (%s)     rfc_rc = "), function);

#if defined(SAPonUNIX)
    gettimeofday(&scallmsec, &tzp);
#elif defined(SAPonOS400)
    scallmsec = time(NULL);
#else
    ftime(&scallmsec);
#endif

    rfc_rc = RfcCallReceive (rfc_handle,
                             function,
                             exporting,
                             importing,
                             tables,
                             &exception);

    if (nrback == 0)
      printfU (cU("%d\n"), rfc_rc);
    else
      printfU (cU("\n     Return of RfcCallReceive:  rfc_rc = %d\n"), rfc_rc);

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

#if defined(SAPonUNIX)
    gettimeofday(&ecallmsec, &tzp);
#elif defined(SAPonOS400)
    ecallmsec = time(NULL);
#else
    ftime(&ecallmsec);
#endif

    noback = atoiU (noback_field);

    if (noback != nrback)
    {
      printfU (cU("\n\n ERROR OCCURS !!!!!"));
      printfU (cU("\nNo. of callback's should be done:  %d"), nrback);
      printfU (cU("\nNo. of callback's are executed  :  %d\n"), noback);
      return 0;
    }

    display (cU("Export Parameter"), exporting);

    display (cU("Import Parameter"), importing);

    /* Compute required times */
#if defined(SAPonUNIX)
    msecopen = diffmsec(&eopenmsec, &sopenmsec);
    mseccall = diffmsec(&ecallmsec, &scallmsec);
#elif defined(SAPonOS400)
    msecopen = diffmsec_AS400(eopenmsec, sopenmsec);
    mseccall = diffmsec_AS400(ecallmsec, scallmsec);
#else
    msecopen = diffmsec_NON_UNIX(&eopenmsec, &sopenmsec);
    mseccall = diffmsec_NON_UNIX(&ecallmsec, &scallmsec);
#endif

    printfU (cU("\n\n"));
    printfU (cU("*******************************************************************************\n"));
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*  Required time for connect and SAP logon                   %10u msec  *\n"), msecopen);
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*  Required time for RFC without connect and logon           %10u msec  *\n"), mseccall);
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*******************************************************************************\n"));

    if (nrcount < nrcall-1)
    {
      if (use_close[0] == cU('Y'))
      {
        printfU (cU("\n<==  RfcClose\n"));
        RfcClose(rfc_handle);
      }
      printfU (cU("\n\nHit return key for next RFC call\n"));
      fflush (stdout);
      getcharU ();

      /* If explizit or RFC server is NON-SAP and use the    */
      /* register mode:                                      */
      /*    ===> Close and Open the RFC connection           */
      if (use_close[0] == cU('Y'))
      {
	/* LOAD BALANCING only for the first time */
	if ((sapsys[0] == RFC_SERVER_R3) && (load_balancing[0] == cU('Y')))
        {
          saprfc_ini[0]     = cU('N');
          load_balancing[0] = cU('N');
          strcpyU (ashost, rfc_attributes.partner_host);
          strcpyU (sysnr,  rfc_attributes.systnr);
        }

        RFC_connect(cU("srfcback_N"));
      }
    }
  }

  /* Get specific info of this RFC connection if server is External */
  if (sapsys[0] == RFC_SERVER_EXT)
    get_attributes(rfc_handle);

  /*----------------------------------------------------------------*/
  /* Close RFC connection                                           */
  /*----------------------------------------------------------------*/
  printfU (cU("\n<==  RfcClose\n"));
  RfcClose(rfc_handle);

  return 0;
}


/*====================================================================*/
/*                                                                    */
/* Check Registered Server                                            */
/*                                                                    */
/*                                                                    */
/*====================================================================*/
int srfcchek()
{
  RFC_INT ntotal,
	  ninit,
	  nready,
	  nbusy;

  get_parameter_for_reg_server();

  printfU (cU("\n\n<==  RfcCheckRegisterServer       rfc_rc = "));

  rfc_rc = RfcCheckRegisterServer(tpid, gwhost, gwserv,
				  &ntotal, &ninit, &nready, &nbusy,
				  &error_info);

  printfU (cU("%d\n"), rfc_rc);

  if (rfc_rc != RFC_OK)
  {
    printfU (cU("\nGroup       Error group %d\n"), error_info.group);
    printfU (cU("Key         %s\n"), error_info.key);
    printfU (cU("Message     %s\n\n"), error_info.message);
    exit(1);
  }

  printfU (cU("\nNo. registered servers               :  %d"), ntotal);
  printfU (cU("\nNo. registered servers in INIT-state :  %d"), ninit);
  printfU (cU("\nNo. registered servers in READY-state:  %d"), nready);
  printfU (cU("\nNo. registered servers in BUSY-state :  %d"), nbusy);

  return 0;
}


/*====================================================================*/
/*                                                                    */
/* Cancel Registered Server                                           */
/*                                                                    */
/*                                                                    */
/*====================================================================*/
int srfccanc()
{
  RFC_INT ntotal,
	  ncancel;

  get_parameter_for_reg_server();

  printfU (cU("\n\n<==  RfcCancelRegisterServer      rfc_rc = "));

  rfc_rc = RfcCancelRegisterServer(tpid, gwhost, gwserv,
				   &ntotal, &ncancel, &error_info);

  printfU (cU("%d\n"), rfc_rc);

  if (rfc_rc != RFC_OK)
  {
    printfU (cU("\nGroup       Error group %d\n"), error_info.group);
    printfU (cU("Key         %s\n"), error_info.key);
    printfU (cU("Message     %s\n\n"), error_info.message);
    exit(1);
  }

  printfU (cU("\nNo. registered servers               :  %d"), ntotal);
  printfU (cU("\nNo. canceled   servers               :  %d"), ncancel);

  return 0;
}


/*====================================================================*/
/*                                                                    */
/* Conversion BCD numbers <===> rfc_char_t string                           */
/*                                                                    */
/*====================================================================*/
int srfcconv()
{
  int   rc,
        bcd_char_len,
        bcd_num_len,
        decimal_no;

  rfc_char_t           bcd_char[33];
  RFC_BCD              bcd_num[16];

  printfU (cU("\n\n"));

  strcpyU (bcd_char, cU("1234567890.12345"));
  printfU (cU("               character string...: (%s): "), bcd_char);
  getsU (ips);
  if (strlenU (ips) != 0)
  {
    ips[32] = 0;
    strcpyU (bcd_char, ips);
  }

  bcd_char_len = strlenU (bcd_char);

  if (memchrU (bcd_char, cU('.'), bcd_char_len) != NULL)
    decimal_no = -1;
  else
    decimal_no = 0;

  bcd_num_len = 16;

  printfU (cU("\n\n"));

  printfU (cU("\n\n<==  RfcConvertCharToBcd       rc = "));
  rc = RfcConvertCharToBcd(bcd_char,
                           bcd_char_len,
                           &decimal_no,
                           bcd_num,
                           bcd_num_len);

  printfU (cU("%d\n"), rc);

  if (rc) exit(1);

  printfU (cU("bcd_char:  Length = %d\n"), bcd_char_len);
  printfU (cU("           Value  = "));
  for (i=0; i<bcd_char_len; i++)
    printfU (cU("%c"), bcd_char[i]);

  printfU (cU("\nbcd_num:   Length = %d\n"), bcd_num_len);
  printfU (cU("           Value  = "));
  for (i=0; i<bcd_num_len; i++)
    printfU (cU("%02X"), bcd_num[i]);
  printfU (cU("\n           Decimal_No = %d\n"), decimal_no);

  bcd_char_len = 32;
  printfU (cU("\n\n<==  RfcConvertBcdToChar       rc = "));
  rc = RfcConvertBcdToChar(bcd_num,
                           bcd_num_len,
                           decimal_no,
                           bcd_char,
                           bcd_char_len);

  printfU (cU("%d\n"), rc);

  printfU (cU("bcd_num:   Length = %d\n"), bcd_num_len);
  printfU (cU("           Value  = "));
  for (i=0; i<bcd_num_len; i++)
    printfU (cU("%02X"), bcd_num[i]);
  printfU (cU("\n           Decimal_No = %d\n"), decimal_no);

  printfU (cU("\nbcd_char:  Length = %d\n"), bcd_char_len);
  printfU (cU("           Value  = "));
  for (i=0; i<bcd_char_len; i++)
    printfU (cU("%c"), bcd_char[i]);

  printfU (cU("\n\n"));

  return 0;
}


/*====================================================================*/
/*                                                                    */
/* Call Function STFC_START_CONNECT_REG_SERVER                        */
/*                                                                    */
/* Start and connect to a registered server during the execution of   */
/* a function module from within an R/3 system                        */
/*                                                                    */
/* This registered server will be started automatically by the RFC    */
/* library using in the running RFC client.                           */
/*                                                                    */
/* Send message and wait for echo and info                            */
/*                                                                    */
/*====================================================================*/
int srfcsreg()
{
  get_parameter_for_sreg_test();

  if (sapsys[0] != RFC_SERVER_R3)
  {
    printfU (cU("\n\nThis feature is only available with R/3 as RFC server !!!!!\n\n"));
    exit(1);
  }

  RFC_connect(cU("srfcreg"));

  check_gui_for_start();

  /* Allows the RFC library starting any program */
  printfU (cU("\n<==  RfcAllowStartProgram    rfc_rc = "));
  rfc_rc = RfcAllowStartProgram (NULL);
  printfU (cU("%d\n"), rfc_rc);
  if (rfc_rc != RFC_OK)
    exit(1);

  for (nrcount=0; nrcount<nrcall; nrcount++)
  {
    /*----------------------------------------------------------------*/
    /* Call Start/Connect Reg. Server Test                            */
    /*----------------------------------------------------------------*/

    function = name_sreg_server_test;

    memsetR (&exporting[0], 0, sizeofR(exporting));
    memsetR (&importing[0], 0, sizeofR(importing));

    memsetU (sreg_tpname, cU(' '), 64);
    sreg_tpname[64] = 0;
    i = strlenU (tpname);
    if (i > 64)
      i = 64;
    memcpyU (sreg_tpname, tpname, i);


    rfc_param(&exporting[0], cU("PROGNAME"), TYPC,    sreg_tpname, 64*sizeofR(SAP_UC));
    rfc_param(&importing[0], cU("ERR_CODE"), TYPINT, &err_code,    sizeofR(int));
    rfc_param(&importing[1], cU("ERR_MESS"), TYPC,    err_mess,    255*sizeofR(SAP_UC));

    /* no table ---> terminate array */
    tables[0].name = NULL;

    printfU (cU("\n<==  RfcCallReceive (%s)     rfc_rc = "), function);

#if defined(SAPonUNIX)
    gettimeofday(&scallmsec, &tzp);
#elif defined(SAPonOS400)
    scallmsec = time(NULL);
#else
    ftime(&scallmsec);
#endif

    rfc_rc = RfcCallReceive(rfc_handle,
                            function,
                            exporting,
                            importing,
                            tables,
                            &exception);
    printfU (cU("%d\n"), rfc_rc);

    /* check return code */
    switch(rfc_rc)
    {
      case RFC_OK:
        break;
      case RFC_EXCEPTION:
      case RFC_SYS_EXCEPTION:
        rfc_error(exception);
        break;
      default:
        rfc_error(cU("RfcCallReceive"));
        break;
    }

#if defined(SAPonUNIX)
    gettimeofday(&ecallmsec, &tzp);
#elif defined(SAPonOS400)
    ecallmsec = time(NULL);
#else
    ftime(&ecallmsec);
#endif

    display (cU("Export Parameter"), exporting);

    display (cU("Import Parameter"), importing);

    /* Compute required times */
#if defined(SAPonUNIX)
    msecopen = diffmsec(&eopenmsec, &sopenmsec);
    mseccall = diffmsec(&ecallmsec, &scallmsec);
#elif defined(SAPonOS400)
    msecopen = diffmsec_AS400(eopenmsec, sopenmsec);
    mseccall = diffmsec_AS400(ecallmsec, scallmsec);
#else
    msecopen = diffmsec_NON_UNIX(&eopenmsec, &sopenmsec);
    mseccall = diffmsec_NON_UNIX(&ecallmsec, &scallmsec);
#endif

    printfU (cU("\n\n"));
    printfU (cU("*******************************************************************************\n"));
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*  Required time for connect and SAP logon                   %10u msec  *\n"), msecopen);
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*  Required time for RFC without connect and logon           %10u msec  *\n"), mseccall);
    printfU (cU("*                                                                             *\n"));
    printfU (cU("*******************************************************************************\n"));

    if (nrcount < nrcall-1)
    {
      if (use_close[0] == cU('Y'))
      {
        printfU (cU("\n<==  RfcClose\n"));
        RfcClose(rfc_handle);
      }
      printfU (cU("\n\nHit return key for next RFC call\n"));
      fflush (stdout);
      getcharU ();

      /* If explizit or RFC server is NON-SAP and use the    */
      /* register mode:                                      */
      /*    ===> Close and Open the RFC connection           */
      if (use_close[0] == cU('Y'))
      {
        /* LOAD BALANCING only for the first time */
        if ((sapsys[0] == RFC_SERVER_R3) && (load_balancing[0] == cU('Y')))
        {
          saprfc_ini[0]     = cU('N');
          load_balancing[0] = cU('N');
          strcpyU (ashost, rfc_attributes.partner_host);
          strcpyU (sysnr,  rfc_attributes.systnr);
        }

        RFC_connect(cU("srfcreg_N"));
      }
    }
  }

  /*----------------------------------------------------------------*/
  /* Close RFC connection                                           */
  /*----------------------------------------------------------------*/
  printfU (cU("\n<==  RfcClose\n"));
  RfcClose(rfc_handle);

  return 0;
}


/*--------------------------------------------------------------------*/
/* Get parameters for working with START/CONNECT to reg. server       */
/*--------------------------------------------------------------------*/
void get_parameter_for_sreg_server()
{
  printfU (cU("\n\n               SPECIFIC TEST DATA:\n"));

  /* Program Name of the according registered server */
  if (strlenU (tpname) == 0)
    strcpyU (tpname, cU("srfcserv.exe"));
  printfU (cU("                 Program Name of the reg. server.........: (%s): "), tpname);
  fflush(stdout);
  getsU (ips);
  if (strlenU (ips) == 0)
    strcpyU (ips, tpname);
  ips[100] = 0;
  strcpyU (tpname, ips);

  return;
}


/*--------------------------------------------------------------------*/
/* Get parameters for working with registerer servers                 */
/*--------------------------------------------------------------------*/
void get_parameter_for_reg_server()
{
  printfU (cU("\n\n               CONNECTION PARAMETERS:\n"));

  /* Host name of a SAP gateway */
  if (strlenU (gwhost) == 0)
    strcpyU (gwhost, stde_gwhost);
  printfU (cU("                 Host name of the SAP gateway............: (%s): "), gwhost);
  fflush(stdout);
  getsU (ips);
  if (strlenU (ips) == 0)
    strcpyU (ips,  gwhost);
  ips[100] = 0;
  strcpyU (gwhost, ips);

  /* Service of the specified SAP gateway */
  if (strlenU (gwserv) == 0)
    strcpyU (gwserv, stde_gwserv);
  printfU (cU("                 Service of the specified gateway........: (%s): "), gwserv);
  fflush(stdout);
  getsU (ips);
  if (strlenU (ips) == 0)
    strcpyU (ips,  gwserv);
  ips[7] = 0;
  strcpyU (gwserv, ips);

  /* Program ID of partner program running in register mode */
  if (strlenU (tpid) == 0)
    strcpyU (tpid, stde_tpid);
  printfU (cU("                 Program ID of registered partner program: (%s): "), tpid);
  fflush(stdout);
  getsU (ips);
  if (strlenU (ips) == 0)
    strcpyU (ips,  tpid);
  ips[100] = 0;
  strcpyU (tpid, ips);

  return;
}

/*--------------------------------------------------------------------*/
/* Get specific info of an RFC connection                             */
/*--------------------------------------------------------------------*/
void get_attributes(RFC_HANDLE rfc_handle)
{
    RFC_RC          rfc_rc;
    RFC_CODE_PAGE   cpOwn, 
                    cpRemote, 
                    cpComm;
    SAP_CHAR        cRepl;
    int             iIgnoreErrors = 0;
    int             iPcs = 0;
    void           *pContext;
    SAP_CHAR       *pCtx;
    
    printfU (cU("\n<==  RfcGetAttributes         rc = "));
    
    rc = RfcGetAttributes(rfc_handle, &rfc_attributes);
    
    printfU (cU("%d\n"), rc);
    
    if (rc)
     rfc_error (cU("RfcGetAttributes"));

    
    printfU (cU("\nAttributes of this RFC connection"));
    printfU (cU("\n---------------------------------"));
    printfU (cU("\nDestination            :  %s"), rfc_attributes.dest);
    printfU (cU("\nMy Host                :  %s"), rfc_attributes.own_host);

    if (rfc_attributes.rfc_role == RFC_ROLE_CLIENT) 
    {
        if (rfc_attributes.partner_type == RFC_SERVER_EXT)
         printfU (cU("\nServer Program Name    :  %s"), rfc_attributes.partner_host);
        else if (rfc_attributes.partner_type == RFC_SERVER_EXT_REG)
         printfU (cU("\nServer Program ID      :  %s"), rfc_attributes.partner_host);
        else
         printfU (cU("\nPartner Host           :  %s"), rfc_attributes.partner_host);
    }
    else
     printfU (cU("\nPartner Host           :  %s"), rfc_attributes.partner_host);

    
    printfU (cU("\nSystem No.             :  %s"), rfc_attributes.systnr);
    printfU (cU("\nSystem Name            :  %s"), rfc_attributes.sysid);
    printfU (cU("\nClient                 :  %s"), rfc_attributes.client);
    printfU (cU("\nUser                   :  %s"), rfc_attributes.user);
    printfU (cU("\nLanguage               :  %s"), rfc_attributes.language);
    printfU (cU("\nISO-Language           :  %s"), rfc_attributes.ISO_language);

    
    if (rfc_attributes.trace == cU('X'))
     printfU (cU("\nRFC Trace              :  ON"));
    else
     printfU (cU("\nRFC Trace              :  OFF"));

    printfU (cU("\nMy Codepage            :  %s"), rfc_attributes.own_codepage);

#ifdef SAPwithUNICODE
    printfU (cU("\nMy PCS Codepage        :  %s"), rfc_attributes.own_codepage_pcs);
#endif
    
    printfU (cU("\nPartner Codepage       :  %s"), rfc_attributes.partner_codepage);

    
    if (rfc_attributes.rfc_role == RFC_ROLE_CLIENT)
     printfU (cU("\nRFC Role               :  External RFC Client"));
    else if (rfc_attributes.own_type == RFC_SERVER_EXT)
     printfU (cU("\nRFC Role               :  External RFC Server, started by SAP gateway"));
    else
     printfU (cU("\nRFC Role               :  External RFC Server, registered at SAP gateway"));

    printfU (cU("\nRFC Library Release    :  %s"), rfc_attributes.own_rel);

    if (rfc_attributes.partner_type == RFC_SERVER_R3)
     printfU (cU("\nRFC Partner            :  SAP R/3"));
    else if (rfc_attributes.partner_type == RFC_SERVER_R2)
     printfU (cU("\nRFC Partner            :  SAP R/2"));
    else if (rfc_attributes.rfc_role == RFC_ROLE_CLIENT)
    {
        if (rfc_attributes.partner_type == RFC_SERVER_EXT)
         printfU (cU("\nRFC Partner            :  External RFC Server, started by SAP gateway"));
        else
         printfU (cU("\nRFC Partner            :  External RFC Server, registered at SAP gateway"));
    }
    else
     printfU (cU("\nRFC Partner            :  External RFC Client"));

    
    printfU (cU("\nPartner System Release :  %s"), rfc_attributes.partner_rel);
    printfU (cU("\nR/3 Kernel Release     :  %s"), rfc_attributes.kernel_rel);
    printfU (cU("\nCPI-C Conversation ID  :  %s"), rfc_attributes.CPIC_convid);
    printfU (cU("\nPartners Char Size     :  %s"), rfc_attributes.pcs);
    printfU (cU("\nRemote code page actual:  %s"), rfc_attributes.real_partner_codepage);
    printfU (cU("\nProgram Name           :  %s"), rfc_attributes.progname);

    printfU (cU("\n"));

    printfU (cU("\n<==  RfcGetCodePagesInfo      rc = "));    
    rfc_rc = RfcGetCodePagesInfo (rfc_handle,  cpOwn, cpRemote, cpComm, &cRepl, &iIgnoreErrors, &iPcs);
    printfU (cU("%d\n"), rfc_rc);
    
    if (rfc_rc)
    {
        rfc_error(cU("RfcGetCodePagesInfo"));
    }

    printfU (cU("\nOwn code page          :  %s"),   cpOwn);
    printfU (cU("\nRemote code page       :  %s"),   cpRemote);
    printfU (cU("\nCommunication code page:  %s"),   cpComm);
    printfU (cU("\nActual pcs             :  %d"),   iPcs);
    printfU (cU("\nReplacement character  :  0x%x"), (unsigned int)cRepl);
    printfU (cU("\nIgnore Errors          :  %d\n"), iIgnoreErrors);

    printfU (cU("\n<==  RfcGetAttachedContext    rc = "));    
    rfc_rc = RfcGetAttachedContext(rfc_handle,  &pContext, NULL);
    printfU (cU("%d\n"), rfc_rc);
    
    if (rfc_rc)
    {
        rfc_error(cU("RfcGetAttachedContext"));
    }

    if (NULL != pContext)
    {
        /*display the attached context
         * in our sample context this is just a string
         */
        pCtx = (SAP_CHAR*)pContext;
        printfU (cU("\nThis handle is used by :  %s\n"),   pCtx);
    }

    return;
}


/*--------------------------------------------------------------------*
 * A Sample for RFC Attach Context
 *
 * This sample context is just a malloced string
 * a normal usage for this API would be to pass a pointer to a class
 * and on call to the handler, to invalidate the handle contained
 * in the class
 *--------------------------------------------------------------------*/
void DLL_CALL_BACK_FUNCTION myOnCloseHandler
(
    RFC_HANDLE   hRfc,      /* @parm The handle being closed */
    unsigned int iReserved, /* @parm reserved. */
    void         *pContext  /* @parm The context installed by <f RfcAttachContext> */
)
{
    SAP_CHAR *pCtx = (SAP_CHAR*)pContext;
    
    if (NULL == pCtx)
        pCtx = cU("<out_of_mem_for_strdup");

    printfU(cU("OnClose: handle %d used by %s is being closed \n"), 
        hRfc,
        pCtx);
        
}/*myOnCloseHandler()*/

void DLL_CALL_BACK_FUNCTION myOnFreeHandler
(
    RFC_HANDLE   hRfc,      /* @parm The handle being closed */
    unsigned int iReserved, /* @parm reserved. */
    void         *pContext  /* @parm The context installed by <f RfcAttachContext> */
)
{
    SAP_CHAR *pCtx = (SAP_CHAR*)pContext;
    
    if (NULL == pCtx)
        pCtx = cU("<out_of_mem_for_strdup");

    printfU(cU("OnFree:  handle %d used by %s is being discarded\n"), 
        hRfc,
        pCtx);

    if (NULL != pContext)
        free(pContext);
        
}/*myOnFreeHandler()*/

/*--------------------------------------------------------------------*/
/* Prepare and Open a RFC-Connection                                  */
/*--------------------------------------------------------------------*/
int RFC_connect(const SAP_CHAR * callerName)
{
    printfU (cU("\n"));

    /* install error handler */
    new_env.allocate     = NULL;
    new_env.errorhandler = rfc_error;
    printfU (cU("\n<==  RfcEnvironment (Install Error Handling function)\n"));
    RfcEnvironment(&new_env);

    
#if defined(SAPonUNIX)
    gettimeofday(&sopenmsec, &tzp);
#elif defined(SAPonOS400)
    sopenmsec = time(NULL);
#else
    ftime(&sopenmsec);
#endif

    memsetU (connect_param, cU('\0'), sizeofU (connect_param));

    sprintfU (connect_param, cU("TYPE=%c"), sapsys[0]);

    if (rfc_trace[0] == cU('Y')) 
     strcpyU (connect_param+strlenU (connect_param), cU(" TRACE=1"));

    
    if ((sapsys[0] == RFC_SERVER_R2) || (sapsys[0] == RFC_SERVER_R3) || (sapsys[0] == RFC_SERVER_SHM))
    {
        if (memcmpU (client, cU("   "), 3) != 0)
        {
            sprintfU (connect_param+strlenU (connect_param), cU(" CLIENT=%s"), client);
        }
    
        sprintfU (connect_param+strlenU (connect_param), cU(" USER=%s PASSWD=%s"), userid, password);

        if (language[0] != cU(' '))
        {
            sprintfU (connect_param+strlenU (connect_param), cU(" LANG=%s"), language);
        }
    }

    
    if (action == 1)
    {
        sprintfU (connect_param+strlenU (connect_param), cU(" LCHECK=0"));
    }


    if (saplogon[0] == cU('Y'))
    {
        /******************/
        /* Using saplogon */
        /******************/
        if (sapsys[0] == RFC_SERVER_R2)
        {
            sprintfU (connect_param+strlenU (connect_param), cU(" SAPLOGON_ID=%s, DEST=%s"), saplogon_id, rfc_dest);
        }
        else
        {
            sprintfU (connect_param+strlenU (connect_param), cU(" SAPLOGON_ID=%s"), saplogon_id);
        }
    }
    else
    {
        if (saprfc_ini[0] == cU('Y'))
        {
            /*****************************/
            /* Working with 'saprfc.ini' */
            /*****************************/
            sprintfU (connect_param+strlenU (connect_param), cU(" DEST=%s"), rfc_dest);
        }
        else
        {
            /*****************************/
            /* Without 'saprfc.ini'      */
            /*****************************/
            if (sapsys[0] == RFC_SERVER_R2)
            {
                /*=============================*/
                /* RFC server is an R/2 system */
                /*=============================*/
                sprintfU (connect_param+strlenU (connect_param), cU(" DEST=%s GWHOST=%s GWSERV=%s"), rfc_dest, gwhost, gwserv);
            }
            else if (sapsys[0] == RFC_SERVER_EXT)
            {
                /*===================================*/
                /* RFC server is an external program */
                /*===================================*/
                if (register_mode[0] == cU('Y'))
                {
                    /*=========================================*/
                    /* RFC server is registered at SAP gateway */
                    /*=========================================*/
                    sprintfU (connect_param+strlenU (connect_param), cU(" GWHOST=%s GWSERV=%s TPNAME=%s"), gwhost, gwserv, tpid);
                }
                else
                {
                    /*===========================================*/
                    /* RFC server will be started by SAP gateway */
                    /*===========================================*/
                    sprintfU (connect_param+strlenU (connect_param), cU(" GWHOST=%s GWSERV=%s TPHOST=%s TPNAME=%s"), 
                              gwhost, gwserv, tphost, tpname);
                }

                if (sncmode[0] == cU('Y'))
                {
                    sprintfU (connect_param+strlenU (connect_param), cU(" SNC_MODE=1 SNC_LIB=%s SNC_PARTNERNAME=%s"),
                              snc_lib, snc_partnername);
                }
            }
            else
            {
                /*=============================*/
                /* RFC server is an R/3 system */
                /*=============================*/
                if (load_balancing[0] == cU('Y'))
                {
                    /*-----------------------------------------*/
                    /* Using Load Balancing for connect to R/3 */
                    /*-----------------------------------------*/
                    if ((strlenU (mshost) == 1) && (mshost[0] == cU('0')))
                    {
                        /* Use sapmsg.ini of SAPLOGON */
                        sprintfU (connect_param+strlenU (connect_param), cU(" R3NAME=%s GROUP=%s"), r3name, group);
                    }
                    else
                    {
                        sprintfU (connect_param+strlenU (connect_param), cU(" MSHOST=%s R3NAME=%s GROUP=%s"),
                                  mshost, r3name, group);
                    }
                }
                else if (sapsys[0] == RFC_SERVER_SHM)
                {
                        sprintfU (connect_param+strlenU (connect_param), cU(" PROFILE_NAME=%s"), profile_name);
                }
                else
                {
                    /*------------------------------------------*/
                    /* Connect to a specific application server */
                    /*------------------------------------------*/
                    sprintfU (connect_param+strlenU (connect_param), cU(" ASHOST=%s SYSNR=%s"), ashost, sysnr);
                }

                if (sncmode[0] == cU('Y'))
                {
                    sprintfU (connect_param+strlenU (connect_param), cU(" SNC_MODE=1 SNC_LIB=%s SNC_PARTNERNAME=%s"),
                              snc_lib, snc_partnername);
                }
            }
        }
    }

    
    if ((gui_at_open[0] == cU('Y')) && (sapsys[0] == RFC_SERVER_R3) && 
        ((saprfc_ini[0] != cU('Y')) || (saplogon[0] == cU('Y'))))
    {
        if (abap_debug[0] == cU('Y'))
        {
            strcpyU (connect_param+strlenU (connect_param), cU(" ABAP_DEBUG=1"));
        }
        else
        {
            if (abap_debug_ext[0] == cU('Y'))
            {
                strcpyU (connect_param+strlenU (connect_param), cU(" ABAP_DEBUG_EXT=1"));
            }


            if (use_sapgui[0] == cU('N'))
            {
                strcpyU (connect_param+strlenU (connect_param), cU(" USE_SAPGUI=0"));
            }
            else if (invisible[0] == cU('N'))
            {
                strcpyU (connect_param+strlenU (connect_param), cU(" USE_SAPGUI=1"));
            }
            else
            {
                strcpyU (connect_param+strlenU (connect_param), cU(" USE_SAPGUI=2"));
            }
        }
    }


#ifdef SAPwithUNICODE
    sprintfU (connect_param+strlenU (connect_param), cU( " PCS=%c"), pcs);
#endif

    
    printfU (cU("\n<==  RfcOpenEx                rfc_handle = "));

    rfc_handle = RfcOpenEx (connect_param, &error_info);
    
    printfU (cU("%u\n"), rfc_handle);

    
    if (rfc_handle == RFC_HANDLE_NULL)
    {
        printfU (cU("\nGroup       Error group %d\n"), error_info.group);
        
        printfU (cU("Key         %s\n"), error_info.key);
    
        printfU (cU("Message     %s\n\n"), error_info.message);
    
        exit(1);
    }


#if defined(SAPonUNIX)
    gettimeofday(&eopenmsec, &tzp);
#elif defined(SAPonOS400)
    eopenmsec = time(NULL);
#else
    ftime(&eopenmsec);
#endif


    /* A sample instalation from a context */
    rfc_rc = RfcAttachContext (rfc_handle,  myOnCloseHandler, myOnFreeHandler, strdupU (callerName));


    /* Get specific info of this RFC connection if server is R/2 */
    /* If R/3, only if with Logon check                          */
    if ((sapsys[0] == RFC_SERVER_R2) || ((sapsys[0] == RFC_SERVER_R3) && (action != 1)))
    {
        get_attributes(rfc_handle);
    }

    
    return 0;
}


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
/* Error Cleanup because of a RFC-Error                               */
/*--------------------------------------------------------------------*/
void DLL_CALL_BACK_FUNCTION rfc_error (rfc_char_t *operation)
{
  printfU (cU("\n<==  RfcLastErrorEx\n"));

  RfcLastErrorEx(&error_info);

  printfU (cU("\nRFC Call/Exception:  %s\n"), operation);
  printfU (cU("Group       Error group %d\n"), error_info.group);
  printfU (cU("Key         %s\n"), error_info.key);
  printfU (cU("Message     %s\n\n"), error_info.message);

  exit(1);
}


/*--------------------------------------------------------------------*/
/* Display RFC-Parameter                                              */
/*--------------------------------------------------------------------*/
void display (rfc_char_t          *arrow,
              RFC_PARAMETER       *rfc_param)
{
  rfc_char_t         s[256];
  RFC_PARAMETER     *p = rfc_param;

  i = 1;
  while (p->name != NULL)
  {
    printfU (cU("\n%s %d:"), arrow, i);
    printfU (cU(" %-08s"), p->name);
    printfU (cU("    length = %-5u"), p->leng);
    if (p->addr != NULL && p->leng > 0)
    {
      switch(p->type)
      {
        case TYPC:
          printfU (cU("  type = TYPC"));
          memsetU (s, cU('\0'), sizeofU(s));
          memcpyU ((SAP_UC*) s, (SAP_UC*) p->addr, p->leng/sizeofR(SAP_UC));
          printfU (cU("\n      value = "));
          printfU (cU("'%s'\n"),s);
          break;

        case TYPINT:
          printfU (cU("  type = TYPINT"));
          memcpyR (&j, p->addr, sizeofR(int));
          printfU (cU("\n      value = "));
          printfU (cU("%d\n"), j );
          break;

        default:
          break;
      }
    }
    p++; 
    i++;
  }
  return;
}



void display_rfcsi( RFCSI * rfcsi )
{
#define OUT( text, value ) \
    printfU ( cU("%-22s %-.*s\n"), text, (int) sizeofU(rfcsi->value), rfcsi->value )
#define OUTX( text, value, add ) \
    printfU ( cU("%-22s %-.*s %s\n"), text, (int) sizeofU(rfcsi->value), \
             rfcsi->value, (add) )
#define NLINE printfU ( cU("\n") )
#define HEADER( text ) \
   printfU ( cU("%s\n-----------------------------------------------\n\n"), \
           (text) )

    NLINE;
    NLINE;
    HEADER( cU("SAP System Information") );
    NLINE;
    OUT( cU("Destination"), rfcdest );
    NLINE;
    if (rfcsi->rfchost2[0] != cU(' '))
      OUT( cU("Host"), rfchost2 );
    else
      OUT( cU("Host"), rfchost );
    OUT( cU("System ID"), rfcsysid );
    OUT( cU("Database"), rfcdatabs );
    OUT( cU("DB host"), rfcdbhost );
    OUT( cU("DB system"), rfcdbsys );
    NLINE;
    OUT( cU("SAP release"), rfcsaprl );
    OUT( cU("SAP kernel release"), rfckernrl );
    NLINE;
    OUT( cU("RFC Protokoll"), rfcproto );
    OUT( cU("Characters"), rfcchartyp );
    OUT( cU("Integers "), rfcinttyp );
    OUT( cU("Floating P."), rfcflotyp );
    OUT( cU("SAP machine id"), rfcmach );
    NLINE;
    OUTX( cU("Timezone"), rfctzone,
          rfcsi->rfcdayst[0] != cU(' ') ? cU("(Daylight saving time)") : cU("") );

    NLINE;
    NLINE;
} /* display_rfcsi */


/*--------------------------------------------------------------------*/
/* Search for certain parameter                                       */
/*--------------------------------------------------------------------*/
int PM_search ( rfc_char_t *param,
                rfc_char_t *text,
                int         leng)
{
  rfc_char_t  *pa,
              *pe;

  memsetU (param, cU('\0'), leng);
  
  pa = strstrU (sbuf, text);
  if (pa == NULL)
    return 99;

  pa = pa + strlenU (text);

  if (*pa == cU('"'))
  {
    pe = memchrU (pa+1, cU('"'), leng);
    if (pe == NULL)
      znr = &sbuf[strlenU (sbuf)] - pa;
    else
      znr = pe - pa + 1;
  }
  else
  {
    pe = memchrU (pa, cU(' '), leng);
    if (pe == NULL)
      znr = &sbuf[strlenU (sbuf)] - pa;
    else
      znr = pe - pa;
  }

  if (znr == 0)
    return 99;

  if (znr > leng)
    znr = leng;

  memcpyU (param, pa, znr);

  return 0;
}


/*--------------------------------------------------------------------*/
/* Get parameters for CONNECTION test                                 */
/*--------------------------------------------------------------------*/
void get_parameter_for_conn_test()
{

  /* Open the work file */
  work_fp = fopenU (work_file, cU("r"));
  if (work_fp == NULL)
  {
    get_connect_data_via_dialog();
    wbuf[strlenU (wbuf)] = cU('\n');
    strcpyU (wbuf+strlenU (wbuf), sbuf2);
    save_test_parameter();
  }
  else
  {
    get_connect_data_from_file();
    if (work_fp != NULL)
    {
      fclose(work_fp);
      work_fp = NULL;
    }
  }
  return;
}


/*--------------------------------------------------------------------*/
/* Get parameters for PERFORMANCE test                                */
/*--------------------------------------------------------------------*/
void get_parameter_for_load_test()
{
  memsetU (lgit0332_field, cU('\0'), 5);
  memsetU (lgit1000_field, cU('\0'), 5);
  memsetU (lget0332_field, cU('\0'), 5);
  memsetU (lget1000_field, cU('\0'), 5);

  /* Open the work file */
  work_fp = fopenU (work_file, cU("r"));
  if (work_fp == NULL)
  {
    get_connect_data_via_dialog();
    get_load_test_data_via_dialog();
    save_test_parameter();
  }
  else
  {
    get_connect_data_from_file();
    get_load_test_data_from_file();
    if (work_fp != NULL)
    {
      fclose(work_fp);
      work_fp = NULL;
    }
  }

  return;
}


/*--------------------------------------------------------------------*/
/* Get parameters for SAPGUI test                                     */
/*--------------------------------------------------------------------*/
void get_parameter_for_diag_test()
{

  /* Open the work file */
  work_fp = fopenU (work_file, cU("r"));
  if (work_fp == NULL)
  {
    get_connect_data_via_dialog();
    get_diag_test_data_via_dialog();
    save_test_parameter();
  }
  else
  {
    get_connect_data_from_file();
    get_diag_test_data_from_file();
    if (work_fp != NULL)
    {
      fclose(work_fp);
      work_fp = NULL;
    }
  }

  return;
}


/*--------------------------------------------------------------------*/
/* Get parameters for Callback test                                   */
/*--------------------------------------------------------------------*/
void get_parameter_for_back_test()
{

  /* Open the work file */
  work_fp = fopenU (work_file, cU("r"));
  if (work_fp == NULL)
  {
    get_connect_data_via_dialog();
    get_back_test_data_via_dialog();
    save_test_parameter();
  }
  else
  {
    get_connect_data_from_file();
    get_back_test_data_from_file();
    if (work_fp != NULL)
    {
      fclose(work_fp);
      work_fp = NULL;
    }
  }

  return;
}


/*--------------------------------------------------------------------*/
/* Get parameters for Start/Connect Reg. Server test                  */
/*--------------------------------------------------------------------*/
void get_parameter_for_sreg_test()
{

  /* Open the work file */
  work_fp = fopenU (work_file, cU("r"));
  if (work_fp == NULL)
  {
    get_connect_data_via_dialog();
    get_sreg_test_data_via_dialog();
    save_test_parameter();
  }
  else
  {
    get_connect_data_from_file();
    get_sreg_test_data_from_file();
    if (work_fp != NULL)
    {
      fclose(work_fp);
      work_fp = NULL;
    }
  }

  return;
}


/*--------------------------------------------------------------------*/
/* Get connect data from file                                         */
/*--------------------------------------------------------------------*/
void get_connect_data_from_file()
{
  if (read_record(work_fp, sbuf1) == NULL)
  {
    printfU (cU("\nCannot read data file\n"));
    exit(1);
  }
  strcpyU (sbuf, sbuf1);
  read_record(work_fp, sbuf2);

  get_connection_parameters_from_file();

  if ((sapsys[0] == RFC_SERVER_R2) || (sapsys[0] == RFC_SERVER_R3) || (sapsys[0] == RFC_SERVER_SHM))
  {
    printfU (cU("\n\n               SAP LOGON DATA:"));

    /* Client */
    memsetU (client, cU('\0'), 3);
    rc = PM_search(client, cU("CLIENT="), 3);
    if (rc)
      strcpyU (client, cU("   "));
    printfU (cU("\n                 Client..................................: %s"), client);

    /* User Id */
    memsetU (userid, cU('\0'), 12);
    rc = PM_search(userid, cU("USERID="), 12);
    if (rc)
    {
      printfU (cU("\n\nInfo about User Id (USERID=) not found or not correct"));
      exit(1);
    }
    printfU (cU("\n                 UserID..................................: %s"), userid);

    /* Password */
    memsetU (password, cU('\0'), 9);
    rc = PM_search(password, cU("PASSWORD="), 8);
    if (rc)
    {
      printfU (cU("\n\nInfo about Password (PASSWORD=) not found or not correct"));
      exit(1);
    }
    printfU (cU("\n                 Password................................: XXXXXXXX"));
    znr = strlenU (password);
    for (i = 0; i < znr; i++)
      password[i] =  (password[i] + 30 + i);

    /* Language */
    memsetU (language, cU('\0'), 3);
    rc = PM_search (language, cU("LANGUAGE="), 2);
    if (rc)
      strcpyU (language, cU("E"));
    printfU (cU("\n                 Language (E)............................: %s"), language);
  }

  /* Number of Calls of this RFC-function */
  memsetU (nrcall_field, 0, 5);
  rc = PM_search (nrcall_field, cU("NRCALL="), 5);
  if (rc)
    strcpyU (nrcall_field, cU("00001"));

  nrcall = atoiU (nrcall_field);
  sprintfU (nrcall_field, cU("%05d"), nrcall);
  printfU (cU("\n\n               #Calls of this RFC-function...............: %s"), nrcall_field);

  /* Close after each RFC-call */
  if (nrcall <= 1)
    use_close[0] = cU('N');
  else
  {
    if ((sapsys[0] == RFC_SERVER_EXT) && (register_mode[0] == cU('Y')))
      use_close[0] = cU('Y');
    else
    {
      memsetU (use_close, cU(' '), 1);
      rc = PM_search(use_close, cU("CLOSE="), 1);
      if (rc)
        use_close[0] = cU('N');
    }
    printfU (cU("\n               Close connection after each call (Y/N)N...: %c"), use_close[0]);
  }

  /* Last confirm for test */
  if ((action == 1) || (action == 2) || (action == 5) || (action == 12) || (action == 13) || (action == 14))
  {
    printfU (cU("\n\n"));
    do
    {
      printfU (cU("            Do You Want To Test With These Parameters (Y/N)..: "));
      fflush(stdout);
      getsU (ips);
      ips[0] =  toupperU(ips[0]);
    } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));

    if ((abap_debug[0] == cU('Y')) && (strcmpU (userid, cU("SAPCPIC")) == 0))
    {
      printfU (cU("\n\nNo access for ABAP debugging as SAPCPIC (It's a CPIC-User)!!!"));
      printfU (cU("\n\nHit return key to continue\n"));
      fflush (stdout);
      getcharU ();
      ips[0] = cU('N');
    }

    if (ips[0] == cU('N'))
    {
      if (work_fp != NULL)
      {
        fclose(work_fp);
        work_fp = NULL;
      }
      get_connect_data_via_dialog();
      save_test_parameter();
    }
  }

  return;
}


/*--------------------------------------------------------------------*/
/* Get password for SAP logon                                         */
/*--------------------------------------------------------------------*/
void get_passwd(rfc_char_t * pwd)
{
#if defined(SAPonUNIX)
  int        i, c;
  rfc_char_t ibuf[256];

  systemU (cU("stty -echo"));
  getsU (ibuf);
  systemU (cU("stty echo"));

  strcpyU (pwd, ibuf);
  i = strlenU (pwd);
  if (i > 40)
    i = 40;
  pwd[i] = 0;
  memcpyU (ibuf, cU("****************************************"), i);
  printfU (cU("%s\n"), ibuf);

#elif defined(SAPonNT)
#include <conio.h>
  int           i, c;
  rfc_char_t    ibuf[256];

  for (i=0; i<8; i++)
  {
    c = getch();
    if (c != cU('\r'))
    {
      pwd[i] =  c;
      printfU (cU("*"));
    }
    else
    {
      pwd[i] = 0;
      break;
    }
  }
  if (c != cU('\r'))
    getsU (ibuf);
  else
    printfU (cU("\n"));

#else
  getsU (pwd);

#endif

  return;
}


/*--------------------------------------------------------------------*/
/* Get connect data via dialog                                        */
/*--------------------------------------------------------------------*/
void get_connect_data_via_dialog()
{
  do
  {
    memsetU (wbuf, cU('\0'), 512);

    get_connection_parameters_via_dialog();

    if ((sapsys[0] == RFC_SERVER_R2) || 
        (sapsys[0] == RFC_SERVER_R3) || 
        (sapsys[0] == RFC_SERVER_SHM))
    {
      printfU (cU("\n               SAP LOGON DATA:\n"));

      /* Client */
      if (strlenU (client) == 0)
        strcpyU (client, cU("000"));

      printfU (cU("                 Client..................................: (%s): "), client);
      fflush (stdout);
      getsU (ips);
      if (strlenU (ips) == 0)
        strcpyU (ips,  client);
      
      ips[3] = cU('\0');
      
      strcpyU (client, ips);
      
      if (client[0] == cU(' '))
        strcpyU (client, cU("   "));
      if (memcmpU (client, cU("   "), 3) != 0)
        sprintfU (wbuf+strlenU (wbuf), cU("CLIENT=%s "), client);

      /* User Id */
      if (strlenU (userid) == 0)
        strcpyU (userid, cU("SAPCPIC"));

      printfU (cU("                 UserID..................................: (%s): "), userid);
      fflush (stdout);
      
      getsU (ips);
      
      if (strlenU (ips) == 0)
        strcpyU (ips,  userid);
      
      ips[12] = cU('\0');
      
      strcpyU (userid, ips);
      
      znr = strlenU (ips);
      
      for (i = 0; i < znr; i++)
        userid[i] =  toupperU(userid[i]);
      
      sprintfU (wbuf+strlenU (wbuf), cU("USERID=%s "), userid);

      /* Password */
      if (strlenU (password) == 0)
        strcpyU (password, cU("admin"));

      printfU (cU("                 Password................................: (XXXXXXXX):"));
      
      fflush(stdout);
      
      get_passwd(ips);
      
      if (strlenU (ips) == 0)
        strcpyU (ips,  password);
      
      ips[8] = cU('\0');
      
      strcpyU (password, ips);
      
      znr = strlenU (password);
      
      for (i = 0; i < znr; i++)
        password[i] =  (toupperU(password[i]) - 30 - i);
      
      sprintfU (wbuf+strlenU (wbuf), cU("PASSWORD=%s "), password);
      
      for (i = 0; i < znr; i++)
        password[i] =  (password[i] + 30 + i);

      /* Language */
      if (strlenU (language) == 0)
        strcpyU (language, cU("E"));

      printfU (cU("                 Language (E)............................: (%s): "), language);

      fflush(stdout);
      
      getsU(ips);
      
      if (strlenU (ips) == 0)
        strcpyU (ips,  language);
      
      ips[2] = cU('\0');
      
      strcpyU (language, ips);
      
      if (language[0] != cU(' '))
        sprintfU (wbuf+strlenU (wbuf), cU("LANGUAGE=%s "), language);
    }

    /* Number of Calls of this RFC-function */
    printfU (cU("\n"));
    if (strlenU (nrcall_field) == 0)
      strcpyU (nrcall_field, cU("00001"));
    
    printfU (cU("               #Calls of this RFC-function...............: (%s): "), nrcall_field);

    fflush(stdout);
    
    getsU (ips);
    
    if (strlenU (ips) == 0)
      strcpyU (ips,  nrcall_field);
    
    ips[5] = cU('\0');
    
    strcpyU (nrcall_field, ips);
    
    nrcall = atoiU (nrcall_field);
    
    sprintfU (nrcall_field, cU("%05d"), nrcall);

    sprintfU (wbuf+strlenU (wbuf), cU("NRCALL=%s "), nrcall_field);

    /* Close after each RFC call */
    if (nrcall <= 1)
      use_close[0] = cU('N');
    else
    {
      if ((sapsys[0] == RFC_SERVER_EXT) && (register_mode[0] == cU('Y')))
      {
        use_close[0] = cU('Y');

        printfU (cU("               Close connection after each call (Y/N)N...: (%c): \n"), use_close[0]);
      }
      else
      {
        if (strlenU (use_close) == 0)
          use_close[0] = cU('N');
        
        do
        {
          printfU (cU("               Close connection after each call (Y/N)N...: (%c): "), use_close[0]);
        
          fflush(stdout);
          
          getsU (ips);

          if (strlenU (ips) == 0)
            ips[0] = use_close[0];
          
          ips[0] =  toupperU(ips[0]);
        } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
        use_close[0] = ips[0];
      }
    }
    sprintfU (wbuf+strlenU (wbuf), cU("CLOSE=%c \n"), use_close[0]);

    /* Last confirm for test */
    if ((action == 1) || (action == 2) || (action == 5) || (action == 12))
    {
      printfU (cU("\n"));
      do
      {
            printfU (cU("            Do You Want To Test With These Parameters (Y/N)..: "));

            fflush(stdout);
            
            getsU(ips);

            ips[0] =  toupperU(ips[0]);
      } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
    }
    else
      ips[0] = cU('Y');

    if ((abap_debug[0] == cU('Y')) && (strcmpU (userid, cU("SAPCPIC")) == 0))
    {
      printfU (cU("\n\nNo access for ABAP debugging as SAPCPIC (It's a CPIC-User)!!!"));
      printfU (cU("\n\nHit return key to continue\n"));

      fflush (stdout);
      
      getcharU ();
      
      ips[0] = cU('N');
    }
  } while (ips[0] != cU('Y'));

  return;
}


/*--------------------------------------------------------------------*/
/* Get connection parameters from file                                */
/*--------------------------------------------------------------------*/
void get_connection_parameters_from_file()
{
  printfU (cU("\n\n               CONNECTION PARAMETERS:"));

  /* Type of RFC server: R/2 or R/3 or External */
  sapsys[0] = cU(' ');

  rc = PM_search (sapsys, cU("SAPSYS="), 1);
  
  if (rc)
    sapsys[0] = RFC_SERVER_R3;
 
  printfU (cU("\n                 Server is R/2, R/3 or External (2/3/E/F)3.: %c"), sapsys[0]);

  /* If External, using register mode ? */
  saplogon[0] = cU('N');

  if (sapsys[0] == cU('E'))
  {
    memsetU (register_mode, 0, 1);
    
    rc = PM_search(register_mode, cU("REGMOD="), 1);
    
    if (rc)
      register_mode[0] = cU('Y');

    printfU (cU("\n                 Using registering feature (Y/N)Y........: %c"), register_mode[0]);
  }
  else if (sapsys[0] == cU('F'))
  {
    memsetU (profile_name, cU('\0'), sizeofU (profile_name));

    rc = PM_search (profile_name, cU("PROFILE_NAME="), sizeofU (profile_name) - 1);

    if (rc)
     strcpyU (profile_name, prf_name);

     printfU (cU("\n                 Profile file name ...........: (%s): "), profile_name);
  }
  else 
  {
    /* Using data of saplogon */
    saplogon[0] = cU(' ');

    rc = PM_search (saplogon, cU("SAPLOGON="), 1);
    
    if (rc)
      saplogon[0] = cU('N');

    printfU (cU("\n                 Using data of saplogon (Y/N)Y...........: %c"), saplogon[0]);
  }

  /* Using saplogon */
  if (saplogon[0] == cU('Y'))
  {
    /* ID in saplogon */
    memsetU (saplogon_id, cU('\0'), 256);

    rc = PM_search (saplogon_id, cU("SAPLOGON_ID="), 256);
    
    if (rc)
    {
      printfU (cU("\n\nInfo about saplogon ID (SAPLOGON_ID=) not found or not correct"));
      exit(1);
    }

    printfU (cU("\n                 ID in saplogon..........................: %s"), saplogon_id);

    /* If R/2, destination in sideinfo for gateway */
    if (sapsys[0] == RFC_SERVER_R2)
    {
      memsetU (rfc_dest, cU('\0'), 32);

      rc = PM_search (rfc_dest, cU("DEST="), 32);

      if (rc)
      {
        printfU (cU("\n\nInfo about destination (DEST=) not found or not correct"));
        exit(1);
      }
      printfU (cU("\n                 DEST in 'sideinfo' for the SAP gateway..: %s"), rfc_dest);
    }
  }
  else
  {
    /* Working with 'saprfc.ini' */
    saprfc_ini[0] = cU(' ');

    rc = PM_search (saprfc_ini, cU("RFCINI="), 1);
    
    if (rc)
      saprfc_ini[0] = cU('N');

    printfU (cU("\n                 Working with 'saprfc.ini' (Y/N)N........: %c"), saprfc_ini[0]);

    if (saprfc_ini[0] == cU('Y'))
    {
      /* Destination in 'saprfc.ini' */
      memsetU (rfc_dest, cU('\0'), 32);

      rc = PM_search(rfc_dest, cU("DEST="), 32);
      if (rc)
      {
        printfU (cU("\n\nInfo about destination (DEST=) not found or not correct"));
        exit(1);
      }

      printfU (cU("\n                 Destination in 'saprfc.ini'.............: %s"), rfc_dest);
    }
    else
    {
      if (sapsys[0] == RFC_SERVER_R2)
      {
        /* Destination in sideinfo */
        memsetU (rfc_dest, cU('\0'), 32);

        rc = PM_search(rfc_dest, cU("DEST="), 32);
        if (rc)
        {
          printfU (cU("\n\nInfo about destination (DEST=) not found or not correct"));
          exit(1);
        }
        printfU (cU("\n                 DEST in 'sideinfo' for the SAP gateway..: %s"), rfc_dest);

        memsetU (gwhost, cU('\0'), 100);

        rc = PM_search (gwhost, cU("GWHOST="), 100);
        if (rc)
        {
          printfU (cU("\n\nInfo about gateway host name (GWHOST=) not found or not correct"));
          exit(1);
        }
        printfU (cU("\n                 Host name of the SAP gateway to R/2.....: %s"), gwhost);

        memsetU (gwserv, cU('\0'), 7);

        rc = PM_search(gwserv, cU("GWSERV="), 7);
        if (rc)
        {
          printfU (cU("\n\nInfo about gateway service (GWSERV=) not found or not correct"));
          exit(1);
        }

        printfU (cU("\n                 Service of the specified SAP gateway....: %s"), gwserv);
      }
      else if (sapsys[0] == RFC_SERVER_EXT)
      {
        memsetU (gwhost, cU('\0'), 100);

        rc = PM_search(gwhost, cU("GWHOST="), 100);
        if (rc)
        {
          printfU (cU("\n\nInfo about gateway host name (GWHOST=) not found or not correct"));
          exit(1);
        }

        printfU (cU("\n                 Host name of the SAP gateway............: %s"), gwhost);

        memsetU (gwserv, cU('\0'), 7);

        rc = PM_search(gwserv, cU("GWSERV="), 7);
        if (rc)
        {
          printfU (cU("\n\nInfo about gateway service (GWSERV=) not found or not correct"));
          exit(1);
        }

        printfU (cU("\n                 Service of the specified SAP gateway....: %s"), gwserv);

        if (register_mode[0] == cU('Y'))
        {
          memsetU (tpid, cU('\0'), 100);

          rc = PM_search (tpid, cU("TPID="), 100);
          if (rc)
            strcpyU (tpid,  stde_tpid);

          printfU (cU("\n                 Program ID of registered partner program: %s"), tpid);
        }
        else
        {
          memsetU (tphost, cU('\0'), 100);

          rc = PM_search (tphost, cU("TPHOST="), 100);
          if (rc)
            strcpyU (tphost, stde_tphost);

          printfU (cU("\n                 Host name of the partner program........: %s"), tphost);

          memsetU (tpname, cU('\0'), 100);

          rc = PM_search(tpname, cU("TPNAME="), 100);
          if (rc)
            strcpyU (tpname, stde_tpname);

          printfU (cU("\n                 Fully qualified name of partner program.: %s"), tpname);
        }
      }
      else if (sapsys[0] != RFC_SERVER_SHM)
      {
        /* Use load balancing for connect to R/3 */	
        memsetU (load_balancing, cU(' '), 1);

        rc = PM_search (load_balancing, cU("BALANCE="), 1);
        
        if (rc)
          load_balancing[0] = cU('Y');

        printfU (cU("\n                 Use load balancing (Y/N)Y...............: %c"), load_balancing[0]);

        if (load_balancing[0] == cU('Y'))
        {
          /* R/3 name */
          memsetU (r3name, cU('\0'), 100);
          
          rc = PM_search (r3name, cU("R3NAME="), 100);

          if (rc)
          {
            printfU (cU("\n\nInfo about R/3 name (R3NAME=) not found or not correct"));
            exit(1);
          }
          printfU (cU("\n                 R/3 system name.........................: %s"), r3name);

          /* R/3 message server */
          memsetU (mshost, 0, 100);

          rc = PM_search(mshost, cU("MSHOST="), 100);
          
          if (rc)
            strcpyU (mshost, cU("0"));
          
          printfU (cU("\n                 Message server or 0 for using sapmsg.ini: %s"), mshost);

          /* R/3 selected group */
          memsetU (group, cU('\0'), 32);

          rc = PM_search(group, cU("GROUP="), 32);
          if (rc)
           strcpyU (group, cU("PUBLIC"));

          printfU (cU("\n                 Selected group..........................: %s"), group);
        }
        else
        {
          /* R/3 Application server */
          memsetU (ashost, cU('\0'), 100);

          rc = PM_search (ashost, cU("ASHOST="), 100);

          if (rc)
          {
            printfU (cU("\n\nInfo about application server (ASHOST=) not found or not correct"));
            exit(1);
          }
          printfU (cU("\n                 Host name of an application server......: %s"), ashost);

          /* R/3 System number */
          memsetU (sysnr, cU('\0'), 2);

          rc = PM_search (sysnr, cU("SYSNR="), 2);
          if (rc)
          {
            printfU (cU("\n\nInfo about system number (SYSNR=) not found or not correct"));
            exit(1);
          }
          printfU (cU("\n                 System number...........................: %s"), sysnr);
        }
      }
    }

    if ((sapsys[0] == RFC_SERVER_R3) || (sapsys[0] == RFC_SERVER_EXT))
    {
      /* Working with SNC */
      memsetU (sncmode, cU(' '), 1);

      rc = PM_search(sncmode, cU("SNC_MODE="), 1);
      
      if (rc)
        sncmode[0] = cU('N');

      printfU (cU("\n                 Working with SNC (Y/N)N.................: %c"), sncmode[0]);

      if (sncmode[0] == cU('Y'))
      {
        memsetU (snc_lib, cU(' '), 256);
        rc = PM_search (snc_lib, cU("SNC_LIB="), 256);
        if (rc)
        {
          ptr = getenvU (cU("SNC_LIB"));
          if (ptr != NULL)
          {
            ptr[256] = cU('\0');
            strcpyU (snc_lib, ptr);
          }
          else
           strcpyU (snc_lib, std_snc_lib);
        }

        printfU (cU("\n                 SNC Library Name........................: %s"), snc_lib);

        memsetU (snc_partnername, cU(' '), 256);

        rc = PM_search(snc_partnername, cU("SNC_PARTNERNAME="), 256);
        
        if (rc)
         strcpyU (snc_partnername, std_snc_partnername);
        
        printfU (cU("\n                 SNC Name of the partner program.........: %s"), snc_partnername);
      }
    }
  }

#ifdef SAPwithUNICODE
  pcs = cU ('1');

  rc = PM_search (&pcs, cU("PCS="), 1);

  if (rc)
   pcs = cU('1');

  printfU (cU("\n                 PCS  value..............................: %c"), pcs);
#endif

  printfU (cU("\n\n               RFC-SPECIFIC PARAMETERS:"));

  if ((sapsys[0] == RFC_SERVER_R3) &&
      ((saplogon[0] == cU('Y')) || (saprfc_ini[0] == cU('N'))))
  {
    /* Using ABAP debugger */
    memsetU (abap_debug, cU(' '), 1);

    rc = PM_search (abap_debug, cU("DEBUG="), 1);
    if (rc)
      abap_debug[0] = cU('N');
    
    printfU (cU("\n                 Working with ABAP debugger (Y/N)N.......: %c"), abap_debug[0]);
      
      
    /* Using ext ABAP debugger */
    if (abap_debug[0] == cU('N'))
    {
        memsetU (abap_debug_ext, cU(' '), 1);

        rc = PM_search (abap_debug_ext, cU("DEBUG_EXT="), 1);
        if (rc)
        abap_debug_ext[0] = cU('N');
        
        printfU (cU("\n                 Working with ext ABAP debugger (Y/N)N...: %c"), abap_debug_ext[0]);
    }
      
    /* Using SAPGUI */
    if (abap_debug[0] == cU('Y'))
    {
      use_sapgui[0] = cU('Y');
      printfU (cU("\n                 Use SAPGUI (Y/N)N.......................: %c"), use_sapgui[0]);
      invisible[0] = cU('N');
      printfU (cU("\n                 Automatically invisible SAPGUI (Y/N)N...: %c"), invisible[0]);
    }
    else
    {
      if (action == 4)
      {
        use_sapgui[0] = cU('Y');
        printfU (cU("\n                 Use SAPGUI (Y/N)N.......................: %c"), use_sapgui[0]);
      }
      else
      {
        memsetU (use_sapgui, cU(' '), 1);

        rc = PM_search (use_sapgui, cU("SAPGUI="), 1);
        if (rc)
          use_sapgui[0] = cU('N');

        printfU (cU("\n                 Use SAPGUI (Y/N)N.......................: %c"), use_sapgui[0]);
      }

      if (use_sapgui[0] == cU('Y'))
      {
        memsetU (invisible, cU(' '), 1);

        rc = PM_search(invisible, cU("INVISIBLE="), 1);
        if (rc)
          invisible[0] = cU('N');
        
        printfU (cU("\n                 Automatically invisible SAPGUI (Y/N)N...: %c"), invisible[0]);
      }
      else
        invisible[0] = cU('N');
    }

    if (use_sapgui[0] == cU('Y'))
    {
      memsetU (gui_at_open, cU(' '), 1);

      rc = PM_search (gui_at_open, cU("GUIATOPEN="), 1);
      if (rc)
        gui_at_open[0] = cU('Y');

      printfU (cU("\n                 Start SAPGUI at connection open (Y/N)Y..: %c"), gui_at_open[0]);
    }
    else
      gui_at_open[0] = cU('Y');
  }

  /* RFC-Trace */
  if ((saplogon[0] == cU('Y')) || (saprfc_ini[0] == cU('N')))
  {
    memsetU (rfc_trace, cU(' '), 1);

    rc = PM_search (rfc_trace, cU("TRACE="), 1);
    if (rc)
      rfc_trace[0] = cU('N');

    printfU (cU("\n                 RFC-Trace (Y/N)N........................: %c"), rfc_trace[0]);
  }
  return;
}


/*--------------------------------------------------------------------*/
/* Get connection parameters via dialog                               */
/*--------------------------------------------------------------------*/
void get_connection_parameters_via_dialog()
{
  printfU (cU("\n\n               CONNECTION PARAMETERS:\n"));

  /* Type of RFC server: R/2 or R/3 or External */
  if (strlenU (sapsys) == 0)
    sapsys[0] = RFC_SERVER_R3;

  do
  {
    printfU (cU("                 Server is R/2, R/3 or External (2/3/E/F)3.: (%c): "), sapsys[0]);

    fflush(stdout);
    
    getsU (ips);

    if (strlenU (ips) == 0)
      ips[0] = sapsys[0];
    
    ips[0] =  toupperU(ips[0]);
  } while ((ips[0] != RFC_SERVER_R2) && (ips[0] != RFC_SERVER_R3) && (ips[0] != RFC_SERVER_EXT) && (ips[0] != RFC_SERVER_SHM));
  
  sapsys[0] = ips[0];
  
  sprintfU (wbuf+strlenU (wbuf), cU("SAPSYS=%c "), sapsys[0]);

  saplogon[0] = cU('N');


  /* If External, using register mode ? */
  if (sapsys[0] == RFC_SERVER_EXT)
  {
    if (strlenU (register_mode) == 0)
      register_mode[0] = cU('Y');
    
    do
    {
      printfU (cU("                 Using registering feature (Y/N)Y........: (%c): "), register_mode[0]);
      
      fflush(stdout);
     
      getsU(ips);

      if (strlenU (ips) == 0)
        ips[0] = register_mode[0];
      
      ips[0] =  toupperU(ips[0]);
    } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
    
    register_mode[0] = ips[0];
    
    sprintfU (wbuf+strlenU (wbuf), cU("REGMOD=%c "), register_mode[0]);
  }
  else if (sapsys[0] == RFC_SERVER_SHM)
  {
    strcpyU (profile_name, prf_name);

    printfU (cU("                 Profile file name ...........: (%s): "), profile_name);
      
    fflush(stdout);
     
    getsU(ips);

    if (strlenU (ips) != 0)
     strcpyU (profile_name, ips);

    sprintfU (wbuf+strlenU (wbuf), cU("PROFILE_NAME=%s "), profile_name);
  }
  else
  {
    /* Using saplogon */
#ifdef SAPonNT
    if (strlenU (saplogon) == 0)
      saplogon[0] = cU('Y');
    do
    {
      printfU (cU("                 Using data of saplogon (Y/N)Y...........: (%c): "), saplogon[0]);

      fflush(stdout);
      
      getsU (ips);

      if (strlenU (ips) == 0)
        ips[0] = saplogon[0];
      
      ips[0] =  toupperU(ips[0]);
    } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
    
    saplogon[0] = ips[0];
    
    sprintfU (wbuf+strlenU (wbuf), cU("SAPLOGON=%c "), saplogon[0]);
#endif
  }

  if (saplogon[0] == cU('Y'))
  {
    /* ID in saplogon */
    if (strlenU (saplogon_id) == 0)
      strcpyU (saplogon_id, std_saplogon_id);

    printfU (cU("                 ID in saplogon..........................: (%s): "), saplogon_id);
    
    fflush(stdout);

    getsU (ips);

    if (strlenU (ips) == 0)
      strcpyU (ips,  saplogon_id);
    
    ips[256] = 0;
    
    strcpyU (saplogon_id, ips);
    
    check_blank(saplogon_id);
    
    sprintfU (wbuf+strlenU (wbuf), cU("SAPLOGON_ID=%s "), saplogon_id);

    /* If R/2, destination in sideinfo for gateway */
    if (sapsys[0] == RFC_SERVER_R2)
    {
      if (strlenU (rfc_dest) == 0)
        strcpyU (rfc_dest, std_rfc_dest_r2);

      printfU (cU("                 DEST in 'sideinfo' for the SAP gateway..: (%s): "), rfc_dest);
      
      fflush(stdout);
      
      getsU (ips);

      if (strlenU (ips) == 0)
        strcpyU (ips,  rfc_dest);
      
      ips[32] = 0;
      
      strcpyU (rfc_dest, ips);
      
      znr = strlenU (rfc_dest);
      
      for (i = 0; i < znr; i++)
        rfc_dest[i] =  toupperU(rfc_dest[i]);
      
      sprintfU (wbuf+strlenU (wbuf), cU("DEST=%s "), rfc_dest);
    }
  }
  else
  {
    /* Working with saprfc.ini */
    if (strlenU (saprfc_ini) == 0)
      saprfc_ini[0] = cU('N');

    do
    {
      printfU (cU("                 Working with 'saprfc.ini' (Y/N)N........: (%c): "), saprfc_ini[0]);
      
      fflush(stdout);
      
      getsU (ips);
      
      if (strlenU (ips) == 0)
        ips[0] = saprfc_ini[0];
      
      ips[0] =  toupperU(ips[0]);
    } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
    
    saprfc_ini[0] = ips[0];
    
    sprintfU (wbuf+strlenU (wbuf), cU("RFCINI=%c "), saprfc_ini[0]);

    if (saprfc_ini[0] == cU('Y'))
    {
      /* Destination in saprfc.ini */
      if (strlenU (rfc_dest) == 0)
        strcpyU (rfc_dest, std_rfc_dest_r3);

      printfU (cU("                 Destination in 'saprfc.ini'.............: (%s): "), rfc_dest);
      
      fflush(stdout);
      
      getsU (ips);

      if (strlenU (ips) == 0)
        strcpyU (ips,  rfc_dest);
      
      ips[32] = 0;
      
      strcpyU (rfc_dest, ips);
      
      znr = strlenU (rfc_dest);
      
      for (i = 0; i < znr; i++)
        rfc_dest[i] =  toupperU(rfc_dest[i]);
      
      sprintfU (wbuf+strlenU (wbuf), cU("DEST=%s "), rfc_dest);
    }
    else
    {
      if (sapsys[0] == RFC_SERVER_R2)
      {
        /* Destination in sideinfo */
        if (strlenU (rfc_dest) == 0)
          strcpyU (rfc_dest, std_rfc_dest_r2);

        printfU (cU("                 DEST in 'sideinfo' for the SAP gateway..: (%s): "), rfc_dest);
        
        fflush(stdout);
        
        getsU (ips);
        
        if (strlenU (ips) == 0)
          strcpyU (ips,  rfc_dest);
        
        ips[32] = 0;
        
        strcpyU (rfc_dest, ips);
        
        znr = strlenU (rfc_dest);
        
        for (i = 0; i < znr; i++)
          rfc_dest[i] =  toupperU(rfc_dest[i]);
        
        sprintfU (wbuf+strlenU (wbuf), cU("DEST=%s "), rfc_dest);

        /* Host name of a SAP gateway */
        if (strlenU (gwhost) == 0)
          strcpyU (gwhost, std_gwhost);

        printfU (cU("                 Host name of the SAP gateway to R/2.....: (%s): "), gwhost);
        
        fflush(stdout);
        
        getsU (ips);

        if (strlenU (ips) == 0)
          strcpyU (ips,  gwhost);

        ips[100] = 0;
        
        strcpyU (gwhost, ips);
        
        sprintfU (wbuf+strlenU (wbuf), cU("GWHOST=%s "), gwhost);

        /* Service of the specified SAP gateway */
        if (strlenU (gwserv) == 0)
          strcpyU (gwserv, std_gwserv);

        printfU (cU("                 Service of the specified gateway........: (%s): "), gwserv);
        
        fflush(stdout);
        
        getsU (ips);
        
        if (strlenU ( ips) == 0)
          strcpyU (ips,  gwserv);

        ips[7] = 0;
        
        strcpyU (gwserv,  ips);
        
        sprintfU (wbuf+strlenU ( wbuf), cU("GWSERV=%s "), gwserv);
      }
      else if (sapsys[0] == RFC_SERVER_EXT)
      {
        /* Host name of a SAP gateway */
        if (strlenU (gwhost) == 0)
          strcpyU (gwhost,  stde_gwhost);

        printfU (cU("                 Host name of the SAP gateway............: (%s): "), gwhost);

        fflush(stdout);
        
        getsU (ips);
        
        if (strlenU (ips) == 0)
          strcpyU (ips,  gwhost);
        
        ips[100] = 0;
        
        strcpyU (gwhost,  ips);
        
        sprintfU (wbuf+strlenU (wbuf), cU("GWHOST=%s "), gwhost);

        /* Service of the specified SAP gateway */
        if (strlenU ( gwserv) == 0)
          strcpyU (gwserv,  stde_gwserv);

        printfU (cU("                 Service of the specified gateway........: (%s): "), gwserv);

        fflush(stdout);
        
        getsU (ips);

        if (strlenU (ips) == 0)
          strcpyU (ips,  gwserv);
        
        ips[7] = 0;
        
        strcpyU (gwserv,  ips);
        
        sprintfU (wbuf+strlenU ( wbuf), cU("GWSERV=%s "), gwserv);

        /* Program ID of partner program if working with register mode */
        if (register_mode[0] == cU('Y'))
        {
          if (strlenU ( tpid) == 0)
            strcpyU (tpid,  stde_tpid);

          printfU (cU("                 Program ID of registered partner program: (%s): "), tpid);

          fflush(stdout);
          
          getsU (ips);

          if (strlenU (ips) == 0)
            strcpyU (ips,  tpid);
          
          ips[100] = 0;
          
          strcpyU (tpid,  ips);
          
          sprintfU (wbuf+strlenU (wbuf), cU("TPID=%s "), tpid);
        }
        else
        {
          /* Host name of partner program */
          if (strlenU ( tphost) == 0)
            strcpyU (tphost, stde_tphost);

          printfU (cU("                 Host name of the partner program........: (%s): "), tphost);
          
          fflush(stdout);
          
          getsU (ips);
          
          if (strlenU (ips) == 0)
            strcpyU (ips,  tphost);
          
          ips[100] = 0;
          
          strcpyU (tphost, ips);
          
          sprintfU (wbuf+strlenU (wbuf), cU("TPHOST=%s "), tphost);

          /* Name of the partner program */
          if (strlenU ( tpname) == 0)
            strcpyU (tpname,  stde_tpname);

          printfU (cU("                 Fully qualified name of partner program.: (%s): "), tpname);
          
          fflush(stdout);
          
          getsU (ips);
          
          if (strlenU (ips) == 0)
            strcpyU (ips,  tpname);
          
          ips[100] = 0;
          
          strcpyU (tpname, ips);
          
          sprintfU (wbuf+strlenU (wbuf), cU("TPNAME=%s "), tpname);
        }
      }
      else if (sapsys[0] != RFC_SERVER_SHM)
      {
        /* Working with load balancing */
        if (strlenU ( load_balancing) == 0)
          load_balancing[0] = cU('Y');
        do
        {
          printfU (cU("                 Use load balancing (Y/N)Y...............: (%c): "), load_balancing[0]);
          
          fflush(stdout);
          
          getsU (ips);
          
          if (strlenU (ips) == 0)
            ips[0] = load_balancing[0];
          
          ips[0] =  toupperU (ips[0]);
        } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
        
        load_balancing[0] = ips[0];
        
        sprintfU (wbuf+strlenU (wbuf), cU("BALANCE=%c "), load_balancing[0]);

        if (load_balancing[0] == cU('Y'))
        {
          /* R/3 System name */
          if (strlenU ( r3name) == 0)
            strcpyU (r3name, std_r3name);

          printfU (cU("                 R/3 system name.........................: (%s): "), r3name);

          fflush(stdout);
          
          getsU (ips);

          if (strlenU (ips) == 0)
            strcpyU (ips,  r3name);

          ips[100] = 0;
          
          strcpyU (r3name, ips);
          if ((r3name[0] == cU('/')) || (r3name[0] == cU('"')))
          {
            ptr = r3name+strlenU ( r3name)-1;
            while (*ptr != cU('/'))
            {
                *ptr =  toupperU (*ptr);
                ptr--;
            }
          }
          else
          {
            znr = strlenU ( r3name);
            for (i = 0; i < znr; i++)
             r3name[i] =  toupperU (r3name[i]);
          }
          sprintfU (wbuf+strlenU (wbuf), cU("R3NAME=%s "), r3name);

          /* R/3 message server */
          if (strlenU ( mshost) == 0)
            strcpyU (mshost, cU("0"));

          printfU (cU("                 Message server or 0 for using sapmsg.ini: (%s): "), mshost);
          
          fflush(stdout);
          
          getsU (ips);

          if (strlenU (ips) == 0)
            strcpyU (ips,  mshost);
          
          ips[100] = 0;
          
          strcpyU (mshost, ips);
          
          sprintfU (wbuf+strlenU (wbuf), cU("MSHOST=%s "), mshost);

          /* R/3 Selected group */
          if (strlenU ( group) == 0)
            strcpyU (group, std_group);

          printfU (cU("                 Selected group (PUBLIC).................: (%s): "), group);
          
          fflush(stdout);
          
          getsU (ips);
          
          if (strlenU (ips) == 0)
            strcpyU (ips, group);
          
          ips[32] = 0;
          
          strcpyU (group, ips);
          
          check_blank(group);
          
          sprintfU (wbuf+strlenU (wbuf), cU("GROUP=%s "), group);
        }
        else
        {
          /* R/3 application server */
          if (strlenU ( ashost) == 0)
            strcpyU (ashost, std_ashost);

          printfU (cU("                 Host name of an application server......: (%s): "), ashost);
          
          fflush(stdout);
          
          getsU (ips);
          
          if (strlenU (ips) == 0)
            strcpyU (ips,  ashost);
          
          ips[100] = 0;
          
          strcpyU (ashost, ips);
          
          sprintfU (wbuf+strlenU (wbuf), cU("ASHOST=%s "), ashost);

          /* R/3 System number */
          if (strlenU ( sysnr) == 0)
            strcpyU (sysnr, std_sysnr);

          printfU (cU("                 System number...........................: (%s): "), sysnr);
          
          fflush(stdout);
          
          getsU (ips);

          if (strlenU (ips) == 0)
            strcpyU (ips,  sysnr);
          
          ips[2] = 0;
          
          strcpyU (sysnr, ips);
          
          sprintfU (wbuf+strlenU (wbuf), cU("SYSNR=%s "), sysnr);
        }
      }

      if ((sapsys[0] == RFC_SERVER_R3) || (sapsys[0] == RFC_SERVER_EXT))
      {
        /* Working with SNC */
        if (strlenU ( sncmode) == 0)
          sncmode[0] = cU('N');
        do
        {
          printfU (cU("                 Working with SNC (Y/N)N.................: (%c): "), sncmode[0]);

          fflush(stdout);
          
          getsU (ips);

          if (strlenU (ips) == 0)
            ips[0] = sncmode[0];
          
          ips[0] =  toupperU(ips[0]);
        } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
        
        sncmode[0] = ips[0];
        
        sprintfU (wbuf+strlenU (wbuf), cU("SNC_MODE=%c "), sncmode[0]);

        if (sncmode[0] == cU('Y'))
        {
          /* SNC Library */
          if (strlenU ( snc_lib) == 0)
          {
                ptr = getenvU(cU("SNC_LIB"));
                if (ptr != NULL)
                {
                    ptr[256] = 0;
                    strcpyU (snc_lib, ptr);
                }
                else
                    strcpyU (snc_lib, std_snc_lib);
          }

          printfU (cU("                 SNC Library Name........................: (%s): "), snc_lib);

          fflush(stdout);
          
          getsU(ips);

          if (strlenU (ips) == 0)
            strcpyU (ips,  snc_lib);
          
          ips[256] = 0;
          
          strcpyU (snc_lib, ips);
          
          check_blank(snc_lib);
          
          sprintfU (wbuf+strlenU (wbuf), cU("SNC_LIB=%s "), snc_lib);

          /* SNC Partner Name */
          if (strlenU ( snc_partnername) == 0)
           strcpyU (snc_partnername, std_snc_partnername);
          
          printfU (cU("                 SNC name of partner program.............: (%s): "), snc_partnername);

          fflush(stdout);
          
          getsU(ips);
          
          if (strlenU (ips) == 0)
            strcpyU (ips,  snc_partnername);
          
          ips[256] = 0;
          
          strcpyU (snc_partnername, ips);
          
          check_blank(snc_partnername);
          
          sprintfU (wbuf+strlenU (wbuf), cU("SNC_PARTNERNAME=%s "), snc_partnername);
        }
      }
    }
  }


  #ifdef SAPwithUNICODE
  do
  {
        if (pcs == cU('\0'))
         pcs = cU('1');

        printfU (cU("                 PCS value...............................: (%c): "), pcs);

        fflush (stdout);

        getsU (ips);

        if (strlenU (ips) == 0)
         ips[0] = pcs;

        ips[0] = toupperU (ips[0]);
  } while ((ips[0] != cU('1')) && (ips[0] != cU('2')));

  pcs = ips[0];

  sprintfU (wbuf+strlenU (wbuf), cU("PCS=%c "), pcs);
  #endif


  printfU (cU("\n               RFC-SPECIFIC PARAMETERS:\n"));

  
  if ((sapsys[0] == RFC_SERVER_R3) &&
      ((saplogon[0] == cU('Y')) || (saprfc_ini[0] == cU('N'))))
  {
    /* Using ABAP debugger */
    if (strlenU ( abap_debug) == 0)
        abap_debug[0] = cU('N');

    do
        {
        printfU (cU("                 Working with ABAP debugger (Y/N)N.......: (%c): "), abap_debug[0]);

        fflush(stdout);

        getsU (ips);

        if (strlenU (ips) == 0)
        ips[0] = abap_debug[0];

        ips[0] =  toupperU(ips[0]);
    } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));

    abap_debug[0] = ips[0];

    sprintfU (wbuf+strlenU (wbuf), cU("DEBUG=%c "), abap_debug[0]);  

    if (ips[0] == cU('N'))
    {
        /* Using ext ABAP debugger */
        if (strlenU ( abap_debug_ext) == 0)
            abap_debug_ext[0] = cU('N');
        do
        {
            printfU (cU("                 Working with ext ABAP debugger (Y/N)N.......: (%c): "), abap_debug_ext[0]);

            fflush(stdout);

            getsU (ips);

            if (strlenU (ips) == 0)
            ips[0] = abap_debug_ext[0];

            ips[0] =  toupperU(ips[0]);
        } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));

        abap_debug_ext[0] = ips[0];

        sprintfU (wbuf+strlenU (wbuf), cU("DEBUG_EXT=%c "), abap_debug_ext[0]);

}


    /* Using SAPGUI */
    if (abap_debug[0] == cU('Y'))
    {
      use_sapgui[0] = cU('Y');

      printfU (cU("                 Use SAPGUI (Y/N)N.......................: (%c): \n"), use_sapgui[0]);

      invisible[0] = cU('N');
      
      printfU (cU("                 Automatically invisible SAPGUI (Y/N)N...: (%c): \n"), invisible[0]);
    }
    else
    {
      if (action == 4)
      {
        use_sapgui[0] = cU('Y');

        printfU (cU("                 Use SAPGUI (Y/N)N.......................: (%c): \n"), use_sapgui[0]);
      }
      else
      {
        if (strlenU ( use_sapgui) == 0)
          use_sapgui[0] = cU('N');
        do
        {
          printfU (cU("                 Use SAPGUI (Y/N)N.......................: (%c): "), use_sapgui[0]);
          
          fflush(stdout);
          
          getsU (ips);

          if (strlenU (ips) == 0)
            ips[0] = use_sapgui[0];

          ips[0] =  toupperU(ips[0]);
        } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
        
        use_sapgui[0] = ips[0];
      }

      if (use_sapgui[0] == cU('Y'))
      {
        if (strlenU ( invisible) == 0)
          invisible[0] = cU('N');
        do
        {
          printfU (cU("                 Automatically invisible SAPGUI (Y/N)N...: (%c): "), invisible[0]);
        
          fflush(stdout);
          
          getsU (ips);

          if (strlenU (ips) == 0)
            ips[0] = invisible[0];
          
          ips[0] =  toupperU(ips[0]);
        } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
        
        invisible[0] = ips[0];
      }
      else
        invisible[0] = cU('N');
    }

    if (use_sapgui[0] == cU('Y'))
    {
      if (strlenU ( gui_at_open) == 0)
        gui_at_open[0] = cU('Y');
      
      do
      {
        printfU (cU("                 Start SAPGUI at connection open (Y/N)Y..: (%c): "), gui_at_open[0]);
        fflush(stdout);
        
        getsU (ips);
        
        if (strlenU (ips) == 0)
          ips[0] = gui_at_open[0];

        ips[0] =  toupperU(ips[0]);
      } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
      
      gui_at_open[0] = ips[0];
    }
    else
      gui_at_open[0] = cU('Y');

    sprintfU (wbuf+strlenU (wbuf), cU("SAPGUI=%c "), use_sapgui[0]);
    sprintfU (wbuf+strlenU (wbuf), cU("INVISIBLE=%c "), invisible[0]);
    sprintfU (wbuf+strlenU (wbuf), cU("GUIATOPEN=%c "), gui_at_open[0]);
  }

  /* RFC-Trace */
  if ((saplogon[0] == cU('Y')) || (saprfc_ini[0] == cU('N')))
  {
    if (strlenU ( rfc_trace) == 0)
      rfc_trace[0] = cU('N');

    do
    {
      printfU (cU("                 RFC trace (Y/N)N........................: (%c): "), rfc_trace[0]);
      
      fflush (stdout);

      getsU (ips);
      
      if (strlenU (ips) == 0)
        ips[0] = rfc_trace[0];
      
      ips[0] = toupperU (ips[0]);
    } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
    
    rfc_trace[0] = ips[0];
    
    sprintfU (wbuf+strlenU (wbuf), cU("RFCTRACE=%c "), rfc_trace[0]);
  }


  return;
}


/*--------------------------------------------------------------------*/
/* Save parameters for next tests                                     */
/*--------------------------------------------------------------------*/
void save_test_parameter()
{

    /* Open the work file */
    work_fp = fopenU (work_file, cU("w"));

    if (work_fp == NULL)
    {
        printfU (cU("\n\nCan not save RFC-parameters for next tests"));
    
        printfU (cU("\n\nHit return key to continue\n"));

        fflush(stdout);

        getcharU ();
    }
    else
    {
        fputsU (wbuf, work_fp);

        fclose(work_fp);

        work_fp = NULL;
    }
    
    
    return;
}


/*--------------------------------------------------------------------*/
/* Get load test data from file                                       */
/*--------------------------------------------------------------------*/
void get_load_test_data_from_file(void)
{
  strcpyU (sbuf, sbuf2);

  printfU (cU("\n\n               PERFORMANCE TEST DATA:"));

  /* No of lines for ITAB0332 */
  memsetU (lgit0332_field, cU('\0'), 5);

  rc = PM_search(lgit0332_field, cU("LGIT0332="), 5);

  if (rc)
    memsetU (lgit0332_field, cU('0'), 5);

  lgit0332 = atoiU ( lgit0332_field);

  sprintfU (lgit0332_field, cU("%05d"), lgit0332);

  printfU (cU("\n                 #Lines in export table (length 0332)....: %s"), lgit0332_field);

  /* No of lines for ITAB1000 */
  memsetU (lgit1000_field, cU('\0'), 5);

  rc = PM_search (lgit1000_field, cU("LGIT1000="), 5);
  if (rc)
    memsetU (lgit1000_field, cU('0'), 5);

  lgit1000 = atoiU( lgit1000_field);
  sprintfU (lgit1000_field, cU("%05d"), lgit1000);
  printfU (cU("\n                 #Lines in export table (length 1000)....: %s"), lgit1000_field);

  /* No of lines for ETAB0332 */
  memsetU (lget0332_field, cU('\0'), 5);
  rc = PM_search(lget0332_field, cU("LGET0332="), 5);
  if (rc)
    memsetU (lget0332_field, cU('0'), 5);

  lget0332 = atoiU( lget0332_field);
  sprintfU (lget0332_field, cU("%05d"), lget0332);
  printfU (cU("\n                 #Lines in import table (length 0332)....: %s"), lget0332_field);

  /* No of lines for ETAB1000 */
  memsetU (lget1000_field, cU('\0'), 5);
  rc = PM_search(lget1000_field, cU("LGET1000="), 5);
  if (rc)
    memsetU (lget1000_field, cU('0'), 5);

  lget1000 = atoiU ( lget1000_field);
  sprintfU (lget1000_field, cU("%05d"), lget1000);
  printfU (cU("\n                 #Lines in import table (length 1000)....: %s"), lget1000_field);

  /* Check contents of tables */
  rc = PM_search (checktab, cU("CHECKTAB="), 1);
  if (rc)
    checktab[0] = cU('N');

  printfU (cU("\n                 Check contents of tables (Y/N)N.........: %c"), checktab[0]);

  /* Output tables on sreen */
  rc = PM_search(printtab, cU("PRINTTAB="), 1);
  if (rc)
    printtab[0] = cU('N');

  printfU (cU("\n                 Output tables on screen (Y/N)N..........: %c"), printtab[0]);

  /* Last confirm for test */
  printfU (cU("\n\n"));
  do
  {
    printfU (cU("            Do You Want To Test With These Parameters (Y/N)..: "));
    fflush(stdout);
    getsU (ips);
    ips[0] =  toupperU(ips[0]);
  } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));

  if ((abap_debug[0] == cU('Y')) && (strcmpU ( userid, cU("SAPCPIC")) == 0))
  {
    printfU (cU("\n\nNo access for ABAP debugging as SAPCPIC (It's a CPIC-User)!!!!"));
    printfU (cU("\n\nHit return key to continue\n"));
    fflush (stdout);
    getcharU ();
    ips[0] = cU('N');
  }

  if (ips[0] == cU('N'))
  {
    if (work_fp != NULL)
    {
      fclose(work_fp);
      work_fp = NULL;
    }
    get_connect_data_via_dialog();
    get_load_test_data_via_dialog();
    save_test_parameter();
  }

  return;
}


/*--------------------------------------------------------------------*/
/* Get load test data via dialog                                      */
/*--------------------------------------------------------------------*/
void get_load_test_data_via_dialog(void)
{
  printfU (cU("\n               PERFORMANCE TEST DATA:\n"));

  /* No of lines for ITAB0332 */
  if (strlenU ( lgit0332_field) == 0)
    strcpyU (lgit0332_field, cU("00000"));

  printfU (cU("                 #Lines in export table (length 0332)....: (%s): "), lgit0332_field);
  fflush(stdout);

  getsU (ips);

  if (strlenU (ips) == 0)
    strcpyU (ips, lgit0332_field);
  ips[5] = 0;
  
  strcpyU (lgit0332_field, ips);

  lgit0332 = atoiU ( lgit0332_field);
  sprintfU (lgit0332_field, cU("%05d"), lgit0332);
  sprintfU (wbuf+strlenU (wbuf), cU("LGIT0332=%s "), lgit0332_field);

  /* No of lines for ITAB1000 */
  if (strlenU ( lgit1000_field) == 0)
    strcpyU (lgit1000_field, cU("00000"));
  printfU (cU("                 #Lines in export table (length 1000)....: (%s): "), lgit1000_field);
  fflush(stdout);
  getsU (ips);

  if (strlenU (ips) == 0)
    strcpyU (ips,  lgit1000_field);
  ips[5] = 0;
  
  strcpyU (lgit1000_field, ips);
  lgit1000 = atoiU ( lgit1000_field);
  sprintfU (lgit1000_field, cU("%05d"), lgit1000);
  sprintfU (wbuf+strlenU (wbuf), cU("LGIT1000=%s "), lgit1000_field);

  /* No of lines for ETAB0332 */
  if (strlenU (lget0332_field) == 0)
    strcpyU (lget0332_field, cU("00000"));

  printfU (cU("                 #Lines in import table (length 0332)....: (%s): "), lget0332_field);
  fflush(stdout);
  getsU (ips);

  if (strlenU (ips) == 0)
    strcpyU (ips, lget0332_field);
  
  ips[5] = 0;
  strcpyU (lget0332_field, ips);
  lget0332 = atoiU (lget0332_field);

  sprintfU (lget0332_field, cU("%05d"), lget0332);
  sprintfU (wbuf+strlenU (wbuf), cU("LGET0332=%s "), lget0332_field);

  /* No of lines for ETAB1000 */
  if (strlenU (lget1000_field) == 0)
    strcpyU (lget1000_field, cU("00000"));

  printfU (cU("                 #Lines in import table (length 1000)....: (%s): "), lget1000_field);
  fflush(stdout);
  getsU(ips);
  if (strlenU (ips) == 0)
    strcpyU (ips, lget1000_field);
  ips[5] = 0;
  strcpyU (lget1000_field, ips);
  lget1000 = atoiU (lget1000_field);
  sprintfU (lget1000_field, cU("%05d"), lget1000);
  sprintfU (wbuf+strlenU (wbuf), cU("LGET1000=%s "), lget1000_field);

  /* Check contents of tables */
  if (strlenU (checktab) == 0)
    checktab[0] = cU('N');
  
  do
  {
    printfU (cU("                 Check contents of tables (Y/N)N.........: (%c): "), checktab[0]);
    fflush(stdout);
    getsU (ips);
    if (strlenU (ips) == 0)
      ips[0] = checktab[0];
    ips[0] =  toupperU(ips[0]);
  } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
  
  checktab[0] = ips[0];
  sprintfU (wbuf+strlenU (wbuf), cU("CHECKTAB=%c "), checktab[0]);

  /* Output tables on screen */
  if (strlenU (printtab) == 0)
    printtab[0] = cU('N');

  do
  {
    printfU (cU("                 Output tables on screen (Y/N)N..........: (%c): "), printtab[0]);
    fflush(stdout);
    getsU (ips);

    if (strlenU (ips) == 0)
      ips[0] = printtab[0];
    
    ips[0] =  toupperU(ips[0]);
  } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
  
  printtab[0] = ips[0];
  
  sprintfU (wbuf+strlenU (wbuf), cU("PRINTTAB=%c \n"), printtab[0]);

  /* Last confirm for test */
  printfU (cU("\n"));
  do
  {
    printfU (cU("            Do You Want To Test With These Parameters (Y/N)..: "));
    fflush(stdout);

    getsU (ips);
    ips[0] =  toupperU(ips[0]);
  
  } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));

  
  if ((abap_debug[0] == cU('Y')) && (strcmpU (userid, cU("SAPCPIC")) == 0))
  {
    printfU (cU("\n\nNo access for ABAP debugging as SAPCPIC (It's a CPIC-User)!!!!"));
    printfU (cU("\n\nHit return key to continue\n"));
    fflush(stdout);
    
    getcharU ();
    ips[0] = cU('N');
  }

  if (ips[0] == cU('N'))
  {
    if (work_fp != NULL)
    {
      fclose(work_fp);
      work_fp = NULL;
    }
    get_connect_data_via_dialog();
    get_load_test_data_via_dialog();
    save_test_parameter();
  }
  return;
}


/*--------------------------------------------------------------------*/
/* Get diag test data from file                                       */
/*--------------------------------------------------------------------*/
void get_diag_test_data_from_file(void)
{
  strcpyU (sbuf, sbuf2);

  printfU (cU("\n\n               RFC WITH SAPGUI TEST DATA:"));

  /* Start a transaction or a function module in R/3 */
  rc = PM_search (transaction_test, cU("GUITEST="), 1);
  if (rc)
    transaction_test[0] = cU('F');
  
  printfU (cU("\n                 Start transaction or function (T/F)F....: %c"), transaction_test[0]);

  /* Name of a transaction or a function module in R/3 */
  if (transaction_test[0] == cU('T'))
  {
    memsetU (tcode, cU('\0'), 30);

    rc = PM_search(tcode, cU("TCODE="), 30);
    if (rc)
      strcpyU (tcode, std_tcode);
    
    printfU (cU("\n                 Name of transaction in R/3..............: %s"), tcode);
  }
  else
  {
    memsetU (fname, cU('\0'), 30);

    rc = PM_search (fname, cU("FNAME="), 30);
    if (rc)
      strcpyU (fname, std_fname);
    
    printfU (cU("\n                 Name of function module in R/3..........: %s"), fname);
  }

  /* Last confirm for test */
  printfU (cU("\n\n"));
  do
  {
    printfU (cU("            Do You Want To Test With These Parameters (Y/N)..: "));
    fflush(stdout);
    getsU (ips);
    ips[0] =  toupperU(ips[0]);
  } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));

  if ((abap_debug[0] == cU('Y')) && (strcmpU (userid, cU("SAPCPIC")) == 0))
  {
    printfU (cU("\n\nNo access for ABAP debugging as SAPCPIC (It's a CPIC-User)!!!"));
    printfU (cU("\n\nHit return key to continue\n"));
    fflush(stdout);

    getcharU ();
    
    ips[0] = cU('N');
  }

  if (ips[0] == cU('N'))
  {
    if (work_fp != NULL)
    {
      fclose(work_fp);
      work_fp = NULL;
    }
    get_connect_data_via_dialog();
    get_diag_test_data_via_dialog();
    save_test_parameter();
  }

  return;
}


/*--------------------------------------------------------------------*/
/* Get diag test data via dialog                                      */
/*--------------------------------------------------------------------*/
void get_diag_test_data_via_dialog(void)
{
  printfU (cU("\n               RFC WITH SAPGUI TEST DATA:\n"));

  /* Start a transaction or a function module in R/3 */
  if (strlenU (transaction_test) == 0)
    transaction_test[0] = cU('F');
  do
  {
    printfU (cU("                 Start transaction or function (T/F)F....: (%c): "), transaction_test[0]);
    fflush(stdout);
    getsU (ips);
    if (strlenU (ips) == 0)
      ips[0] = transaction_test[0];
    
    ips[0] =  toupperU (ips[0]);
  } while ((ips[0] != cU('T')) && (ips[0] != cU('F')));
  transaction_test[0] = ips[0];
  sprintfU (wbuf+strlenU (wbuf), cU("GUITEST=%c "), transaction_test[0]);

  /* Name of a transaction or a function module in R/3 */
  if (transaction_test[0] == cU('T'))
  {
    if (strlenU (tcode) == 0)
      strcpyU (tcode, std_tcode);

    printfU (cU("                 Name of transaction (SE38)..............: (%s): "), tcode);
    fflush(stdout);
    getsU (ips);
    if (strlenU (ips) == 0)
      strcpyU (ips, tcode);
    
    ips[4] = 0;
    
    znr = strlenU (ips);
    
    for (i = 0; i < znr; i++)
      ips[i] =  toupperU(ips[i]);
    
    strcpyU (tcode, ips);
    
    sprintfU (wbuf+strlenU (wbuf), cU("TCODE=%s "), tcode);
  }
  else
  {
    if (strlenU (fname) == 0)
      strcpyU (fname, std_fname);

    printfU (cU("                 Name of function module (STFC_SAPGUI)...: (%s): "), fname);
    fflush(stdout);
    getsU (ips);
    
    if (strlenU (ips) == 0)
      strcpyU (ips, fname);
    
    ips[30] = 0;
    
    znr = strlenU (ips);
    
    for (i = 0; i < znr; i++)
      ips[i] =  toupperU(ips[i]);
    
    strcpyU (fname, ips);
    
    sprintfU (wbuf+strlenU (wbuf), cU("FNAME=%s "), fname);
  }

  /* Last confirm for test */
  printfU (cU("\n"));
  do
  {
    printfU (cU("            Do You Want To Test With These Parameters (Y/N)..: "));
    fflush(stdout);
    getsU (ips);
    ips[0] =  toupperU(ips[0]);
  } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));

  if ((abap_debug[0] == cU('Y')) && (strcmpU (userid, cU("SAPCPIC")) == 0))
  {
    printfU (cU("\n\nNo access for ABAP debugging as SAPCPIC (It's a CPIC-User)!!!"));
    printfU (cU("\n\nHit return key to continue\n"));
    fflush(stdout);
    getcharU();
    ips[0] = cU('N');
  }

  if (ips[0] == cU('N'))
  {
    if (work_fp != NULL)
    {
      fclose(work_fp);
      work_fp = NULL;
    }
    get_connect_data_via_dialog();
    get_diag_test_data_via_dialog();
    save_test_parameter();
  }
  return;
}


/*--------------------------------------------------------------------*/
/* Get back test data from file                                       */
/*--------------------------------------------------------------------*/
void get_back_test_data_from_file(void)
{
  strcpyU (sbuf, sbuf2);

  printfU (cU("\n\n               RFC WITH CALLBACK TEST DATA:"));

  /* No of Callbacks from RFC server */
  memsetU (nrback_field, cU('\0'), 5);
  rc = PM_search(nrback_field, cU("NRBACK="), 5);
  if (rc)
    strcpyU (nrback_field, cU("00001"));

  nrback = atoiU (nrback_field);
  sprintfU (nrback_field, cU("%05d"), nrback);
  printfU (cU("\n                 #Callbacks from RFC server..............: %s"), nrback_field);

  /* Last confirm for test */
  printfU (cU("\n\n"));
  do
  {
    printfU (cU("            Do You Want To Test With These Parameters (Y/N)..: "));
    fflush(stdout);
    getsU (ips);
    ips[0] =  toupperU(ips[0]);
  } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));


  if (ips[0] == cU('N'))
  {
    if (work_fp != NULL)
    {
      fclose(work_fp);
      work_fp = NULL;
    }
    get_connect_data_via_dialog();
    get_back_test_data_via_dialog();
    save_test_parameter();
  }

  return;
}


/*--------------------------------------------------------------------*/
/* Get back test data via dialog                                      */
/*--------------------------------------------------------------------*/
void get_back_test_data_via_dialog(void)
{
  printfU (cU("\n               RFC WITH CALLBACK TEST DATA:\n"));

  /* No of Callbacks from RFC server */
  if (strlenU (nrback_field) == 0)
    strcpyU (nrback_field, cU("00001"));

  printfU (cU("                 #Callbacks from RFC server..............: (%s): "), nrback_field);
  fflush(stdout);
  
  getsU(ips);
  
  if (strlenU (ips) == 0)
    strcpyU (ips, nrback_field);
  ips[5] = 0;
  
  strcpyU (nrback_field, ips);
  nrback = atoiU (nrback_field);

  sprintfU (nrback_field, cU("%05d"), nrback);
  sprintfU (wbuf+strlenU (wbuf), cU("NRBACK=%s "), nrback_field);

  /* Last confirm for test */
  printfU (cU("\n"));
  do
  {
    printfU (cU("            Do You Want To Test With These Parameters (Y/N)..: "));
    fflush(stdout);
    getsU(ips);
    ips[0] =  toupperU(ips[0]);
  } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));

  if (ips[0] == cU('N'))
  {
    if (work_fp != NULL)
    {
      fclose(work_fp);
      work_fp = NULL;
    }
    get_connect_data_via_dialog();
    get_back_test_data_via_dialog();
    save_test_parameter();
  }
  return;
}


/*--------------------------------------------------------------------*/
/* Get sreg test data from file                                       */
/*--------------------------------------------------------------------*/
void get_sreg_test_data_from_file(void)
{
  strcpyU (sbuf, sbuf2);

  printfU (cU("\n\n               START/CONNECT REG. SERVER TEST DATA:"));

  /* Program Name of the according RFC server */
  memsetU (tpname, cU('\0'), 100);

  rc = PM_search(tpname, cU("TPNAME="), 100);
  if (rc)
    strcpyU (tpname, cU("srfcserv"));

  printfU (cU("\n                 Program Name of the RFC server..........: %s"), tpname);

  /* Last confirm for test */
  printfU (cU("\n\n"));
  do
  {
    printfU (cU("            Do You Want To Test With These Parameters (Y/N)..: "));
    fflush(stdout);
    getsU (ips);
    ips[0] =  toupperU(ips[0]);
  } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));

  if (ips[0] == cU('N'))
  {
    if (work_fp != NULL)
    {
      fclose(work_fp);
      work_fp = NULL;
    }
    get_connect_data_via_dialog();
    get_sreg_test_data_via_dialog();
    save_test_parameter();
  }

  return;
}


/*--------------------------------------------------------------------*/
/* Get back test data via dialog                                      */
/*--------------------------------------------------------------------*/
void get_sreg_test_data_via_dialog(void)
{
  printfU (cU("\n               START/CONNECT REG. SERVER TEST DATA:\n"));

  /* Program Name of the according RFC server */
  if (strlenU (tpname) == 0)
    strcpyU (tpname, cU("srfcserv"));
  printfU (cU("                 Program Name of the RFC server..........: (%s): "), tpname);
  fflush(stdout);
  getsU(ips);
  if (strlenU (ips) == 0)
    strcpyU (ips, tpname);

  ips[64] = 0;
  strcpyU (tpname, ips);

  sprintfU (wbuf+strlenU (wbuf), cU("TPNAME=%s "), tpname);

  /* Last confirm for test */
  printfU (cU("\n"));
  
  do
  {
    printfU (cU("            Do You Want To Test With These Parameters (Y/N)..: "));
    fflush(stdout);
    getsU (ips);
    ips[0] =  toupperU(ips[0]);
  } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));

  if (ips[0] == cU('N'))
  {
    if (work_fp != NULL)
    {
      fclose(work_fp);
      work_fp = NULL;
    }
    get_connect_data_via_dialog();
    get_sreg_test_data_via_dialog();
    save_test_parameter();
  }
  return;
}


/*--------------------------------------------------------------------*/
/* Check blank in buffer and put "..." if necessary                   */
/*--------------------------------------------------------------------*/
void check_blank(rfc_char_t *text)
{
  int         len;
  rfc_char_t *pa,
              buf[512];

  len = strlenU (text);
  if ((text[0] == cU('"')) && (text[len-1] != cU('"')))
  {
    text[len]   = cU('"');
    text[len+1] = cU('\0');
  }

  pa = memchrU (text, cU(' '), len);
  if ((pa != NULL) && (text[0] != cU('"')))
  {
    strcpyU (buf, text);
    text[0] = cU('"');
    strcpyU (text+1, buf);
    text[len+1] = cU('"');
    text[len+2] = cU('\0');
  }
  return;
}


/*--------------------------------------------------------------------*/
/* read record from file with single or multi line                    */
/*--------------------------------------------------------------------*/
rfc_char_t *read_record (FILE *fp,
                         rfc_char_t *ibuf)
{
  int         offset = 0;
  rfc_char_t  line[LINE_SIZE+1];

  ibuf[0] = 0;

  for (;;)
  {
    if ((ptr = fgetsU (line, LINE_SIZE, fp)) == NULL)
      return (NULL);

    if (line[strlenU (line)-1] == cU('\n'))
      line[strlenU (line)-1] = 0;

    ptr = line;
    switch (*ptr)
    {
      case cU('\0'): continue;
      case cU('*') : continue;
      case cU(' ') : continue;
      default  : ;
    }

    strcpyU (ibuf+offset, ptr);

    if (ibuf[strlenU (ibuf)-1] == cU('&'))
    {
      ibuf[strlenU (ibuf)-1] = 0;
      offset = strlenU (ibuf);
      continue;
    }
    break;
  }
  return (ibuf);
}


/*--------------------------------------------------------------------*/
/* Fill internal table as required                                    */
/*--------------------------------------------------------------------*/
int fill_table (ITAB_H itab_h, int    leng)
{
  int 		        position, linenr;
  rfc_char_t		etext[1001];

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
    sprintfU (etext+30, cU("%05d"), linenr);
    etext[35] = cU(' ');

    ptr = (SAP_UC*) ItAppLine(itab_h);
    if (ptr == NULL) 
     break;
    memcpyU (ptr, etext, ulen);
  }

  if (linenr <= leng)
    return 1;
  else
    return 0;
}


/*--------------------------------------------------------------------*/
/* Check ETABnnnn                                                     */
/*--------------------------------------------------------------------*/
int check_table ( ITAB_H        itab_h,
                  RFC_CHAR     *itabname,
                  int           recvleng,
                  rfc_char_t   *checktab,
                  rfc_char_t   *rtext,
                  rfc_char_t   *retcode)
{
  int 	position;
  rfc_char_t	value[2],
	            tbuff[1001],
	            etext[73]= iU(" "),
	            errline[40]= iU("ERROR:  #Lines = nnnnn  Expected: nnnnn"),
	            errcont[40]= iU("ERROR at line nnnnn position nnnn:  n/n"),
	            mtext[73]  = iU("Import table (length nnnn):  #Lines = nnnnn: OK,  Contents: OK");

  memsetU (etext, cU(' '), 72);

  itableng = ItFill(itab_h);
  lineleng = ItLeng(itab_h)/sizeofR(SAP_UC);
  ulen     = ItLeng(itab_h)/sizeofR(SAP_UC);

  strcpyU (etext, mtext);
  etext[strlenU (etext)] = cU(' ');

  if (itabname[0] == cU('I'))
    memcpyU (etext, cU("Ex"), 2);

  memcpyU (etext+21, itabname+4, 4);

  if (itableng != recvleng)
  {
    memcpyU (etext+28, errline, 39);
    sprintfU (etext+45, cU("%05d"), itableng);
    etext[50] = cU(' ');
    sprintfU (etext+62, cU("%05d"), recvleng);
    etext[67] = cU(' ');
    retcode[0] = cU('E');
  }
  else
  {
    sprintfU (etext+38, cU("%05d"), itableng);
    etext[43] = cU(':');
    retcode[0] = cU('O');

    if (checktab[0] != cU('Y'))
      memcpyU (etext+60, cU("NO CHECK"), 8);
    else
    {
      for (j = 1; ; j++)
      {
        ptr = (rfc_char_t *) ItGetLine(itab_h, j);
        if (ptr == NULL) 
         break;

        memcpyU (tbuff, ptr, ulen);
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
          memcpyU (etext+28, errcont, 39);
          sprintfU (etext+42, cU("%05d"), j);
          etext[47] = cU(' ');
          sprintfU (etext+57, cU("%05d"), position);
          etext[62] = cU(' ');
          etext[64] = tbuff[position];
          etext[66] = value[0];
          break;
        }
      }
    }
  }
  strcpyU (rtext, etext);
  return 0;
}


/*--------------------------------------------------------------------*/
/* Output itab on screen                                              */
/*--------------------------------------------------------------------*/
int output_table ( ITAB_H itab_h,
                   rfc_char_t   *itabname)
{
  rfc_char_t tbuff[1001];

  lineleng = ulen = ItLeng(itab_h)/sizeofR(SAP_UC);

  printfU (cU("\n%s:\n"), itabname);

  for (j = 1; ; j++)
  {
    ptr = (rfc_char_t *) ItGetLine(itab_h, j);
    if (ptr == NULL) 
     break;

    memcpyU (tbuff, ptr, ulen);
    tbuff[lineleng] = cU('\0');
    printfU (cU("'%s'\n"), tbuff);
  }
  return 0;
}


/*--------------------------------------------------------------------*/
/* Compute time in millisec.                                          */
/*--------------------------------------------------------------------*/
#if defined(SAPonUNIX)
unsigned int diffmsec (struct timeval *tmend,
		       struct timeval *tmstart)
{
  if (tmstart->tv_usec > tmend->tv_usec)
  {
    tmend->tv_usec += 1000000;
    tmend->tv_sec--;
  }
  msectime = (tmend->tv_usec - tmstart->tv_usec)/1000 +
	     (tmend->tv_sec - tmstart->tv_sec)*1000;

  return msectime;
}
#elif defined(SAPonOS400)
unsigned int diffmsec_AS400 (time_t tmend, time_t tmstart)
{
  return (1000 * ((unsigned int)tmend - (unsigned int)tmstart));
}
#else
unsigned int diffmsec_NON_UNIX (struct timeb *tmend,
				struct timeb *tmstart)
{

  if (tmstart->millitm > tmend->millitm)
  {
      tmend->millitm = tmend->millitm + 1000;
      tmend->time = tmend->time - 1;
  }
  msectime = (tmend->millitm - tmstart->millitm) +
	     (tmend->time - tmstart->time)*1000;

  return msectime;
} 
#endif


/*--------------------------------------------------------------------*/
/* Output specific structure on screen                                */
/*--------------------------------------------------------------------*/
int print_mystruct(RFCTEST * rfctest)
{
    rfc_char_t s[100];

    printfU (cU("\nRFCFLOAT: %f"), rfctest->rfcfloat);
    printfU (cU("\nRFCCHAR1: %c"), rfctest->rfcchar1[0]);
    printfU (cU("\nRFCINT2 : %d"), rfctest->rfcint2 );
    printfU (cU("\nRFCINT1 : %d"), rfctest->rfcint1 );
    
    memcpyU (s, rfctest->rfcchar4, 4);
    
    s[4] = 0;
    
    printfU (cU("\nRFCCHAR4: %s"), s);
    printfU (cU("\nRFCINT4 : %d"), rfctest->rfcint4 );
    printfU (cU("\nRFCHEX3 : %x%x%x"), (unsigned) rfctest->rfchex3[0], 
                                       (unsigned) rfctest->rfchex3[1], 
                                       (unsigned) rfctest->rfchex3[2]);
    
    memcpyU (s, rfctest->rfcchar2, 2);
    
    s[2] = 0;
    
    printfU (cU("\nRFCCHAR2: %s"), s);
    
    memcpyU (s, rfctest->rfctime, 6);
    
    s[6] = 0;
    
    printfU (cU("\nRFCTIME : %s"), s);
    
    memcpyU (s, rfctest->rfcdate, 8);
    
    s[8] = 0;
    
    printfU (cU("\nRFCDATE : %s"), s);

    return 0;
}


/*--------------------------------------------------------------------*/
/* Output specific internal table on screen                           */
/*--------------------------------------------------------------------*/
int print_mytable(ITAB_H itab_h)
{
    RFCTEST    rfctest_buff;
    SAP_RAW  * ptr_raw;

    
    lineleng = ulen = ItLeng(itab_h);
    
    for (j = 1; ; j++)
    {
        ptr_raw = (SAP_RAW *) ItGetLine(itab_h, j);
    
        if (ptr_raw == NULL) 
         break;

        printfU (cU("\n\nLine No.: %d"), j);
    
        memcpyR (&rfctest_buff, ptr_raw, ulen);
    
        print_mystruct(&rfctest_buff);
    }
    
    return 0;
}



/*====================================================================*/
/*                                                                    */
/* Documentation for function STFC_CONNECTION                         */
/*                                                                    */
/*====================================================================*/
#define NL iU("\n")
static rfc_char_t *connection_test_docu (void)
{
    static rfc_char_t docu[] =
        iU("RFC-Client has to send a message to this function and wait ")       NL
        iU("for the echo and a response message (max. 255 B).          ")       NL
        iU("")                                                                  NL
        iU("IMPORTING")                                                         NL
        iU("  REQUTEXT        C(255)")                                          NL
        iU("EXPORTING")                                                         NL
        iU("  ECHOTEXT        C(255)")                                          NL
        iU("  RESPTEXT        C(255)")                                          NL
        ;
  
    return docu;
}


/*====================================================================*/
/*                                                                    */
/* RFC-FUNCTION:  STFC_CONNECTION                                     */
/*                                                                    */
/* Send an echo and a response message to the RFC client program      */
/*                                                                    */
/* The prototype of this function must be defined with _loadds on     */
/* Windows because it will run as a QuickWin application and work     */
/* with fprintf.                                                      */
/*                                                                    */
/*====================================================================*/
static RFC_RC DLL_CALL_BACK_FUNCTION connection_test (RFC_HANDLE rfc_handle)
{
    printfU (cU("\n\n     Start Function '%s'"), name_connection_test);

    memsetR (&parameters[0], 0, sizeofR (parameters));

    /* parameters */
    rfc_param (&parameters[0], cU("REQUTEXT"), TYPC, requtext, 255*sizeofR(SAP_UC));

    /* no table ---> terminate array */
    tables[0].name = NULL;

    /* receive data */
    printfU (cU("\n     <==  RfcGetData               rfc_rc = "));
    
    rfc_rc = RfcGetData (rfc_handle, parameters, tables);

    printfU (cU("%d"), rfc_rc);

    if (rfc_rc != RFC_OK)
    {
        rfc_error (cU("RfcGetData"));
    }
    
    
    /* Get specific info of this RFC connection */
    get_attributes(rfc_handle);

    
    /* Echo text */
    strcpyU (echotext, requtext);

    
    /* Info text */
    memsetU (resptext, BLANK, 255);
    
    memcpyU (resptext, cU("Sysid:"), 6);
    
    ptr = getenvU (cU("HOSTNAME"));
    
    if (ptr == NULL)
    {
        ptr = getenvU (cU("USER"));
    }
    
    if (ptr == NULL)
    {
        ptr = getenvU (cU("LOGNAME"));
    }
        
    if (ptr != NULL)
    {
        strcpyU (resptext+7, ptr);
    }
    else
    {
        strcpyU (resptext+7, cU("????????"));
    }
    
    resptext[strlenU (resptext)] = cU(' ');

    memcpyU (resptext+18, cU("Date & Time:"), 12);
    actutime = time(NULL);
    /*CCQ_CLIB_LOCTIME_OK*/
    time_ptr = localtime(&actutime);
    
    memcpyU (&resptext[31], asctimeU (time_ptr), 24);

    /*------------------------------------------------------------------*/
    /* Return parameters and tables                                     */
    /*------------------------------------------------------------------*/
    memsetR (&parameters[0], 0, sizeofR (parameters));

    /* parameters */
    rfc_param (&parameters[0], cU("ECHOTEXT"), TYPC, echotext, 255*sizeofR(SAP_UC));
    rfc_param (&parameters[1], cU("RESPTEXT"), TYPC, resptext, 255*sizeofR(SAP_UC));

    /* no table ---> terminate array */
    tables[0].name = NULL;


    /* return to the caller */
    printfU (cU("\n     <==  RfcSendData              rfc_rc = "));
    
    rfc_rc = RfcSendData(rfc_handle, parameters, tables);

    printfU (cU("%d\n"), rfc_rc);

    if (rfc_rc != RFC_OK)
    {
        rfc_error(cU("RfcSendData"));
    }


    return rfc_rc;
}


/*====================================================================*/
/*                                                                    */
/* Cancel an running ABAP Funktion                                    */
/*                                                                    */
/* Get system info of a RFC server                                    */
/*                                                                    */
/*====================================================================*/
int srfccancel()
{
 RFC_CANCEL_TYPE        cancel_type = RFC_CANCEL_SOFT;
 int                    tmpCancelType;
 RFC_INT                counter;
 RFC_CHAR               method[4] = iU("0");
 RFC_CHAR               messagetype = cU('S');
 RFC_CHAR               parameter[250] = iU(" ");

 RFC_INT                seconds;
 RFC_CHAR               busy_waiting;


 get_parameter_for_conn_test();

 RFC_connect(cU("srfccancel"));

 counter = 0;

 tmpCancelType = get_cancel_type (&busy_waiting, &seconds);

 if (tmpCancelType != 3)
 {
	 cancel_type = (RFC_CANCEL_TYPE) tmpCancelType;
 }

 for (nrcount=0; nrcount<nrcall; nrcount++)
 { 



  function = name_rfc_raise_exception;

  memsetR (&exporting[0], 0, sizeofR (exporting));
  memsetR (&importing[0], 0, sizeofR (importing));

  rfc_param (&exporting[0], cU("METHOD"),      TYPC, method,       4*sizeofR(SAP_UC));
  rfc_param (&exporting[1], cU("MESSAGETYPE"), TYPC, &messagetype, 1*sizeofR(SAP_UC));
  rfc_param (&exporting[2], cU("PARAMETER"),   TYPC, parameter,    250*sizeofR(SAP_UC));

  rfc_param (&importing[0], cU("COUNTER"),     TYPINT, &counter,   sizeofR (RFC_INT));

  /* no table ---> terminate array */
  tables[0].name = NULL;

  printfU (cU("\n\n<==  RfcCallReceive (%s,		rfc_handle = "), function);


  rfc_rc = RfcCallReceive(rfc_handle,
                            function,
                            exporting,
                            importing,
                            tables,
                            &exception);
   printfU (cU("%d)\n"), rfc_handle);
 
    /* check return code */
   switch(rfc_rc)
   {
    case RFC_OK:
        break;
    case RFC_EXCEPTION:
    case RFC_SYS_EXCEPTION:
        rfc_error(exception);
        break;
    default:
        rfc_error(cU("RfcCallReceive"));
        break;
   }
   display (cU("Export Parameter"), exporting);
   display (cU("Import Parameter"), importing);


   function = name_rfc_ping_and_wait;

   memsetR (&exporting[0], 0, sizeofR (exporting));
   memsetR (&importing[0], 0, sizeofR (importing));

   rfc_param (&exporting[0], cU("SECONDS"),      TYPINT, &seconds,      sizeofR (RFC_INT));
   rfc_param (&exporting[1], cU("BUSY_WAITING"), TYPC,   &busy_waiting, 1*sizeofR(SAP_UC));
   
   tables[0].name = NULL;

   printfU (cU("\n\n\n<==  RfcCall(%s,		rfc_handle = "), function);

   rfc_rc = RfcCall ( rfc_handle,
                      function,
                      exporting,
                      tables);
   printfU (cU("%d)\n"), rfc_handle);
   display (cU("Export Parameter"), exporting);

   /* waiting until call proceed */
   do
   {
    rfc_rc = RfcListen (rfc_handle);
    if (rfc_rc == RFC_RETRY)
    {
#ifndef SAPonNT
     sleep(2);
#endif
     /* Cancel or not cancel */
	 if (tmpCancelType != 3)
	 {
      printfU (cU("\n<==  RfcCancel(%s, rfc_handle = %d, cancel_type = %d) ...  "), function, rfc_handle, cancel_type);
    
      rfc_rc = RfcCancel (rfc_handle, cancel_type);
	 }
    }
   } while (rfc_rc == RFC_RETRY);

 
   if (rfc_rc == RFC_OK) 
   {
    printfU (cU("\n<==  RfcReceive(%s,		rfc_handle = "), function);
    rfc_rc = RfcReceive (rfc_handle,
                        importing,
                        tables,
                        &exception);
	 printfU (cU("%d)\n"), rfc_handle);
   }


   /* check return code */
   switch(rfc_rc)
   {
      case RFC_OK:
       printfU  (cU("\n>==  Funktion succesfully ended before cancel could be proceed\n"));
       display (cU("Import Parameter"), importing);
       break; 
      case RFC_CANCELED:
        printfU  (cU("\n>==  Funktion canceled. Connection still exists\n"));
        break;
      case RFC_CLOSED:
        printfU  (cU("\n>==  Funktion canceled and connection closed\n"));
        return 0;
      case RFC_EXCEPTION:
      case RFC_SYS_EXCEPTION:
        rfc_error(exception);
        break;
      default:
        rfc_error(cU("RfcCancel"));
        break;
   } /* end of swith (rfc_rc) */

   function = name_rfc_raise_exception;

   memsetR (&exporting[0], 0, sizeofR (exporting));
   memsetR (&importing[0], 0, sizeofR (importing));

   rfc_param (&exporting[0], cU("METHOD"),      TYPC, method,       4*sizeofR(SAP_UC));
   rfc_param (&exporting[1], cU("MESSAGETYPE"), TYPC, &messagetype, 1*sizeofR(SAP_UC));
   rfc_param (&exporting[2], cU("PARAMETER"),   TYPC, parameter,    250*sizeofR (SAP_UC));

   rfc_param (&importing[0], cU("COUNTER"),     TYPINT, &counter,   sizeofR (RFC_INT));

   /* no table ---> terminate array */
   tables[0].name = NULL;

   printfU (cU("\n\n\n<==  RfcCallReceive (%s,		rfc_handle = "), function);


   rfc_rc = RfcCallReceive(rfc_handle,
                            function,
                            exporting,
                            importing,
                            tables,
                            &exception);
    printfU (cU("%d)\n"), rfc_handle);

   /* check return code */
   switch(rfc_rc)
   {
      case RFC_OK:
        break;
      case RFC_EXCEPTION:
      case RFC_SYS_EXCEPTION:
        rfc_error(exception);
        break;
      default:
        rfc_error (cU("RfcCallReceive"));
        break;
   }

   display (cU("Export Parameter"), exporting);
   display (cU("Import Parameter"), importing);


   fflush(stdin);
   fflush(stdout);
 } /* end of  for (nrcount=0; nrcount<nrcall; nrcount++)*/

 return 0;
} /* end of srfccancel */


static int get_cancel_type (rfc_char_t *busy, RFC_INT* sec)
{
 rfc_char_t   inp;
 int          result = 0;
 short        tryInp = 0;

 while (tryInp == 0)
 {
  printfU  (cU("\n"));
  printfU  (cU("      Please choose cancel type :\n\n"));
  printfU  (cU("       0 ...... RFC_CANCEL_HARD \n"));
  printfU  (cU("       1 ...... RFC_CANCEL_SOFT \n"));
  printfU  (cU("       2 ...... RFC_CANCEL_CLOSE\n"));
  printfU  (cU("       n ...... Do not cancel\n\n"));
  printfU  (cU("      Your choice: "));

  inp = getcharU (); 
  fflush (stdin);
  fflush (stdout);
  tryInp = 1;
  switch (inp)
  {
   case cU('0'):
    result = (int) RFC_CANCEL_HARD;
    break;

   case cU('1'):
    result = (int) RFC_CANCEL_SOFT;
    break;

   case cU('2'):
    result = (int) RFC_CANCEL_CLOSE;
    break;

   case cU('N'):
	result = 3;
	break;

   default:
    printfU (cU("Wrong value. try again\n"));
    tryInp = 0;
    break;
  }


  if (tryInp == 1)
  {
   fflush(stdin);
   printfU  (cU("\n"));
   printfU  (cU("      Please input parameters for RFC_PING_AND_WAIT ABAP-function\n"));
   printfU  (cU("       SECONDS      (sec): "));
   fflush (stdout);
   /* SAPUNICODEOK_UFCT */
   scanfU  (cU("%d"), sec);

   fflush (stdin);

   do 
   {
    printfU  (cU("       BUSY_WAITING (cU('X') or cU(' ')): "));
    *busy = getcharU ();
    printfU (cU("\n"));
    fflush (stdout);
    fflush (stdin);
   } while ( !((*busy == cU('X')) || (*busy == cU(' '))));
  

  }
 } /* end of while */

 return result; 
}


int srfcchn (void)
{
    get_parameter_for_conn_test();

    RFC_connect(cU("srfcchn"));

    check_gui_for_start();

    function = name_rfc_chn;

    start_value  = 10;
    
    counter = 0;
    
    result = 0;

    for (nrcount=0; nrcount<nrcall; nrcount++)
    {
        memsetR (&exporting[0], 0, sizeofR (exporting));

        memsetR (&importing[0], 0, sizeofR (importing));

        memsetR (&changing[0], 0, sizeofR (changing));

        rfc_param (&changing[0], cU("COUNTER"),      TYPINT, &counter, sizeofR (counter)); 

        rfc_param (&importing[0], cU("RESULT"),      TYPINT, &result, sizeofR (result));

        rfc_param (&exporting[0], cU("START_VALUE"), TYPINT, &start_value, sizeofR (start_value));

        /* no table ---> terminate array */
        tables[0].name = NULL;

        
        display (cU("Export Parameter"), exporting);

        display (cU("Import Parameter"), importing);

        display (cU("Changing Parameter"), changing);


        printfU (cU("\n<==  RfcCallReceiveEx (%s)     rfc_rc = "), function);


        rfc_rc = RfcCallReceiveEx (rfc_handle,
                                   function,
                                   exporting,
                                   importing,
                                   changing,
                                   tables,
                                   &exception);

        printfU (cU("%d\n"), rfc_rc);


        /* check return code */
        switch(rfc_rc)
        {
            case RFC_OK:
                break;

            case RFC_EXCEPTION:
            case RFC_SYS_EXCEPTION:
                rfc_error(exception);
                break;

            default:
                rfc_error (cU("RfcCallReceiveEx"));
                break;
        }


        display (cU("Export Parameter"), exporting);

        display (cU("Import Parameter"), importing);

        display (cU("Changing Parameter"), changing);

        if (nrcount < nrcall-1)
        {
            if (use_close[0] == cU('Y'))
            {
                printfU (cU("\n<==  RfcClose\n"));

                RfcClose(rfc_handle);
            }

            printfU (cU("\n\nHit return key for next RFC call\n"));

            fflush(stdout);

            getcharU();


            /* If explizit or RFC server is NON-SAP and use the    */
            /* register mode:                                      */
            /*    ===> Close and Open the RFC connection           */
            if (use_close[0] == cU('Y'))
            {
                /* LOAD BALANCING only for the first time */
                if ((sapsys[0] == RFC_SERVER_R3) && (load_balancing[0] == cU('Y')))
                {
                    saprfc_ini[0]     = cU('N');
            
                    load_balancing[0] = cU('N');
            
                    strcpyU (ashost, rfc_attributes.partner_host);
            
                    strcpyU (sysnr,  rfc_attributes.systnr);
                }


                RFC_connect(cU("srfcchn_N"));
            }

        }
    }

    /* Get specific info of this RFC connection */
    get_attributes(rfc_handle);
    
    /*----------------------------------------------------------------*/
    /* Close RFC connection                                           */
    /*----------------------------------------------------------------*/
    printfU (cU("\n<==  RfcClose\n"));
    
    RfcClose(rfc_handle);

    return 0;
}


/*====================================================================*/
/*                                                                    */
/* Calls the function module RFC_RAISE_ERROR                          */
/*                                                                    */
/*====================================================================*/
int srfcraiseerror()
{
    RFC_INT                counter = 0;
    RFC_CHAR               method[4] = iU("0");
    RFC_CHAR               messagetype[2] = iU("S");
    RFC_CHAR               parameter[250] = iU(" ");
    RFC_CHAR               return_value[250] = iU(" ");

    get_parameter_for_conn_test();

    RFC_connect(cU("srfcraiseerror"));

    getParametersForRaiseError (method, messagetype, parameter);
    function = name_rfc_raise_exception;
    
    memsetR (&exporting[0], 0, sizeofR (exporting));
    memsetR (&importing[0], 0, sizeofR (importing));
    memsetR (&tables[0],    0, sizeofR (tables));

    rfc_param (&exporting[0], cU("METHOD"),      TYPC, method,       4*sizeofR(SAP_UC));
    rfc_param (&exporting[1], cU("MESSAGETYPE"), TYPC, messagetype,  1*sizeofR(SAP_UC));
    rfc_param (&exporting[2], cU("PARAMETER"),   TYPC, parameter,    250*sizeofR(SAP_UC));
    /* last entry ---> terminate array */
    exporting[3].name 	= NULL;
    
    rfc_param (&importing[0], cU("COUNTER"),      TYPINT, &counter,       sizeofR (RFC_INT));
    rfc_param (&importing[1], cU("RETURN_VALUE"), TYPC, return_value,  250*sizeofR(SAP_UC));
    /* last entry ---> terminate array */
    importing[2].name 	= NULL;
    
    /* no table ---> terminate array */
    tables[0].name = NULL;
    
    printfU (cU("\n\n<==  RfcCallReceive (%s,		rfc_handle = "), function);

    rfc_rc = RfcCallReceive(rfc_handle,
    function,
    exporting,
    importing,
    tables,
    &exception);
    printfU (cU("%d)\n"), rfc_handle);
    
    /* check return code */
    switch(rfc_rc)
    {
    case RFC_OK:
        break;
    case RFC_EXCEPTION:
    case RFC_SYS_EXCEPTION:
        rfc_error(exception);
        break;
    default:
        rfc_error(cU("RfcCallReceive"));
        break;
    }
    display (cU("Export Parameter"), exporting);
    display (cU("Import Parameter"), importing);
    
    fflush(stdin);
    fflush(stdout);
    
    /*----------------------------------------------------------------*/
    /* Close RFC connection                                           */
    /*----------------------------------------------------------------*/
    printfU (cU("\n<==  RfcClose\n"));
    
    RfcClose(rfc_handle);

    return 0;
} /* end of srfcraiseerror */


int getParametersForRaiseError(rfc_char_t * method, rfc_char_t * messagetype, rfc_char_t * parameter)
{
    /* save the standard values */
    rfc_char_t  tmp_method[4],
                tmp_messagetype[2],
                tmp_parameter[250],
                dummy[] = iU("    ");/* to fill method up to 4 chars */
    int imethod;
    int sk_tmp_inp;

    strcpyU (tmp_method, method);
    strcpyU (tmp_messagetype,  messagetype);
    strcpyU (tmp_parameter, parameter);

    /* read method value from stdin */
    do
    {
        printfU (cU("\n\n                 Parameters for RFC_RAISE_ERROR"));
        printfU (cU("\n                 Method No.........................: (%s): "), tmp_method);
        
        fflush(stdout);
        
        getsU (method);
        
        if (strlenU (method) == 0)
            strcpyU (method,  tmp_method);
            imethod = atoiU(method);
    }
    while (imethod < 0 && imethod > 24);
    sk_tmp_inp = strlenU(method);

    /* fills with spaces up to 4 chars */
    /*strncatU(method, dummy, strlenU(dummy) - strlenU(method));*/
    memsetU (&method[sk_tmp_inp], cU(' '), 4  - sk_tmp_inp);
        
    /* read messagetype value from stdin */
    do
    {
        printfU (cU("\n                 Message Type"));
        printfU (cU("\n                 Possible values are :\n"));
        printfU (cU("\n                 A = Abort & end"));
        printfU (cU("\n                 E = Error message"));
        printfU (cU("\n                 I = Information message"));
        printfU (cU("\n                 S = Status message"));
        printfU (cU("\n                 W = Warning"));
        printfU (cU("\n                 X = Exit with Shortdump"));   
        printfU (cU("\n                 your choice ......................: (%s): "), tmp_messagetype);
        
        fflush (stdout);
        
        getsU (messagetype);
        
        if (strlenU (messagetype) == 0)
        {    
            strcpyU (messagetype,  tmp_messagetype);
        }
 
        messagetype[0] = toupperU (messagetype[0]);
    }
	while (!strrchrU (cU("SEAWIX"),messagetype[0]));

    /* read parameter value from stdin */
    printfU (cU("\n                 Parameter   ......................: (%s): "), tmp_parameter);
    
    fflush(stdout);
    
    getsU (parameter);
    
    if (strlenU (parameter) == 0)
        strcpyU (parameter,  tmp_parameter);
        
    return 1;
}
