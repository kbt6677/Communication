/*****************************************************************************/
/*****                    <<     HEADER PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVX50                                    */
/*        FUNCTION          ････ 電文中継(PUT)                               */
/*                                                                           */
/*        AUTHER            ････ HAS S.Makino                                */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2024-09-26                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  S.Makino   2024/09/26 (xxxxx)新規作成                               */
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
#include "DDLCIFC.h(data_ctrl_info, header_info_rq)"
#include "DDLCTIC.h"
#include "common.h"
#include "limit.h"
#include "file.h"
#include "ems.h"
#include "errcd.h"
#include "ipc.h"
#include "GFPCGX50.h"
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
#define QPUT_RET_OK                         0
#define QPUT_RET_ERR                        9
#define QPUT_FILE_CLOSED                    -1

#define QPUT_NORMAL_TERMINATION             0
#define QPUT_ABNORMAL_TERMINATION           1

#define QPUT_FILEIO_OPEN                    "OPEN"
#define QPUT_FILEIO_CLOSE                   "CLOSE"
#define QPUT_FILEIO_WRITE                   "WRITE"
#define QPUT_FILEIO_READ                    "READ"
#define QPUT_FILE_LOGI_NAME                 "GQNWQ"
#define QPUT_NON_MLT_NUM                    "0000"

#define DEF_IF_ID_DEFAULT                   "}}}}}"     /* インタフェース識別.指定なしALL"}" */
#define DEF_STATION_ID_DEFAULT              "}}}}}}"    /* ステーション識別.指定なしALL"}"   */
/* TAG */
#define DEF_Q000                            "Q000"
#define DEF_MSG_ID_CM                       "CM"
/* フラグ */
#define QPUT_ON                             1
#define QPUT_OFF                            0

/* サイズ */
#define QPUT_PHY_FILE_NAME_SIZE             48
#define QPUT_QFILE_KEY_SIZE                 8

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
    char    gfnwi_fname[48];                            /* NW情報ファイル物理ファイル名       */
    short   gfnwi_fnum;                                 /* NW情報ファイルファイル番号         */
    long    file_timer;                                 /* ファイルI/Oタイマー                */
    long    send_timer;                                 /* PATHSENDタイマー                   */
    char    msg_serverclass_name[16];                   /* メッセージ出力サーバクラス名       */
    char    msg_pathmon_name[16];                       /* メッセージ出力PATHMON名            */
    char    module_id[8];                               /* モジュールID                       */
    char    gfp_lcn[15];                                /* GFP内部LCN                         */
} myinfo_def;

/* キューファイル情報 */
typedef struct __qfileinfo_def
{
    short       qfile_num;      /* キューファイル数             */
    short       cur_index;      /* カレントインデックス         */
    struct
    {
        char    fname[48];      /* 物理ファイル名               */
        short   fno;            /* ファイル論理番号             */
        short   findex_len;     /* ファイル冗長化番号           */
        short   fnum;           /* ファイル番号                 */
        short   ferr;           /* エラーコード                 */
        char    rec_key[50];    /* レコードキー                 */
    } finfo[30];                /* ファイル情報                 */
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

/* IPCレイアウト(TIMEOUT)・共通インターフェース */
typedef struct __ipc_timeout_if_def
{
    data_ctrl_info_def      data_ctrl_info;         /* DATA制御情報     */
    header_info_rq_def      header;                 /* ヘッダー         */
    db_gqnwq_def            qdata;                  /* QUEUE FILE DATA  */
} ipc_timeout_if_def;

/* EMS出力サブルーチン用 */
oggz1in_def     cg010in;

/* トレース共通モジュール用 */
short TRACEOUT( char* );
typedef struct __g_trc_def {
    lk_zac2001r_arg_1_def   recv;
} g_trc_def;

g_trc_def       g_trc;

char trace_buf[sizeof(lk_zac2001i_arg_1_def)];
lk_zac2001i_arg_1_def *ipc_trace = (lk_zac2001i_arg_1_def *) &trace_buf;
lk_zac2001r_arg_1_def *rcv_trace = (lk_zac2001r_arg_1_def *) &trace_buf;
lk_zac2001p_arg_1_def *scs_trace = (lk_zac2001p_arg_1_def *) &trace_buf;
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
char    ch_trc_buf[lk_zac2001t_arg_1_def_Size+1];
lk_zac2001i_arg_1_def   *p_trc_ipc = (lk_zac2001i_arg_1_def *)&ch_trc_buf;

/* 関数のプロトタイプ宣言 */
void   QPUT_init(void);
void   QPUT_get_param(void);
void   QPUT_get_config(char *);
void   QPUT_main (void);
void   QPUT_recv_read(void);
void   QPUT_send_reply(char *, short, short);
void   QPUT_req_recv(void);
void   QPUT_qfile_open(short);
short  QPUT_qfile_write(void);
void   QPUT_qfile_close(short);
void   QPUT_message_output(short, char, char*, char*, ...);
void   QPUT_finish(void);
void   QPUT_abend(void);
//void   HEX2CHAR(unsigned char *hex_p, char *terget_p,short s_len);
