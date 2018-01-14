/* @(#) $Id: rfcsi.h $ SAP */
/*
 *   RFCSI
 *
 *
 * File generated from SAP Data Dictionary
 * by //bas/BIN/src/krn/runt/saphfile.c#27
 * on Tue Dec 05 15:15:53 2000
 *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * !!          NEVER CHANGE THIS FILE MANUALLY         !!
 * !! use                                              !!
 * !! saphfile rfcsi                                   !!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 */

#ifndef RFCSI_H
#define RFCSI_H

/* Time stamp */
#define SAP_RFCSI_TS                   "19980630155750"

/* ID */
#define SAP_RFCSI_ID                   30155750L

typedef struct RFCSI
{
		SAP_CHAR               rfcproto [3];
#define RFCSI_RFCPROTO_LN              CCQ_CAST(intU)     3
#define RFCSI_RFCPROTO_LENR            CCQ_CAST(intR)(    3 * SAP_UC_LN)
#define RFCSI_RFCPROTO_TYP                                0
#define RFCSI_RFCPROTO_RTYP                               0

		SAP_CHAR               rfcchartyp [4];
#define RFCSI_RFCCHARTYP_LN            CCQ_CAST(intU)     4
#define RFCSI_RFCCHARTYP_LENR          CCQ_CAST(intR)(    4 * SAP_UC_LN)
#define RFCSI_RFCCHARTYP_TYP                              0
#define RFCSI_RFCCHARTYP_RTYP                             0

		SAP_CHAR               rfcinttyp [3];
#define RFCSI_RFCINTTYP_LN             CCQ_CAST(intU)     3
#define RFCSI_RFCINTTYP_LENR           CCQ_CAST(intR)(    3 * SAP_UC_LN)
#define RFCSI_RFCINTTYP_TYP                               0
#define RFCSI_RFCINTTYP_RTYP                              0

		SAP_CHAR               rfcflotyp [3];
#define RFCSI_RFCFLOTYP_LN             CCQ_CAST(intU)     3
#define RFCSI_RFCFLOTYP_LENR           CCQ_CAST(intR)(    3 * SAP_UC_LN)
#define RFCSI_RFCFLOTYP_TYP                               0
#define RFCSI_RFCFLOTYP_RTYP                              0

		SAP_CHAR               rfcdest [32];
#define RFCSI_RFCDEST_LN               CCQ_CAST(intU)    32
#define RFCSI_RFCDEST_LENR             CCQ_CAST(intR)(   32 * SAP_UC_LN)
#define RFCSI_RFCDEST_TYP                                 0
#define RFCSI_RFCDEST_RTYP                                0

		SAP_CHAR               rfchost [8];
#define RFCSI_RFCHOST_LN               CCQ_CAST(intU)     8
#define RFCSI_RFCHOST_LENR             CCQ_CAST(intR)(    8 * SAP_UC_LN)
#define RFCSI_RFCHOST_TYP                                 0
#define RFCSI_RFCHOST_RTYP                                0

		SAP_CHAR               rfcsysid [8];
#define RFCSI_RFCSYSID_LN              CCQ_CAST(intU)     8
#define RFCSI_RFCSYSID_LENR            CCQ_CAST(intR)(    8 * SAP_UC_LN)
#define RFCSI_RFCSYSID_TYP                                0
#define RFCSI_RFCSYSID_RTYP                               0

		SAP_CHAR               rfcdatabs [8];
#define RFCSI_RFCDATABS_LN             CCQ_CAST(intU)     8
#define RFCSI_RFCDATABS_LENR           CCQ_CAST(intR)(    8 * SAP_UC_LN)
#define RFCSI_RFCDATABS_TYP                               0
#define RFCSI_RFCDATABS_RTYP                              0

		SAP_CHAR               rfcdbhost [32];
#define RFCSI_RFCDBHOST_LN             CCQ_CAST(intU)    32
#define RFCSI_RFCDBHOST_LENR           CCQ_CAST(intR)(   32 * SAP_UC_LN)
#define RFCSI_RFCDBHOST_TYP                               0
#define RFCSI_RFCDBHOST_RTYP                              0

		SAP_CHAR               rfcdbsys [10];
#define RFCSI_RFCDBSYS_LN              CCQ_CAST(intU)    10
#define RFCSI_RFCDBSYS_LENR            CCQ_CAST(intR)(   10 * SAP_UC_LN)
#define RFCSI_RFCDBSYS_TYP                                0
#define RFCSI_RFCDBSYS_RTYP                               0

		SAP_CHAR               rfcsaprl [4];
#define RFCSI_RFCSAPRL_LN              CCQ_CAST(intU)     4
#define RFCSI_RFCSAPRL_LENR            CCQ_CAST(intR)(    4 * SAP_UC_LN)
#define RFCSI_RFCSAPRL_TYP                                0
#define RFCSI_RFCSAPRL_RTYP                               0

		SAP_CHAR               rfcmach [5];
#define RFCSI_RFCMACH_LN               CCQ_CAST(intU)     5
#define RFCSI_RFCMACH_LENR             CCQ_CAST(intR)(    5 * SAP_UC_LN)
#define RFCSI_RFCMACH_TYP                                 0
#define RFCSI_RFCMACH_RTYP                                0

		SAP_CHAR               rfcopsys [10];
#define RFCSI_RFCOPSYS_LN              CCQ_CAST(intU)    10
#define RFCSI_RFCOPSYS_LENR            CCQ_CAST(intR)(   10 * SAP_UC_LN)
#define RFCSI_RFCOPSYS_TYP                                0
#define RFCSI_RFCOPSYS_RTYP                               0

		SAP_CHAR               rfctzone [6];
#define RFCSI_RFCTZONE_LN              CCQ_CAST(intU)     6
#define RFCSI_RFCTZONE_LENR            CCQ_CAST(intR)(    6 * SAP_UC_LN)
#define RFCSI_RFCTZONE_TYP                                0
#define RFCSI_RFCTZONE_RTYP                               0

		SAP_CHAR               rfcdayst [1];
#define RFCSI_RFCDAYST_LN              CCQ_CAST(intU)     1
#define RFCSI_RFCDAYST_LENR            CCQ_CAST(intR)(    1 * SAP_UC_LN)
#define RFCSI_RFCDAYST_TYP                                0
#define RFCSI_RFCDAYST_RTYP                               0

		SAP_CHAR               rfcipaddr [15];
#define RFCSI_RFCIPADDR_LN             CCQ_CAST(intU)    15
#define RFCSI_RFCIPADDR_LENR           CCQ_CAST(intR)(   15 * SAP_UC_LN)
#define RFCSI_RFCIPADDR_TYP                               0
#define RFCSI_RFCIPADDR_RTYP                              0

		SAP_CHAR               rfckernrl [4];
#define RFCSI_RFCKERNRL_LN             CCQ_CAST(intU)     4
#define RFCSI_RFCKERNRL_LENR           CCQ_CAST(intR)(    4 * SAP_UC_LN)
#define RFCSI_RFCKERNRL_TYP                               0
#define RFCSI_RFCKERNRL_RTYP                              0

		SAP_CHAR               rfchost2 [32];
#define RFCSI_RFCHOST2_LN              CCQ_CAST(intU)    32
#define RFCSI_RFCHOST2_LENR            CCQ_CAST(intR)(   32 * SAP_UC_LN)
#define RFCSI_RFCHOST2_TYP                                0
#define RFCSI_RFCHOST2_RTYP                               0

		SAP_CHAR               rfcsi_resv [12];
#define RFCSI_RFCSI_RESV_LN            CCQ_CAST(intU)    12
#define RFCSI_RFCSI_RESV_LENR          CCQ_CAST(intR)(   12 * SAP_UC_LN)
#define RFCSI_RFCSI_RESV_TYP                              0
#define RFCSI_RFCSI_RESV_RTYP                             0

}
RFCSI;


#define RFCSI_RFCPROTO_OFF             CCQ_CAST(intR)     0
#if   defined(SAP_UC_is_1B)
#define RFCSI_RFCCHARTYP_OFF           CCQ_CAST(intR)     3
#define RFCSI_RFCINTTYP_OFF            CCQ_CAST(intR)     7
#define RFCSI_RFCFLOTYP_OFF            CCQ_CAST(intR)    10
#define RFCSI_RFCDEST_OFF              CCQ_CAST(intR)    13
#define RFCSI_RFCHOST_OFF              CCQ_CAST(intR)    45
#define RFCSI_RFCSYSID_OFF             CCQ_CAST(intR)    53
#define RFCSI_RFCDATABS_OFF            CCQ_CAST(intR)    61
#define RFCSI_RFCDBHOST_OFF            CCQ_CAST(intR)    69
#define RFCSI_RFCDBSYS_OFF             CCQ_CAST(intR)   101
#define RFCSI_RFCSAPRL_OFF             CCQ_CAST(intR)   111
#define RFCSI_RFCMACH_OFF              CCQ_CAST(intR)   115
#define RFCSI_RFCOPSYS_OFF             CCQ_CAST(intR)   120
#define RFCSI_RFCTZONE_OFF             CCQ_CAST(intR)   130
#define RFCSI_RFCDAYST_OFF             CCQ_CAST(intR)   136
#define RFCSI_RFCIPADDR_OFF            CCQ_CAST(intR)   137
#define RFCSI_RFCKERNRL_OFF            CCQ_CAST(intR)   152
#define RFCSI_RFCHOST2_OFF             CCQ_CAST(intR)   156
#define RFCSI_RFCSI_RESV_OFF           CCQ_CAST(intR)   188
#elif defined(SAP_UC_is_2B)
#define RFCSI_RFCCHARTYP_OFF           CCQ_CAST(intR)     6
#define RFCSI_RFCINTTYP_OFF            CCQ_CAST(intR)    14
#define RFCSI_RFCFLOTYP_OFF            CCQ_CAST(intR)    20
#define RFCSI_RFCDEST_OFF              CCQ_CAST(intR)    26
#define RFCSI_RFCHOST_OFF              CCQ_CAST(intR)    90
#define RFCSI_RFCSYSID_OFF             CCQ_CAST(intR)   106
#define RFCSI_RFCDATABS_OFF            CCQ_CAST(intR)   122
#define RFCSI_RFCDBHOST_OFF            CCQ_CAST(intR)   138
#define RFCSI_RFCDBSYS_OFF             CCQ_CAST(intR)   202
#define RFCSI_RFCSAPRL_OFF             CCQ_CAST(intR)   222
#define RFCSI_RFCMACH_OFF              CCQ_CAST(intR)   230
#define RFCSI_RFCOPSYS_OFF             CCQ_CAST(intR)   240
#define RFCSI_RFCTZONE_OFF             CCQ_CAST(intR)   260
#define RFCSI_RFCDAYST_OFF             CCQ_CAST(intR)   272
#define RFCSI_RFCIPADDR_OFF            CCQ_CAST(intR)   274
#define RFCSI_RFCKERNRL_OFF            CCQ_CAST(intR)   304
#define RFCSI_RFCHOST2_OFF             CCQ_CAST(intR)   312
#define RFCSI_RFCSI_RESV_OFF           CCQ_CAST(intR)   376
#endif /* -/+ UNICODE */

/* Warning : C structure (UNICODE) may be larger than defined in dictionary. */

/* Warning : Structure may need 2-byte alignment (UNICODE). */


/* Size of key in bytes. A key has no alignment at end. */
#if   defined(SAP_UC_is_1B)
#define RFCSIKEYLENR                   CCQ_CAST(intR)     0
#elif defined(SAP_UC_is_2B)
#define RFCSIKEYLENR                   CCQ_CAST(intR)     0
#endif /* -/+ UNICODE */

/* Size (according to Data Dictionary) */
#if   defined(SAP_UC_is_1B)
#define SAP_RFCSI_LN                   CCQ_CAST(intR)   200
#elif defined(SAP_UC_is_2B)
#define SAP_RFCSI_LN                   CCQ_CAST(intR)   400
#endif /* -/+ UNICODE */

#define RFCSILN                        sizeofR( RFCSI )

#endif  /* RFCSI_H */

/*
 *   RFCSI
 *
 *
 * File generated from SAP Data Dictionary
 * by //bas/BIN/src/krn/runt/saphfile.c#27
 * on Tue Dec 05 15:15:53 2000
 *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * !!          NEVER CHANGE THIS FILE MANUALLY         !!
 * !! use                                              !!
 * !! saphfile rfcsi                                   !!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 */
