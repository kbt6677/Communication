/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVX00                                    */
/*        FUNCTION          ････ リスナー                                    */
/*                               サーバ接続機能/リスナー機能                 */
/*                                                                           */
/*        AUTHER            ････ HAS M.Matsumoto                             */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2024-10-18                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  Matsumoto  2024/10/19 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/

#ifndef _GFPCVX00_H
#define _GFPCVX00_H

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
#include "limit.h"
#include "file.h(db_gflin)"
#include "file.h(db_gclst)"
#include "file.h(db_gfphi)"
#include "file.h(db_gfnwi)"
#include "file.h(db_gcscn)"
#include "file.h(db_glnlg)" // ← ipc.hで必要のため
#include "ipc.h"
#include "common.h"
#include "ems.h"
#include "errcd.h"

#include "vproc.h"
#include "GFPCGXD0.h"       // ASSIGN情報取得
#include "GFPCGXC0.h"       // オープナープロセス管理
#include "GFPCGXB0.h"       // IOモジュール
#include "GFPCGXA0.h"       // トランザクション管理
#include "GFPCGX50.h"       // システム日時取得
#include "GFPCGX60.h"       // IOタグ生成・解析
#include "GFPCGXG0.h"       // プロセス情報取得

#include "GFPOGGZ3_encode.h"        // コード変換
#include "GFPOGGZ4_traceout.h"      // トレース出力

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
/* 通信内部エラーコード */
//#define DEF_NERR_LSTN_DST_ADRS_CHK_ERR        "SCBB001"       /* 接続先アドレスチェックエラー             */
//#define DEF_NERR_LSTN_CON_NUM_OVER            "SCBB002"       /* コネクション数オーバー検知               */
//#define DEF_NERR_LSTN_ACC_RETRY_OVER          "SCBB003"       /* ACCEPTリトライオーバー                   */
//#define DEF_NERR_LSTN_FREE_CON_NON            "SCBB004"       /* 接続待ちコネクション無し                 */
//#define DEF_NERR_LSTN_ALL_TBL_CON             "SCBB005"       /* 全コネクション接続済                     */

/* 接続拒否理由 */
#define DEF_REJECT_CON_NUM_OVER             "ｺﾈｸｼｮﾝｽｳｵｰﾊﾞｰ"
#define DEF_REJECT_FREE_CON_NON             "ｾﾂｿﾞｸﾏﾁｺﾈｸｼｮﾝﾅｼ"
#define DEF_REJECT_ALL_TBL_CON              "ｾﾞﾝｺﾈｸｼｮﾝｾﾂｿﾞｸｽﾞﾐ"
#define DEF_REJECT_ADDR_CHK_ERR             "ｾﾂｿﾞｸｻｷｱﾄﾞﾚｽﾁｪｯｸｴﾗｰ"

/* 共通EMS出力サブルーチン */
#define DEF_CG010_NOR               '0'
#define DEF_CG010_SVRERR            '1' // server error
#define DEF_CG010_TCSERR            '2' // tacspro error
#define DEF_CG010_PRMERR            '8' // parameter error
#define DEF_CG010_SNDERR            '9' // pathsend error

/* 共通ファイルI/Oモジュール */
#define DEF_FILEIO_OPEN             "OPEN    "
#define DEF_FILEIO_START            "START   "
#define DEF_FILEIO_READ             "READ    "
#define DEF_FILEIO_WRITE            "WRITE   "
#define DEF_FILEIO_REWRITE          "REWRITE "
#define DEF_FILEIO_DELETE           "DELETE  "
#define DEF_FILEIO_UNLOCK           "UNLOCK  "
#define DEF_FILEIO_CLOSE            "CLOSE   "

#define DEF_POSITION_APPROXIMATE    0
#define DEF_POSITION_GENERIC        1
#define DEF_POSITION_EXACT          2

/****************************************************************************/
/*   DEFINE定義(リスナー)                                                   */
/****************************************************************************/
/* プログラム制御 */
#define LSTN_CONTINUE               0                   /* 処理継続                             */
#define LSTN_TERMINATE              1                   /* 処理終了                             */

#define LSTN_NORMAL_TERMINATION     0                   /* 正常終了                             */
#define LSTN_ABNORMAL_TERMINATION   1                   /* 異常終了                             */

/* マトリクスステート */
#define LSTN_ST_INIT                0                   /* 初期状態                             */
#define LSTN_ST_DISCONNECT          1                   /* 切断状態                             */
#define LSTN_ST_ACCEPT              2                   /* 接続待ち状態                         */
#define LSTN_ST_RETRY               3                   /* リトライ待ち状態                     */
#define LSTN_ST_RECONNECT           4                   /* 切断待ち状態                         */
#define LSTN_ST_TERMINATE           9                   /* 終了状態                             */

/* マトリクスイベント */
#define LSTN_EV_NONE                0                   /* イベント無し                         */
#define LSTN_EV_SYSOPEN             1                   /* オープンメッセージ受信               */
#define LSTN_EV_SYSCLOSE            2                   /* クローズメッセージ受信               */
#define LSTN_EV_SYSTIMEOUT          3                   /* タイムアウトメッセージ受信           */
#define LSTN_EV_SYSCPUDOWN          4                   /* CPUダウンメッセージ受信              */
#define LSTN_EV_CTLASYNC            11                  /* 接続通知用非同期要求                 */
#define LSTN_EV_CTLACCEPT           12                  /* コネクション接続開始要求             */
#define LSTN_EV_CTLCONNECT          13                  /* コネクション接続完了通知要求         */
#define LSTN_EV_CTLDISCONNECT       14                  /* コネクション切断完了通知要求         */
#define LSTN_EV_CTLRECONNECT        15                  /* コネクション入替・切断完了通知要求   */
#define LSTN_EV_SKTACCEPT           21                  /* ACCEPT完了                           */
#define LSTN_EV_SKTERROR            22                  /* ACCEPTエラー                         */
#define LSTN_EV_CMDLISTENSTART      31                  /* コマンド要求(リスナー開始)           */
#define LSTN_EV_CMDLISTENSTOP       32                  /* コマンド要求(リスナー終了)           */
#define LSTN_EV_CMDFILERELOAD       33                  /* コマンド要求(ファイル再読込み)       */

/* 管理テーブル最大数 */
#define LSTN_LISTENPORT_TBL_MAX     LSN_MAX_interface   /* リスンポート管理テーブル             */
#define LSTN_CONTROLSERVER_TBL_MAX  LSN_MAX_connection  /* コネクション制御管理テーブル         */
#define LSTN_LISTENER_TBL_MAX       LSN_MAX_interface   /* リスナー管理テーブル                 */
#define LSTN_CONNECTION_TBL_MAX     LSN_MAX_ports       /* コネクション管理テーブル             */
#define LSTN_COUNT_TBL_MAX          LSN_MAX_connection  /* コネクション数管理テーブル           */
#define LSTN_CONTROLSERVER_QUE_MAX  30                  /* コネクション制御キュー               */

/* I/O・テーブル管理 */
#define LSTN_RECEIVE_FILENAME       "$RECEIVE"          /* $RECEIVEファイル名                   */
#define LSTN_NOWAITDEPTH            1                   /* NOWAIT DEPTH                         */
#define LSTN_RECVDEPTH              31                  /* $RECEIVE DEPTH                       */
#define LSTN_COLLECTOR_FILENAME     "$0"                /* EMSコレクター名                      */

/* リスンポートオープン結果 */
#define LSTN_OPEN_NORMAL            0                   /* オープン正常                         */
#define LSTN_OPEN_ALREADY           1                   /* オープン済                           */
#define LSTN_OPEN_ERROR             -1                  /* オープンエラー                       */

/* ファイルI/Oパラメータ */
#define LSTN_READ_1ST               1                   /* 初回READ                             */
#define LSTN_READ_NEXT              2                   /* NEXTREAD                             */

#define LSTN_FILEIO_NORMAL          0                   /* ファイル正常                         */
#define LSTN_FILEIO_EOF             1                   /* ファイルEOF                          */
#define LSTN_FILEIO_ERROR           -1                  /* ファイルエラー                       */

/* I/Oタグ */
#define LSTN_TAG_NULL               -1                  /* タグNULL                             */
#define LSTN_TAG_RECV               1                   /* $RECEIVE用I/Oタグ                    */
#define LSTN_TAG_ACCEPT             2                   /* ACCEPT用I/Oタグ                      */
#define LSTN_TAG_SIGNAL             3                   /* SIGNALTIMEOUT用タグ                  */
#define LSTN_TAG_SOCKET             9                   /* ソケット用I/Oタグ                    */
#define LSTN_COMPONENT_LISTEN       0                   /* コンポーネント                       */

#define LSTN_SO_NOWAIT_DEPTH        1                   /* ソケットフラグ                       */
#define LSTN_MIN_LISTEN             1                   /* LISTEN最小値                         */

#define LSTN_ASYNC_USED             12                  /* 非同期要求受信済み                   */
#define LSTN_UNRELATED_TABLE        301                 /* 非同期要求受信済み                   */

/* 検索キー長 */
#define LSTN_KEYLEN_GROUP           7                   /* グループ識別                         */
#define LSTN_KEYLEN_INTERFACE       12                  /* インタフェース識別                   */
#define LSTN_KEYLEN_STATION         18                  /* ステーション識別                     */
#define LSTN_KEYLEN_CONNECTION      24                  /* コネクション識別                     */

/* 管理テーブル */
#define LSTN_FILE_CLOSED            -1                  /* ファイルCLOSE                        */

#define LSTN_TBL_LISTENPORT         1                   /* リスンポート管理テーブル             */
#define LSTN_TBL_LISTENER           2                   /* リスナー管理テーブル                 */
#define LSTN_TBL_CONTROLSERVER      3                   /* コネクションサーバ制御管理テーブル   */
#define LSTN_TBL_CONNECTION         4                   /* コネクション管理テーブル             */
#define LSTN_TBL_COUNTER            5                   /* コネクション数管理テーブル           */

#define LSTN_TBL_NOT_ENTRY          -1                  /* テーブル未登録                       */
#define LSTN_TBL_ONLINE             0                   /* オンラインテーブル                   */
#define LSTN_TBL_RELOAD             1                   /* 再読込みテーブル                     */

#define LSTN_TBLKEYLEN_INTERFACE    5                   /* インタフェース                       */
#define LSTN_TBLKEYLEN_STATION      11                  /* インタフェース～ステーション         */
#define LSTN_TBLKEYLEN_CONNECTION   17                  /* インタフェース～コネクション         */

#define LSTN_GET_NORMAL             0                   /* テーブル検索正常                     */
#define LSTN_GET_ERROR              -1                  /* テーブル検索エラー                   */

#define LSTN_FLG_ON                 1                   /* フラグオン                           */
#define LSTN_FLG_OFF                0                   /* フラグオフ                           */

#define LSTN_CONTROL_STS_OPEN       1                   /* コネクション制御オープン状態         */
#define LSTN_CONTROL_STS_CLOSE      0                   /* コネクション制御クローズ状態         */

#define LSTN_MAX_TEXT_LEN           MAX_TEXT_BUF_LEN    /* 最大電文長                           */
                                                        /* キューバッファサイズ                 */
#define LSTN_QUE_TEXT_LEN           _max(sizeof(n101_def),sizeof(n102_def)) 

/* コネクション識別 */
#define LSTN_CONNID_LISTNER         "CL"                /* リスナー                             */
#define LSTN_CONNID_SERVER          "CS"                /* コネクション制御(サーバ)             */
#define LSTN_CONNID_CLIENT          "CC"                /* コネクション制御(クライアント)       */

/* オープン情報 */
#define LSTN_CTLSERVER_QUALIFY      "#SCNCON.SV"        /* コネクション制御サーバクラス論理ID   */
#define LSTN_LABEL_CREATOR          -2                  /* 親プロセス用オープンラベル           */

/* エラーメッセージ要否 */
#define LSTN_MSG_NONE               0                   /* エラーメッセージ出力不要             */
#define LSTN_MSG_INTERNAL_ERR       100                 /* インターナルエラー                   */

/* コマンドレベル */
#define LSTN_CMDLVL_GROUP           1                   /* グループ識別指定                     */
#define LSTN_CMDLVL_INTERFACE       2                   /* インタフェース識別指定               */
#define LSTN_CMDLVL_STATION         3                   /* ステーション識別指定                 */

/****************************************************************************/
/*   TYPEDEF定義                                                            */
/****************************************************************************/

/* 自プロセス情報 */
typedef struct __myinfo_def
{
    /* コンフィグ情報 */
    struct
    {
        char    site_id;                                    /* サイト識別                           */
        char    network_id;                                 /* N/W識別                              */
        char    group_id[5];                                /* グループ識別                         */
        char    serverclass_id[12];                         /* サーバクラス論理ID                   */
        char    gfphi_fname[ZSYS_VAL_LEN_FILENAME+1];       /* 物理名情報ファイル名                 */
        short   gfphi_fname_len;                            /* 物理名情報ファイル名長               */
        char    gfnwi_fname[ZSYS_VAL_LEN_FILENAME+1];       /* N/W情報ファイル名                    */
        short   gfnwi_fname_len;                            /* N/W情報ファイル名長                  */
        char    gflin_fname[ZSYS_VAL_LEN_FILENAME+1];       /* 回線管理ファイル名                   */
        short   gflin_fname_len;                            /* 回線管理ファイル名長                 */
        char    gclst_fname[ZSYS_VAL_LEN_FILENAME+1];       /* 回線ステータスファイル名             */
        short   gclst_fname_len;                            /* 回線ステータスファイル名長           */
        char    gcscn_fname[ZSYS_VAL_LEN_FILENAME+1];       /* 受信コネクション数管理ファイル名     */
        short   gcscn_fname_len;                            /* 受信コネクション数管理ファイル名長   */
        long    fileio_timer;                               /* ファイルI/O完了待ちタイマー値        */
        long    socketio_timer;                             /* ソケットI/O完了待ちタイマー値        */
        long    pathsendio_timer;                           /* PATHSEND完了待ちタイマー値           */
        char    connection_counter_layer;                   /* コネクション数管理単位               */
        char    connection_counter_type;                    /* コネクション数管理種類               */
    } config_info;

    /* プロセス情報 */
//  struct
//  {
//      short   my_phandle[ZSYS_VAL_PHANDLE_WLEN];          /* 自プロセスハンドル                   */
//      char    my_pname[ZSYS_VAL_LEN_PROCESSNAME+1];       /* 自プロセス名                         */
//      short   my_pname_len;                               /* 自プロセス名長                       */
//      short   creator_phandle[ZSYS_VAL_PHANDLE_WLEN];     /* 親プロセスハンドル                   */
//      char    pathmon_name[ZSYS_VAL_LEN_PROCESSNAME+1];   /* 親プロセス名                         */
//      short   pathmon_name_len;                           /* 親プロセス名長                       */
//  } process_info;
    procinfo_def    process_info;

    /* 管理テーブル情報 */
    struct
    {
        short   listenporttbl_cnt;                          /* リスンポート管理テーブル数           */
        short   controlservertbl_cnt;                       /* コネクション制御管理テーブル数       */
        short   listenertbl_cnt;                            /* リスナー管理テーブル数               */
        short   connectiontbl_cnt;                          /* コネクション管理テーブル数           */
        short   counttbl_cnt;                               /* コネクション数管理テーブル数         */
    } table_info[2];
} myinfo_def;

/* リスンポート管理テーブル */
typedef struct __listenport_tbl_def
{
    char                interface_id[5];                            /* インタフェース識別                   */
    char                station_id[6];                              /* ステーション識別                     */
    char                listen_id[6];                               /* リスンポート識別                     */
    struct
    {
        char                tcpip_name[ZSYS_VAL_LEN_PROCESSNAME+1]; /* TCP/IPプロセス名                     */
        char                src_ip_text[16];                        /* 自IPアドレステキスト(回線管理)       */
        unsigned short      src_port_no;                            /* 自ポート番号(回線管理)               */
        struct sockaddr_in  src_sockaddr;                           /* 自ソケットアドレス(回線管理)         */
        char                dst_ip_text[16];                        /* 接続先IPアドレステキスト(回線管理)   */
        unsigned short      dst_port_no;                            /* 接続先ポート番号(回線管理)           */
        struct sockaddr_in  acc_sockaddr;                           /* 接続先ソケットアドレス(ACCEPT)       */
        int                 acc_sockaddr_len;                       /* 接続先ソケットアドレス長(ACCEPT)     */
        char                acc_ip_text[16];                        /* 接続先IPアドレステキスト(ACCEPT)     */
        long                so_tag;                                 /* ソケットI/Oタグ                      */
        short               so_error;                               /* ソケットI/Oエラー                    */
        short               listen_state;                           /* リスナーステート                     */
        char                listen_status[2];                       /* リスンポート状態                     */
        short               listen_fd;                              /* ソケットファイル番号                 */
        long                retry_timer;                            /* リスンリトライタイマー値             */
        long                retry_max;                              /* リスンリトライ回数                   */
        short               retry_tag;                              /* リトライタイマータグ                 */
        long                retry_cnt;                              /* リトライ回数                         */
        char                connect_time[20];                       /* 接続タイムスタンプ                   */
        char                disconnect_time[20];                    /* 切断タイムスタンプ                   */
        char                internal_error_code[7];                 /* 内部エラーコード                     */
    } ctl;
    struct
    {
        short               lsntbl_no;                              /* リスナー管理テーブル番号             */
        short               cnttbl_no;                              /* コネクション数管理テーブル番号       */
    } tbl;
} listenport_tbl_def;

/* コネクション制御管理テーブル */
typedef struct __controlserver_tbl_def
{
    char                serverclass_id[12];                         /* サーバクラス論理ID                   */
    struct
    {
        short               control_status;                         /* コネクション制御状態                 */
        short               phandle[ZSYS_VAL_PHANDLE_WLEN];         /* プロセスハンドル                     */
        short               cpu_no;                                 /* CPU番号                              */
        char                proc_name[ZSYS_VAL_LEN_PROCESSNAME+1];  /* プロセス名                           */
        short               proc_name_len;                          /* プロセス名長                         */
        short               async_tag;                              /* 非同期タグ                           */
        short               reconnect_cnttbl_no;                    /* 切断指示時コネクション数管理テーブル */
//      short               reconnect_queue_flg;                    /* 切断指示キューイングフラグ           */
//      char                reconnect_queue_buf[sizeof(n102_def)+1];/* 切断指示キューイングデータ           */
        short               queue_cnt;                              /* キューイングカウント                 */
        short               queue_idx;                              /* キューイング開始インデックス         */
        struct
        {
            char                queue_buf[LSTN_QUE_TEXT_LEN+1];     /* 切断指示キューイングデータ           */
            short               queue_len;                          /* 切断指示キューイングデータ長         */
        } queue_tbl [LSTN_CONTROLSERVER_QUE_MAX];
    } ctl;
    struct
    {
        short               connection_max;                         /* コネクション管理数                   */
        short               connection_cnt;                         /* 接続コネクション数                   */
    } tbl;
} controlserver_tbl_def;

/* リスナー管理テーブル */
typedef struct __listener_tbl_def
{
    char                interface_id[5];                    /* インタフェース識別                   */
    char                station_id[6];                      /* ステーション識別                     */
    char                listen_id[6];                       /* リスンポート識別                     */
    struct
    {
        char                serverclass_id[12];             /* サーバクラス論理ID                   */
    } ctl;
    struct
    {
        short               connection_max;                 /* コネクション数                       */
        short               lpttbl_no;                      /* リスンポート管理テーブル番号         */
    } tbl;
} listener_tbl_def;

/* コネクション管理テーブル */
typedef struct __connection_tbl_def
{
    char                interface_id[5];                    /* インタフェース識別                   */
    char                station_id[6];                      /* ステーション識別                     */
    char                connection_id[6];                   /* コネクション識別                     */
    struct
    {
        char                connection_sts[2];              /* コネクション状態                     */
        char                connect_time[20];               /* 接続タイムスタンプ                   */
        char                disconnect_time[20];            /* 切断タイムスタンプ                   */
        short               sockno;                         /* ソケットアドレス番号                 */
        struct sockaddr_in  sockaddr;                       /* 接続先ソケットアドレス               */
        short               error_code;                     /* エラーコード                         */
    } ctl;
    struct
    {
        short               ctstbl_no;                      /* コネクション制御管理テーブル番号     */
        short               lsntbl_no;                      /* リスナー管理テーブル番号             */
        short               lpttbl_no;                      /* リスンポート管理テーブル番号         */
        short               cnttbl_no;                      /* コネクション数管理テーブル番号       */
    } tbl;
} connection_tbl_def;

/* コネクション数管理テーブル */
typedef struct __count_tbl_def
{
    char                interface_id[5];                    /* インタフェース識別                   */
    char                station_id[6];                      /* ステーション識別                     */
    struct
    {
        char                reconnect_status[2];            /* 再接続ステータス                     */
        char                reconnect_serverclass[12];      /* 再接続サーバクラス論理ID             */
        short               reconnect_lpttbl_no;            /* 再接続リスンポート管理テーブル番号   */
        short               reconnect_cnt;                  /* 再接続待ちコネクション数             */
        char                reconnect_starttime[20];        /* 再接続処理開始日時                   */
        char                reconnect_endtime[20];          /* 再接続処理終了日時                   */
    } ctl;
    struct
    {
        short               max_connection_cnt;             /* 最大コネクション数                   */
        short               cur_connection_cnt;             /* カレントコネクション数               */
    } tbl;
} count_tbl_def;

/* ファイルI/O */
typedef struct __fileio_def
{
    char                func_type[4];
    char                sub_prog_sts[2];
    COM_IOM_arg_3_def   arg3;
    COM_IOM_arg_4_def   arg4;
    COM_IOM_arg_5_def   arg5;
    COM_IOM_arg_6_def   arg6;
} fileio_def;

/* I/O完了情報 */
typedef struct __iocmp_def
{
    short   fd;             /* ファイル番号                     */
    short   fs_err;         /* エラーコード                     */
    short   len;            /* 完了長                           */
    long    addr;           /* バッファーアドレス               */
    long    tag;            /* 完了TAG                          */
    short   compo;          /* I/Oコンポーネント                */
    short   cts_no;         /* コネクション制御管理テーブル番号 */
    short   lpt_no;         /* リスンポート管理テーブル番号     */
    short   event;          /* I/Oイベント                      */
    zsys_ddl_receiveinformation_def rinf;       /* $RECEIVE完了情報 */
} iocmp_def;

/* 階層情報 */
typedef struct __layer_info_def
{
    char            site_id;                    /* サイト識別               */
    char            network_id;                 /* N/W識別                  */
    char            group_id[5];                /* グループ識別             */
    char            interface_id[5];            /* インタフェース識別       */
    char            station_id[6];              /* ステーション識別         */
    char            connection_id[6];           /* コネクション識別         */
} layer_info_def;

/****************************************************************************/
/*   プロトタイプ関数宣言                                                   */
/****************************************************************************/
void LSTN_init(void);
void LSTN_get_params(void);
void LSTN_get_assign(void);
void LSTN_get_phisical_name(void);
short LSTN_get_listen_info(short);
short LSTN_get_network_info(short);
short LSTN_get_count_info(short);
short LSTN_get_connection_info(short);
short LSTN_get_controlserver_info(short);
short LSTN_get_status(void);
void LSTN_set_count_info(void);
void LSTN_main(void);
void LSTN_io_wait(void);
short LSTN_event_judgement(void);
void LSTN_open_message(void);
void LSTN_close_message(void);
void LSTN_cpudown_message(void);
void LSTN_timeout_message(void);
void LSTN_listen_start_command(void);
void LSTN_listen_stop_command(void);
void LSTN_file_reload_command(void);
short LSTN_file_reload_listen(short);
short LSTN_file_reload_connection(short);
void LSTN_control_async(void);
void LSTN_control_accept(void);
void LSTN_control_connect(void);
void LSTN_control_disconnect(void);
void LSTN_control_reconnect(void);
void LSTN_control_close(short);
void LSTN_control_queue(short, char*, short);
void LSTN_control_queue_reply(short);
short LSTN_port_open(short);
void LSTN_port_open_retry(short);
void LSTN_port_close(short);
void LSTN_accept(short);
void LSTN_accept_complete(short);
void LSTN_accept_error(short);
void LSTN_accept_reject(short);
void LSTN_connection_over(short);
void LSTN_reconnect_start(short);
void LSTN_reconnect_end(short);
short LSTN_linefile_open(void);
void LSTN_linefile_close(void);
void LSTN_statusfile_open(void);
short LSTN_statusfile_read(char*, char*, char*, short);
void LSTN_statusfile_update(void);
void LSTN_statusfile_close(void);
void LSTN_countfile_open(void);
short LSTN_countfile_read(char*, char*, short);
void LSTN_countfile_update(void);
void LSTN_countfile_unlock(void);
void LSTN_countfile_close(void);
void LSTN_countfile_update_count(short, short);
void LSTN_listenporttbl_init(short);
short LSTN_listenporttbl_search(short, char*, char*, char*);
short LSTN_listenporttbl_add(short, db_gflin_def *);
short LSTN_listenporttbl_move(short);
void LSTN_listenertbl_init(short);
short LSTN_listenertbl_search(short, char*, char*, char*);
short LSTN_listenertbl_add(short, db_gflin_def *);
void LSTN_controltbl_init(short);
short LSTN_controltbl_search(short, char*);
short LSTN_controltbl_add(short, db_gflin_def *);
short LSTN_controltbl_move(short);
void LSTN_connectiontbl_init(short);
short LSTN_connectiontbl_search(short, char*, char*, char*);
short LSTN_connectiontbl_add(short, db_gflin_def *);
short LSTN_connectiontbl_move(short);
short LSTN_connectiontbl_status(short, char*, char*);
void LSTN_counttbl_init(short);
short LSTN_counttbl_search(short, char*, char*);
short LSTN_counttbl_add(short, db_gcscn_def *);
void LSTN_reply(char*, short, short, short, short);
void LSTN_final(void);
void LSTN_message_output(short, char, char*, char*, ...);
void LSTN_internal_error(short, short);
char* ltrim (const char* string);
char* rtrim (const char* string);
char* trim (const char* string);

//short TRACEOUT( char* );

//void HEX2CHAR(unsigned char *hex_p, char *terget_p,short s_len);

#endif /* _GFPCVX00_H */
