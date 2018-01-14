/* static char sccsid[] = "@(#) $Id: //bas/640_REL/src/krn/rfc/rfcthcli.c#4 $ SAP";  */
/*====================================================================*/
/*                                                                    */
/*     PROGRAM     :   rfcthcli.c (Windows NT/95)                     */
/*                                                                    */
/*                                                                    */
/*     DESCRIPTION :   Test program for multi-threaded RFC library    */
/*                                                                    */
/*                     - The function module STFC_PERFORMANCE will be */
/*                       called by each thread in this program for    */
/*                       send/receive of internal tables. The number  */
/*                       of entries in each table will be required    */
/*                       via dialog.                                  */
/*                       oprionaly:                                   */
/*                     - The function module STFC_STRUCTURE will be   */
/*                       called by each thread in this program for    */
/*                       send/receive of internal tables. The number  */
/*                       of entries in each table will be required    */
/*                       via dialog.                                  */
/*                                                                    */
/*                     - The RFC-Server-System can be R/2 or R/3 or   */
/*                       an external program which supports the       */
/*                       RFC function STFC_PERFORMANCE.               */
/*                                                                    */
/*     ATTENTION:      This program can also be compiled/linked for   */
/*                     running in single threaded mode. In that case, */
/*                     the #define RFCwithTHREADS in this program     */
/*                     must be deleted.                               */
/*                     Currently, this flag is set for Windows NT, 95 */
/*                                                                    */
/*                                                                    */
/*                                                                    */
/*     SAP AG Walldorf                                                */
/*     Systeme, Anwendungen und Produkte in der Datenverarbeitung     */
/*                                                                    */
/*     Copyright (C) SAP AG 1996                                      */
/*                                                                    */
/*====================================================================*/

/**********************************************************************/
/* Set Compile/Link flag for multi-thread                             */
/*                                                                    */
/* At this time only for Windows NT or 95                             */
/*                                                                    */
/**********************************************************************/

#ifndef SAPonLIN
/* Next line deleted because of clash in Sinix and Linux
#define _XOPEN_SOURCE
*/
#define _XOPEN_SOURCE_EXTENDED 1
#ifndef  _HPUX_SOURCE
# define _HPUX_SOURCE   /* gettimeofday(), timeval, timezone */
#endif
#define _ALL_SOURCE    /* AIX gettimeofday(), timeval, timezone */
#define __EXTENSIONS__ /* SUN gettimeofday(), timeval, timezone */
#define _OSF_SOURCE    /* OSF gettimeofday(), timeval, timezone */
#ifdef SAPonRM600
#define _XOPEN_SOURCE 1 /* on rm600, because already in saptype.h */
#endif
#if defined(SAPwithTHREADS) && defined(SAPonALPHA) && !defined(_POSIX_C_SOURCE)
#  define _POSIX_C_SOURCE 199506L
#endif
#endif

#if defined(_WIN32)
#define RFCwithTHREADS
#endif


/*--------------------------------------------------------------------*/
/* Set SAP flag for compile/link                                      */
/*--------------------------------------------------------------------*/
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
#  ifdef _MSC_VER
#    include <process.h>    
#  endif
#endif


/*--------------------------------------------------------------------*/
/* Defines                                                            */
/*--------------------------------------------------------------------*/

#define BLANK                  cU(' ')
#define LINE_SIZE              256
#define RFC_MIN_STACK          (300 * 1024)

/*--------------------------------------------------------------------*/
/* Setup includes                                                     */
/*--------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/* RFC Header Files                                                   */
/*--------------------------------------------------------------------*/
#include "saprfc.h"
#include "sapitab.h"

#ifdef DGUX
#include <sys/time.h>
#endif


#if !defined(SAPonUNIX) && !defined(SAPonNT) && !defined(SAPonOS2_2x) && !defined(SAPonOS400)
#include "saptype.h"
/*--------------------------------------------------------------------*/
/* The include file saptype.h is only for SAP internal make.          */
/*                                                                    */
/* If you want to make this program on your environment you must      */
/* compile and link with some flags depending on your operating       */
/* system.                                                            */
/*                                                                    */
/* Following flags are possible:                                      */
/*                                                                    */
/* UNIX:    #define SAPonUNIX                                         */
/* OS/400:  #define SAPonOS400                                        */
/* OS/2, Windows 3.x, NT or 95: already defined in this file          */
/*                                                                    */
/* If you have some problems with the struct timeval, timezone        */
/* please add the flag SAP_RFC_TIME in your own saptype.h or in your  */
/* compile options                                                    */
/*--------------------------------------------------------------------*/
#endif

#ifndef SAPonOS400
#  include <sys/timeb.h>
#else
#  ifndef TIME_DEFINED
#    define TIME_DEFINED
#    include <sys/time.h>       /* for dcl of timeval */
#  endif
#endif

#ifdef SAPonUNIX
#	include <sys/time.h>
#	include <unistd.h>
#endif

/*--------------------------------------------------------------------*/
/* Macro for output RFC-Call on screen                                */
/*--------------------------------------------------------------------*/
#ifdef RFCwithTHREADS
#define PRINT_RFC_CALL(thread_id, text)                               \
	printfU(cU("\n Thread %3d:  %s"), thread_id, text);                \
	fflush(stdout)
#else
#define PRINT_RFC_CALL(thread_id, text)                               \
	printfU(cU("\n              %s"), text);                           \
	fflush(stdout)
#endif /* RFCwithTHREADS */

/*--------------------------------------------------------------------*/
/* Definitions for working with multi-threads on Windows NT or 95     */
/* Definitions for working with multi-threads on UNIX                 */
/*   - Assume Posix threads.                                          */
/*   - Compile will fail if pthread.h is missing                      */
/*--------------------------------------------------------------------*/
#define RFCT_MAX_THREADS       999
#define RFCT_RUNNING           cU('X')
#define RFCT_TERMINATED        cU(' ')

#ifdef RFCwithTHREADS

#ifdef SAPonNT /* --- NT ---------------------------------------------*/
#  include <windows.h>
#  if !defined(EBUSY) || !defined(ESRCH)
    #include <errno.h>
#  endif

typedef HANDLE THREAD_ID_TYPE;
#define THREAD_INVALID_ID                    (THREAD_ID_TYPE)0xffffffff

#else          /* --- !NT == pthread ---------------------------------*/

#  include <pthread.h>
#  include <limits.h>
#    if !defined(EBUSY)
    #include <errno.h>
#  endif

typedef pthread_t THREAD_ID_TYPE;
#ifdef SAPonOS400
static THREAD_ID_TYPE THREAD_INVALID_ID = { 0, 0, 0, 0, 0, 0 };
#elif defined(SAPonOS390)
static THREAD_ID_TYPE THREAD_INVALID_ID = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
#elif defined(SAPonRM600)
static THREAD_ID_TYPE THREAD_INVALID_ID = { NULL, -1, -1 };
#else
#define THREAD_INVALID_ID		(THREAD_ID_TYPE)0xffffffff
#endif

#endif        /* --- NT/pthread---------------------------------------*/

#else         /* -- no threads----------------------------------------*/

typedef int THREAD_ID_TYPE;
#define THREAD_INVALID_ID                      (THREAD_ID_TYPE)0xffffffff

#endif        /* -- no threads----------------------------------------*/

/* admin stucture
 */
typedef struct _rfct_th_adm {
    SAP_CHAR		cRunning;
    THREAD_ID_TYPE	thId;
} RFCT_TH_ADM;

static RFCT_TH_ADM rfct_adm[RFCT_MAX_THREADS+1];/* 0 == main, 1 == first thread*/
static int         thr_attr_init = 0;

#ifdef RFCwithTHREADS
#ifdef SAPonNT /* --- NT ---------------------------------------------*/
/*--------------------------------------------------------------------*/
/* Definitions for working with multi-threads on Windows NT or 95     */
/*--------------------------------------------------------------------*/
/* Thread Management Macros/Functions */

void THREAD_INIT(void)
{
    /* InterlockedIncrement not beedded since called from the main trhread
     */
    if (0 == thr_attr_init)
    {
        int i;
        memsetR(rfct_adm, 0, sizeofR(rfct_adm));

        for (i = 0; i <= RFCT_MAX_THREADS ; i++)
            rfct_adm[i].cRunning =  RFCT_TERMINATED;

        thr_attr_init = 1;
    }
}

void THREAD_FINALIZE(void)
{
}



int rfct_thread_create(int th_nb,
		       LPTHREAD_START_ROUTINE start_ptr,
		       LPVOID arg)
{

    int rc;
    HANDLE hThr;

    THREAD_INIT();

    /* ALPHA/OSF kill does not return an error for exited thread
     * thus we maintain a running flag
     * this flag is set before starting
     * and cleared if strating failed
     */
    if (th_nb < 0 || th_nb > RFCT_MAX_THREADS)
        return EINVAL;

    rfct_adm[th_nb].cRunning = RFCT_RUNNING;
    rfct_adm[th_nb].thId = 
#ifdef _MSC_VER
	       (HANDLE)_beginthreadex(NULL,
			    0,
			    (unsigned int (__stdcall *)(void *))start_ptr,
			    arg,
			    0,
			    (unsigned int *)&hThr);
#else	/* !MSVC */
	       CreateThread(NULL,
			    0,
			    (DWORD (*)(void *))start_ptr,
			    arg,
			    0,
			    (LPDWORD)&hThr);
#endif	/* MSVC */
    if (0 == hThr)
    {
    	rfct_adm[th_nb].thId     = THREAD_INVALID_ID; 
    	rfct_adm[th_nb].cRunning = RFCT_TERMINATED;
    	rc = GetLastError();
    }
    else
    	rc = 0;
    return rc;
}

#define THREAD_CREATE(th_nb, start_ptr, arg)              \
		rfct_thread_create((th_nb),                       \
			    (LPTHREAD_START_ROUTINE)(start_ptr),      \
			    (LPVOID)(arg))


int THREAD_EXIT_CODE(int th_nb, int * exit_code_ptr)
{
    if (  th_nb >= 0 
       && th_nb <= RFCT_MAX_THREADS 
       &&  rfct_adm[th_nb].thId  != THREAD_INVALID_ID) 
    {
	if (GetExitCodeThread(rfct_adm[th_nb].thId, (DWORD*)exit_code_ptr))
	{
	    if (*exit_code_ptr == STILL_ACTIVE)
	        return EBUSY;
	    else
	    {
	        CloseHandle(rfct_adm[th_nb].thId);
	        rfct_adm[th_nb].cRunning = RFCT_TERMINATED ;
	        rfct_adm[th_nb].thId     = THREAD_INVALID_ID; 
	        return 0;
	    }
	}
	else
	{
	    *exit_code_ptr = GetLastError();
	    /*avoid retry */
	    CloseHandle(rfct_adm[th_nb].thId);
	    rfct_adm[th_nb].cRunning = RFCT_TERMINATED ;
	    rfct_adm[th_nb].thId     = THREAD_INVALID_ID; 
	    return ESRCH;
	}
    }
    else
    {
	*exit_code_ptr = -1;
	return EBADF;
    }
}

#define THREAD_LEAVE(th_nb)				         \
		 ((th_nb >= 0 && th_nb <= RFCT_MAX_THREADS) ?    \
		   rfct_adm[th_nb].cRunning = RFCT_TERMINATED :  \
		   0)

int THREAD_EQUAL( THREAD_ID_TYPE lVal, THREAD_ID_TYPE rVal)
{
    return (lVal == rVal);
}

#endif  /* SAPonNT */


#ifdef SAPonUNIX
/*--------------------------------------------------------------------*/
/* Definitions for working with multi-threads on UNIX                 */
/*   - Assume Posix threads.                                          */
/*   - Compile will fail if pthread.h is missing                      */
/*--------------------------------------------------------------------*/
#include <signal.h>

static pthread_attr_t thr_attr;
 

#ifdef SAPonRS6000
  #ifndef PTHREAD_CREATE_JOINABLE
    #define PTHREAD_CREATE_JOINABLE	PTHREAD_CREATE_UNDETACHED
  #endif
#endif
#ifdef SAPonOS390
  #ifndef PTHREAD_CREATE_JOINABLE
    #define PTHREAD_CREATE_JOINABLE     __UNDETACHED
  #endif
#endif

int THREAD_EQUAL( THREAD_ID_TYPE lVal, THREAD_ID_TYPE rVal)
{
    return pthread_equal(lVal,  rVal);
}

void THREAD_INIT(void)
{
    /* pthread_once not needed since called from the main thread
     */
    if (0 == thr_attr_init)
    {
	int    rc, i;

	/* pthread stuff----------------------------------------------
	*/
	pthread_attr_init(&thr_attr);
	pthread_attr_setdetachstate(&thr_attr, PTHREAD_CREATE_JOINABLE);
#ifndef SAPonOS390
	pthread_attr_setscope(&thr_attr, PTHREAD_SCOPE_PROCESS);
#endif

#ifndef PTHREAD_STACKSIZE_MIN
#    define PTHREAD_STACKSIZE_MIN RFC_MIN_STACK
#endif
	{
	    /* MAKE sure that we have enough stack space
	    * some defaults are too small
	    */
	    size_t stack_size, min_size;

	    rc = pthread_attr_getstacksize(&thr_attr, &stack_size);
	    if (0 != rc)
	    {
		printfU(cU("cannot get threads stacksize\n"));
		exit(1);
	    }
	    min_size = PTHREAD_STACKSIZE_MIN;

	    if (min_size <= RFC_MIN_STACK)
		min_size = RFC_MIN_STACK;
	    if (stack_size < min_size)
	    {
		int page_size;
		page_size = (int) sysconf(_SC_PAGESIZE);
		min_size = ((min_size / page_size) + 1 ) * page_size;

		printfU(cU("adjusting stack size from %d to %d\n"), 
			    stack_size, min_size);
		rc = pthread_attr_setstacksize(&thr_attr, min_size);
		if (0 != rc)
		{
		    printfU(cU("cannot set threads stacksize\n"));
		    exit(1);
		}
	    }
	}
	/* wrapper stuff----------------------------------------------
	*/
      memsetR(rfct_adm, 0, sizeofR(rfct_adm));

      for (i = 0; i <= RFCT_MAX_THREADS ; i++)
            rfct_adm[i].cRunning =  RFCT_TERMINATED;

	thr_attr_init = 1;
    }
}

void THREAD_FINALIZE(void)
{
    if (0 != thr_attr_init)
    {
        pthread_attr_destroy(&thr_attr);
    }
}

int rfct_thread_create(int th_nb,
			void*(*start_ptr)(void*),
			void* arg)
{
    int rc;
    THREAD_INIT();

    /* ALPHA kill does not return an error for exited thread
     * thus we maintain a running flag
     * this flag is set before starting
     * and cleared if strating failed
     */
    if (th_nb < 0 || th_nb > RFCT_MAX_THREADS)
        return 0;

    rfct_adm[th_nb].cRunning = RFCT_RUNNING;
    if (0 != (rc = pthread_create(&(rfct_adm[th_nb].thId), 
				&thr_attr, start_ptr, arg)))
    {
    	rfct_adm[th_nb].thId     = THREAD_INVALID_ID; 
    	rfct_adm[th_nb].cRunning = RFCT_TERMINATED;
    }
    return rc;
}

#define THREAD_CREATE(th_nb, start_ptr, arg) 		\
		rfct_thread_create((th_nb), 				\
				  (void*(*)(void*))(start_ptr),		\
				  (arg))

int THREAD_EXIT_CODE(int th_nb, int * exit_code_ptr)
{
    int rc;
    if (  th_nb >= 0 
       && th_nb <= RFCT_MAX_THREADS 
       &&  !THREAD_EQUAL(rfct_adm[th_nb].thId, THREAD_INVALID_ID) )
    {
	    void *dummy_exit = NULL;  /* pointer sized variable */

#ifdef SAPonALPHA
/*--------------------------------------------------------------------*/
	    if (rfct_adm[th_nb].cRunning == RFCT_RUNNING)
	           return EBUSY;

#else /*!APLPHA*/
/*--------------------------------------------------------------------*/
	    if (0 == pthread_kill(rfct_adm[th_nb].thId, 0))
	       return EBUSY;
#endif
	    rc = pthread_join(rfct_adm[th_nb].thId, (void **)&dummy_exit);

	    rfct_adm[th_nb].cRunning = RFCT_TERMINATED ;
	    rfct_adm[th_nb].thId     = THREAD_INVALID_ID; 
	    if (rc != 0)
	        *exit_code_ptr = rc ;
	    else
		*exit_code_ptr = (int)dummy_exit;

	    return (rc == 0 ? 0 : ESRCH);
    }
    else
    {
        *exit_code_ptr = -1;
        return EBADF;
    }
}

#define THREAD_LEAVE(th_nb)				         \
		 ((th_nb >= 0 && th_nb <= RFCT_MAX_THREADS) ?    \
		   rfct_adm[th_nb].cRunning = RFCT_TERMINATED :  \
		   0)


#endif  /* SAPonUNIX */

#endif /* RFCwithTHREADS */


#if defined(SAPwithPASE400) || defined(SAPonOS400)
externC SAP_UC* as4_fgetsU ( SAP_UC* string, int n, FILE* f ) { return o4fgetsU(string,n,f); } 
externC int     as4_fputsU ( const SAP_UC* string,  FILE* f ) { return o4fputsU(string,f);   }
#endif


/*--------------------------------------------------------------------*/
/* Function prototypes                                                */
/*--------------------------------------------------------------------*/

int RFC_PerformanceThread(void *lpParms);
void srfcstructure(int tid, RFC_HANDLE hRfc);

void get_parameter_for_conn_test(void);

void get_parameter_for_load_test(void);

void get_connect_data_from_file(void);

void get_connect_data_via_dialog(void);

void get_load_test_data_from_file(void);

void get_load_test_data_via_dialog(void);

void save_test_parameter(void);

void get_connection_parameters_from_file(void);

void get_connection_parameters_via_dialog(void);

RFC_HANDLE RFC_connect(int);

SAP_CHAR *read_record(FILE *fp,
		  SAP_CHAR *ibuf);

void check_blank(SAP_CHAR *text);

int PM_search(SAP_CHAR *param,
	      SAP_CHAR *text,
	      int  leng);

int fill_table(ITAB_H itab_h,
	       long   leng);

int check_table(ITAB_H      itab_h,
		SAP_CHAR   *itabname,
		long        recvleng,
		SAP_CHAR   *checktab,
		SAP_CHAR   *text,
		SAP_CHAR   *retcode);

int output_table(ITAB_H      itab_h,
		 SAP_CHAR   *itabname);

void display(SAP_CHAR      *arrow,
	     RFC_PARAMETER *rfc_param);

void DLL_CALL_BACK_FUNCTION rfclib_error(SAP_CHAR *operation);

void rfc_error(SAP_CHAR *operation, int thread_id);

void rfc_param(RFC_PARAMETER *rfc_param,
	       SAP_CHAR      *abap_field_name,
	       unsigned      data_type,
	       void          *data_addr,
	       unsigned      data_len );

#if  defined(SAP_RFC_TIME)
struct timeval
{
  unsigned long   tv_sec;    /* seconds since Jan. 1, 1970 */
  long            tv_usec;   /* and microseconds */
};
#endif
#if  defined(SAPonRM600) || defined(SAP_RFC_TIME)
struct timezone
{
  int     tz_minuteswest;    /* of UTC */
  int     tz_dsttime;        /* type of DST correction to apply */
};
#endif

#if defined(SAPonUNIX) || defined(SAPonOS400)
unsigned long int diffmsec (struct timeval *, struct timeval *);
#else
unsigned long int diffmsec_NON_UNIX(struct timeb *, struct timeb *);
#endif


/*--------------------------------------------------------------------*/
/* Globals                                                            */
/*--------------------------------------------------------------------*/
FILE              *work_fp;

SAP_CHAR          work_file[129],
		  std_work_file[13] = iU("rfcthcli.txt");

SAP_CHAR          std_saplogon_id[]  = iU("BIN [PUBLIC]"),
                  std_rfc_dest_r3[]  = iU("BIN"),
		  std_rfc_dest_r2[]  = iU("K50"),
		  std_rfc_dest_ext[] = iU("RFCEXT"),
		  std_gwhost[]       = iU("ihsap4"),
		  std_gwserv[]       = iU("sapgw00"),
		  std_r3name[]       = iU("BIN"),
		  std_mshost[]       = iU("hs0335"),
		  std_group[]        = iU("PUBLIC"),
		  std_ashost[]       = iU("hs0335"),
		  std_sysnr[]        = iU("53"),
		  stde_gwhost[]      = iU("hs0335"),
		  stde_gwserv[]      = iU("sapgw53"),
		  stde_tphost[]      = iU("hs0335"),
		  stde_tpid[]        = iU("hw1145.rfcthsrv"),
		  stde_tpname[]      = iU("/bas/BIN/gen/dbg/hp/rfcthsrv"),
		  std_snc_partnername[] = iU("s:sample@hw1145"),
		  std_snc_lib[]         = iU("/krb5/hpux/lib/libkrb5.sl");

int               i,
		  j,
		  rc,
		  znr,
		  syntax_OK,
		  for_ever, for_ever_loops,
		  silent_mode;

SAP_CHAR          *ptr,
                  ips[257],
		  wbuf[513],
		  sbuf1[513],
		  sbuf2[513],
		  sbuf[4096+1];

SAP_CHAR          saplogon[2],
                  saplogon_id[257],
                  saprfc_ini[2],
		  rfc_dest[33],
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
		  rfc_trace[2],
		  use_close[2],
		  tphost[101],
		  tpid[101],
		  tpname[101],
		  sncmode[2],
		  snc_partnername[256],
		  snc_lib[256];

SAP_CHAR          client[4],
		  userid[13],
		  password[9],
		  language[3];

int               nrcall,
		  lgit0332,
		  lgit1000,
		  lget0332,
		  lget1000,
		  thread_no;

SAP_CHAR          retcode[2],
		  exitcode[2],
		  checktab[2] = iU("N"),
		  nrcall_field[4],
		  lgit0332_field[6],
		  lgit1000_field[6],
		  lget0332_field[6],
		  lget1000_field[6],
		  thread_no_field[4];

SAP_CHAR          name_performance_test[31] = iU("STFC_PERFORMANCE");

SAP_CHAR          do_structure_test [2] = iU("N");


RFC_TYPEHANDLE      handleOfRfcTest;
#ifndef SAPwithUNICODE

#define RFCTEST_LN sizeofR(RFCTEST)
typedef struct
{
  RFC_FLOAT   rfcfloat;
  RFC_CHAR    rfcchar1[1];
  RFC_INT2    rfcint2;
  RFC_INT1    rfcint1;
  RFC_CHAR    rfcchar4[4];
  RFC_INT     rfcint4;
  RFC_BYTE    rfchex3[3];
  RFC_CHAR    rfcchar2[2];
  RFC_TIME    rfctime;
  RFC_DATE    rfcdate;
}
RFCTEST;
static RFC_TYPE_ELEMENT fieldsOfRfcTest[] =
{
    /* name */      /* type */  /* length */           /* decimals */
  { cU("rfcfloat"), TYPFLOAT,   sizeofR (double),             0 },
  { cU("rfcchar1"), TYPC,       sizeofR (RFC_CHAR),           0 },
  { cU("rfcint2"),  TYPINT2,    sizeofR (RFC_INT2),           0 },
  { cU("rfcint1"),  TYPINT1,    sizeofR (RFC_INT1),           0 },
  { cU("rfcchar4"), TYPC,       sizeofR (RFC_CHAR)*4,         0 },
  { cU("rfcint4"),  TYPINT,     sizeofR (RFC_INT),            0 },
  { cU("rfchex3"),  TYPX,       sizeofR (RFC_BYTE)*3,         0 },
  { cU("rfcchar2"), TYPC,       sizeofR (RFC_CHAR)*2,         0 },
  { cU("rfctime"),  TYPTIME,    sizeofR (RFC_TIME),           0 },
  { cU("rfcdate"),  TYPDATE,    sizeofR (RFC_DATE),           0 }

};
#else
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

static RFC_UNICODE_TYPE_ELEMENT fieldsOfRfcTest[] =
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
#endif

#define ENTRIES( tab ) ( sizeofR(tab)/sizeofR((tab)[0]) )

/*--------------------------------------------------------------------*/
/* main                                                               */
/*--------------------------------------------------------------------*/

int mainU(int argc, SAP_CHAR **argv)
{
  RFC_RC rfc_rc;
  setbuf(stdout, NULL);

  strcpyU(work_file, std_work_file);

  if (argc == 1)
    syntax_OK = 1;
  else
    syntax_OK = 0;

  for (i = 1; i < argc; i++)
  {
    if (memcmpU(argv[i], cU("file="), 5) == 0)
    {
      syntax_OK = 1;
      znr = strlenU(argv[i]) - 5;
      if (znr)
      {
	if (znr > 128)
	    argv[i][5+128] = 0;
	strcpyU(work_file, &argv[i][5]);
      }
      silent_mode=1;
    }
    else if (memcmpU(argv[i], cU("for_ever="), 9) == 0)
    {
	syntax_OK = 1;
	for_ever_loops = atoiU((argv[i])+9);
    }
  }

  if (syntax_OK == 0)
  {
    printfU(cU("\nSyntax:   rfcthcli [options]\n"));
    printfU(cU("\n          options:  file=<test data file>\n\n"));
    exit(1);
  }

  for (;;)
  {
    if (0 == for_ever)
    {
    printfU(cU("\n\n"));
    printfU(cU("               **********************************************\n"));
    printfU(cU("               *                                            *\n"));
    printfU(cU("               *              SAP RFC-API TEST              *\n"));
    printfU(cU("               *                                            *\n"));
    printfU(cU("               *             Single threaded on             *\n"));
    printfU(cU("               *           all supported platforms          *\n"));
    printfU(cU("               *                                            *\n"));
    printfU(cU("               *                     or                     *\n"));
    printfU(cU("               *                                            *\n"));
    printfU(cU("               *             Multi threaded on              *\n"));
    printfU(cU("               *               Windows NT/95                *\n"));
    printfU(cU("               *     R/3-based UNIX with thread support     *\n"));
    printfU(cU("               *                                            *\n"));
    printfU(cU("               **********************************************\n\n"));

    get_parameter_for_load_test();
    printfU(cU("\n"));
    fflush(stdout);

    /**** In Threaded programs, RfcInit Must be called first!!!!!!
    */
    THREAD_INIT();
    RfcInit();

    /* register the structure */
    if (cU('Y') == do_structure_test[0])
    {
#ifdef SAPwithUNICODE
        rfc_rc = RfcInstallUnicodeStructure (cU("RFCTEST"),
                                             fieldsOfRfcTest,
                                             ENTRIES(fieldsOfRfcTest),
                                             0, NULL,
                                             &handleOfRfcTest );
#else
        rfc_rc = RfcInstallStructure (cU("RFCTEST"),
                                      fieldsOfRfcTest,
                                      ENTRIES(fieldsOfRfcTest),
                                      &handleOfRfcTest );
#endif
        if (RFC_OK != rfc_rc)
        {
            printfU(cU("RfcInstallstucture error %d\n"), rfc_rc);
            
            exit(1) ;
        }
    }
    }

#ifdef RFCwithTHREADS
    {
      int            i,
		     thread_rc,
		     active = 0,
		     success = 0,
		     failure = 0,
		     delay_time = 0,
		     dwExitCode = 0;

      for (i=1; i<=thread_no; i++)
      {
	    thread_rc  = THREAD_CREATE(i, RFC_PerformanceThread, (void *)i);

	    if ((thread_no > 1) && (delay_time > 0))
	    {
	      printfU(cU("\n\n Main Thread:  Wait %d seconds before starting new thread\n"),
			 delay_time);

#if defined(SAPonNT)
	    Sleep(delay_time*1000);
#elif defined(SAPonUNIX)
	    sleep(delay_time);
#endif
	}
      }

      for(;;)
      {
	active = 0;
	for (i=1; i<=thread_no; i++)
	{
	    thread_rc = THREAD_EXIT_CODE(i, &dwExitCode);
	    if (thread_rc == EBUSY)
	    {
	      active = 1;
	    }
	    else if (thread_rc == EBADF)
		continue;
	    else if (thread_rc == 0)
	    {
	        if (dwExitCode == 0)
		  success++;
	        else
	        {
		  if (dwExitCode == 99)
		    printfU(cU("\n\n Main Thread:  RFC-ERROR OCCURS  ==>  Thread %d exited\n"), i);
		  else
		    printfU(cU("\n\n Main Thread:  Thread %d exited with rc=%d\n"),
			 i, dwExitCode);
		  failure++;
	        }
	    }
	    else
	    {
	        printfU(cU("\n\n Main Thread:  Thread %d returned %d\n"),
		       i, thread_rc);
	        failure++;
	    }
	}
        if (active == 0)
	  break;

	/* Don't steal all the CPU */
#if defined(SAPonNT)
	Sleep(1000);
#elif defined(SAPonUNIX)
	sleep(1);
#endif
      }

      if (success)
	printfU(cU("\n\n Main Thread:  %d thread(s) successfully terminated.\n"), success);

      if (failure)
	printfU(cU("\n\n Main Thread:  %d thread(s) terminated with an error.\n"), failure);
    }
#else

    RFC_PerformanceThread(0);

#endif /* RFCwithTHREADS */

    printfU(cU("\n\n"));
    if (0 != silent_mode && 0 == for_ever_loops)
    {
	break;
    }
    if (0 != for_ever_loops)
	for_ever = 1;
    if (0 == for_ever)
    {
	do
	{
	    printfU(cU("                    Continue (y/n) or (e: for ever).:  "));
	    fflush(stdout);
	    getsU(ips);
	    ips[0] = (SAP_CHAR) toupperU(ips[0]);
	} while ((ips[0] != cU('Y')) && (ips[0] != cU('N')) && (ips[0] != cU('E')));
	if (cU('E') == ips[0])
	    for_ever_loops = -1;
    }
    if (0 != for_ever)
    {
	printfU(cU("wait 5 seconds and run again!\n"));
#if defined(SAPonNT)
	Sleep(5000);
#elif defined(SAPonUNIX)
	sleep(5);
#endif
	if (-1 != for_ever_loops)
	{
	    if (0 == for_ever_loops--)
		break;
	}
        continue ;
    }

    if (ips[0] == cU('N'))
      break;
  }

  printfU(cU("\n"));
  THREAD_FINALIZE();

  return 0;
}



#define ITAB0332_ENTRY_SIZE_CHAR    332
#define ITAB0332_ENTRY_SIZE_RAW     ITAB0332_ENTRY_SIZE_CHAR*sizeofR(SAP_UC)

#define ITAB1000_ENTRY_SIZE_CHAR    1000
#define ITAB1000_ENTRY_SIZE_RAW     ITAB1000_ENTRY_SIZE_CHAR*sizeofR(SAP_UC)

#define ETAB0332_ENTRY_SIZE_CHAR    332
#define ETAB0332_ENTRY_SIZE_RAW     ETAB0332_ENTRY_SIZE_CHAR*sizeofR(SAP_UC)

#define ETAB1000_ENTRY_SIZE_CHAR    1000
#define ETAB1000_ENTRY_SIZE_RAW     ETAB1000_ENTRY_SIZE_CHAR*sizeofR(SAP_UC)

/*====================================================================*/
/*                                                                    */
/* Thread for calling function module STFC_PERFORMANCE                */
/*                                                                    */
/*====================================================================*/
int RFC_PerformanceThread (void * lpParms)
{
    int                 i;
    int                 nrcount;
    SAP_CHAR            dbuf[512];
    RFC_RC              rfc_rc;
    RFC_ENV             new_env;
    RFC_HANDLE          rfc_handle;
    RFC_ATTRIBUTES      rfc_attributes;
    RFC_PARAMETER       exporting[6],
                        importing[4];
    RFC_TABLE           tables[5];
    SAP_CHAR          * exception = NULL,
                      * function  = NULL,
                        text0332[73],
                        text1000[73],
                        texi0332[73],
                        texi1000[73];
    unsigned long int   msecopen,
                        mseccall;

#if defined(SAPonUNIX) || defined(SAPonOS400)
    struct timeval      sopenmsec,
                        eopenmsec,
                        scallmsec,
                        ecallmsec;
    struct timezone     tzp;
#else
    struct timeb        sopenmsec,
                        eopenmsec,
                        scallmsec,
                        ecallmsec;
#endif

    
    /********************************************************************/
    /* install error handler                                            */
    /********************************************************************/
    new_env.allocate     = NULL;
    new_env.errorhandler = rfclib_error;
    RfcEnvironment(&new_env);

    
    /********************************************************************/
    /* Open RFC connection                                              */
    /********************************************************************/
#if defined(SAPonUNIX) || defined(SAPonOS400)
    gettimeofday(&sopenmsec, &tzp);
#else
    ftime(&sopenmsec);
#endif

    rfc_handle = RFC_connect((int) lpParms);


#if defined(SAPonUNIX) || defined(SAPonOS400)
    gettimeofday (&eopenmsec, &tzp);
    
    msecopen = diffmsec (&eopenmsec, &sopenmsec);
#else
    ftime (&eopenmsec);

    msecopen = diffmsec_NON_UNIX (&eopenmsec, &sopenmsec);
#endif

    
    if (rfc_handle == RFC_HANDLE_NULL)
    {
        THREAD_LEAVE ((int) lpParms);
    
        return 99;
    }


    /********************************************************************/
    /* Get specific data (attributes) of this RFC connection            */
    /********************************************************************/
    if (RfcGetAttributes (rfc_handle, &rfc_attributes))
    {
        rfc_error (cU("RfcGetAttributes"), (int) lpParms);
    
        RfcClose (rfc_handle);
    
        THREAD_LEAVE ((int) lpParms);
    
        return 99;
    }


    i = strlenU (rfc_attributes.user) - 1;
    
    while (rfc_attributes.user[i] == BLANK)
    {
        rfc_attributes.user[i] = 0;
    
        i = i - 1;
    }

    
    /* Output Open time and Attributes */
    if (sapsys[0] == RFC_SERVER_R3)
     sprintfU (dbuf, cU("CONNECT Time     handle = %4u   /%s/%s   ..... %5u msec"),
               rfc_handle,
               rfc_attributes.partner_host,
               rfc_attributes.systnr,
               msecopen);
    else
     sprintfU(dbuf, cU("CONNECT Time     handle = %4u                ..... %5u msec"), rfc_handle, msecopen);
    
    PRINT_RFC_CALL((int) lpParms, dbuf);

    
    /* Prepare internal table ITAB0332 */
    tables[0].name     = cU("ITAB0332");
    tables[0].nlen     = 8;
    tables[0].type     = TYPC;
    tables[0].leng     = ITAB0332_ENTRY_SIZE_RAW;
    tables[0].itmode   = RFC_ITMODE_BYREFERENCE;

    tables[0].ithandle = ItCreate(cU("ITAB0332"), ITAB0332_ENTRY_SIZE_RAW, 0, 0);

    if (tables[0].ithandle == NULL )
    {
        rfc_error (cU("ItCreate ITAB0332"), (int) lpParms);
    
        RfcClose(rfc_handle);
    
        THREAD_LEAVE( (int) lpParms);
    
        return 99;
    }


    /* Prepare internal table ITAB1000 */
    tables[1].name     = cU("ITAB1000");
    tables[1].nlen     = 8;
    tables[1].type     = TYPC;
    tables[1].leng     = ITAB1000_ENTRY_SIZE_RAW;
    tables[1].itmode   = RFC_ITMODE_BYREFERENCE;


    tables[1].ithandle = ItCreate(cU("ITAB1000"),ITAB1000_ENTRY_SIZE_RAW, 0, 0);


    if (tables[1].ithandle == NULL )
    {
        rfc_error(cU("ItCreate ITAB1000"), (int) lpParms);
    
        RfcClose(rfc_handle);
    
        THREAD_LEAVE( (int) lpParms);
    
        return 99;
    }


    /* Prepare internal table ETAB0332 */
    tables[2].name     = cU("ETAB0332");
    tables[2].nlen     = 8;
    tables[2].type     = TYPC;
    tables[2].leng     = ETAB0332_ENTRY_SIZE_RAW;
    tables[2].itmode   = RFC_ITMODE_BYREFERENCE;

    tables[2].ithandle = ItCreate(cU("ETAB0332"), ETAB0332_ENTRY_SIZE_RAW, 0, 0);

    if (tables[2].ithandle == NULL )
    {
        rfc_error(cU("ItCreate ETAB0332"), (int) lpParms);
    
        RfcClose(rfc_handle);
    
        THREAD_LEAVE( (int) lpParms);
    
        return 99;
    }


    /* Prepare internal table ETAB1000 */
    tables[3].name     = cU("ETAB1000");
    tables[3].nlen     = 8;
    tables[3].type     = TYPC;
    tables[3].leng     = ETAB1000_ENTRY_SIZE_RAW;
    tables[3].itmode   = RFC_ITMODE_BYREFERENCE;

    tables[3].ithandle = ItCreate(cU("ETAB1000"), ETAB1000_ENTRY_SIZE_RAW, 0, 0);


    if (tables[3].ithandle == NULL )
    {
        rfc_error(cU("ItCreate ETAB1000"), (int) lpParms);
    
        RfcClose(rfc_handle);
    
        THREAD_LEAVE( (int) lpParms);
    
        return 99;
    }


    for (nrcount=0; nrcount<nrcall; nrcount++)
    {
        /*----------------------------------------------------------------*/
        /* Call Performance Test                                          */
        /*----------------------------------------------------------------*/
        function = name_performance_test;

        memsetR(&exporting[0], 0, sizeofR(exporting));
        memsetR(&importing[0], 0, sizeofR(importing));

        rfc_param(&exporting[0], cU("CHECKTAB"), TYPC, checktab, 1*sizeofR(SAP_UC));
        rfc_param(&exporting[1], cU("LGIT0332"), TYPC, lgit0332_field, 5*sizeofR(SAP_UC));
        rfc_param(&exporting[2], cU("LGIT1000"), TYPC, lgit1000_field, 5*sizeofR(SAP_UC));
        rfc_param(&exporting[3], cU("LGET0332"), TYPC, lget0332_field, 5*sizeofR(SAP_UC));
        rfc_param(&exporting[4], cU("LGET1000"), TYPC, lget1000_field, 5*sizeofR(SAP_UC));

        rfc_param(&importing[0], cU("EXITCODE"), TYPC, exitcode, 1*sizeofR(SAP_UC));
        rfc_param(&importing[1], cU("TEXT0332"), TYPC, text0332, 72*sizeofR(SAP_UC));
        rfc_param(&importing[2], cU("TEXT1000"), TYPC, text1000, 72*sizeofR(SAP_UC));

        /* terminate array */
        tables[4].name = NULL;


        /* Fill table ITAB0332 as required */
        if (fill_table(tables[0].ithandle, lgit0332))
        {
            sprintfU (dbuf, cU("ERROR: Fill table itab0332"));
    
            PRINT_RFC_CALL((int) lpParms, dbuf);
    
            RfcClose(rfc_handle);
    
            THREAD_LEAVE( (int) lpParms);
    
            return 99;
        }

    
        /* Fill table ITAB1000 as required */
        if (fill_table(tables[1].ithandle, lgit1000))
        {
            sprintfU(dbuf, cU("ERROR: Fill table itab1000"));
    
            PRINT_RFC_CALL((int) lpParms, dbuf);
    
            RfcClose(rfc_handle);
    
            THREAD_LEAVE( (int) lpParms);
        
            return 99;
        }

#if defined(SAPonUNIX) || defined(SAPonOS400)
        gettimeofday(&scallmsec, &tzp);
#else
        ftime(&scallmsec);
#endif

        rfc_rc = RfcCallReceive (rfc_handle,
                                 function,
                                 exporting,
                                 importing,
                                 tables,
                                 &exception);

#if defined(SAPonUNIX) || defined(SAPonOS400)
        gettimeofday(&ecallmsec, &tzp);
    
        mseccall = diffmsec(&ecallmsec, &scallmsec);
#else
        ftime(&ecallmsec);
    
        mseccall = diffmsec_NON_UNIX(&ecallmsec, &scallmsec);
#endif

    
        /* check return code */
        switch (rfc_rc)
        {
            case RFC_OK:
                break;
    
            case RFC_EXCEPTION:
            case RFC_SYS_EXCEPTION:
            {
                rfc_error(exception, (int) lpParms);
                RfcClose(rfc_handle);
                THREAD_LEAVE( (int) lpParms);
                return 99;
            }
    
            default:
            {
                rfc_error(cU("RfcCallReceive"), (int) lpParms);
                RfcClose(rfc_handle);
                THREAD_LEAVE( (int) lpParms);
                return 99;
            }
        }

        
        /*----------------------------------------------------------------*/
        /* Check and output result of test                                */
        /*----------------------------------------------------------------*/
        if (exitcode[0] != cU('O'))
        {
            sprintfU(dbuf, cU("Wrong data received in R/3 !!!"));
            PRINT_RFC_CALL((int) lpParms, dbuf);
            RfcClose(rfc_handle);
            THREAD_LEAVE( (int) lpParms);
            return 99;
        }

    
        /* Check receive length, real length and contents of ETAB0332 */
        check_table (tables[2].ithandle, cU("ETAB0332"), lget0332, checktab, texi0332, exitcode);

        /* Check receive length and real length of ETAB1000 */
        check_table(tables[3].ithandle, cU("ETAB1000"), lget1000, checktab, texi1000, retcode);

    
        /*----------------------------------------------------------------*/
        /* Output result of test if everything OK                         */
        /*----------------------------------------------------------------*/
        if ((exitcode[0] != cU('O')) || (retcode[0] != cU('O')))
        {
            sprintfU(dbuf, cU("Wrong data received in R/3 !!!"));
            
            PRINT_RFC_CALL((int) lpParms, dbuf);
    
            RfcClose(rfc_handle);
    
            THREAD_LEAVE( (int) lpParms);
    
            return 99;
        }

    
        sprintfU (dbuf, cU("Execute Time     handle = %4u   Call-Nr: %3d ..... %5d msec"),
                  rfc_handle, nrcount+1, mseccall);
    
        PRINT_RFC_CALL((int) lpParms, dbuf);

    
        if (cU('Y') == do_structure_test[0])
        {
            /*----------------------------------------------------------------*/
            /* optional strcuture call                                        */
            /*----------------------------------------------------------------*/
            srfcstructure((int)lpParms, rfc_handle);
        }

    
        if (nrcount < nrcall-1)
        {
            /* Free all internal tables */
            ItFree(tables[0].ithandle);
    
            ItFree(tables[1].ithandle);
    
            ItFree(tables[2].ithandle);
    
            ItFree(tables[3].ithandle);

            /* Close and Reopen current RFC connection if required */
            /* or RFC server is external and register mode is used */
            if (use_close[0] == cU('Y'))
            {
                RfcClose(rfc_handle);

                /* LOAD BALANCING only for the first time */
                if ((sapsys[0] == RFC_SERVER_R3)      &&
                    (saprfc_ini[0] == cU('N'))        &&
                    (load_balancing[0] == cU('Y')))
                {
                    strcpyU(ashost, rfc_attributes.partner_host);
    
                    strcpyU(sysnr, rfc_attributes.systnr);
    
                    load_balancing[0] = cU('N');
                }

#if defined(SAPonUNIX) || defined(SAPonOS400)
                gettimeofday(&sopenmsec, &tzp);
#else
                ftime(&sopenmsec);
#endif

                rfc_handle = RFC_connect ((int) lpParms);

#if defined(SAPonUNIX) || defined(SAPonOS400)
                gettimeofday(&eopenmsec, &tzp);
    
                msecopen = diffmsec(&eopenmsec, &sopenmsec);
#else
                ftime(&eopenmsec);
    
                msecopen = diffmsec_NON_UNIX(&eopenmsec, &sopenmsec);
#endif

                if(rfc_handle == RFC_HANDLE_NULL)
                {
                    THREAD_LEAVE( (int) lpParms);
                
                    return 99;
                }

            
                /* Output Open time and Attributes */
                if (sapsys[0] == RFC_SERVER_R3)
                 sprintfU(dbuf, cU("CONNECT Time     handle = %4u   /%s/%s   ..... %5u msec"),
                          rfc_handle,
                          rfc_attributes.partner_host,
                          rfc_attributes.systnr,
                          msecopen);
                else
                 sprintfU (dbuf, cU("CONNECT Time     handle = %4u                ..... %5u msec"),
                           rfc_handle, msecopen);
    
                PRINT_RFC_CALL((int) lpParms, dbuf);

            }
        }
    }

    
    /*----------------------------------------------------------------*/
    /* Close RFC connection                                           */
    /*----------------------------------------------------------------*/
    RfcClose (rfc_handle);


    /*----------------------------------------------------------------*/
    /* Delete all used internal tables                                */
    /*----------------------------------------------------------------*/
    ItDelete(tables[0].ithandle);
    
    ItDelete(tables[1].ithandle);
    
    ItDelete(tables[2].ithandle);
    
    ItDelete(tables[3].ithandle);

    
    PRINT_RFC_CALL((int) lpParms, cU("DONE"));
    
    THREAD_LEAVE( (int) lpParms);
    
    
    return 0;
}







/* test heterogenuous structures and its thread consumption */
void srfcstructure(int tid, RFC_HANDLE hRfc)
{
    const SAP_RAW hex3_value[] = { 0x31, 0x32, 0x33 };
    RFC_RC	rfc_rc;
    RFC_TABLE	rfc_tables[2] ;
    RFC_PARAMETER	rfc_in__parameters[2];
    RFC_PARAMETER	rfc_out_parameters[3];
    RFCTEST             imstruct, exstruct, *pLine;
    SAP_CHAR            resptext[256];
    SAP_CHAR            *exception;
    void		*ptr;
    unsigned            lineleng;

    memsetR(rfc_tables, 0, sizeofR(rfc_tables));
    memsetR(rfc_in__parameters, 0, sizeofR(rfc_in__parameters));
    memsetR(rfc_out_parameters, 0, sizeofR(rfc_out_parameters));

    /* import parameters */
    rfc_in__parameters[0].name = cU("IMPORTSTRUCT");
    rfc_in__parameters[0].nlen = 12;
    rfc_in__parameters[0].addr = &imstruct;
    rfc_in__parameters[0].leng = sizeofR(RFCTEST);
    rfc_in__parameters[0].type = handleOfRfcTest;

    /* Fill structure 'IMPORTSTRUCT' */
    memsetR(&imstruct,0,sizeofR(imstruct));
    imstruct.rfcfloat       = 1000000.0;
    imstruct.rfcchar1[0]    = cU('A');
    imstruct.rfcint2        = 1000;
    imstruct.rfcint1        = 10;
    memcpyU(imstruct.rfcchar4, cU("EXT."), 4);
    imstruct.rfcint4        = 100000;
    memcpyR(imstruct.rfchex3,  hex3_value, 3);
    memcpyU(imstruct.rfcchar2, cU("BC"),       2);
    memcpyU(imstruct.rfctime,  cU("124500"),   6);
    memcpyU(imstruct.rfcdate,  cU("19960924"), 8);

    /* tables */
    rfc_tables[0].name     = cU("RFCTABLE");
    rfc_tables[0].nlen     = 8;
    rfc_tables[0].type     = handleOfRfcTest;
    rfc_tables[0].leng     = sizeofR(RFCTEST);
    rfc_tables[0].itmode   = RFC_ITMODE_BYREFERENCE;
    rfc_tables[0].ithandle = ItCreate(cU("RFCTABLE"),
				sizeofR(RFCTEST),
				0,
				0);
    if (ITAB_NULL == rfc_tables[0].ithandle)
    {
	rfc_error(cU("ItCreate"), tid);
	return;
    }
    /* Fill table 'RFCTABLE' */
    lineleng = ItLeng(rfc_tables[0].ithandle);
    ptr = ItAppLine(rfc_tables[0].ithandle);
    if (ptr == NULL)
    {
	rfc_error(cU("ItAppLine"), tid);
	ItDelete(rfc_tables[0].ithandle);
	return;
    }
    memcpyR(ptr, &imstruct, lineleng);

    /* export parameters */
    rfc_out_parameters[0].name = cU("RESPTEXT");
    rfc_out_parameters[0].nlen = 8;
    rfc_out_parameters[0].addr = resptext;
    rfc_out_parameters[0].leng = 255;
    rfc_out_parameters[0].type = TYPC;

    rfc_out_parameters[1].name = cU("ECHOSTRUCT");
    rfc_out_parameters[1].nlen = 10;
    rfc_out_parameters[1].addr = &exstruct;
    rfc_out_parameters[1].leng = sizeofR(RFCTEST);
    rfc_out_parameters[1].type = handleOfRfcTest;
    memsetR(&exstruct,0,sizeofR(exstruct));


    /* call ----------------------------------------------------------*/
    rfc_rc = RfcCallReceive(hRfc,
			    cU("STFC_STRUCTURE"),
			    rfc_in__parameters,
			    rfc_out_parameters,
			    rfc_tables,
			    &exception);
    /* check return code */
    switch(rfc_rc)
    {
      case RFC_OK:
	break;
      case RFC_EXCEPTION:
      case RFC_SYS_EXCEPTION:
      {
	rfc_error(exception, tid);
        THREAD_LEAVE( tid) ;
      }
	/*NOTREACHED*/
	break;
      default:
      {
	rfc_error(cU("RfcCallReceive"), tid);
        THREAD_LEAVE( tid) ;
      }
    }
    if((exstruct.rfcint4 != imstruct.rfcint4)
    || (exstruct.rfcint2 != imstruct.rfcint2)
    || (exstruct.rfcint1 != imstruct.rfcint1)
    )
    {
      SAP_CHAR buf[80];
      PRINT_RFC_CALL(tid, cU("STRUCTURE ERROR"));
      sprintfU(buf, cU("INT4: %d , %d"), exstruct.rfcint4, imstruct.rfcint4);
      PRINT_RFC_CALL(tid, buf);
      sprintfU(buf, cU("INT2: %d , %d"), exstruct.rfcint2, imstruct.rfcint2);
      PRINT_RFC_CALL(tid, buf);
      sprintfU(buf, cU("INT1: %d , %d"), exstruct.rfcint1, imstruct.rfcint1);
      PRINT_RFC_CALL(tid, buf);

    }
    else
    {
      PRINT_RFC_CALL(tid, cU("STRUCTURE OK"));
    }

    pLine = (RFCTEST*)ItGetLine(rfc_tables[0].ithandle, 1);
    if (NULL != pLine)
    {
	if((pLine->rfcint4 != (imstruct.rfcint4))
	|| (pLine->rfcint2 != (imstruct.rfcint2))
	|| (pLine->rfcint1 != (imstruct.rfcint1))
	)
	{
	  SAP_CHAR buf[80];
	  PRINT_RFC_CALL(tid, cU("TABLE ERROR"));
	  sprintfU(buf, cU("INT4: %d , %d"), pLine->rfcint4, imstruct.rfcint4);
	  PRINT_RFC_CALL(tid, buf);
	  sprintfU(buf, cU("INT2: %d , %d"), pLine->rfcint2, imstruct.rfcint2);
	  PRINT_RFC_CALL(tid, buf);
	  sprintfU(buf, cU("INT1: %d , %d"), pLine->rfcint1, imstruct.rfcint1);
	  PRINT_RFC_CALL(tid, buf);
	}
    }

    ItDelete(rfc_tables[0].ithandle);
}/*srfcstructure()*/

/*--------------------------------------------------------------------*/
/* Prepare and Open a RFC-Connection                                  */
/*--------------------------------------------------------------------*/
RFC_HANDLE RFC_connect(int thread_id)
{
  SAP_CHAR              connect_param[512];
  RFC_HANDLE            rfc_handle;
  RFC_ERROR_INFO_EX     error_info;

  memsetR(connect_param, 0, sizeofR(connect_param));

  sprintfU(connect_param, cU("TYPE=%c"), sapsys[0]);

  if (rfc_trace[0] == cU('Y')) 
    strcpyU(connect_param+strlenU(connect_param), cU(" TRACE=1"));

  if ((sapsys[0] == RFC_SERVER_R2) || (sapsys[0] == RFC_SERVER_R3))
  {
    if (memcmpU(client, cU("   "), 3) != 0)
      sprintfU(connect_param+strlenU(connect_param),
              cU(" CLIENT=%s"), client);

    sprintfU(connect_param+strlenU(connect_param),
          cU(" USER=%s PASSWD=%s"), userid, password);

    if (language[0] != cU(' '))
      sprintfU(connect_param+strlenU(connect_param),
              cU(" LANG=%s"), language);
  }

  if (saplogon[0] == cU('Y'))
  {
    /******************/
    /* Using saplogon */
    /******************/
    if (sapsys[0] == RFC_SERVER_R2)
      sprintfU(connect_param+strlenU(connect_param),
              cU(" SAPLOGON_ID=%s, DEST=%s"), saplogon_id, rfc_dest);
    else
      sprintfU(connect_param+strlenU(connect_param),
              cU(" SAPLOGON_ID=%s"), saplogon_id);
  }
  else
  {
    if (saprfc_ini[0] == cU('Y'))
    {
      /*****************************/
      /* Working with 'saprfc.ini' */
      /*****************************/
      sprintfU(connect_param+strlenU(connect_param),
              cU(" DEST=%s"), rfc_dest);
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
        sprintfU(connect_param+strlenU(connect_param),
                cU(" DEST=%s GWHOST=%s GWSERV=%s"),
                rfc_dest, gwhost, gwserv);
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
          sprintfU(connect_param+strlenU(connect_param),
                  cU(" GWHOST=%s GWSERV=%s TPNAME=%s"),
                  gwhost, gwserv, tpid);
        }
        else
        {
          /*===========================================*/
          /* RFC server will be started by SAP gateway */
          /*===========================================*/
          sprintfU(connect_param+strlenU(connect_param),
                  cU(" GWHOST=%s GWSERV=%s TPHOST=%s TPNAME=%s"),
                  gwhost, gwserv, tphost, tpname);
        }

        if (sncmode[0] == cU('Y'))
        {
          sprintfU(connect_param+strlenU(connect_param),
                  cU(" SNC_MODE=1 SNC_LIB=%s SNC_PARTNERNAME=%s"),
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
          if ((strlenU(mshost) == 1) && (mshost[0] == cU('0')))
          {
            /* Use sapmsg.ini of SAPLOGON */
            sprintfU(connect_param+strlenU(connect_param),
                    cU(" R3NAME=%s GROUP=%s"),
                    r3name, group);
          }
          else
            sprintfU(connect_param+strlenU(connect_param),
                    cU(" MSHOST=%s R3NAME=%s GROUP=%s"),
                    mshost, r3name, group);
        }
        else
        {
          /*------------------------------------------*/
          /* Connect to a specific application server */
          /*------------------------------------------*/
          sprintfU(connect_param+strlenU(connect_param),
                  cU(" ASHOST=%s SYSNR=%s"),
                  ashost, sysnr);

          if (sncmode[0] == cU('Y'))
          {
            sprintfU(connect_param+strlenU(connect_param),
                    cU(" SNC_MODE=1 SNC_LIB=%s SNC_PARTNERNAME=%s"),
                    snc_lib, snc_partnername);
          }
        }
      }
    }
  }

  rfc_handle = RfcOpenEx(connect_param,
                         &error_info);

  if (rfc_handle == RFC_HANDLE_NULL)
  {
    SAP_CHAR ebuf[1024];

    strcpyU(ebuf, cU("\nRFC RfcOpenEx error:\n"));
    sprintfU(ebuf+strlenU(ebuf), cU("\nGroup       Error group %d\n"), error_info.group);
    sprintfU(ebuf+strlenU(ebuf), cU("Key         %s\n"), error_info.key);
    sprintfU(ebuf+strlenU(ebuf), cU("Message     %s\n\n"), error_info.message);
    PRINT_RFC_CALL(thread_id, ebuf);
  }

  return rfc_handle;
}


/*--------------------------------------------------------------------*/
/* set up RFC parameters                                              */
/*--------------------------------------------------------------------*/
void rfc_param(RFC_PARAMETER *rfc_param,
	       SAP_CHAR      *abap_field_name,
	       unsigned      data_type,
	       void          *data_addr,
	       unsigned      data_len)
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
/* Error Cleanup because of a RFC-Error                               */
/* This function is defined as an error handler for the RFC library   */
/*--------------------------------------------------------------------*/
void DLL_CALL_BACK_FUNCTION rfclib_error(SAP_CHAR *operation)
{
  SAP_CHAR          ebuf[1024];
  RFC_ERROR_INFO_EX error_info;

  memsetR(ebuf, 0, sizeofR(ebuf));

  RfcLastErrorEx(&error_info);

  sprintfU(ebuf, cU("\nRFC Call/Exception:  %s\n"), operation);
  sprintfU(ebuf+strlenU(ebuf), cU("Group       Error group %d\n"), error_info.group);
  sprintfU(ebuf+strlenU(ebuf), cU("Key         %s\n"), error_info.key);
  sprintfU(ebuf+strlenU(ebuf), cU("Message     %s\n\n"), error_info.message);
  printfU(cU("%s"), ebuf);
#ifdef NDEBUG
  exit(3);
#else
 abort();   /* make postmoterm analyse*/
#endif
}


/*--------------------------------------------------------------------*/
/* Error Cleanup because of an RFC-Error                              */
/*--------------------------------------------------------------------*/
void rfc_error(SAP_CHAR *operation, int thread_id)
{
  SAP_CHAR          ebuf[1024];
  RFC_ERROR_INFO_EX error_info;

  memsetR(ebuf, 0, sizeofR(ebuf));

  RfcLastErrorEx(&error_info);

  sprintfU(ebuf, cU("\nRFC Call/Exception:  %s\n"), operation);
  sprintfU(ebuf+strlenU(ebuf), cU("Group       Error group %d\n"), error_info.group);
  sprintfU(ebuf+strlenU(ebuf), cU("Key         %s\n"), error_info.key);
  sprintfU(ebuf+strlenU(ebuf), cU("Message     %s\n"), error_info.message);
  PRINT_RFC_CALL(thread_id, ebuf);
  return;
}


/*--------------------------------------------------------------------*/
/* Search for certain parameter                                       */
/*--------------------------------------------------------------------*/
int PM_search(SAP_CHAR *param,
	      SAP_CHAR *text,
	      int  leng)
{
  SAP_CHAR  *pa,
            *pe;

  memsetU(param, 0, leng);
  pa = strstrU(sbuf, text);
  if (pa == NULL)
    return 99;

  pa = pa + strlenU(text);

  if (*pa == cU('"'))
  {
    pe = (SAP_CHAR *) memchrU(pa+1, cU('"'), leng);
    if (pe == NULL)
      znr = &sbuf[strlenU(sbuf)] - pa;
    else
      znr = pe - pa + 1;
  }
  else
  {
    pe = (SAP_CHAR *) memchrU(pa, cU(' '), leng);
    if (pe == NULL)
      znr = &sbuf[strlenU(sbuf)] - pa;
    else
      znr = pe - pa;
  }

  if (znr == 0)
    return 99;

  if (znr > leng)
    znr = leng;

  memcpyU(param, pa, znr);

  return 0;
}


/*--------------------------------------------------------------------*/
/* Get parameters for CONNECTION test                                 */
/*--------------------------------------------------------------------*/
void get_parameter_for_conn_test()
{

  /* Open the work file */
  work_fp = fopenU(work_file, cU("r"));
  if (work_fp == NULL)
  {
    get_connect_data_via_dialog();
    wbuf[strlenU(wbuf)] = cU('\n');
    strcpyU(wbuf+strlenU(wbuf), sbuf2);
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
/* Get parameters for test                                            */
/*--------------------------------------------------------------------*/
void get_parameter_for_load_test()
{
  memsetU(lgit0332_field, 0, 5);
  memsetU(lgit1000_field, 0, 5);
  memsetU(lget0332_field, 0, 5);
  memsetU(lget1000_field, 0, 5);

  /* Open the work file */
  work_fp = fopenU(work_file, cU("r"));
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
/* Get connect data from file                                         */
/*--------------------------------------------------------------------*/
void get_connect_data_from_file()
{
  if (read_record(work_fp, sbuf1) == NULL)
  {
    printfU(cU("\nCannot read data file\n"));
    exit(1);
  }
  strcpyU(sbuf, sbuf1);
  read_record(work_fp, sbuf2);

  get_connection_parameters_from_file();

  if ((sapsys[0] == RFC_SERVER_R2) ||  (sapsys[0] == RFC_SERVER_R3))
  {
    printfU(cU("\n\n               SAP LOGON DATA:"));

    /* Client */
    memsetU(client, 0, 3);
    rc = PM_search(client, cU("CLIENT="), 3);
    if (rc)
      strcpyU(client, cU("   "));
    printfU(cU("\n                 Client..................................: %s"), client);

    /* User Id */
    memsetU(userid, 0, 12);
    rc = PM_search(userid, cU("USERID="), 12);
    if (rc)
    {
      printfU(cU("\n\nInfo about User Id (USERID=) not found or not correct"));
      exit(1);
    }
    printfU(cU("\n                 UserID..................................: %s"), userid);

    /* Password */
    memsetU(password, 0, 9);
    rc = PM_search(password, cU("PASSWORD="), 8);
    if (rc)
    {
      printfU(cU("\n\nInfo about Password (PASSWORD=) not found or not correct"));
      exit(1);
    }
    printfU(cU("\n                 Password................................: XXXXXXXX"));
    znr = strlenU(password);
    for (i = 0; i < znr; i++)
      password[i] = (SAP_CHAR) (password[i] + 30 + i);

    /* Language */
    memsetU(language, 0, 3);
    rc = PM_search(language, cU("LANGUAGE="), 2);
    if (rc)
      strcpyU(language, cU("E"));
    printfU(cU("\n                 Language................................: %s"), language);
  }

  return;
}


/*--------------------------------------------------------------------*/
/* Get password for SAP logon                                         */
/*--------------------------------------------------------------------*/
void get_passwd(SAP_CHAR * pwd)
{
#if defined(SAPonUNIX)
  int  i;
  SAP_CHAR ibuf[256];

  systemU(cU("stty -echo"));
  getsU(ibuf);
  systemU(cU("stty echo"));

  strcpyU(pwd, ibuf);
  i = strlenU(pwd);
  if (i > 40)
    i = 40;
  pwd[i] = 0;
  memcpyU(ibuf, cU("****************************************"), i);
  printfU(cU("%s\n"), ibuf);

#elif defined(SAPonNT)
#include <conio.h>
  int  i, c;
  SAP_CHAR ibuf[256];

  for (i=0; i<8; i++)
  {
    c = getch();
    if (c != cU('\r'))
    {
      pwd[i] = (SAP_CHAR) c;
      printfU(cU("*"));
    }
    else
    {
      pwd[i] = 0;
      break;
    }
  }
  if (c != cU('\r'))
    getsU(ibuf);
  else
    printfU(cU("\n"));

#else
  gets(pwd);

#endif

  return;
}


/*--------------------------------------------------------------------*/
/* Get connect data via dialog                                        */
/*--------------------------------------------------------------------*/
void get_connect_data_via_dialog()
{
    memsetU(wbuf, 0, 512);

    get_connection_parameters_via_dialog();

    if (sapsys[0] == RFC_SERVER_EXT)
      sprintfU(wbuf+strlenU(wbuf), cU("\n"));
    else
    {
      printfU(cU("\n               SAP LOGON DATA:\n"));

      /* Client */
      if (strlenU(client) == 0)
	strcpyU(client, cU("000"));
      printfU(cU("                 Client..................................: (%s): "), client);
      fflush(stdout);
      getsU(ips);
      if (strlenU(ips) == 0)
	strcpyU(ips, client);
      ips[3] = 0;
      strcpyU(client, ips);
      if (client[0] == cU(' '))
	strcpyU(client, cU("   "));
      if (memcmpU(client, cU("   "), 3) != 0)
        sprintfU(wbuf+strlenU(wbuf), cU("CLIENT=%s "), client);

      /* User Id */
      if (strlenU(userid) == 0)
	strcpyU(userid, cU("SAPCPIC"));
      printfU(cU("                 UserID..................................: (%s): "), userid);
      fflush(stdout);
      getsU(ips);
      if (strlenU(ips) == 0)
	strcpyU(ips, userid);
      ips[12] = 0;
      strcpyU(userid, ips);
      znr = strlenU(ips);
      for (i = 0; i < znr; i++)
	userid[i] = (SAP_CHAR) toupperU(userid[i]);
      sprintfU(wbuf+strlenU(wbuf), cU("USERID=%s "), userid);

      /* Password */
      if (strlenU(password) == 0)
	strcpyU(password, cU("admin"));
      printfU(cU("                 Password................................: (XXXXXXXX):"));
      fflush(stdout);
      get_passwd(ips);
      if (strlenU(ips) == 0)
	strcpyU(ips, password);
      ips[8] = 0;
      strcpyU(password, ips);
      znr = strlenU(password);
      for (i = 0; i < znr; i++)
	password[i] = (SAP_CHAR) (toupperU(password[i]) - 30 - i);
      sprintfU(wbuf+strlenU(wbuf), cU("PASSWORD=%s "), password);
      for (i = 0; i < znr; i++)
	password[i] = (SAP_CHAR) (password[i] + 30 + i);

      /* Language */
      if (strlenU(language) == 0)
	strcpyU(language, cU("E"));
      printfU(cU("                 Language (E)............................: (%s): "), language);
      fflush(stdout);
      getsU(ips);
      if (strlenU(ips) == 0)
	strcpyU(ips, language);
      ips[2] = 0;
      strcpyU(language, ips);
      if (language[0] != cU(' '))
        sprintfU(wbuf+strlenU(wbuf), cU("LANGUAGE=%s \n"), language);
    }

    return;
}


/*--------------------------------------------------------------------*/
/* Get connection parameters from file                                */
/*--------------------------------------------------------------------*/
void get_connection_parameters_from_file()
{
  printfU(cU("\n\n               CONNECTION PARAMETERS:"));

  /* Type of RFC server: R/2 or R/3 or External */
  sapsys[0] = cU(' ');
  rc = PM_search(sapsys, cU("SAPSYS="), 1);
  if (rc)
    sapsys[0] = RFC_SERVER_R3;
  printfU(cU("\n                 Server is R/2, R/3 or External (2/3/E)3.: %c"), sapsys[0]);

  /* If External, using register mode ? */
  if (sapsys[0] == RFC_SERVER_EXT)
  {
    saplogon[0] = cU('N');
    memsetU(register_mode, 0, 1);
    rc = PM_search(register_mode, cU("REGMOD="), 1);
    if (rc)
      register_mode[0] = cU('Y');
    printfU(cU("\n                 Using registering feature (Y/N)Y........: %c"), register_mode[0]);
  }
  else
  {
    /* Using data of saplogon */
    saplogon[0] = cU(' ');
    rc = PM_search(saplogon, cU("SAPLOGON="), 1);
    if (rc)
      saplogon[0] = cU('N');
    printfU(cU("\n                 Using data of saplogon (Y/N)Y...........: %c"), saplogon[0]);
  }

  /* Using saplogon */
  if (saplogon[0] == cU('Y'))
  {
    /* ID in saplogon */
    memsetU(saplogon_id, 0, 256);
    rc = PM_search(saplogon_id, cU("SAPLOGON_ID="), 256);
    if (rc)
    {
      printfU(cU("\n\nInfo about saplogon ID (SAPLOGON_ID=) not found or not correct"));
      exit(1);
    }
    printfU(cU("\n                 ID in saplogon..........................: %s"), saplogon_id);

    /* If R/2, destination in sideinfo for gateway */
    if (sapsys[0] == RFC_SERVER_R2)
    {
      memsetU(rfc_dest, 0, 32);
      rc = PM_search(rfc_dest, cU("DEST="), 32);
      if (rc)
      {
        printfU(cU("\n\nInfo about destination (DEST=) not found or not correct"));
        exit(1);
      }
      printfU(cU("\n                 DEST in 'sideinfo' for the SAP gateway..: %s"), rfc_dest);
    }
  }
  else
  {
    /* Working with 'saprfc.ini' */
    saprfc_ini[0] = cU(' ');
    rc = PM_search(saprfc_ini, cU("RFCINI="), 1);
    if (rc)
      saprfc_ini[0] = cU('N');
    printfU(cU("\n                 Working with 'saprfc.ini' (Y/N)N........: %c"), saprfc_ini[0]);

    if (saprfc_ini[0] == cU('Y'))
    {
      /* Destination in 'saprfc.ini' */
      memsetU(rfc_dest, 0, 32);
      rc = PM_search(rfc_dest, cU("DEST="), 32);
      if (rc)
      {
        printfU(cU("\n\nInfo about destination (DEST=) not found or not correct"));
        exit(1);
      }
      printfU(cU("\n                 Destination in 'saprfc.ini'.............: %s"), rfc_dest);
    }
    else
    {
      if (sapsys[0] == RFC_SERVER_R2)
      {
        /* Destination in sideinfo */
        memsetU(rfc_dest, 0, 32);
        rc = PM_search(rfc_dest, cU("DEST="), 32);
        if (rc)
        {
	  printfU(cU("\n\nInfo about destination (DEST=) not found or not correct"));
	  exit(1);
        }
        printfU(cU("\n                 DEST in 'sideinfo' for the SAP gateway..: %s"), rfc_dest);

        memsetU(gwhost, 0, 100);
        rc = PM_search(gwhost, cU("GWHOST="), 100);
        if (rc)
        {
          printfU(cU("\n\nInfo about gateway host name (GWHOST=) not found or not correct"));
          exit(1);
        }
        printfU(cU("\n                 Host name of the SAP gateway to R/2.....: %s"), gwhost);

        memsetU(gwserv, 0, 7);
        rc = PM_search(gwserv, cU("GWSERV="), 7);
        if (rc)
        {
          printfU(cU("\n\nInfo about gateway service (GWSERV=) not found or not correct"));
          exit(1);
        }
        printfU(cU("\n                 Service of the specified SAP gateway....: %s"), gwserv);
      }
      else if (sapsys[0] == RFC_SERVER_EXT)
      {
        memsetU(gwhost, 0, 100);
        rc = PM_search(gwhost, cU("GWHOST="), 100);
        if (rc)
        {
          printfU(cU("\n\nInfo about gateway host name (GWHOST=) not found or not correct"));
          exit(1);
        }
        printfU(cU("\n                 Host name of the SAP gateway............: %s"), gwhost);

        memsetU(gwserv, 0, 7);
        rc = PM_search(gwserv, cU("GWSERV="), 7);
        if (rc)
        {
          printfU(cU("\n\nInfo about gateway service (GWSERV=) not found or not correct"));
          exit(1);
        }
        printfU(cU("\n                 Service of the specified SAP gateway....: %s"), gwserv);

        if (register_mode[0] == cU('Y'))
        {
          memsetU(tpid, 0, 100);
          rc = PM_search(tpid, cU("TPID="), 100);
          if (rc)
            strcpyU(tpid, stde_tpid);
          printfU(cU("\n                 Program ID of registered partner program: %s"), tpid);
        }
        else
        {
          memsetU(tphost, 0, 100);
          rc = PM_search(tphost, cU("TPHOST="), 100);
          if (rc)
            strcpyU(tphost, stde_tphost);
          printfU(cU("\n                 Host name of the partner program........: %s"), tphost);

          memsetU(tpname, 0, 100);
          rc = PM_search(tpname, cU("TPNAME="), 100);
          if (rc)
            strcpyU(tpname, stde_tpname);
          printfU(cU("\n                 Fully qualified name of partner program.: %s"), tpname);
        }
      }
      else
      {
        /* Use load balancing for connect to R/3 */
        memsetU(load_balancing, cU(' '), 1);
        rc = PM_search(load_balancing, cU("BALANCE="), 1);
        if (rc)
          load_balancing[0] = cU('Y');
        printfU(cU("\n                 Use load balancing (Y/N)Y...............: %c"), load_balancing[0]);

        if (load_balancing[0] == cU('Y'))
        {
          /* R/3 name */
          memsetU(r3name, 0, 4);
          rc = PM_search(r3name, cU("R3NAME="), 4);
          if (rc)
          {
            printfU(cU("\n\nInfo about R/3 name (R3NAME=) not found or not correct"));
            exit(1);
          }
          printfU(cU("\n                 R/3 system name.........................: %s"), r3name);

          /* R/3 message server */
          memsetU(mshost, 0, 100);
          rc = PM_search(mshost, cU("MSHOST="), 100);
          if (rc)
            strcpyU(mshost, cU("0"));
          printfU(cU("\n                 Message server or 0 for using sapmsg.ini: %s"), mshost);

          /* R/3 selected group */
          memsetU(group, 0, 32);
          rc = PM_search(group, cU("GROUP="), 32);
          if (rc)
	    strcpyU(group, cU("PUBLIC"));
          printfU(cU("\n                 Selected group..........................: %s"), group);
        }
        else
        {
          /* R/3 Application server */
          memsetU(ashost, 0, 100);
          rc = PM_search(ashost, cU("ASHOST="), 100);
          if (rc)
          {
            printfU(cU("\n\nInfo about application server (ASHOST=) not found or not correct"));
            exit(1);
          }
          printfU(cU("\n                 Host name of an application server......: %s"), ashost);

          /* R/3 System number */
          memsetU(sysnr, 0, 2);
          rc = PM_search(sysnr, cU("SYSNR="), 2);
          if (rc)
          {
            printfU(cU("\n\nInfo about system number (SYSNR=) not found or not correct"));
            exit(1);
          }
          printfU(cU("\n                 System number...........................: %s"), sysnr);
        }
      }
    }

    if ((sapsys[0] == RFC_SERVER_R3) || (sapsys[0] == RFC_SERVER_EXT))
    {
      /* Working with SNC */
      memsetU(sncmode, cU(' '), 1);
      rc = PM_search(sncmode, cU("SNC_MODE="), 1);
      if (rc)
        sncmode[0] = cU('N');
      printfU(cU("\n                 Working with SNC (Y/N)N.................: %c"), sncmode[0]);

      if (sncmode[0] == cU('Y'))
      {
        memsetU(snc_lib, cU(' '), 256);
        rc = PM_search(snc_lib, cU("SNC_LIB="), 256);
        if (rc)
        {
          ptr = getenvU(cU("SNC_LIB"));
          if (ptr != NULL)
          {
            ptr[256] = 0;
            strcpyU(snc_lib, ptr);
          }
          else
            strcpyU(snc_lib, std_snc_lib);
        }
        printfU(cU("\n                 SNC Library Name........................: %s"), snc_lib);

        memsetU(snc_partnername, cU(' '), 256);
        rc = PM_search(snc_partnername, cU("SNC_PARTNERNAME="), 256);
        if (rc)
          strcpyU(snc_partnername, std_snc_partnername);
        printfU(cU("\n                 SNC Name of the partner program.........: %s"), snc_partnername);
      }
    }
  }

  printfU(cU("\n\n               RFC-SPECIFIC PARAMETERS:"));
  if ((saplogon[0] == cU('Y')) || (saprfc_ini[0] == cU('N')))
  {
    /* RFC-Trace */
    memsetU(rfc_trace, cU(' '), 1);
    rc = PM_search(rfc_trace, cU("TRACE="), 1);
    if (rc)
      rfc_trace[0] = cU('N');
    printfU(cU("\n                 RFC-Trace (Y/N)N........................: %c"), rfc_trace[0]);
  }
  return;
}


/*--------------------------------------------------------------------*/
/* Get connection parameters via dialog                               */
/*--------------------------------------------------------------------*/
void get_connection_parameters_via_dialog()
{
  printfU(cU("\n\n               CONNECTION PARAMETERS:\n"));

  /* Type of RFC server: R/2 or R/3 or External */
  if (strlenU(sapsys) == 0)
    sapsys[0] = RFC_SERVER_R3;
  do
  {
    printfU(cU("                 Server is R/2, R/3 or External (2/3/E)3.: (%c): "), sapsys[0]);
    fflush(stdout);
    getsU(ips);
    if (strlenU(ips) == 0)
      ips[0] = sapsys[0];
    ips[0] = (SAP_CHAR) toupperU(ips[0]);
  } while ((ips[0] != RFC_SERVER_R2) && (ips[0] != RFC_SERVER_R3) && (ips[0] != RFC_SERVER_EXT));
  sapsys[0] = ips[0];
  sprintfU(wbuf+strlenU(wbuf), cU("SAPSYS=%c "), sapsys[0]);

  /* If External, using register mode ? */
  if (sapsys[0] == RFC_SERVER_EXT)
  {
    saplogon[0] = cU('N');
    if (strlenU(register_mode) == 0)
      register_mode[0] = cU('Y');
    do
    {
      printfU(cU("                 Using registering feature (Y/N)Y........: (%c): "), register_mode[0]);
      fflush(stdout);
      getsU(ips);
      if (strlenU(ips) == 0)
        ips[0] = register_mode[0];
      ips[0] = (SAP_CHAR) toupperU(ips[0]);
    } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
    register_mode[0] = ips[0];
    sprintfU(wbuf+strlenU(wbuf), cU("REGMOD=%c "), register_mode[0]);
  }
  else
  {
    /* Using saplogon */
#ifdef SAPonNT
    if (strlenU(saplogon) == 0)
      saplogon[0] = cU('Y');
    do
    {
      printfU(cU("                 Using data of saplogon (Y/N)Y...........: (%c): "), saplogon[0]);
      fflush(stdout);
      getsU (ips);
      if (strlenU(ips) == 0)
        ips[0] = saplogon[0];
      ips[0] = (SAP_CHAR) toupper(ips[0]);
    } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
    saplogon[0] = ips[0];
    sprintfU(wbuf+strlenU(wbuf), cU("SAPLOGON=%c "), saplogon[0]);
#else
    saplogon[0] = cU('N');
#endif
  }

  if (saplogon[0] == cU('Y'))
  {
    /* ID in saplogon */
    if (strlenU(saplogon_id) == 0)
      strcpyU(saplogon_id, std_saplogon_id);
    printfU(cU("                 ID in saplogon..........................: (%s): "), saplogon_id);
    fflush(stdout);
    getsU(ips);
    if (strlenU(ips) == 0)
      strcpyU(ips, saplogon_id);
    ips[256] = 0;
    strcpyU(saplogon_id, ips);
    check_blank(saplogon_id);
    sprintfU(wbuf+strlenU(wbuf), cU("SAPLOGON_ID=%s "), saplogon_id);

    /* If R/2, destination in sideinfo for gateway */
    if (sapsys[0] == RFC_SERVER_R2)
    {
      if (strlenU(rfc_dest) == 0)
        strcpyU(rfc_dest, std_rfc_dest_r2);
      printfU(cU("                 DEST in 'sideinfo' for the SAP gateway..: (%s): "), rfc_dest);
      fflush(stdout);
      getsU(ips);
      if (strlenU(ips) == 0)
        strcpyU(ips, rfc_dest);
      ips[32] = 0;
      strcpyU(rfc_dest, ips);
      znr = strlenU(rfc_dest);
      for (i = 0; i < znr; i++)
        rfc_dest[i] = (SAP_CHAR) toupperU(rfc_dest[i]);
      sprintfU(wbuf+strlenU(wbuf), cU("DEST=%s "), rfc_dest);
    }
  }
  else
  {
    /* Working with saprfc.ini */
    if (strlenU(saprfc_ini) == 0)
      saprfc_ini[0] = cU('N');
    do
    {
      printfU(cU("                 Working with 'saprfc.ini' (Y/N)N........: (%c): "), saprfc_ini[0]);
      fflush(stdout);
      getsU(ips);
      if (strlenU(ips) == 0)
        ips[0] = saprfc_ini[0];
      ips[0] = (SAP_CHAR) toupperU(ips[0]);
    } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
    saprfc_ini[0] = ips[0];
    sprintfU(wbuf+strlenU(wbuf), cU("RFCINI=%c "), saprfc_ini[0]);

    if (saprfc_ini[0] == cU('Y'))
    {
      /* Destination in saprfc.ini */
      if (strlenU(rfc_dest) == 0)
        strcpyU(rfc_dest, std_rfc_dest_r3);
      printfU(cU("                 Destination in 'saprfc.ini'.............: (%s): "), rfc_dest);
      fflush(stdout);
      getsU(ips);
      if (strlenU(ips) == 0)
        strcpyU(ips, rfc_dest);
      ips[32] = 0;
      strcpyU(rfc_dest, ips);
      znr = strlenU(rfc_dest);
      for (i = 0; i < znr; i++)
        rfc_dest[i] = (SAP_CHAR) toupperU(rfc_dest[i]);
      sprintfU(wbuf+strlenU(wbuf), cU("DEST=%s "), rfc_dest);
    }
    else
    {
      if (sapsys[0] == RFC_SERVER_R2)
      {
        /* Destination in sideinfo */
        if (strlenU(rfc_dest) == 0)
	  strcpyU(rfc_dest, std_rfc_dest_r2);
        printfU(cU("                 DEST in 'sideinfo' for the SAP gateway..: (%s): "), rfc_dest);
        fflush(stdout);
        getsU(ips);
        if (strlenU(ips) == 0)
	  strcpyU(ips, rfc_dest);
        ips[32] = 0;
        strcpyU(rfc_dest, ips);
        znr = strlenU(rfc_dest);
        for (i = 0; i < znr; i++)
	  rfc_dest[i] = (SAP_CHAR) toupperU(rfc_dest[i]);
        sprintfU(wbuf+strlenU(wbuf), cU("DEST=%s "), rfc_dest);

        /* Host name of a SAP gateway */
        if (strlenU(gwhost) == 0)
          strcpyU(gwhost, std_gwhost);
        printfU(cU("                 Host name of the SAP gateway to R/2.....: (%s): "), gwhost);
        fflush(stdout);
        getsU(ips);
        if (strlenU(ips) == 0)
          strcpyU(ips, gwhost);
        ips[100] = 0;
        strcpyU(gwhost, ips);
        sprintfU(wbuf+strlenU(wbuf), cU("GWHOST=%s "), gwhost);

        /* Service of the specified SAP gateway */
        if (strlenU(gwserv) == 0)
          strcpyU(gwserv, std_gwserv);
        printfU(cU("                 Service of the specified gateway........: (%s): "), gwserv);
        fflush(stdout);
        getsU(ips);
        if (strlenU(ips) == 0)
          strcpyU(ips, gwserv);
        ips[7] = 0;
        strcpyU(gwserv, ips);
        sprintfU(wbuf+strlenU(wbuf), cU("GWSERV=%s "), gwserv);
      }
      else if (sapsys[0] == RFC_SERVER_EXT)
      {
        /* Host name of a SAP gateway */
        if (strlenU(gwhost) == 0)
          strcpyU(gwhost, stde_gwhost);
        printfU(cU("                 Host name of the SAP gateway............: (%s): "), gwhost);
        fflush(stdout);
        getsU(ips);
        if (strlenU(ips) == 0)
          strcpyU(ips, gwhost);
        ips[100] = 0;
        strcpyU(gwhost, ips);
        sprintfU(wbuf+strlenU(wbuf), cU("GWHOST=%s "), gwhost);

        /* Service of the specified SAP gateway */
        if (strlenU(gwserv) == 0)
          strcpyU(gwserv, stde_gwserv);
        printfU(cU("                 Service of the specified gateway........: (%s): "), gwserv);
        fflush(stdout);
        getsU(ips);
        if (strlenU(ips) == 0)
          strcpyU(ips, gwserv);
        ips[7] = 0;
        strcpyU(gwserv, ips);
        sprintfU(wbuf+strlenU(wbuf), cU("GWSERV=%s "), gwserv);

        /* Program ID of partner program if working with register mode */
        if (register_mode[0] == cU('Y'))
        {
          if (strlenU(tpid) == 0)
            strcpyU(tpid, stde_tpid);
          printfU(cU("                 Program ID of registered partner program: (%s): "), tpid);
          fflush(stdout);
          getsU(ips);
          if (strlenU(ips) == 0)
            strcpyU(ips, tpid);
          ips[100] = 0;
          strcpyU(tpid, ips);
          sprintfU(wbuf+strlenU(wbuf), cU("TPID=%s "), tpid);
        }
        else
        {
          /* Host name of partner program */
	  if (strlenU(tphost) == 0)
	    strcpyU(tphost, stde_tphost);
	  printfU(cU("                 Host name of the partner program........: (%s): "), tphost);
	  fflush(stdout);
	  getsU(ips);
	  if (strlenU(ips) == 0)
	    strcpyU(ips, tphost);
	  ips[100] = 0;
	  strcpyU(tphost, ips);
	  sprintfU(wbuf+strlenU(wbuf), cU("TPHOST=%s "), tphost);

	  /* Name of the partner program */
	  if (strlenU(tpname) == 0)
	    strcpyU(tpname, stde_tpname);
	  printfU(cU("                 Fully qualified name of partner program.: (%s): "), tpname);
	  fflush(stdout);
	  getsU(ips);
	  if (strlenU(ips) == 0)
	    strcpyU(ips, tpname);
	  ips[100] = 0;
	  strcpyU(tpname, ips);
	  sprintfU(wbuf+strlenU(wbuf), cU("TPNAME=%s "), tpname);
        }
      }
      else
      {
        /* Working with load balancing */
        if (strlenU(load_balancing) == 0)
          load_balancing[0] = cU('Y');
        do
        {
          printfU(cU("                 Use load balancing (Y/N)Y...............: (%c): "), load_balancing[0]);
          fflush(stdout);
          getsU(ips);
          if (strlenU(ips) == 0)
            ips[0] = load_balancing[0];
          ips[0] = (SAP_CHAR) toupperU(ips[0]);
        } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
        load_balancing[0] = ips[0];
        sprintfU(wbuf+strlenU(wbuf), cU("BALANCE=%c "), load_balancing[0]);

        if (load_balancing[0] == cU('Y'))
        {
          /* R/3 System name */
          if (strlenU(r3name) == 0)
            strcpyU(r3name, std_r3name);
          printfU(cU("                 R/3 system name.........................: (%s): "), r3name);
          fflush(stdout);
          getsU(ips);
	  if (strlenU(ips) == 0)
	    strcpyU(ips, r3name);
          ips[100] = 0;
          strcpyU(r3name, ips);
          if ((r3name[0] == cU('/')) || (r3name[0] == cU('"')))
          {
            ptr = r3name+strlenU(r3name)-1;
            while (*ptr != cU('/'))
            {
              *ptr = (SAP_CHAR) toupperU(*ptr);
              ptr--;
            }
          }
          else
          {
	    znr = strlenU(r3name);
            for (i = 0; i < znr; i++)
              r3name[i] = (SAP_CHAR) toupperU(r3name[i]);
          }
          sprintfU(wbuf+strlenU(wbuf), cU("R3NAME=%s "), r3name);

	  /* R/3 message server */
          if (strlenU(mshost) == 0)
            strcpyU(mshost, cU("0"));
          printfU(cU("                 Message server or 0 for using sapmsg.ini: (%s): "), mshost);
	  fflush(stdout);
	  getsU(ips);
	  if (strlenU(ips) == 0)
	    strcpyU(ips, mshost);
	  ips[100] = 0;
	  strcpyU(mshost, ips);
	  sprintfU(wbuf+strlenU(wbuf), cU("MSHOST=%s "), mshost);

	  /* R/3 Selected group */
	  if (strlenU(group) == 0)
	    strcpyU(group, std_group);
	  printfU(cU("                 Selected group (PUBLIC).................: (%s): "), group);
	  fflush(stdout);
	  getsU(ips);
	  if (strlenU(ips) == 0)
	    strcpyU(ips, group);
	  ips[32] = 0;
	  strcpyU(group, ips);
	  check_blank(group);
	  sprintfU(wbuf+strlenU(wbuf), cU("GROUP=%s "), group);
        }
        else
        {
	  /* R/3 application server */
	  if (strlenU(ashost) == 0)
	    strcpyU(ashost, std_ashost);
	  printfU(cU("                 Host name of an application server......: (%s): "), ashost);
	  fflush(stdout);
	  getsU(ips);
	  if (strlenU(ips) == 0)
	    strcpyU(ips, ashost);
	  ips[100] = 0;
	  strcpyU(ashost, ips);
	  sprintfU(wbuf+strlenU(wbuf), cU("ASHOST=%s "), ashost);

	  /* R/3 System number */
	  if (strlenU(sysnr) == 0)
	    strcpyU(sysnr, std_sysnr);
	  printfU(cU("                 System number...........................: (%s): "), sysnr);
	  fflush(stdout);
	  getsU(ips);
	  if (strlenU(ips) == 0)
	    strcpyU(ips, sysnr);
	  ips[2] = 0;
	  strcpyU(sysnr, ips);
	  sprintfU(wbuf+strlenU(wbuf), cU("SYSNR=%s "), sysnr);
        }
      }
      if ((sapsys[0] == RFC_SERVER_R3) || (sapsys[0] == RFC_SERVER_EXT))
      {
        /* Working with SNC */
        if (strlenU(sncmode) == 0)
          sncmode[0] = cU('N');
        do
        {
          printfU(cU("                 Working with SNC (Y/N)N.................: (%c): "), sncmode[0]);
          fflush(stdout);
          getsU(ips);
          if (strlenU(ips) == 0)
            ips[0] = sncmode[0];
          ips[0] = (SAP_CHAR) toupperU(ips[0]);
        } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
        sncmode[0] = ips[0];
        sprintfU(wbuf+strlenU(wbuf), cU("SNC_MODE=%c "), sncmode[0]);

        if (sncmode[0] == cU('Y'))
        {
          /* SNC Library */
          if (strlenU(snc_lib) == 0)
          {
            ptr = getenvU(cU("SNC_LIB"));
            if (ptr != NULL)
            {
              ptr[256] = 0;
              strcpyU(snc_lib, ptr);
            }
            else
              strcpyU(snc_lib, std_snc_lib);
          }
          printfU(cU("                 SNC Library Name........................: (%s): "), snc_lib);
          fflush(stdout);
          getsU(ips);
          if (strlenU(ips) == 0)
            strcpyU(ips, snc_lib);
          ips[256] = 0;
          strcpyU(snc_lib, ips);
          check_blank(snc_lib);
          sprintfU(wbuf+strlenU(wbuf), cU("SNC_LIB=%s "), snc_lib);

          /* SNC Partner Name */
          if (strlenU(snc_partnername) == 0)
            strcpyU(snc_partnername, std_snc_partnername);
          printfU(cU("                 SNC name of partner program.............: (%s): "), snc_partnername);
          fflush(stdout);
          getsU(ips);
          if (strlenU(ips) == 0)
            strcpyU(ips, snc_partnername);
          ips[256] = 0;
          strcpyU(snc_partnername, ips);
          check_blank(snc_partnername);
          sprintfU(wbuf+strlenU(wbuf), cU("SNC_PARTNERNAME=%s "), snc_partnername);
        }
      }
    }
  }

  printfU(cU("\n               RFC-SPECIFIC PARAMETERS:\n"));

  /* RFC-Trace */
  if ((saplogon[0] == cU('Y')) || (saprfc_ini[0] == cU('N')))
  {
    if (strlenU(rfc_trace) == 0)
      rfc_trace[0] = cU('N');
    do
    {
      printfU(cU("                 RFC trace (Y/N)N........................: (%c): "), rfc_trace[0]);
      fflush(stdout);
      getsU(ips);
      if (strlenU(ips) == 0)
        ips[0] = rfc_trace[0];
      ips[0] = (SAP_CHAR) toupperU(ips[0]);
    } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
    rfc_trace[0] = ips[0];
    sprintfU(wbuf+strlenU(wbuf), cU("RFCTRACE=%c "), rfc_trace[0]);
  }
  return;
}


/*--------------------------------------------------------------------*/
/* Save parameters for next tests                                     */
/*--------------------------------------------------------------------*/
void save_test_parameter()
{

  /* Open the work file */
  work_fp = fopenU(work_file, cU("w"));
  if (work_fp == NULL)
  {
    printfU(cU("\n\nCan not save RFC-parameters for next tests"));
    printfU(cU("\n\nHit return key to continue\n"));
    fflush(stdout);
    getcharU();
  }
  else
  {
    fputsU(wbuf, work_fp);
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
  strcpyU(sbuf, sbuf2);

  printfU(cU("\n\n               PERFORMANCE TEST DATA:"));

  /* No of lines for ITAB0332 */
  memsetU(lgit0332_field, cU('0'), 5);
  lgit0332 = atolU(lgit0332_field);

  /* No of lines for ITAB1000 */
  memsetU(lgit1000_field, 0, 5);
  rc = PM_search(lgit1000_field, cU("LGIT1000="), 5);
  if (rc)
    strcpyU(lgit1000_field, cU("01000"));

  lgit1000 = atolU(lgit1000_field);
  sprintfU(lgit1000_field, cU("%05d"), lgit1000);
  printfU(cU("\n                 #Lines in export table (length 1000)....: %s"), lgit1000_field);

  /* No of lines for ETAB0332 */
  memsetU(lget0332_field, cU('0'), 5);
  lget0332 = atolU(lget0332_field);

  /* No of lines for ETAB1000 */
  memsetU(lget1000_field, 0, 5);
  rc = PM_search(lget1000_field, cU("LGET1000="), 5);
  if (rc)
    strcpyU(lgit1000_field, cU("01000"));

  lget1000 = atolU(lget1000_field);
  sprintfU(lget1000_field, cU("%05d"), lget1000);
  printfU(cU("\n                 #Lines in import table (length 1000)....: %s"), lget1000_field);

    printfU(cU("\n\n               MARSHALING TEST:"));
    {
	memsetU(do_structure_test, cU(' '), 1);
	rc = PM_search(do_structure_test, cU("STR_CHECK="), 1);
	if (rc)
	do_structure_test[0] = cU('N');
    }
    printfU(cU("\n                 Do STFC_STRUCTURE TEST..........(Y/N)N: %c"), do_structure_test[0]);

  printfU(cU("\n\n               MULTI-THREADED TEST DATA:"));

  /* Number of Calls in one thread */
  memsetU(nrcall_field, 0, 3);
  rc = PM_search(nrcall_field, cU("NRCALL="), 3);
  if (rc)
    strcpyU(nrcall_field, cU("001"));

  nrcall = atolU(nrcall_field);
  sprintfU(nrcall_field, cU("%03d"), nrcall);
  printfU(cU("\n                 #Calls of an RFC-function in one thread.: %s"), nrcall_field);

  /* Close after each RFC-call */
  if (nrcall == 1)
    use_close[0] = cU('N');
  else
  {
    if ((sapsys[0] == RFC_SERVER_EXT) && (register_mode[0] == cU('Y')))
      use_close[0] = cU('Y');
    else
    {
      memsetU(use_close, cU(' '), 1);
      rc = PM_search(use_close, cU("CLOSE="), 1);
      if (rc)
	use_close[0] = cU('N');
    }
    printfU(cU("\n                 Close connection after each call (Y/N)N.: %c"), use_close[0]);
  }

  /* No. of threads */
#ifdef RFCwithTHREADS
  memsetU(thread_no_field, 0, 3);
  rc = PM_search(thread_no_field, cU("THREAD="), 3);
  if (rc)
    strcpyU(thread_no_field, cU("001"));

  thread_no = atolU(thread_no_field);
  if (thread_no > RFCT_MAX_THREADS)
  {
	thread_no = RFCT_MAX_THREADS;
	printfU( cU("\nthread number truncated to %d\n"), thread_no);
  }
  sprintfU(thread_no_field, cU("%03d"), thread_no);
  printfU(cU("\n                 #Threads to be started (Max. 999).......: %s"), thread_no_field);
#endif

  /* Last confirm for test */
  printfU(cU("\n\n"));
if (0 == silent_mode)
{
  do
  {
    printfU(cU("            Do You Want To Test With These Parameters (Y/N)..: "));
    fflush(stdout);
    getsU(ips);
    ips[0] = (SAP_CHAR) toupperU(ips[0]);
  } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));

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
}
else
{
	if (work_fp != NULL)
    {
      fclose(work_fp);
      work_fp = NULL;
    }
 
}
  return;
}


/*--------------------------------------------------------------------*/
/* Get load test data via dialog                                      */
/*--------------------------------------------------------------------*/
void get_load_test_data_via_dialog(void)
{
  printfU(cU("\n               PERFORMANCE TEST DATA:\n"));

  /* No of lines for ITAB0332 */
  strcpyU(lgit0332_field, cU("00000"));
  lgit0332 = atolU(lgit0332_field);

  /* No of lines for ITAB1000 */
  if (strlenU(lgit1000_field) == 0)
    strcpyU(lgit1000_field, cU("01000"));
  printfU(cU("                 #Lines in export table (length 1000)....: (%s): "), lgit1000_field);
  fflush(stdout);
  getsU(ips);
  if (strlenU(ips) == 0)
    strcpyU(ips, lgit1000_field);
  ips[5] = 0;
  strcpyU(lgit1000_field, ips);
  lgit1000 = atolU(lgit1000_field);
  sprintfU(lgit1000_field, cU("%05d"), lgit1000);
  sprintfU(wbuf+strlenU(wbuf), cU("LGIT1000=%s "), lgit1000_field);

  /* No of lines for ETAB0332 */
  strcpyU(lget0332_field, cU("00000"));
  lget0332 = atolU(lget0332_field);

  /* No of lines for ETAB1000 */
  if (strlenU(lget1000_field) == 0)
    strcpyU(lget1000_field, cU("01000"));
  printfU(cU("                 #Lines in import table (length 1000)....: (%s): "), lget1000_field);
  fflush(stdout);
  getsU(ips);
  if (strlenU(ips) == 0)
    strcpyU(ips, lget1000_field);
  ips[5] = 0;
  strcpyU(lget1000_field, ips);
  lget1000 = atolU(lget1000_field);
  sprintfU(lget1000_field, cU("%05d"), lget1000);
  sprintfU(wbuf+strlenU(wbuf), cU("LGET1000=%s "), lget1000_field);

  printfU(cU("\n               MARSHALLING TEST:\n"));

    if (strlenU(do_structure_test) == 0)
    do_structure_test[0] = cU('N');
    do
    {
    printfU(cU("                 Do STFC_STRUCTURE TEST........... (Y/N)N.: (%c): "), do_structure_test[0]);
	fflush(stdout);
	getsU(ips);
	if (strlenU(ips) == 0)
	    ips[0] = do_structure_test[0];
	ips[0] = (SAP_CHAR) toupperU(ips[0]);
    } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
    do_structure_test[0] = ips[0];
    sprintfU(wbuf+strlenU(wbuf), cU("STR_CHECK=%c "), do_structure_test[0]);

  printfU(cU("\n               MULTI-THREADED TEST DATA:\n"));

  /* Number of Calls of this RFC-function */
  if (strlenU(nrcall_field) == 0)
    strcpyU(nrcall_field, cU("001"));
  printfU(cU("                 #Calls of an RFC-function in one thread.: (%s): "), nrcall_field);
  fflush(stdout);
  getsU(ips);
  if (strlenU(ips) == 0)
    strcpyU(ips, nrcall_field);
  ips[3] = 0;
  strcpyU(nrcall_field, ips);
  nrcall = atolU(nrcall_field);
  sprintfU(nrcall_field, cU("%03d"), nrcall);
  sprintfU(wbuf+strlenU(wbuf), cU("NRCALL=%s "), nrcall_field);

  /* Close after each RFC call */
  if (nrcall == 1)
    use_close[0] = cU('N');
  else
  {
    if ((sapsys[0] == RFC_SERVER_EXT) && (register_mode[0] == cU('Y')))
    {
      use_close[0] = cU('Y');
      printfU(cU("                 Close connection after each call (Y/N)N.: (%c): \n"), use_close[0]);
    }
    else
    {
      if (strlenU(use_close) == 0)
	use_close[0] = cU('N');
      do
      {
	printfU(cU("                 Close connection after each call (Y/N)N.: (%c): "), use_close[0]);
	fflush(stdout);
	getsU(ips);
	if (strlenU(ips) == 0)
	  ips[0] = use_close[0];
	ips[0] = (SAP_CHAR) toupperU(ips[0]);
      } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));
      use_close[0] = ips[0];
    }
  }
  sprintfU(wbuf+strlenU(wbuf), cU("CLOSE=%c "), use_close[0]);

  /* Number of threads */
#ifdef RFCwithTHREADS
  if (strlenU(thread_no_field) == 0)
    strcpyU(thread_no_field, cU("001"));
  printfU(cU("                 #Threads to be started (Max. 999).......: (%s): "), thread_no_field);
  fflush(stdout);
  getsU(ips);
  if (strlenU(ips) == 0)
    strcpyU(ips, thread_no_field);
  ips[3] = 0;
  strcpyU(thread_no_field, ips);
  thread_no = atolU(thread_no_field);
  if (thread_no > RFCT_MAX_THREADS)
  {
	thread_no = RFCT_MAX_THREADS;
	printfU( cU("\nthread number truncated to %d\n"), thread_no);
  }
  sprintfU(thread_no_field, cU("%03d"), thread_no);
  sprintfU(wbuf+strlenU(wbuf), cU("THREAD=%s "), thread_no_field);
#endif

  /* Last confirm for test */
  printfU(cU("\n"));
  do
  {
    printfU(cU("            Do You Want To Test With These Parameters (Y/N)..: "));
    fflush(stdout);
    getsU(ips);
    ips[0] = (SAP_CHAR) toupperU(ips[0]);
  } while ((ips[0] != cU('Y')) && (ips[0] != cU('N')));

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
/* Check blank in buffer and put "..." if necessary                   */
/*--------------------------------------------------------------------*/
void check_blank(SAP_CHAR *text)
{
  int   len;
  SAP_CHAR *pa,
            buf[512];

  len = strlenU(text);
  if ((text[0] == cU('"')) && (text[len-1] != cU('"')))
  {
    text[len]   = cU('"');
    text[len+1] = 0;
  }

  pa = (SAP_CHAR *) memchrU(text, cU(' '), len);
  if ((pa != NULL) && (text[0] != cU('"')))
  {
    strcpyU(buf, text);
    text[0] = cU('"');
    strcpyU(text+1, buf);
    text[len+1] = cU('"');
    text[len+2] = 0;
  }
  return;
}


/*--------------------------------------------------------------------*/
/* read record from file with single or multi line                    */
/*--------------------------------------------------------------------*/
SAP_CHAR *read_record(FILE     *fp,
	 	      SAP_CHAR *ibuf)
{
  int        offset = 0;
  SAP_CHAR  *ptr,
	     line[LINE_SIZE+1];

  ibuf[0] = 0;

  for (;;)
  {
    if ((ptr = fgetsU(line, LINE_SIZE, fp)) == NULL)
      return (NULL);

    if (line[strlenU(line)-1] == cU('\n'))
      line[strlenU(line)-1] = 0;

    ptr = line;
    switch (*ptr)
    {
      case cU('\0'): continue;
      case cU('*') : continue;
      case cU(' ') : continue;
      default  : ;
    }

    strcpyU(ibuf+offset, ptr);

    if (ibuf[strlenU(ibuf)-1] == cU('&'))
    {
      ibuf[strlenU(ibuf)-1] = 0;
      offset = strlenU(ibuf);
      continue;
    }
    break;
  }
  return (ibuf);
}


/*--------------------------------------------------------------------*/
/* Fill internal table as required                                    */
/*--------------------------------------------------------------------*/
int fill_table (ITAB_H itab_h,
	            long   leng)
{
    unsigned      ulen;
    long          position,
                  lineleng,
                  linenr;
    SAP_CHAR    * ptr,
                  etext[1001];

    if (leng == 0)
     return 0;

    
    lineleng = ulen = ItLeng(itab_h)/sizeofR(SAP_UC);

    
    memsetU (etext, BLANK, 1000);


    for (position=40; position<lineleng; position++)
    {
        sprintfU (etext+position, cU("%01d"), position % 10);
    }

    
    sprintfU(etext, cU("Line Length: %04d   Line No.: "), lineleng);

    
    for (linenr=1; linenr<=leng; linenr++)
    {
        sprintfU(etext+30, cU("%05d"), linenr);
        etext[35] = cU(' ');

        ptr = (SAP_CHAR *) ItAppLine(itab_h);
    
        if (ptr == NULL) break;
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
int check_table (ITAB_H       itab_h,
                 SAP_CHAR   * itabname,
                 long         recvleng,
                 SAP_CHAR   * checktab,
                 SAP_CHAR   * rtext,
                 SAP_CHAR   * retcode)
{
    unsigned    ulen;
    long        position,
                lineleng,
                itableng;
    SAP_CHAR    value[2],
                tbuff[1001],
                etext[73]   = iU(" "),
                errline[40] = iU("ERROR:  #Lines = nnnnn  Expected: nnnnn"),
                errcont[40] = iU("ERROR at line nnnnn position nnnn:  n/n"),
                mtext[73]   = iU("Import table (length nnnn):  #Lines = nnnnn: OK,  Contents: OK");

    
    memsetU (etext, cU(' '), 72);

    itableng = ItFill(itab_h);

    if (itableng != 0)
     itableng = ItFill(itab_h);

    
    lineleng = ulen = ItLeng(itab_h)/sizeofR(SAP_UC);

    strcpyU (etext, mtext);
    etext[strlenU(etext)] = cU(' ');


    if (itabname[0] == cU('I'))
     memcpyU(etext, cU("Ex"), 2);

    memcpyU (etext+21, itabname+4, 4);

    
    if (itableng != recvleng)
    {
        memcpyU (etext+28, errline, 39);
    
        sprintfU (etext+45, cU("%05d"), itableng);
    
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
         memcpyU (etext+60, cU("NO CHECK"), 8);
        else
        {
            for (j = 1; ; j++)
            {
                SAP_CHAR *ptr = (SAP_CHAR*) ItGetLine(itab_h, j);

                if (ptr == NULL) 
                 break;

                memcpyU (tbuff, ptr, ulen);
    
                tbuff[lineleng] = 0;

                for (position=40; position<lineleng; position++)
                {
                    sprintfU(value, cU("%01d"), position % 10);
    
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
/* Output itab on screen                                              */
/*--------------------------------------------------------------------*/
int output_table (ITAB_H      itab_h,
		          SAP_CHAR   *itabname)
{
    unsigned  ulen;
    long      lineleng;
    SAP_CHAR  tbuff[1001];

    lineleng = ulen = ItLeng(itab_h)/sizeofR(SAP_UC);

    printfU(cU("\n%s:\n"), itabname);

    for (j = 1; ; j++)
    {
        SAP_CHAR *ptr = (SAP_CHAR *) ItGetLine(itab_h, j);
    
        if (ptr == NULL) 
         break;

        memcpyU (tbuff, ptr, ulen);
    
        tbuff[lineleng] = 0;
    
        printfU(cU("'%s'\n"), tbuff);
    }
    
    return 0;
}


/*--------------------------------------------------------------------*/
/* Compute time in millisec.                                          */
/*--------------------------------------------------------------------*/
#if defined(SAPonUNIX) || defined(SAPonOS400)
unsigned long int diffmsec (struct timeval *tmend,
			    struct timeval *tmstart)
{
  unsigned long int msectime;

  if (tmstart->tv_usec > tmend->tv_usec)
  {
    tmend->tv_usec += 1000000;
    tmend->tv_sec--;
  }
  msectime = (tmend->tv_usec - tmstart->tv_usec)/1000 +
	     (tmend->tv_sec - tmstart->tv_sec)*1000;

  return msectime;
}
#else
unsigned long int diffmsec_NON_UNIX (struct timeb *tmend,
				     struct timeb *tmstart)
{
  unsigned long int msectime;

  if (tmstart->millitm > tmend->millitm)
  {
      tmend->millitm = tmend->millitm + 1000;
      tmend->time = tmend->time - 1;
  }
  msectime = ((long)tmend->millitm - (long)tmstart->millitm) +
	     (tmend->time - tmstart->time)*1000;

  return msectime;
}
#endif

