/* @(#) $Id: //bas/640_REL/src/krn/rfc/trfcserv.h#1 $ SAP*/

/*====================================================================*/
/*                                                                    */
/*     PROGRAM     :   trfcserv.h (running on Windows_NT, Windows_95, */
/*                                 OS/2 and R/3-platforms)            */
/*                                                                    */
/*                                                                    */
/*     DESCRIPTION :   Includes file for Test with transactional RFC  */
/*                                                                    */
/*                                                                    */
/*     SAP AG Walldorf                                                */
/*     Systeme, Anwendungen und Produkte in der Datenverarbeitung     */
/*                                                                    */
/*     Copyright (C) SAP AG 1995                                      */
/*                                                                    */
/*====================================================================*/


/*--------------------------------------------------------------------*/
/* Defines                                                            */
/*--------------------------------------------------------------------*/

#define BLANK                  cU(' ')
#define MAXBUFF                8192
#define LINE_SIZE_CHAR         256
#define LINE_SIZE_RAW          256*sizeofR(SAP_UC)
#define TCPICDAT_ENTRY_SIZE    72
#define NL_AR                  iU("\n")
#define NL                     cU("\n")


/*--------------------------------------------------------------------*/
/* Set up includes                                                    */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* SAP flag for OS/2 */
#if defined(OS2)
#  ifndef SAPonOS2_2x
#    define SAPonOS2_2x
#  endif
 /* map os/2 DosSleep to NT Sleep call */
   extern unsigned short _System Sleep( unsigned long msec );
#  pragma import(Sleep,,"DOSCALLS",229)
#endif

/* SAP flag for Windows NT or 95 */
#ifdef _WIN32
#  ifndef SAPonNT
#    define SAPonNT
#  endif
#endif

/* SAP flag for Windows 3.xx */
#ifdef _WINDOWS
#  ifndef _WIN32
#    ifndef SAPonWINDOWS
#      define SAPonWINDOWS
#    endif
#  endif
#endif



#include "saprfc.h"
#include "sapitab.h"

#ifdef DGUX
#include <sys/time.h>
#endif

#ifdef SAPonUNIX
#include <fcntl.h>
#include <unistd.h>

#elif defined(SAPonNT)
#include <windows.h>
#include <winsock.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <process.h>
#include <io.h>

#elif defined(SAPonOS2_2x)
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <process.h>
#include <io.h>
#define INCL_DOSPROCESS
#include <os2.h>

#elif defined(SAPonVMS)
#include <file.h>

#elif defined(SAPonOS400)
#include <unistd.h>
#include <fcntl.h>

#elif defined(SAPonOS390)
#include <unistd.h>
#include <fcntl.h>

#else
#error "Error Message > Please update the lock/unlock functions for your OS"
#endif

/*--------------------------------------------------------------------*/
/* Type Definitions                                                   */
/*--------------------------------------------------------------------*/

/* Return code from TID Management */
typedef enum
{
  TID_OK,		/* TID function OK       */
  TID_FOUND,      	/* TID already exists    */
  TID_ERROR_LOCK, 	/* TID lock failed       */
  TID_ERROR_UPDATE      /* TID update failed     */
}
TID_RC;

/* State of TID in the TID Management */
typedef enum
{
  CREATED,              /* Transaction created   */
  EXECUTED,		/* Transaction executed  */
  CONFIRMED,		/* Transaction confirmed */
  ROLLBACK 		/* Transaction rollback  */
}
TID_STATE;


/*--------------------------------------------------------------------*/
/* Function prototypes                                                */
/*--------------------------------------------------------------------*/

static RFC_RC install(void);

static int  DLL_CALL_BACK_FUNCTION  TID_check(RFC_TID tid);

static void DLL_CALL_BACK_FUNCTION  TID_commit(RFC_TID tid);

static void DLL_CALL_BACK_FUNCTION  TID_confirm(RFC_TID tid);

static void DLL_CALL_BACK_FUNCTION  TID_rollback(RFC_TID tid);

#ifdef SAP_RFC_GETNAME
static RFC_RC DLL_CALL_BACK_FUNCTION user_global_server(RFC_HANDLE rfc_handle);

static rfc_char_t *user_global_server_docu(void);
#endif

static RFC_RC DLL_CALL_BACK_FUNCTION transactional_test(RFC_HANDLE rfc_handle);

static rfc_char_t *transactional_test_docu(void);

static void init_TID(void);

static TID_RC check_TID(RFC_TID tid, rfc_char_t *datafile);

static TID_RC update_TID(RFC_TID tid, TID_STATE tid_state, rfc_char_t *datafile);

static int lock_TID(rfc_char_t *lockfile, RFC_TID tid);

static int unlock_TID(rfc_char_t *lockfile);

static int write_TID(FILE *fp, RFC_TID tid, TID_STATE tid_state, rfc_char_t *datafile);

void DLL_CALL_BACK_FUNCTION rfc_error(rfc_char_t *operation );

void TRFC_trace(rfc_char_t *text);

void function_abort(rfc_char_t *atext);

void rfc_param (RFC_PARAMETER *rfc_param,
                rfc_char_t          *abap_field_name,
                unsigned      data_type,
                void          *data_addr,
                unsigned      data_len );

int write_itab_to_file(ITAB_H itab_h, rfc_char_t *filename, rfc_char_t *itab_name);

static void help(void);

static void create_file_name(rfc_char_t *filename);


/*--------------------------------------------------------------------*/
/* Declarations                                                       */
/*--------------------------------------------------------------------*/

int               i,
                  j,
                  rc,
                  znr;

long                file_nr = 0;

rfc_char_t          ips[101],
                    tpname[101],
                    tphost[101],
                    gwhost[101],
                    gwserv[9];

rfc_char_t          tid_file[65],
                    data_file[65],
                    lock_file[65],
                    trace_file[65],
                    working_dir[65],
                    file_nr_field[11],
                    std_tid_file[13]  = iU("trfcserv.tid"),
                    std_lock_file[13] = iU("trfcserv.lck");

FILE               *trace_fp;

int                 lock_fd;

rfc_char_t         *ptr,
                    datetime[25],
                    abort_text[80],
                    tbuf[MAXBUFF+1],
                    last_error_text[150];

struct tm          *time_ptr;

time_t              actutime;

rfc_char_t         *exception = NULL,
                   *function  = NULL;

RFC_RC              rfc_rc;
RFC_HANDLE          rfc_handle;
RFC_FUNCTIONNAME    function_name;
RFC_ATTRIBUTES      rfc_attributes;

RFC_PARAMETER       parameters[1];

RFC_TABLE           tables[2];

RFC_TID             TransID;

rfc_char_t          name_transactional_test[31] = iU("STFC_WRITE_TO_TCPIC");

#ifdef SAP_RFC_GETNAME
rfc_char_t          name_user_global_server[31] = cU("%%USER_GLOBAL_SERVER");
#endif

TID_RC              tid_rc;

TID_STATE           tid_state;

rfc_char_t          tid_etext00[] = iU("Create_TID: Cannot create the TID-Management"),

                    tid_etext01[] = iU("Check_TID: Lock the TID-Management failed"),
                    tid_etext02[] = iU("Check_TID: Update the TID-Management failed"),

                    tid_etext03[] = iU("Commit_TID: Lock the TID-Management failed"),
                    tid_etext04[] = iU("Commit_TID: Update the TID-Management failed"),

                    tid_etext05[] = iU("Confirm_TID: Lock the TID-Management failed"),
                    tid_etext06[] = iU("Confirm_TID: Update the TID-Management failed"),

                    tid_etext07[] = iU("Rollback_TID: Lock the TID-Management failed"),
                    tid_etext08[] = iU("Rollback_TID: Update the TID-Management failed"),

                    tid_etext10[] = iU("Cannot write data in received file");

RFC_CALL_INFO       rfc_call_info;
