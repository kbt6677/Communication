/*****************************************************************************/
/*****                    <<     HEADER PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVX60                                    */
/*        FUNCTION          ････ 電文中継(GET)                               */
/*                                                                           */
/*        AUTHER            ････ HAS S.Makino                                */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2024-11-07                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  S.Makino   2024/11/07 (xxxxx)新規作成                               */
/****************************************************************************/
/*   ｺﾝﾊﾟｲﾙｵﾌﾟｼｮﾝ                                                           */
/****************************************************************************/

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <stdlib.h> nolist
#include <stdarg.h> nolist
#include <string.h> nolist
#include <tal.h> nolist
#include <netdb.h>
#include <cextdecs.h> nolist

/* USER HEADER     */
#include "GFPOGGZ3_encode.h"
#include "GFPOGGZ4_traceout.h"
#include "common.h"
#include "file.h"
#include "ems.h"
#include "errcd.h"
#include "ipc.h"
#include "GFPCGX40.h"
#include "GFPCGX50.h"
#include "GFPCGX80.h"
#include "GFPCGXA0.h"
#include "GFPCGXB0.h"
#include "GFPCGXC0.h"
#include "GFPCGXD0.h"
#include "GFPCGXG0.h"
#include "vproc.h"

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
/* 共通DEFINE */
#define QGET_RET_OK                         0
#define QGET_RET_ERR                        9
#define QGET_FILE_CLOSED                    -1

#define QGET_NORMAL_TERMINATION             0
#define QGET_ABNORMAL_TERMINATION           1

#define DEF_ABORT_DELAY_10MSECOND           "ABORT-DELAY-10MSECOND"     /* AT時DELAYタイマ */

/* フラグ */
#define QGET_ON                             1
#define QGET_OFF                            0

#define QGET_FILEIO_OPEN                    "OPEN"
#define QGET_FILEIO_READ                    "READ"
#define QGET_FILEIO_CLOSE                   "CLOSE"
#define QGET_FILE_LOGI_NAME                 "GQNWQ"
#define QGET_NON_MLT_NUM                    "0000"

#define DEF_IF_ID_DEFAULT                   "}}}}}"     /* インタフェース識別.指定なしALL"}" */
#define DEF_STATION_ID_DEFAULT              "}}}}}}"    /* ステーション識別.指定なしALL"}"   */

/****************************************************************************/
/*   内部変数定義                                                           */
/****************************************************************************/

/********** 構造体 ***********/
/* 自プロセス情報 */
typedef struct __myinfo_def
{
    short   recv_fno;                                   /* $RECEIVEファイル番号                   */
    char    recv_buf;                                   /* $RECEIVEバッファーポインタ             */
    char    psend_serverclass_name[16];                 /* PATHSEND先サーバクラス名               */
    char    psend_pathmon_name[16];                     /* PATHSEND先PATHMON名                    */
    short   end_flg;                                    /* 終了フラグ                             */
    short   read_end_flg;                               /* 読み込み終了フラグ                     */
    char    trc_time_begin[21];                         /* トレース開始時間                       */
    char    trc_time_end[21];                           /* トレース終了時間                       */
    char    site_id;                                    /* サイト識別                             */
    char    network_id;                                 /* N/W識別                                */
    char    nw_kubun[2];                                /* N/W区分                                */
    char    group_id[5];                                /* グループ識別                           */
    char    serverclass_kind[8];                        /* サーバクラス種類                       */
    char    serverclass_no[4];                          /* サーバクラス論理番号                   */
    char    serverclass_name[12];                       /* サーバクラス論理名                     */
    long    file_timer;                                 /* ファイルI/Oタイマー                    */
    long    send_timer;                                 /* PATHSENDタイマー                       */
    short   send_retry;                                 /* PATHSENDリトライ回数                   */
    char    msg_serverclass_name[16];                   /* メッセージ出力サーバクラス名           */
    char    msg_pathmon_name[16];                       /* メッセージ出力PATHMON名                */
    long    expiry_second;                              /* 送信期限切れタイマー                   */
    long    qfile_read_wait_timer;                      /* キューファイル読み込み完了待ちタイマー */
    char    module_id[8];                               /* モジュールID                           */
    char    gfphi_asn_fname[12];                        /* 物理名情報ファイル.アサイン名          */
    char    gfphi_fname[47];                            /* 物理名情報ファイル.物理ファイル名      */
    short   gfphi_fnum;                                 /* 物理名情報ファイル.ファイル番号        */
    char    gfnwi_fname[48];                            /* NW情報ファイル物理ファイル名           */
    short   gfnwi_fnum;                                 /* NW情報ファイルファイル番号             */
    char    erl_fname[48];                              /* エラーログ出力ファイル.物理ファイル名  */
    short   erl_fnum;                                   /* エラーログ出力ファイル.ファイル番号    */
    long    abort_delay_timer;                          /* ABORT TRAN DELAYタイマー               */
} myinfo_def;

/* キューファイル情報 */
typedef struct __qfileinfo_def
{
    char    fname[48];      /* 物理ファイル名               */
    short   fno;            /* ファイル論理番号             */
    short   fnum;           /* ファイル番号                 */
    short   ferr;           /* エラーコード                 */
    char    rec_key[50];    /* レコードキー                 */
} qfileinfo_def;

/* I/O完了情報 */
typedef struct __iocomp_def
{
    short fno;
    long  addr;
    short len;
    long  tag;
    short ferr;
    zsys_ddl_receiveinformation_def  recv_info;
} iocomp_def;

/* EMS出力サブルーチン用 */
oggz1in_def     cg010in;                /* メッセージ出力   */

/* トレース共通モジュール用 */
//short TRACEOUT( char* );

typedef struct __lk_iocoreq_arg_3
{
   char                            prog_id[8];
   char                            file_id[8];
   char                            file_name[47];
   char                            file_io_type[8];
} lk_iocoreq_arg_3_def;
#define lk_iocoreq_arg_3_def_Size 71


typedef struct __lk_iocoreq_arg_6
{
   short                           guardian_errcode;
   char                            err_proc[30];
   char                            file_name[47];
   char                            future_use;
   short                           rec_len;
   char                            rec_area[20000];
} lk_iocoreq_arg_6_def;
#define lk_iocoreq_arg_6_def_Size 20082

typedef struct __g_trc_def {
    lk_zac2001r_arg_1_def   recv;
} g_trc_def;

g_trc_def       g_trc;                                      /* トレース出力モジュール用     */

//char trace_buf[sizeof(lk_zac2001i_arg_1_def)];
//lk_zac2001i_arg_1_def *ipc_trace = (lk_zac2001i_arg_1_def *) &trace_buf;
//lk_zac2001r_arg_1_def *rcv_trace = (lk_zac2001r_arg_1_def *) &trace_buf;
//lk_zac2001p_arg_1_def *scs_trace = (lk_zac2001p_arg_1_def *) &trace_buf;
char  EXMYSRVCLSNAME[16];
char  EXMYPROCNAME[6];
short EXTRACEMODE;
char  EXTRACEFILENAME[47];
short EXTRACEFILENO;

#define FUNC_INIT                       '0'       /* 初期処理   */
#define FUNC_MAIN                       '1'       /* メイン処理 */
#define FUNC_OUTPUT                     '1'       /* 出力処理   */
#define FUNC_END                        '2'       /* 終了処理   */
#define FUNC_EXTENDED_OUTPUT            '9'       /* 拡張出力   */

/* エラー出力ログ用 */
COM_ERL_arg_1_def   timeup_COM_ERL_arg_1;
COM_ERL_arg_2_def   timeup_COM_ERL_arg_2;
COM_ERL_arg_3_def   timeup_COM_ERL_arg_3;

/* 関数のプロトタイプ宣言 */
void   QGET_init(void);
void   QGET_get_param(void);
void   QGET_get_config(char *);
void   QGET_main (void);
void   QGET_send_reply(char *, short, short);
void   QGET_req_send(void);
void   QGET_qfile_open(void);
void   QGET_qfile_read(void);
void   QGET_qfile_close(void);
void   QGET_message_output(short, char, char*, char*, ...);
void   QGET_finish(void);
void   QGET_abend(void);
//void   HEX2CHAR(unsigned char *hex_p, char *terget_p,short s_len);
