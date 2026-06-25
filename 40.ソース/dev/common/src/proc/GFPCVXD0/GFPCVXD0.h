/*****************************************************************************/
/*****                    <<     HEADER PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCVXD0                                    */
/*        FUNCTION          ････ コマンドサーバ                              */
/*                                                                           */
/*                                                                           */
/*        AUTHER            ････ ISYS Y.Kawasaki                             */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2024-12-09                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  Y.Kawasaki 2024/12/09 (J0680)新規作成                               */
/*  1.1  M.Matumoto 2026/05/20 (J0680)DR運用対応                             */
/*                                                                           */
/*****************************************************************************/
#ifndef __GFPCVXD0_H__
#define __GFPCVXD0_H__

/* ################### */
/* # INCLUDE         # */
/* ################### */
/* STANDARD HEADER */
#include  <zsysc>        nolist
/* USER HEADER */
#include "common.h"      nolist      // common
#include "file.h"        nolist      // file
#include "ipc.h"         nolist      // IPC
#include "GFPCGXB0.h"    nolist      // IOモジュール
#include "GFPCGXG0.h"    nolist      // プロセス情報取得
#include "GFPCGXC0.h"    nolist      // サーバ停止判定

/* -------------------------------------------------------*/
/* 定数定義                                               */
/* -------------------------------------------------------*/
#define     DEF_RECVFILE_NAME               "$RECEIVE"          // $RECEIVEファイル名
#define     DEF_CMDS_QUORIFIRE              ".#GFPCI"           // コマンドサーバクオリファイア
#define     DEF_MY_PROGID                   DEF_GFPCVXD0        // 自プログラムID
#define     DEF_MY_SCID                     DEF_SC_CMD_SRV      // 自サーバクラス論理ID
#define     DEF_EMS_DEF_RET_CODE            "0"                 // EMS出力モジュールデフォルトリターンコード
#define     DEF_IFID_DEFAULT                "}}}}}"             // インターフェース識別指定なし
#define     DEF_STID_DEFAULT                "}}}}}}"            // ステーション識別指定なし
#define     DEF_NERR_EMPTY                  "       "           //
#define     DEF_GFLIN_VALID_REC             ' '                 // 回線管理ファイル有効レコードフラグ

#define     DEF_FLG_OFF                     0                   // OFF
#define     DEF_FLG_ON                      1                   // ON
#define     DEF_FLG_OK                      0                   // OK
#define     DEF_FLG_NG                      1                   // NG
#define     DEF_PROC_NORMAL_END             0                   // プロセス正常終了
#define     DEF_PROC_ABNORMAL_END           1                   // プロセス異常終了
#define     DEF_OPENID_ANCESTOR             1                   // オープンID(親プロセス)
#define     DEF_OPENID_ROUT                 2                   // オープンID(ROUT)
#define     DEF_RCV_BUF_SIZE                16384               // 受信バッファサイズ
#define     DEF_RPL_BUF_SIZE                16384               // 応答バッファサイズ
#define     DEF_RECV_DEPTH_MAX              4047                // $RECEIVE 受信メッセージの最大数
#define     DEF_SC_OFFSET_SITE              0                   // site_idオフセット
#define     DEF_SC_OFFSET_NW                2                   // network_idオフセット
#define     DEF_SC_OFFSET_GRP               4                   // group_idオフセット
#define     DEF_SC_OFFSET_SCKND             10                  // serverclass種類オフセット
#define     DEF_SC_OFFSET_SCNUM             19                  // serverclass論理番号オフセット
#define     DEF_SC_KIND_LEN                 8                   // サーバクラス種類レングス
#define     DEF_CHK_IPC_OK                  0                   // IPC内容精査：OK
#define     DEF_CHK_IPC_NG                  1                   // IPC内容精査：NG
#define     DEF_CHK_IPC_IFCD_ERR            1                   // IPC内容精査：I/Fコードエラー
#define     DEF_CHK_IPC_DLEN_ERR            2                   // IPC内容精査：データ長エラー
#define     DEF_CHK_IPC_CMD_ERR             3                   // IPC内容精査：コマンド識別エラー
#define     DEF_CHK_IPC_NW_ERR              4                   // IPC内容精査：N/W識別エラー
#define     DEF_CHK_IPC_GRP_ERR             5                   // IPC内容精査：グループ識別エラー
#define     DEF_CHK_IPC_OPT_ERR             6                   // IPC内容精査：必須項目エラー
#define     DEF_CHK_IPC_SC_ERR              7                   // IPC内容精査：サーバークラス論理IDエラー
#define     DEF_CONNCTRL_NUM_MAX            100                 // コネクション制御、リスナサーバークラス最大数(30+30+30+α)
#define     DEF_PROC_SC_NUM_MAX             200                 // 1サーバクラスあたりのプロセス数最大数
#define     DEF_STA_GRP_NUM_MAX             1024                // 1環境グループあたりのステーション最大数
#define     DEF_CONN_GRP_NUM_MAX            8192                // 1環境グループあたりのコネクション最大数
#define     DEF_EVT_RSP_ERR_DETAIL_LEN      20                  // IPC応答エラー エラー内容長

#define     DEF_IPC_CMD_CD_OPN              1010                // オープン 
#define     DEF_IPC_CMD_CD_CLS              1020                // クローズ 
#define     DEF_IPC_CMD_CD_LSN_START        1031                // リスナー（開始）
#define     DEF_IPC_CMD_CD_LSN_END          1032                // リスナー（終了）
#define     DEF_IPC_CMD_CD_STS_DSP          1000                // コネクションステータス照会
#define     DEF_IPC_CMD_CD_CNT_OPN          2010                // 開局
#define     DEF_IPC_CMD_CD_CNT_OPN_ABS      2011                // 開局強制実行
#define     DEF_IPC_CMD_CD_CNT_OPN_UPD      2012                // 開局状態更新のみ
#define     DEF_IPC_CMD_CD_CNT_OPN_AO1      2013                // 自動開局（開局要求電文受信）
#define     DEF_IPC_CMD_CD_CNT_OPN_AO2      2014                // 自動開局（コネクション確立）
#define     DEF_IPC_CMD_CD_CNT_CLS          2020                // 閉局
#define     DEF_IPC_CMD_CD_CNT_CLS_ABS      2021                // 閉局強制実行
#define     DEF_IPC_CMD_CD_CNT_CLS_UPD      2022                // 閉局状態更新のみ
#define     DEF_IPC_CMD_CD_CNT_STS_DSP      2000                // 局状態照会
#define     DEF_IPC_CMD_CD_ECH_SND          2110                // エコー送信
#define     DEF_IPC_CMD_CD_ECH_STS_DSP      2100                // エコーステータス照会
#define     DEF_IPC_CMD_CD_KEY_EXC_REQ      2210                // 鍵交換依頼 
#define     DEF_IPC_CMD_CD_KEY_EXC          2220                // 鍵交換
#define     DEF_IPC_CMD_CD_LOG_FL_EXC       3010                // ログファイル切替
#define     DEF_IPC_CMD_CD_FL_RE_READ       4010                // ファイル再読込
#define     DEF_IPC_CMD_CD_FL_RE_LIN        4011                // ファイル再読込 接続構成変更
#define     DEF_IPC_CMD_CD_FL_RE_NSW        4012                // ファイル再読込 東阪振分比率変更

//// EMS出力内容 /////////////////////////////////////////////////
#define     DEF_EMS_REC_NOT_EXISTS          "target record not found"
#define     DEF_EMS_EXC_MAX_REC_CNT         "exceeded maximum record count"
#define     DEF_EMS_IPC_CHK_IFCD            "ｲﾝﾀｰﾌｪｰｽｺｰﾄﾞ"
#define     DEF_EMS_IPC_CHK_LEN             "ﾚﾝｸﾞｽ"
#define     DEF_EMS_IPC_CHK_ERRCD_FMT       "ｴﾗｰｺｰﾄﾞ %1d(%7s)"
#define     DEF_EMS_IPC_CHK_CMDCD           "ｺﾏﾝﾄﾞｼｷﾍﾞﾂ"
#define     DEF_EMS_IPC_CHK_NW              "NWｼｷﾍﾞﾂ"
#define     DEF_EMS_IPC_CHK_GRP             "ｸﾞﾙｰﾌﾟｼｷﾍﾞﾂ"
#define     DEF_EMS_IPC_CHK_OPT             "ﾋｯｽﾊﾟﾗﾒｰﾀﾌｿｸ"
#define     DEF_EMS_IPC_CHK_SC              "ｻｰﾊﾞｰｸﾗｽﾛﾝﾘID"
#define     DEF_EMS_IPC_CHK_MNGLYR          "ｶﾝﾘﾀﾝｲ"

//// 暫定共通モジュール用 /////////////////////////////////////////////////
// ★COM_IOM
#define     DEF_COM_IOM_FIO_OPEN            "OPEN    "
#define     DEF_COM_IOM_FIO_START           "START   "
#define     DEF_COM_IOM_FIO_READ            "READ    "
#define     DEF_COM_IOM_FIO_CLOSE           "CLOSE   "

// サイト
enum {
    ESITEEAST,              // 東京
    ESITEWEST,              // 大阪
    ESITENUM                // サイト数
};

// ファイル種別
enum {
    EFILEGCLST,             // 回線ステータスファイル
    EFILEGCSST,             // 局状態管理ファイル
    EFILEGCEST,             // エコー状態管理ファイル
    EFILEGFLIN,             // 回線管理ファイル
    EFILEGFNWI,             // NW情報ファイル
    EFILEKINDNUM            // ファイル種別数
};

// コネクション系サーバクラス種別
enum {
    ELISTEN,                // リスナー
    ECONSVR,                // コネクション制御(サーバ)
    ECONCLT,                // コネクション制御(クライアント)
    ECONSCNUM               // リスナー、コネクション制御種別数
};

// プロセス種別
enum {
    EPROCMSDSI,             // 電文振分(inbound)
    EPROCMSDSO,             // 電文振分(outbound)
    EPROCLOGSV,             // ログ出力
    EPROCNUM                // プロセス種別数
};

// 管理単位種別
enum {
    EMNGLYROPCL,            // 開局/閉局管理単位
    EMNGLYRECHO,            // エコーテスト管理単位
    EMNGLYRKCHG,            // 鍵交換管理単位
    EMNGLYRCTOV,            // カットオーバー管理単位
    EMNGLYRCONN,            // コネクション数管理単位
    EMNGLYRNUM              // 管理単位種別数
};

// 管理単位階層
enum {
    ELYRSITE,               // サイト
    ELYRNW,                 // ネットワーク
    ELYRGRP,                // グループ
    ELYRINTF,               // インターフェース
    ELYRSTA,                // ステーション
    ELYRCONN,               // コネクション
    ELYRNUM                 // 階層数
};

/* -------------------------------------------------------*/
/* 構造体のtypedef定義                                    */
/* -------------------------------------------------------*/
// I/O完了情報
typedef struct __iocomp
{
    short    fno;                                       // 完了ファイル番号
    long     addr;                                      // バッファアドレス
    unsigned short len;                                 // 完了サイズ
    long     tag;                                       // 完了タグ
    short    errno;                                     // エラーコード
    zsys_ddl_receiveinformation_def  recv_info;         // $RECEIVE完了情報
} iocomp_def;

// 応答情報
typedef struct __replyinfo
{
    char     replybuf[DEF_RPL_BUF_SIZE];                // 応答バッファ
    short    reply_len;                                 // 応答レングス
    short    reply_code;                                // 応答コード
} replyinfo_def;

// 自プロセス情報
typedef struct __myinfo
{
    char                site_id[1];                     // サイト識別
    char                network_id[1];                  // N/W識別
    char                group_id[5];                    // グループ識別
    char                serverclass_id[12];             // サーバクラス論理ID
    char                serverclass_name[6];            // サーバクラス名
    short               end_flg;                        // プロセス終了フラグ
    short               command_code;                   // コマンド識別(binary)
    short               recv_fno;                       // $RECEIVEファイル番号
    long                fio_timer;                      // ファイルIOタイマー(10msec)
    long                pwr_timer;                      // プロセスW/Rタイマー(10msec)
    long                psd_timer;                      // PATHSENDタイマー(10msec)
    short               psd_retrycnt;                   // PATHSENDリトライ回数
    char                recvbuf[DEF_RCV_BUF_SIZE];      // 受信バッファ
    procinfo_def        procinfo;                       // プロセス情報
    iocomp_def          iocomp;                         // I/O完了情報
    COM_STP_arg_1_def   stp_arg;                        // サーバ停止判定I/F
    replyinfo_def       replyinfo;                      // reply情報
    COM_IOM_arg_3_def   iom_trace;                      // 汎用IOモジュール第3アーギュメント
    COM_IOM_arg_4_def   iom_gfphi;                      // 物理名情報ファイル用IOモジュール第4アーギュメント
} myinfo_def;

// ファイル情報
typedef struct __fileinfo
{
    short       key_len;                                // key長
    short       rec_len;                                // レコード長
    short       rec_max;                                // レコード最大数
    const char* logical_fname;                          // ファイル論理名
    char        name[ESITENUM][ZSYS_VAL_LEN_FILENAME+1];// ファイル物理名
} fileinfo_def;

// 送信先情報
typedef struct __destinfo
{
    char            srv_cls_kind[8];
    char            srv_cls_num[4];
    char            srv_cls_mlt_num[4];
    char            domain_name[8];
    char            pathmon_name[16];
    char            srv_cls_name[16];
    char            prc_file_name[48];
    short           is_forwarded;
} destinfo_def;

// 回線管理データ
typedef struct __gflindata
{
    gflin_pkey_def  pri_key;
    char            srv_cls_kind[8];
    char            srv_cls_num[4];
    char            operation_id;
    char            filler;
} gflindata_def;

// NW情報データ
#pragma fieldalign shared2 __gfnwidata
typedef struct __gfnwidata
{
   struct
   {
      char                            site_id;
      char                            nw_id;
      char                            grp_id[5];
      char                            if_id[5];
      char                            station_id[6];
   } pri_key;
   struct
   {
      char                            nw_kubun[2];
      char                            nw_if[20];
      char                            nw_station[11];
      char                            future_use[29];
   } nw_id_info;
} gfnwidata_def;

/* -------------------------------------------------------*/
/* 関数プロトタイプ宣言                                   */
/* -------------------------------------------------------*/
void CMDS_init(void);
void CMDS_main(void);
void CMDS_final(void);
void CMDS_abend(void);
void CMDS_get_params(void);
void CMDS_read_recv(void);
void CMDS_handle_sys_msg(void);
void CMDS_handle_req_msg(void);
void CMDS_reply(void);
void CMDS_handle_sys_open(void);
void CMDS_handle_sys_close(void);
short CMDS_validate_req_ipc(void);
short CMDS_validate_res_ipc(r502_def* r502, destinfo_def* destinfo);
void CMDS_forward_connctrl(void);
void CMDS_forward_cmdst(void);
void CMDS_forward_logswitch(void);
void CMDS_forward_reload_lin(void);
void CMDS_forward_reload_nsw(void);
void CMDS_inquire_status(void);
void CMDS_forward_process(short* fwd_cnt, short* err_cnt);
void CMDS_setmsgid(short msgid);
void CMDS_getparam(char *prmid, char *val, size_t len);
void CMDS_load_GFPHI();
void CMDS_load_GFLIN();
void CMDS_load_GFNWI();
short CMDS_get_destinfo(gflindata_def* lindt, destinfo_def** dest);
short CMDS_pathsend(destinfo_def* dest, void* conn_id, size_t len);
void CMDS_strtohex(const char* input, char* output, short len);
int CMDS_atoii(const char *, size_t);
void CMDS_strtrim(char *p);
int CMDS_strncmpi(const char *, const char *, size_t);


#endif   // __GFPCVXD0_H__
