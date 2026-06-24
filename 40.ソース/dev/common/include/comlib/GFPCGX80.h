/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCGX80                                    */
/*        FUNCTION          ････ 共通モジュール                              */
/*                               エラー出力ログ編集出力モジュール            */
/*                                                                           */
/*        AUTHER            ････ HAS T.Fukunaga                              */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2024-10-01                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  T.Fukunaga 2024/10/01 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
#ifndef _GFPCGX80_H
#define _GFPCGX80_H

#define DEF_COM_ERL_ARG1_OPEN     1            /* ファイルIO種別  OPEN          */
#define DEF_COM_ERL_ARG1_WRITE    2            /* ファイルIO種別  WRITE         */
#define DEF_COM_ERL_ARG1_CLOSE    3            /* ファイルIO種別  CLOSE         */
#define DEF_COM_ERL_EMS_MSGTTKB   "E"          /* メッセージ通知区分            */
#define DEF_COM_ERL_EMS_INN_ECD1  "SCAA001"    /* ファイルIOエラー              */
#define DEF_COM_ERL_EMS_INN_ECD2  "SCAA002"    /* ファイルオープンエラー        */

/* typedef定義 */
#pragma fieldalign shared2 __COM_ERL_arg_1
typedef struct __COM_ERL_arg_1
{
    short           file_io_type;
    long            io_timer;
    short           data_len;
    char           *data_area;
} COM_ERL_arg_1_def;

#pragma fieldalign shared2 __COM_ERL_arg_2
typedef struct __COM_ERL_arg_2
{
    char            file_name[48];
    short           file_no;
} COM_ERL_arg_2_def;

#pragma fieldalign shared2 __COM_ERL_arg_3
typedef struct __COM_ERL_arg_3
{
    char            srv_logical_id[8];
    char            lcn[15];
    char            connect[24];
} COM_ERL_arg_3_def;


/* プロトタイプ宣言 */
short COM_ERL(COM_ERL_arg_1_def *, COM_ERL_arg_2_def *,oggz1in_def *,COM_ERL_arg_3_def * ,char *);

#endif

