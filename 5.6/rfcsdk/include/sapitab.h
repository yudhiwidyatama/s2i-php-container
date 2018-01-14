/*@(#) $Id: //bas/640_REL/src/include/sapitab.h#1 $ SAP */

#ifndef SAPITAB_H
#define SAPITAB_H

/*---------------------------------------------------------------------
 * @doc
 *
 * @module ABAP/4 internal tables API |
 *
 * These functions allow processing of ABAP/4 internal tables in 
 * C.
 *
 * ABAP/4 internal tables follow the model of a relational database
 * table.
 *
 * ABAP/4 internal tables consist of identical rows.
 * If created, a table is yet empty.
 * In ABAP/4 you can fill rows into a table by the statements
 * 'Insert' or 'Append'. You can access a row by 'Read Table' and
 * you can delete a row by 'Delete'. You can free a Table by 'Free Table'
 * and you can retrieve information about tables by 'Describe'.
 *
 * These language constructs correspond to the following C routines.
 *
 * @flag <f ItCreate>    | creates a new internal table.
 * @flag <f ItAppLine>   | appends a line (row) to an internal table.
 * @flag <f ItInsLine>   | inserts a line into the given position.
 * @flag <f ItDelLine>   | deletes a line.
 * @flag <f ItGetLine>   | reads a line.
 * @flag <f ItGupLine>   | reads a line for update.
 * @flag <f ItFree>      | resets an internal table to  initial state.
 * @flag <f ItDelete>    | deletes (frees) a complete table.
 * @flag <f ItFill>      | returns the number of lines in a table.
 * @flag <f ItLeng>      | returns the width of a table, i.e. the
 *                         size of a row  of the table.
 *
 * @ex Creating and filling a new internal table |
 *
 *   const int myTableSize = 200;
 *   ITAB_H    handle;
 *   void *    ptr;
 *
 *   handle = ItCreate( "MyTable", myTableSize, 0, 0 );
 *   if ( handle == ITAB_NULL )
 *   {
 *       ....error
 *   }
 *
 *   do
 *   {
 *      ptr = ItAppLine( handle );
 *
 *      if( ptr != NULL )
 *      {
 *         memcpy( ptr, ...., myTableSize );
 *      }
 *   }
 *   while( ptr != NULL );
 *
 *
 *---------------------------------------------------------------------
 */
#ifdef __cplusplus
extern "C"
{
#endif

/*
 * For Windows 3.x _WINDOWS has to be defined, as it is done
 * by MS Visual C++
 */

#ifndef SAPTYPE_H

#ifdef _WIN32
#  ifndef SAPonNT
#    define SAPonNT
#  endif
#endif  /* _WIN32       */

#ifdef _WINDOWS
#  ifndef _WIN32
#    ifndef SAPonWINDOWS
#      define SAPonWINDOWS
#    endif
#  endif  /* _WIN16     */
#endif  /* _WINDOWS     */

#endif  /* SAPTYPE_H    */

#if !defined(SAPTYPE_H)
/*
 * Non-SAP RFC Compile: include the SAP types
 */
#include "sapucx.h"
#endif


/* @type SAP_API |
 * The token SAP_API contains plattform dependent keywords,
 * which are necessary to allow dynamic linking of these function
 * from various environments. On Windows for example SAP_API is
 * __extern __loadds __pascal __far.
 */

#if !defined(SAP_FAR)|| !defined(SAP_PASCAL)|| !defined(SAP_EXPORT)|| !defined(SAP_LOADDS) || !defined(SAP_STDCALL)
#   undef SAP_FAR
#   undef SAP_PASCAL
#   undef SAP_EXPORT
#   undef SAP_LOADDS
#   undef SAP_STDCALL
#   if defined(SAPonWINDOWS)
#       define SAP_FAR __far
#       define SAP_PASCAL __pascal
#       define SAP_EXPORT __export
#       define SAP_LOADDS __loadds
#       define SAP_STDCALL
#   elif defined(SAPonNT)
#       define SAP_FAR
#       define SAP_PASCAL
#       define SAP_EXPORT
#       define SAP_LOADDS
#       define SAP_STDCALL _stdcall
#   else
#       define SAP_FAR
#       define SAP_PASCAL
#       define SAP_EXPORT
#       define SAP_LOADDS
#       define SAP_STDCALL
#   endif
#endif

#if !defined(SAP_API)
#   if defined(OS2)
#       define _SYSTEM _System
#       define SAP_API _SYSTEM
#   else
#       define _SYSTEM
#       define SAP_API SAP_FAR SAP_LOADDS SAP_PASCAL SAP_STDCALL
#   endif
#endif /* SAP_API */

#if !defined(WIN_DLL_EXPORT_FLAGS)
#   define WIN_DLL_EXPORT_FLAGS SAP_API
#endif /* WIN_DLL_EXPORT_FLAGS */


#if !defined(DLL_CALL_BACK_FUNCTION)
#   if defined(SAPonWINDOWS)
#       define DLL_CALL_BACK_FUNCTION SAP_FAR SAP_PASCAL
#   elif defined(SAPonNT)
#       define DLL_CALL_BACK_FUNCTION SAP_STDCALL
#   elif defined(OS2)
#       define DLL_CALL_BACK_FUNCTION _SYSTEM
#   else
#       define DLL_CALL_BACK_FUNCTION
#   endif
#   if defined(OS2)
#      define DLL_CALL_BACK_FUNCTION_PTR * _SYSTEM
#   else
#      define DLL_CALL_BACK_FUNCTION_PTR DLL_CALL_BACK_FUNCTION *
#   endif
#endif /* DLL_CALL_BACK_FUNCTION */


#ifndef RFC_H
/* @type ITAB_H | Handle for an internal table returned by
 *                <f ItCreate> and used to access a table by
 *                all the other routines of the internal table API.
 *
 * @flag ITAB_NULL | invalid table handle.
 */
typedef void * ITAB_H;
#endif /* RFC_H */

/* Invalid handle */
#define ITAB_NULL ((void *)NULL)

/* --------------------------------------------------------------------
 * Platzhalter fuer Tabellenheader in LOAD: soll fuer alle
 * Maschinentypen ausreichen !
 * Achtung, bei Aenderung muessen angepasst werden:
 * - abtypes.h (hat 256 angegeben)
 * - Programm RSITABHD
 * - Domaene  SYTABH
 * --------------------------------------------------------------------
 */
#define TABHL_SIZE 256

typedef union
{
  SAP_RAW dummy_char[TABHL_SIZE];
  double  dummy_double;
  void *  dummy_addr;
}
TABHL;


#ifndef ABTSV_H
/* -----------------------------------------------------------------
 * Supported log operations
 * ---------------------------------------------------------------*/
typedef enum
{
   /* General entries */
   LOG_OBJECT_CONFIRM,     /* Confirmation of a share request */
   LOG_OBJECT_REJECT,      /* 'share request' rejected */

   /* Playback notification */
   LOG_PLAYBACK_START,     /* Start Playback-Operation        */
   LOG_PLAYBACK_END,       /* Ende  Playback-Operation        */

   /* for internal tables */
   LOG_ITAB_APPEND,        /* Usual internal table operations */
   LOG_ITAB_DELETE,
   LOG_ITAB_INSERT,
   LOG_ITAB_UPDATE,
   LOG_ITAB_FREE,
   LOG_ITAB_DISCARD,       /* Discard share link */
   LOG_ITAB_REBUILD,       /* Transport completely, no further logs */
   LOG_ITAB_CORRECTION,    /* Same as UPDATE, but to repair
			    * invalid log entries
			    */
   LOG_ITAB_POP            /* Table popped from stack */

   /*---------------------------------------------------------------
    * C A U T I O N : New log operations may only be added to
    * the end of the list (to ensure compatibility with earlier
    * versions of the RFC Delta Manager)
    * ------------------------------------------------------------*/
}
LOG_OPERATION;

/* Playback function for the Delta Manager */
typedef void (* _SYSTEM PLAYBACK_F)( void *        pb_handle,
			     LOG_OPERATION operation,
			     unsigned      line );
#endif

/* There are several RFC libraries:
 * - ASCII/MBCS libraries, 
 * - UNICODE librarries, 
 * - Libraries that use ASCII for C strings on EBCDC platforms
 *
 * in order to prevent usage of the wrong libraries,
 * the entry points are mangeled with U or A extension
 * so that they cannot be linked accidentially to non-Unicode executables.
 */

#ifndef RFC_API_ENTRY
#  if defined(SAPwithUNICODE)

    /* we have UNICODE */
#   define RFC_API_ENTRY(fn)   fn##U

#  elif defined(SAPonOS390) && defined(SAPwithCHAR_ASCII)

    /* OS/390 with ASCII char */
#   define RFC_API_ENTRY(fn)   fn##A

#  else    
    /* default native char type */
#   define RFC_API_ENTRY(fn)   fn

#  endif
#endif

#define ItCreate	                 RFC_API_ENTRY(ItCreate)
#define ItDelete	                 RFC_API_ENTRY(ItDelete)
#define ItGetLine	                 RFC_API_ENTRY(ItGetLine)
#define ItPutLine	                 RFC_API_ENTRY(ItPutLine)
#define ItCpyLine	                 RFC_API_ENTRY(ItCpyLine)
#define ItInsLine	                 RFC_API_ENTRY(ItInsLine)
#define ItAppLine	                 RFC_API_ENTRY(ItAppLine)
#define ItDelLine	                 RFC_API_ENTRY(ItDelLine)
#define ItGupLine	                 RFC_API_ENTRY(ItGupLine)
#define ItFree	                     RFC_API_ENTRY(ItFree)
#define ItFill	                     RFC_API_ENTRY(ItFill)
#define ItLeng	                     RFC_API_ENTRY(ItLeng)
#define ItRegTable	                 RFC_API_ENTRY(ItRegTable)
#define ItUserData	                 RFC_API_ENTRY(ItUserData)

         

/* Table functions */
/*---------------------------------------------------------------------
 * @func 
 * 
 * Creates a new internal table.
 *
 * @rdesc
 * returns handle to a table control structure (<t ITAB_H>),
 * if successful, or
 * ITAB_NULL, if there is not enough memory available.
 *
 * @comm
 * The token <t SAP_API> contains plattform dependent keywords.
 *--------------------------------------------------------------------*/
ITAB_H SAP_API ItCreate ( 
 SAP_UC * name,  /* @parm name of the internal table. 
                  * null terminated string, is used to 
                  * identify the table when tracing.
                  */
 unsigned leng,  /* @parm line length for the internal table */
 unsigned occu,  /* @parm occurs value for internal table,
                  * amount of lines being allocated when
                  * first appending a line to the table.
                  */
 unsigned memo   /* @parm only use heap memory for allocating
                  * table lines (internal use, should be 0).
                  * Not used any more.
                  */
                        );


/*---------------------------------------------------------------------
 * @func
 *
 * Deletes (frees) an internal table.
 * The memory allocated for the table is freed. The table handle
 * is not valid then and must be used any more.
 * 
 * Corresponding ABAP/4 operation : Free ... 
 * 
 * @rdesc
 * returns 0, if successful.
 *
 * @xref <f ItFree>.
 *--------------------------------------------------------------------*/
int SAP_API ItDelete (
  ITAB_H itab  /* @parm handle of an internal table */
                     );


/*---------------------------------------------------------------------
 * @func
 *
 * Get a row from an internal table, and return its address.
 * The contents of the row must not be modified.
 *
 * Corresponding ABAP/4 operation : Read Table ... Index ...
 *
 * @rdesc
 * returns pointer to a table row, if successful.
 * Otherwise returns NULL (table row does not exist).
 *
 * @xref <f ItGupLine>, <f ItCpyLine>, <f ItPutLine>.
 *--------------------------------------------------------------------*/
 void * SAP_API ItGetLine(
 ITAB_H itab,    /*  @parm  handle of an internal table */
 unsigned line   /*  @parm number of the desired row,
                  *  row numbers start with 1 similar than
                  *  Sy-Tabix in ABAP/4.
                  */
                        );

/*---------------------------------------------------------------------
 * @func
 *
 * The contents of the row is copied into the
 * destination area.
 * The length of the copy operation
 * is implicitly assumed as the size of one row.
 * Memory must be supplied by the caller, no check is done.
 * The row must not be updated.
 *
 * Corresponding ABAP/4 operation : Read Table ... Index ...
 *
 * @rdesc
 * 0 O.K., -1 if line does not exist.
 *
 * @xref <f ItGetLine>, <f ItPutLine>.
 *--------------------------------------------------------------------*/
int SAP_API ItCpyLine(
  ITAB_H itab,    /* @parm handle of an internal table */
  unsigned line,  /* @parm number of the desired row,
                  *  row numbers start with 1 similar than
                  *  Sy-Tabix in ABAP/4. */
  void * dest     /* @parm pointer to destination area */
                     );

/*---------------------------------------------------------------------
 * @func
 *
 * Copies the source onto line 'line' of an internal table.
 *
 * Corresponding ABAP/4 operation : Modify ... Index ...
 *
 * @rdesc
 * 0 O.K., -1 error.
 *
 * @xref <f ItGetLine>, <f ItGupLine>, <f ItCpyLine>.
 *--------------------------------------------------------------------*/
int SAP_API ItPutLine(
  ITAB_H itab,    /* @parm handle of an internal table. */
  unsigned line,  /* @parm line number onto which 'src' is copied. */
  void * src      /* @parm pointer to source, source is
                   * copied in the length of a  table row .
                   */
                     );


/*---------------------------------------------------------------------
 * @func
 *
 * Inserts a new row into an internal table at the given position.
 * The new line is initialized with 0.
 *
 * Corresponding ABAP/4 operation : Insert ... Index ...
 *
 * @rdesc
 * Pointer to the inserted table row, if successful,
 * otherwise NULL (no space available).
 *
 * @xref <f ItAppLine>
 *--------------------------------------------------------------------*/
void * SAP_API ItInsLine(
  ITAB_H itab,  /* @parm handle of an internal table. */
  unsigned line /* @parm row number before which the line
                 * is to be inserted.
                 */
                        );


/*---------------------------------------------------------------------
 * @func
 * Appends a new row to the end of an internal table.
 * The new line is initialized with 0.
 *
 * Corresponding ABAP/4 operation : Append ...
 *
 * @rdesc
 * Pointer to the appended table row, if successful,
 * otherwise NULL (no space available).
 *
 * @xref <f ItInsLine>.
 *--------------------------------------------------------------------*/
void * SAP_API ItAppLine(
  ITAB_H itab  /* @parm handle of an internal table. */
                        );

/*---------------------------------------------------------------------
 * @func
 *
 * Delete a row from an internal table.
 *
 * Corresponding ABAP/4 operation : Delete ...
 *
 * @rdesc
 * returns
 * @flag 0        | O.K.
 * @flag <gt> 0   | row does not exist.
 * @flag <lt> 0   | other error (no space available for index, etc.)
 *--------------------------------------------------------------------*/
int SAP_API ItDelLine(
  ITAB_H itab,  /* @parm handle of an internal table. */
  unsigned line /* @parm row number of the line to be deleted. */
                     );



/*---------------------------------------------------------------------
 * @func
 *
 * Get a row from an internal table, and return its address for
 * update.
 * The contents of the row is to be modified.
 *
 * Corresponding ABAP/4 operation : Read Table ... Index ..., Modify ...
 *
 *
 * @rdesc
 * returns pointer to a table row, if successful.
 * Otherwise returns NULL (table row does not exist).
 *
 * @xref <f ItGetLine>, <f ItCpyLine>, <f ItPutLine>.
 *--------------------------------------------------------------------*/
void * SAP_API ItGupLine(
 ITAB_H itab,    /*  @parm  handle of an internal table. */
 unsigned line   /*  @parm number of the desired row,
                  *  row numbers start with 1 similar than
                  *  Sy-Tabix in ABAP/4.
                  */
                        );

/*---------------------------------------------------------------------
 * @func
 *
 * Frees an internal table. The storage of the internal table is
 * freed. The table contains no more rows. The table handle remains
 * valid, however.
 *
 * Corresponding ABAP/4 operation : Free ...
 *
 * @rdesc
 * returns 0 if successful.
 *
 * @xref <f ItCreate>, <f ItDelete>.
 *--------------------------------------------------------------------*/
int SAP_API ItFree(
    ITAB_H itab     /* @parm handle of an internal table. */
                   );

/*---------------------------------------------------------------------
 * @func
 * Get the number of rows in an internal table.
 * @rdesc
 * returns number of rows in the internal table.
 *--------------------------------------------------------------------*/
unsigned SAP_API ItFill   (
    ITAB_H itab     /* @parm handle of an internal table. */
                          );


/*---------------------------------------------------------------------
 * @func
 * Get the size of the rows of an internal table.
 * @rdesc
 * returns length of a row of the internal table.
 *--------------------------------------------------------------------*/
/*SAPUNICODEOK_RAWLEN*/
unsigned SAP_API ItLeng   (
    ITAB_H itab     /* @parm handle of an internal table. */
                          );


int SAP_API ItRegTable( ITAB_H itab, PLAYBACK_F ufkt,
				     void * udat );
/*---------------------------------------------------------------------
 * I t R e g T a b l e
 * Register a user function and an address for additional data
 *---------------------------------------------------------------------
 * INPUT:  itab: Pointer to a table control structure
 *               (returned from ItCreate)
 *         ufkt: Function to be registered
 *         udat: Pointer to user data
 * OUTPUT: itab: Table control structure
 *---------------------------------------------------------------------
 * RETURN: 0 if successful; a negative value otherwise.
 *--------------------------------------------------------------------*/


int SAP_API ItUserData	( ITAB_H itab, void ** udat_p );
/*---------------------------------------------------------------------
 * I t U s e r D a t a
 * Get a pointer to the additional user data for an internal table.
 *---------------------------------------------------------------------
 * INPUT:  itab: Pointer to a table control structure
 *               (returned from ItCreate)
 * OUTPUT: *udat_p: Pointer to the user data
 *---------------------------------------------------------------------
 * RETURN: 0 if successful; a negative value otherwise.
 *--------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* SAPITAB_H */

