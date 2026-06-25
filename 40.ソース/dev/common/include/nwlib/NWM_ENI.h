/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSX20                                    */
/*        FUNCTION          ････ 共通モジュール・ヘッダー                    */
/*        AUTHER            ････ HAS s.kimura                                */
/*        PROGRAM-CALL      ････ EMS出力モジュール                           */
/*                               IOモジュール                                */
/*        WRITTEN-DATE      ････ 2024-10-01                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  T.Fukunaga 2024/10/01 (J0680)新規作成                               */
/*  1.1  S.Kimura   2025/01/29 (J0680)VisaNet更新                            */
/*                                                                           */
/*****************************************************************************/

#ifndef _GFPCSX20_H
#define _GFPCSX20_H

#define  DEF_NWM_ENI_RTN_OK              0               /* RETURN CODE OK            */
#define  DEF_NWM_ENI_RTN_NG             -1               /* RETURN CODE NG            */
#define  DEF_NWM_ENI_FILENO_INIT        -1               /* ファイル番号(未設定)      */
#define  DEF_NWM_ENC_SPACE              ' '              /* SPACE SET DATA            */

#define  DEF_NWM_ENI_OWN_NODE_ONLY      0                /* 自ノードのみ              */
#define  DEF_NWM_ENI_OTHER_NODE_INC     1                /* 他ノードも必要            */


/* typedef定義 */
#pragma fieldalign shared2 __NWM_ENI_arg_1
typedef struct __NWM_ENI_arg_1
{
   char     file_id[8];
   char     file_name[48];
   short    file_no;
   long     io_timer;
} NWM_ENI_arg_1_def;

#pragma fieldalign shared2 __NWM_ENI_arg_2
typedef struct __NWM_ENI_arg_2
{
   char     file_id[8];
   char     file_name[48];
   short    file_no;
   long     io_timer;
} NWM_ENI_arg_2_def;

#pragma fieldalign shared2 __NWM_ENI_arg_3
typedef struct __NWM_ENI_arg_3
{
   char     domain_name[16];
   char     server_name[16];
   long     pathsend_timer;
   short    pathsend_retry_cnt;
} NWM_ENI_arg_3_def;

#pragma fieldalign shared2 __NWM_ENI_arg_4
typedef struct __NWM_ENI_arg_4
{
   char     site_id;
   char     nw_id;
   char     grp_id[5];
   char     if_id[5];
   char     station_id[6];
} NWM_ENI_arg_4_def;

#pragma fieldalign shared2 __ems_info_add
typedef struct __ems_info_add
{
   char     srv_logical_id[8];
   char     lcn[15];
   char     connect[24];
} ems_info_add;

/* プロトタイプ宣言 */
short NWM_ENI(char
            , NWM_ENI_arg_1_def *
            , NWM_ENI_arg_2_def *
            , NWM_ENI_arg_2_def *
            , NWM_ENI_arg_3_def *
            , NWM_ENI_arg_4_def *
            , char *
            , oggz1in_def*
            , ems_info_add*);

#endif

