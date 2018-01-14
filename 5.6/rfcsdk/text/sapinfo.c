static char sccsid[] = "@(#) $Id: //bas/640_REL/src/krn/rfc/sapinfo.c#7 $ SAP";

/*--------------------------------------------------------------------*
 *	DEPENDENCIES
 *--------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* Dont use SAP.. switches before this inclusion: */
#include "saprfc.h"
#include "rfcsi.h"

#ifdef SAPonUNIX
#include <unistd.h>
#endif
#ifdef SAPonNT
#include <windows.h>
#endif

#if defined(OS2)
 /* map os/2 DosSleep to NT Sleep call */
 extern unsigned short _System Sleep( unsigned long msec );
 #pragma import(Sleep,,"DOSCALLS",229)
#endif


/*--------------------------------------------------------------------*
 *	DEFINES
 *--------------------------------------------------------------------*/
#define SAPINFO_OK              0 /* OK */
#define SAPINFO_ERROR_PARAMETER 1 /* Wrong argc/argv */
#define SAPINFO_ERROR_CONNECT   2 /* Connect failure (system down wrong parameters)*/
#define SAPINFO_ERROR_TIMEOUT   3 /* Timeout from call/receive */
#define SAPINFO_ERROR_FAILURE   4 /* Others (codepage, memory, logon,...)*/


/*--------------------------------------------------------------------*
 *	LOCAL
 *--------------------------------------------------------------------*/
void DLL_CALL_BACK_FUNCTION rfc_error( rfc_char_t * operation );

static void display_rfcsi( RFCSI * rfcsi );
static void help( void );

/*helper define (saptypec not part from sdk)*/
typedef SAP_CHAR RFCSI_CP[4] ;
#define mycpeq(l,r)   (    ((l)[0] == (r)[0]) \
						&& ((l)[1] == (r)[1]) \
						&& ((l)[2] == (r)[2]) \
						&& ((l)[3] == (r)[3]) )


/*--------------------------------------------------------------------*
 *	GLOBALS
 *--------------------------------------------------------------------*/
RFC_ERROR_INFO_EX  error_info;

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
    time_t              start_t, now_t; 


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

	/*
	 * give compiler version.
	 * Tells RFC SDK API programmers which compiler to use.
	 * Cf http://predef.sourceforge.net/precomp.html for
	 * predefined macros giving version info. 
	 */
	printfU (
		cU("Compiled by compiler version:\n  ")
#if defined SAPonAIX
	#if !defined(SAPccQ)
		cU("%04X (VVRR)\n"), __xlC__
	#else
		cU("ccQ (VVRR)\n")
	#endif
#elif defined SAPonHP_UX
		cU("%06d (VVRRPP. %s Compiler)\n"),	/*yes, decimal here!*/
	#if defined __HP_cc
			__HP_cc, cU("C")
	#elif defined __HP_aCC
			__HP_aCC, cU("C++")
	#else
			0, cU("Unknown Version")
	#endif
#elif defined SAPonLINUX
		cU("%s\n"), cU(__VERSION__)
#elif defined SAPonNT
		cU("%04d (VVRR)\n"), _MSC_VER		/*decimal!*/
#elif defined SAPonOSF1
		cU("%09d (VVRRTPPPP. %s Compiler)\n"),	/*decimal!*/
	#if defined __DECC
			__DECC_VER, cU("C")
	#elif defined __DECCXX
			__DECCXX_VER, cU("C++")
	#else
			0, cU("Unknown Version")
	#endif
#elif defined SAPonSUN
		cU("%03X (VRP. %s Compiler)\n"),
	#if defined __SUNPRO_C
			__SUNPRO_C, cU("C")
	#elif defined __SUNPRO_CC
			__SUNPRO_CC, cU("C++")
	#else
			0, cU("Unknown Version")
	#endif
#else
		cU("%s\n"), cU("Version not available.")
#endif
	);
    
        return SAPINFO_OK;
    }


    /* -----------------------------------------------
     * Check input for old version of sapinfo
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
    }

    
    /* No logon check at OPEN time */
    strcpyU (connect_param+strlenU (connect_param), cU("LCHECK=0"));

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
     * sapinfo is used by many customers to monitor
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
				printfU (cU("sapinfo: timeout\n"));
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
    }

    
    /* -----------------------------------------------
     * Close connection
     * ---------------------------------------------*/
    RfcClose( handle );

    
    return SAPINFO_OK;
} /* main */


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
    printfU (cU("Syntax:     sapinfo -v        show version of the RFC library") NL );
    printfU (cU("        or")                                                    NL );
    printfU (cU("            sapinfo <options>")                                 NL );
    printfU ( NL                                                                    );
    printfU (cU("  Options will be directly used as connection parameters")      NL );
    printfU (cU("  in RfcOpenEx and must have the following structure:")         NL );
    printfU (cU("     ID=value ID=value ... ID=value")                           NL );
    printfU (cU("  Values are case-sensitive but ID's are not.")                 NL );
    printfU (cU("  Blanks included values must be set in inverted commas.")      NL );

    printfU ( NL                                                                    );
    printfU (cU("ID's FOR SAP LOGON:")                                           NL );
    printfU (cU("  client=... user=... passwd=... lang=...")                     NL );

    printfU ( NL                                                                    );
    printfU (cU("SPECIAL ID's")                                                  NL );
    printfU (cU("  trace=[0,1] abap_debug=[0,1] use_sapgui=[0,1,2]")             NL );

    printfU ( NL                                                                    );
    printfU (cU("CONNECT TO R/3:")                                               NL );
    printfU (cU("  + USING saprfc.ini:")                                         NL );
    printfU (cU("      sapinfo dest=BIN")                                        NL );
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
    printfU (cU("            sapinfo r3name=BIN [group=...]")                    NL );
    printfU (cU("          or")                                                  NL );
    printfU (cU("            sapinfo r3name=/H/sapgate1/S/3297/BIN")             NL );
    printfU (cU("          or")                                                  NL );
    printfU (cU("            sapinfo r3name=/<Router name in saprouter.ini>/BIN")NL );
    printfU ( NL                                                                    );
    printfU (cU("      * NO USE OF sapmsg.ini/saproute.ini")                     NL );
    printfU (cU("          sapinfo r3name=BIN mshost=hs0311 [group=...]")        NL );
    printfU (cU("        or")                                                    NL );
    printfU (cU("          sapinfo r3name=BIN mshost=/H/sapgate1/S/3297/H/hs0311") NL );
    printfU ( NL                                                                    );
    printfU (cU("    - SPECIFIC APPLICATION SERVER")                             NL );
    printfU (cU("        sapinfo ashost=pcintel sysnr=53")                       NL );
    printfU (cU("      or")                                                      NL );
    printfU (cU("        sapinfo ashost=/H/sapgate1/S/3297/H/pcintel sysnr=53")  NL );

    printfU ( NL                                                                    );
    printfU (cU("CONNECT TO R/2:")                                               NL );
    printfU (cU("  + USING saprfc.ini:")                                         NL );
    printfU (cU("      sapinfo type=2 dest=K50")                                 NL );
    printfU ( NL                                                                    );
    printfU (cU("  + NO USE OF saprfc.ini")                                      NL );
    printfU (cU("      sapinfo type=2 dest=K50 gwhost=... gwserv=...")           NL );
    printfU ( NL                                                                    );

    printfU (cU("CONNECT TO EXTERNAL PROGRAM")                                   NL );
    printfU (cU("  + USING saprfc.ini:")                                         NL );
    printfU (cU("      sapinfo type=E dest=RFCEXT")                              NL );
    printfU ( NL                                                                    );
    printfU (cU("  + NO USE OF saprfc.ini")                                      NL );
    printfU (cU("    - RFC SERVER IS ALREADY REGISTERED AT SAP GATEWAY")         NL );
    printfU (cU("        sapinfo type=E gwhost=... gwserv=... tpname=...")       NL );
    printfU ( NL                                                                    );
    printfU (cU("    - RFC SERVER WILL BE STARTED BY SAP GATEWAY")               NL );
    printfU (cU("        sapinfo type=E gwhost=... gwserv=... tpname=... tphost=...") NL );
    printfU ( NL                                                                    );
    printfU (cU("RETURN VALUE")                                                  NL );
    printfU (cU("  0 OK")                                                        NL );
    printfU (cU("  1 Unreckognized parameter")                                   NL );
    printfU (cU("  2 Connect Failure: system down or wrong parameters")          NL );
    printfU (cU("  3 Timeout from call")                                         NL );
    printfU (cU("  4 Unspecified failure: logon/memory/codepage")                NL );

    return;
}
