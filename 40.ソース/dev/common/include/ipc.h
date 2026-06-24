/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････                                             */
/*        SUB-SYSTEM        ････                                             */
/*        PROGRAM-ID        ････                                             */
/*        FUNCTION          ････                                             */
/*                                                                           */
/*        AUTHER            ････                                             */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ yyyy-mm-dd                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  xxxxxxxx   YYYY/MM/DD (xxxxx)新規作成                               */
#ifndef COM_IPC
#define COM_IPC

#include <sys/socket.h> nolist
#include <netinet/in.h> nolist
#include  <zsysc>       nolist

#ifndef COM_LIMIT
#include "limit.h"
#endif

/* -------------------------------------------------------*/
/* 定数定義                                               */
/* -------------------------------------------------------*/
#define     DEF_CMD_DATA_SIZE       9999

/* -------------------------------------------------------*/
/* 構造体のtypedef定義                                    */
/* -------------------------------------------------------*/
/* 共通ヘッダー */
#pragma fieldalign shared2 __common_header_def
typedef struct __common_header_def
{
    char            interface_code[4];          /* インターフェースコード */
    short           error_code;                 /* エラーコード */
    char            internal_error_code[7];     /* 内部エラーコード */
    char            filler_1[9];                /* 予備 */
    unsigned short  control_data_length;        /* データ長 */
} common_header_def;

/* 回線管理ファイルプライマリキー */
#pragma fieldalign shared2 __gflin_pkey_def
typedef struct __gflin_pkey_def
{
    char             site_name;                 /* サイト識別 */
    char             nw_name;                   /* N/W識別 */
    char             group_name[5];             /* グループ識別 */
    char             interface_name[5];         /* インタフェース識別 */
    char             station_name[6];           /* ステーション識別 */
    char             connection_name[6];        /* コネクション識別 */
} gflin_pkey_def;

/* GFP内部LCN */
#pragma fieldalign shared2 __gfplcn
typedef struct __gfplcn
{
    char                        site_code;     /* 採番システム         */
    char                        network_code;  /* 場所                 */
    char                        reserve;       /* 予備                 */
    char                        year;          /* 西暦の下1桁          */
    char                        mdh[3];        /* 月日時(32進数)       */
    char                        mmss[4];       /* 分秒                 */
    char                        seqnum[4];     /* 連番                 */
} gfplcn_def;

/* コネクションステータス照会結果 */
#pragma fieldalign shared2 __gclst_data
typedef struct __gclst_data
{
    gflin_pkey_def          connection_logical_name;
    struct
    {
        char                            connect_sts[2];
        char                            connect_sts_update_time[20];
    } connect_sts_info;
    struct
    {
        char                            prc_sts[2];
        char                            prc_sts_update_time[14];
    } prc_sts_info;
    struct
    {
        char                            ip_address_src[15];
        char                            port_num_src[5];
        char                            ip_address_dst[15];
        char                            port_num_dst[5];
        char                            err_code[4];
        char                            disconnect_rsn[2];
     } connect_info;
} gclst_data_def;

/* 局状態照会結果 */
#pragma fieldalign shared2 __gcsst_data
typedef struct __gcsst_data
{
    gflin_pkey_def          connection_logical_name;
    struct
    {
        char                            nw_if[20];
        char                            nw_station[11];
        char                            filler;
    } nw_id_info;
    struct
    {
        char                            state_sts[2];
        char                            state_sts_update_time[14];
        char                            open_state_retry_num[8];
    } state_sts_info;
} gcsst_data_def;

/* エコーステータス照会結果 */
#pragma fieldalign shared2 __gcest_data
typedef struct __gcest_data
{
    gflin_pkey_def          connection_logical_name;
    struct
    {
        struct
        {
            char                            last_echo_start_time[14];
            char                            last_echo_end_time[14];
            char                            last_echo_result[2];
            char                            last_echo_ok_time[14];
        } cbs_echo_info;
        struct
        {
            char                            last_echo_req_recv_time[14];
            char                            last_echo_result[2];
            char                            last_echo_ok_time[14];
        } dst_echo_info;
    } echo_info;
} gcest_data_def;

/* 制御電文種別 */
#pragma fieldalign shared2 __control_kind_def
typedef struct __control_kind_def
{
    char            kinou_kbn;              /* 制御機能区分             */
    char            req_res_kbn;            /* 要求応答区分             */
    char            ctl_text_kbn;           /* 制御電文区分             */
    char            int_proc_kbn;           /* 内部処理区分             */
} control_kind_def;

/*--------------------------------------------------------------------------*/

/* A001 非同期要求 */
#pragma fieldalign shared2 __a001_def
typedef struct __a001_def
{
    common_header_def           common_header;
} a001_def;

/* N101 コネクション接続通知 */
#pragma fieldalign shared2 __n101_def
typedef struct __n101_def
{
    common_header_def           common_header;
    gflin_pkey_def              line_info;
    gflin_pkey_def              listen_info;
    struct
    {
        char    src_ip_address[15];
        char    src_port[5];
        char    dest_ip_address[15];
        char    dest_port[5];
    } connection_info;
    struct sockaddr_in socket_info;
    char   yobi[46];
} n101_def;

/* N102 コネクション入替・切断指示通知 */
#pragma fieldalign shared2 __n102_def
typedef struct __n102_def
{
    common_header_def           common_header;
    gflin_pkey_def              line_info;
    gflin_pkey_def              listen_info;
    struct
    {
        char    src_ip_address[15];
        char    src_port[5];
        char    dest_ip_address[15];
        char    dest_port[5];
    } connection_info;
    struct  sockaddr_in socket_info;
    struct
    {
        char    disconnect_info;
        char    disconnect_yobi;
    } disconnect_info;
    char    yobi[44];
} n102_def;

/* C103 コネクション接続開始要求 */
#pragma fieldalign shared2 __c103_def
typedef struct __c103_def
{
    common_header_def           common_header;
    gflin_pkey_def              line_info;
    struct
    {
        char    connection_status[2];
        char    connection_status_time[20];
    } status_info;
    struct
    {
        char    process_status[2];
        char    process_status_time[14];
    } process_info;
    char    yobi[88];
} c103_def;

/* R103 コネクション接続開始応答 */
#pragma fieldalign shared2 __r103_def
typedef struct __r103_def
{
    common_header_def           common_header;
    gflin_pkey_def              line_info;
    struct
    {
        char    connection_status[2];
        char    connection_status_time[20];
    } status_info;
    struct
    {
        char    process_status[2];
        char    process_status_time[14];
    } process_info;
    char    yobi[88];
} r103_def;

/* C104 コネクション接続完了通知要求 */
#pragma fieldalign shared2 __c104_def
typedef struct __c104_def
{
    common_header_def           common_header;
    gflin_pkey_def              line_info;
    struct
    {
        char    connection_status[2];
        char    connection_status_time[20];
    } status_info;
    struct
    {
        char    process_status[2];
        char    process_status_time[14];
    } process_info;
    struct
    {
        char    error_code[4];
        char    disconnect_reason[2];
    } connection_info;
    char    yobi[82];
} c104_def;

/* R104 コネクション接続完了通知応答 */
#pragma fieldalign shared2 __r104_def
typedef struct __r104_def
{
    common_header_def           common_header;
    gflin_pkey_def              line_info;
    struct
    {
        char    connection_status[2];
        char    connection_status_time[20];
    } status_info;
    struct
    {
        char    process_status[2];
        char    process_status_time[14];
    } process_info;
    struct
    {
        char    error_code[4];
        char    disconnect_reason[2];
    } connection_info;
    char    yobi[82];
} r104_def;

/* C105 コネクション切断完了通知要求 */
#pragma fieldalign shared2 __c105_def
typedef struct __c105_def
{
    common_header_def           common_header;
    gflin_pkey_def              line_info;
    struct
    {
        char    connection_status[2];
        char    connection_status_time[20];
    } status_info;
    struct
    {
        char    process_status[2];
        char    process_status_time[14];
    } process_info;
    struct
    {
        char    error_code[4];
        char    disconnect_reason[2];
    } connection_info;
    char    yobi[82];
} c105_def;

/* R105 コネクション切断完了通知応答 */
#pragma fieldalign shared2 __r105_def
typedef struct __r105_def
{
    common_header_def           common_header;
    gflin_pkey_def              line_info;
    struct
    {
        char    connection_status[2];
        char    connection_status_time[20];
    } status_info;
    struct
    {
        char    process_status[2];
        char    process_status_time[14];
    } process_info;
    struct
    {
        char    error_code[4];
        char    disconnect_reason[2];
    } connection_info;
    char    yobi[82];
} r105_def;

/* C106 コネクション入替・切断完了通知要求 */
#pragma fieldalign shared2 __c106_def
typedef struct __c106_def
{
    common_header_def           common_header;
    gflin_pkey_def              line_info;
    struct
    {
        char    connection_status[2];
        char    connection_status_time[20];
    } status_info;
    struct
    {
        char    process_status[2];
        char    process_status_time[14];
    } process_info;
    struct
    {
        char    error_code[4];
        char    disconnect_reason[2];
    } connection_info;
    struct
    {
        char    disconnect_info;
        char    disconnect_yobi;
    } disconnect_info;
    char    yobi[80];
} c106_def;

/* R106 コネクション入替・切断完了通知応答 */
#pragma fieldalign shared2 __r106_def
typedef struct __r106_def
{
    common_header_def           common_header;
    gflin_pkey_def              line_info;
    struct
    {
        char    connection_status[2];
        char    connection_status_time[20];
    } status_info;
    struct
    {
        char    process_status[2];
        char    process_status_time[14];
    } process_info;
    struct
    {
        char    error_code[4];
        char    disconnect_reason[2];
    } connection_info;
    struct
    {
        char    disconnect_info;
        char    disconnect_yobi;
    } disconnect_info;
    char    yobi[80];
} r106_def;

/* C201 電文受信通知要求 */
#pragma fieldalign shared2 __c201_def
typedef struct __c201_def
{
    common_header_def           common_header;
    struct
    {
        gflin_pkey_def          recv_con_id;
        struct
        {
            char    src_ip_address[15];
            char    src_port[5];
            char    dest_ip_address[15];
            char    dest_port[5];
        } recv_con_info;
        struct
        {
            char    time_stamp[20];
            char    ts_unique_data[16];
        } recv_timestamp;
        char        filler_1[50];
    } text_recv_notify;
    struct
    {
        short   msg_len;                    /* 電文長 */
        char    msg_data[MAX_TEXT_BUF_LEN]; /* 電文 */
    } msg_info;                             /* 電文情報*/
} c201_def;

/* R201 電文受信通知応答 */
#pragma fieldalign shared2 __r201_def
typedef struct __r201_def
{
    common_header_def           common_header;
} r201_def;

/* C202 電文送信要求 */
#pragma fieldalign shared2 __c202_def
typedef struct __c202_def
{
    common_header_def           common_header;
    struct
    {
        gflin_pkey_def          recv_con_id;
    } text_send_info;
    struct
    {
        struct 
        {
            char    gfp_lcn[15];        /* GFP内部LCN */
            char    text_format;        /* 電文形態 */
        } transaction_id;               /* トランザクションID */
        char        text_type;          /* 電文種別 */
        char        retry_count[3];     /* 再送回数 */
    } text_log_key;                     /* 電文ログKEY */
    struct
    {
        struct
        {
            char    prc_kind[8];        /* プロセス論理名 */
            char    prc_num[4];         /* プロセス論理番号 */
        } prc_id;                       /* プロセス論理ID */
        char        prc_mlt_num[4];     /* プロセス冗長化番号 */
    } src_prc_lgc_key;                  /* 電文送信要求元プロセス論理KEY */
    char            filler_1[90];       /* 予備 */
    struct
    {
        short   msg_len;                    /* 電文長 */
        char    msg_data[MAX_TEXT_BUF_LEN]; /* 電文 */
    } msg_info;                             /* 電文情報*/
} c202_def;

/* R202 電文送信応答 */
#pragma fieldalign shared2 __r202_def
typedef struct __r202_def
{
    common_header_def           common_header;
} r202_def;

/* C301 キュー登録要求 */
#pragma fieldalign shared2 __c301_def
typedef struct __c301_def
{
    common_header_def           common_header;
    struct
    {
        char            gfp_lcn[15];                /* GFP内部LCN */
        struct
        {
            char        site_name;                  /* サイト識別 */
            struct
            {
                char    serverclass_name[8];        /*サーバクラス論理名 */
                char    serverclass_num[4];         /*サーバクラス論理番号 */
            } serverclass_id;                       /* サーバクラス論理ID */
        } serverclass_info;                         /* 要求元サーバクラス情報 */
        char            filler_1[22];               /* 予備 */
        char            msg_data[MAX_QUEUE_BUF_LEN]; /* キュー登録データ */
    } que_rgs_info;
} c301_def;

/* R301 キュー登録応答 */
#pragma fieldalign shared2 __r301_def
typedef struct __r301_def
{
    common_header_def           common_header;
} r301_def;

/* C302 キュー取出し通知要求 */
#pragma fieldalign shared2 __c302_def
typedef struct __c302_def
{
    common_header_def           common_header;
    char            msg_data[MAX_QUEUE_BUF_LEN];     /* キュー取出しデータ */
} c302_def;

/* R302 キュー取り出し通知応答 */
#pragma fieldalign shared2 __r302_def
typedef struct __r302_def
{
    common_header_def           common_header;
} r302_def;

/* C107 コネクション状態通知要求 */
#pragma fieldalign shared2 __c107_def
typedef struct __c107_def
{
    common_header_def           common_header;
    struct
    {
        char    site_name;
        char    nw_name;
        char    group_name[5];
        char    interface_name[5];
        char    station_name[6];
        char    src_connection_name[6];
        char    dest_connection_name[6];
    } line_info;
    struct
    {
        char    connection_site_name;
        char    connection_nw_name;
        char    connection_group_name[5];
        char    connection_interface_name[5];
        char    connection_station_name[6];
        char    connection_status[2];
        char    connection_status_time[20];
    } connection_status_info;
    struct
    {
        char    process_status[2];
        char    process_status_time[24];
    } process_state_info;
    struct
    {
        char    src_ip_address[15];
        char    src_port[5];
        char    dest_ip_address[15];
        char    dest_port[5];
        char    connection_error_code[4];
        char    disconnect_reason[2];
    } connection_info;
    struct sockaddr_in sockaddr_in;
} c107_def;

/* R107 コネクション状態通知応答 */
#pragma fieldalign shared2 __r107_def
typedef struct __r107_def
{
    common_header_def           common_header;
    struct
    {
        char    site_name;
        char    nw_name;
        char    group_name[5];
        char    interface_name[5];
        char    station_name[6];
        char    src_connection_name[6];
        char    dest_connection_name[6];
    } line_info;
    struct
    {
        char    connection_site_name;
        char    connection_nw_name;
        char    connection_group_name[5];
        char    connection_interface_name[5];
        char    connection_station_name[6];
        char    connection_status[2];
        char    connection_status_time[20];
    } connection_status_info;
    struct
    {
        char    process_status[2];
        char    process_status_time[24];
    } process_state_info;
    struct
    {
        char    src_ip_address[15];
        char    src_port[5];
        char    dest_ip_address[15];
        char    dest_port[5];
        char    connection_error_code[4];
        char    disconnect_reason[2];
    } connection_info;
    struct sockaddr_in sockaddr_in;
} r107_def;


/* CR401 NW電文受信 */
#pragma fieldalign shared2 __cr401_def
typedef struct __cr401_def
{
    common_header_def       common_header;
    struct
    {
        char                request_kind[2];            /* 要求種別                     */
        char                response_kind[2];           /* 応答種別                     */
        control_kind_def    control_kind;               /* 制御電文種別                 */
        gflin_pkey_def      connection_lid;             /* コネクション論理ID           */
        char                interface_name[20];         /* インタフェース名             */
        char                station_name[11];           /* ステーション名               */
        char                yobi1;
        struct                                          /* 電文ログKEY                  */
        {
            struct                                      /* トランザクションID           */
            {
                char        gfp_lcn[15];                /* GFP内部LCN                   */
                char        denbun_keitai;              /* 電文形態                     */
            } tran_id;
            char            denbun_shubetu;             /* 電文種別                     */
            char            re_send_num[3];             /* 再送回数                     */
        } denbun_log_key;
        char                mti[4];                     /* MTI                          */
        char                req_gfp_lcn[15];            /* 仕向要求電文GFP内部LCN       */
        char                send_naibu_err_code[7];     /* 送信不可時内部エラーコード   */
        char                denbun_len[4];              /* 受信電文長                   */
        char                yobi2[6];
    } control_info;
    struct
    {
        char                message_text[MAX_TEXT_BUF_LEN];
    } data_bu;
} cr401_def;

/* CR402 制御電文作成 */
#pragma fieldalign shared2 __cr402_def
typedef struct __cr402_def
{
    common_header_def       common_header;
    struct
    {
        char                response_kind[2];           /* 応答種別                     */
        control_kind_def    control_kind;               /* 制御電文種別                 */
        gflin_pkey_def      connection_lid;             /* コネクション論理ID           */
        char                interface_name[20];         /* インタフェース名             */
        char                station_name[11];           /* ステーション名               */
        char                yobi1;
        struct                                          /* 電文ログKEY                  */
        {
            struct                                      /* トランザクションID           */
            {
                char        gfp_lcn[15];                /* GFP内部LCN                   */
                char        denbun_keitai;              /* 電文形態                     */
            } tran_id;
            char            denbun_shubetu;             /* 電文種別                     */
            char            re_send_num[3];             /* 再送回数                     */
        } denbun_log_key;
        char                mti[4];                     /* MTI                          */
        char                yobi2[34];
    } control_info;
    struct
    {
        char                message_text[MAX_TEXT_BUF_LEN];
    } data_bu;
} cr402_def;

/* C501 コマンド要求 */
#pragma fieldalign shared2 __c501_def
typedef struct __c501_def
{
    common_header_def           common_header;
    struct
    {
        char    command_name[4];
        gflin_pkey_def          connection_logical_name;
        char    interface_ext_name[20];
        char    station_ext_name[11];
        char    srv_cls_id[8];
        char    filler;
    } command_info;
    unsigned short              record_count;
} c501_def;

/* R501 コマンド応答 照会系コマンド以外 */
#pragma fieldalign shared2 __r501_def
typedef struct __r501_def
{
    common_header_def           common_header;
    struct
    {
        char    command_name[4];
        gflin_pkey_def          connection_logical_name;
        char    interface_ext_name[20];
        char    station_ext_name[11];
        char    srv_cls_id[8];
        char    filler;
    } command_info;
    unsigned short              record_count;
} r501_def;

/* R501 コマンド応答  コネクションステータス照会コマンド*/
#pragma fieldalign shared2 __r501_cn_st_def
typedef struct __r501_cn_st_def
{
    common_header_def           common_header;
    struct
    {
        char    command_name[4];
        gflin_pkey_def          connection_logical_name;
        char    interface_ext_name[20];
        char    station_ext_name[11];
        char    srv_cls_id[8];
        char    filler;
    } command_info;
    unsigned short              record_count;
    gclst_data_def              gclst_data[DEF_CMD_DATA_SIZE/sizeof(gclst_data_def)];
} r501_cn_st_def;

/* R501 コマンド応答  局状態照会コマンド*/
#pragma fieldalign shared2 __r501_sta_st_def
typedef struct __r501_sta_st_def
{
    common_header_def           common_header;
    struct
    {
        char    command_name[4];
        gflin_pkey_def          connection_logical_name;
        char    interface_ext_name[20];
        char    station_ext_name[11];
        char    srv_cls_id[8];
        char    filler;
    } command_info;
    unsigned short              record_count;
    gcsst_data_def              gcsst_data[DEF_CMD_DATA_SIZE/sizeof(gcsst_data_def)];
} r501_sta_st_def;

/* R501 コマンド応答  エコーステータス照会コマンド*/
#pragma fieldalign shared2 __r501_echo_st_def
typedef struct __r501_echo_st_def
{
    common_header_def           common_header;
    struct
    {
        char    command_name[4];
        gflin_pkey_def          connection_logical_name;
        char    interface_ext_name[20];
        char    station_ext_name[11];
        char    srv_cls_id[8];
        char    filler;
    } command_info;
    unsigned short              record_count;
    gcest_data_def              gcest_data[DEF_CMD_DATA_SIZE/sizeof(gcest_data_def)];
} r501_echo_st_def;

/* C502 コマンド処理要求 */
#pragma fieldalign shared2 __c502_def
typedef struct __c502_def
{
    common_header_def           common_header;
    struct
    {
        char    command_name[4];
        gflin_pkey_def          connection_logical_name;
        char    interface_ext_name[20];
        char    station_ext_name[11];
        char    srv_cls_id[8];
    } command_info;
} c502_def;

/* R502 コマンド処理応答 */
#pragma fieldalign shared2 __r502_def
typedef struct __r502_def
{
    common_header_def           common_header;
    struct
    {
        char    command_name[4];
        gflin_pkey_def          connection_logical_name;
        char    interface_ext_name[20];
        char    station_ext_name[11];
        char    srv_cls_id[8];
    } command_info;
} r502_def;


/* C601 ログ出力要求 */
#pragma fieldalign shared2 __c601_def
typedef struct __c601_def
{
    common_header_def          common_header;             /* 共通ヘッダ       */
    char                       logfile_id[48];            /* ログファイル名   */
    short                      entry_update_cate;         /* 登録/更新区分    */
    db_glnlg_def               t_glnlg;                   /* NW通信ログ       */
} c601_def;

/* R601 ログ出力応答 */
#pragma fieldalign shared2 __r601_def
typedef struct __r601_def
{
    common_header_def         common_header;              /* 共通ヘッダ       */
    char                      logfile_id[48];             /* ログファイル名   */
    char                      lcn[15];                    /* LCN              */
    char                      thnkbn[10];                 /* 東阪振分区分     */
} r601_def;

/* C701 LCN採番要求 */
#pragma fieldalign shared2 __c701_def
typedef struct __c701_def
{
    common_header_def           common_header;
    char                        process_name[ZSYS_VAL_LEN_UNIQUEPROCESSNAME];
    char                        site_code;
    char                        network_code;
} c701_def;

/* R701 LCN採番応答 */
#pragma fieldalign shared2 __r701_def
typedef struct __r701_def
{
    common_header_def           common_header;
    gfplcn_def                  gfplcn;
} r701_def;

/* SS01 システム採番要求 */
#pragma fieldalign shared2 __ss01_def
typedef struct __ss01_def
{
    char        interface_code[4];          /* インターフェースコード */
    char        serial_number_kbn[2];       /* 通番区分 */
} ss01_def;

/* SS02 システム採番応答 */
#pragma fieldalign shared2 __ss02_def
typedef struct __ss02_def
{
    char        interface_code[4];          /* インターフェースコード */
    char        serial_number_kbn[2];       /* 通番区分 */
    char        numbering_value[6];         /* 採番値 */
    char        internal_error_code[7];     /* 内部エラーコード */
    char        guardian_error_code[5];     /* ガーディアンエラーコード */
    char        err_detection_info[36];     /* エラー検知情報 */
} ss02_def;

/* C801 鍵管理ファイル更新要求 */
#pragma fieldalign shared2 __c801_def
typedef struct __c801_def
{
    common_header_def           common_header;
    struct
    {
        char            gfp_lcn[15];                /* GFP内部LCN */
        struct
        {
            char        site_name;                  /* サイト識別 */
            struct
            {
                char    serverclass_name[8];        /* サーバクラス論理名 */
                char    serverclass_num[4];         /* サーバクラス論理番号 */
            } serverclass_id;                       /* サーバクラス論理ID */
        } serverclass_info;                         /* 要求元サーバクラス情報 */
        short           upd_rec_cnt;                /* 更新レコード数 */
        char            filler_1[20];               /* 予備 */
    } req_info;
    char                upd_rec_1[800];             /* 更新レコード1 */
    char                upd_rec_2[800];             /* 更新レコード2 */
} c801_def;

/* R801 鍵管理ファイル更新応答 */
#pragma fieldalign shared2 __r801_def
typedef struct __r801_def
{
    common_header_def           common_header;
} r801_def;

#endif /* COM_IPC */
