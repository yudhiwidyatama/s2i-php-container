/* @(#) $Id: //bas/640_REL/src/krn/rfc/srfcserv.h#2 $ SAP*/

/*====================================================================*/
/*                                                                    */
/*     PROGRAM     :   srfcserv.h (running on Windows, Windows_NT,    */
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

#define BLANK                  cU(' ')
#define MAXBUFF                32000
#define LINE_SIZE              256
#define ITAB0332_ENTRY_SIZE    332*sizeofR(SAP_UC)
#define ITAB1000_ENTRY_SIZE    1000*sizeofR(SAP_UC)
#define ETAB0332_ENTRY_SIZE    332*sizeofR(SAP_UC)
#define ETAB1000_ENTRY_SIZE    1000*sizeofR(SAP_UC)
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


#include "saprfc.h"
#include "sapitab.h"

#if defined(SAPonNT)
#include <windows.h>
#endif

#if defined(OS2)
 /* map os/2 DosSleep to NT Sleep call */
 extern unsigned short _System Sleep( unsigned long msec );
 #pragma import(Sleep,,"DOSCALLS",229)
#endif


/************************************************************/
/*  Definition for inhomogeneous structure/table 'RFCTEST'  */
/*                                                          */
/*     NOT available with 16-bit RFC library on Windows     */
/*                                                          */
/************************************************************/

static RFC_TYPEHANDLE handleOfRfcTest;


/* this is generated using genh */
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

static void help(void);

int fill_table(ITAB_H itab_h,
               int    leng);

int check_table(ITAB_H itab_h,
                rfc_char_t   *itabname,
                int    recvleng,
                rfc_char_t   *checktab,
                rfc_char_t   *text,
                rfc_char_t   *retcode);

void SNC_check(void);

void function_abort(rfc_char_t *atext);

static RFC_RC install(void);

static RFC_RC DLL_CALL_BACK_FUNCTION connection_test(RFC_HANDLE rfc_handle);
static RFC_RC DLL_CALL_BACK_FUNCTION callback_test(RFC_HANDLE rfc_handle);
static RFC_RC DLL_CALL_BACK_FUNCTION performance_test(RFC_HANDLE rfc_handle);
static RFC_RC DLL_CALL_BACK_FUNCTION structure_test(RFC_HANDLE rfc_handle);
static RFC_RC DLL_CALL_BACK_FUNCTION rfc_raise_error(RFC_HANDLE rfc_handle);
static RFC_RC DLL_CALL_BACK_FUNCTION string_test(RFC_HANDLE rfc_handle);

static rfc_char_t *connection_test_docu(void);

static rfc_char_t *callback_test_docu(void);

static rfc_char_t *performance_test_docu(void);

static rfc_char_t *structure_test_docu(void);

static rfc_char_t *rfc_raise_error_docu(void);

static rfc_char_t *string_test_docu(void);

void rfc_param(RFC_PARAMETER *rfc_param,
               rfc_char_t    *abap_field_name,
               unsigned      data_type,
               void          *data_addr,
               unsigned      data_len );

void DLL_CALL_BACK_FUNCTION rfc_error(rfc_char_t *operation );

/*--------------------------------------------------------------------*/
/* Declarations                                                       */
/*--------------------------------------------------------------------*/

int               i,
                  j,
                  rc,
                  znr,
                  nrcount;

unsigned	  ulen;

int               lineleng,
                  itableng;

rfc_char_t              ips[101],
                  tpname[101],
		  tphost[101],
		  gwhost[101],
		  gwserv[9];

rfc_char_t              requtext[256],
                  echotext[256],
                  resptext[256];

rfc_char_t              *ptr,
                  sbuf[32000+1];

int               nrback,
                  noback,
                  lgit0332,
                  lgit1000,
                  lget0332,
                  lget1000;

rfc_char_t              retcode[2],
                  exitcode[2],
                  checktab[2],
                  printtab[2],
                  nrback_field[6],
                  noback_field[6],
                  lgit0332_field[6],
                  lgit1000_field[6],
                  lget0332_field[6],
                  lget1000_field[6];

rfc_char_t        text0332[73],
                  text1000[73],
                  texi0332[73],
                  texi1000[73];

rfc_char_t        abort_text[80],
                  last_error_text[150];

rfc_char_t        *exception = NULL,
                  *function  = NULL;

struct         tm *time_ptr;

time_t            actutime;

RFC_RC            rfc_rc;
RFC_HANDLE        rfc_handle;
RFC_FUNCTIONNAME  function_name;
RFC_ATTRIBUTES    rfc_attributes;

RFC_PARAMETER     exporting[6],
                  importing[4],
                  parameters[6];

RFC_TABLE         tables[5];

RFC_TID           tid;

rfc_char_t        name_connection_test[31]  = iU("STFC_CONNECTION"),
                  name_callback_test[31]    = iU("STFC_CONNECTION_BACK"),
                  name_performance_test[31] = iU("STFC_PERFORMANCE"),
                  name_structure_test[31]   = iU("STFC_STRUCTURE"),
                  name_raise_error[31]      = iU("RFC_RAISE_ERROR"),
                  name_string_test[31] 	    = iU("STFC_STRING");

RFC_STRING	  questions, answer;

/*--------------------------------------------------------------------*/
/* Declarations for working with SNC                                  */
/*--------------------------------------------------------------------*/

#define SNC_NAME_LN            256
#define SNC_LIB_LN             256

int               aclkey_len,
                  SNC_checked,
                  register_mode;

rfc_char_t        snc_partner_name[SNC_NAME_LN+1];
RFC_BYTE          snc_partner_aclkey[SNC_NAME_LN+1];

RFC_SNC_MODE      snc_mode;

RFC_CALL_INFO     rfc_call_info;

