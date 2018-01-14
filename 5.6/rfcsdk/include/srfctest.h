/* @(#) $Id: //bas/640_REL/src/krn/rfc/srfctest.h#2 $ SAP*/

/*====================================================================*/
/*                                                                    */
/*     PROGRAM     :   srfctest.h (running on Windows, Windows_NT,    */
/*                                 Windows_95, OS/2 and R/3-Platforms)*/
/*                                                                    */
/*                                                                    */
/*     DESCRIPTION :   Includes file for RFC-Test                     */
/*                                                                    */
/*                                                                    */
/*     SAP AG Walldorf                                                */
/*     Systeme, Anwendungen und Produkte in der Datenverarbeitung     */
/*                                                                    */
/*     Copyright (C) SAP AG 1994                                      */
/*                                                                    */
/*====================================================================*/


/*--------------------------------------------------------------------*/
/* Defines                                                            */
/*--------------------------------------------------------------------*/
#ifndef SAPonLIN
#define _XOPEN_SOURCE_EXTENDED 1
#define _HPUX_SOURCE   /* HP  gettimeofday(), timeval, timezone */
#define _ALL_SOURCE    /* AIX gettimeofday(), timeval, timezone */
#define __EXTENSIONS__ /* SUN gettimeofday(), timeval, timezone */
#define _OSF_SOURCE    /* OSF gettimeofday(), timeval, timezone */
#ifdef SAPonRM600
#define _XOPEN_SOURCE 1 /* on rm600, because already in saptype.h */
#endif
#endif


#define BLANK                  cU(' ')
#define LINE_SIZE              1024
#define ITAB0332_ENTRY_SIZE    332*sizeofR(SAP_UC)
#define ITAB1000_ENTRY_SIZE    1000*sizeofR(SAP_UC)
#define ETAB0332_ENTRY_SIZE    332*sizeofR(SAP_UC)
#define ETAB1000_ENTRY_SIZE    1000*sizeofR(SAP_UC)

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


#if defined(SAPonUNIX) || defined(SAPonOS390) || defined(SAPonOS400)
#  include <sys/time.h>
#endif

#ifdef SAPonSUN
int gettimeofday(struct timeval *, void *);
#endif

#ifndef SAPonOS400
#  include <sys/timeb.h>
#endif /* ! SAPonOS400 */



#include "saprfc.h"
#include "sapitab.h"
#include "rfcsi.h"

#ifdef DGUX
#include <sys/time.h>
#endif


/************************************************************/
/*  Definition for inhomogeneous structure/table 'RFCTEST'  */
/************************************************************/
static RFC_TYPEHANDLE handleOfRfcTest;


/* this is generated using the genh utilty in .../rfcsdk/bin */
typedef struct RFCTEST
{
        SAP_DOUBLE      rfcfloat;
#define RFCTEST_RFCFLOAT_LN     8
#define RFCTEST_RFCFLOAT_TYP    RFCTYPE_FLOAT

        SAP_CHAR        rfcchar1 [1];
#define RFCTEST_RFCCHAR1_LN     (1 * sizeofR(SAP_CHAR))
#define RFCTEST_RFCCHAR1_CCNT   1
#define RFCTEST_RFCCHAR1_TYP    RFCTYPE_CHAR

/* Filler[s] for compiler independent alignment of fields: */
        rfc_padd_4_1bperC(0)    /* filler  for 1 byte  per SAP_UC */

        SAP_SHORT       rfcint2;
#define RFCTEST_RFCINT2_LN      2
#define RFCTEST_RFCINT2_TYP     RFCTYPE_INT2

        SAP_RAW rfcint1;
#define RFCTEST_RFCINT1_LN      1
#define RFCTEST_RFCINT1_TYP     RFCTYPE_INT1

/* Filler[s] for compiler independent alignment of fields: */
        rfc_padd_4_2bperC(0)    /* filler  for 2 bytes per SAP_UC */

        rfc_padd_4_4bperC(0)    /* filler  for 4 bytes per SAP_UC */

        SAP_CHAR        rfcchar4 [4];
#define RFCTEST_RFCCHAR4_LN     (4 * sizeofR(SAP_CHAR))
#define RFCTEST_RFCCHAR4_CCNT   4
#define RFCTEST_RFCCHAR4_TYP    RFCTYPE_CHAR

/* Filler[s] for compiler independent alignment of fields: */
        rfc_padd_4_1bperC(1)    /* filler  for 1 byte  per SAP_UC */
        rfc_padd_4_1bperC(2)    /* filler  for 1 byte  per SAP_UC */
        rfc_padd_4_1bperC(3)    /* filler  for 1 byte  per SAP_UC */

        rfc_padd_4_2bperC(1)    /* filler  for 2 bytes per SAP_UC */
        rfc_padd_4_2bperC(2)    /* filler  for 2 bytes per SAP_UC */

        SAP_INT rfcint4;
#define RFCTEST_RFCINT4_LN      4
#define RFCTEST_RFCINT4_TYP     RFCTYPE_INT

        SAP_RAW rfchex3 [3];
#define RFCTEST_RFCHEX3_LN      3
#define RFCTEST_RFCHEX3_TYP     RFCTYPE_BYTE

/* Filler[s] for compiler independent alignment of fields: */
        rfc_padd_4_2bperC(3)    /* filler  for 2 bytes per SAP_UC */

        rfc_padd_4_4bperC(1)    /* filler  for 4 bytes per SAP_UC */

        SAP_CHAR        rfcchar2 [2];
#define RFCTEST_RFCCHAR2_LN     (2 * sizeofR(SAP_CHAR))
#define RFCTEST_RFCCHAR2_CCNT   2
#define RFCTEST_RFCCHAR2_TYP    RFCTYPE_CHAR

        SAP_CHAR        rfctime [6];
#define RFCTEST_RFCTIME_LN      (6 * sizeofR(SAP_CHAR))
#define RFCTEST_RFCTIME_CCNT    6
#define RFCTEST_RFCTIME_TYP     RFCTYPE_TIME

        SAP_CHAR        rfcdate [8];
#define RFCTEST_RFCDATE_LN      (8 * sizeofR(SAP_CHAR))
#define RFCTEST_RFCDATE_CCNT    8
#define RFCTEST_RFCDATE_TYP     RFCTYPE_DATE

        SAP_CHAR        rfcdata1 [50];
#define RFCTEST_RFCDATA1_LN     (50 * sizeofR(SAP_CHAR))
#define RFCTEST_RFCDATA1_CCNT   50
#define RFCTEST_RFCDATA1_TYP    RFCTYPE_CHAR

        SAP_CHAR        rfcdata2 [50];
#define RFCTEST_RFCDATA2_LN     (50 * sizeofR(SAP_CHAR))
#define RFCTEST_RFCDATA2_CCNT   50
#define RFCTEST_RFCDATA2_TYP    RFCTYPE_CHAR

/* Filler for compiler independant alligment of arrays from this stucture: */
        rfc_padd_4_1bperCF(4,1) /* filler for 1 byte  per SAP_UC */
}
RFCTEST;

/* Warning : Structure may need 8-bytes alignment. */

/* Warning : up to 5 filler bytes inserted. */

#define RFCTEST_LN      sizeofR( RFCTEST )


/* set this to generate also structure information for
 * RFC programs.
 *
 * You can use this on both UNICODE and non-UNICODE Programs
 *
 */

static RFC_UNICODE_TYPE_ELEMENT typeOfRfcTest[] =
{
{ cU("Rfcfloat"),        RFCTEST_RFCFLOAT_TYP,  16,   8,   0,   8,   0,   8,   0 },
{ cU("Rfcchar1"),        RFCTEST_RFCCHAR1_TYP,   0,   1,   8,   2,   8,   4,   8 },
{ cU("Rfcint2"),         RFCTEST_RFCINT2_TYP,    0,   2,  10,   2,  10,   2,  12 },
{ cU("Rfcint1"),         RFCTEST_RFCINT1_TYP,    0,   1,  12,   1,  12,   1,  14 },
{ cU("Rfcchar4"),        RFCTEST_RFCCHAR4_TYP,   0,   4,  13,   8,  14,  16,  16 },
{ cU("Rfcint4"),         RFCTEST_RFCINT4_TYP,    0,   4,  20,   4,  24,   4,  32 },
{ cU("Rfchex3"),         RFCTEST_RFCHEX3_TYP,    0,   3,  24,   3,  28,   3,  36 },
{ cU("Rfcchar2"),        RFCTEST_RFCCHAR2_TYP,   0,   2,  27,   4,  32,   8,  40 },
{ cU("Rfctime"),         RFCTEST_RFCTIME_TYP,    0,   6,  29,  12,  36,  24,  48 },
{ cU("Rfcdate"),         RFCTEST_RFCDATE_TYP,    0,   8,  35,  16,  48,  32,  72 },
{ cU("Rfcdata1"),        RFCTEST_RFCDATA1_TYP,   0,  50,  43, 100,  64, 200, 104 },
{ cU("Rfcdata2"),        RFCTEST_RFCDATA2_TYP,   0,  50,  93, 100, 164, 200, 304 }
};

#define ENTRIES( tab ) ( sizeofR(tab)/sizeofR((tab)[0]) )


RFCTEST   imstruct,
          exstruct;

/************************************************************/
/*  End Definition for inhomogeneous structure/table        */
/************************************************************/


/*--------------------------------------------------------------------*/
/* Function prototypes                                                */
/*--------------------------------------------------------------------*/

int srfcinfo(void);

int srfcconn(void);

int srfcload(void);

int srfcdiag(void);

int srfctruc(void);

int ssncconv(void);

int srfcback(void);

int srfcchek(void);

int srfccanc(void);

int srfcconv(void);

int srfcsreg(void);

int srfccancel(void);

int srfcchn (void);

int srfcraiseerror (void);

static int get_cancel_type (RFC_CHAR *busy, RFC_INT* sec);

#ifndef SAPonNT
#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif	/* __cplusplus */

extern unsigned sleep(unsigned);

#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif	/* __cplusplus */
#endif

void check_gui_for_start(void);

static RFC_RC DLL_CALL_BACK_FUNCTION connection_test(RFC_HANDLE rfc_handle);

static rfc_char_t *connection_test_docu(void);

void get_attributes(RFC_HANDLE rfc_handle);

void get_parameter_for_reg_server(void);

void get_parameter_for_conn_test(void);

void get_parameter_for_load_test(void);

void get_parameter_for_diag_test(void);

void get_parameter_for_back_test(void);

void get_parameter_for_sreg_test(void);

void get_connect_data_from_file(void);

void get_connect_data_via_dialog(void);

void get_load_test_data_from_file(void);

void get_load_test_data_via_dialog(void);

void get_diag_test_data_from_file(void);

void get_diag_test_data_via_dialog(void);

void get_back_test_data_from_file(void);

void get_back_test_data_via_dialog(void);

void get_sreg_test_data_from_file(void);

void get_sreg_test_data_via_dialog(void);

void save_test_parameter(void);

int getParametersForRaiseError (rfc_char_t * method, rfc_char_t * messagetype, rfc_char_t * parameter);

#ifdef __VMS
  /* der compiler auf VAX/VMS kann maximal 31 chars fuer identifier */
  /* wegen object file format */
  #define get_connection_parameters_from_file  get_connectn_params_from_file
  #define get_connection_parameters_via_dialog get_connectn_params_via_dialog
#endif

void get_connection_parameters_from_file(void);

void get_connection_parameters_via_dialog(void);

int RFC_connect(const SAP_CHAR * callerName);

rfc_char_t *read_record(FILE       * fp,
                        rfc_char_t * ibuf);

void check_blank(rfc_char_t *text);

int PM_search(rfc_char_t *param,
              rfc_char_t *text,
              int  leng);

int fill_table(ITAB_H itab_h,
               int    leng);

int check_table(ITAB_H        itab_h,
                RFC_CHAR     *itabname,
                int           recvleng,
                rfc_char_t   *checktab,
                rfc_char_t   *text,
                rfc_char_t   *retcode);

int output_table(ITAB_H         itab_h,
                 rfc_char_t   * itabname);

int print_mystruct(RFCTEST * rfctest);

int print_mytable(ITAB_H itab_h);

void display(rfc_char_t         * arrow,
             RFC_PARAMETER      * rfc_param);

void display_rfcsi(RFCSI *rfcsi);

void DLL_CALL_BACK_FUNCTION rfc_error(rfc_char_t *operation);

void rfcsnc_error (rfc_char_t        * operation,
                   RFC_ERROR_INFO_EX   error_info);

void rfc_param(RFC_PARAMETER *rfc_param,
               rfc_char_t          *abap_field_name,
               unsigned      data_type,
               void          *data_addr,
               unsigned      data_len );

#if defined(SAP_RFC_TIME)
struct timeval
{
  unsigned long   tv_sec;    /* seconds since Jan. 1, 1970 */
  long            tv_usec;   /* and microseconds */
};
struct timezone
{
  int     tz_minuteswest;    /* of UTC */
  int     tz_dsttime;        /* type of DST correction to apply */
};
#endif

#if defined(SAPonUNIX)
unsigned int diffmsec (struct timeval *, struct timeval *);
#elif defined(SAPonOS400)
unsigned int diffmsec_AS400 (time_t, time_t);
#else
unsigned int diffmsec_NON_UNIX(struct timeb *, struct timeb *);
#endif

/*--------------------------------------------------------------------*/
/* Declarations                                                       */
/*--------------------------------------------------------------------*/

int               i,
                  j,
                  rc,
                  znr,
                  action,
                  syntax_OK;

unsigned	      ulen;

int               lineleng,
                  itableng;

rfc_char_t        work_file[129],
                  std_data_file[13] = iU("srfctest.txt");

rfc_char_t        requtext[256],
                  echotext[256],
                  resptext[256];

rfc_char_t      * ptr,
                  ips[257],
                  wbuf[LINE_SIZE+1],
                  sbuf1[LINE_SIZE+1],
                  sbuf2[LINE_SIZE+1],
                  sbuf[4096+1];

rfc_char_t        rfc_dest[33],
                  client[4],
                  userid[13],
                  password[41],
                  language[3],
                  connect_param[1024];

rfc_char_t        saplogon[2],
                  saplogon_id[257],
                  saprfc_ini[2],
                  sapsys[2],
                  register_mode[2],
                  load_balancing[2],
                  mshost[101],
                  group[33],
                  r3name[101],
                  ashost[101],
                  sysnr[3],
                  gwhost[101],
                  gwserv[8],
                  invisible[2],
                  use_sapgui[2],
                  abap_debug[2],
                  abap_debug_ext[2],
                  gui_at_open[2],
                  rfc_trace[2],
                  use_close[2],
                  tphost[101],
                  tpid[101],
                  tpname[101],
                  sreg_tpname[65],
#ifdef SAPwithUNICODE
                  pcs = cU('\0'),
#endif
                  sncmode[2];

rfc_char_t        std_saplogon_id[]  = iU("BIN [PUBLIC]"),
                  std_rfc_dest_r3[]  = iU("BIN"),
                  std_rfc_dest_r2[]  = iU("K50"),
                  std_rfc_dest_ext[] = iU("RFCEXT"),
                  std_gwhost[]       = iU("ihsap4"),
                  std_gwserv[]       = iU("sapgw00"),
                  std_r3name[]       = iU("BIN"),
                  std_mshost[]       = iU("binmain"),
                  std_group[]        = iU("PUBLIC"),
                  std_ashost[]       = iU("binmain"),
                  std_sysnr[]        = iU("53"),
                  std_tcode[]        = iU("SE38"),
                  std_fname[]        = iU("STFC_SAPGUI"),
                  stde_gwhost[]      = iU("binmain"),
                  stde_gwserv[]      = iU("sapgw53"),
                  stde_tphost[]      = iU("binmain"),
                  stde_tpid[]        = iU("hs0335.srfcserv"),
                  stde_tpname[]      = iU("/bas/BIN/gen/dbg/hp/srfcserv"),
                  std_snc_partnername[] = iU("s:sample@hs0335"),
                  std_snc_lib[]         = iU("/krb5/hpux/lib/libkrb5.sl");

int               nrcall,
                  nrback,
                  noback,
                  nrcount,
                  lgit0332,
                  lgit1000,
                  lget0332,
                  lget1000;

rfc_char_t        retcode[2],
                  exitcode[2],
                  checktab[2],
                  printtab[2],
                  nrcall_field[6],
                  nrback_field[6],
                  noback_field[6],
                  lgit0332_field[6],
                  lgit1000_field[6],
                  lget0332_field[6],
                  lget1000_field[6],
                  transaction_test[2],
                  tcode[31],
                  fname[31];

rfc_char_t        text0332[73],
                  text1000[73],
                  texi0332[73],
                  texi1000[73];

int               err_code;
rfc_char_t        err_mess[255];

rfc_char_t        abort_text[80],
                  last_error_text[150];

FILE              *work_fp;

rfc_char_t        *exception = NULL,
                  *function  = NULL;

struct tm         *time_ptr;

time_t            actutime;

#if defined(SAPonUNIX)
struct timeval  sopenmsec,
                eopenmsec,
                scallmsec,
                ecallmsec,
                sfillmsec,
                efillmsec,
                schekmsec,
                echekmsec;

    #ifdef SAPonRM600
    struct timezone
    {
      int     tz_minuteswest;    /* of UTC */
      int     tz_dsttime;        /* type of DST correction to apply */
    };
    #endif
struct timezone tzp;
#elif defined(SAPonOS400)
time_t  	sopenmsec,
                eopenmsec,
                scallmsec,
                ecallmsec,
                sfillmsec,
                efillmsec,
                schekmsec,
                echekmsec;

#else
struct timeb    sopenmsec,
                eopenmsec,
                scallmsec,
                ecallmsec,
                sfillmsec,
                efillmsec,
                schekmsec,
                echekmsec;
#endif

unsigned int    msectime,
                msecopen,
                mseccall,
                msecfill,
                msecchek,
                totalsnd,
                totalrcv,
                totaldat;

RFC_RC                 rfc_rc;
RFC_ENV                new_env;
RFC_HANDLE             rfc_handle;
RFC_FUNCTIONNAME       function_name;
RFC_ERROR_INFO_EX      error_info;
RFC_ATTRIBUTES         rfc_attributes;

RFC_PARAMETER          exporting[6],
                       importing[4],
                       parameters[6],
                       changing[3];

RFC_TABLE              tables[5];

RFCSI                  rfcsi;

RFC_INT                start_value, counter, result;


rfc_char_t             name_systeminfo_test[31]       = iU("RFC_SYSTEM_INFO"),
                       name_connection_test[31]       = iU("STFC_CONNECTION"),
                       name_callback_test[31]         = iU("STFC_CONNECTION_BACK"),
                       name_performance_test[31]      = iU("STFC_PERFORMANCE"),
                       name_structure_test[31]        = iU("STFC_STRUCTURE"),
                       name_sreg_server_test[31]      = iU("STFC_START_CONNECT_REG_SERVER"),
                       name_rfc_raise_exception[31]   = iU("RFC_RAISE_ERROR"),
                       name_rfc_ping_and_wait[31]     = iU("RFC_PING_AND_WAIT"),
                       name_rfc_chn[31]               = iU("STFC_CHANGING");


/*--------------------------------------------------------------------*/
/* Declarations for working with SNC                                  */
/*--------------------------------------------------------------------*/

#define SNC_NAME_LN            256
#define SNC_LIB_LN             256

int               aclkey_len;

rfc_char_t        snc_name[SNC_NAME_LN+1];
rfc_char_t        snc_myname[SNC_NAME_LN+1];
rfc_char_t        snc_partnername[SNC_NAME_LN+1];
rfc_char_t        snc_lib[SNC_LIB_LN+1];
RFC_BYTE          snc_aclkey[SNC_NAME_LN+1];

RFC_EXT_DATA      rfc_ext_data;

SAP_RFC_STATISTIC stats;


rfc_char_t          profile_name[512];

static rfc_char_t   prf_name[] = iU ("profil_gen.txt");

