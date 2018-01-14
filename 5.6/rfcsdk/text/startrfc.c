static char sccsid[] = "@(#) $Id: //bas/640_REL/src/krn/rfc/startrfc.c#5 $ SAP";


/*
 *   S  T  A  R  T  R  F  C
 *
 *   generic rfc client program allowing to do RFC calls
 *   from the command line
 *
 *
 *   (C) Copyright SAP AG 1993
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#if defined(SAPonOS400)
#define externC
#include "o4global.h"
#undef externC
#endif

 /*
 * include RFC API
 */
#include "saprfc.h"
#include "sapitab.h"

/*
 * local prototypes & declarations
 */

/* static funcitions cant be exported in Windows, so dont make rfc_error static */
void DLL_CALL_BACK_FUNCTION rfc_error (rfc_char_t * error_id);

static int check_use_saprfc_ini (rfc_char_t ** argv, 
                                 rfc_char_t *  dest,
                                 rfc_char_t *  client,
                                 rfc_char_t *  user,
                                 rfc_char_t *  pass,
                                 rfc_char_t *  lang);

static void tables_out (void);

static int parse_commandline (rfc_char_t ** argv,
			                  RFC_FUNCTIONNAME functionname,
			                  RFC_PARAMETER  * exporting,
			                  RFC_PARAMETER  * importing,
			                  RFC_TABLE      * tables);

static void parse_export (rfc_char_t * string, RFC_PARAMETER * parameter);

static void parse_tables (rfc_char_t * string, RFC_TABLE * table);

static void help (void);

static int itab2file     (FILE * file,   ITAB_H itab_h);

static int file2itab     (ITAB_H itab_h, FILE * file);

static rfc_char_t * dupstr (rfc_char_t * string );  /* strdup */


/* some limit for parameter to allow static allocation */
#define MAX_PARA 64


typedef struct _OUT
{
  FILE   * file;

  ITAB_H   itab;
} _OUT;


static ITAB_H out_itab;

/* static int    errorline; */

static rfc_char_t   buffer[4000];  /* maximum line size */

RFC_ERROR_INFO_EX     error_info;

rfc_char_t            dest[32],
                      client[4] = iU("000"),
                      user[13]  = iU("SAP*"),
                      pass[9]   = iU("PASS"),
                      lang[3]   = iU("EN");

/*
 *  main function for an RFC client program
 */

mainU (int argc, rfc_char_t ** argv)
{
   static RFC_OPTIONS               rfc_opt;
   static RFC_CONNOPT_CPIC          rfc_connopt_cpic;
   static RFC_CONNOPT_R3ONLY        rfc_connopt_r3only;
   RFC_FUNCTIONNAME                 functionname;
   static RFC_PARAMETER             exporting[MAX_PARA];
   static RFC_PARAMETER             importing[MAX_PARA];
   static RFC_TABLE                 tables[MAX_PARA];
   RFC_ENV                          new_env;
   RFC_HANDLE                       handle;
   RFC_RC                           rc;
   rfc_char_t *                     exception_ptr;
   int                              use_saprfc_ini = 0;
   rfc_char_t                       connect_param[1024];

   if (argc < 2)
   {
     help();
     return 1;
   }

#ifdef SAPonApple
   MacInit(&argc, &argv);
#endif

   /*
    * sign on defaults
    */
   rfc_opt.client   = cU("000");
   rfc_opt.user     = cU("SAP*");
   rfc_opt.language = cU("E");
   rfc_opt.password = cU("PASS");
   rfc_opt.trace    = 0;
   rfc_opt.mode     = RFC_MODE_CPIC;


   /*
    * read connection and sign on data from the command line
    */
   if (RfcFileArgv(&argc, &argv) == RFC_FAILURE)
	printfU (cU("Error handling reading or writing commandline arguments from/to file\n"));


   use_saprfc_ini = check_use_saprfc_ini (argv, dest, client, user, pass, lang);


   if (use_saprfc_ini == 0)
   {
        RfcConnArgv (argv, &rfc_opt, &rfc_connopt_cpic, &rfc_connopt_r3only);
   }


   /*
    * install error handler
    */
   new_env.allocate = NULL;
   new_env.errorhandler = rfc_error;
   RfcEnvironment (&new_env);

   /*
    * Create table of output tables
    */
   out_itab = ItCreate (cU("OUT"),          /* some name */
			            sizeofR (_OUT),     /* width of the table */
			            0, 0);             /* internal use,
                                             * can always be zero
                                             */
   if (out_itab == NULL)
   {
        fprintfU (stderr, cU("No memory\n"));
     
        return 1;
   }


   /*
    * look for function name and parameters,
    * read in tables from files
    */
   parse_commandline (argv, functionname, exporting, importing, tables);


   if (functionname[0] == cU('\0'))
   {
      help();
      return 1;
   }


   /* connect */
   if (use_saprfc_ini == 0)
   {
     handle = RfcOpen (&rfc_opt);

     if( handle == RFC_HANDLE_NULL )
     {
       rfc_error (cU("open"));
       return 1;
     }
   }
   else
   {
     sprintfU (connect_param,
               cU("DEST=%s CLIENT=%s USER=%s PASSWD=%s LANG=%s"),
               dest, client, user, pass, lang);

     handle = RfcOpenEx(connect_param, &error_info);

     if (handle == RFC_HANDLE_NULL)
     {
       fprintfU (stderr, cU("RFC Call/Exception: RfcOpenEx\n"));
       fprintfU (stderr, cU("Group       Error group %d\n"), error_info.group );
       fprintfU (stderr, cU("Key         %s\n"), error_info.key );
       fprintfU (stderr, cU("Message     %s\n"), error_info.message );
       
       return 1;
     }
   }


   /* issue the RFC call */
   rc = RfcCallReceive (handle,
                        functionname,
                        exporting,
                        importing,
                        tables,
                        &exception_ptr);


   /* check return code */
   switch (rc)
   {
      case RFC_OK:
        break;

      case RFC_EXCEPTION :
      case RFC_SYS_EXCEPTION :
        rfc_error( exception_ptr );
        break;
      
      default :
        rfc_error (cU("call/receive error"));
       break;
   }


   /* close the connection */
   RfcClose (handle);


   /* Return 1 if RFC error occurs */
   if (rc) 
    return 1;


   /* export received tables */
   tables_out();


   return 0;
}




/*
 * Check the command line for using with or without saprfc.ini
 */
static int check_use_saprfc_ini (rfc_char_t ** argv,
                                 rfc_char_t *  dest,
                                 rfc_char_t *  client,
                                 rfc_char_t *  user,
                                 rfc_char_t *  pass,
                                 rfc_char_t *  lang)
{
  rfc_char_t  * str;
  int           token_2;
  int           rc = 0;


  while (argv[0] != NULL)
  {

    if (argv[0][0] == cU('-'))
    {
        str = argv[0] + 2;
        
        if ( (argv[0][1] == 0) || (str[0] == 0))
        {
           str = argv[1];
           token_2 = 1;
        }
        else 
         token_2 = 0;


        /* option found */
        switch (argv[0][1])
        {
           case cU('D') :  /* Destination in saprfc.ini */
             if (str == NULL ) 
              return 1;
             strcpyU (dest, str);
             rc = 1;
             break;

           case cU('c') :  /* Client */
             if (str == NULL) 
              return 1;
             strcpyU (client, str);
             break;

           case cU('u') :  /* User */
             if (str == NULL ) 
              return 1;
             strcpyU (user, str);
             break;

           case cU('p') :  /* Password */
             if (str == NULL ) 
              return 1;
             strcpyU (pass, str);
             break;

           case cU('l') :  /* Language */
             if (str == NULL ) 
              return 1;
             strcpyU (lang, str);
             break;

           default  :
           token_2 = 0;
           break;
        }


        if (token_2) 
         argv += 2;
        else 
         argv++;
    }
    else
    {
      *argv++;
    }
  }


  return rc;
}



/*
 * parse the command line for function name and parameters
 */
static int parse_commandline (rfc_char_t ** argv,
                              RFC_FUNCTIONNAME functionname,
                              RFC_PARAMETER  * exporting,
                              RFC_PARAMETER  * importing,
                              RFC_TABLE      * tables)
{
  rfc_char_t ** argv_out = argv;
  rfc_char_t *  str;
  int           token_2;
  int           exp_count = 0,
                tab_count = 0;


  functionname[0] = cU('\0');
  importing = importing;   /* to avoid compiler warnings */


  while (argv[0] != NULL)
  {
    if( argv[0][0] == cU('-') )
    {
        str = argv[0] + 2;
        if ((argv[0][1] == 0) || (str[0] == 0))
        {
           str = argv[1];
           token_2 = 1;
        }
        else 
         token_2 = 0;


        /* option found */
        switch( argv[0][1] )
        {
           case cU('F') :  /* function name */
             if (str == NULL) 
              return 1;
             strncpyU (functionname, str,
		               sizeofU (RFC_FUNCTIONNAME) - 1);
             break;

           case cU('E') :  /* exporting parameter */
             if (str == NULL) 
              return 1;
             if (exp_count >= MAX_PARA) 
              return 2;
             parse_export (str, exporting + exp_count);
             exp_count++;
             break;

           case cU('T') :  /* table */
             if (str == NULL) 
              return 1;
             if (tab_count >= MAX_PARA) 
              return 2;
             parse_tables (str, tables + tab_count);
             tab_count++;
             break;

           default  :
            *argv_out++ = argv[0];
            token_2 = 0;
            break;
        }

	    if (token_2) 
         argv += 2;
	    else          
         argv++;
    }
    else
    {
      *argv_out++ = *argv++;
    }
  }


  *argv_out = NULL;


  return 0;
}




static void parse_export (rfc_char_t * string, RFC_PARAMETER * parameter)
{
    rfc_char_t * ptr = strchrU ( string, cU('='));
    rfc_char_t * name;
    rfc_char_t * value;

    /*
     * supported syntax :
     *
     * -E NAME=VALUE
     *
     */

    if( ptr == NULL )
    {
       fprintfU (stderr, cU("format error : %s\n"), string);
       
       help();
       
       exit(1);
    }

    *ptr++ = 0;
    name   = dupstr( string );
    value  = dupstr( ptr );
    
    if( name == NULL || value == NULL )
    {
       fprintfU ( stderr, cU("no memory.\n"));
       exit(1);
    }

    parameter[0].name = name;
    parameter[0].nlen = strlenU (name);
    parameter[0].addr = value;
    parameter[0].leng = strlenU(value)*sizeofR(SAP_UC);
    parameter[0].type = RFCTYPE_CHAR;


    return;
}


static void parse_tables (rfc_char_t * string, RFC_TABLE * table)
{
    int          t_size;
    rfc_char_t   str_b[256];
    rfc_char_t   * t_name;
    rfc_char_t   * t_io1;
    rfc_char_t   * t_io2;
    rfc_char_t   * t_io;
    rfc_char_t   * f_name;
    rfc_char_t   * name;
    rfc_char_t   * ptr;
    ITAB_H         itab_h;
    FILE         * file_r, 
                 * file_w;
    _OUT          * out;


    /*
     * supported syntax :
     *
     * -T NAME,123,r=/tmp/file1,w=/tmp/file2
     *
     */
    strncpyU (str_b, string , sizeofU(str_b) - 1);
    str_b[sizeofU( str_b ) - 1] = 0;

    t_name = strtokU (str_b, cU(","));


    ptr    = strtokU (NULL, cU(","));
    
    if( ptr == NULL ) 
     goto format_error;
    
    t_size = atoiU (ptr);
    
    ptr    = strtokU ( NULL, cU(","));
    
    if( ptr == NULL ) 
     goto format_error;
    

    t_io1  = ptr;
    
    t_io2  = strtokU (NULL, cU(","));

    if (t_name[0] == 0 || t_size <= 0 || t_io1[0] == 0)
    {
      goto format_error;
    }
    

    itab_h = ItCreate (t_name, t_size, 0 , 0);
    
    if( itab_h == NULL ) 
     goto no_memory;


    t_io = t_io1;
    

    name = dupstr( t_name );
    

    if( name == NULL ) 
     goto no_memory;


    table[0].name     = name;
    table[0].nlen     = strlenU ( name );
    table[0].ithandle = itab_h;
    table[0].leng     = t_size;
    table[0].type     = RFCTYPE_CHAR;


    while( t_io != NULL )
    {

      if( t_io[0] != 0 )
      {
        if( t_io[1] != cU('=')) 
         goto format_error;

        switch( t_io[0] )
        {
            case cU('r'):
                f_name = t_io + 2;
                if( strcmpU (f_name, cU("-") ) == 0 )
                 file_r = stdin;
                else
                 file_r = fopenU (f_name, cU("r"));
            
                if( file_r == NULL )
                {
                    perrorU (f_name);
                    exit (1);
                }
            
                file2itab( itab_h, file_r );
                break;

            case cU('w') :
                f_name = t_io + 2;
                if (strcmpU (f_name, cU("-")) == 0 )
                    file_w = stdout;
                else
                    file_w = fopenU (f_name, cU("w"));

                if( file_w == NULL )
                {
                    perrorU (f_name);
                    exit (1);
                }
                out = (_OUT*) ItAppLine( out_itab );
                if( out == NULL ) 
                 goto no_memory;
                out->file = file_w;
                out->itab = itab_h;
                break;

            default :
                goto format_error;
        }
      }
            if( t_io == t_io1 ) t_io = t_io2;
            else                t_io = NULL;

    }

    return;

format_error :
    fprintfU (stderr, cU("format error : %s\n"), string );
    help();
    exit(1);

no_memory :
      fprintfU (stderr, cU("no memory.\n"));
      exit(1);
}



static void tables_out( void )
{
   int i;

   for (i = 1; ;i++)
   {
     _OUT * out = (_OUT *) ItGetLine( out_itab, i );

     if (out == NULL) 
      break;
     
     itab2file( out->file, out->itab );
   }
   return;
}



void DLL_CALL_BACK_FUNCTION rfc_error (rfc_char_t * operation )
{
   fprintfU (stderr, cU("RFC Call/Exception: %s\n"), operation);

   RfcLastErrorEx(&error_info);

   fprintfU (stderr, cU("Group       Error group %d\n"), error_info.group );
   fprintfU (stderr, cU("Key         %s\n"), error_info.key );
   fprintfU (stderr, cU("Message     %s\n"), error_info.message );
   RfcClose (RFC_HANDLE_NULL );
   return;
}



static int itab2file (FILE * file, ITAB_H itab_h )
{
   int    l = ItLeng (itab_h)/sizeofR (SAP_UC);
   int    rc;
   int    i;


   for (i = 1; ; i++)
   {
     int    j;
   
     rfc_char_t * ptr = (rfc_char_t *) ItGetLine( itab_h, i );
     
     if (ptr == NULL) 
      break;

     j = l - 1;
     
     while (j >= 0 && ptr[j] == cU(' ')) 
      j--;

     j++;

     sprintfU (buffer, cU("%.*s\n"), j, ptr );

     rc = fputsU (buffer, file );
     
     if (rc == EOF)
     {
       /* errorline = __LINE__; */
       return 1;
     }
   }


   return 0;
}



static int file2itab (ITAB_H itab_h, FILE * file)
{
   int    l = ItLeng (itab_h)/sizeofR(SAP_UC);


   for(;;)
   {
     rfc_char_t * p;
     int    read_l;


     p = fgetsU (buffer, sizeofU(buffer), file );
     
     if (p == NULL)
     {
        if (ferror (file))
        {
           /* errorline = __LINE__; */
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
     

     if (p == NULL)
     {
        /* errorline = __LINE__; */
        return  2;
     }


     if (read_l < l)
     {
       memcpyU (p, buffer, read_l);
       memsetU (p + read_l, cU(' '), l - read_l);
     }
     else
     {
       memcpyU (p, buffer, l);
     }
   }


   return 0;
}



static void RfcConnArgvHelp ( FILE * file )
{
   fprintfU (file, 
             cU("RFC connection options:\n\n") );

   fprintfU (file, 
             cU("  -d <destination>      name of the RFC destination.\n")
             cU("           Necessary, if you are using a 'sideinfo' file.\n\n") );
   
   fprintfU ( file,
              cU("  -2                    SNA mode on.\n")
              cU("           You must set this if you want to connect to R/2.\n")
              cU("           All other conection data must be suppplied by a\n")
              cU("           sideinfo file.\n\n"));

   fprintfU ( file,
              cU("  -3                    R/3 mode on.\n")
              cU("           You must set this if you want to connect to R/3.\n")
              cU("           Specify the following options:\n\n"));

   fprintfU ( file,
              cU("      -h <hostname>         hostname of the R/3 target system.\n\n"));

   fprintfU ( file,
              cU("      -s <system number>    system number of the target system.\n")
              cU("           this determines the TCP/IP service to be used to connect\n")
              cU("           to the R/3 system. The default value is 0 and the default\n")
              cU("           service being used then is sapgw00.\n\n"));

#if ( ! defined(SAPonWINDOWS) ) && ( ! defined(SAPonOS400) )
   fprintfU ( file,
              cU("      -gui               start sapgui\n")
              cU("           to allow dynpro and graphics processing\n")
              cU("           (3.0C or later required on the target system).\n\n"));  
#endif                       
   fprintfU ( file, 
              cU("      Using an intermediate SAP gateway, specify:\n")
              cU("           -g <gateway host>\n")
              cU("           -x <gateway service>\n")
              cU("           (must not be used with -gui or -debug option).\n\n"));      

#ifndef SAPonWINDOWS
   fprintfU ( file,
              cU("  -balanced             load balancing mode.\n")
              cU("           Another way to connect to R/3, if the R/3 system is 3.0C\n")
              cU("           or later and workload balancing is active on that system.\n")
              cU("           Requests are automatically routed to the application server\n")
              cU("           having the best response times in the moment.\n")
              cU("           Specify the following options:\n\n"));  

   fprintfU ( file,
              cU("      -h <host name>     hostname of R/3's message server.\n\n"));

   fprintfU ( file,
              cU("      -s <system name>   name of the target system.\n")
              cU("           This determines the TCP/IP service to be used to connect\n")
              cU("           to the R/3 system. \n")
              cU("           The system name is a 3 letter word. If the system name\n")
              cU("           is XXX, the service being used is sapXXX.\n\n"));

   fprintfU ( file,
          cU("      -g <group name>    name of application server group.\n")
          cU("           The default is PUBLIC.\n\n"));
#endif

#if ( ! defined(SAPonWINDOWS) ) && ( ! defined(SAPonOS400) )
   fprintfU ( file,
              cU("      -gui               start sapgui\n")
              cU("           to allow dynpro and graphics processing.\n\n"));                   
#endif
   fprintfU ( file,
              cU("additional options:\n\n"));
   fprintfU ( file,
              cU("  -t              turn trace on.\n")
              cU("           all operations are written to the trace file 'dev_rfc'\n\n"));

#if ( ! defined(SAPonWINDOWS) ) && ( ! defined(SAPonOS400) )
   fprintfU ( file,
              cU("  -debug          turn ABAP/4 debugging mode on.\n")
              cU("           this can only be done, if sapgui is installed on the client\n")
              cU("           system and the target system has version 3.0C or later.\n\n"));
#endif

   fprintfU ( file,
              cU("Using the 'saprfc.ini'-file:\n\n"));

   fprintfU ( file,
              cU("  -D <destination>    name of the RFC destination in saprfc.ini\n")
              cU("           Instead of using the connection and additional options\n")
              cU("           defined above, you can also work with the 'saprfc.ini'-file\n")
              cU("           and all needed options must be defined in this file.\n")
              cU("           Using this feature, this program can run in an SNC\n")
              cU("           environment.\n\n"));

   fprintfU ( file, 
              cU("RFC logon data:\n\n"));

   fprintfU ( file, 
              cU("  -u <userid>      SAP userid.\n\n"));

   fprintfU ( file, 
              cU("  -p <password>    password.\n\n"));

   fprintfU ( file, 
              cU("  -c <client>      client.\n\n"));

   fprintfU ( file, 
              cU("  -l <language>    logon language.\n\n"));

   return;
}



static void help( void )
{
#define NL cU("\n")

   printfU (NL);
   printfU (cU("RFC command line interface")                              NL NL );
   printfU (cU("Syntax :")                                                NL );
   printfU (cU("         startrfc [connect options] <function options>")  NL NL );
   printfU (cU("with")                                                    NL );
   printfU (cU("function options =")                                      NL );
   printfU (cU("          -F <function module")                           NL );
   printfU (cU("          -E <parameter>=<value>")                        NL );
   printfU (cU("          -T <table name>,<width>,[r=<file>][,w=<file>]") NL );
   printfU (cU("             where <file> is a path name to read from (r)") NL );
   printfU (cU("             or write to (w) the internal table. ")         NL );
   printfU (cU("             If <file> is -, stdin or stdout is used.")   NL NL );

   RfcConnArgvHelp (stdout);

   printfU (cU( "further options:")                                       NL NL );
   printfU (cU( "         -i <input file for argv>       ")               NL );
   printfU (cU( "         -o <output file for argv>      ")               NL );
   printfU (cU("          -?                             this text")      NL NL );
}




static rfc_char_t * dupstr (rfc_char_t * string)
{
   int    l    = strlenU (string);
   
   
   rfc_char_t * ptr  = mallocU (l + 1);

   if( ptr != NULL )
   {
      strcpyU (ptr, string);
   }


   return ptr;
}
