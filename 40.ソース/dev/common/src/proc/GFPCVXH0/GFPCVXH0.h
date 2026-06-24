/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVXH0                                    */
/*        FUNCTION          ････ 鍵管理ファイルIO                            */
/*                                                                           */
/*        AUTHER            ････ HAS T.Hashimoto                             */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2026-04-21                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== =========== ========== ============================================ */
/*  1.0  T.Hashimoto 2026/04/21 (xxxxx)新規作成                              */
/*****************************************************************************/
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
#include "DDLCIFC.h(data_ctrl_info, header_info_rq)"
#include "DDLCTIC.h"
#include "common.h"
#include "limit.h"
#include "file.h"
#include "ems.h"
#include "errcd.h"
#include "ipc.h"
#include "GFPCGX50.h"   /* システム日時取得 */
#include "GFPCGXA0.h"   /* トランザクション管理 */
#include "GFPCGXB0.h"   /* I/Oモジュール */
#include "GFPCGXC0.h"   /* オープナープロセス管理 */
#include "GFPCGXD0.h"   /* ASSIGN情報取得 */
#include "GFPCGXG0.h"   /* プロセス情報取得 */
#include "vproc.h"


/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
/* 共通DEFINE */
#define KYFU_RET_OK                         0
#define KYFU_RET_NG                         -1
#define KYFU_RET_ERR                        9
#define KYFU_FILE_CLOSED                    -1

#define KYFU_NORMAL_TERMINATION             0
#define KYFU_ABNORMAL_TERMINATION           1

#define KYFU_FILEIO_OPEN                    "OPEN    "
#define KYFU_FILEIO_CLOSE                   "CLOSE   "
#define KYFU_FILEIO_WRITE                   "WRITE   "
#define KYFU_FILEIO_READ                    "READ    "
#define KYFU_FILEIO_UPDATE                  "REWRITE "
#define KYFU_FILE_LOGI_NAME                 "GCKEY"
#define KYFU_NON_MLT_NUM                    "0000"

#define DEF_IF_ID_DEFAULT                   "}}}}}"     /* インタフェース識別.指定なしALL"}" */
#define DEF_STATION_ID_DEFAULT              "}}}}}}"    /* ステーション識別.指定なしALL"}"   */

/* TAG */
#define DEF_MSG_ID_CM                       "CM"

/* フラグ */
#define KYFU_ON                             1
#define KYFU_OFF                            0

/* 処理レコード番号 */
#define KYFU_REC_1                          1
#define KYFU_REC_2                          2
/* サイズ */
#define KYFU_PHY_FILE_NAME_SIZE             48
#define KYFU_GCKEY_KEY_SIZE                 22

/****************************************************************************/
/*   内部変数定義                                                           */
/****************************************************************************/

/********** 構造体 ***********/
/* 自プロセス情報 */
typedef struct __myinfo_def
{
    short   recv_fno;                                   /* $RECEIVEファイル番号               */
    char*   recv_buf;                                   /* $RECEIVEバッファーポインタ         */
    char    pathmon_name[ZSYS_VAL_LEN_PROCESSNAME+1];   /* PATHMONプロセス名                  */
    short   end_flg;                                    /* 終了フラグ                         */
    char    trc_time_begin[21];                         /* トレース開始時間                   */
    char    trc_time_end[21];                           /* トレース終了時間                   */
    char    site_id;                                    /* サイト識別                         */
    char    network_id;                                 /* N/W識別                            */
    char    nw_kubun[2];                                /* N/W区分                            */
    char    group_id[5];                                /* グループ識別                       */
    char    serverclass_id[12];                         /* サーバクラス論理ID                 */
    char    serverclass_kind[8];                        /* サーバクラス種類                   */
    char    serverclass_no[4];                          /* サーバクラス論理番号               */
    char    serverclass_mlt_num[4];                     /* サーバクラス冗長化番号             */
    char    gfphi_asn_fname[12+1];                      /* 物理名情報ファイルアサイン名       */
    char    gfphi_fname[48];                            /* 物理名情報ファイル物理ファイル名   */
    short   gfphi_fnum;                                 /* 物理名情報ファイルファイル番号     */
    char    gckey_fname[48];                            /* 鍵管理ファイル物理ファイル名       */
    short   gckey_fnum;                                 /* 鍵管理ファイルファイル番号         */
    long    file_timer;                                 /* ファイルI/Oタイマー                */
    long    send_timer;                                 /* PATHSENDタイマー                   */
    char    msg_serverclass_name[16];                   /* メッセージ出力サーバクラス名       */
    char    msg_pathmon_name[16];                       /* メッセージ出力PATHMON名            */
    char    module_id[8];                               /* モジュールID                       */
    char    gfp_lcn[15];                                /* GFP内部LCN                         */
} myinfo_def;

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
oggz1in_def     cg010in;

/* トレース共通モジュール用 */
short TRACEOUT( char* );
typedef struct __g_trc_def {
    lk_zac2001r_arg_1_def   recv;
} g_trc_def;

g_trc_def       g_trc;

char  EXMYSRVCLSNAME[16];
char  EXMYPROCNAME[6];
short EXTRACEMODE;
char  EXTRACEFILENAME[47];
short EXTRACEFILENO;

#define FUNC_INIT                       '0'       /* 初期処理   */
#define FUNC_OUTPUT                     '1'       /* 出力処理   */
#define FUNC_END                        '2'       /* 終了処理   */

/* 関数のプロトタイプ宣言 */
void   KYFU_init(void);
void   KYFU_get_param(void);
void   KYFU_get_config(char *);
void   KYFU_main (void);
void   KYFU_recv_read(void);
void   KYFU_send_reply(char *, short, short);
void   KYFU_req_recv(void);
short  KYFU_gckey_update(short);
void   KYFU_message_output(short, char, char*, char*, ...);
void   KYFU_finish(void);
void   KYFU_abend(void);
