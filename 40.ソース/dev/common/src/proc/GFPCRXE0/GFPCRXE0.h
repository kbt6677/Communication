/*****************************************************************************/
/*****                    <<     HEADER PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCRXE0                                    */
/*        FUNCTION          ････ コマンドI/F                                 */
/*                                                                           */
/*                               画面またはINファイルでコマンド入力を受け    */
/*                               付け、入力されたコマンドを精査し、コマン    */
/*                               ドサーバに送信する。                        */
/*                                                                           */
/*                                                                           */
/*        AUTHER            ････ ISYS K.Mishima                              */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2024-10-24                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  K.Mishima  2024/10/24 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
#ifndef __GFPCRXE0_H__
#define __GFPCRXE0_H__
/* ################### */
/* # INCLUDE         # */
/* ################### */
/* STANDARD HEADER */
#include  <stdio.h>    nolist
#include  <stdlib.h>   nolist
#include  <string.h>   nolist
#include  <cextdecs.h> nolist
#include  <ctype.h>    nolist
#include  <zspic>      nolist
#include  <zfilc>      nolist
#include  <zsysc>      nolist
#include  <errno.h>      nolist

/* USER HEADER */
#include "common.h"     nolist      // common
#include "file.h"       nolist      // file
#include "ipc.h"        nolist      // ISYS担当IPC関連
#include "GFPOGGZ4_traceout.h" nolist   // trace
#include "ems.h"        nolist      // メッセージ出力モジュール
#include "errcd.h"      nolist      // エラーコードdefine
#include "GFPCGXD0.h"   nolist      // ASSIGN情報取得
#include "GFPCGXB0.h"   nolist      // IOモジュール
#include "GFPCGX40.h"   nolist      // PATHSENDモジュール
#include "GFPCGXG0.h"   nolist      // プロセス取得モジュール

/* -------------------------------------------------------*/
/* 定数定義                                               */
/* -------------------------------------------------------*/
#define     DEF_LOGICALNAME_GFPHI       "FLNGFPHI"                  // 物理名情報ファイル：論理ファイル名
#define     DEF_LOGICALNAME_CMDSV       "SCNCMDSV"                  // コマンドサーバ論理名

#define     DEF_FLG_OFF                 0               // OFF
#define     DEF_FLG_ON                  1               // ON
#define     DEF_INFILE_BUFFER_LEN       128             // EDITREADバッファ長
#define     DEF_INFILE_NUM              1               // INファイル番号
#define     DEF_CMDIF_PROGRAM_ID        "GFPCRXE0"      // コマンドIFプログラムID
#define     DEF_MODE_EXACT              2               //EXACT
#define     DEF_PROC_NORMAL_END         0               // プロセス正常終了
#define     DEF_PROC_ABNORMAL_END       1               // プロセス異常終了
#define     DEF_INP_PROMPT_MODE         0               // プロンプトコマンドモード
#define     DEF_INP_INFILE_MODE         1               // INファイルコマンドモード
#define     DEF_OUT_PROMPT_MODE         0               // プロンプト出力モード
#define     DEF_OUT_OUTFILE_MODE        1               // OUTファイル出力モード
#define     DEF_EOF_ERROR               -1              // EOFエラーコード
#define     DEF_RET_OK                  0               // リターンコード正常
#define     DEF_RET_NG                  9               // リターンコード異常
#define     DEF_EDITREAD_EOF            -1              // EDITREADリターンEOF
#define     DEF_DATE_MONTH_OFFSET       4               // 日時のMMDDhhmmssの開始位置
#define     DEF_DATE_YY_OFFSET          2               // 日時のYYMMDDhhmmssの開始位置
#define     DEF_SIZE_MMDD               4               // MMDDのサイズ
#define     DEF_SIZE_HHMMSS             6               // yymmssのサイズ
#define     DEF_SIZE_YYMMDD             6               // YYMMDDのサイズ
#define     DEF_ECHO_INIT_GFP           "GFP"
#define     DEF_ECHO_INIT_NW            "NW"



/* 画面出力メッセージコード定義 */
#define     DEF_CODE_COMMAND_SUCCESS         0           //コマンド成功
#define     DEF_CODE_RESPONSE_ERROR          1           //異常応答
#define     DEF_CODE_TIME_OUT                2           //タイムアウト
#define     DEF_CODE_EXCEEDING_UPPER_LIMIT   3           //照会最大数オーバー
#define     DEF_CODE_COMMAND_NOT_FOUND       4           //存在しないコマンド
#define     DEF_CODE_OPTION_ERROR            5           //オプション不正
#define     DEF_CODE_ABEND                   6           //異常終了
#define     DEF_CODE_NORMAL_END              7           //正常終了
/* 画面出力メッセージ定義 */
#define     DEF_MSG_COMMAND_SUCCESS         "COMMAND SUCCESS"                           //コマンド成功
#define     DEF_MSG_RESPONSE_ERROR          "COMMAND FAIL - RESPONSE ERROR"            //異常応答
#define     DEF_MSG_TIME_OUT                "COMMAND FAIL - TIME OUT"                  //タイムアウト
#define     DEF_MSG_EXCEEDING_UPPER_LIMIT   "WARNING - EXCEEDING UPPER-LIMIT"           //照会最大数オーバー
#define     DEF_MSG_COMMAND_NOT_FOUND       "COMMAND FAIL - COMMAND NOT FOUND"         //存在しないコマンド
#define     DEF_MSG_OPTION_ERROR            "COMMAND FAIL - OPTION NOT CORRECT"        //オプション不正
#define     DEF_MSG_ABEND                   "PROGRAM ABEND"                             //異常終了
#define     DEF_MSG_NORMAL_END              "PROGRAM NORMAL END"                        //正常終了
/* HELPメッセージ定義 */
#define     DEF_HELP_MSG_TCP_CONNECT        "TCP_OPEN       <site>-<nw>-<gr>[-<if>[-<st>[-<cn>]]]"
#define     DEF_HELP_MSG_TCP_CLOSE          "TCP_CLOSE      <site>-<nw>-<gr>[-<if>[-<st>[-<cn>]]]"
#define     DEF_HELP_MSG_TCP_LISTEN         "TCP_LISTEN     <site>-<nw>-<gr>[-<if>[-<st>[-<cn>]]] { START | END }"
#define     DEF_HELP_MSG_INFO_TCP_STATE     "STATUS_TCP     <site>-<nw>-<gr>[-<if>[-<st>[-<cn>]]]"
#define     DEF_HELP_MSG_SIGN_ON_CN_ID      "SIGN_ON        <site>-<nw>-<gr>[-<if>[-<st>[-<cn>]]] [ FORCE | UPDATE ]"
#define     DEF_HELP_MSG_SIGN_ON_IF_NAME    "SIGN_ON        -I<if-name> [ FORCE | UPDATE ]"
#define     DEF_HELP_MSG_SIGN_ON_ST_NAME    "SIGN_ON        -S<st-name> [ FORCE | UPDATE ] "
#define     DEF_HELP_MSG_SIGN_OFF_CN_ID     "SIGN_OFF       <site>-<nw>-<gr>[-<if>[-<st>[-<cn>]]] [ FORCE | UPDATE ]"
#define     DEF_HELP_MSG_SIGN_OFF_IF_NAME   "SIGN_OFF       -I<if-name> [ FORCE | UPDATE ]"
#define     DEF_HELP_MSG_SIGN_OFF_ST_NAME   "SIGN_OFF       -S<st-name> [ FORCE | UPDATE ] "
#define     DEF_HELP_MSG_INFO_STATE         "STATUS_SERVICE <site>-<nw>-<gr>[-<if>[-<st>[-<cn>]]]"
#define     DEF_HELP_MSG_ECHO               "ECHO           <site>-<nw>-<gr>-<if>[-<st>[-<cn>]]"
#define     DEF_HELP_MSG_INFO_ECHO          "STATUS_ECHO    <site>-<nw>-<gr>-<if>[-<st>[-<cn>]]"
#define     DEF_HELP_MSG_KEY_REQUEST        "KEY_REQUEST    <site>-<nw>-<gr>-<if>[-<st>]"
#define     DEF_HELP_MSG_KEY_PUSH           "KEY_PUSH       <site>-<nw>-<gr>-<if>[-<st>]"
#define     DEF_HELP_MSG_LOG_ROTATE         "LOG_ROTATE     <site>-<nw>-<gr>"
#define     DEF_HELP_MSG_RELOAD_GFLIN       "RELOAD_GFLIN   <site>-<nw>-<gr>-<if>[-<st>] [<serverclass_name>]"
#define     DEF_HELP_MSG_RELOAD_GFNSW       "RELOAD_GFNSW"
#define     DEF_HELP_MSG_EXIT               "EXIT"
#define     DEF_HELP_MSG_DELAY              "DELAY          <time(100ms)>"
#define     DEF_HELP_MSG_COMMENT            "=="

/* コマンド名定義 */
#define     DEF_TCP_CONNECT         "TCP_OPEN"
#define     DEF_TCP_CLOSE           "TCP_CLOSE"
#define     DEF_TCP_LISTEN          "TCP_LISTEN"
#define     DEF_INFO_TCP_STATE      "STATUS_TCP"
#define     DEF_SIGN_ON             "SIGN_ON"
#define     DEF_SIGN_OFF            "SIGN_OFF"
#define     DEF_INFO_STATE          "STATUS_SERVICE"
#define     DEF_ECHO                "ECHO"
#define     DEF_INFO_ECHO           "STATUS_ECHO"
#define     DEF_KEY_REQUEST         "KEY_REQUEST"
#define     DEF_KEY_PUSH            "KEY_PUSH"
#define     DEF_LOG_ROTATE          "LOG_ROTATE"
#define     DEF_RELOAD_GFLIN        "RELOAD_GFLIN"
#define     DEF_RELOAD_GFNSW        "RELOAD_GFNSW"
#define     DEF_EXIT                "EXIT"
#define     DEF_DELAY               "DELAY"
#define     DEF_HELP                "HELP"
/* モード名定義 */
#define     DEF_MODE_START          "START"
#define     DEF_MODE_END            "END"
#define     DEF_MODE_FORCE          "FORCE"
#define     DEF_MODE_UPDATE         "UPDATE"

/* 照会コマンド共通ラベル */
#define     DEF_RABEL_GROUP_ID          "GROUP-ID"
#define     DEF_RABEL_INTERFACE_ID      "INTERFACE-ID"
#define     DEF_RABEL_MANAGEMENT_ID     "MANAGEMENT-ID"

/* コネクションステータス照会コマンドラベル定義 */
#define     DEF_RABEL_CONNECTION_ID     "CONNECTION-ID"
#define     DEF_RABEL_CONNECT_STS       "ST"
#define     DEF_RABEL_DISCONNECT_RSN    "RS"
#define     DEF_RABEL_ERR_CODE          "ERR"
#define     DEF_RABEL_IP_ADDRESS_SRC    "LOCAL-ADDR"
#define     DEF_RABEL_PORT_NUM_SRC      ":PORT"
#define     DEF_RABEL_IP_ADDRESS_DST    "REMOTE-ADDR"
#define     DEF_RABEL_PORT_NUM_DST      ":PORT"
#define     DEF_RABEL_LAST_MODIFIED     "LASTMODIFIED"

/* 局状態照会コマンドラベル定義 */
#define     DEF_RABEL_STATION_STS           "ST"
#define     DEF_RABEL_STATE_UPDATE_DATE     "LASTMODIFIED"
#define     DEF_RABEL_INTERFACE_NAME        "INTERFACE-NAME"
#define     DEF_RABEL_STATION_NAME          "STATION-NAME"
/* エコーステータス照会コマンドラベル定義 */
#define     DEF_RABEL_ECHO_INIT             "INIT"
#define     DEF_RABEL_ECHO_RESULT           "RSLT"
#define     DEF_RABEL_ECHO_LAST_REQUEST     "LAST_REQUEST"
#define     DEF_RABEL_ECHO_LAST_RESPONSE    "LAST_RESPONSE"
#define     DEF_RABEL_ECHO_LAST_SUCCESS     "LAST_SUCCESS"

#define     DEF_ERROR_CODE_EXCEEDING_UPPER_LIMIT    "WCFQ001"                 //最大数超過の内部エラーコード

#define     DEF_TRC_TYPE_OPEN           "OPEN"
#define     DEF_TRC_TYPE_CLOSE          "CLOSE"
#define     DEF_TRC_TYPE_READ           "READ"

#define     DEF_LONG_MAX                "2147483647"
#define     DEF_SHORT_MAX               "32767"

#define     DEF_COMPLETION_ERROR        8999
#define     DEF_COMPLETION_NORMAL       8000

/* -------------------------------------------------------*/
/* 構造体のtypedef定義                                    */
/* -------------------------------------------------------*/
// I/O完了情報

typedef struct __connection_id
{
    char             site_name;                 /* サイト識別 */
    char             nw_name;                   /* N/W識別 */
    char             group_name[5];             /* グループ識別 */
    char             interface_name[5];         /* インタフェース識別 */
    char             station_name[6];           /* ステーション識別 */
    char             connection_name[6];        /* コネクション識別 */
}connection_id_def;


typedef struct __infile_info
{
    char infile_name[48];                       /* インファイル名 */
    short infile_num;                           /* インファイル番号 */
    short edit_controlblk[104];                 /* コントロールブロック */
    char buffer_command[128];                   /* コマンド入力バッファ */
    short buffer_command_length;                /* コマンド入力長 */
    int sequence_num;                           /* シーケンスナンバー */
}infile_info_def;


typedef struct __my_info
{
    short end_flag;                             /* 終了フラグ */
    short startup_mode;                         /* 起動モード */
    short output_mode;                          /* 出力モード */
    char cmd_srv_domain_name[8];                /* コマンドサーバドメイン名 */
    char cmd_srv_serverclass_name[32];          /* コマンドサーバサーバクラス名 */
    char ems_mon_name[14];                      /* 運用監視端末PATHMON名 */
    char ems_srv_name[13];                      /* 運用監視端末サーバクラス名 */
    short continue_flag;                        /* 処理続行フラグ */
    struct
    {
        char site_id;                           /* サイト識別 */
        char nw_id;                             /* NW識別 */
        char grp_id[5];                         /* グループ識別 */
    } my_env;
    short completion_code;                      /* コンプリッションコード */
    procinfo_def procinfo;                      /* プロセス情報 */ 
    
}my_info_def;

typedef struct __command_sep
{
    char input_command[81];                     /* 入力コマンド */
    char separate_command[4][40];               /* 分割後コマンド */
}command_sep_def;

typedef struct __timer_value
{
    long pathsend_timer;                        /* Pathsendタイマー */
    short pathsend_retry_num;                   /* Pathsendリトライ回数 */
    long io_timer;                              /* ファイルIOタイマー */
}timer_value_def;

typedef struct __connect_sts_display
{
    char connection_id[14];                     /* コネクション論理ID */
    char connect_sts[3];                        /* コネクションステータス */
    char disconnect_rsn[3];                     /* 切断理由 */
    char err_code[5];                           /* エラーコード */
    char ip_address_src[15];                    /* 接続元IPアドレス */
    char port_num_src[7];                       /* 接続元ポート番号 */
    char ip_address_dst[15];                    /* 接続先IPアドレス */
    char port_num_dst[6];                       /* 接続先ポート番号 */
    char sts_update_time[12];                   /* プロセス状態変更日時 */
    char end_null;                              /* NULLストップ */
}connect_sts_display;

typedef struct __state_sts_display
{
    char connection_id[20];                     /* コネクション論理ID */
    char station_sts[3];                        /* 局状態 */
    char state_sts_update_time[14];             /* 局状態更新日時 */
    char interface_name[21];                    /* インターフェース名 */
    char station_name[12];                      /* ステーション名 */
    char end_null;                              /* NULLストップ */
}state_sts_display;

typedef struct __echo_sts_display
{
    char connection_id[20];                     /* コネクション論理ID */
    char echo_init[5];                          /* エコー契機 */
    char echo_result[5];                        /* エコー結果 */
    char echo_last_request_time[14];            /* 接続先契機直前最終エコー完了日時 */
    char echo_last_response_time[14];           /* 接続先契機直前最終エコー結果 */
    char echo_last_success_time[14];            /* 接続先契機直前最終エコー結果 */
    char end_null;                              /* NULLストップ */
}echo_sts_display;


/* -------------------------------------------------------*/
/* 関数プロトタイプ宣言                                   */
/* -------------------------------------------------------*/
int main();
void CMDI_init();
void CMDI_main();
void CMDI_finish();
void CMDI_get_params();
void CMDI_input_prompt_cmd();
void CMDI_input_infile_cmd();
void CMDI_select_command();
short CMDI_open_tcp(char*,char*);
short CMDI_close_tcp(char*,char*);
short CMDI_listen_tcp(char*,char*);
short CMDI_status_tcp(char*,char*);
short CMDI_sign_on(char*,char*);
short CMDI_sign_off(char*,char*);
short CMDI_status_service(char*,char*);
short CMDI_echo(char*,char*);
short CMDI_status_echo(char*,char*);
short CMDI_request_key(char*,char*);
short CMDI_push_key(char*,char*);
short CMDI_rotate_log(char*,char*);
short CMDI_reload_gflin(char*,char*);
short CMDI_reload_gfnsw(char*,char*);
short CMDI_display_help(char*,char*);
short CMDI_exit_command(char*,char*);
short CMDI_delay_command(char*,char*);
short CMDI_sep_identifier(char*,connection_id_def*);
void CMDI_output_msg(short);
void CMDI_abend();
void CMDI_sep_check_command(char* );
void CMDI_edit_ipc(char*,c501_def*);
void CMDI_edit_pathsend(COM_PSD_arg_1_def*,COM_PSD_arg_2_def*,c501_def*,short,short);
int CMDI_isStrDigit(char*);
int CMDI_strcmpi(const char*, const char*);

#endif   // __GFPCRXE0_H__
