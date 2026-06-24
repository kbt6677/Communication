/*****************************************************************************/
/*****                    <<     head PROGRAM      >>                    *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVX30                                    */
/*        FUNCTION          ････ 電文振分(inbound)                           */
/*                                                                           */
/*        AUTHER            ････ HAS kimura                                  */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2024-09-24                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS 木村   2024/09/25 (電文振分(inbound))新規作成                   */
/****************************************************************************/

/* USER HEADER */
/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
/* 共通DEFINE */
#define MDSI_RET_OK                         0
#define MDSI_RET_NG                         -1
#define MDSI_IPC_ERR                        9
#define MDSI_TOHAN_TOKYO_INT                1
#define MDSI_TOHAN_OSAKA_INT                2
#define MDSI_DETOUR_IMPOSSIBLE_INT          0
#define MDSI_RET_DOUBLE_SET                 3
#define MDSI_ZERO                           0
#define MDSI_SPACE                          ' '
#define MDSI_DETOUR_IMPOSSIBLE              ' '
#define MDSI_SC_ID_LENG                     23  /* サーバークラス論理ID長 */
#define MDSI_SITE_ID_LENG                   1   /* サイトID長 */
#define MDSI_NW_ID_LENG                     1   /* NW ID長 */
#define MDSI_GROUP_ID_LENG                  5   /* グループID長 */
#define MDSI_SERVERCLASS_NAME_LENG          8   /* サーバクラス種類長 */
#define MDSI_SERVERCLASS_NO_LENG            4   /* サーバクラス論理番号長 */
#define MDSI_MSG_SERVERCLASS_LENG           16  /* メッセージサーバークラス長 */
#define MDSI_MSG_PASSMON_LENG               6   /* メッセージPATHMON長 */
#define MDSI_EMS_COLLECTOR_LENG             6   /* EMSコレクタ長 */
#define MDSI_TOHAN_LIST_MAX                 100 /* 東阪振分リスト最大長 */
#define MDSI_STATION_STATUS_TBL_MAX         (30*80) /* 局状態管理ファイル数 */
#define MDSI_IF_MAX                         (30*80) /* インタフェース最大数 */
#define MDSI_REDUN_ZERO                     "0000" /* 冗長番号 0000 */
#define MDSI_UKAI_EXEC                      1   /* 迂回実行 */

/* フラグ */
#define MDSI_ON                             1
#define MDSI_OFF                            0
#define MDSI_NOMAL_END                      2    /* プロセス正常終了 */
#define MDSI_NOT_SET                        0xFF /* 未設定           */

/* MTI取得対象メッセージ判定結果 */
#define MDSI_MTI_JUDGE_MTI                  1
#define MDSI_MTI_JUDGE_QUE                  2
#define MDSI_MTI_JUDGE_HB                   3
#define MDSI_MTI_JUDGE_IDL                  4
#define MDSI_MTI_JUDGE_ERR                  9

/* パラメータサイズ */
#define MDSI_LOG_FILE_NAME_SIZE             48

/* 電文ログ出力パラメータ */
#define MDSI_LOG_SET                         1
#define MDSI_LOG_UPDATE                      2
#define MDSI_RECEIVE_DATA                    '1'
#define MDSI_LOG_HB                          "HB"
#define MDSI_LOG_RJ                          "RJ"
#define MDSI_LOG_IDL                         "00"
#define MDSI_LOG_ERR                         "ZZ"

/* IPC インタフェースコード */
#define MDSI_IPC_C301                       "C301"
#define MDSI_IPC_C201                       "C201"

/* IPC 処理コード区分 */
#define MDSI_DATA_KIND_C301                  "00" /* 電文受信通知 */
#define MDSI_DATA_KIND_C301_FAILURE          "01" /* 障害電文通知 */

/* 東阪振分区分 */
#define MDSI_TOHAN_DETOUR_1                  1  /* 常に東阪振分の対象 */
#define MDSI_TOHAN_DETOUR_2                  2  /* エラー時のみ東阪振分の対象 */
#define MDSI_TOHAN_DETOUR_3                  3  /* 東阪振分の対象外 */
#define MDSI_TOHAN_DETOUR_1_char             '1'  /* 常に東阪振分の対象 */
#define MDSI_TOHAN_DETOUR_2_char             '2'  /* エラー時のみ東阪振分の対象 */
#define MDSI_TOHAN_DETOUR_3_char             '3'  /* 東阪振分の対象外 */

/* IPCチェックエラーコード */
#define MDSI_IPC_ERR_MSG                     1 /* インタフェースコード異常 */
#define MDSI_IPC_ERR_DATA_LEN                2 /* データ長異常 */

/* RECEIVE用 */
#define MDSI_MAX_DATA_SIZE                   12000  /* プロセス間通信データの最大長 */
#define MDSI_RECEIVE_FILENAME                "$RECEIVE" /* $RECEIVE file name */
#define MDSI_UNKNOWN                         -1
#define MDSI_DETOUR_TBL_MAX                  30
#define MDSI_RCV_NOWAITDEPTH                 0     /* enable nowait operations */
#define MDSI_RECVDEPTH                       1     /* enable replies on $RECEIVE */
#define MDSI_COLLECTOR_FILENAME              "$0"  /*メッセージコレクタープロセス名*/
#define MDSI_TAG_RECV                        7     /*$RECEIVE用I/Oタグ*/
#define MDSI_DELAY_TIME                      10    /*$RECEIVE用DELAY時間 */

/* COMファイル */
#define MDSI_FILEIO_TYPE_OPEN                "OPEN    "
#define MDSI_FILEIO_TYPE_CLOSE               "CLOSE   "
#define MDSI_FILEIO_TYPE_START               "START   "
#define MDSI_FILEIO_TYPE_READ                "READ    "
#define MDSI_IO_NORMAL_END                   "00"

/* エラーログファイル */
#define MDSI_ERR_FILE_OPEN                   1
#define MDSI_ERR_FILE_WRITE                  2
#define MDSI_ERR_FILE_CLOSE                  3

/* エラーログ出力パラメータ */
#define MDSI_ERR_LOG_OPEN                    1
#define MDSI_ERR_LOG_WRITE                   2
#define MDSI_ERR_LOG_CLOSE                   3

/* C301データ種別 */
#define MDSI_C301_KIND_NOMAL                 1  /* 通常電文受信 */
#define MDSI_C301_KIND_ERR                   2  /* 障害電文作成依頼 */
#define MDSI_C301_KIND_NOMAL_IPC             "00"  /* 通常電文受信 */
#define MDSI_C301_KIND_ERR_IPC               "01"  /* 障害電文作成依頼 */

/* 暗号化複合 */
#define MDSI_ENCODE                          1  /* 暗号化 */
#define MDSI_DECODE                          2  /* 復号 */

/* SYSTEM メッセージ */
#define MDSI_NORMAL_TERMINATION              0  /*正常終了*/
#define MDSI_ABNORMAL_TERMINATION            1  /*異常終了*/

/* ログ出力エラー */
#define MDSI_RET_LCN_DOUBLE                  1  /* GFP内部LCN重複 */

/* 振分先設定データ */
#define MDSI_SVC_REQ                          '1'  /* 業務要求 */
#define MDSI_SVC_RSP                          '2'  /* 業務応答 */
#define MDSI_CTL_REQ                          '3'  /* 制御要求 */
#define MDSI_CTL_RSP                          '4'  /* 制御応答 */
#define MDSI_CTL_HB                           '5'  /* ハートビート */
#define MDSI_CTL_HAKI                         '9'  /* 破棄通知 */

/* 局状態 */
#define MDSI_STATION_ST_OPEN                  '0'  /* 開局       */
#define MDSI_STATION_ST_CLOSE                 '9'  /* 閉局       */
#define MDSI_STATION_ST_OPEN_PROC             '1'  /* 開局処理中 */
#define MDSI_STATION_ST_CLOSE_PROC            '8'  /* 閉局処理中 */

/* 振分先選択処理区分 */
#define MDSI_TOHAN_PROC_NOMAL     "10"  /* 常に東阪振分の対象電文(正常) */
#define MDSI_TOHAN_PROC_UKAI      "11"  /* 常に東阪振分の対象電文(迂回) */
#define MDSI_TOHAN_PROC_ERR_NOMAL "20"  /* エラー時のみ東阪振分の対象電文(正常) */
#define MDSI_TOHAN_PROC_ERR_UKAI  "21"  /* エラー時のみ東阪振分の対象電文(迂回) */
#define MDSI_TOHAN_PROC_NOT       "30"  /* 東阪振分の対象外電文 */

/* エラーメッセージ */
#define MDSI_MSG_NONE               0
#define MDSI_MSG_INTERNAL_ERR       100

/* EMS */
#define MDSI_CG010_NOR  '0'
#define MDSI_CG010_WARN '9' // pathsend error

/* オープンID */
#define MDSI_OPENID_ANCESTOR        1    /* オープンID(親プロセス)       */
#define MDSI_OPENID_ROUT            2    /* オープンID(ROUT)             */

/* 障害通知有無 */
#define MDSI_SYOGAITUCHI_NASHI     '0'  /* 障害電文通知作成依頼不要     */
#define MDSI_SYOGAITUCHI_ARI       '1'  /* 障害電文通知作成依頼要       */

/* PATHSENDエラーコード */
#define MDSI_PATHSEND_ERR_905      905
#define MDSI_PATHSEND_ERR_913      913
#define MDSI_PATHSEND_ERR_915      915

/* 内部エラーコード */
//#define DEF_NERR_NOMAL                      "0000000"            /* 正常                                       */
//#define DEF_NERR_FILE_IO_ERR                "SCAA001"            /* ファイルIOエラー                           */
//#define DEF_NERR_PSEND_ERR_RE_OUT           "SCAA007"            /* Pathsendエラー(リトライアウト)             */
//#define DEF_NERR_IPC_SEISA_ERR              "SCAA009"            /* IPC精査エラー                              */
//#define DEF_NERR_PRM_RD_ERR_INV             "SCAA017"            /* パラメータ読込エラー(設定不正)             */
//#define DEF_NERR_RCV_DENBUN_LEN_ERR         "SCBA004"            /* 受信電文データ長チェックエラー             */
//#define DEF_NERR_SEND_ERR                   "SCBA006"            /* SENDエラー                                 */
//#define DEF_NERR_DST_SELECT_ERR             "SCCF001"            /* 送信先選択不可                             */
//#define DEF_NERR_SMK_RSP_SEISA_ERR          "SCCE001"            /* 仕向応答精査エラー                         */
//#define DEF_NERR_KC_ENC_ERR                 "SCCE002"            /* 電文暗号化エラー(KC)                       */
//#define DEF_NERR_KMAC_CALC_ERR              "SCCF002"            /* 電文認証値算出エラー(KMAC)                 */
//#define DEF_NERR_SMK_REQ_DUAL_SEND          "SCCF003"            /* 仕向要求二重送信チェックエラー             */
//#define DEF_NERR_SMK_RSP_DUAL_RCV           "SCCE003"            /* 仕向応答二重受信チェックエラー             */
//#define DEF_NERR_HSMK_REQ_SEISA_ERR         "SCCE004"            /* 被仕向要求精査エラー                       */
//#define DEF_NERR_HSMK_REQ_MTI_HANTE_ERR     "SCCE005"            /* 被仕向要求MTI判定不可                      */
//#define DEF_NERR_KC_DEC_ERR                 "SCCE006"            /* 電文復号エラー(KC)                         */
//#define DEF_NERR_KMAC_HANTE_ERR             "SCCE007"            /* 電文認証値判定エラー(KMAC)                 */
//#define DEF_NERR_TO_FURIWAKE_FC2002_ERR     "SCCE008"            /* 東阪振分エラー(FC2002)                     */
//#define DEF_NERR_HSMK_REQ_DUAL_RCV_ERR      "SCCE009"            /* 被仕向要求二重受信チェックエラー           */
//#define DEF_NERR_HSMK_RSP_DUAL_SEND_ERR     "SCCF004"            /* 被仕向応答二重送信チェックエラー           */
//#define DEF_NERR_LCN_GET_ERR                "SCCE010"            /* LCN取得エラー                              */
//#define DEF_NERR_RCV_DENBUN_HEADR_ERR       "SCCE011"            /* 受信電文ヘッダ精査エラー                   */
//#define DEF_NERR_DENBUN_DEC_ERR             "SCCE012"            /* 電文復号処理エラー                         */
//#define DEF_NERR_MTI_GET_ERR                "SCCE013"            /* MTI取得エラー                              */
//#define DEF_NERR_FURIWAKE_DST_HANTE_ERR     "SCCE014"            /* 振分先判定エラー                           */
//#define DEF_NERR_TO_FURIWAKE_FC2004_ERR     "SCCE015"            /* 東阪振分エラー(FC2004)                     */
//#define DEF_NERR_NW_LOG_OUT_ERR             "SCCE016"            /* NW通信ログ出力エラー                       */


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
    char  pathmon_name[16];           /* PATHMONプロセス名              */
    short end_flag;                   /* 終了フラグ                     */
    char  trc_time_begin[21];         /* トレース開始時間               */
    char  trc_time_end[21];           /* トレース終了時間               */
    char  nwid_sign[2];               /* ネットワーク識別サイン         */
    char  msg_pname[6];               /* メッセージ出力プロセス名       */
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

/* I/O完了情報 */
typedef struct __t_iocomp_def
{
    char  *fno;
    short addr;
    short len;
    short tag;
    short ferr;
    zsys_ddl_receiveinformation_def  recv_info;
} t_iocomp_def;

/* 受信電分振分先設定テーブル */
typedef struct __t_detour_tbl
{
    struct {
        char  mti[4];
    } t_primary_key;
    struct {
        char  site_ctl;
        char  denbun_shubetu;
        char  dmy1[8];
    } t_site_ctl;
    struct {
        struct{
            struct{
                char  tokyo_srv_kind[8];
                char  tokyo_srv_num[4];
            } t_tokyo_serverclass;
            struct{
                char  tokyo_file_kind[8];
                char  tokyo_file_num[4];
            } t_tokyo_file_id;
            char  dmy2[16];
        } t_tokyo_site;
        struct{
            struct{
                char  osaka_srv_kind[8];
                char  osaka_srv_num[4];
            } t_osaka_serverclass;
            struct{
            char  osaka_file_kind[8];
            char  osaka_file_num[4];
            } t_osaka_file_id;
            char  dmy3[16];
        } t_osaka_site;
        char  dmy4[20];
    } t_rcv_que;
    struct {
        struct{
            char tokyo_domain_name[8];
            char tokyo_pathmon[16];
            char tokyo_serverclass[16];
        } t_tokyo_site_phy;
        struct {
            char osaka_domain_name[8];
            char osaka_pathmon[16];
            char osaka_serverclass[16];
        }  t_osaka_site_phy;
    } t_phy_data;
} t_detour_tbl;

/* 東阪振分順リスト */
typedef struct __t_tohan_list
{
    char list_no_old;
    char list_no;
    char sort_list[100];
} t_tohan_list;

/* 東阪振分比率 */
typedef struct __t_tohan_rate
{
    short tokyo_rate;
    short osaka_rate;
} t_tohan_rate;

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
    } dec_start_data;
    struct {
        char          mac_value[32];
        short         mac_len;
        char          kc[32];
        short         kc_len;
        char          kmac[32];
        short         kmac_len;
        char          decode_data[9999];
        short         decode_data_length;
    } dec_conf_data;
} t_encdec_con;

/* GFP内部LCN採番接続情報 */
typedef struct __t_lcncon_data
{
    char          domain_name[8];
    char          pathmon_name[16];
    char          server_class[16];
    unsigned long pathsend_timer;
    short         retry_cnt;
} t_lcncon_data;

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
    char      assign_file_name[47+1];
    short     assign_file_no;
    char      tohan_rate_file_name[47+1];
    short     tohan_rate_file_no;
    char      nw_file_name[47+1];
    short     nw_file_no;
} t_file_data;

/* エラーログ情報 */
typedef struct __t_err_log
{
    short     file_id;
} t_err_log;

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
#define DEF_gfphi_pkey_size 7
#define DEF_gfphi_p_key_grp_id 5
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
#define DEF_gfnwi_pkey_size 7
#define DEF_gfnwi_p_key_grp_id 5

/* 受信電文振分先設定ファイル プライマリーキー */
#pragma fieldalign shared2 __t_filekey_gfqsw
typedef struct __t_filekey_gfqsw
{
    struct
    {
        char   site_id;
        char   nw_id;
        char   grp_id[5];
    } pri_key_part1;
    char   mti_id[4];
} t_filekey_gfqsw;
#define DEF_gfqsw_pkey_size 7
#define DEF_gfqsw_p_key_grp_id 5

/* 関数のプロトタイプ宣言 */
int main (void);
void  MDSI_initialize (void);
short MDSI_init_param (void);
short MDSI_init_getphyfile (void);
short MDSI_init_getnwfile (void);
short MDSI_encdec_init (void);
short MDSI_detour_tbl_make(void);
short MDSI_detourlist_make(void);
void  MDSI_main (void);
short MDSI_recv_read (void);
short MDSI_req_recv (common_header_def*, short);
short MDSI_ipc_chk (common_header_def*, short);
short MDSI_recv_reply (char*, short, char*);
short MDSI_con_req_recv (c201_def*);
short MDSI_lcn_get (char*);
short MDSI_decode ( c201_def * );
short MDSI_log_output ( char*,char*,char*,char* );
short MDSI_log_output_up ( char * );
short MDSI_swich_judge ( short ,char*, c201_def*,char*,char* );
short MDSI_detour_que_get ( char*, char*);
short MDSI_tohan_detour ( char );
short MDSI_data_make ( char, c201_def*, t_detour_tbl*, char, char*, c301_def* );
short MDSI_data_send ( t_detour_tbl*, c301_def*, c201_def*);
short MDSI_err_data_make ( c201_def* );
short MDSI_err_detour ( t_detour_tbl* );
short MDSI_err_log_set ( c201_def*, char, char*);
short MDSI_file_up (void);
void  MDSI_sys_open (void);
void  MDSI_sys_close (void);
void  MDSI_end (void);
short MDSI_station_sts_read (c201_def*, char* );
void  MDSI_message_output(short, char*, char*, ...);
void  MDSI_module_ems_make( oggz1in_def* );
void  MDSI_BCD2CHAR(unsigned char*, char*,short);

// 文字変換ルーチン
//_cobol void EBCDIC2SJIS(char*, char*, short);
//void HEX2CHAR(unsigned char *hex_p, char *terget_p,short s_len);

//short TRACEOUT( char* );
