/* static char sccsid[] = "@(#) $Id: //bas/640_REL/src/krn/rfc/rfcthsrv.c#1 $ SAP";  */
/*====================================================================*/
/*                                                                    */
/*     PROGRAM     :   rfcthsrv.c (Windows NT/95)                     */
/*                                                                    */
/*                                                                    */
/*     DESCRIPTION :   Test program for multi threaded RFC library    */
/*                                                                    */
/*                     - The function STFC_PERFORMANCE can be called  */
/*                       by an R/2, R/3 or an external program.       */
/*                                                                    */
/*                     - The function STFC_STRUCTURE   can be called  */
/*                       by an R/2, R/3 or an external program.       */
/*                                                                    */
/*                     - If running in registered mode, the number of */
/*                       of threads can be defined in the command     */
/*                       line followed by -T (Default: 1).            */
/*                                                                    */
/*                     - Each thread handles only one RFC connection. */
/*                                                                    */
/*                     - If you want to compile/link this program     */
/*                       in your UNIX-environment you must set the    */
/*                       flag SAPonUNIX.                              */
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
#if defined(SAPwithTHREADS) && defined(SAPonALPHA) && !defined(_POSIX_C_SOURCE)
#  define _POSIX_C_SOURCE 199506L
#endif

/**********************************************************************/
/* Set Compile/Link flag for multi-thread                             */
/*                                                                    */
/* At this time only for Windows NT or 95                             */
/*                                                                    */
/**********************************************************************/
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
/* Set up includes                                                    */
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

/*--------------------------------------------------------------------*/
/* Macro for output RFC-Call on screen                                */
/*--------------------------------------------------------------------*/
#ifdef RFCwithTHREADS
#define PRINT_RFC_CALL(thread_id, text)                               \
	if (thread_id)                                                \
	  printfU (cU("\n Thread %3d:  %s"), thread_id, text);              \
        else                                                          \
	  printfU (cU("\n Main Thread  %s"), text);                         \
	fflush(stdout)
#else
#define PRINT_RFC_CALL(thread_id, text)                               \
	printfU (cU("\n              %s"), text);                           \
	fflush(stdout)
#endif /* RFCwithTHREADS */

/*--------------------------------------------------------------------*/
/* Definitions for working with multi-threads on Windows NT or 95     */
/* Definitions for working with multi-threads on UNIX                 */
/*   - Assume Posix threads.                                          */
/*   - Compile will fail if pthread.h is missing                      */
/*--------------------------------------------------------------------*/
#define RFC_MIN_STACK          (300 * 1024)
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
#    if !defined(EBUSY)
    #include <errno.h>
#  endif
#  include <unistd.h>

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
typedef struct _rfct_th_adm 
{
    rfc_char_t		cRunning;

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


void THREAD_INIT (void)
{
    /* should use InterlockedIncrement....
    */
    if (0 == thr_attr_init)
    {
        int i;
    
        memsetR (rfct_adm, 0, sizeofR(rfct_adm));

        for (i = 0; i <= RFCT_MAX_THREADS ; i++)
        {
            rfct_adm[i].cRunning =  RFCT_TERMINATED;
        }

        thr_attr_init = 1;
    }
}


void THREAD_FINALIZE (void)
{
    return;
}



int rfct_thread_create (int                     th_nb,
		                LPTHREAD_START_ROUTINE  start_ptr,
		                LPVOID                  arg)
{
    int     rc;
    HANDLE  hThr;


    THREAD_INIT ();

    /* ALPHA/OSF kill does not return an error for exited thread
    * thus we maintain a running flag
    * this flag is set before starting
    * and cleared if strating failed
    */
    if (th_nb < 0 || th_nb > RFCT_MAX_THREADS)
     return EINVAL;


    rfct_adm[th_nb].cRunning = RFCT_RUNNING;
    

#ifdef _MSC_VER
    rfct_adm[th_nb].thId = (HANDLE) _beginthreadex (NULL,
                                                    0,
                                                    (unsigned int (__stdcall *)(void *))start_ptr,
                                                    arg,
                                                    0,
                                                    (unsigned int *) &hThr);
#else	/* !MSVC */
    rfct_adm[th_nb].thId = CreateThread (NULL,
                                         0,
                                         (DWORD (*)(void *)) start_ptr,
                                         arg,
                                         0,
                                         (LPDWORD) &hThr);
#endif	/* MSVC */


    if (0 == hThr)
    {
        rfct_adm[th_nb].thId     = THREAD_INVALID_ID; 
    
        rfct_adm[th_nb].cRunning = RFCT_TERMINATED;
    
        rc = GetLastError ();
    }
    else
    {
        rc = 0;
    }

    
    return rc;
}



#define THREAD_CREATE(th_nb, start_ptr, arg)              \
        rfct_thread_create ((th_nb),                       \
                            (LPTHREAD_START_ROUTINE)(start_ptr),      \
                            (LPVOID)(arg))



int THREAD_EXIT_CODE (int th_nb, int * exit_code_ptr)
{
    if (th_nb >= 0 && 
        th_nb <= RFCT_MAX_THREADS &&  
        rfct_adm[th_nb].thId  != THREAD_INVALID_ID) 
    {
        if (GetExitCodeThread(rfct_adm[th_nb].thId, (DWORD *)exit_code_ptr))
        {
            if (*exit_code_ptr == STILL_ACTIVE)
            {
                return EBUSY;
            }
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


int THREAD_EQUAL (THREAD_ID_TYPE lVal, THREAD_ID_TYPE rVal)
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
    /* should use pthread_once....
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
		    printfU (cU("cannot get threads stacksize\n"));
		
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

		    printfU (cU("adjusting stack size from %d to %d\n"), stack_size, min_size);

		    rc = pthread_attr_setstacksize(&thr_attr, min_size);
		    if (0 != rc)
		    {
		        printfU (cU("cannot set threads stacksize\n"));
		        
                exit(1);
		    }
	    }
	}
	/* wrapper stuff----------------------------------------------
	*/
      memsetR (rfct_adm, 0, sizeofR (rfct_adm));

      for (i = 0; i <= RFCT_MAX_THREADS ; i++)
            rfct_adm[i].cRunning =  RFCT_TERMINATED;

	thr_attr_init = 1;
    }
}


void THREAD_FINALIZE(void)
{
    if (0 == thr_attr_init)
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
       &&  !THREAD_EQUAL(rfct_adm[th_nb].thId,  THREAD_INVALID_ID) )
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

/*--------------------------------------------------------------------*/
/* Function prototypes                                                */
/*--------------------------------------------------------------------*/
static void help (void);

int RFC_RegisterThread (void * lpParms);

int fill_table (ITAB_H itab_h, long leng);

int check_table (ITAB_H         itab_h, 
                 rfc_char_t   * itabname,
		         long           recvleng,
                 rfc_char_t   * checktab,
                 rfc_char_t   * text,
                 rfc_char_t   * retcode);


int SNC_check (RFC_HANDLE rfc_handle);


void function_abort (RFC_HANDLE rfc_handle, rfc_char_t * atext);

static RFC_RC install (int thread_id);

void DLL_CALL_BACK_FUNCTION rfclib_error (rfc_char_t * operation );

static RFC_RC DLL_CALL_BACK_FUNCTION performance_test (RFC_HANDLE rfc_handle);

static rfc_char_t * performance_test_docu (void);

static RFC_RC DLL_CALL_BACK_FUNCTION user_global_server (RFC_HANDLE rfc_handle);
static rfc_char_t * user_global_server_docu (void);


static RFC_RC DLL_CALL_BACK_FUNCTION structure_test(RFC_HANDLE rfc_handle);
static rfc_char_t * structure_test_docu(void);

void rfc_error (rfc_char_t * operation, int thread_id);

void rfc_param (RFC_PARAMETER * rfc_param,
	            rfc_char_t    * abap_field_name,
	            unsigned        data_type,
	            void          * data_addr,
	            unsigned        data_len );


/*--------------------------------------------------------------------*/
/* Globals                                                            */
/*--------------------------------------------------------------------*/
int             register_mode;
rfc_char_t    * new_argv[9];
int             rfc_thread[10000];

rfc_char_t          name_performance_test[31]   = iU("STFC_PERFORMANCE");
rfc_char_t          name_user_global_server[31] = iU("%%USER_GLOBAL_SERVER");
rfc_char_t          name_structure_test[31]     = iU("STFC_STRUCTURE");

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

mainU (int argc, rfc_char_t ** argv)
{
    int            i;
    RFC_RC         rfc_rc;

    
    setbuf (stdout, NULL);

    
    if (argc == 1)
    {
        help();
    
        return 0;
    }

    /*------------------------------------------------------------------*/
    /* Output argument list                                             */
    /*------------------------------------------------------------------*/

    printfU (cU("\nargc    = %d"), argc);
    
    
    for (i=0; i<argc; i++)
     printfU (cU("\nargv[%d] = '%s'"), i, argv[i]);


    /*------------------------------------------------------------------*/
    /* Check working in register mode                                   */
    /*------------------------------------------------------------------*/
    register_mode = 0;
    
    for (i=0; i<argc; i++)
    {
        new_argv[i] = argv[i];
    
        if ((memcmpU (argv[i], cU("-D"), 2) == 0) || (memcmpU (argv[i], cU("-a"), 2) == 0))
         register_mode = 1;
    }


    printfU (cU("\n\n"));
    printfU (cU("               **********************************************\n"));
    printfU (cU("               *                                            *\n"));
    printfU (cU("               *              SAP RFC-API TEST              *\n"));
    printfU (cU("               *                                            *\n"));   
    printfU (cU("               *             Single threaded on             *\n"));
    printfU (cU("               *           all supported platforms          *\n"));
    printfU (cU("               *                                            *\n"));
    printfU (cU("               *                     or                     *\n"));
    printfU (cU("               *                                            *\n"));
    printfU (cU("               *             Multi threaded on              *\n"));
    printfU (cU("               *               Windows NT/95                *\n"));
    printfU (cU("               *                     &                      *\n"));
    printfU (cU("               *                    UNIX                    *\n"));
    printfU (cU("               *                                            *\n"));   
    printfU (cU("               **********************************************\n\n\n"));

    fflush(stdout);


    /*------------------------------------------------------------------*/
    /* Install offering functions                                       */
    /*------------------------------------------------------------------*/
    rfc_rc = install (0);
    
    if( rfc_rc != RFC_OK )
     return 1;


    /*
     * In Threaded programs, RfcInit Must be called first!!!!!!
     */
    RfcInit ();


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
        printfU (cU("RfcInstallstucture error %d\n"), rfc_rc);
        
        exit(1) ;
    }


#ifdef RFCwithTHREADS
    {
        int i,
            thread_rc,
            thread_max = 0,
            active = 0,
            success = 0,
            failure = 0,
            dwExitCode = 0;


        THREAD_INIT();


        for (i=0; i<argc; i++)
        {
            if (memcmpU (argv[i], cU("-T"), 2) == 0)
            {
                int znr = strlenU (argv[i]);
        
                if (znr == 2)
                {
                    i = i + 1;
        
                    if (i < argc)
                     thread_max = atoiU (argv[i]);
                }
                else
                {
                    thread_max = atoiU (argv[i]+2);
                }
            }
        }


        if (thread_max == 0)
         thread_max = 1;


        for (i=1; i<=thread_max; i++)
        {
            THREAD_CREATE (i, (void*(*)(void*)) RFC_RegisterThread, (void *) i);
        }

    
        for(;;)
        {
            active = 0;
            
            for (i=1; i<=thread_max; i++)
            {
                thread_rc = THREAD_EXIT_CODE(i, &dwExitCode);
        
                if (thread_rc == EBUSY)
                {
                    active = 1;
                }
                else if (thread_rc == EBADF)
                {
                 continue;
                }
                else if (thread_rc == 0)
                {
                    if (dwExitCode == 0)
                    {
                        success++;
                    }
                    else
                    {
                        if (dwExitCode == 99)
                        {
                            printfU (cU("\n\n Main Thread:  RFC-ERROR OCCURS  ==>  Thread %d exited\n"), i);
                        }
                        else
                        {
                            printfU (cU("\n\n Main Thread:  Thread %d exited with rc=%d\n"), i, dwExitCode);
                            
                            failure++;
                        }
                    }
                }
                else
                {
                    printfU (cU("\n\n Main Thread:  Thread %d returned %d\n"), i, thread_rc);
        
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
         printfU (cU("\n\n Main Thread:  %d thread(s) successfully terminated.\n"), success);

        if (failure)
         printfU (cU("\n\n Main Thread:  %d thread(s) terminated with an error.\n"), failure);
    }


    THREAD_FINALIZE();
#else 
    RFC_RegisterThread(0);
#endif /* RFCwithTHREADS */

    return 0;
}



/*====================================================================*/
/*                                                                    */
/* Thread for working as RFC server in register mode                  */
/*                                                                    */
/* Following function is available:  STFC_PERFORMANCE                 */
/*                                                                    */
/*====================================================================*/
int RFC_RegisterThread (void * lpParms)
{
    RFC_HANDLE          rfc_handle;
    RFC_RC              rfc_rc;
    rfc_char_t          dbuf[512];
    RFC_ERROR_INFO_EX   error_info;

    
    for (;;)
    {
        /*------------------------------------------------------------------
         * Accept RFC-Connection                                            
         *------------------------------------------------------------------*/
        rfc_handle = RfcAccept (new_argv);

        sprintfU (dbuf, cU("RfcAccept          handle = %u"), rfc_handle);
    
        PRINT_RFC_CALL((int) lpParms, dbuf);

        if (rfc_handle == RFC_HANDLE_NULL)
        {
            rfc_error (cU("RfcAccept"), (int) lpParms);
    
            THREAD_LEAVE( (int) lpParms);
    
            return 99;
        }

        rfc_thread[rfc_handle] = (int) lpParms;

        /*------------------------------------------------------------------
         * Wait for RFC call                                                
         *------------------------------------------------------------------*/
        do
        {
          #if defined(SAP_RFC_WAIT) || defined(SAP_RFC_LISTEN)
            for (rfc_rc = RFC_RETRY; rfc_rc == RFC_RETRY;)
            {
              #ifdef SAP_RFC_WAIT
                RFC_INT wtime = 60;     /* 60 sec. */
    
                sprintfU (dbuf, cU("Wait with RfcWaitForRequest (60 sec)............."));
    
                PRINT_RFC_CALL((int) lpParms, dbuf);
    
                rfc_rc = RfcWaitForRequest (rfc_handle, wtime);
              #else
                sprintfU (dbuf, cU("Check next RFC call with RfcListen..............."));
    
                PRINT_RFC_CALL((int) lpParms, dbuf);
    
                rfc_rc = RfcListen(rfc_handle);
              #endif

    
                if (rfc_rc == RFC_RETRY)
                {
                    /* do something else while waiting for the next RFC call */
                   #if defined(SAPonNT)
                    Sleep(2000);
                   #elif defined(SAPonUNIX)
                    Sleep(2);
                   #endif
                }
            }
    
            
            if (rfc_rc != RFC_OK)
            {
                rfc_error (cU("RfcListen"), (int) lpParms);
                
                if (register_mode)
                {
                    if (rfc_rc == RFC_CLOSED)
                    {
                        RfcLastErrorEx (&error_info);
    
                        if (error_info.group == RFC_ERROR_CANCELLED)
                         break;
                    }
                    else
                     continue;
                }
                else
                {
                    THREAD_LEAVE( (int) lpParms);
    
                    return 99;
                }
            }

    
            /* Check working with SNC and if yes, SNC name and ACL Key ok? */
            if (SNC_check (rfc_handle))
            {
                if (register_mode)
                 continue;
                else
                {
                    THREAD_LEAVE( (int) lpParms);
    
                    return 99;
                }
            }
          #endif

    
            sprintfU (dbuf, cU("Wait for next RFC call with RfcDispatch.........."));
    
            PRINT_RFC_CALL((int) lpParms, dbuf);
    
            rfc_rc = RfcDispatch (rfc_handle);
        } while (rfc_rc == RFC_OK);

    
        if (register_mode)
        {
            if (rfc_rc == RFC_CLOSED)
            {
                RfcLastErrorEx (&error_info);
    
                if (error_info.group == RFC_ERROR_CANCELLED)
                 break;
            }
            else
             continue;
        }
        else
        {
            THREAD_LEAVE ((int) lpParms);
    
            return 99;
        }
    }
    
    
    THREAD_LEAVE( (int) lpParms);
    
    
    return 0;
}



/*====================================================================*/
/*                                                                    */
/* Install all RFC-functions offered in this program                  */
/*                                                                    */
/*====================================================================*/
static RFC_RC install (int thread_id)
{
    RFC_RC      rfc_rc = RFC_OK;
    rfc_char_t  dbuf[512];


    /* Install USER_GLOBAL_SERVER for check every function not supported */
    rfc_rc = RfcInstallFunction (name_user_global_server,
                                 (RFC_ONCALL) user_global_server,
                                 user_global_server_docu ());

    if (rfc_rc != RFC_OK)
     return rfc_rc;

    
    /* Install STFC_PERFORMANCE */
    rfc_rc = RfcInstallFunction (name_performance_test,
                                 (RFC_ONCALL) performance_test,
                                 performance_test_docu ());

    if (rfc_rc != RFC_OK)
     return rfc_rc;

    sprintfU (dbuf, cU("RfcInstallFunction (%s)"), name_performance_test);
    
    
    PRINT_RFC_CALL (thread_id, dbuf);

    
    rfc_rc = RfcInstallFunction (name_structure_test,
                                 (RFC_ONCALL) structure_test,
                                 structure_test_docu());
    
    if (rfc_rc != RFC_OK)
     return rfc_rc;

    sprintfU (dbuf, cU("RfcInstallFunction (%s)"), name_structure_test);
    
    
    PRINT_RFC_CALL(thread_id, dbuf);

    
    return rfc_rc;
}


/*====================================================================*/
/*                                                                    */
/* RFC-FUNCTION: %%USER_GLOBAL_SERVER                                 */
/*                                                                    */
/* The RFC library will call this function if any uninstalled         */
/* should be called. This program can then dispatch itself or         */ 
/* close the RFC connection with an Exception Message.                */
/*                                                                    */
/*====================================================================*/
static RFC_RC DLL_CALL_BACK_FUNCTION user_global_server (RFC_HANDLE rfc_handle)
{
    RFC_RC      rfc_rc;
    rfc_char_t  dbuf[512],
                abort_text[80],
                function_name[31];
    int         thread_id = rfc_thread[rfc_handle];

    
    /* Check working with SNC and if yes, SNC name and ACL Key ok? */
    if (SNC_check(rfc_handle))
     return (RFC_RC) 99;

    
    rfc_rc = RfcGetName(rfc_handle, function_name);

    if (rfc_rc != RFC_OK)
    {
        rfc_error (cU("RfcGetName"), thread_id);
    
        return (RFC_RC) 99;
    }


    sprintfU (dbuf, cU("Start Function     %s"), function_name);
    
    
    PRINT_RFC_CALL(thread_id, dbuf);

    
    sprintfU (abort_text, cU("Required RFC function not supported"));
    
    
    function_abort (rfc_handle, abort_text);

    
    return (RFC_RC)99;
}



#define BLANK                  cU(' ')
#define ITAB0332_ENTRY_SIZE    332
#define ITAB1000_ENTRY_SIZE    1000
#define ETAB0332_ENTRY_SIZE    332
#define ETAB1000_ENTRY_SIZE    1000
/*====================================================================*/
/*                                                                    */
/* RFC-FUNCTION:  STFC_PERFORMANCE                                    */
/*                                                                    */
/* Send and Receive internal tables as required                       */
/*                                                                    */
/*====================================================================*/
static RFC_RC DLL_CALL_BACK_FUNCTION performance_test(RFC_HANDLE rfc_handle)
{
    int                 i,
                        lgit0332,
                        lgit1000,
                        lget0332,
                        lget1000;
    rfc_char_t          retcode[2],
                        exitcode[2],
                        checktab[2],
                        text0332[73],
                        text1000[73],
                        lgit0332_field[6],
                        lgit1000_field[6],
                        lget0332_field[6],
                        lget1000_field[6];
    RFC_RC              rfc_rc;
    RFC_PARAMETER       parameters[6];
    RFC_TABLE           tables[5];
    RFC_ATTRIBUTES      rfc_attributes;
    rfc_char_t        * exception = NULL,
                      * function  = NULL;
    rfc_char_t          dbuf[512];
    int                 thread_id = rfc_thread[rfc_handle];


    /* Initialize IMPORT-/EXPORT-parameters */
    memsetR (lgit0332_field, 0, sizeofR (lgit0332_field));
    memsetR (lgit1000_field, 0, sizeofR (lgit1000_field));
    memsetR (lget0332_field, 0, sizeofR (lget0332_field));
    memsetR (lget1000_field, 0, sizeofR (lget1000_field));


    /* Check working with SNC and if yes, SNC name and ACL Key ok? */
    if (SNC_check (rfc_handle))
     return (RFC_RC) 99;

    
    memsetR (parameters, 0, sizeofR(parameters));


    /* parameter */
    rfc_param (&parameters[0], cU("CHECKTAB"), TYPC, checktab, 1*sizeofR(SAP_UC));
    rfc_param (&parameters[1], cU("LGIT0332"), TYPC, lgit0332_field, 5*sizeofR(SAP_UC));
    rfc_param (&parameters[2], cU("LGIT1000"), TYPC, lgit1000_field, 5*sizeofR(SAP_UC));

    rfc_param (&parameters[3], cU("LGET0332"), TYPC, lget0332_field, 5*sizeofR(SAP_UC));
    rfc_param (&parameters[4], cU("LGET1000"), TYPC, lget1000_field, 5*sizeofR(SAP_UC));

    
    /* Prepare internal table ITAB0332 */
    tables[0].name     = cU("ITAB0332");
    tables[0].nlen     = 8;
    tables[0].type     = TYPC;
    tables[0].leng     = ITAB0332_ENTRY_SIZE*sizeofR(SAP_UC);
    tables[0].itmode   = RFC_ITMODE_BYREFERENCE;

    /* Prepare internal table ITAB1000 */
    tables[1].name     = cU("ITAB1000");
    tables[1].nlen     = 8;
    tables[1].type     = TYPC;
    tables[1].leng     = ITAB1000_ENTRY_SIZE*sizeofR(SAP_UC);
    tables[1].itmode   = RFC_ITMODE_BYREFERENCE;

    /* Prepare internal table ETAB0332 */
    tables[2].name     = cU("ETAB0332");
    tables[2].nlen     = 8;
    tables[2].type     = TYPC;
    tables[2].leng     = ETAB0332_ENTRY_SIZE*sizeofR(SAP_UC);
    tables[2].itmode   = RFC_ITMODE_BYREFERENCE;

    /* Prepare internal table ETAB1000 */
    tables[3].name     = cU("ETAB1000");
    tables[3].nlen     = 8;
    tables[3].type     = TYPC;
    tables[3].leng     = ETAB1000_ENTRY_SIZE*sizeofR(SAP_UC);
    tables[3].itmode   = RFC_ITMODE_BYREFERENCE;

    /* terminate array */
    tables[4].name = NULL;


    /* receive data */
    rfc_rc = RfcGetData (rfc_handle, parameters, tables);

    if (rfc_rc != RFC_OK)
    {
        rfc_error (cU("RfcGetData"), thread_id);
    
        return (RFC_RC) 99;
    }

    
    if (RfcGetAttributes (rfc_handle, &rfc_attributes))
    {
        rfc_error (cU("RfcGetAttributes"), thread_id);
    
        return (RFC_RC) 99;
    }


    i = strlenU (rfc_attributes.user) - 1;
    
    while (rfc_attributes.user[i] == BLANK)
    {
        rfc_attributes.user[i] = 0;
    
        i = i - 1;
    }

    
    sprintfU (dbuf, cU("Executing Function handle = %u"), rfc_handle);
    
    sprintfU (dbuf+strlenU (dbuf), cU("     /%s/%s/%s/%s/%s/%s"), 
              rfc_attributes.sysid, rfc_attributes.partner_host, 
              rfc_attributes.systnr, rfc_attributes.client, 
              rfc_attributes.user, rfc_attributes.language);
    
    
    PRINT_RFC_CALL(thread_id, dbuf);


    /* Receive length of all ITABnnnn and ETABnnnn */
    lgit0332 = atolU (lgit0332_field);
    lgit1000 = atolU (lgit1000_field);
    lget0332 = atolU (lget0332_field);
    lget1000 = atolU (lget1000_field);


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
            sprintfU (dbuf, cU("ERROR: Fill table etab0332"));
    
            PRINT_RFC_CALL(thread_id, dbuf);
    
            exitcode[0] = cU('E');
        }
        else
        {
            /* Fill table ETAB1000 as required */
            if (fill_table (tables[3].ithandle, lget1000))
            {
                sprintfU (dbuf, cU("ERROR: Fill table itab1000"));
    
                PRINT_RFC_CALL(thread_id, dbuf);
    
                exitcode[0] = cU('E');
            }
        }
    }

    
    /* Refresh ITAB0332 for better performance */
    ItFree(tables[0].ithandle);

    /* Refresh ITAB1000 for better performance */
    ItFree(tables[1].ithandle);

    
    /*------------------------------------------------------------------*/
    /* Return parameters and tables                                     */
    /*------------------------------------------------------------------*/
    memsetR (&parameters[0], 0, sizeofR (parameters));


    /* parameters */
    rfc_param (&parameters[0], cU("EXITCODE"), TYPC, exitcode, 1*sizeofR(SAP_UC));
    rfc_param (&parameters[1], cU("TEXT0332"), TYPC, text0332, 72*sizeofR(SAP_UC));
    rfc_param (&parameters[2], cU("TEXT1000"), TYPC, text1000, 72*sizeofR(SAP_UC));


    /* return to the caller */
    rfc_rc = RfcSendData(rfc_handle, parameters, tables);


    if (rfc_rc != RFC_OK)
    {
        rfc_error (cU("RfcSendData"), thread_id);
    
        return (RFC_RC)99;
    }

    
    return rfc_rc;
}



/*====================================================================*/
/*                                                                    */
/* RFC-FUNCTION:  STFC_STRUCTURE                                      */
/*                                                                    */
/* Send and Receive inhomogeneous structure/table                     */
/*                                                                    */
/*====================================================================*/
static RFC_RC DLL_CALL_BACK_FUNCTION structure_test (RFC_HANDLE rfc_handle)
{
    const SAP_RAW   hex3_value[] = { 0xf1, 0xf2, 0xf3 };
    RFC_RC	        rfc_rc;
    RFC_TABLE	    rfc_tables[2] ;
    RFC_PARAMETER	rfc_in__parameters[2];
    RFC_PARAMETER	rfc_out_parameters[3];
    RFCTEST         imstruct, exstruct, 
                  * pLine;
    rfc_char_t      resptext[256];
    rfc_char_t    * exception;
    SAP_RAW       * ptr;
    rfc_char_t    * pChar;
    unsigned        lineleng;
    time_t          actutime;
    struct tm       tm_buff, 
                  * tmb_ptr;
    rfc_char_t      lct_buff[32]; /* >=26 */
    int             thread_id = rfc_thread[rfc_handle];


    memsetR (rfc_tables, 0, sizeofR (rfc_tables));

    memsetR (rfc_in__parameters, 0, sizeofR (rfc_in__parameters));

    memsetR (rfc_out_parameters, 0, sizeofR (rfc_out_parameters));


    /* import parameters */
    rfc_in__parameters[0].name = cU("IMPORTSTRUCT");
    rfc_in__parameters[0].nlen = 12;
    rfc_in__parameters[0].addr = &imstruct;
    rfc_in__parameters[0].leng = sizeofR(RFCTEST);
    rfc_in__parameters[0].type = handleOfRfcTest;

    /* tables */
    rfc_tables[0].name     = cU("RFCTABLE");
    rfc_tables[0].nlen     = 8;
    rfc_tables[0].type     = handleOfRfcTest;
    rfc_tables[0].leng     = sizeofR(RFCTEST);
    rfc_tables[0].itmode   = RFC_ITMODE_BYREFERENCE;
    rfc_tables[0].ithandle = ITAB_NULL;


    /* receive data */
    rfc_rc = RfcGetData (rfc_handle, rfc_in__parameters, rfc_tables);


    if (rfc_rc != RFC_OK)
     rfc_error (cU("RfcGetData"), thread_id);


    /* Echo structure */
    exstruct = imstruct;


    /* Info text */
    memsetU (resptext, BLANK, 255);

    memcpyU (resptext, cU("Sysid:"), 6);


    pChar = getenvU (cU("HOSTNAME"));

    if (pChar == NULL)
     pChar = getenvU (cU("USER"));


    if (pChar == NULL)
     pChar = getenvU (cU("LOGNAME"));


    if (pChar != NULL)
     strcpyU (resptext+7, pChar);
    else
     strcpyU (resptext+7, cU("????????"));


    resptext[strlenU (resptext)] = cU(' ');

    memcpyU (resptext+18, cU("Date & Time:"), 12);


    actutime = time(NULL);
  #if defined(SAPonNT) || defined(SAPonOS2)
    /*CCQ_CLIB_LOCTIME_OK*/
    tmb_ptr = localtime(&actutime);
    memcpyU (&resptext[31], asctimeU (tmb_ptr), 24);
   #else
    /*CCQ_CLIB_LOCTIME_OK*/
    tmb_ptr = localtime_r(&actutime, &tm_buff);
   #ifdef SAPonSUN
    #ifdef __STDC__
    memcpyU (&resptext[31], asctime_r(tmb_ptr, lct_buff), 24);
    #else
    memcpyR (&resptext[31], asctime_r(tmb_ptr, lct_buff, sizeofR(lct_buff)), 24);
    #endif
   #else
    memcpyU (&resptext[31], asctime_rU (tmb_ptr, lct_buff), 24);
   #endif
  #endif


    /* New structure value */
    imstruct.rfcfloat    = imstruct.rfcfloat + 1;
    imstruct.rfcchar1[0] = cU('X');
    imstruct.rfcint2     = imstruct.rfcint2 + 1;
    imstruct.rfcint1     = imstruct.rfcint1 + 1;


    memcpyU (imstruct.rfcchar4, cU("EXT."), 4);
    memcpyR (imstruct.rfchex3,  hex3_value, 3);
    memcpyU (imstruct.rfcchar2, cU("YZ"),       2);
    memcpyU (imstruct.rfctime,  cU("124500"),   6);
    memcpyU (imstruct.rfcdate,  cU("19960924"), 8);


    /* Append into table 'RFCTABLE' */
    lineleng = ItLeng (rfc_tables[0].ithandle);

    
    ptr = (SAP_RAW*) ItAppLine (rfc_tables[0].ithandle);
    
    if (ptr == NULL)
     rfc_error (cU("ItAppLine"), thread_id);


    memcpyR (ptr, &imstruct, lineleng);


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


    rfc_rc = RfcSendData(rfc_handle,
    rfc_out_parameters,
    rfc_tables);

    if (rfc_rc != RFC_OK)
     rfc_error (cU("RfcSendData"), thread_id);


    return rfc_rc;
}


#undef  NL
#define NL iU("\n")
#undef  NL_PTR
#define NL_PTR cU("\n")
/*====================================================================*/
/*                                                                    */
/* Documentation for function %%USER_GLOBAL_SERVER                    */
/*                                                                    */
/*====================================================================*/
static rfc_char_t * user_global_server_docu (void)
{
    static rfc_char_t docu[] =
        iU("The RFC library will call this function if any unknown")            NL
        iU("RFC function should be executed in this RFC server program.")       NL
        ;
    
    return docu;
}


/*====================================================================*/
/*                                                                    */
/* Documentation for functions STFC_PERFORMANCE                       */
/*                                                                    */
/*====================================================================*/
static rfc_char_t * performance_test_docu (void)
{
    static rfc_char_t docu[] =
        iU("RFC-Client can send data in max. 2 different tables and then ")     NL
        iU("wait for response (data in max. 2 different tables).         ")     NL
        iU("")                                                                  NL
        iU("RFC-Client must define:                                      ")     NL
        iU("     - the number of lines in 4 tables as importing parameter")     NL
        iU("     - the parameter checktab (Y/N) for checking the contents")     NL
        iU("       of the 2 importing tables (ITAB0332 and ITAB1000)     ")     NL
        iU("")                                                                  NL
        iU("IMPORTING")                                                         NL
        iU("  CHECKTAB        C(1)")                                            NL
        iU("  LGIT0332        C(5)")                                            NL
        iU("  LGIT1000        C(5)")                                            NL
        iU("  LGET0332        C(5)")                                            NL
        iU("  LGET1000        C(5)")                                            NL
        iU("EXPORTING")                                                         NL
        iU("  EXITCODE        C(1)")                                            NL
        iU("  TEXT0332        C(72)")                                           NL
        iU("  TEXT1000        C(72)")                                           NL
        iU("TABLES")                                                            NL
        iU("  ITAB0332        C(0332)")                                         NL
        iU("  ITAB1000        C(1000)")                                         NL
        iU("  ETAB0332        C(0332)")                                         NL
        iU("  ETAB1000        C(1000)")                                         NL
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
        iU("Send/Receive data in homogeneous structure/table             ")     NL
        iU("  NOT available with 16-bit RFC library on Windows           ")     NL
        iU("")                                                                  NL
        iU("IMPORTING")                                                         NL
        iU("  IMPORTSTRUCT    Structure RFCTEST(48)")                           NL
        iU("EXPORTING")                                                         NL
        iU("  RESPTEXT        C(255)               ")                           NL
        iU("  ECHOSTRUCT      Structure RFCTEST(48)")                           NL
        iU("TABLES")                                                            NL
        iU("  RFCTABLE        Structure RFCTEST(48)")                           NL
        ;
    
    
    return docu;
}





/*--------------------------------------------------------------------*/
/* set up RFC parameters                                              */
/*--------------------------------------------------------------------*/
void rfc_param (RFC_PARAMETER * rfc_param,
                rfc_char_t    * abap_field_name,
                unsigned        data_type,
                void          * data_addr,
                unsigned        data_len)
{
    RFC_PARAMETER * p = rfc_param;

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
/* This function is defined as an error handler for the RFC library   */
/*--------------------------------------------------------------------*/
void DLL_CALL_BACK_FUNCTION rfclib_error (rfc_char_t * operation)
{
    rfc_char_t          ebuf[1024];
    RFC_ERROR_INFO_EX   error_info;


    memsetU (ebuf, 0, sizeofU(ebuf));
    
    sprintfU (ebuf, cU("\n RfcLastErrorEx\n\n"));

    RfcLastErrorEx (&error_info);

    sprintfU (ebuf+strlenU (ebuf), cU("\nRFC Call/Exception: %s\n"), operation);
    sprintfU (ebuf+strlenU (ebuf), cU("Group       Error group %d\n"), error_info.group);
    sprintfU (ebuf+strlenU (ebuf), cU("Key         %s\n"), error_info.key);
    sprintfU (ebuf+strlenU (ebuf), cU("Message     %s\n"), error_info.message);

    printfU (cU("%s"), ebuf);
    
    
    RfcClose (RFC_HANDLE_NULL);
    
    
    exit(1);
}




/*--------------------------------------------------------------------*/
/* Error Cleanup because of a RFC-Error                               */
/*--------------------------------------------------------------------*/
void rfc_error (rfc_char_t * operation, int thread_id)
{
    rfc_char_t          ebuf[1024];
    RFC_ERROR_INFO_EX   error_info;


    memsetU (ebuf, 0, sizeofU (ebuf));
    
    sprintfU (ebuf, cU("\n RfcLastErrorEx\n\n"));


    RfcLastErrorEx (&error_info);

    sprintfU (ebuf+strlenU (ebuf), cU("\nRFC call/exception: %s\n"), operation);
    sprintfU (ebuf+strlenU (ebuf), cU("Group       Error group %d\n"), error_info.group);
    sprintfU (ebuf+strlenU (ebuf), cU("Key         %s\n"), error_info.key);
    sprintfU (ebuf+strlenU (ebuf), cU("Message     %s\n"), error_info.message);
    
    PRINT_RFC_CALL (thread_id, ebuf);
    
    
    RfcClose (RFC_HANDLE_NULL);
    
    
    return;
}


/*--------------------------------------------------------------------*/
/* Issue RfcAbort with Abort Text because of an Application Error     */
/*--------------------------------------------------------------------*/
void function_abort (RFC_HANDLE rfc_handle, rfc_char_t * atext)
{
    int         thread_id = rfc_thread[rfc_handle];
    rfc_char_t  dbuf[512];


    sprintfU (dbuf, cU("RfcAbort         %s\n"), atext);
    
    PRINT_RFC_CALL(thread_id, dbuf);

    RfcAbort (rfc_handle, atext);
    
    return;
}


/*--------------------------------------------------------------------*/
/* Fill internal table as required                                    */
/*--------------------------------------------------------------------*/
int fill_table (ITAB_H itab_h, long leng)
{
    unsigned        ulen;
    long            position,
                    lineleng,
                    linenr;
    rfc_char_t    * ptr,
                    etext[1001];


    if (leng == 0)
     return 0;

    
    lineleng = ulen = ItLeng (itab_h)/sizeofR(SAP_UC);

    memsetU (etext, BLANK, 1000);


    for (position=40; position<lineleng; position++)
     sprintfU (etext+position, cU("%01d"), position % 10);


    sprintfU (etext, cU("Line Length: %04d   Line No.: "), lineleng);

   
    for (linenr=1; linenr<=leng; linenr++)
    {
        sprintfU (etext+30, cU("%05d"), linenr);
    
        etext[35] = cU(' ');

        ptr = (rfc_char_t *) ItAppLine(itab_h);
        
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
int check_table (ITAB_H         itab_h,
                 rfc_char_t   * itabname,
                 long           recvleng,
                 rfc_char_t   * checktab,
                 rfc_char_t   * rtext,
                 rfc_char_t   * retcode)
{
    int             j;
    unsigned        ulen;
    long            position,
                    lineleng,
                    itableng;
    rfc_char_t    * ptr;
    rfc_char_t      value[2],
                    tbuff[1001],
                    etext[73]   = iU(" "),
                    errline[40] = iU("ERROR:  #Lines = nnnnn  Expected: nnnnn"),
                    errcont[40] = iU("ERROR at line nnnnn position nnnn:  n/n"),
                    mtext[73]   = iU("Import table (length nnnn):  #Lines = nnnnn: OK,  Contents: OK");

    
    memsetU (etext, cU(' '), 72);

    
    itableng = ItFill(itab_h);
    
    lineleng = ulen = ItLeng (itab_h)/sizeofR(SAP_UC);

    
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
        {
            memcpyU (etext+60, cU("NO CHECK"), 8);
        }
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


#define SNC_NAME_LN            256
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
int SNC_check (RFC_HANDLE rfc_handle)
{
    int                 i,
                        aclkey_len;
    rfc_char_t          partner_name[SNC_NAME_LN+1];
    RFC_RC              rfc_rc;
    RFC_BYTE            partner_aclkey[SNC_NAME_LN+1];
    RFC_SNC_MODE        snc_mode;
    rfc_char_t          abort_text[80];
    rfc_char_t          dbuf[512];
    int                 thread_id = rfc_thread[rfc_handle];


    rfc_rc = RfcSncMode (rfc_handle, &snc_mode);


    if (rfc_rc != RFC_OK)
    {
        rfc_error (cU("RfcSncMode"), thread_id);
    
        return 99;
    }


    if (snc_mode == RFC_SNC_MODE_OFF)
     return 0;

    
    rfc_rc = RfcSncPartnerName (rfc_handle, partner_name, SNC_NAME_LN);

    if (rfc_rc != RFC_OK)
    {
        rfc_error (cU("RfcSncPartnerName"), thread_id);
    
        return 99;
    }


    sprintfU (dbuf, cU("SNC-PartnerName  %s"), partner_name);
    

    PRINT_RFC_CALL (thread_id, dbuf);


    memsetR (partner_aclkey, cR(' '), SNC_NAME_LN);
    
    
    rfc_rc = RfcSncPartnerAclKey (rfc_handle, partner_aclkey, SNC_NAME_LN, &aclkey_len);

    if (rfc_rc != RFC_OK)
    {
        rfc_error (cU("RfcSncPartnerAclKey"), thread_id);
    
        return 99;
    }


    sprintfU (dbuf, cU("PartnerAclKey    "));

    for (i=0; i<aclkey_len; i++)
     sprintfU (dbuf+strlenU (dbuf), cU("%02X"), (int) partner_aclkey[i]);


    PRINT_RFC_CALL(thread_id, dbuf);


    /* Sample coding for check and refuse */
    if (strcmpU (partner_name, cU("not_authorized_snc_name")) == 0)
    {
        sprintfU (abort_text, cU("SNC name '%s' is not authorized for calling this program"), partner_name);
    
        function_abort(rfc_handle, abort_text);
    
        return 99;
    }
    
    
    return 0;
}


/*--------------------------------------------------------------------*/
/* Output help for starting program                                   */
/*--------------------------------------------------------------------*/
static void help (void)
{
    printfU ( NL_PTR                                                             );
    printfU (cU("Syntax for start and run in register mode:")                NL_PTR );
    printfU (cU(" ")                                                         NL_PTR );
    printfU (cU("  rfcthsrv [options]")                                      NL_PTR );
    printfU (cU(" ")                                                         NL_PTR );
    printfU (cU("  with")                                                    NL_PTR );
    printfU (cU("  options = -D<destination with type 'R' in saprfc.ini>")   NL_PTR );
    printfU (cU("          = -t             RFC-Trace on")                   NL_PTR );
    printfU (cU("  or")                                                      NL_PTR );
    printfU (cU("  options = -a<program ID> e.g.  <own host name>.rfcthsrv") NL_PTR );
    printfU (cU("          = -g<SAP gateway host name>        e.g.  hs0311") NL_PTR );
    printfU (cU("          = -x<SAP gateway service>          e.g. sapgw53") NL_PTR );
    printfU (cU("          = -t             RFC-Trace on")                   NL_PTR );
    printfU (cU("          = -L<SNC library> e.g./krb5/hpux/lib/libkrb5.sl") NL_PTR );
    printfU (cU("          = -S<SNC own name>         e.g. s:sample@hw1145") NL_PTR );
    printfU (cU("          = -T<#threads>   Default: 1, Max. 999")           NL_PTR );
    printfU (cU(" ")                                                         NL_PTR );
    printfU (cU(" Option L and S are only necessary if working with SNC")    NL_PTR );
    printfU (cU(" (Secure Network Communication).")                          NL_PTR );
    printfU (cU(" ")                                                         NL_PTR );


    return;
}

