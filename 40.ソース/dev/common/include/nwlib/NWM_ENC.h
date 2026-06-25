/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSJ10                                    */
/*        FUNCTION          ････ 共通モジュール・ヘッダー                    */
/*        AUTHER            ････ HAS T.Fukunaga                              */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2024-10-01                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  T.Fukunaga 2024/10/01 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
#ifndef _NWM_ENC_H
#define _NWM_ENC_H

#define  DEF_NWM_ENC_ARG1_ENC            1               /* FUNC ENC                       */
#define  DEF_NWM_ENC_ARG1_DEC            2               /* FUNC DEC                       */
#define  DEF_NWM_ENC_RTN_OK              0               /* RETURN CODE OK                 */
#define  DEF_NWM_ENC_RTN_OK_NONE         1               /* RETURN CODE OK NO CHENGE       */
#define  DEF_NWM_ENC_RTN_NG_AUTHORI      2               /* RETUUN CODE AUTHORI ERR        */
#define  DEF_NWM_ENC_RTN_NG_DIGITS       3               /* RETURN CODE CHK DIGITS ERR     */
#define  DEF_NWM_ENC_RTN_NG_ATALLA       4               /* RETURN CODE ATALLA ERR         */
#define  DEF_NWM_ENC_RTN_NG_IO           5               /* RETURN CODE IO ERR             */
#define  DEF_NWM_ENC_RTN_NG_PARAM        6               /* RETURN CODE PARAM ERR          */
#define  DEF_NWM_ENC_RTN_NG_KMAC         7               /* RETURN CODE GCKEY KMAC ERR     */
#define  DEF_NWM_ENC_RTN_NG_KC           8               /* RETURN CODE GCKEY KC ERR       */
#define  DEF_NWM_ENC_RTN_NG_DIGITS_KMAC  9               /* RETURN CODE CHK DIGITS KMAC ERR*/
#define  DEF_NWM_ENC_RTN_NG_DIGITS_KC   10               /* RETURN CODE CHK DIGITS KC ERR  */

/* typedef定義 */
#pragma fieldalign shared2 __NWM_ENC_arg_2
typedef struct __NWM_ENC_arg_2
{
   char     file_id[8];
   char     file_name[48];
   short    file_no;
   long     io_timer;
} NWM_ENC_arg_2_def;

#pragma fieldalign shared2 __NWM_ENC_arg_3
typedef struct __NWM_ENC_arg_3
{
   char     domain_name[16];
   char     server_name[16];
   long     pathsend_timer;
   short    pathsend_retry_cnt;
} NWM_ENC_arg_3_def;

#pragma fieldalign shared2 __NWM_ENC_arg_4
typedef struct __NWM_ENC_arg_4
{
   char     site_id;
   char     nw_id;
   char     grp_id[5];
   char     if_id[5];
   char     station_id[6];
} NWM_ENC_arg_4_def;

#pragma fieldalign shared2 __NWM_ENC_arg_5
typedef struct __NWM_ENC_arg_5
{
   char    *before_msg;
   short    before_len;
   char    *after_msg;
   short    after_len;
} NWM_ENC_arg_5_def;

/* プロトタイプ宣言 */
short NWM_ENC(short
            , NWM_ENC_arg_2_def *
            , NWM_ENC_arg_3_def *
            , NWM_ENC_arg_4_def *
            , NWM_ENC_arg_5_def *
            , char *);

#endif

