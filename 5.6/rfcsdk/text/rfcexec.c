

/*
 *   R  F  C  E  X  E  C
 *
 *   rfc server program offering function modules
 *   for file and pipe access.
 *
 */

#define _XOPEN_SOURCE   /* popen(), pclose() */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>


/*
 * include RFC API
 */

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

#if defined (WIN32) || defined (WIN64)
#define popen _popen
#define pclose _pclose
#endif

#ifdef VMS
#include "vmspipe.h"
rfc_char_t *vmsstrerror (int error_code);
#endif

#ifdef SAPonOS400
#include <spawn.h>
extern int o4_convert_environ_a(void);
DECLAREenvironU;
#endif

#if defined(SAPonWINDOWS) || defined(SAPonNT)
#include "windows.h"
#endif

#ifdef SIGCHLD
#ifndef SAPonVMS
#include <sys/wait.h>
#endif
#endif

#ifdef OS2
  FILE *       popen  ( rfc_char_t *command,  rfc_char_t *type );
  int          pclose ( FILE *stream );
#endif

#if defined(SAPonUNIX)
    static const SAP_UC os_separator = cU('/');
#elif defined(SAPonNT)
    static const SAP_UC os_separator = cU('\\');
#elif defined(SAPonOS400)
    static const SAP_UC os_separator = cU('/');
#elif
    #error Please define your separator
#endif


/*
 * local prototypes & declarations
 */
static RFC_RC run ( RFC_HANDLE handle, rfc_char_t * command, 
                    unsigned comsize, ITAB_H itab_h, int mode,
                    RFC_INT *status );

static RFC_RC file ( RFC_HANDLE handle, rfc_char_t * filename,
                     unsigned size, ITAB_H itab_h, int mode );

static RFC_RC pipe_itab  ( RFC_HANDLE handle,
                           rfc_char_t * command,
                           ITAB_H itab_h,
                           int    mode,
                           RFC_INT *status );

#ifdef SAPonWINDOWS
/* _loadds in protoype is necessary for QuickWin programs */

static RFC_RC DLL_CALL_BACK_FUNCTION _loadds remote_pipe( RFC_HANDLE handle );
static RFC_RC DLL_CALL_BACK_FUNCTION _loadds remote_file( RFC_HANDLE handle );
static RFC_RC DLL_CALL_BACK_FUNCTION _loadds remote_exec( RFC_HANDLE handle );
static RFC_RC DLL_CALL_BACK_FUNCTION _loadds mail       ( RFC_HANDLE handle );
#else

static RFC_RC DLL_CALL_BACK_FUNCTION remote_pipe( RFC_HANDLE handle );
static RFC_RC DLL_CALL_BACK_FUNCTION remote_file( RFC_HANDLE handle );
static RFC_RC DLL_CALL_BACK_FUNCTION remote_exec( RFC_HANDLE handle );
static RFC_RC DLL_CALL_BACK_FUNCTION mail       ( RFC_HANDLE handle );
#endif

static RFC_RC DLL_CALL_BACK_FUNCTION rfc_raise_error( RFC_HANDLE handle );
static RFC_RC file_access ( RFC_HANDLE,
                            rfc_char_t * filename,
                            ITAB_H itab_h,
                            int    open_mode );

static RFC_RC install ( RFC_HANDLE handle );

static int itab2file  ( FILE * fil, ITAB_H itab_h );
static int file2itab  ( ITAB_H itab_h, FILE * fil );

static rfc_char_t * remote_exec_docu ( void );
static rfc_char_t * mail_docu        ( void );
static rfc_char_t * remote_file_docu ( void );
static rfc_char_t * remote_pipe_docu ( void );
static rfc_char_t * rfc_raise_error_docu ( void );

#define RUN_FREE    0
#define RUN_WAIT    1
#define RUN_READ    2

#define FILE_READ   0
#define FILE_WRITE  1

#define NL          cU("\n")
#define NL_AR       iU("\n")

static const unsigned table_size = 80;
static int            errorline;
static void help(void);

/* security extenions */
typedef struct
{
    rfc_char_t    user[12+1];
    rfc_char_t    function_list[RFC_FUNCTIONNAME_LN*100+1]; 
    rfc_char_t    sysid[8+1];
    rfc_char_t    client[3+1];
    rfc_char_t    language[1+1];
    rfc_char_t    partner_type[12+1];
    rfc_char_t    snc_partner_name[2048];
    rfc_char_t    os_commands[2048];
} PERMISSIONS_ATTRIBUTES;

static ITAB_H sec_itab = ITAB_NULL;

RFC_SNC_MODE  snc_mode = RFC_SNC_MODE_OFF;

PERMISSIONS_ATTRIBUTES  sec_attr;

static unsigned check_os_commands (rfc_char_t * command);

static RFC_RC install_security_attributes (RFC_HANDLE handle);

static rfc_char_t * search_sec_entry (const rfc_char_t * source, 
                                      const rfc_char_t * ident,
                                      rfc_char_t       * target,
                                      unsigned           tsize);


static RFC_EXTERNAL_LOGON_RC DLL_CALL_BACK_FUNCTION 
            rfcexec_logon_check  (RFC_HANDLE                handle,
                                  RFC_EXTERNAL_LOGON_CNTL * logon_cntl, 
                                  rfc_char_t              * message);


/*
 * error handler : ignore all errors and exit.
 * static functions cannot be exported in Windows,
 * so dont make myErrorhandler static
 */
void DLL_CALL_BACK_FUNCTION myErrorhandler ( rfc_char_t * errorId )
{
   /* Wegen Compiler Warnings */
   errorId = errorId;

   exit( 1 );
}


/*
 *  main function for an RFC server program
 */

/*ARGSUSED*/
int mainU (int argc, rfc_char_t ** argv)
{
    /* initialized data */
    static RFC_ENV    env;

    RFC_HANDLE handle;
    RFC_RC     rc;

    if (argc == 1)
    {
        help();
    
        return 0;
    }

    /*
    * install error handler
    */
    env.errorhandler = myErrorhandler;

    RfcEnvironment( &env );

    /*
     * accept connection
     *
     * (command line argv must be passed to RfcAccept)
     */
    handle = RfcAccept (argv);


    /* in case of error exit without message */
    if (handle == RFC_HANDLE_NULL)
    {
        exit (1);
    }


    /* Initialize permissions mechanims */
    rc = install_security_attributes (handle);

    if (rc != RFC_OK)
    {
        /* RfcAbort was already called in install_security_attributes */
        exit (1);
    }

    /*
    * static function to install offered function modules
    * ( see below )
    */
    rc = install(handle);
    
    if (rc != RFC_OK )
    {
        /*
         * if error occured,
         * close connection with error message and exit
         */
        RfcAbort (handle, cU("Initialization error") );
    
        exit(1);
    }


#ifdef SAPonNT
    /* if one uses rfcexec as a bootstrap to start the
     * RFC COM support features, one need to initialize
     * Win32's COM routines
     * we discared the return value since there are few
     * users for this scenario. If this call fails the
     * follwing COM calls will break anyway, so that users
     * which do need this call will not go far.
     * for users, which do not need this call,
     * it would be unfortunate to stop here
     */
    (void)CoInitialize(NULL);
#endif

    /*
    * enter main loop
    */
    do
    {
        rc = RfcDispatch (handle);
    } while (rc == RFC_OK);


    /*
     * connection was closed by the client :
     * also close connection and terminate
     */
    RfcClose( handle );


#ifdef SAPonNT
    (void)CoUninitialize();
#endif


    return 0;
} /* main */


/*ARGSUSED*/
static RFC_RC install( RFC_HANDLE handle )
{
   RFC_RC rc;

   /*
    * install the function modules offered
    *
    * the documentation texts are placed in static memory
    * within some static functions to keep things readable.
    */

   /*
    * Mail interface (only as example, not for official use)
    */
#ifdef SAPonWINDOWS
   rc = RfcInstallFunctionExt (handle, cU("RFC_MAIL"),
                               (RFC_ONCALL)mail,
                               mail_docu() );
#else
   rc = RfcInstallFunction (cU("RFC_MAIL"),
                            mail,
                            mail_docu() );
#endif /* SAPonWindows */
   if( rc != RFC_OK ) return rc;

   /*
    * Pipe interface
    */
#ifdef SAPonWINDOWS
   rc = RfcInstallFunctionExt (handle, cU("RFC_REMOTE_PIPE"),
                               (RFC_ONCALL)remote_pipe,
                               remote_pipe_docu() );
#else
   rc = RfcInstallFunction (cU("RFC_REMOTE_PIPE"),
                            remote_pipe,
                            remote_pipe_docu() );
#endif /* SAPonWindows */
   if( rc != RFC_OK ) return rc;

   /*
    * Text file interface
    */
#ifdef SAPonWINDOWS
   rc = RfcInstallFunctionExt (handle, cU("RFC_REMOTE_FILE"),
                               (RFC_ONCALL)remote_file,
                               remote_file_docu() );
#else
   rc = RfcInstallFunction (cU("RFC_REMOTE_FILE"),
                            remote_file,
                            remote_file_docu() );
#endif /* SAPonWindows */
   if( rc != RFC_OK ) return rc;

   /*
    * Asynchronous pipe interface
    */
#ifdef SAPonWINDOWS
   rc = RfcInstallFunctionExt (handle, cU("RFC_REMOTE_EXEC"),
                               (RFC_ONCALL)remote_exec,
                               remote_exec_docu() );
#else
   rc = RfcInstallFunction (cU("RFC_REMOTE_EXEC"),
                            remote_exec,
                            remote_exec_docu() );
#endif /* SAPonWindows */
   if( rc != RFC_OK ) return rc;

   /*
    * raises an error like RFM RFC_RAISE_ERROR
    */
#ifdef SAPonWINDOWS
   rc = RfcInstallFunctionExt (handle, cU("RFC_RAISE_ERROR"),
                               (RFC_ONCALL)rfc_raise_error,
                               rfc_raise_error_docu() );
#else
   rc = RfcInstallFunction (cU("RFC_RAISE_ERROR"),
                            rfc_raise_error,
                            rfc_raise_error_docu() );
#endif /* SAPonWindows */
   if( rc != RFC_OK ) return rc;

   return RFC_OK;
} /* install */

/*
 *
 * code representing the offered function modules
 *
 */

/*
 * function RFC_REMOTE_FILE
 *
 * _loadds in protoype is necessary for QuickWin programs
 *
 */
#ifdef SAPonWINDOWS
static RFC_RC DLL_CALL_BACK_FUNCTION _loadds remote_file(  RFC_HANDLE handle )
#else
static RFC_RC DLL_CALL_BACK_FUNCTION remote_file(  RFC_HANDLE handle )
#endif
{
    rfc_char_t          filename[256];
    RFC_PARAMETER       parameter[4];
    RFC_TABLE           table[2];
    RFC_RC              rc;
    RFC_CHAR            write_flag;
    int                 mode;

    /*
     * prolog
     *
     * receive parameters and tables
     */

    /* defaults */
    memsetU ( filename, cU('\0'), sizeofU( filename ) );
    write_flag = 0;

    /* 1st parameter */
    parameter[0].name = cU("FILE");
    parameter[0].nlen = 4;
    parameter[0].addr = (void *) filename;
    parameter[0].leng = sizeofR(filename);
    parameter[0].type = RFCTYPE_CHAR;

    /* 2nd parameter */
    parameter[1].name = cU("WRITE");
    parameter[1].nlen = 5;
    parameter[1].addr = (void *) &write_flag;
    parameter[1].leng = sizeofR(write_flag);
    parameter[1].type = RFCTYPE_CHAR;

    /* terminate array */
    parameter[2].name = NULL;

    /* table */
    table[0].name = cU("FILEDATA");
    table[0].nlen = 8;
    table[0].type = RFCTYPE_CHAR;
    table[0].leng = table_size*sizeofR(SAP_UC);
    table[0].itmode = RFC_ITMODE_BYREFERENCE; /* pass by reference */

    /* terminate array */
    table[1].name = NULL;

    /* receive data */
    rc = RfcGetData( handle, parameter, table );

    if( rc != RFC_OK ) return rc;

    if( write_flag != cU('X') )
      mode = FILE_READ;
    else
      mode = FILE_WRITE;

    /*
     * do the work
     */
    rc = file( handle, filename, sizeofU(filename),
		       table[0].ithandle, mode );

    if( rc != RFC_OK ) return rc;

    /*
     * epilog
     *
     * return parameters and tables
     *
     * 'table' must be passed from RfcGetData without changes
     */

    /* no parameters */
    parameter[0].name = NULL;

    /* return to the caller */
    rc = RfcSendData( handle, parameter, table );

    return rc;
} /* remote_file */


/*
 * Function RFC_REMOTE_PIPE
 *
 * _loadds in protoype is necessary for QuickWin programs
 *
 */
#ifdef SAPonWINDOWS
static RFC_RC DLL_CALL_BACK_FUNCTION _loadds remote_pipe(  RFC_HANDLE handle )
#else
static RFC_RC DLL_CALL_BACK_FUNCTION remote_pipe(  RFC_HANDLE handle )
#endif
{
    rfc_char_t          command[256];
    RFC_PARAMETER       parameter[4];
    RFC_TABLE           table[2];
    RFC_RC              rc;
    RFC_CHAR            read_flag = 0;
    int                 mode;

    memsetR ( command, 0, sizeofR( command ) );

    parameter[0].name = cU("COMMAND");
    parameter[0].nlen = 7;
    parameter[0].addr = (void *) command;
    parameter[0].leng = sizeofR(command);
    parameter[0].type = RFCTYPE_CHAR;

    parameter[1].name = cU("READ");
    parameter[1].nlen = 4;
    parameter[1].addr = (void *) &read_flag;
    parameter[1].leng = sizeofR(read_flag);
    parameter[1].type = RFCTYPE_CHAR;
    parameter[2].name = NULL;

    table[0].name = cU("PIPEDATA");
    table[0].nlen = 8;
    table[0].type = RFCTYPE_CHAR;
    table[0].leng = table_size*sizeofR(SAP_UC);
    table[0].itmode = RFC_ITMODE_BYREFERENCE;

    table[1].name = NULL;

    rc = RfcGetData( handle, parameter, table );
    if( rc != RFC_OK ) return rc;

#ifdef SAPonWINDOWS
    RfcAbort(handle, cU("Function RFC_REMOTE_PIPE is not supported on Windows"));
    exit(1);
#endif

    if( read_flag != cU('X') )
      mode = RUN_WAIT;
    else
      mode = RUN_READ;

    rc = run( handle, command, sizeofU(command),
		      table[0].ithandle, mode, (RFC_INT *)0 );

    if( rc != RFC_OK ) return rc;

    parameter[0].name = NULL;
    rc = RfcSendData( handle, parameter, table );

    return rc;
} /* remote_pipe */


/*
 * Function RFC_REMOTE_EXEC
 *
 * _loadds in protoype is necessary for QuickWin programs
 *
 */
#ifdef SAPonWINDOWS
static RFC_RC DLL_CALL_BACK_FUNCTION _loadds remote_exec(  RFC_HANDLE handle )
#else
static RFC_RC DLL_CALL_BACK_FUNCTION remote_exec(  RFC_HANDLE handle )
#endif
{
    rfc_char_t          command[256];
    RFC_PARAMETER       parameter[2];
    RFC_TABLE           table[2];
    RFC_RC              rc;

    memsetR (command, 0, sizeofR (command));

    parameter[0].name = cU("COMMAND");
    parameter[0].nlen = 7;
    parameter[0].addr = (void *) command;
    parameter[0].leng = sizeofR (command);
    parameter[0].type = RFCTYPE_CHAR;
    parameter[1].name = NULL;

    table[0].name = cU("PIPEDATA");
    table[0].nlen = 8;
    table[0].type = RFCTYPE_CHAR;
    table[0].leng = table_size*sizeofR(SAP_UC);
    table[0].itmode = RFC_ITMODE_BYREFERENCE;

    table[1].name = NULL;


    rc = RfcGetData( handle, parameter, table );
    
    if( rc != RFC_OK ) 
    {
        return rc;
    }


    rc = run( handle, command, sizeofU(command),
		      table[0].ithandle, RUN_FREE, (RFC_INT *)0 );

    if (rc != RFC_OK) 
    {
        return rc;
    }

    parameter[0].name = NULL;
    
    rc = RfcSendData (handle, parameter, table);
    

    return rc;
} /* remote_exec */


/*
 * Function RFC_MAIL (only as example, not for official use)
 *
 * _loadds in protoype is necessary for QuickWin programs
 *
 */
#ifdef SAPonWINDOWS
static RFC_RC DLL_CALL_BACK_FUNCTION _loadds mail(  RFC_HANDLE handle )
#else
static RFC_RC DLL_CALL_BACK_FUNCTION mail(  RFC_HANDLE handle )
#endif
{
    rfc_char_t          user[256];
    rfc_char_t          command[sizeofU(user) + 32];
    RFC_PARAMETER       parameter[2];
    RFC_TABLE           table[2];
    RFC_RC              rc;

    memsetR (user, 0, sizeofR( user ) );

    parameter[0].name = cU("USER");
    parameter[0].nlen = 4;
    parameter[0].addr = (void *) user;
    parameter[0].leng = sizeofR(user);
    parameter[0].type = RFCTYPE_CHAR;
    parameter[1].name = NULL;

    table[0].name = cU("MAIL");
    table[0].nlen = 4;
    table[0].type = RFCTYPE_CHAR;
    table[0].leng = table_size*sizeofR(SAP_UC);
    table[0].itmode = RFC_ITMODE_BYREFERENCE;

    table[1].name = NULL;

    rc = RfcGetData( handle, parameter, table );
    if( rc != RFC_OK ) return rc;

    sprintfU ( command, cU("mail %.*s"), (int) sizeofU(user), user );

#if defined(SAPonNT) || defined(SAPonWINDOWS)
    RfcAbort(handle, cU("Function RFC_MAIL is not supported on Windows"));
    exit(1);
#endif

    rc = run( handle, command, strlenU (command),
		      table[0].ithandle, RUN_WAIT, (RFC_INT *)0 );

    if( rc != RFC_OK ) return rc;

    parameter[0].name = NULL;
    rc = RfcSendData( handle, parameter, table );

    return rc;
} /* mail */

/* Raises an Error like RFM RFC_RAISE_ERROR */
static RFC_RC DLL_CALL_BACK_FUNCTION rfc_raise_error( RFC_HANDLE handle )
{
    /* variables declaration */
    int div_by_zero = 0;
    int i_method    = 0; /* Error method as int */
    
    RFC_PARAMETER    import[4];
    RFC_PARAMETER    export[3];
    RFC_TABLE        table[1];
    RFC_RC           rc;

    RFC_INT          counter = 0;
    RFC_CHAR         method[4];
    RFC_CHAR         messagetype[2] = iU("S");
    RFC_CHAR         parameter[250];
    RFC_CHAR         return_value[250] = iU(" ");
    
    /* variables for call back */
    rfc_char_t          *exception  = NULL;
    RFC_ERROR_INFO_EX   error_info;
    RFC_PARAMETER       cbexport[2];
    RFC_CHAR            cbmethod[4] = iU("0");
    RFC_CHAR            msg006[] = iU("Function not supported");
    /* end of variables declaration */
    
    memsetU (parameter, cU(' '), 250);
    memsetU (method, cU(' '), 4);
    
    
    memsetR (&export[0], 0, sizeofR (export));
    memsetR (&import[0], 0, sizeofR (import));
    memsetR (&table[0], 0, sizeofR (table));
    

    /* declaration of meta data  */
    import[0].name = cU("METHOD");
    import[0].nlen = 6;
    import[0].addr = &method;
    import[0].leng = sizeofR(method);
    import[0].type = RFCTYPE_CHAR;
    
    import[1].name = cU("MESSAGETYPE");
    import[1].nlen = 11;
    import[1].addr = &messagetype;
    import[1].leng = sizeofR(messagetype);
    import[1].type = RFCTYPE_CHAR;
    
    import[2].name = cU("PARAMETER");
    import[2].nlen = 9;
    import[2].addr = &parameter;
    import[2].leng = sizeofR(parameter);
    import[2].type = RFCTYPE_CHAR;
	
	import[3].name = NULL;

    export[0].name = cU("COUNTER");
    export[0].nlen = 7;
    export[0].addr = &counter;
    export[0].leng = sizeofR(counter);
    export[0].type = RFCTYPE_INT;
    
    export[1].name = cU("RETURN_VALUE");
    export[1].nlen = 12;
    export[1].addr = &return_value;
    export[1].leng = sizeofR(return_value);
    export[1].type = RFCTYPE_CHAR;
    
	export[2].name = NULL;

    table[0].name = NULL;
    /* end of declaration of meta data  */

    rc = RfcGetData( handle, import, NULL );
    
    if( rc != RFC_OK ) return rc;
    
    /* which error should be raised */
    i_method = atoiU (method);
    
    /* abort the connection */
    if (i_method == 1)
    {
        RfcAbort ( handle, msg006 );
    }
    
    /* division by zero */
    if (i_method == 4)
    {
        div_by_zero = 1/div_by_zero; 
    }
    
    /* call back RFCPING */
    if (i_method == 11)
    {
        rc = RfcCallReceive(handle, cU("RFCPING"), NULL, NULL, NULL, &exception);
        
        if (rc != RFC_OK)
        {
            RfcLastErrorEx(&error_info);

            return rc;
        }
    }
    
    /* call back RFC_RAISE_ERROR */
    if (i_method == 20)
    {
        cbexport[0].name = cU("METHOD");
        cbexport[0].nlen = 6;
        cbexport[0].addr = &parameter;
        cbexport[0].leng = sizeofR(cbmethod);
        cbexport[0].type = RFCTYPE_CHAR;
        cbexport[1].name = NULL;
        
        rc = RfcCallReceive(handle, cU("RFC_RAISE_ERROR"), cbexport, NULL, NULL, &exception);
        
        if (rc != RFC_OK)
        {
            RfcLastErrorEx(&error_info);

            return rc;
        }
        
    }   

    rc = RfcSendData( handle, export, table );

    return rc;
} /* rfc_raise_error */


/*
 * static functions which do the work
 */
static RFC_RC run ( RFC_HANDLE handle, rfc_char_t * command,
                    unsigned comsize, ITAB_H itab_h, int mode,
                    RFC_INT * status)
{
    int     i = 0;
    RFC_RC  rc = RFC_OK;

    for (i = comsize - 1;
         i > 0 && command[i] == cU(' ');
         i--)
    {
        command[i] = 0;
    }

    if (command[0] == 0)
    {
        RfcAbort( handle, cU("don't know what to do") );
    
        exit(1);
    }
    
    i = check_os_commands (command);

    if (i)
    {
        RfcAbort (handle, cU("Command not allowed"));

        exit (1);
    }

    rc = pipe_itab( handle, command, itab_h, mode, status );
    
    
    return rc;
} /* run */


static RFC_RC file ( RFC_HANDLE handle, rfc_char_t * filename,
                     unsigned size, ITAB_H itab_h, int mode)
{
    int    i;
    RFC_RC rc;

    for( i = size - 1;
	 i > 0 && filename[i] == cU(' ');
	 i-- )
    {
       filename[i] = 0;
    }
    if( filename[0] == 0 )
    {
      RfcAbort( handle, cU("no filename given") );
      exit(1);
    }
    rc = file_access( handle, filename, itab_h, mode );
    return rc;
} /* file */


static RFC_RC file_access ( RFC_HANDLE handle,
                            rfc_char_t * filename,
                            ITAB_H itab_h,
                            int    open_mode )
{
   FILE         * fil;
   rfc_char_t   * open_string;
   int            rc = 0;

   if( open_mode == FILE_READ ) 
    open_string = cU("r");
   else
    open_string = cU("w");

   fil = fopenU (filename, open_string);
   if (fil == NULL)
   {
      rfc_char_t * ptr = strerrorU ( errno );
      rfc_char_t   buffer[256];

      errorline = __LINE__;

      sprintfU (buffer, cU("%s (Line %d in rfcexec.c)"), ptr, errorline);
      
      RfcAbort (handle, buffer);
      
      exit(1);
   }

   if( itab_h != NULL )
   {
     switch( open_mode )
     {
       case FILE_READ :
        rc = file2itab( itab_h, fil );
        break;
       case FILE_WRITE :
        rc = itab2file( fil, itab_h );
        break;
     }

     if( rc )
     {
        rfc_char_t * ptr;
        rfc_char_t   buffer[256];
        if( rc == 2 ) 
         ptr = cU("not enough memory");
        else          
         ptr = strerrorU ( errno );

        sprintfU ( buffer, cU("%s (Line %d in rfcexec.c)"), ptr, errorline );
        RfcAbort( handle, buffer );
        exit(1);
     }
   }

   fclose (fil);
   
   
   return RFC_OK;
} /* file_access */



static int itab2file( FILE * fil, ITAB_H itab_h )
{
   int          l = ItLeng (itab_h)/sizeofR(SAP_UC);
   rfc_char_t   buffer[256];
   int          rc;
   int          i;


   for( i = 1; ; i++ )
   {
     int    j;
     
     rfc_char_t * ptr = (rfc_char_t *) ItGetLine( itab_h, i );

     if( ptr == NULL ) 
      break;

     j = l - 1;
     
     while (j >= 0 && ptr[j] == cU(' ')) 
      j--;
     
     j++;

     sprintfU (buffer, cU("%.*s\n"), j, ptr);
     
     rc = fputsU (buffer, fil);
     if (rc == EOF)
     {
       errorline = __LINE__;
   
       return 1;
     }
   }

   return 0;
} /* itab2file */




static int file2itab( ITAB_H itab_h, FILE * fil )
{
    int          l = ItLeng(itab_h)/sizeofR(SAP_UC);
    rfc_char_t   buffer[256];


    for(;;)
    {
        rfc_char_t  * p;
        int           read_l;


        p = fgetsU (buffer, sizeofU (buffer), fil);
        
        if (p == NULL)
        {
            if (ferror (fil))
            {
                errorline = __LINE__;

                return 1;
            }
            else
            {
                return 0;
            }
        }


        read_l = strlenU (p);
    
        if (p[read_l - 1] == cU('\n')) 
         read_l--;

        if (read_l > l) 
         read_l = l;


        p = (rfc_char_t *) ItAppLine (itab_h);
        if( p == NULL )
        {
            errorline = __LINE__;
        
            return  2;
        }

  
        if( read_l < l )
        {
            memcpyU (p, buffer, read_l);
            
            memsetU (p + read_l, cU(' '), l - read_l);
        }
        else
        {
            memcpyU (p, buffer, l);
        }
    }
} /* file2itab */



static RFC_RC pipe_itab ( RFC_HANDLE handle,
                          rfc_char_t * command,
                          ITAB_H itab_h,
                          int    mode,
                          RFC_INT *status )
{
#if defined(SAPonWINDOWS)
    /* there are no Pipes for MS-Windows, so just start the new process
       and no communication between parent and child process available
     */
    {
      UINT rc;

      rc = WinExec(command,1);   /* Start child and show normal */

      if(rc < 32)
      {
        rfc_char_t   buffer[256];
        sprintf(buffer, cU("Error %d by executing with WinExec (DOS command not allowed!)"), rc);
        RfcAbort( handle, buffer );
        exit(1);
      }
      else
      {
        if ( status != (RFC_INT *)0 )
          *status = (RFC_INT)RFC_OK;
        return RFC_OK;
      }
    }
#else /* ! SAPonWINDOWS */
#ifdef SAPonOS400
    /* there are no Pipes for OS/400, so just start the new process
     * and no communication between parent and child process available.
     *   Assumption: the called program is in the *LIBL of rfcexec
    */
    {
        pid_t pid;
        rfc_char_t *pathvar;
        rfc_char_t *argv[20];
        rfc_char_t *ptr, *cmd, *tmpPath, *tmpObj;
        int argc=0;
        int cmdlen, i;
        inheritance_t inherit;

        cmdlen = strlenU(command)+1;		/* create temp buffer for argv */
        if ((cmd = ptr = (rfc_char_t *) mallocU (cmdlen)) == NULL)
        {
           rfc_char_t* p = strerrorU(errno);
           rfc_char_t buffer[256];
           errorline = __LINE__;
           sprintfU(buffer, cU("malloc: %s (Line %d in rfcexec.c)"), ptr, errorline);
           RfcAbort(handle, buffer);
           return(RFC_FAILURE);
        }
    
        memcpyU(ptr, command, cmdlen);
    
        i=0;
        argc=0;
    
        while ((i<cmdlen) && (argc<19))	/* set up argv array */
        {
           argv[argc++] = ptr+i;
           for (; ((i<cmdlen) && (ptr[i] != cU(' ')) && (ptr[i] != 0)); i++)
               ;
           if (i<cmdlen)
           {
              ptr[i]=0;
              i++;
           }
        }
        argv[argc] = NULL;

        if ((pathvar = getenvU(cU("PATH"))) == NULL)
            putenvU(cU("PATH=%LIBL%:"));

        inherit.flags = 0;  /* initialize inheritance structure */
        inherit.pgroup = SPAWN_NEWPGROUP;

        GETenvironU;
		pid = -1;
#ifndef SAPwithUNICODE
        if (environU == NULL)		/* Init environ pointer             */
        {					/* If error don't use environ       */
            rfc_char_t *envvar_array[1] = { NULL };
            pid = spawn(cmd, 0, NULL, &inherit, argv, envvar_array);
        }
        else /* Pass child parent's set of env vars */
        {
            pid = spawn(cmd, 0, NULL, &inherit, argv, environU);
        }
#endif    
        if (pid == -1) {
            rfc_char_t* p = strerrorU(errno);
            rfc_char_t buffer[256];
            free(cmd);
            errorline = __LINE__;
            sprintfU(buffer, cU("spawn: %s (Line %d in rfcexec.c)"), ptr, errorline);
            RfcAbort(handle, buffer);
            return(RFC_FAILURE);
        }

        free(cmd);
        return ( RFC_OK );

    }/* end of SAPonOS400 */
#else
    {/* Windows 95/NT, UNIX, OS/2, Apple, VMS */
        FILE * fil;
        int    rc;
        rfc_char_t * open_string;

        if( mode == RUN_READ ) open_string = cU("r");
        else                   open_string = cU("w");

#ifdef SIGCHLD
#ifndef SAPonVMS

    if( mode == RUN_FREE )
    {
      signal( SIGCHLD, SIG_IGN );
    }
    else
    {
      signal( SIGCHLD, SIG_DFL );
    }

    /*
     * eat up zombies
     *
     * (wait for some childs, but don't wait)
     *
     */
    while( waitpid (-1, (int *)0, WNOHANG ) > 0 )
    {
      ;
    }

#endif
#endif

    fil = popenU (command, open_string);
    if( fil == NULL )
    {
      rfc_char_t * ptr = strerrorU ( errno );
      rfc_char_t   buffer[256];

      errorline = __LINE__;
      sprintfU ( buffer, cU("%s (Line %d in rfcexec.c)"), ptr, errorline );

#ifdef SAPonNT
      if (errno == 0)
        strcpyU (buffer, cU("Old version of R/3 or SAPGUI: Please start rfcexec via a bat/cmd-file."));
#endif

      RfcAbort( handle, buffer );
      exit(1);
    }

    if( itab_h != NULL )
    {
      if( mode == RUN_READ )
      {
        rc = file2itab( itab_h, fil );
      }
      else
      {
        rc = itab2file( fil, itab_h );
      }
      if( rc != 0 )
      {
        rfc_char_t * ptr;
        rfc_char_t   buffer[256];
        if( rc == 2 ) 
         ptr = cU("not enough memory");
        else          
         ptr = strerrorU ( errno );

        sprintfU (buffer, cU("%s (Line %d in rfcexec.c)"), ptr, errorline);
        RfcAbort (handle, buffer );
        exit(1);
      }
    }
    if( mode == RUN_FREE )
    {
      fclose( fil );

      if ( status != (RFC_INT *)0 )
          *status = (RFC_INT)0;

    }
    else
    {
      rc = pclose( fil );

      if( rc > 0 )
      {
        rfc_char_t buffer[512];
        if ( status != (RFC_INT *)0 )
         *status = (RFC_INT)rc;

        sprintfU (buffer, cU("%s terminated with exit code %d (Command not found ?)."), command, rc);

        RfcAbort (handle, buffer );
        exit(1);
      }
    }

    if ( status != (RFC_INT *)0 )
     *status = (RFC_INT)RFC_OK;

    return RFC_OK;
    }
#endif
#endif /* ! (SAPonWINDOWS || SAPonOS400) */
} /* pipe_itab */


/*
 *
 * function module documentation
 *
 */

/*
 * insert newline characters to start a new line
 */

static rfc_char_t * remote_pipe_docu( void )
{
   static rfc_char_t docu[] =
 iU("allows to start programs and to pipe an internal table into ")      NL_AR
 iU("stdin. The caller waits until the program ends.")                   NL_AR
 iU("")                                                                  NL_AR
 iU("IMPORTING")                                                         NL_AR
 iU("  COMMAND        C(256)")                                           NL_AR
 iU("    program name")                                                  NL_AR
 iU("  READ           C(1)                 (Default SPACE)")             NL_AR
 iU("    if READ is 'X' the internal table is filled from stdout")       NL_AR
 iU("    instead of being piped into stdin.")                            NL_AR
 iU("TABLES")                                                            NL_AR
 iU("  PIPEDATA       C(80)")                                            NL_AR
 iU("    internal table serves as input to the started program, if")     NL_AR
 iU("    READ is 'X' it is filled by the started program instead.")      NL_AR
   ;

   return docu;
} /* remote_pipe_docu */


static rfc_char_t * remote_file_docu( void )
{
   static rfc_char_t docu[] =
 iU("allows to read or write text files.")                               NL_AR
 iU("")                                                                  NL_AR
 iU("IMPORTING")                                                         NL_AR
 iU("  FILE           C(256)")                                           NL_AR
 iU("    file name")                                                     NL_AR
 iU("  WRITE          C(1)                 (Default SPACE)")             NL_AR
 iU("    if WRITE is 'X' the internal table is written to the file,")    NL_AR
 iU("    else it is filled from the file.")                              NL_AR
 iU("TABLES")                                                            NL_AR
 iU("  FILEDATA       C(80)")                                            NL_AR
 iU("    internal table serves as input to the file, if WRITE is 'X'")   NL_AR
 iU("    else it is filled from the file.")                              NL_AR
   ;

   return docu;
} /* remote_file_docu */


static rfc_char_t * mail_docu( void )
{
   static rfc_char_t docu[] =
 iU("allows to send mail via e-mail (example only).")                    NL_AR
 iU("")                                                                  NL_AR
 iU("IMPORTING")                                                         NL_AR
 iU("  USER           C(256)")                                           NL_AR
 iU("    user name (e-mail address).")                                   NL_AR
 iU("TABLES")                                                            NL_AR
 iU("  MAIL           C(80)")                                            NL_AR
 iU("    internal table containing mail.")                               NL_AR
   ;

   return docu;
} /* mail_docu */


static rfc_char_t * remote_exec_docu( void )
{
   static rfc_char_t docu[] =
 iU("allows to start programs and to pipe an internal table into ")      NL_AR
 iU("stdin without waiting for the program to end.")                     NL_AR
 iU("")                                                                  NL_AR
 iU("IMPORTING")                                                         NL_AR
 iU("  COMMAND        C(256)")                                           NL_AR
 iU("    program name.")                                                 NL_AR
 iU("TABLES")                                                            NL_AR
 iU("  PIPEDATA       C(80)")                                            NL_AR
 iU("    internal table serves as input to the started program.")        NL_AR
   ;

   return docu;
} /* remote_exec_docu */


/* Gets the docu to rfc_raise_error_docu*/
static rfc_char_t * rfc_raise_error_docu( void )
{
   static rfc_char_t docu[] =
 iU("!!! FOR INTERNAL USE ONLY !!!");

   return docu;
} /* rfc_raise_error_docu */


/*--------------------------------------------------------------------*/
/* Output help for starting program                                   */
/*--------------------------------------------------------------------*/
static void help(void)
{
  printfU (NL);
  printfU (cU( "Syntax for start and run in register mode:")                NL );
  printfU (cU( " ")                                                         NL );
  printfU (cU( "  rfcexec [options]")                                       NL );
  printfU (cU( " ")                                                         NL );
  printfU (cU( "  with")                                                    NL );
  printfU (cU( "  options = -D<destination with type 'R' in saprfc.ini>")   NL );
  printfU (cU( "          = -t             RFC-Trace on")                   NL );
  printfU (cU( "  or")                                                      NL );
  printfU (cU( "  options = -a<Program ID> e.g. <own host name>.rfcexec")   NL );
  printfU (cU( "          = -g<SAP gateway host name>        e.g.  hs0311") NL );
  printfU (cU( "          = -x<SAP gateway service>          e.g. sapgw00") NL );
  printfU (cU( "          = -t             RFC-Trace on")                   NL );
  printfU (cU( " ")                                                         NL );
  return;
}


static RFC_RC install_security_attributes (RFC_HANDLE handle)
{
    RFC_RC          rfc_rc  = RFC_OK;
    FILE          * fil     = NULL;
    SAP_UC          fname[] = iU("rfcexec.sec");
    SAP_UC          full_file_name[1024];
    SAP_UC        * path_to_file = NULL;
    SAP_UC          str_buff[sizeofU(PERMISSIONS_ATTRIBUTES)];

    memsetU (full_file_name, cU('\0'), sizeofU (full_file_name));

    /* open the configuration file */
    path_to_file = getenvU (cU("RFCEXEC_SEC_FILE_PATH"));

    /* 
     * we had a bug on SUN-Solaris so environment 
     * variable name longer that 20 chars has 
     * not been accepted by C-Shell
     */
    if (path_to_file == NULL)
    {
        path_to_file = getenvU (cU("RFCEXEC_SEC_PATH"));
    }

    if (path_to_file != NULL)
    {
	    #if defined(SAPonOS400) || defined(SAPwithPASE400)
		if (   strlenU (path_to_file) > 10
			&& strncmpU (path_to_file, cU("/QSYS.LIB/"), 10 ) == 0
		   ) /* in this case environment variable must contain path and filename */
			strcpyU (full_file_name, path_to_file);
		else
		#endif
			sprintfU (full_file_name, cU("%s%c%s"), path_to_file, os_separator, fname);
    }
    else
    {
      sprintfU (full_file_name, cU("%s"), fname);
	  #if defined(SAPonOS400) || defined(SAPwithPASE400)
		fil = fopenU (full_file_name, cU("r"));
		if (fil == NULL)
			strcpyU (full_file_name, cU("../../SYS/exe/run/KERNELLIB/SEC.FILE/RFCEXEC.MBR") );
		else 
			fclose(fil);
	  #endif
    }


    fil = fopenU (full_file_name, cU("r"));

    if (fil == NULL)
    {
        rfc_char_t  buffer[256];

        sprintfU (buffer, cU("Could not open file %s"), full_file_name);
        
        RfcAbort (handle, buffer);
      
        exit(1);
    }


    /* create the internal list for user attributes */
    sec_itab = ItCreate (cU("SecTable"), sizeofR(PERMISSIONS_ATTRIBUTES), 0, 0 );

    if (sec_itab == ITAB_NULL)
    {
        fclose (fil);

        RfcAbort (handle, cU("Could not create SecTable"));

        exit (1);
    }
    

    /* read the file line by line and fill the internal list */
    while (!feof(fil))
    {
        SAP_UC                 * str = NULL,
                               * ptr = NULL;
        PERMISSIONS_ATTRIBUTES * sec_attr_ptr = NULL;


        memsetR (&sec_attr, 0, sizeofR (sec_attr));

        if (fgetsU (str_buff, sizeofU(str_buff), fil) == NULL)
        {
            if (feof(fil))
            {
                break;
            }

            fclose (fil);

            ItDelete (sec_itab);

            RfcAbort (handle, cU("Could not read the rfcexec.sec file"));

            exit (1);
        }
        
        /* ignore all unneccessary symbols */
        ptr = str_buff;

        switch (*ptr)
        {
            case cU('\0'):
            case cU('\n'):
            case cU('\r'):
                continue;

            default  : 
                break;
        }

        /* ignore last CR and last new line if necessary */
        while ((str_buff[strlenU (str_buff) -1] == cU('\n')) ||
               (str_buff[strlenU (str_buff) -1] == cU('\r')))
        {
            str_buff[strlenU(str_buff)-1] = cU('\0');
        }

    

        /* read user entry*/
        str = search_sec_entry (str_buff, cU("/U/"), 
                                sec_attr.user, (sizeofU(sec_attr.user) - 1));
        if (str == NULL)
        {
            fclose (fil);

            ItDelete (sec_itab);

            RfcAbort (handle, cU("missing entry or invalid format for /U/"));

            exit (1);
        }

        /* read the function list */
        str = search_sec_entry (str_buff, cU("/F/"), 
                                sec_attr.function_list, (sizeofU(sec_attr.function_list) - 1));

        if (str == NULL)
        {
            fclose (fil);

            ItDelete (sec_itab);

            RfcAbort (handle, cU("missing entry or invalid format for /F/"));

            exit (1);
        }

        /* read connection type*/
        str = search_sec_entry (str_buff, cU("/T/"), 
                                sec_attr.partner_type, (sizeofU(sec_attr.partner_type) - 1));

        if (str == NULL)
        {
            fclose (fil);

            ItDelete (sec_itab);

            RfcAbort (handle, cU("missing entry or invalid format for /T/"));

            exit (1);
        }

        /* read snc name*/
        str = search_sec_entry (str_buff, cU("/S/"), 
                                sec_attr.snc_partner_name, (sizeofU(sec_attr.snc_partner_name) - 1));

        if (str == NULL)
        {
            fclose (fil);

            ItDelete (sec_itab);

            RfcAbort (handle, cU("missing entry or invalid format for /S/"));

            exit (1);
        }

        /* read client name*/
        str = search_sec_entry (str_buff, cU("/C/"), 
                                sec_attr.client, (sizeofU(sec_attr.client) - 1));

        if (str == NULL)
        {
            fclose (fil);

            ItDelete (sec_itab);

            RfcAbort (handle, cU("missing entry or invalid format for /C/"));

            exit (1);
        }


        /* read os-commands negative list */
        str = search_sec_entry (str_buff, cU("/O/"), 
                                sec_attr.os_commands, (sizeofU(sec_attr.os_commands) - 1));

        if (str == NULL)
        {
            fclose (fil);

            ItDelete (sec_itab);

            RfcAbort (handle, cU("missing entry or invalid format for /O/"));

            exit (1);
        }

        /* create new entry in the list */
        sec_attr_ptr = (PERMISSIONS_ATTRIBUTES * ) ItAppLine (sec_itab);

        if (sec_attr_ptr == NULL)
        {
            fclose (fil);

            ItDelete (sec_itab);

            RfcAbort (handle, cU("Could not append a row to the sec table"));

            exit (1);
        }

        memcpyR (sec_attr_ptr, &sec_attr, sizeofR (PERMISSIONS_ATTRIBUTES));
    }
        
    if (fil != NULL)
    {
        fclose (fil);
    }


    /* Actual install the logon handler */
    rfc_rc = RfcInstallExternalLogonHandler (handle, rfcexec_logon_check);

    if (rfc_rc != RFC_OK)
    {
        RfcAbort (handle, cU("Could not install logon handler"));
        
        exit (1);
    }

    return rfc_rc;
}


/* interprete the content of the current line */
static rfc_char_t * search_sec_entry (const rfc_char_t * source, 
                                      const rfc_char_t * ident,
                                      rfc_char_t       * target,
                                      unsigned           tsize)
{
    rfc_char_t    * str_begin = NULL,
                  * str_end   = NULL;
    unsigned        to_copy   = 0;


    /* detrminate user filed */
    str_begin = strstrU (source, ident);

    if (str_begin == NULL)
    {
        return NULL;
    }

    str_begin += 3;

    /* find terminator slash */
    str_end = strstrU (str_begin, cU("/"));

    if (str_end == NULL)
    {
        /* wrong format */
        return NULL;
    }

    /* min construct */
    to_copy = ((tsize < (str_end-str_begin)) ? tsize : (str_end-str_begin));

    memcpyU (target, str_begin, to_copy);

   
    return target;
}

static unsigned check_user (rfc_char_t * sec_user, rfc_char_t * logon_user)
{
    if (*sec_user == cU('*'))
    {
        /* all users are allowed */
        return 1;
    }

    if (memcmpU (sec_user, logon_user, sizeofU(RFC_USER_ID_LN_CHAR)) == 0)
    {
        return 1;
    }


    return 0;
}

static unsigned check_os_commands (rfc_char_t * command)
{
    rfc_char_t * ptr_begin = sec_attr.os_commands,
               * ptr_end    = NULL;

    if (*ptr_begin == cU(' '))
    {
        /* all functions are allowed */
        return 0;
    }

    while (ptr_begin != NULL)
    {
        rfc_char_t      tmp[2048];
        unsigned        length = 0;

        ptr_end = strstrU (ptr_begin, cU(","));
        
        if (ptr_end != NULL)
        {
            length = ptr_end - ptr_begin;
        }
        else
        {
            length = strlenU (ptr_begin);
        }
        
        memsetR (tmp, 0, sizeofR(tmp));

        memcpyU (tmp, ptr_begin, length);

        if (strstrU (command, tmp)!= NULL)
        {
            return 1; /* command found */
        }

        ptr_begin = ptr_end;

        if (ptr_begin != NULL)
        {
            ptr_begin++;
        }
    }

    return 0;
}

static unsigned check_function (rfc_char_t * function_list, rfc_char_t * function)
{
    rfc_char_t * ptr_begin = function_list,
               * ptr_end    = NULL;

    if (*function_list == cU('*'))
    {
        /* all functions are allowed */
        return 1;
    }

    while ((ptr_begin != NULL) && (ptr_begin = strstrU (ptr_begin, function)) != NULL)
    {
        unsigned length = 0;

        ptr_end = strstrU (ptr_begin, cU(","));

        if (ptr_end != NULL)
        {
            length = ptr_end - ptr_begin;
        }
        else
        {
            length = strlenU (ptr_begin);
        }
            
        if (memcmpU (function, ptr_begin, length) == 0)
        {
            return 1;
        }

        ptr_begin = ptr_end;

        if (ptr_begin!= NULL)
        {
            ptr_begin++;
        }
    }

    /* could not find the function in the list */
    return 0;
}

static unsigned check_type (rfc_char_t * type_list, rfc_char_t * logon_type)
{
    if (*type_list == cU('*'))
    {
        /* all functions are allowed */
        return 1;
    }

   if (strstrU (type_list, logon_type) != NULL)
   {
       return 1;
   }

    return 0;
}


static unsigned check_snc_name (rfc_char_t * sec_snc_name, rfc_char_t * logon_snc_name)
{
    if (snc_mode == RFC_SNC_MODE_OFF)
    {
        return 1;
    }


    if (*sec_snc_name == cU('*'))
    {
        return 1;
    }

    if (memcmpU (sec_snc_name, logon_snc_name, strlenU(sec_snc_name)) == 0)
    {
        return 1;
    }


    return 0;
}


static unsigned check_client (rfc_char_t * sec_client, rfc_char_t * logon_client)
{
    if (*sec_client == cU('*'))
    {
        return 1;
    }


    if (memcmpU (sec_client, logon_client, 3) == 0)
    {
        return 1;
    }

    return 0;
}

/* the logon handler */
static RFC_EXTERNAL_LOGON_RC DLL_CALL_BACK_FUNCTION 
            rfcexec_logon_check  (RFC_HANDLE                handle,
                                  RFC_EXTERNAL_LOGON_CNTL * logon_cntl, 
                                  rfc_char_t              * message)
{
    RFC_ATTRIBUTES           rfc_attr;
    PERMISSIONS_ATTRIBUTES * p_sec_attr = NULL;
    RFC_RC                   rfc_rc = RFC_OK;
    int                      i = 0;
    unsigned                 length = 0;
    rfc_char_t               partner_type[1+1];
    rfc_char_t               snc_partner_name[2048];
    

    memsetR (&rfc_attr, 0, sizeofR(rfc_attr));

    memsetU (snc_partner_name, cU(' '), sizeofU (snc_partner_name));

    memsetR (&sec_attr, 0, sizeofR (sec_attr));


    i = RfcGetAttributes (handle, &rfc_attr);

    if (i)
    {
        strcpyU (message, cU("Security: RfcGetAttributes failed"));

        return RFC_EXTERNAL_LOGON_FAILED;
    }

    partner_type[0] = rfc_attr.partner_type;
    partner_type[1] = cU('\0');

    
    rfc_rc = RfcSncMode (handle, &snc_mode);
    
    if (rfc_rc != RFC_OK)
    {
        sprintfU  (message, cU("Could not check snc mode(%u)"), rfc_rc);

        return RFC_EXTERNAL_LOGON_FAILED;
    }


    if (snc_mode == RFC_SNC_MODE_ON)
    {
        rfc_rc = RfcSncPartnerName (handle,
                                    snc_partner_name,
                                    sizeofU (snc_partner_name));

        if (rfc_rc != RFC_OK)
        {
            sprintfU  (message, cU("Could not read snc name (%u)"), rfc_rc);

            return RFC_EXTERNAL_LOGON_FAILED;
        }
    }


    if (sec_itab == ITAB_NULL)
    {
        strcpyU (message, cU("Security table invalid"));

        return RFC_EXTERNAL_LOGON_FAILED;
    }
    
    
    length = ItFill (sec_itab);
    
    /* search the stored list */
    for (i=1; i<=length; i++)
    {
        unsigned user_ok = 0;

        p_sec_attr = (PERMISSIONS_ATTRIBUTES *) ItGetLine (sec_itab, i);

        if (p_sec_attr == NULL)
        {
            sprintfU  (message, cU("Could not read security table (%d)"), i);

            return RFC_EXTERNAL_LOGON_FAILED;
        }


        if (check_user (p_sec_attr->user, logon_cntl->user) &&
            check_function (p_sec_attr->function_list, logon_cntl->function_name)&&
            check_type (p_sec_attr->partner_type, partner_type) &&
            check_snc_name (p_sec_attr->snc_partner_name, snc_partner_name) &&
            check_client (p_sec_attr->client, logon_cntl->client)
           )
        {
            /* found matched entry  */

            /* store it as current */
            memcpyR (&sec_attr, p_sec_attr, sizeofR(sec_attr));

            return RFC_EXTERNAL_LOGON_OK;
        }
    }


    /* there is no valid entry */
    strcpyU (message, cU("No autority for this connection or user"));

    return RFC_EXTERNAL_LOGON_FAILED;
}
