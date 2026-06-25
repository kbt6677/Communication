/*****************************************************************************/
/*****                    <<     HEADER PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVX40                                    */
/*        FUNCTION          ････ 電文振分(outbond)                           */
/*                                                                           */
/*        AUTHER            ････ HAS kimura                                  */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2024-11-04                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS 木村   2024/11/04 (電文振分(outbound))新規作成                   */
/****************************************************************************/

/* USER HEADER */
/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
/* 共通DEFINE */
#define MDSO_RET_OK                         0
#define MDSO_RET_NG                         -1
#define MDSO_IPC_ERR                        9
#define MDSO_RET_DOUBLE_SET                 3
#define MDSO_RET_NO_DETOUR                  1
#define MDSO_RET_BAD_SEND                   1
#define MDSO_ZERO                           0
#define MDSO_SPACE                          ' '
#define MDSO_SITE_ID_LENG                   1   /* サイトID長 */
#define MDSO_NW_ID_LENG                     1   /* NW ID長 */
#define MDSO_GROUP_ID_LENG                  5   /* グループID長 */
#define MDSO_SERVERCLASS_NAME_LENG          8   /* サーバクラス種類長 */
#define MDSO_SERVERCLASS_NO_LENG            4   /* サーバクラス論理番号長 */
#define MDSO_MAX_TURN_TBL                   100
#define MDSO_IF_MAX                         (30*80) /* インタフェース最大数 */
#define MDSO_CONNECTION_MAX                 (30*200) /* コネクション最大数 */
#define MDSO_REDUN_ZERO                     "0000" /* 冗長番号 0000 */
#define MDSO_CST_SIZE                       2   /* 局状態のサイズ(bytes) */
#define MDSO_CON_RETRY_CNT                  1   /* コネクション再選択リトライ回数 */

/* フラグ */
#define MDSO_ON                             1
#define MDSO_OFF                            0
#define MDSO_NOMAL_END                      2   /* プロセス正常終了 */

/* パラメータサイズ */
#define MDSO_LOG_FILE_NAME_SIZE             48

/* 電文ログ出力パラメータ */
#define MDSO_LOG_SET                         1
#define MDSO_LOG_UPDATE                      2
#define MDSO_RECEIVE_DATA                    '1'
#define MDSO_SEND_DATA                       '2'

/* エラーログ出力パラメータ */
#define MDSO_ERR_LOG_NAIBU_ERR               '2'
#define MDSO_ERR_LOG_OPEN                    1
#define MDSO_ERR_LOG_WRITE                   2
#define MDSO_ERR_LOG_CLOSE                   3

/* IPCチェックエラーコード */
#define MDSO_IPC_ERR_MSG                     1 /* インタフェースコード異常 */
#define MDSO_IPC_ERR_DATA_LEN                2 /* データ長異常 */

/* RECEIVE用 */
#define MDSO_MAX_DATA_SIZE                   12000  /* プロセス間通信データの最大長 */
#define MDSO_RECEIVE_FILENAME                "$RECEIVE" /* $RECEIVE file name */
#define MDSO_TUERN_DETOUR_TBL_MAX            30
#define MDSO_RCV_NOWAITDEPTH                 0     /* enable nowait operations */
#define MDSO_RECVDEPTH                       1     /* enable replies on $RECEIVE */

/* COMファイル */
#define MDSO_FILEIO_TYPE_OPEN                "OPEN    "
#define MDSO_FILEIO_TYPE_CLOSE               "CLOSE   "
#define MDSO_FILEIO_TYPE_START               "START   "
#define MDSO_FILEIO_TYPE_READ                "READ    "
#define MDSO_IO_NORMAL_END                   "00"

/* C301データ種別 */
#define MDSO_C301_KIND_TURN_IPC              "09"  /* 電文送信不可応答 */

/* 暗号化複合 */
#define MDSO_ENCODE                          1  /* 暗号化 */
#define MDSO_DECODE                          2  /* 復号 */

/* SYSTEM メッセージ */
#define MDSO_NORMAL_TERMINATION              0  /*正常終了*/
#define MDSO_ABNORMAL_TERMINATION            1  /*異常終了*/

/* ログ出力エラー */
#define MDSO_RET_LCN_DOUBLE                  1  /* GFP内部LCN重複 */

/* 振分先設定データ */
#define MDSO_SVC_REQ                          '1'  /* 業務要求 */
#define MDSO_SVC_RSP                          '2'  /* 業務応答 */
#define MDSO_CTL_REQ                          '3'  /* 制御要求 */
#define MDSO_CTL_RSP                          '4'  /* 制御応答 */
#define MDSO_CTL_HB                           '5'  /* ハートビート */

/* コネクション状態 */
#define MDSO_CONNECT_STS_DISCONN_INT        1        /* 切断                                     */
#define MDSO_CONNECT_STS_LISTEN_INT         2        /* 接続待ち                                 */
#define MDSO_CONNECT_STS_CONNECT_INT        3        /* 接続                                     */
#define MDSO_CONNECT_STS_RECONNECT_INT      4        /* 再接続処理中                             */

/* 送信先再選択要否 */
#define MDSO_NO_DETOUR             '0'  /* 再選択不要       */
#define MDSO_EXE_DETOUR            '1'  /* 再選択要         */

/* オープンID */
#define MDSO_OPENID_ANCESTOR        1    /* オープンID(親プロセス)       */
#define MDSO_OPENID_ROUT            2    /* オープンID(ROUT)             */

/* ファイル設定内容確認用データ */
#define MDSO_STATION_NONE         "}}}}}}"                       /* ステーション情報なし                       */
#define MDSO_INTERFACE_NONE       "}}}}}"                        /* インタフェース情報なし                     */

/* コネクション制御種別 */
#define MDSO_CON_CC               "CC"  /* コネクション制御(クライアント) */
#define MDSO_CON_CS               "CS"  /* コネクション制御(サーバー)     */

/* 送信先選択範囲 */
#define MDSO_SEND_RANGE_INIT          0    /* 初期値            */
#define MDSO_SEND_RANGE_INTERFACE     1    /* インタフェース    */
#define MDSO_SEND_RANGE_STATION       2    /* ステーション      */
#define MDSO_SEND_RANGE_CONNECTION    3    /* コネクション      */

/* 局状態取得済みフラグ */
#define MDSO_CST_GET_NON              0    /* 未取得    */
#define MDSO_CST_GET_DONE             1    /* 取得済み  */

/* コネクション選択済みフラグ */
#define MDSO_CON_GET_NON              0    /* 未選択    */
#define MDSO_CON_GET_DONE             1    /* 選択済み  */

/* GUARDIANエラーコード */
#define MDSO_GERR_TIMEOUT            40    /* タイムアウト      */

/****************************************************************************/
/*   内部変数定義                                                           */
/****************************************************************************/

/********** 構造体 ***********/
/* 自プロセス情報 */
typedef struct __t_myinfo_def
{
    struct                      /* コンフィグ情報 */
    {
        char  site_id;                /* サイト識別                     */
        char  network_id;             /* N/W識別                        */
        char  group_id[5];            /* グループ識別                   */
        char  serverclass_id[12];     /* サーバクラス論理ID             */
        char  serverclass_name[8];    /* サーバクラス名                 */
        char  serverclass_no[4];      /* サーバクラス論理番号           */
        long  io_timer;               /* ファイルI/O完了待ちタイマー値  */
        char  ems_serverclass_name[16];/* 運用監視端末出力サーバ・サーバクラス名  */
        char  ems_pathmon[16];         /* 運用監視端末出力サーバ・PATHMON名*/
        char  ems_collector[6];       /* EMSコレクタ                    */
        unsigned long  send_timer;    /* PATHSENDタイマ                 */
        unsigned long  send_retry_count;/* PATHSENDリトライ回数         */
        char  trace_flag;             /* トレースフラグ                 */
    } config_data;
    procinfo_def proc_data;           /* プロセス情報                   */
    struct                            /* プロセスサブ情報               */
    {
        char  my_prcname[8];          /* 自プロセス論理名               */
        char  my_prcno[4];            /* 自プロセス論理番号             */
        char  my_prcmlt[4];           /* 自プロセス冗長化番号           */
        char  module_id[12];          /* モジュールID                   */
        short open_num;               /* プロセスオープン数             */
    } proc_data_sub;
    short recv_fno;                   /* $RECEIVEファイル番号           */
    char* recv_buf;                   /* $RECEIVEバッファーポインタ     */
    char  pathmon_name[16];            /* PATHMONプロセス名              */
    short end_flag;                   /* 終了フラグ                     */
    char  trc_time_begin[21];         /* トレース開始時間               */
    char  trc_time_end[21];           /* トレース終了時間               */
    unsigned long  file_timer;        /* ファイルI/Oタイマ              */
    char  file_timer_c[6];            /* ファイルI/Oタイマ(文字列)      */
    char  nwid_sign[2];               /* ネットワーク識別サイン         */
    char  msg_pname[6];               /* メッセージ出力プロセス名       */
    char  nw_id_sec[2];               /* NW識別子                       */
    short data_len;                   /* データ長                       */
    char  data_len_del_flag;          /* データ長領域削除済みフラグ     */
} t_myinfo_def;

typedef struct __nw_info              /* N/W識別情報 */
{
    char  interface_id[5];        /* インタフェース識別             */
    char  station_id[6];          /* ステーション識別               */
    char  nw_segment[2];          /* NW区分                         */
    char  interface_name[20];     /* インタフェース識別名           */
    char  station_name[11];       /* ステーション名                 */
    char  send_re_select_need;    /* 送信先再選択要否               */
} t_nw_info;

/* 電文項目位置情報 */
typedef struct __t_denbun_lct_info
{
    char  interface_id[5];            /* インタフェース識別             */
    char  station_id[6];              /* ステーション識別               */
    short data_len_start_lct;         /* データレングス開始位置         */
    short data_len_size;              /* データレングス項目長           */
    char  data_len_attribute[3];      /* データレングス項目属性         */
    char  data_len_include_id;        /* データレングスINCLUDE識別      */
    short denbun_start_lct;           /* 電文データ開始位置             */
    short mti_start_lct;              /* MTI開始位置                    */
    short mti_item_len;               /* MTI項目長                      */
    char  mti_item_attribute[3];      /* MTI項目属性                    */
} t_denbun_lct_info;


/* PATHSEND情報 */
typedef struct __t_sendinfo_def
{
  char*  *psend_data;
  short  psend_send_len;
  short  psend_resp_len;
  short  psend_err;
  short  gerr;
} t_sendinfo_def;

/* 送信電文折返し振分先設定テーブル */
typedef struct __t_turn_tbl
{
    struct {
        char  mti[4];
    } t_primary_key;
    struct {
        char  srv_kind[8];
        char  srv_num[4];
    } t_rcv_que;
    struct {
        char domain_name[8];
        char pathmon[16];
        char serverclass[16];
    } t_phy_data;
} t_turn_tbl;

/* コネクションリスト */
typedef struct __t_con_list
{
    char  interface_id[5];        /* インタフェース識別          */
    char  station_id[6];          /* ステーション識別            */
    char  connection_id[6];       /* コネクション識別            */
    char  server_class[12];       /* サーバークラス論理番号      */
    char  domain_name[8];         /* ドメイン名                  */
    char  pathmon_name[16];       /* PATHMON名                   */
    char  server_class_phy[16];   /* 物理サーバークラス名        */
    char  dmy1;
    short connection_st;          /* コネクション状態            */
    char  station_sts[2];         /* 局状態                      */
    short station_sts_get;        /* 局状態取得済みフラグ        */
    short connection_get;         /* コネクション選択済みフラグ  */
	short line_list_st_idx;       /* 回線ラウンドロビンリスト(ステーション)IDX */
} t_con_list;

/* 回線ラウンドロビンリスト(ステーション) */
typedef struct __t_line_list_st
{
    char  interface_id[5];        /* インタフェース識別         */
    char  station_id[6];          /* ステーション識別           */
    char  dmy1;
    short list_cnt;               /* コネクションリストエレメント数        */
    short list_top_no;            /* コネクションリスト先頭IDX             */
    short list_no;                /* ラウンドロビン番号(0～エレメント数-1) */
    char  station_sts[2];         /* 局状態(ステーション単位)              */
    short station_sts_get;        /* 局状態取得済みフラグ                  */
    short line_list_if_idx;       /* 回線ラウンドロビンリスト(インタフェース)IDX */
} t_line_list_st;

/* 回線ラウンドロビンリスト(インタフェース) */
typedef struct __t_line_list_if
{
    char  interface_id[5];        /* インタフェース識別         */
    char  dmy1;
    short list_cnt;               /* 回線ラウンドロビンリスト(ステーション)エレメント数   */
    short list_top_no;            /* 回線ラウンドロビンリスト(ステーション)先頭IDX        */
    short list_no;                /* ラウンドロビン番号(0～エレメント数-1)            */
    char  station_sts[2];         /* 局状態(インタフェース単位)              */
    short station_sts_get;        /* 局状態取得済みフラグ                  */
} t_line_list_if;

/* ATALLA接続情報 */
typedef struct __t_encdec_con
{
    struct {
        char          key_file_id[8];
        char          key_file_name[48];
        short         key_file_no;
        unsigned long key_io_timer;
        char          domain_name[16];
        char          pathmon_name[16];
        char          server_class[16];
        unsigned long pathsend_timer;
        short         retry_cnt;
    } enc_start_data;
    struct {
        char          mac_value[32];
        short         mac_len;
        char          kc[32];
        short         kc_len;
        char          kmac[32];
        short         kmac_len;
    } enc_conf_data;
} t_encdec_con;

/* ログ出力接続情報 */
typedef struct __t_logcon_data
{
    char          domain_name[8];
    char          pathmon_name[16];
    char          server_class[16];
    unsigned long pathsend_timer;
    short         retry_cnt;
} t_logcon_data;

/* ファイル情報 */
typedef struct __t_file_data
{
    char      phy_file_name[47+1];
    short     phy_file_no;
    char      station_sts_file_name[47+1];
    short     station_sts_file_no;
    char      line_ctl_file_name[47+1];
    short     line_ctl_file_no;
    char      line_st_file_name[47+1];
    short     line_st_file_no;
    char      nw_file_name[47+1];
    short     nw_file_no;
    char      data_turn_file_name[47+1];
    short     data_turn_file_no;
} t_file_data;

/* エラーログ情報 */
typedef struct __t_err_log
{
    short     file_id;
} t_err_log;

/* 編集電文 */
typedef struct __t_make_data
{
    short   make_data_length;         /* 電文長          */
    char    make_data[10000];         /* 電文            */
} t_make_data;

typedef struct __t_make_data_tbl
{
    t_make_data   rcv_data_enc_set;   /* 受信電文(暗号化情報含む(編集前)) */
    t_make_data   encode_data;        /* 暗号化電文(編集前)               */
    t_make_data   rcv_make_data;      /* 受信電文(暗号化情報含む(編集後)) */
    t_make_data   send_data;          /* 送信電文                         */
} t_make_data_tbl;

/* 物理名情報ファイル プライマリーキー */
#pragma fieldalign shared2 __t_filekey_gfphi
typedef struct __t_filekey_gfphi
{
    struct
    {
        char   site_id;
        char   nw_id;
        char   grp_id[5];
    } pri_key_part1;
    struct
    {
        struct
        {
            char   srv_cls_kind[8];
            char   srv_cls_num[4];
         } srv_cls_id;
         char   srv_cls_mlt_num[4];
    } srv_cls_key;
    struct
    {
        struct
        {
            char   prc_file_kind[8];
            char   prc_file_num[4];
        } prc_file_id;
        char   prc_file_mlt_num[4];
    } prc_file_key;
} t_filekey_gfphi;
#define DEF_GFPHI_PKEY_LEN 39

/* NW情報ファイル プライマリーキー */
#pragma fieldalign shared2 __t_filekey_gfnwi
typedef struct __t_filekey_gfnwi
{
    struct
    {
        char   site_id;
        char   nw_id;
        char   grp_id[5];
    } pri_key_part1;
    char   if_id[5];
    char   station_id[6];
} t_filekey_gfnwi;

/* 送信電文折返し振分先設定ファイル プライマリーキー */
#pragma fieldalign shared2 __t_filekey_gfqbk
typedef struct __t_filekey_gfqbk
{
    struct
    {
        char   site_id;
        char   nw_id;
        char   grp_id[5];
    } pri_key_part1;
    char   mti_id[4];
} t_filekey_gfqbk;

/* 回線管理ファイル プライマリーキー (回線ステータスファイル　プライマリーキー) */
#pragma fieldalign shared2 __t_filekey_gflin
typedef struct __t_filekey_gflin
{
    struct
    {
        char   site_id;
        char   nw_id;
        char   grp_id[5];
    } pri_key_part1;
    char   if_id[5];
    char   station_id[6];
    char   connect_id[6];
} t_filekey_gflin;
#define DEF_GFLIN_PKEY_LEN 24

/* 局状態管理ファイル プライマリーキー */
#pragma fieldalign shared2 __t_filekey_gcsst
typedef struct __t_filekey_gcsst
{
    char   site_id;
    char   nw_id;
    char   grp_id[5];
    char   if_id[5];
    char   station_id[6];
    char   connect_id[6];
} t_filekey_gcsst;
#define DEF_GCSST_PKEY_LEN 24

/* 関数のプロトタイプ宣言 */
int main (void);
void  MDSO_initialize (void);
short MDSO_init_param (void);
short MDSO_init_getphyfile (void);
short MDSO_init_getnwfile (void);
short MDSO_encdec_init (void);
short MDSO_turn_tbl_make(void);
short MDSO_conlist_make(void);
void  MDSO_main (void);
short MDSO_recv_read (void);
short MDSO_req_recv (common_header_def*, short );
short MDSO_ipc_chk (common_header_def*, short );
short MDSO_recv_reply (char*, short, char*);
short MDSO_que_req_recv (c302_def*);
short MDSO_send_judge ( c302_def*, short* );
short MDSO_send_judge_rr ( short* );
short MDSO_encode ( c302_def*);
short MDSO_header_edit(void);
short MDSO_data_make ( c302_def*, t_con_list*, c202_def* );
short MDSO_log_output ( c302_def*, c202_def*, char* );
short MDSO_data_con_send ( t_con_list *, c202_def *);
short MDSO_err_resp ( c302_def* );
short MDSO_turn_que_get ( char*, short* );
short MDSO_turn_ipc_make ( c302_def*, c301_def* );
short MDSO_turn_ipc_send (  t_turn_tbl*, c301_def*);
short MDSO_status_req (c107_def*);
short MDSO_err_log_set ( c302_def*, char, char*);
short MDSO_file_up(void);
void  MDSO_sys_open (void);
void  MDSO_sys_close (void);
void  MDSO_end (void);
void  MDSO_message_output(short, char*, char*, ...);
void  MDSO_module_ems_make( oggz1in_def* );
short MDSO_station_sts_read (short, char*);
short MDSO_phy_file_read ( t_con_list* );

