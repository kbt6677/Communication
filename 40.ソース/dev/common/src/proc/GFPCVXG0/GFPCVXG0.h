/*****************************************************************************/
/*****                    <<     HEADER PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCVXG0                                    */
/*        FUNCTION          ････ GFP内部LCN採番                              */
/*                                                                           */
/*                               電文振分(inbound)、制御電文機能サーバから   */
/*                               GFP内部LCN採番要求を受信し、                */
/*                               採番したGFP内部LCNを                        */
/*                               要求元にREPLAYする。                        */
/*                                                                           */
/*        AUTHER            ････ ISYS Y.Kawasaki                             */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2024-09-17                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  Y.Kawasaki 2024/09/17 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
#ifndef __GFPCVXG0_H__
#define __GFPCVXG0_H__

/* ################### */
/* # INCLUDE         # */
/* ################### */
/* STANDARD HEADER */
#include  <zsysc>        nolist
/* USER HEADER */
#include "common.h"      nolist      // common
#include "GFPCGXG0.h"    nolist      // プロセス情報取得
#include "GFPCGXC0.h"    nolist      // サーバ停止判定

/* -------------------------------------------------------*/
/* 定数定義                                               */
/* -------------------------------------------------------*/
#define     DEF_RECVFILE_NAME           "$RECEIVE"                  // $RECEIVEファイル名
#define     DEF_FL_TRACE                "FLNTRACE"                  // トレースファイル：論理ファイル名
#define     DEF_MY_PROGID               DEF_GFPCVXG0                // 自プログラムID
#define     DEF_MY_SCID                 DEF_SC_GFP_LCN              // 自サーバクラス論理ID
#define     DEF_GFPLCN_RESERVE_VAL      "0"                         // GFP内部LCN予備値
#define     DEF_EMS_DEF_RET_CODE        "0"                         // EMS出力モジュールデフォルトリターンコード
#define     DEF_NERR_EMPTY              "       "                   //
#define     DEF_NUM4DIGIT0              "0000"
#define     DEF_NUM4DIGIT1              "0001"

// サイト識別
#define     DEF_FEP_SITE_ID_TKY         'T'             // FEP東京
#define     DEF_FEP_SITE_ID_OSK         'O'             // FEP大阪

#define     DEF_NW_ID_CBSP              'P'             // CBSポータル
#define     DEF_NW_ID_FEP               'F'             // FEP
#define     DEF_NW_ID_DTP               'T'             // CDTP

#define     DEF_FLG_OFF                 0               // OFF
#define     DEF_FLG_ON                  1               // ON
#define     DEF_PROC_NORMAL_END         0               // プロセス正常終了
#define     DEF_PROC_ABNORMAL_END       1               // プロセス異常終了

#define     DEF_OPENID_ANCESTOR         1               // オープンID(親プロセス)
#define     DEF_OPENID_ROUT             2               // オープンID(ROUT)
#define     DEF_RCV_BUF_SIZE            4096            // 受信バッファサイズ
#define     DEF_RPL_BUF_SIZE            4096            // 応答バッファサイズ
#define     DEF_RECV_DEPTH_MAX          4047            // $RECEIVE 受信メッセージの最大数
#define     DEF_START_DELAY_CS          10000           // 起動時DELAY時間(1/100秒)
#define     DEF_PROC_SEQNUM_MIN         0               // 連番最小値
#define     DEF_PROC_SEQNUM_MAX         8191            // 連番最代値
#define     DEF_SEQNUM_BIT_BOUNDARY     5               // 32進変換データビット境界
#define     DEF_SEQNUM_BIT_MSK_VALUE    0x1f            // 32進変換用5ビットマスク値
#define     DEF_SC_OFFSET_SITE          0               // site_idオフセット
#define     DEF_SC_OFFSET_NW            2               // network_idオフセット
#define     DEF_SC_OFFSET_GRP           4               // group_idオフセット
#define     DEF_SC_OFFSET_SCKND         10              // serverclass種類オフセット
#define     DEF_SC_OFFSET_SCNUM         19              // serverclass論理番号オフセット
#define     DEF_SC_KIND_LEN             8               // サーバクラス種類レングス
#define     DEF_CHK_IPC_OK              0               // IPC内容精査：OK
#define     DEF_CHK_IPC_IFCD_ERR        1               // IPC内容精査：I/Fコードエラー
#define     DEF_CHK_IPC_DLEN_ERR        2               // IPC内容精査：データ長エラー
#define     DEF_CHK_IPC_SITE_ERR        3               // IPC内容精査：採番システムエラー
#define     DEF_CHK_IPC_NW_ERR          4               // IPC内容精査：場所エラー

//// 暫定共通モジュール用 /////////////////////////////////////////////////
// ★COM_SDT
#define     DEF_COM_SDT_CHG_TYPE_GMT    1               // グリニッジ標準時
#define     DEF_COM_SDT_CHG_TYPE_JST    2               // 日本時間
#define     DEF_COM_SDT_CHG_TYPE_CST    3               // 中国時間
// ★COM_IOM
#define     DEF_COM_IOM_FIO_OPEN        "OPEN"
#define     DEF_COM_IOM_FIO_START       "START"
#define     DEF_COM_IOM_FIO_READ        "READ"
#define     DEF_COM_IOM_FIO_CLOSE       "CLOSE"

//// EMS出力内容 /////////////////////////////////////////////////
#define     DEF_EMS_IPC_ERR   "IPCﾁｪｯｸｴﾗｰ"
#define     DEF_EMS_IPC_CHK_IFCD            "ｲﾝﾀｰﾌｪｰｽｺｰﾄﾞ"
#define     DEF_EMS_IPC_CHK_LEN             "ﾚﾝｸﾞｽ"
#define     DEF_EMS_IPC_CHK_SITE            "ｻｲﾊﾞﾝｼｽﾃﾑ"
#define     DEF_EMS_IPC_CHK_NW              "ﾊﾞｼｮ"


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

// 連番情報
typedef struct __seqnum
{
    short    seqnum;                                    // 現在の連番値
    short    seqnum_min;                                // 連番最小値
    short    seqnum_max;                                // 連番最大値
} seqnum_def;

// 応答情報
typedef struct __replyinfo_st
{
    char     replybuf[DEF_RPL_BUF_SIZE];                // 応答バッファ
    short    reply_len;                                 // 応答レングス
    short    reply_code;                                // 応答コード
} replyinfo_def;

// 自プロセス情報
typedef struct __myinfo
{
    short    recv_fno;                                  // $RECEIVEファイル番号
    char*    recv_buf;                                  // READUPDATE用バッファーポインタ
    char     pathmon_name[ZSYS_VAL_LEN_PROCESSNAME+1];  // PATHMONプロセス名
    short    end_flg;                                   // プロセス終了フラグ
    //short    trace_flg;                                 // トレースフラグ
    char     site_id[1];                                // サイト識別
    char     network_id[1];                             // N/W識別
    char     group_id[5];                               // グループ識別
    char     serverclass_id[12];                        // サーバクラス論理ID
    char     serverclass_name[6];                       // サーバクラス名
    char     recvbuf[DEF_RCV_BUF_SIZE];                 // 受信バッファ
    procinfo_def        procinfo;                       // プロセス情報
    iocomp_def          iocomp;                         // I/O完了情報
    seqnum_def          seqnum;                         // 連番情報
    COM_STP_arg_1_def   stp_arg;                        // サーバ停止判定I/F
    replyinfo_def       replyinfo;                      // reply情報
} myinfo_def;

/* -------------------------------------------------------*/
/* 関数プロトタイプ宣言                                   */
/* -------------------------------------------------------*/
void LCNN_init(void);
void LCNN_main(void);
void LCNN_final(void);
void LCNN_abend(void);
void LCNN_get_params(void);
void LCNN_read_recv(void);
void LCNN_handle_sys_msg(void);
void LCNN_handle_req_msg(void);
void LCNN_reply(void);
void LCNN_handle_sys_open(void);
void LCNN_handle_sys_close(void);
short LCNN_validate_ipc(char*);
void LCNN_number_gfplcn(void);
void LCNN_cnv32d(unsigned long, char*, short);
int LCNN_procnamecmp(const procinfo_def *p1, const char *p2, size_t n);
int LCNN_strncmpi(const char *, const char *, size_t);
void LCNN_setmsgid(short msgid);
void LCNN_strtohex(const char* input,char* output,short len);

#endif   // __GFPCVXG0_H__
