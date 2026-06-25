/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVX90                                    */
/*        FUNCTION          ････ 鍵交換制御機能                              */
/*                                                                           */
/*        AUTHER            ････ HAS S.Kimura                                */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025-03-25                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  S.Kimura   2025/03/25 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/

#ifndef _GFPCVX90_H
#define _GFPCVX90_H

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <errno.h>    nolist
#include <string.h>   nolist
#include <stdlib.h>   nolist
#include <stdarg.h>   nolist
#include <tal.h>      nolist
#include <ctype.h>    nolist
#include <cextdecs.h> nolist
#include <zsysc>      nolist
#include <stdio.h>    nolist
#include <string.h>   nolist
#include <errno.h>    nolist
#include <tal.h>      nolist
#include <zspic>      nolist
#include <zfilc>      nolist

/* USER HEADER     */
#include "vproc.h"
#include "common.h"
#include "limit.h"
#include "file.h(db_gfphi)"
#include "file.h(db_gckey)"
#include "file.h(db_glnlg)"
#include "file.h(db_gfnwi)"
#include "file.h(db_gfnws)"
#include "file.h(db_glmlg)"
#include "file.h(db_gcsst)"
#include "file.h(db_glelg)"
#include "ipc.h"
#include "ems.h"
#include "GFPCGX40.h"           // PATHSEND処理
#include "GFPCGXH0.h"           // PATHSEND(システム採番用)処理
#include "GFPCGX50.h"           // システム日時取得
#include "GFPCGX80.h"           // エラー出力ログ編集出力
#include "GFPCGXA0.h"           // トランザクション管理
#include "GFPCGXB0.h"           // IOモジュール
#include "GFPCGXC0.h"           // オープナープロセス管理
#include "GFPCGXE0.h"           // タイムアウト時刻算出
#include "GFPCVXZ0.h"           // メインモジュールテンプレート
#include "GFPCVXZ2.h"           // メインモジュールextern
#include "NWM_KYX.h"            // 鍵管理個別処理
#include "NWM_ENI.h"            // ATALLA情報取得処理
#include "GFPCGX90.h"           // ユニーク時間取得処理

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/

#define CKYX_SPACE                         ' '
#define CKYX_ZERO                          0
#define CKYX_CHAR_ZERO                     '0'

#define DEF_STAN6                           "03"          /* システム採番の通番区分                     */

#define CKYX_C401_FLG                       1            /* C401受信                                  */
#define CKYX_C402_FLG                       2            /* C402受信                                  */

#define CKYX_RSP_KIND_NORMAL                0           /* IPC応答種別 正常                           */
#define CKYX_RSP_KIND_REJ                   1           /* IPC応答種別 拒否応答                       */
#define CKYX_RSP_KIND_FAULT                 2           /* IPC応答種別 障害電文通知                   */
#define CKYX_RSP_KIND_HAKI                  3           /* IPC応答種別 破棄                           */
#define CKYX_RSP_KIND_ERR                   9           /* IPC応答種別 エラー応答                     */

#define CKYX_MAX_ATALLA_CMD_LEN             5000        /* ATALLAコマンド長                           */
#define CKYX_MAX_DATA_SIZE                  12000       /* 最大PATHSENDデータ長                       */
#define CKYX_RET_OK                         0           /* 戻り値 正常                                */
#define CKYX_RET_NG                         -1          /* 戻り値 異常                                */
#define CKYX_RET_ABORT                      2           /* ABORT                                      */
#define CKYX_FLG_ON                         1
#define CKYX_FLG_OFF                        0

#define CKYX_LOG_SET                        1            /* ログ新規設定                              */
#define CKYX_LOG_UPDATE                     2            /* ログ更新                                  */

#define CKYX_LCN_LENG                       15           /* LCN長                                     */

/* 個別関数用設定情報 */
#define CKYX_ATALLA_11A_VER_B               'B'
#define CKYX_CTL_LOG_HISIMUKE               'H'
#define CKYX_CTL_LOG_SIMUKE                 'S'
#define CKYX_CTL_LOG_DATA_ARI               '1'
#define CKYX_CTL_LOG_DATA_NASHI             '0'
#define CKYX_UNIT_GROUP                     0
#define CKYX_UNIT_NW                        1
#define CKYX_UNIT_INTERFACE                 2
#define CKYX_UNIT_STATION                   3
#define CKYX_UNIT_CONNECTION                4

#define CKYX_ATALLA_1PUNE                   "1PUNE000"
#define CKYX_ATALLA_1PUNS                   "1PUNS000"

#define CKYX_KEY_EXPORT_E                   'E'
#define CKYX_KEY_EXPORT_S                   'S'

#define CKYX_OWN_FILE_OWN_SITE              0           /* 鍵交換ファイル種別(自ノード 自サイト)     */
#define CKYX_OWN_FILE_OTHER_SITE            1           /* 鍵交換ファイル種別(自ノード 他サイト)     */

#define CKYX_ERR_NASHI                      0
#define CKYX_ERR_LOG_WRITE                  2
#define CKYX_ERR_LOG_NAIBU_ERR              '2'

#define CKYX_REQUEST_TRIGGER_UNDEFINED      0           /* リクエスト契機未定義 */
#define CKYX_REQUEST_TRIGGER_CENTER         1           /* リクエスト契機接続先 */
#define CKYX_REQUEST_TRIGGER_GFP            2           /* リクエスト契機GFP    */

#define CKYX_CMD_KEY_DEC                    1
#define CKYX_CMD_KEY_MAKE                   2
#define CKYX_CMD_KEY_ENC                    3
#define CKYX_CMD_KEY_CKDIGIT                4
#define CKYX_KEY_DS1                        "DS1"
#define CKYX_KEY_DS2                        "DS2"
#define CKYX_KEY_DS3                        "DS3"
#define CKYX_KEY_FMT_VA                     "VA"
#define CKYX_KEY_FMT_KB                     "KB"
#define CKYX_KEY_ALG_S                      'S'
#define CKYX_KEY_ALG_D                      'D'
#define CKYX_KEY_ALG_T                      'T'
#define CKYX_KEY_LEN                        96
#define CKYX_KEK_LEN                        74
#define CKYX_CHKDIGIT_LEN                   6
#define CKYX_KEY_MAC                        "KMAC"
#define CKYX_KEY_ENC                        "KC"
#define CKYX_TR31_KEY_BLK_LEN               4000
#define CKYX_KEY_DB_KMAC                    "KMAC"
#define CKYX_KEY_DB_KC                      "KC  "
#define CKYX_KEY_DB_KPE                     "KPE "
#define CKYX_KEY_DB_CONV                    "CONV"

#define CKYX_ATALLA_CMD_10                  "10"
#define CKYX_ATALLA_CMD_1A                  "1A"
#define CKYX_ATALLA_CMD_11A                 "11A"
#define CKYX_ATALLA_CMD_11B                 "11B"
#define CKYX_ATALLA_CMD_119                 "119"
#define CKYX_ATALLA_CMD_7E                  "7E"

#define CKYX_ATALLA_RSP_20                  "20"
#define CKYX_ATALLA_RSP_2A                  "2A"
#define CKYX_ATALLA_RSP_21A                 "21A"
#define CKYX_ATALLA_RSP_21B                 "21B"
#define CKYX_ATALLA_RSP_219                 "219"
#define CKYX_ATALLA_RSP_8E                  "8E"

#define CKYX_KEY_READ_UNLOCK                1
#define CKYX_KEY_READ_LOCK                  2

/* 局状態 */
#define CKYX_STATION_ST_OPEN               '0'  /* 開局       */
#define CKYX_STATION_ST_CLOSE              '9'  /* 閉局       */
#define CKYX_STATION_ST_OPEN_PROC          '1'  /* 開局処理中 */
#define CKYX_STATION_ST_CLOSE_PROC         '8'  /* 閉局処理中 */

/* 送信電文有無 */
#define CKYX_SEND_DATA_ARI                 "10"  /* 送信電文あり */
#define CKYX_SEND_DATA_NASHI               "20"  /* 送信電文なし */

/* PATHSENDエラーコード */
#define CKYX_PSD_ERR_CONNECT               902   /* PATHMON CONNECT ERROR */
#define CKYX_PSD_ERR_SHUTDOWN              915   /* PATHMON SHUTDOWN */

/* GUARDIANエラーコード */
#define CKYX_FIL_ERR_NOSUCHDEV              14   /* device does not exist */

/* キーインデックス更新有無 */
#define CKYX_KEY_IDX_UPD_NO                  0   /* 更新無し */
#define CKYX_KEY_IDX_UPD_YES                 1   /* 更新無し */

/****************************************************************************/
/*   TYPEDEF定義                                                            */
/****************************************************************************/

/* 個別ファイル情報 */

                                                        /* ---------------------------------- */
typedef struct __file_info_def                          /* ファイル情報                       */
{                                                       /* ---------------------------------- */
    char      gfphi_fid[8];                             /*  物理名情報ファイル名   (論理)     */
    short     gfphi_fid_len;                            /*  物理名情報ファイル名長 (論理)     */
    char      gfphi_fname[48];                          /*  物理名情報ファイル名   (物理)     */
    short     gfphi_fname_len;                          /*  物理名情報ファイル名長 (物理)     */
    short     gfphi_fno;                                /*  物理名情報ファイル番号            */
    char      gfnwi_fid[8];                             /*  N/W情報ファイル名      (論理)     */
    short     gfnwi_fid_len;                            /*  N/W情報ファイル名長    (論理)     */
    char      gfnwi_fname[48];                          /*  N/W情報ファイル名      (物理)     */
    short     gfnwi_fname_len;                          /*  N/W情報ファイル名長    (物理)     */
    short     gfnwi_fno;                                /*  N/W情報ファイル番号               */
    char      gfnws_fid[8];                             /*  接続先固有情報ファイル名      (論理)    */
    short     gfnws_fid_len;                            /*  接続先固有情報ファイル名長    (論理)    */
    char      gfnws_fname[48];                          /*  接続先固有情報ファイル名      (物理)    */
    short     gfnws_fname_len;                          /*  接続先固有情報ファイル名長    (物理)    */
    short     gfnws_fno;                                /*  接続先固有情報ファイル番号              */
    char      own_gckey_fid[8];                         /*  鍵管理情報ファイル名(自ノード)    (論理)*/
    short     own_gckey_fid_len;                        /*  鍵管理情報ファイル名長(自ノード)  (論理)*/
    char      own_gckey_fname[48];                      /*  鍵管理情報ファイル名(自ノード)    (物理)*/
    short     own_gckey_fname_len;                      /*  鍵管理情報ファイル名長(自ノード)  (物理)*/
    short     own_gckey_fno;                            /*  鍵管理情報ファイル番号(自ノード)        */
    char      gcsst_fid[8];                             /*  局情報ファイル名       (論理)     */
    short     gcsst_fid_len;                            /*  局情報ファイル名長     (論理)     */
    char      gcsst_fname[48];                          /*  局情報ファイル名       (物理)     */
    short     gcsst_fname_len;                          /*  局情報ファイル名長     (物理)     */
    short     gcsst_fno;                                /*  局情報ファイル番号                */
    char      glmlg_fid[8];                             /*  制御ログファイル名     (論理)     */
    short     glmlg_fid_len;                            /*  制御ログファイル名長   (論理)     */
    char      glmlg_fname[48];                          /*  制御ログファイル名     (物理)     */
    short     glmlg_fname_len;                          /*  制御ログファイル名長   (物理)     */
    short     glmlg_fno;                                /*  制御ログファイル番号              */
} file_info_def;

                                                        /* ---------------------------------- */
                                                        /* カット対象日付管理ファイル情報     */
typedef struct __file_info_gccut_def {                  /* ---------------------------------- */
    char    file_id[8];                                 /* ファイルID                         */
    char    file_name[48];                              /* ファイル名                         */
    short   file_no;                                    /* ファイル番号                       */
    long    io_timer;                                   /* I/Oタイマー                        */
} file_info_gccut_def;

                                                        /* ---------------------------------- */
typedef struct __file_def                               /* ファイル読み込み領域               */
{                                                       /* ---------------------------------- */
    db_gfnwi_def      db_gfnwi_g;                       /* NW情報レコード(グループ単位)       */
    db_gfnwi_def      db_gfnwi_i;                       /* NW情報レコード(インタフェース単位) */
    char              db_gfnws_n[200];                  /* 接続先固有情報(NW単位)             */
    char              db_gfnws_i[200];                  /* 接続先固有情報(インタフェース単位) */
    char              db_gfnws_s[200];                  /* 接続先固有情報(ステーション単位)   */
    char              db_gfnws_c[200];                  /* 接続先固有情報(コネクション単位)   */
    db_gckey_def      db_gckei_i;                       /* 鍵管理情報レコード                 */
} file_def;

                                                        /* ---------------------------------- */
typedef struct __atalla_con_def                         /* ATALLAサーバー情報保存領域         */
{                                                       /* ---------------------------------- */
    char          pathmon_name[16];                     /* PATHMON名                          */
    char          server_class[16];                     /* サーバークラス                     */
    unsigned long pathsend_timer;                       /* PATHSENDタイマー                   */
    short         retry_cnt;                            /* リトライ回数                       */
} atalla_con_def;

                                                        /* ---------------------------------- */
typedef struct __atalla_info_def                        /* ATALLA処理結果保存領域             */
{                                                       /* ---------------------------------- */
    char              key_use_10[8];                    /* Key Usage                          */
    char              chkdigit_len;                     /* チェックディジット長               */
    char              chkdigit[CKYX_CHKDIGIT_LEN];      /* チェックディジット                 */
    char              key_len;                          /* キー長                             */
    char              key[CKYX_KEY_LEN];                /* キー                               */
    char              key_usage_len;                    /* Key Usage長                        */
    char              key_usage_11B[2];                 /* Key Usage                          */
} atalla_info_def;

                                                        /* ---------------------------------- */
typedef struct __atalla_info_20_def                     /* ATALLA処理結果保存領域(20)         */
{                                                       /* ---------------------------------- */
    short             key_mfk_len;                      /* Working Key (MFK encrypted)長      */
    char              key_mfk[CKYX_KEY_LEN];            /* Working Key (MFK encrypted)        */
    short             chk_digit_len;                    /* チェックディジット長               */
    char              chk_digit[CKYX_CHKDIGIT_LEN];     /* チェックディジット                 */
    short             key_kek_len;                      /* Working Key (KEK encrypted)長      */
    char              key_kek[CKYX_KEY_LEN];            /* Working Key (KEK encrypted)        */
} atalla_info_20_def;

                                                        /* ---------------------------------- */
typedef struct __atalla_info_2A_def                     /* ATALLA処理結果保存領域(2A)         */
{                                                       /* ---------------------------------- */
    short             key_len;                          /* キー長                             */
    char              key[CKYX_KEY_LEN];                /* キー                               */
    short             chk_digit_len;                    /* チェックディジット長               */
    char              chk_digit[CKYX_CHKDIGIT_LEN];     /* チェックディジット                 */
    short             key_akb_len;                      /* キー長                             */
    char              key_akb[CKYX_KEY_LEN];            /* キー                               */
} atalla_info_2A_def;

                                                        /* ---------------------------------- */
typedef struct __atalla_info_21A_def                    /* ATALLA処理結果保存領域(21A)        */
{                                                       /* ---------------------------------- */
    short             chk_digit_len;                    /* チェックディジット長               */
    char              chk_digit[CKYX_CHKDIGIT_LEN];     /* チェックディジット                 */
    short             key_akb_len;                      /* キーAKB長                          */
    char              key_akb[CKYX_KEY_LEN];            /* キーAKB                            */
    short             key_blk_len;                      /* キー長                             */
    char              key_blk[4000];                    /* TR-31 Key Block                    */
} atalla_info_21A_def;

                                                        /* ---------------------------------- */
typedef struct __atalla_info_21B_def                    /* ATALLA処理結果保存領域(21B)        */
{                                                       /* ---------------------------------- */
    short             key_len;                          /* キー長                             */
    char              key[CKYX_KEY_LEN];                /* キー                               */
    short             chk_digit_len;                    /* チェックディジット長               */
    char              chk_digit[CKYX_CHKDIGIT_LEN];     /* チェックディジット                 */

} atalla_info_21B_def;

                                                        /* ---------------------------------- */
typedef struct __atalla_info_219_def                    /* ATALLA処理結果保存領域(219)        */
{                                                       /* ---------------------------------- */
    short             key_len;                          /* キー長                             */
    char              key[CKYX_KEY_LEN];                /* キー                               */
    short             chk_digit_len;                    /* チェックディジット長               */
    char              chk_digit[CKYX_CHKDIGIT_LEN];     /* チェックディジット                 */
} atalla_info_219_def;

                                                        /* ---------------------------------- */
typedef struct __atalla_info_8E_def                     /* ATALLA処理結果保存領域(8E)         */
{                                                       /* ---------------------------------- */
    short             chk_digit_len;                    /* チェックディジット長               */
    char              chk_digit[CKYX_CHKDIGIT_LEN];     /* チェックディジット                 */
} atalla_info_8E_def;
                                                        /* ---------------------------------- */
typedef struct __key_info_save_def                      /* KEY情報保存領域                    */
{                                                       /* ---------------------------------- */
    short             key_len;                          /* キー長                             */
    char              key[CKYX_KEY_LEN];                /* キー                               */
    short             chk_digit_len;                    /* チェックディジット長               */
    char              chk_digit[CKYX_CHKDIGIT_LEN];     /* チェックディジット                 */
    short             key_akb_len;                      /* キー長                             */
    char              key_akb[CKYX_KEY_LEN];            /* キー                               */
    char              key_usage_len;                    /* Key Usage長                        */
    char              key_usage_11B[2];                 /* Key Usage                          */
    short             key_blk_len;                      /* キー長                             */
    char              key_blk[4000];                    /* TR-31 Key Block                    */
    short             file_set_wk_key_len;              /* キー長(ファイル設定用)             */
    char              file_set_wk_key[CKYX_KEY_LEN];    /* キー  (ファイル設定用)             */
} key_info_save_def;

/* GFP内部LCN採番接続情報 */
typedef struct __t_lcncon_data
{
    char          domain_name[8];
    char          pathmon_name[16];
    char          server_class[16];
    unsigned long pathsend_timer;
    short         retry_cnt;
} t_lcncon_data;

/* システム採番接続情報 */
typedef struct __t_sys_no_data
{
    char          domain_name[8];
    char          pathmon_name[16];
    char          server_class[16];
    unsigned long pathsend_timer;
    short         retry_cnt;
} t_sys_no_data;

/* 他ノード鍵管理ファイル更新サーバ情報 */
typedef struct __t_gckey_upd_srv
{
    char          domain_name[8];
    char          pathmon_name[16];
    char          server_class[16];
    unsigned long pathsend_timer;
    short         retry_cnt;
} t_gckey_upd_srv;

/* 他ノード鍵管理ファイル更新レコード情報 */
typedef struct __t_gckey_upd_rec
{
    short         upd_rec_cnt;                /* 更新レコード数 */
    struct
    {
        char      upd_rec[800];               /* 更新レコード */
    } rectbl[2];
} t_gckey_upd_rec;

/****************************************************************************/
/*   グローバルデータ                                                       */
/****************************************************************************/
t_myinfo_def                 myinfo;                    /* 自プロセス情報テーブル             */
short                        g_resp_kind;               /* 応答種別                           */
file_info_def                g_file_info;               /* ファイル情報                       */
t_lcncon_data                g_lcncon_data;             /* GFP内部LCN採番接続情報             */
t_sys_no_data                g_sys_no_data_pri;         /* システム採番接続情報(primary)      */
t_sys_no_data                g_sys_no_data_sec;         /* システム採番接続情報(secondary)    */
gflin_pkey_def               g_rcv_con_info;            /* 受信コネクション情報               */
NWM_KYX_arg_1_def            g_req_seisa_result;        /* 要求電文精査結果                   */
NWM_KYX_arg_2_def            g_rsp_seisa_result;        /* 応答電文精査結果                   */
atalla_info_def              g_atalla_info;             /* ATALLA処理取得情報                 */
key_info_save_def            g_key_info_save;           /* KEY情報保存領域                    */
db_gfnwi_def                 g_nwi_g;                   /* NW情報ファイル読込(グループ)       */
db_gfnwi_def                 g_nwi_i;                   /* NW情報ファイル読込(インターフェース)*/
db_gfnws_def                 g_nws_n;                   /* 接続先固有情報読込(ネットワーク)    */
db_gfnws_def                 g_nws_i;                   /* 接続先固有情報読込(インターフェース)*/
db_gfnws_def                 g_nws_s;                   /* 接続先固有情報読込(ステーション)    */
db_gfnws_def                 g_nws_c;                   /* 接続先固有情報読込(コネクション)    */
db_gckey_def                 g_gckey;                   /* 鍵管理ファイル読込                  */
db_gckey_def                 g_gckey_conv;              /* 鍵管理ファイル読込(内部変換用)      */
db_gckey_def                 g_key_info;                /* 鍵管理ファイル更新                  */
atalla_con_def               g_atalla_con;              /* ATALLA接続情報                      */
char                         g_internal_error_code[7];  /* 内部エラーコード                    */
char                         g_station_sts[2];          /* 局状態設定領域                      */
char                         g_lcn[15];                 /* GFP内部LCN                          */
char                         g_sys_num[6];              /* システム採番                        */
char                         g_key_kind[4];             /* キー種別                            */
char                         g_key_rsp_msg[CKYX_MAX_DATA_SIZE]; /* レスポンス設定領域          */
short                        g_key_rsp_leng;            /* レスポンス長                        */
file_info_gccut              g_file_info_gccut;         /* カット対象日付管理ファイル情報      */
char                         g_rcv_key[256];            /* 受信したキー                        */
char                         g_rcv_checkdigit[32];      /* 受信したチェックディジット          */
char                         g_send_err_code[7];        /* 送信不可応答エラーコード            */
short                        g_request_trigger;         /* 処理契機                            */
char                         g_save_checkdigit[32];     /* ANSI X9.17形式に変換時取得したチェックディジット */
short                        g_save_checkdigit_len;     /* ANSI X9.17形式に変換時取得したチェックディジット長 */
t_gckey_upd_srv              g_gckey_upd_srv;           /* 他ノード鍵管理ファイル更新サーバ情報 */
t_gckey_upd_rec              g_gckey_upd_rec;           /* 他ノード鍵管理ファイル更新レコード情報 */

/****************************************************************************/
/*   プロトタイプ関数宣言                                                   */
/****************************************************************************/
short CKYX_handle_req_msg_c401(cr401_def *);
short CKYX_handle_req_msg_c402(cr402_def *);
short CKYX_key_exchange_make_req(cr402_def *);
short CKYX_key_req_rsp(cr401_def *);
short CKYX_key_exchange_req(cr401_def *);
short CKYX_key_exchange_rsp_err(cr401_def *);
short CKYX_gfp_key_exchange_send_req(cr402_def *);
short CKYX_gfp_key_send_rsp(cr401_def *);
short CKYX_key_cmd_dec(cr401_def *);
short CKYX_key_cmd_enc(cr402_def *);
short CKYX_atalla_cmd_make(short, char *, short *);
short CKYX_atalla_make_10(char *, short *);
short CKYX_atalla_make_1A(char *, short *);
short CKYX_atalla_make_11A(char *, short *);
short CKYX_atalla_make_11B(char *, short *);
short CKYX_atalla_make_119(char *, short *);
short CKYX_atalla_make_7E(char *, short *);
short CKYX_atalla_pathsend(char *, short , char *);
short CKYX_atalla_rsp_chk(char , char *);
short CKYX_atalla_rsp_20(char *, atalla_info_20_def *);
short CKYX_atalla_rsp_2A(char *, atalla_info_2A_def *);
short CKYX_atalla_rsp_21A(char *, atalla_info_21A_def *);
short CKYX_atalla_rsp_21B(char *, atalla_info_21B_def *);
short CKYX_atalla_rsp_219(char *, atalla_info_219_def *);
short CKYX_atalla_rsp_8E(char *, atalla_info_8E_def *);
short CKYX_key_file_update(short);
short CKYX_key_write(char, db_gckey_def*);
short CKYX_key_file_num_chg();
short CKYX_other_node_gckey_upd();
short CKYX_r401_make(cr401_def *, short );
short CKYX_r402_make(short , cr402_def *, short );
short CKYX_get_lcn (char *);
short CKYX_system_num_get (char *);
short CKYX_ctl_log_output ( char *,char, char, char *);
short CKYX_err_log_output ( char *,char, char *);
short CKYX_read_nwfile(short, gflin_pkey_def*, db_gfnwi_def*);
short CKYX_read_nwsfile(short, gflin_pkey_def*, db_gfnws_def*);
short CKYX_key_read(char*, char, char, gflin_pkey_def*, db_gckey_def*);
short CKYX_key_read_internal(char*, char, char, gflin_pkey_def*, db_gckey_def*);
short CKYX_char2hex(char *, char *, short );
short CKYX_read_station_st(gflin_pkey_def *, char *);
short CKYX_all_file_read(gflin_pkey_def *);
short CKYX_reply ();

#endif /* _GFPCVX90_H */

