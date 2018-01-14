/* static rfc_char_t sccsid[] = "@(#) $Id: //bas/640_REL/src/krn/rfc/trfctest.c#5 $ SAP"; */

/*====================================================================*/
/*                                                                    */
/*     PROGRAM     :   trfctest.c (running on Windows_NT, Windows_95, */
/*                                 OS/2 and R/3-platforms)            */
/*                                                                    */
/*                                                                    */
/*     DESCRIPTION :   Sample Client Program for transactional RFC    */
/*                     Only for calling Function Module in R/3 >=3.0  */
/*                     Following FM will be called:                   */
/*                                                                    */
/*                       - STFC_WRITE_TO_TCPIC                        */
/*                         Send internal tables via tranactional RFC  */
/*                                                                    */
/*                                                                    */
/*                     trfctest.tid:  TID-Management                  */
/*                     trfctest.lck:  Lock file for access TID-Mgm.   */
/*                     trfctest.dat:  Data to transfer to R/3         */
/*                                                                    */
/*                     These files will be created/expected in the    */
/*                     directory defined via TRFC_WORK_DIR. If this   */
/*                     Environment Variable is not defined these files*/
/*                     will be in the actual directory.               */
/*                                                                    */
/*                     On UNIX this program can be started via a      */
/*                     script with setting the required Environment   */
/*                     Variable as follow:                            */
/*                                                                    */
/*                     trancrfc:                                      */
/*                     #!/bin/csh                                     */
/*                     setenv TRFC_WORK_DIR /usr/sap/rfctest          */
/*                     /usr/sap/rfctest/trfctest                      */
/*                                                                    */
/*                     Please do the same for other platforms.        */
/*                                                                    */
/*                     Because of working with file system it is      */
/*                     recommended to choose the working directory as */
/*                     a local directory and NOT mounted via NFS.     */
/*                                                                    */
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

#define _POSIX_SOURCE   /* open(), O_*, fctnl(), F_*, flock */

/*--------------------------------------------------------------------*/
/* Setup includes                                                     */
/*--------------------------------------------------------------------*/

#include "trfctest.h"


/*--------------------------------------------------------------------*/
/* main                                                               */
/*--------------------------------------------------------------------*/

mainU (int argc, rfc_char_t **argv)
{
  setbuf(stdout, NULL);

  memsetU (working_dir, cU('\0'), 64);

  if ((ptr = getenvU (cU("TRFC_WORK_DIR"))) != NULL)
  {
    if (strlenU (ptr) > 50)
    {
      printfU (cU("\nMax. string length of working directory exceeded (50 B)\n"));
      exit(1);
    }
    sprintfU (working_dir, cU("%s/"), ptr);
  }

  strcpyU (tid_file,  working_dir);
  strcpyU (para_file, working_dir);
  strcpyU (lock_file, working_dir);
  strcpyU (tid_file+strlenU (tid_file),   std_tid_file);
  strcpyU (para_file+strlenU (para_file), std_para_file);
  strcpyU (lock_file+strlenU (lock_file), std_lock_file);

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
          argv[i][5+128] = cU('\0');
        strcpyU (para_file, &argv[i][5]);
      }
    }
  }

  if (syntax_OK == 0)
  {
    printfU (cU("\nSyntax:   trfctest  [options]\n"));
    printfU (cU("\n          options:  file=<test parameter file>\n\n"));
    exit(1);
  }

  printfU (cU("\n\n"));
  printfU (cU("               **********************************************\n"));
  printfU (cU("               *                                            *\n"));
  printfU (cU("               *              SAP RFC-API TEST              *\n"));
  printfU (cU("               *                                            *\n"));
  printfU (cU("               *                   OS/2,                    *\n"));
  printfU (cU("               *           Windows_NT, Windows_95           *\n"));
  printfU (cU("               *                R/3-Platforms               *\n"));
  printfU (cU("               *                                            *\n"));
  printfU (cU("               *                                            *\n"));
  printfU (cU("               *              TRANSACTIONAL RFC             *\n"));
  printfU (cU("               *                                            *\n"));
  printfU (cU("               *          SEND DATA TO TCPIC-TABLE          *\n"));
  printfU (cU("               *                                            *\n"));
  printfU (cU("               **********************************************\n\n\n"));

  for (;;)
  {
    srfctran();

    printfU (cU("\n"));

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

  printfU (cU("n"));
  return 0;
}


/*====================================================================*/
/*                                                                    */
/* Call Function STFC_WRITE_TO_TCPIC via transactional RFC            */
/*                                                                    */
/* Send internal tables                                               */
/*                                                                    */
/*====================================================================*/
int srfctran()
{
  get_parameter_for_trfc_test();

  /* Open user data file */
  data_fp = fopenU (data_file, cU("r"));
  if (data_fp == NULL)
  {
    printfU (cU("\nCannot open user data file '%s'\n"), data_file);
    exit(1);
  }

  RFC_connect();

  /*--------------------------------------------------------------------*/
  /* Call Function module STFC_WRITE_TO_TCPIC via tRFC                  */
  /*--------------------------------------------------------------------*/

  function = name_transactional_test;

  memsetR (&exporting[0], 0, sizeofR(exporting));

  /* Prepare internal table TCPICDAT */
  tables[0].name     = cU("TCPICDAT");
  tables[0].nlen     = 8;
  tables[0].type     = TYPC;
  tables[0].leng     = TCPICDAT_ENTRY_SIZE*sizeofR(SAP_UC);
  tables[0].itmode   = RFC_ITMODE_BYREFERENCE;

  printfU (cU("\n<==  ItCreate (TCPICDAT)\n"));
  tables[0].ithandle = ItCreate(cU("TCPICDAT"),	
				                TCPICDAT_ENTRY_SIZE*sizeofR(SAP_UC),
				                0,
				                0);

  if (tables[0].ithandle == NULL )
    rfc_error(cU("ItCreate TCPICDAT"));

  /* terminate array */
  tables[1].name = NULL;

  /* Search for failed previous TID */
  printfU (cU("\n\nSearch for failed transaction (TID with state CREATED/EXECUTED).....\n"));
  tid_search_rc = search_TID(tid, data_file);

  switch(tid_search_rc)
  {
    case TID_ERROR_LOCK:
    {
      app_error(tid_etext01);
      break;
    }

    case TID_CREATED_FOUND:
    {
      printfU (cU("\n***  PREVIOUS TRANSACTIONAL RFC FAILED  ***"));
      printfU (cU("\n     Data file name:  %s"), data_file);
      printfU (cU("\n     TransID       :  %s"), tid);
      printfU (cU("\n     State:        :  CREATED\n"));
      printfU (cU("\nHit return key to repeat the transfer via tRFC ....."));
      fflush (stdout);
      getcharU ();
      break;
    }

    case TID_EXECUTED_FOUND:
    {
      printfU (cU("\n***  PREVIOUS TRANSACTIONAL RFC FAILED  ***"));
      printfU (cU("\n     Data file name:  %s"), data_file);
      printfU (cU("\n     TransID       :  %s"), tid);
      printfU (cU("\n     State:        :  EXECUTED\n"));
      printfU (cU("\nHit return key to repeat the confirmation ....."));
      fflush (stdout);
      getcharU ();
      break;
    }

    default:
    {
      printfU (cU("\n***  FIRST TRANSFER OF THIS TRANSACTION  ***\n"));
      printfU (cU("\nHit return key to get new TID from R/3 ............."));
      fflush (stdout);
      getcharU ();

      /* Get new Transaction ID from R/3 system */
      printfU (cU("\n<==  RfcCreateTransID         rfc_rc = "));
      rfc_rc = RfcCreateTransID(rfc_handle, tid);
      printfU (cU("%d\n"), rfc_rc);
      printfU (cU("     TID = '%s'\n"), tid);

      /* check return code */
      switch(rfc_rc)
      {
        case RFC_OK:
          break;
        case RFC_EXCEPTION :
        case RFC_SYS_EXCEPTION :
          rfc_error(exception);
          break;
        default :
          rfc_error(cU("RfcCreateTransID"));
          break;
      }

      /* Put TID with data_file in TID-Management */
      printfU (cU("\n\nUpdate state of the transaction (CREATED).......\n"));
      tid_rc = update_TID(tid, CREATED, data_file);

      switch(tid_rc)
      {
        case TID_ERROR_LOCK:
          app_error(tid_etext03);
          break;

        case TID_ERROR_UPDATE:
          app_error(tid_etext04);
          break;

        default:
          break;
      }

      simulation_test();

    }
  }

  if (tid_search_rc != TID_EXECUTED_FOUND)
  {
    /* Move data from input file into table TCPICDAT */
    rc = write_file_to_itab(data_fp, tables[0].ithandle);

    if (rc)
      rfc_error(tid_etext10);

    /* Transfer data to R/3 via tRFC-Interface */
    printfU (cU("\n<==  RfcIndirectCallEx '%s'     rfc_rc = "), function);
    rfc_rc = RfcIndirectCallEx (rfc_handle,
                                function,
                                exporting,
                                tables,
                                tid);
    printfU (cU("%d\n"), rfc_rc);

    /* check return code */
    switch(rfc_rc)
    {
      case RFC_OK:
        break;
      case RFC_EXCEPTION :
      case RFC_SYS_EXCEPTION :
        rfc_error(exception);
        break;
      default :
        rfc_error(cU("RfcIndirectCall"));
        break;
    }

    simulation_test();

    /* Update TID with data_file in TID-Management */
    printfU (cU("\n\nUpdate state of the transaction (EXECUTED).....\n"));
    tid_rc = update_TID(tid, EXECUTED, data_file);

    switch(tid_rc)
    {
      case TID_ERROR_LOCK:
        app_error(tid_etext05);
        break;

      case TID_ERROR_UPDATE:
        app_error(tid_etext06);
        break;

      default:
        break;
    }

    simulation_test();
  }

  /* Confirm the transaction */
  printfU (cU("\n<==  RfcConfirmTransID '%s'     rfc_rc = "), tid);
  rfc_rc = RfcConfirmTransID(rfc_handle, tid);
  printfU (cU("%d\n"), rfc_rc);

  /* check return code */
  switch(rfc_rc)
  {
    case RFC_OK:
      break;
    case RFC_EXCEPTION :
    case RFC_SYS_EXCEPTION :
      rfc_error(exception);
      break;
    default :
      rfc_error(cU("RfcConfirmTransID"));
      break;
  }

  simulation_test();

  /* Update TID with data_file in TID-Management */
  printfU (cU("\n\nUpdate state of the transaction (CONFIRMED).....\n"));
  tid_rc = update_TID(tid, CONFIRMED, data_file);

  switch(tid_rc)
  {
    case TID_ERROR_LOCK:
      app_error(tid_etext07);
      break;

    case TID_ERROR_UPDATE:
      app_error(tid_etext08);
      break;

    default:
      break;
  }

  /*----------------------------------------------------------------*/
  /* Print specific data of this RFC connection                     */
  /*----------------------------------------------------------------*/
  printfU (cU("\n<==  RfcGetAttributes         rc = "));
  rc = RfcGetAttributes(rfc_handle, &rfc_attributes);
  printfU (cU("%d\n"), rc);
  if (rc)
    rfc_error(cU("RfcGetAttributes"));

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
  printfU (cU("\nCPI-C Conversation ID  :  %s\n"), rfc_attributes.CPIC_convid);

  printfU (cU("\n\nData successfully transferred to SAP R/3 via transactional RFC\n"));

  /*------------------------------------------------------------------*/
  /* Close RFC connection                                             */
  /*------------------------------------------------------------------*/
  printfU (cU("\n<==  RfcClose\n"));
  RfcClose(rfc_handle);

  /*------------------------------------------------------------------*/
  /* Delete internal table                                            */
  /*------------------------------------------------------------------*/
  printfU (cU("\n<==  ItDelete (TCPICDAT)\n"));
  ItDelete(tables[0].ithandle);

  return 0;
}


/*--------------------------------------------------------------------*/
/* Prepare and Open a RFC-Connection                                  */
/*--------------------------------------------------------------------*/
int RFC_connect()
{
    printfU (cU("\n"));

    /* install error handler */
    new_env.allocate     = NULL;
    new_env.errorhandler = rfc_error;
    
    printfU (cU("\n<==  RfcEnvironment (Install Error Handling Function)\n"));

    RfcEnvironment(&new_env);


    memsetR (connect_param, 0, sizeofR(connect_param));


    sprintfU (connect_param, cU("DEST=%s"), rfc_dest);


    if (sapsys[0] == cU('3'))
    {
        if (memcmpU (client, cU("   "), 3) != 0)
         sprintfU (connect_param+strlenU (connect_param), cU(" CLIENT=%s"), client);

        sprintfU (connect_param+strlenU (connect_param), cU(" USER=%s PASSWD=%s"), userid, password);

        if (language[0] != cU(' '))
         sprintfU (connect_param+strlenU (connect_param), cU(" LANG=%s"), language);
    }
    else
     sprintfU (connect_param+strlenU (connect_param), cU(" TYPE=E"));


#ifdef SAPwithUNICODE
    sprintfU (connect_param+strlenU (connect_param), cU(" PCS=%c"), pcs[0]);
#endif


    /* Open a RFC connection */
    printfU (cU("\n<==  RfcOpenEx                rfc_handle = "));

    rfc_handle = RfcOpenEx(connect_param, &error_info);
    
    printfU (cU("%d\n"), (int) rfc_handle);

    if (rfc_handle == RFC_HANDLE_NULL)
    {
        printfU (cU("\nGroup       Error group %d\n"), error_info.group);
        printfU (cU("Key         %s\n"), error_info.key);
        printfU (cU("Message     %s\n\n"), error_info.message);
        exit(1);
    }

    return 0;
}




/*--------------------------------------------------------------------*/
/* Error Cleanup because of an Application Error                      */
/*--------------------------------------------------------------------*/
void app_error(rfc_char_t *etext)
{
    if (function != NULL)
     printfU (cU("\nFUNCTION: %s\n"), function);


    printfU (cU("\nError Message:  %s\n"), etext);


    printfU (cU("\n<==  RfcClose\n"));
    
    
    RfcClose(RFC_HANDLE_NULL);
    
    
    exit(1);
}


/*--------------------------------------------------------------------*/
/* Error Cleanup because of a RFC-Error                               */
/*--------------------------------------------------------------------*/
void DLL_CALL_BACK_FUNCTION rfc_error (rfc_char_t *operation)
{
    printfU (cU("\n<==  RfcLastErrorEx\n"));


    RfcLastErrorEx (&error_info);


    printfU (cU("\nRFC Call/Exception: %s\n"),   operation);
    printfU (cU("Group       Error group %d\n"), error_info.group);
    printfU (cU("Key         %s\n"),             error_info.key);
    printfU (cU("Message     %s\n\n"),           error_info.message);


    printfU (cU("\n<==  RfcClose\n"));
    
    RfcClose (RFC_HANDLE_NULL);


    exit (1);
}




/*--------------------------------------------------------------------*/
/* Search for certain parameter                                       */
/*--------------------------------------------------------------------*/
int PM_search (rfc_char_t *param,
               rfc_char_t *text,
               int  leng)
{
  rfc_char_t  *pa,
              *pe;

  memsetU (param, cU('\0'), leng);
  pa = strstrU (tbuf, text);
  if (pa == NULL)
    return 99;

  pa = pa + strlenU (text);

  pe = memchrU (pa, cU(' '), leng);
  if (pe == NULL)
    znr = &tbuf[strlenU (tbuf)] - pa;
  else
    znr = pe - pa;

  if (znr == 0)
    return 99;

  if (znr > leng)
    znr = leng;

  memcpyU (param, pa, znr);

  return 0;
}


/*--------------------------------------------------------------------*/
/* Get parameters for tRFC test                                       */
/*--------------------------------------------------------------------*/
void get_parameter_for_trfc_test()
{
  /* Open the work file */
  para_fp = fopenU (para_file, cU("r"));
  if (para_fp == NULL)
  {
    get_connect_data_via_dialog();
    get_trfc_test_data_via_dialog();
    save_test_parameter();
  }
  else
  {
    get_connect_data_from_file();
    get_trfc_test_data_from_file();
    if (para_fp != NULL)
    {
      fclose(para_fp);
      para_fp = NULL;
    }
  }

  return;
}


/*--------------------------------------------------------------------*/
/* Get connect data from file                                         */
/*--------------------------------------------------------------------*/
void get_connect_data_from_file(void)
{
  read_record(para_fp, sbuf1);
  strcpyU (tbuf, sbuf1);
  read_record(para_fp, sbuf2);


  /* Destination in 'saprfc.ini' */
  memsetU (rfc_dest, cU('\0'), 32);
  rc = PM_search(rfc_dest, cU("DEST="), 32);
  if (rc)
    strcpyU (rfc_dest, cU("BIN"));
  printfU (cU("\n\n\n               Destination in 'saprfc.ini'.............: %s"), rfc_dest);


  /* RFC server is SAP R/3 or External */
  memsetU (sapsys, cU('\0'), sizeofU(sapsys));
  rc = PM_search (sapsys, cU("SAPSYS="), 1);
  if (rc)
    sapsys[0] = cU('3');
  printfU (cU("\n               Server is R/3 or External (3/E).........: %c"), sapsys[0]);


  #ifdef SAPwithUNICODE
  memsetU (pcs, cU('\0'), sizeofU(pcs));
  rc = PM_search (pcs, cU("PCS="), 1);
  if (rc)
   pcs[0] = cU('1');
  printfU (cU("\n               Partners Char Size (1/2)................: %c"), pcs[0]);
  #endif


  if (sapsys[0] == cU('3'))
  {
    printfU (cU("\n\n               SAP logon data:"));


    /* Client */
    memsetU (client, cU('\0'), sizeofU(client));
    rc = PM_search (client, cU("CLIENT="), 3);
    if (rc)
      strcpyU (client, cU("   "));
    printfU (cU("\n                 Client................................: %s"), client);


    /* User Id */
    memsetU (userid, cU('\0'), 12);
    rc = PM_search (userid, cU("USERID="), 12);
    if (rc)
    {
      printfU (cU("\n\nInfo about User Id (USERID=) not found or not correct"));
      exit(1);
    }


    printfU (cU("\n                 UserID................................: %s"), userid);


    /* Password */
    memsetU (password, cU('\0'), 9);
    rc = PM_search(password, cU("PASSWORD="), 8);
    if (rc)
    {
      printfU (cU("\n\nInfo about Password (PASSWORD=) not found or not correct"));
      exit(1);
    }
    printfU (cU("\n                 Password..............................: XXXXXXXX"));
    znr = strlenU (password);
    for (i = 0; i < znr; i++)
      password[i] = (password[i] + 30 + i);


    /* Language */
    memsetU (language, cU('\0'), 3);
    rc = PM_search (language, cU("LANGUAGE="), 2);
    if (rc)
      strcpyU (language, cU(" "));
    printfU (cU("\n                 Language..............................: %s"), language);
  }

  return;
}


/*--------------------------------------------------------------------*/
/* Get password for SAP logon                                         */
/*--------------------------------------------------------------------*/
void get_passwd(rfc_char_t * pwd)
{
#if defined(SAPonUNIX)
  int  i, c;
  rfc_char_t ibuf[256];

  systemU (cU("stty -echo"));
  getsU (ibuf);
  systemU (cU("stty echo"));

  strcpyU (pwd, ibuf);
  i = strlenU (pwd);
  if (i > 40)
    i = 40;
  pwd[i] = 0;
  memcpyU(ibuf, cU("****************************************"), i);
  printfU (cU("%s\n"), ibuf);

#elif defined(SAPonNT)
#include <conio.h>
  int  i;
  rfc_char_t c;
  rfc_char_t ibuf[256];

  for (i=0; i<8; i++)
  {
    c = getch ();
    if (c != cU('\r'))
    {
      pwd[i] = c;
      printfU (cU("*"));
    }
    else
    {
      pwd[i] = cU('\0');
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
/* Get connect data via Dialog                                        */
/*--------------------------------------------------------------------*/
void get_connect_data_via_dialog(void)
{
  memsetU (tbuf, cU('\0'), 512);
  
  printfU (cU("\n\n"));

  /* Destination in 'saprfc.ini' */
  if (strlenU (rfc_dest) == 0)
    strcpyU (rfc_dest, cU("BIN"));

  printfU (cU("               Destination in 'saprfc.ini'.............: (%s): "), rfc_dest);
  
  fflush(stdout);
  
  getsU (ips);
  if (strlenU (ips) == 0)
    strcpyU (ips, rfc_dest);
  ips[32] = 0;
  
  strcpyU (rfc_dest, ips);
  
  for (i = 0; i < (int) strlenU (rfc_dest); i++)
    rfc_dest[i] = (rfc_char_t) toupperU (rfc_dest[i]);
  
  sprintfU (tbuf+strlenU (tbuf), cU("DEST=%s "), rfc_dest);


  /* RFC server is SAP R/2 or R/3 or External */
  if (strlenU (sapsys) == 0)
    sapsys[0] = cU('3');
  
  do
  {
    printfU (cU("               Server is R/3 or External (3/E)3........: (%c): "), sapsys[0]);
    fflush(stdout);
    getsU (ips);
    if (strlenU (ips) == 0)
      ips[0] = sapsys[0];
    ips[0] = toupperU (ips[0]);
  } while ((ips[0] != cU('3')) && (ips[0] != cU('E')));
  
  sapsys[0] = ips[0];
  
  sprintfU (tbuf+strlenU (tbuf), cU("SAPSYS=%c "), sapsys[0]);


  #ifdef SAPwithUNICODE
  if (strlenU (pcs) == 0)
   pcs[0] = cU('1');

  do
  {
    printfU (cU("               Partners Char Size (1/2)1...............: (%c): "), pcs[0]);

    fflush(stdout);
    
    getsU (ips);
    
    if (strlenU (ips) == 0)
      ips[0] = pcs[0];
    
    pcs[0] = toupperU (ips[0]);
  } while ((ips[0] != cU('1')) && (ips[0] != cU('2')));

  pcs[0] = ips[0];

  sprintfU (tbuf+ strlenU (tbuf), cU("PCS=%c "), pcs[0]);
  #endif


  if (sapsys[0] == cU('3'))
  {
    printfU (cU("\n               SAP logon data:\n"));

    /* Client */
    if (strlenU (client) == 0)
      strcpyU (client, cU("000"));
  
    printfU (cU("                 Client................................: (%s): "), client);
    fflush (stdout);

    getsU (ips);
    if (strlenU (ips) == 0)
      strcpyU (ips, client);
    else
      strcpyU (client, cU("000"));
    ips[3] = 0;

    strcpyU (client, ips);
    
    if (client[0] == cU(' '))
      strcpyU (client, cU("   "));
    
    if (memcmpU (client, cU("   "), 3) != 0)
      sprintfU (tbuf+strlenU (tbuf), cU("CLIENT=%s "), client);

    /* User Id */
    if (strlenU (userid) == 0)
      strcpyU (userid, cU("SAPCPIC"));
    
    printfU (cU("                 UserID................................: (%s): "), userid);

    fflush (stdout);
    
    getsU (ips);
    if (strlenU (ips) == 0)
      strcpyU (ips, userid);
    ips[12] = 0;

    strcpyU (userid, ips);
    
    for (i = 0; i < (int) strlenU (ips); i++)
      userid[i] = toupperU (userid[i]);
    
    sprintfU (tbuf+strlenU (tbuf), cU("USERID=%s "), userid);

    /* Password */
    if (strlenU (password) == 0)
      strcpyU (password, cU("admin"));
    
    printfU (cU("                 Password..............................: (XXXXXXXX):"));
    
    fflush (stdout);
    
    get_passwd (ips);
    if (strlenU (ips) == 0)
      strcpyU (ips, password);
    ips[8] = 0;

    strcpyU (password, ips);
    
    for (i = 0; i < (int) strlenU (password); i++)
      password[i] = (toupperU (password[i]) - 30 - i);
    
    sprintfU (tbuf+strlenU (tbuf), cU("PASSWORD=%s "), password);
    
    for (i = 0; i < (int) strlenU (password); i++)
      password[i] = (rfc_char_t) (password[i] + 30 + i);

    /* Language */
    if (strlenU (language) == 0)
      strcpyU (language, cU("E"));
    
    printfU (cU("                 Language (E)..........................: (%s): "), language);
    fflush (stdout);
    
    getsU (ips);
    
    if (strlenU (ips) == 0)
      strcpyU (ips, language);
    ips[2] = 0;
    
    strcpyU (language, ips);

    if (language[0] != cU(' '))
      sprintfU (tbuf+strlenU (tbuf), cU("LANGUAGE=%s \n"), language);
  }


  return;
}


/*--------------------------------------------------------------------*/
/* Save parameters for next tests                                     */
/*--------------------------------------------------------------------*/
void save_test_parameter()
{

  /* Open the para file */
  para_fp = fopenU (para_file, cU("w"));
  if (para_fp == NULL)
  {
    printfU (cU("\nCan not save RFC-parameters for next tests\n"));
    printfU (cU("\nHit return key to continue"));
    fflush (stdout);
    getcharU ();
  }
  else
  {
    fputsU (tbuf, para_fp);
    fclose (para_fp);
    para_fp = NULL;
  }
  return;
}


/*--------------------------------------------------------------------*/
/* Get trfc test data from file                                       */
/*--------------------------------------------------------------------*/
void get_trfc_test_data_from_file(void)
{

  strcpyU (tbuf, sbuf2);

  printfU (cU("\n\n               Transactional RFC Test Data:"));

  /* Filename of user data  */
  rc = PM_search (data_file, cU("DATAFILE="), 64);
  if (rc)
    sprintfU (data_file, cU("%s%s"), working_dir, std_data_file);

  printfU (cU("\n                 File name of user data.................: %s"), data_file);

  printfU (cU("\n\n"));
  do
  {
    printfU (cU("               Do You Want To Test With These Parameters (Y/N)....: "));
    fflush (stdout);
    getsU (ips);
    ips[0] = toupperU (ips[0]);
  } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));

  if (ips[0] == cU('N'))
  {
    if (para_fp != NULL)
    {
      fclose(para_fp);
      para_fp = NULL;
    }
    get_connect_data_via_dialog ();
    get_trfc_test_data_via_dialog ();
    save_test_parameter ();
  }

  return;
}


/*--------------------------------------------------------------------*/
/* Get trfc test data via dialog                                      */
/*--------------------------------------------------------------------*/
void get_trfc_test_data_via_dialog(void)
{
  printfU (cU("\n               Transactional RFC Test Data:\n"));

  /* Filename of user data  */
  if (strlenU (data_file) == 0)
    strcpyU (data_file, std_data_file);
  printfU (cU("                 File name of user data.................: (%s): "), data_file);
  fflush (stdout);
  getsU (ips);
  if (strlenU (ips) == 0)
    strcpyU (ips, data_file);
  if (ips[0] == cU('/'))
    strcpyU (data_file, ips);
  else
    sprintfU (data_file, cU("%s%s"), working_dir, ips);

  sprintfU (tbuf+strlenU (tbuf), cU("DATAFILE=%s \n"), data_file);

  printfU (cU("\n"));
  do
  {
    printfU (cU("               Do You Want To Test With These Parameters (Y/N)....: "));
    fflush (stdout);
    getsU (ips);
    ips[0] = toupperU (ips[0]);
  } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));

  if (ips[0] == cU('N'))
  {
    if (para_fp != NULL)
    {
      fclose(para_fp);
      para_fp = NULL;
    }
    get_connect_data_via_dialog();
    get_trfc_test_data_via_dialog();
    save_test_parameter();
  }

  return;
}


/*--------------------------------------------------------------------*/
/* read record from file with single or multi line                    */
/*--------------------------------------------------------------------*/
rfc_char_t *read_record (FILE *fp,
                         rfc_char_t *ibuf)
{
  int           offset = 0;
  rfc_char_t    line[512+1];

  ibuf[0] = cU('\0');

  for (;;)
  {
    if ((ptr = fgetsU (line, 512, fp)) == NULL)
      return (NULL);

    if (line[strlenU (line)-1] == cU('\n'))
      line[strlenU (line)-1] = cU('\0');

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
      ibuf[strlenU (ibuf)-1] = cU('\0');
      offset = strlenU (ibuf);
      continue;
    }
    break;
  }
  return (ibuf);
}


/*--------------------------------------------------------------------*/
/* Search for TID in the TID-Management                               */
/*--------------------------------------------------------------------*/
static TID_RC search_TID(RFC_TID tid, rfc_char_t *datafile)
{
  TID_RC        tidrc;
  FILE          *fp;

  memsetU (tid, cU('\0'), RFC_TID_LN);

  /* Try to lock TID-Management */
  if (lock_TID(lock_file, tid))
    return TID_ERROR_UPDATE;

  /* Read TID-file for search */
  fp = fopenU (tid_file, cU("rb"));
  if (fp == NULL)
  {
    /* No TID-file existent */
    fp = fopenU (tid_file, cU("ab"));
    if (fp == NULL)
    {
      /* ERROR: Create file for TID-Management failed */
      unlock_TID(lock_file);
      return TID_ERROR_UPDATE;
    }

    memsetU (tbuf, cU('\0'), LINE_SIZE_CHAR);
    strcpyU (tbuf, cU("***   TID-MANAGEMENT FOR TRANSACTIONAL RFC (Client Program)   ***\n"));
    if (fputsU (tbuf, fp) < 0)
      /* ERROR: Write in TID-Management failed */
      tidrc = TID_ERROR_UPDATE;
    else
      tidrc = TID_OK;

    /* Unlock TID-Management */
    unlock_TID(lock_file);
    fclose(fp);
    return tidrc;
  }

  while ((ptr = fgetsU (tbuf, LINE_SIZE_CHAR+1, fp)) != NULL)
  {
    if ( (memcmpU (tbuf+63, datafile, strlenU (datafile)) == 0) 
         &&
         (
          (memcmpU (tbuf+52, cU("CREATED"),             7) == 0) 
          ||
          (memcmpU (tbuf+52, cU("EXECUTED"), 8)            == 0)
         )
       )
      break;
  }

  if (ptr != NULL)
  {
    if (memcmpU (tbuf+52, cU("CREATED"), 7) == 0)
    {
      /* Previous TID in state CREATED found: Repeat transfer to R/3 */
      memcpyU (tid, tbuf+26, RFC_TID_LN);
      tidrc = TID_CREATED_FOUND;
    }
    else
    {
      /* Previous TID in state EXECUTED found: Confirmation necessary */
      memcpyU (tid, tbuf+26, RFC_TID_LN);
      tidrc = TID_EXECUTED_FOUND;
    }

  }
  else
    /* No previous TID with data file found: New transfer to R/3 */
    tidrc = TID_OK;

  /* Unlock TID-Management */
  unlock_TID (lock_file);
  fclose (fp);
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

  /* If new TID write entry at the end of the TID-Management */
  if (tid_state == CREATED)
  {
    /* Open TID-file for update */
    fp = fopenU (tid_file, cU("ab"));
    if (fp == NULL)
    {
      /* ERROR: Open TID-Management failed */
      unlock_TID(lock_file);
      return TID_ERROR_UPDATE;
    }
    else
    {
      if (write_TID(fp, tid, tid_state, datafile))
        /* ERROR: Write in TID-Management failed */
        tidrc = TID_ERROR_UPDATE;
      else
	tidrc = TID_OK;

      /* Unlock TID-Management */
      unlock_TID(lock_file);
      fclose(fp);
      return tidrc;
    }
  }

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
    if (memcmpU (tbuf+26, tid, RFC_TID_LN) == 0)
      break;
    offset = offset + strlenU (tbuf)*sizeofR(SAP_UC);
  }

  if ((ptr != NULL) &&
      (fseek (fp, offset, SEEK_SET) == 0) &&
      (write_TID (fp, tid, tid_state, datafile) == 0))
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
  memcpyU (datetime, asctimeU(time_ptr), 24);

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
static int lock_TID(rfc_char_t *lockfile, RFC_TID tid)
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
      memcpyU (datetime, asctimeU(time_ptr), 24);

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
      memcpyU (datetime, asctimeU(time_ptr), 24);

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
      memcpyU (datetime, asctimeU(time_ptr), 24);

      sprintfU (tbuf, cU("%s  %s  EXECUTING  \n"), datetime, tid);
      write(lock_fd, tbuf, LINE_SIZE_RAW);
      return 0;
    }
  }
  return 1;

#elif defined(SAPonOS2_2x)
  while (try_lock)
  {
    lock_fd = openU (lockfile, O_CREAT|O_RDWR, S_IREAD|S_IWRITE);
    if (lock_fd == -1)
    {
      DosSleep(1000L);
      try_lock--;
    }
    else
    {
      memsetU (tbuf, cU('\0', LINE_SIZE_CHAR);
      actutime = time(NULL);
      /*CCQ_CLIB_LOCTIME_OK*/
      time_ptr = localtime(&actutime);
      memcpyU (datetime, asctimeU(time_ptr), 24);

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
     if ((lock_fd = openU(lockfile, O_CREAT|O_RDWR)) == -1)
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
       memcpyU (datetime, asctimeU(time_ptr), 24);

       sprintfU (tbuf, cU("%s  %s  EXECUTING  \n"), datetime, tid);
       write (lock_fd, (void*)tbuf, LINE_SIZE_RAW);
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
      memsetU (tbuf, cU(\'0'), LINE_SIZE_CHAR);
      actutime = time(NULL);
      /*CCQ_CLIB_LOCTIME_OK*/
      time_ptr = localtime(&actutime);
      memcpyU (datetime, asctimeU(time_ptr), 24);

      sprintfU (tbuf, cU("%s  %s  EXECUTING  \n"), datetime, tid);
      write (lock_fd, tbuf, LINE_SIZE_RAW);
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
  close(lock_fd);
  unlinkU (lockfile);
  return 0;

#elif defined(SAPonNT) || defined(SAPonVMS) || defined(SAPonOS2_2x)
  close(lock_fd);
  return 0;

#else
#error "Error Message > Please update this function for your OS"
#endif
}


/*--------------------------------------------------------------------*/
/* Write file into internal table                                     */
/*--------------------------------------------------------------------*/
int write_file_to_itab(FILE *fp, ITAB_H itab_h)
{
  int           rc = 0,
                it_leng;
  rfc_char_t    itab_buf[8192+2];


  it_leng = ItLeng(itab_h)/sizeofR(SAP_UC);

  while ((ptr = fgetsU (itab_buf, 8192+2, fp)) != NULL)
  {
    ptr = (rfc_char_t *) ItAppLine(itab_h);
    if (ptr != NULL)
    {
      int leng = strlenU (itab_buf);
      if ((leng > 0) && (itab_buf[leng-1] == cU('\n')))
        leng = leng - 1;
      if (leng == 0)
      {
        memsetU (ptr, BLANK, it_leng);
        continue;
      }
      if (leng < it_leng)
        memsetU (itab_buf+leng, BLANK, it_leng-leng);
      memcpyU (ptr, itab_buf, it_leng);
    }
    else
    {
      rc = 1;
      break;
    }
  }
  return rc;
}


/*--------------------------------------------------------------------*/
/* Simulation test                                                    */
/*--------------------------------------------------------------------*/
void simulation_test()
{
  printfU (cU("\n"));
  do
  {
    printfU (cU("Continue or Break for Simulation Test (C/B)C..: "));
    fflush (stdout);
    getsU (ips);
    if (strlenU (ips) == 0)
      ips[0] = cU('C');
      ips[0] = toupperU (ips[0]);
  } while ((ips[0] != cU('C')) && (ips[0] != cU('B')));

  if (ips[0] == cU('B'))
  {
    printfU (cU("\n"));
    exit(1);
  }
  return;
}

