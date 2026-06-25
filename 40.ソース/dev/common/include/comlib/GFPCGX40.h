/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCGX40                                    */
/*        FUNCTION          ････ 共通モジュール・ヘッダー                    */
/*                               PATHSEND処理モジュール                      */
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
#ifndef _GFPCGX40_H
#define _GFPCGX40_H

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define  DEF_COM_PSD_ERR_INVALID_FILE   13
#define  DEF_COM_PSD_EMS_MSGTTKB        "E"
#define  DEF_COM_PSD_TRC_FILE_ID        "PATHSEND"
#define  DEF_COM_PSD_TRC_FILE_IO_WRITE  "WRITE"
#define  DEF_COM_PSD_TRC_FILE_IO_READ   "READ"
//#define DEF_NERR_PSEND_ERR_RE_OUT       "SCAA007"            /* Pathsendエラー(リトライアウト)             */
//#define DEF_NERR_PSEND_ERR_RE_OK        "SCAA005"            /* Pathsendエラー(リトライ可)                 */


/* typedef定義 */
#pragma fieldalign shared2 __COM_PSD_arg_1
typedef struct __COM_PSD_arg_1
{
    char    pathmon_name[16];
    char    serverclass_name[32];
    char    msg_buf[15000];
    short   req_send_len;
    short   receive_max_len;
    short   receive_len;
    long    send_timer_msec;
    short   retry_cnt;
} COM_PSD_arg_1_def;


#pragma fieldalign shared2 __COM_PSD_arg_2
typedef struct __COM_PSD_arg_2
{
    char prog_id[8];
} COM_PSD_arg_2_def;


#pragma fieldalign shared2 __COM_PSD_arg_3
typedef struct __COM_PSD_arg_3
{
    short guardian_errcode;
    short pathsend_errcode;
} COM_PSD_arg_3_def;

#pragma fieldalign shared2 __COM_PSD_arg_4
typedef struct __COM_PSD_arg_4
{
    char            srv_logical_id[16];
    char            lcn[15];
} COM_PSD_arg_4_def;

typedef struct __COM_PSD_zac2001p_arg_1
{
   char                            func_flg;
   struct
   {
      char                            prog_id[8];
      char                            file_id[8];
      char                            file_name[47];
      char                            file_io_type[8];
      char                            guardian_errcode[4];
      char                            shori_start_time[12];
      char                            shori_end_time[12];
   } trace_info;
   struct
   {
      char                            rec_len[5];
      char                            rec_area[27000];
   } data_info;
} COM_PSD_zac2001p_arg_1_def;
#define COM_PSD_zac2001p_arg_1_def_Size 27105

/* プロトタイプ宣言 */
short COM_PSD(COM_PSD_arg_1_def *, COM_PSD_arg_2_def *, COM_PSD_arg_3_def *, oggz1in_def *,COM_PSD_arg_4_def *);

/*ZAC20XX*/
short TRACEOUT(char *);

#endif
