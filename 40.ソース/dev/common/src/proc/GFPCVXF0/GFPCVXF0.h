/*****************************************************************************/
/*****                    <<     HEADER PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCVXF0                                    */
/*        FUNCTION          ････ ログ出力                                    */
/*                                                                           */
/*                               電文振分(inbound)、電文振分(outbound)から   */
/*                               電文ログ出力要求を受けて                    */
/*                               NW通信ログにログを出力する。                */
/*                                                                           */
/*        AUTHER            ････ ISYS K.Tanigawa                             */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2024-09-26                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  K.Tanigawa 2024/09/26 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
#ifndef __GFPCVXF0_H__
#define __GFPCVXF0_H__

/* ################### */
/* # INCLUDE         # */
/* ################### */
/* STANDARD HEADER */
#include  <zsysc>        nolist
/* USER HEADER */
#include "common.h"      nolist      // common
#include "GFPCGXG0.h"    nolist      // プロセス情報取得
#include "GFPCGXC0.h"    nolist      /* オープナープロセス管理 */

/* -------------------------------------------------------*/
/* 定数定義                                               */
/* -------------------------------------------------------*/
/* マクロprefix DEF_ */
#define     DEF_RECEIVE_NAME            "$RECEIVE"                  /* $RECEIVEファイル名                   */
#define     DEF_FL_TRACE                "FLNTRACE"                  /* トレースファイル：論理ファイル名     */
#define     DEF_MY_PROGID               DEF_GFPCVXF0                /* プログラムID                         */
#define     DEF_MY_SCID                 DEF_SC_LOG_OUT              /* サーバクラスID                         */
#define     DEF_EMS_DEF_RET_CODE        "0"                         // EMS出力モジュールデフォルトリターンコード
#define     DEF_GFPCGXG0                "GFPCGXG0"

#define     DEF_RCV_BUF_SIZE            20000           /* 要求バッファサイズ                   */
#define     DEF_RPL_BUF_SIZE            4096            /* 応答バッファサイズ                   */
#define     DEF_RECV_DEPTH_MAX          4047            // $RECEIVE 受信メッセージの最大数
#define     DEF_NOWAIT_DEPTH            1               // $RECEIVE 受信メッセージの最大数
#define     DEF_OPENID_ANCESTOR         1               /* オープンID(親プロセス)               */
#define     DEF_OPENID_ROUT             2               /* オープンID(ROUT)                     */
#define     DEF_GFPHI_COMP_KEYLEN       33              /* 物理名情報ファイルCOMPARE長          */
#define     DEF_GFPHI_DT                "DT"            /* 物理名情報ファイル                   */
                                                        /* プロセス／ファイル論理番号上2桁      */
#define     DEF_RET_OK                  0               /* 正常応答                             */
#define     DEF_RET_PKEY                10              /* PKEY重複                             */
#define     DEF_RET_AKEY                551             /* AKEY重複                             */
#define     DEF_RET_FFUL                45              /* ファイルフル                         */
#define     DEF_RET_UPDATE              11              /* 更新対象未存在                       */
#define     DEF_RET_EOF                  1              /* 更新対象未存在                       */
#define     DEF_RET_NG                   1              /* エラー応答                           */
#define     DEF_OFFSET_YES              -1              /* 前日のオフセット                     */
#define     DEF_OFFSET_TOD              0               /* 当日のオフセット                     */
#define     DEF_OFFSET_TOM              1               /* 翌日のオフセット                     */
#define     DEF_IDX_YES                 0               /* 前日のNW通信ログ区分                 */
#define     DEF_IDX_TOD                 1               /* 当日のNW通信ログ区分                 */
#define     DEF_IDX_TOM                 2               /* 翌日のNW通信ログ区分                 */
#define     DEF_IDX_SET                 3               /* 指定ログのNW通信ログ区分             */
#define     DEF_FILENAME_LEN            47              /* ファイル名の長さ                     */
#define     DEF_SC_OFFSET_SITE          0               // site_idオフセット
#define     DEF_SC_OFFSET_NW            2               // network_idオフセット
#define     DEF_SC_OFFSET_GRP           4               // group_idオフセット
#define     DEF_SC_OFFSET_SCKND         10              // serverclass種類オフセット
#define     DEF_SC_OFFSET_SCNUM         19              // serverclass論理番号オフセット
#define     DEF_SC_KIND_LEN             8               // サーバクラス種類レングス
#define     DEF_FLG_OFF                 0               // OFF
#define     DEF_FLG_ON                  1               // ON
#define     DEF_PROC_NORMAL_END         0               // プロセス正常終了 
#define     DEF_PROC_ABNORMAL_END       1               // プロセス異常終了

#define     DEF_IF_COM_NAME             "3010"          /* コマンド要求のコマンド識別           */
#define     DEF_ERCD_NOM                0               /* エラーコード:正常応答                */
#define     DEF_ERCD_LCN                1               /* エラーコード:LCN重複                 */
#define     DEF_ERCD_OTHER              9               /* エラーコード:拒否（異常）応答        */

#define     DEF_SEV_ID                  "{"             /* サーバクラス論理ID                   */

#define     DEF_ENTRY_CATE              1               /* 登録更新区分：登録                   */
#define     DEF_UPDATE_CATE             2               /* 登録更新区分：更新                   */

//// 暫定共通モジュール用 /////////////////////////////////////////////////
// ★COM_OPP
#define     DEF_COM_OPP_FUNC_TYPE_INI   1               // オープナープロセス管理機能名識別：初期化
#define     DEF_COM_OPP_FUNC_TYPE_CNT   2               // オープナープロセス管理機能名識別：カウンタ管理
// ★COM_IOM TRACE用 file_io_type
#define     DEF_FLIOTYPE_OPEN          "OPEN"           /* トレースIO：OPEN         */
#define     DEF_FLIOTYPE_START         "START"          /* トレースIO：START        */
#define     DEF_FLIOTYPE_READ          "READ"           /* トレースIO：READ         */
#define     DEF_FLIOTYPE_WRITE         "WRITE"          /* トレースIO：WRITE        */
#define     DEF_FLIOTYPE_CLOSE         "CLOSE"          /* トレースIO：CLOSE        */

//// EMS出力内容 /////////////////////////////////////////////////
#define     DEF_EMS_RCV_LEN_ERR         "ｼﾞｭｼﾝﾃﾞｰﾀﾁｮｳｴﾗｰ"
#define     DEF_EMS_CATEGORY_ERR        "ｸﾌﾞﾝﾁｪｯｸｴﾗｰ"
#define     DEF_EMS_COMMAND_NAME_ERR    "ｺﾏﾝﾄﾞﾒｲﾁｪｯｸｴﾗｰ"
#define     DEF_EMS_DATE_ERR            "ﾋﾂﾞｹﾁｪｯｸｴﾗｰ"

/* -------------------------------------------------------*/
/* 構造体のtypedef定義                                    */
/* -------------------------------------------------------*/
/* I/O完了情報 */
typedef struct __iocomp_def
{
    short    fno;                                    /* 完了ファイル番号     */
    long     l_addr;                                 /* バッファアドレス     */
    short    len;                                    /* 完了サイズ           */
    long     l_tag;                                  /* 完了タグ             */
    short    errno;                                  /* エラーコード         */
    zsys_ddl_receiveinformation_def  recv_info;      /* $RECEIVE完了情報     */
} iocomp_def;

/* オープナープロセス管理情報 */
typedef struct __openers_mgr_def
{
    char     sys_msg[DEF_RCV_BUF_SIZE];              /* システムメッセージ   */
    COM_STP_arg_1_def  opp_arg1;                     /* オープナー情報       */
} openers_mgr_def;

/* 自プロセス情報 */
typedef struct __myinfo_def
{
    short    recv_fno;                               /* $RECEIVEファイルNO   */
    char     site_id[1];                             /* サイト識別           */
    char     network_id[1];                          /* N/W識別              */
    char     group_id[5];                            /* グループ識別         */
    char     serverclass_id[12];                     /* サーバクラス論理ID   */
    char     serverclass_name[6];                    /* サーバクラス名       */
    char     recvbuf[DEF_RCV_BUF_SIZE];              /* 受信バッファ         */
    short    end_flg;                                /* 終了フラグ           */
    short    trace_flg;                              /* トレースフラグ       */
    long     fio_timer;                              /* ファイルI/Oタイマー  */
    procinfo_def        procinfo;                    /* プロセス情報         */
    iocomp_def           iocomp;                     /* I/O完了情報          */
    openers_mgr_def      op_mgr;                     /* オープナープロセス管理情報 */
} myinfo_def;

/* 物理名情報ファイルから取得したNW通信ログファイル名 */
typedef struct __name_table_def
{
    char     log_fname[48];
} name_table_def;

/* NW通信ログ情報 */
typedef struct __fileinfo_def
{
    short    file_idx;                                /* 出力対象ファイルIDX  */
    char     reply_buf[DEF_RPL_BUF_SIZE];             /* リプライバッファ     */
    short    reply_len;                               /* リプライコード       */
    short    reply_code;                              /* リプライコード       */
    char     open_log_date[8];                        /* カレント日付         */
    struct {
        char     log_fname[48];
        short    file_no;
    } log_table[4];
} fileinfo_def;

/* -------------------------------------------------------*/
/* 関数プロトタイプ宣言                                   */
/* -------------------------------------------------------*/
void  LOGS_init(void);
void  LOGS_control(void);
void  LOGS_recv(void);
void  LOGS_sys_msg(void);
void  LOGS_open_sys(void);
void  LOGS_close_sys(void);
void  LOGS_req_msg(void);
void  LOGS_req_log(void);
void  LOGS_req_cmd(void);
void  LOGS_get_day( char *baseday, short offset, short *day);
void  LOGS_open_log( char *filename, short *filenum );
void  LOGS_close_log(char *filename, short filenum);
void  LOGS_getname_log(void);
short LOGS_write_log(void);
short LOGS_update_log(void);
void  LOGS_edit_reply( short errcode, char* in_errcode, char* lcn, char* frwk_kbn);
void  LOGS_recv_reply(void);
void  LOGS_final(void);
void  LOGS_abend(void);
void  LOGS_setmsgid(short msgid);
void  LOGS_strtohex(const char* input,char* output,short len);

#endif
