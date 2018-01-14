static char sccsid[] = "@(#) $Id: //bas/640_REL/src/krn/rfc/rfcping.c#3 $ SAP";
/*---------------------------------------------------------------------
 * RFCPING -- simple utility to test RFC links
 *
 * ID's FOR RFCPING:
 *   cnt_ping=<nb> optional: number of echo request to send
 *            defaulf value 4, value -1 == for ever
 *   cnt_kb=<nb> optional: number of KB to send
 *           default value 0, this additional data is send oneway
 *           to the server, only the aknowledge is sent back.
 *
 */
/*--------------------------------------------------------------------*
 *	DEPENDENCIES
 *--------------------------------------------------------------------*/
 /*STDC*/
#ifndef SAPonLIN
#ifndef SAPonRM600
#  ifndef _XOPEN_SOURCE_EXTENDED
#    define _XOPEN_SOURCE_EXTENDED 1
#  endif
#  ifndef _HPUX_SOURCE
#    define _HPUX_SOURCE   /* HP  gettimeofday(), timeval, timezone */
#  endif
#  ifndef _ALL_SOURCE
#    define _ALL_SOURCE    /* AIX gettimeofday(), timeval, timezone */
#  endif
#  ifndef __EXTENSIONS__ 
#    define __EXTENSIONS__ /* SUN gettimeofday(), timeval, timezone */
#  endif
#  ifndef _OSF_SOURCE
#    define _OSF_SOURCE    /* OSF gettimeofday(), timeval, timezone */
#  endif
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>


#ifdef SAPonUNIX
#  include <unistd.h>
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


/*STDRFC*/
#include "saprfc.h"
#include "sapitab.h"
#include "rfcsi.h"

#ifdef SAPonOS400
#include "o4global.h"
#endif

#ifdef SAPonNT
#include <windows.h>
#endif

#if defined(OS2)
 /* map os/2 DosSleep to NT Sleep call */
 extern unsigned short _System Sleep( unsigned long msec );
 #pragma import(Sleep,,"DOSCALLS",229)
#endif

/* stdlib, macro, ???*/
#ifndef min
#define min(a, b) ( ((a) < (b)) ? (a) : (b) )
#endif
#ifndef max
#define max(a, b) ( ((a) > (b)) ? (a) : (b) )
#endif


/*--------------------------------------------------------------------*
 *	DEFINES
 *--------------------------------------------------------------------*/
#define SAPINFO_OK              0 /* OK */
#define SAPINFO_ERROR_PARAMETER 1 /* Wrong argc/argv */
#define SAPINFO_ERROR_CONNECT   2 /* Connect failure (system down wrong parameters)*/
#define SAPINFO_ERROR_TIMEOUT   3 /* Timeout from call/receive */
#define SAPINFO_ERROR_FAILURE   4 /* Others (codepage, memory, logon,...)*/
#define SAPINFO_ERROR_RECONNECT 5 /* ReConnect failure (system down wrong parameters)*/


/*--------------------------------------------------------------------*
 *	LOCAL
 *--------------------------------------------------------------------*/
void DLL_CALL_BACK_FUNCTION rfc_error( rfc_char_t * operation );

static void display_rfcsi( RFCSI * rfcsi );
static void help( void );
static RFC_RC rfcping(RFC_HANDLE hRfc, 
                      rfc_char_t *szDest, int ccDest,
                      unsigned kb, int iTimeoutPing);
static void fill_pay_load(ITAB_H hItab, unsigned kb) ;
void dump_stats(void);
void myCtrlC(int sig);

/*helper define (saptypec not part from sdk)*/
typedef SAP_CHAR RFCSI_CP[4] ;
#define mycpeq(l,r)   (    ((l)[0] == (r)[0]) \
						&& ((l)[1] == (r)[1]) \
						&& ((l)[2] == (r)[2]) \
						&& ((l)[3] == (r)[3]) )

/*timemsec wraper:
 *MYTIMEMS abstract platform independant type form timespans
 */
typedef 
#if defined(SAPonUNIX)
                        struct timeval
#elif defined(SAPonOS400)
                        time_t
#else
                        struct timeb
#endif
                        MYTIMEMS;
static void         setTimeMs(MYTIMEMS *pToInit);
static unsigned int diffTimeMs(MYTIMEMS *pTo, MYTIMEMS *pFrom);
static int          getResolution(unsigned int *pRes, SAP_CHAR** ppUnit);

/*sleep wraper*/
static void mySleep(int sec );

/*--------------------------------------------------------------------*
 *	GLOBALS
 *--------------------------------------------------------------------*/
RFC_ERROR_INFO_EX  error_info;
unsigned g_uMin =  ~0u, g_uMax = 0, g_uTotal = 0;
unsigned g_calls ;
unsigned g_abend = 0;

static unsigned int g_minRes = 1;
static SAP_CHAR    *g_pRes   = cU("s");

/*--------------------------------------------------------------------*
 *	IMPLEMENTATION
 *--------------------------------------------------------------------*/
int mainU (int argc, rfc_char_t ** argv)
{
    RFC_ENV             new_env;
    RFC_HANDLE          handle = RFC_HANDLE_NULL;
    RFC_RC              rfc_rc = RFC_OK;
    RFC_PARAMETER       exporting[32];
    RFC_PARAMETER       importing[32];
    RFC_TABLE           tables[32];
    RFCSI               rfcsi;
    RFC_ATTRIBUTES      rfc_attribs;
    int                 fMustReconnect = 0;
    rfc_char_t          exception[64];
    rfc_char_t        * exception_ptr = exception;
    rfc_char_t          VersionBuffer[256];
    rfc_char_t        * ptr = NULL,
                      * pa  = NULL,
                      * pb  = NULL;
    rfc_char_t          pm_buf[257];
    rfc_char_t          connect_param[1024];

    unsigned            i = 0, 
                        j = 0, 
                        len = 0,
                        client_found   = 0,
                        user_found     = 0,
                        passwd_found   = 0,
                        language_found = 0;
    rfc_char_t          std_client[4] = iU("000"),
                        std_user[13]  = iU("SAPCPIC"),
                        std_pass[9]   = iU("ADMIN"),
                        std_lang[3]   = iU("EN");
    time_t              start_t, now_t; 
    int                 cntPing = 4, pcnt, iIntervalPing = 0, iTimeoutPing = 30;
    unsigned            uKbPing = 0; 
    

    if (argc < 2)
    {
        help();
    
        return SAPINFO_ERROR_PARAMETER;
    }


    /* -----------------------------------------------
     * Macintosh init
     * ---------------------------------------------*/
#ifdef SAPonApple
    MacInit(&argc, &argv);
#endif


    /* -----------------------------------------------
     * Check Get Library Verion
     * ---------------------------------------------*/
    if (strcmpU (argv[1], cU("-v")) == 0)
    {
        RfcGetAllLibVersions (VersionBuffer, sizeofU (VersionBuffer));
    
        printfU (VersionBuffer);
    
        return SAPINFO_OK;
    }


    /* -----------------------------------------------
     * Check input for old version of rfcping
     * ---------------------------------------------*/
    if (argv[1][0] == cU('-'))
    {
        help();
    
        return SAPINFO_ERROR_PARAMETER;
    }


    /* -----------------------------------------------
     * Install error handler
     * ---------------------------------------------*/
    new_env.allocate = NULL;
    new_env.errorhandler = rfc_error;
    RfcEnvironment (&new_env);


    /* -----------------------------------------------
     * Put parameters from command to RfcOpenEx-Param.
     * ---------------------------------------------*/
    memsetU (connect_param, cU('\0'), sizeofU(connect_param));

    ptr = connect_param;
    
    for (i=1; i<(unsigned)argc; i++)
    {
        len = strlenU (argv[i]);

        /* Check syntax of connection parameters */
        pa = memchrU (argv[i], cU('='), len);
    
        if (pa == NULL)
        {
            help();
    
            return SAPINFO_ERROR_PARAMETER;
        }

        /* Values which include blanks must be set in "..." */
        pb = memchrU (argv[i], cU(' '), len);
    
        if (pb == NULL)
        {
            strcpyU (pm_buf, argv[i]);
        }
        else
        {
            memsetU (pm_buf, cU('\0'), sizeofU(pm_buf));
    
            pa = pa + 1;
    
            len = pa - argv[i];
    
            memcpyU (pm_buf, argv[i], len);
    
            pm_buf[len] = cU('"');
    
            strcpyU (pm_buf+len+1, pa);
    
            pm_buf[strlenU (pm_buf)] = cU('"');
        }


        /* Copy into connect_param of RfcOpenEx */
        strcpyU (ptr, pm_buf);
    
        ptr = ptr + strlenU (ptr);
    
        *ptr++ = cU(' ');


        /* Special handling for SAP logon with default data */
        for (j = 0; j < strlenU(argv[i]); j++)
        {
            if (argv[i][j] == cU('='))
             break;

            argv[i][j] = toupperU (argv[i][j]);
        }


        if (memcmpU (argv[i], cU("CLIENT="), 7) == 0)
        {
            client_found = 1;
        }
        else if (memcmpU (argv[i], cU("USER="), 5) == 0)
        {
            user_found = 1;
        }
        else if (memcmpU (argv[i], cU("PASSWD="), 7) == 0)
        {
            passwd_found = 1;
        }
        else if (memcmpU (argv[i], cU("LANG="), 5) == 0)
        {
            language_found = 1;
        }
        else if (memcmpU (argv[i], cU("PING_CNT="), 9) == 0)
        {
            cntPing = atoiU((argv[i]) + 9);
        }
        else if (memcmpU (argv[i], cU("PING_KB="), 8) == 0)
        {
            uKbPing = atoiU((argv[i]) + 8);
        }
        else if (memcmpU (argv[i], cU("PING_INTERVAL="), 14) == 0)
        {
            iIntervalPing = atoiU((argv[i]) + 14);
        }
        else if (memcmpU (argv[i], cU("PING_TIMEOUT="), 13) == 0)
        {
            iTimeoutPing = atoiU((argv[i]) + 13);
        }
    }

    
    /* No logon check at OPEN time */
    strcpyU (connect_param+strlenU (connect_param), cU("LCHECK=0"));

    
    /* Use default logon data if missing */
    if (client_found == 0)
    {
        sprintfU (connect_param+strlenU (connect_param), cU(" CLIENT=%s"), std_client);
    }
    
    
    if (user_found == 0)
    {
        sprintfU (connect_param+strlenU (connect_param), cU(" USER=%s"), std_user);
    }
    
    
    if (passwd_found == 0)
    {
        sprintfU (connect_param+strlenU (connect_param), cU(" PASSWD=%s"), std_pass);
    }
    
    
    if (language_found == 0)
    {
        sprintfU (connect_param+strlenU (connect_param), cU(" LANG=%s"), std_lang);
    }


    /* -----------------------------------------------
     * Open connection
     * ---------------------------------------------*/
    handle = RfcOpenEx (connect_param, &error_info);

    if (handle == RFC_HANDLE_NULL)
    {
        printfU (cU("\nGroup       Error group %d"), error_info.group);
        printfU (cU("\nKey         %s"), error_info.key);
        printfU (cU("\nMessage     %s\n"), error_info.message);
    
        exit(SAPINFO_ERROR_CONNECT);
    }


    if (0 == RfcGetAttributes(handle, &rfc_attribs))
    {
        if (RFC_SERVER_EXT_REG == rfc_attribs.partner_type)
        {
            /* partner is an external registered server
             * we must reconnect between calls
             */
            fMustReconnect = 1;
        }
    }


    /* -----------------------------------------------
     * Parameters
     * ---------------------------------------------*/
    importing[0].name = cU("RFCSI_EXPORT");
    importing[0].nlen = 12;
    importing[0].addr = &rfcsi;
    importing[0].leng = sizeofR (rfcsi);
    importing[0].type = RFCTYPE_CHAR;

    importing[1].name = NULL;
    exporting[0].name = NULL;
    tables[0].name    = NULL;

    
    /* -----------------------------------------------
     * Call RFC function
     * ---------------------------------------------*/
    rfc_rc = RfcCall (handle, 
                      cU("RFC_SYSTEM_INFO"),
                      exporting,
                      tables);
    
    if (rfc_rc != RFC_OK)
    {
        rfc_error (cU("RfcCall"));
    } 
    

    /* -----------------------------------------------
     * Wait for response or timeout
     * rfcping is used by many customers to monitor
     * SAP systems.
     * In rare occasions, (when R/3 is just comming up
     * after a restart) the first rfc request may lockup. 
     * ---------------------------------------------*/
    start_t = time(NULL);
    do
    {
        rfc_rc = RfcListen (handle);
		if (rfc_rc == RFC_RETRY)
		{
#if defined(SAPonUNIX) || defined (SAPonOS400) || defined(SAPonOS390)
			sleep(1); /*1 second*/
#elif defined(SAPonNT) || defined(OS2)
			Sleep(10); /*10 milli seconds*/
#else
#error "Error Message > Please update this function for your OS"
#endif
			now_t = time(NULL);

			if ( difftime(now_t, start_t) > 10.0) /*10 seconds*/
			{
				printfU (cU("rfcping: timeout\n"));
				exit (SAPINFO_ERROR_TIMEOUT);
			}
		}
    } while (rfc_rc == RFC_RETRY);
    
    
    if (rfc_rc != RFC_OK)
    {
        rfc_error (cU("RfcListen"));
    }          
    

    rfc_rc = RfcReceive (handle,
                         importing,
                         tables,
                         &exception_ptr);
    
    
    if (rfc_rc != RFC_OK)
    {
        if ((rfc_rc == RFC_EXCEPTION) || (rfc_rc == RFC_SYS_EXCEPTION))
        {
            rfc_error (exception_ptr);
        }
        else
        {
            rfc_error (cU("RfcReceive"));
        }
    }
    else
    {
        /* -----------------------------------------------
         * Display results
         * ---------------------------------------------*/
        display_rfcsi( &rfcsi );
        
        /* -----------------------------------------------
         * Ping Loop
         * ---------------------------------------------*/
         getResolution(&g_minRes, &g_pRes);

         atexit(dump_stats); /*register for rabax or exit */
         signal(SIGINT, myCtrlC); /*register one time mask */

         for( pcnt = 0; (pcnt < cntPing) || (cntPing == -1 ); pcnt ++)
         {
            if (g_abend != 0)
                exit(0);

            if (fMustReconnect)
            {
                 RfcClose( handle );
                 handle = RfcOpenEx (connect_param, &error_info);
                 if (handle == RFC_HANDLE_NULL)
                 {
                     printfU (cU("\nGroup       Error group %d"), error_info.group);
                     printfU (cU("\nKey         %s"), error_info.key);
                     printfU (cU("\nMessage     %s\n"), error_info.message);
                 
                     exit(SAPINFO_ERROR_RECONNECT);
                 }
             }


            rfc_rc = rfcping(handle, 
                        rfcsi.rfcdest, sizeofU(rfcsi.rfcdest), 
                        uKbPing, iTimeoutPing);
            if (RFC_OK != rfc_rc)
                break;
            
            if (g_abend != 0)
                exit(0);

            if (iIntervalPing)
                mySleep(iIntervalPing);
         }
    }

    
    /* -----------------------------------------------
     * Close connection
     * ---------------------------------------------*/
    RfcClose( handle );

    
    return SAPINFO_OK;
} /* main */

/*---------------------------------------------------------------------
 * rfcping call RFC_PING with optional payload
 *
 * @param hRfc   handle to use
 * @param szDest destination name
 * @param ccDest szDest's char count
 * @param kb     additional kb pay load
 *
 * @return success
 */
static RFC_RC rfcping(RFC_HANDLE hRfc, 
                      rfc_char_t *szDest, int ccDest,
                      unsigned kb, int iTimeoutPing)
{
    RFC_RC      rfc_rc; 
    rfc_char_t *X = NULL;
    RFC_TABLE   tables[2];
    MYTIMEMS    tmFrom, tmTo;
    unsigned int pingTime;    
    time_t      start_t, now_t;
    unsigned int uSleepTime = 1000;
    
    if (0 < kb)
    {
        tables[0].name = cU("%%PAYLOAD%%");
        tables[0].nlen = 11;
        tables[0].type = TYPX;
        tables[0].leng = 1024;
        tables[0].itmode = RFC_ITMODE_BYREFERENCE;
        tables[0].ithandle = ItCreate (cU("LOAD"), 1024, 0, 0);
        tables[0].newitab  = 0;
        if (ITAB_NULL != tables[0].ithandle)
            fill_pay_load(tables[0].ithandle, kb);            
    }
    else
    {
        tables[0].name     = NULL;
        tables[0].ithandle = ITAB_NULL;
    }
    
    tables[1].name     = NULL;
    tables[1].ithandle = ITAB_NULL;
    
    setTimeMs(&tmFrom);

    if (iTimeoutPing < 0)      /* no timeout */
    {
        rfc_rc = RfcCallReceive(hRfc, cU("RFC_PING"),
                               NULL,
                               NULL,
                               tables,
                               &X);
    }
    else /* ping with timeout */
    {
        rfc_rc = RfcCall (hRfc,
                         cU("RFC_PING"),
                         NULL,
                         tables);

        if (rfc_rc != RFC_OK)
        {
            rfc_error (cU("RfcCall"));
        }
        start_t = time(NULL);
        do
        {
            rfc_rc = RfcListen (hRfc);
		if (rfc_rc == RFC_RETRY)
		{
#if defined(SAPonUNIX) || defined (SAPonOS400) || defined(SAPonOS390)
			usleep(uSleepTime); /*1 ... 1000  millisecond*/
                       if (uSleepTime < 1000000) uSleepTime = 2*uSleepTime;
#elif defined(SAPonNT) || defined(OS2)
			Sleep(1); /*10 milli seconds*/
#else
#error "Error Message > Please update this function for your OS"
#endif
			now_t = time(NULL);

			if (difftime(now_t, start_t) >= iTimeoutPing)
			{
				printfU (cU("rfcping: timeout\n"));
				exit (SAPINFO_ERROR_TIMEOUT);
			}
		}
        } while (rfc_rc == RFC_RETRY);

        if (rfc_rc != RFC_OK)
        {
            rfc_error (cU("RfcListen"));
        }

        rfc_rc = RfcReceive (hRfc,
                             NULL,
                             tables,
                             NULL);
    }

    setTimeMs(&tmTo);
    if (RFC_OK != rfc_rc)
        rfc_error(cU("RFC_PING"));
        
    if (tables[0].ithandle)
        ItDelete(tables[0].ithandle);        
    
    pingTime = diffTimeMs(&tmTo, &tmFrom);
    if (0 != pingTime)
        printfU(cU("Reply from %.*s : bytes=%d kb time=%d %s\n"),
                    ccDest, szDest, 
                    kb, 
                    pingTime,
                    g_pRes);
    else                    
        printfU(cU("Reply from %.*s : bytes=%d kb time<%d %s\n"),
                    ccDest, szDest, 
                    kb, 
                    g_minRes,
                    g_pRes);

    if (RFC_OK == rfc_rc)
    {
        g_uMin = min(g_uMin, pingTime);
        g_uMax = max(g_uMax, pingTime);
        g_uTotal += pingTime;
        g_calls++;
    }
        
    return rfc_rc;
}/*rfcping()*/

/*---------------------------------------------------------------------
 * display calls stats at prcess end
 */
void dump_stats(void)
{
    if (g_calls != 0)
    {
        printfU(cU("\nRfcping statistics:\n"));
        printfU(cU("    Packets sent: %d \n"), g_calls);
        printfU(cU("Approximate round trip times\n"));
        printfU(cU("    Minimum = %u%s, Maximum = %u%s, Average = %u%s\n"),
            g_uMin, g_pRes,
            g_uMax, g_pRes,
            g_uTotal / g_calls, g_pRes
            );
    }
}/*dump_stats*/

/*---------------------------------------------------------------------
 * this ctrl-c handler mask ctrl-c once
 * and unregister itself
 *
 * so that we can abort long runing compressions or ni session
 * if we wake up frm the first ctrl-c we get stats
 * if we wake up from the second ctrl-c we do not get stats
 *
 * @param sig the signal (SIGINT for us)
 *
 */
void myCtrlC(int sig)
{
    /*set abend flag */
    g_abend = 1;

    /*unregister self */
    signal(sig, SIG_DFL);
}

/*---------------------------------------------------------------------
 * fill_pay_load fill a table with random load
 * we fill with random 7 bits ascii
 * 
 * @param hItab the tabel to fill
 * @param kb    the number of bytes to fill in
 *
 * @return success
 */
static void fill_pay_load(ITAB_H hItab, unsigned kb)
{
    static RFC_CHAR szPad[] = iU("0123456789")
                              iU("abcdefghijklmnopqrstuvwxyz")
                              iU("ABCDEFGHIJKLMNOPQRSTUVWXYZ") ;
                              
    unsigned   uLineSize = (ItLeng(hItab) / sizeofR(RFC_CHAR));
    unsigned   ToFill = (kb * 1024) / ItLeng(hItab);
    unsigned   u;
    
    for ( u = 0; u < ToFill; u++ )
    {
        RFC_CHAR *pE;
        RFC_CHAR *pC = (RFC_CHAR*)ItAppLine(hItab);
        if (NULL == pC)
            break;
        
        /* end of this line */    
        pE = pC + uLineSize;
        for ( ; pC < pE; pC++) 
        {
            *pC = szPad[rand() % (sizeofR(szPad) / sizeofR(RFC_CHAR))];
        }
    }       
}/*fill_pay_load()*/  


/*---------------------------------------------------------------------
 * platform wraper
 *
 * @param sec time to sleep  in seconds
 */
static void mySleep(int sec )
{
#if defined(SAPonUNIX) || defined (SAPonOS400) || defined(SAPonOS390)

    /* unix wakes up from sleep on ctrl-c 
     * without fuss
     */
    sleep(sec);         /* seconds */

#elif defined(SAPonNT) || defined(OS2)
    {
        /* we could call Sleep(1000 * sec);  
         * but then we could not react to ctrl-c
         */
        double dSecs = (double) sec;
        time_t start_t, now_t; 


        start_t = time(NULL);

        do
        {
            Sleep(300);  /* 300 msec */
            now_t = time(NULL);

            if (g_abend)
                break ;
        }
        while ( difftime(now_t, start_t) <= dSecs);
    }
#else
#error "Error Message > Please update this function for your OS"
#endif
}


#define OUTC(text, value) \
    printfU (cU("%-22s %-.*s\n"),    text, (int) sizeofU (rfcsi->value), rfcsi->value )

#define OUTX(text, value, add) \
    printfU (cU("%-22s %-.*s %s\n"), text, (int) sizeofU (rfcsi->value), rfcsi->value, (add))

#define NLINE printfU (cU("\n"))

#define HEADER(text) \
   printfU (cU("%s\n-----------------------------------------------\n\n"), \
	        (text))

static void display_rfcsi( RFCSI * rfcsi )
{
    NLINE;
    NLINE;
    
    HEADER( cU("SAP System Information") );
    
    NLINE;

    OUTC ( cU("Destination"), rfcdest );
    
    NLINE;

    if (rfcsi->rfchost2[0] != cU(' '))
      OUTC ( cU("Host"), rfchost2 );
    else
      OUTC ( cU("Host"), rfchost );

    OUTC (cU("System ID"), rfcsysid );
    OUTC (cU("Database"), rfcdatabs );
    OUTC (cU("DB host"), rfcdbhost );
    OUTC (cU("DB system"), rfcdbsys );
    NLINE;

    OUTC (cU("SAP release"), rfcsaprl );
    OUTC (cU("SAP kernel release"), rfckernrl );
    NLINE;

    OUTC (cU("RFC Protokoll"), rfcproto );

	/*UNICODE info*/
	{
		static const RFCSI_CP US_CPS[] = { 
							{ cU('4'), cU('1'), cU('0'), cU('0') }, /*UCS-2  BE*/
							{ cU('4'), cU('1'), cU('0'), cU('1') }, /*UCS-2  LE*/
							{ cU('4'), cU('1'), cU('0'), cU('2') }, /*UTF-16 BE*/
							{ cU('4'), cU('1'), cU('0'), cU('3') }  /*UTF 16 LE*/
		} ;
		static const RFCSI_CP US4_CPS[] = { /*unsupported CP*/
							{ cU('4'), cU('1'), cU('0'), cU('4') }, /*UCS-4 BE*/
							{ cU('4'), cU('1'), cU('0'), cU('5') }  /*UCS-4 LE*/
		} ;
		const RFCSI_CP *p;
		int   iFound = 0;
		for (p = US_CPS; p < (US_CPS + (sizeofR(US_CPS) / sizeofR(US_CPS[0]))) ;p++)
		{
			if (mycpeq(rfcsi->rfcchartyp, *p))
			{

				iFound = 1;
				OUTX( cU("Characters"), rfcchartyp, cU("(UNICODE PCS=2)") );
				break ;
			}
		}
		if (0 == iFound)
		{
			for (p = US4_CPS; p < (US4_CPS + (sizeofR(US4_CPS) / sizeofR(US4_CPS[0]))) ;p++)
			{
				if (mycpeq(rfcsi->rfcchartyp, *p))
				{
					OUTX( cU("Characters"), rfcchartyp, cU("(UNICODE PCS=4)") );

					iFound = 1;
					break ;
				}
			}
		}
		if (0 == iFound)
		{
			OUTX( cU("Characters"), rfcchartyp, cU("(NON UNICODE PCS=1)") );
		}

	}


    OUTC (cU("Integers "), rfcinttyp );
    OUTC (cU("Floating P."), rfcflotyp );
    OUTC (cU("SAP machine id"), rfcmach );
    NLINE;
    
    OUTX (cU("Timezone"), rfctzone,
	      rfcsi->rfcdayst[0] != cU(' ') ? cU("(Daylight saving time)") : cU("") );

    NLINE;
    NLINE;
} /* display_rfcsi */



void DLL_CALL_BACK_FUNCTION rfc_error (rfc_char_t * operation)
{
   printfU (cU("RFC Call/Exception: %s\n"), operation );

   RfcLastErrorEx(&error_info);

   printfU (cU("\nGroup       Error group %d"), error_info.group);
   printfU (cU("\nKey         %s"), error_info.key);
   printfU (cU("\nMessage     %s\n"), error_info.message);

   exit(SAPINFO_ERROR_FAILURE);
} /* rfc_error */


#define NL cU("\n")

static void help (void)
{
    printfU ( NL                                                                    );
    printfU (cU("Syntax:     rfcping -v        show version of the RFC library") NL );
    printfU (cU("        or")                                                    NL );
    printfU (cU("            rfcping <options>")                                 NL );
    printfU ( NL                                                                    );
    printfU (cU("  Options will be directly used as connection parameters")      NL );
    printfU (cU("  in RfcOpenEx and must have the following structure:")         NL );
    printfU (cU("     ID=value ID=value ... ID=value")                           NL );
    printfU (cU("  Values are case-sensitive but ID's are not.")                 NL );
    printfU (cU("  Blanks included values must be set in inverted commas.")      NL );

    printfU ( NL                                                                    );
    printfU (cU("ID's FOR RFCPING:")                                             NL );
    printfU (cU("  ping_cnt=<nb> optional: number of echo request to send")      NL );
    printfU (cU("           default value 4, value -1 == for ever")              NL );
    printfU (cU("  ping_kb=<nb> optional: number of KB to send")                  NL );
    printfU (cU("          default value 0, this additional data is send oneway") NL );
    printfU (cU("          to the server, only the acknowledge is sent back.")    NL );
    printfU (cU("  ping_interval=<nb> optional: number of seconds between calls.")      NL );
    printfU (cU("           default value 0.")                                   NL );
    printfU (cU("  ping_timeout=<nb> optional: number of seconds before timeout occurs for RFC_PING.")      NL );
    printfU (cU("           default value 30, value -1 == no timeout")           NL );

    printfU ( NL                                                                    );
    printfU (cU("ID's FOR SAP LOGON:")                                           NL );
    printfU (cU("  client=... user=... passwd=... lang=...")                     NL );

    printfU ( NL                                                                    );
    printfU (cU("SPECIAL ID's")                                                  NL );
    printfU (cU("  trace=[0,1] abap_debug=[0,1] use_sapgui=[0,1,2]")             NL );

    printfU ( NL                                                                    );
    printfU (cU("CONNECT TO R/3:")                                               NL );
    printfU (cU("  + USING saprfc.ini:")                                         NL );
    printfU (cU("      rfcping dest=BIN")                                        NL );
    printfU ( NL                                                                    );
    printfU (cU("  + NO USE OF saprfc.ini")                                      NL );
    printfU (cU("    - LOAD BALANCING")                                          NL );
    printfU (cU("      * USING sapmsg.ini/saproute.ini OF SAPLOGON")             NL );
    printfU (cU("          Usually, these files are installed in the")           NL );
    printfU (cU("          Windows directory. You can also copy these")          NL );
    printfU (cU("          files in a directory which is specified by")          NL );
    printfU (cU("          the environment RFC_LOGON_INI_PATH.")                 NL );
    printfU (cU("          On NON-Windows platforms, you can work with")         NL );
    printfU (cU("          this environment or copy these files in your")        NL );
    printfU (cU("          working directory.")                                  NL );
    printfU (cU("            rfcping r3name=BIN [group=...]")                    NL );
    printfU (cU("          or")                                                  NL );
    printfU (cU("            rfcping r3name=/H/sapgate1/S/3297/BIN")             NL );
    printfU (cU("          or")                                                  NL );
    printfU (cU("            rfcping r3name=/<Router name in saprouter.ini>/BIN")NL );
    printfU ( NL                                                                    );
    printfU (cU("      * NO USE OF sapmsg.ini/saproute.ini")                     NL );
    printfU (cU("          rfcping r3name=BIN mshost=hs0311 [group=...]")        NL );
    printfU (cU("        or")                                                    NL );
    printfU (cU("          rfcping r3name=BIN mshost=/H/sapgate1/S/3297/H/hs0311") NL );
    printfU ( NL                                                                    );
    printfU (cU("    - SPECIFIC APPLICATION SERVER")                             NL );
    printfU (cU("        rfcping ashost=pcintel sysnr=53")                       NL );
    printfU (cU("      or")                                                      NL );
    printfU (cU("        rfcping ashost=/H/sapgate1/S/3297/H/pcintel sysnr=53")  NL );

    printfU ( NL                                                                    );
    printfU (cU("CONNECT TO R/2:")                                               NL );
    printfU (cU("  + USING saprfc.ini:")                                         NL );
    printfU (cU("      rfcping type=2 dest=K50")                                 NL );
    printfU ( NL                                                                    );
    printfU (cU("  + NO USE OF saprfc.ini")                                      NL );
    printfU (cU("      rfcping type=2 dest=K50 gwhost=... gwserv=...")           NL );
    printfU ( NL                                                                    );

    printfU (cU("CONNECT TO EXTERNAL PROGRAM")                                   NL );
    printfU (cU("  + USING saprfc.ini:")                                         NL );
    printfU (cU("      rfcping type=E dest=RFCEXT")                              NL );
    printfU ( NL                                                                    );
    printfU (cU("  + NO USE OF saprfc.ini")                                      NL );
    printfU (cU("    - RFC SERVER IS ALREADY REGISTERED AT SAP GATEWAY")         NL );
    printfU (cU("        rfcping type=E gwhost=... gwserv=... tpname=...")       NL );
    printfU ( NL                                                                    );
    printfU (cU("    - RFC SERVER WILL BE STARTED BY SAP GATEWAY")               NL );
    printfU (cU("        rfcping type=E gwhost=... gwserv=... tpname=... tphost=...") NL );
    printfU ( NL                                                                    );
    printfU (cU("RETURN VALUE")                                                  NL );
    printfU (cU("  0 OK")                                                        NL );
    printfU (cU("  1 Unreckognized parameter")                                   NL );
    printfU (cU("  2 Connect Failure: system down or wrong parameters")          NL );
    printfU (cU("  3 Timeout from call")                                         NL );
    printfU (cU("  4 Unspecified failure: logon/memory/codepage")                NL );
    printfU (cU("  5 Reconnect Failure: cannot reconnect to external server")    NL );

    return;
}

/*time span wrapper
*/
/* UNIX ----------------------------------------------------------------*/
#if defined(SAPonUNIX)

static void setTimeMs(MYTIMEMS *pToInit)
{
    struct timezone tzp;
    gettimeofday(pToInit, &tzp);
}/*setTimeMs unix*/

static unsigned int diffTimeMs(MYTIMEMS *pTo, MYTIMEMS *pFrom)
{
    unsigned int    msectime;
    
    if (pFrom->tv_usec > pTo->tv_usec)
    {
        pTo->tv_usec += 1000000;
        pTo->tv_sec--;
    }
    msectime = (pTo->tv_usec - pFrom->tv_usec)/1000 +
	           (pTo->tv_sec - pFrom->tv_sec)*1000;

    return msectime;
}/*diffTimeMs unix */

static int getResolution(unsigned int *pRes, SAP_CHAR** ppUnit)
{
    MYTIMEMS tsS, tsE;
    unsigned int s1, s2, s3, r;
    
    setTimeMs(&tsS);
    do{
        setTimeMs(&tsE);
    } while( 0 == (s1 = diffTimeMs(&tsE, &tsS)));
    
    setTimeMs(&tsS);
    do{
        setTimeMs(&tsE);
    } while( 0 == (s2 = diffTimeMs(&tsE, &tsS)));
    setTimeMs(&tsS);
    
    do{
        setTimeMs(&tsE);
    } while( 0 == (s3 = diffTimeMs(&tsE, &tsS)));
    
    r = min(s1, s2);
    r = min( r, s3);
    
    *pRes = r;
    *ppUnit = cU("ms");

    return 0;
}/*getResolution UNIX*/

                        
/* OS400 ---------------------------------------------------------------*/
/* the perflib contains beter resolution, but this is to cumbersome to
 * use in customer code , so keep it simple with second resolution
 */

#elif defined(SAPonOS400)
static void setTimeMs(MYTIMEMS *pToInit)
{
    *pToInit = time(NULL);
} /*setTimeMs os400 */

static unsigned int diffTimeMs(MYTIMEMS *pTo, MYTIMEMS *pFrom)
{
    if (((unsigned int)(*pTo)) > ((unsigned int)(*pFrom)))
        return (((unsigned int)(*pTo)) - ((unsigned int)(*pFrom)));
    else
        return (((unsigned int)(*pFrom)) - ((unsigned int)(*pTo)));
}/*diffTimeMs os400*/

static int getResolution(unsigned int *pRes, SAP_CHAR** ppUnit)
{
    
    *pRes = 1;
    *ppUnit = cU("s");

    return 0;

}/*getResolution os400*/
                                              
/* NT, OS2, MAC---------------------------------------------------------*/
#else
static void setTimeMs(MYTIMEMS *pToInit)
{
    ftime(pToInit);
}/*setTimeMs NT*/

static unsigned int diffTimeMs(MYTIMEMS *pTo, MYTIMEMS *pFrom)
{
    unsigned int    msectime;
    
    if (pFrom->millitm > pTo->millitm)
    {
        pTo->millitm = pTo->millitm + 1000;
        pTo->time = pTo->time - 1;
    }
    msectime = (pTo->millitm - pFrom->millitm) +
	           (pTo->time - pFrom->time)*1000;

    return msectime;
}/*diffTimeMs NT*/

static int getResolution(unsigned int *pRes, SAP_CHAR** ppUnit)
{
    MYTIMEMS tsS, tsE;
    unsigned int s1, s2, s3, r;
    
    setTimeMs(&tsS);
    do{
        setTimeMs(&tsE);
    } while( 0 == (s1 = diffTimeMs(&tsE, &tsS)));
    
    setTimeMs(&tsS);
    do{
        setTimeMs(&tsE);
    } while( 0 == (s2 = diffTimeMs(&tsE, &tsS)));
    setTimeMs(&tsS);
    
    do{
        setTimeMs(&tsE);
    } while( 0 == (s3 = diffTimeMs(&tsE, &tsS)));
    
    r = min(s1, s2);
    r = min( r, s3);
    
    *pRes = r;
    *ppUnit = cU("ms");

    return 0;
}/*getResolution NT*/

#endif
/*end of rfcping*/
