/* @(#) $Id: //bas/640_REL/src/krn/rfc/trfctest.h#2 $ SAP*/

/*====================================================================*/
/*                                                                    */
/*     PROGRAM     :   trfctest.h (running on Windows_NT, Windows_95, */
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
#endif

/* SAP flag for Windows NT or 95 */
#ifdef _WIN32
#  ifndef SAPonNT
#    define SAPonNT
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
#include <io.h>

#elif defined(SAPonOS2_2x)
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
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

#error "Error Message > Please update the lock_TID function for on your OS"
#endif


/*--------------------------------------------------------------------*/
/* Type Definitions                                                   */
/*--------------------------------------------------------------------*/

/* Return code from TID Management */
typedef enum
{
  TID_OK,               /* TID function OK            */
  TID_CREATED_FOUND,    /* TID in state CREATED found */
  TID_EXECUTED_FOUND,   /* TID already EXECUTED found */
  TID_ERROR_LOCK,       /* TID lock failed            */
  TID_ERROR_UPDATE      /* TID update failed          */
}
TID_RC;

/* State of TID in the TID Management */
typedef enum
{
  CREATED,              /* Transaction created   */
  EXECUTED,             /* Transaction executed  */
  CONFIRMED,            /* Transaction confirmed */
  ROLLBACK              /* Transaction rollback  */
}
TID_STATE;


/*--------------------------------------------------------------------*/
/* Function prototypes                                                */
/*--------------------------------------------------------------------*/

int srfctran(void);

void get_parameter_for_conn_test(void);

void get_parameter_for_trfc_test(void);

void get_connect_data_from_file(void);

void get_connect_data_via_dialog(void);

void get_trfc_test_data_from_file(void);

void get_trfc_test_data_via_dialog(void);

void save_test_parameter(void);

int RFC_connect(void);

static TID_RC search_TID(RFC_TID tid, rfc_char_t *datafile);

static TID_RC update_TID(RFC_TID tid, TID_STATE tid_state, rfc_char_t *datafile);

static int lock_TID(rfc_char_t *lockfile, RFC_TID tid);

static int unlock_TID(rfc_char_t *lockfile);

static int write_TID(FILE *fp, RFC_TID tid, TID_STATE tid_state, rfc_char_t *datafile);

int write_file_to_itab(FILE *fp, ITAB_H itab_h);

rfc_char_t *read_record(FILE *fp, rfc_char_t *ibuf);

int PM_search(rfc_char_t *param, rfc_char_t *text, int leng);

void DLL_CALL_BACK_FUNCTION rfc_error(rfc_char_t *operation);


void app_error(rfc_char_t *etext);

void simulation_test(void);

/*--------------------------------------------------------------------*/
/* Declarations                                                       */
/*--------------------------------------------------------------------*/

int                     i,
                        j,
                        rc,
                        znr,
                        syntax_OK;

rfc_char_t              tid_file[65],
                        para_file[65],
                        data_file[65],
                        lock_file[65],
                        working_dir[65],
                        std_tid_file[13]  = iU("trfctest.tid"),
                        std_para_file[13] = iU("trfctest.txt"),
                        std_data_file[13] = iU("trfctest.dat"),
                        std_lock_file[13] = iU("trfctest.lck");

rfc_char_t             *ptr,
                        ips[101],
                        datetime[25],
                        sbuf1[513],
                        sbuf2[513],
                        tbuf[MAXBUFF+1];

rfc_char_t              rfc_dest[33],
                        sapsys[2],
                        client[4],
                        userid[13],
                        password[9],
                        language[3],
#ifdef SAPwithUNICODE
                        pcs[2],
#endif
                        connect_param[1024];

rfc_char_t              abort_text[80],
                        last_error_text[150];

FILE                   *para_fp,
                       *data_fp;

int                     lock_fd;

struct tm              *time_ptr;

time_t                  actutime;


rfc_char_t             *exception = NULL,
                       *function  = NULL;

RFC_RC                  rfc_rc;
RFC_ENV                 new_env;
RFC_HANDLE              rfc_handle;
RFC_FUNCTIONNAME        function_name;
RFC_ERROR_INFO_EX       error_info;
RFC_ATTRIBUTES          rfc_attributes;

RFC_PARAMETER           exporting[1],
                        importing[1],
                        parameters[1];

RFC_TABLE               tables[2];

RFC_TID 	            tid;

TID_RC		            tid_rc,
                        tid_search_rc;

TID_STATE	            tid_state;

rfc_char_t              name_transactional_test[31]   = iU("STFC_WRITE_TO_TCPIC");

rfc_char_t              tid_etext01[] = iU("Search_TID: Lock the TID-Management failed"),
                        tid_etext02[] = iU("Search_TID: Update the TID-Management failed"),

                        tid_etext03[] = iU("Create_TID: Lock the TID-Management failed"),
                        tid_etext04[] = iU("Create_TID: Update the TID-Management failed"),

                        tid_etext05[] = iU("Execute_TID: Lock the TID-Management failed"),
                        tid_etext06[] = iU("Execute_TID: Update the TID-Management failed"),

                        tid_etext07[] = iU("Confirm_TID: Lock the TID-Management failed"),
                        tid_etext08[] = iU("Confirm_TID: Update the TID-Management failed"),

                        tid_etext10[] = iU("Cannot write data into internal table (Memory insufficient)");

