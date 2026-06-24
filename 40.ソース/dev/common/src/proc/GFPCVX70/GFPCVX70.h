/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVX70                                    */
/*        FUNCTION          ････ 制御電文振分                                */
/*                               制御電文機能                                */
/*                                                                           */
/*        AUTHER            ････ HAS M.Matsumoto                             */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025-02-25                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  Matsumoto  2025/02/25 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/

#ifndef _GFPCVX70_H
#define _GFPCVX70_H

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/

/* サーバ管理テーブルインデックス */
#define DEF_CMSD_SV_NONE                -1      /* 未登録                           */
#define DEF_CMSD_SV_TIMER               0       /* タイマー制御サーバ               */
#define DEF_CMSD_SV_TIMER_B             1       /* タイマー制御サーバ(BACKUP)       */
#define DEF_CMSD_SV_SIGN_ECHO           2       /* 局状態・エコー制御サーバ         */
#define DEF_CMSD_SV_SIGN_ECHO_AUTO      3       /* 局状態・エコー制御サーバ(自動)   */
#define DEF_CMSD_SV_KEYEXC              4       /* 鍵交換制御サーバ                 */
#define DEF_CMSD_SV_CUTOVER             5       /* カットオーバー制御サーバ         */
#define DEF_CMSD_SV_SAF                 6       /* SAF送信制御サーバ                */
#define DEF_CMSD_SV_NOTICE              7       /* 通知電文制御サーバ               */
#define DEF_CMSD_SV_CTL_OUT             8       /* 制御電文IF(outbound)サーバ       */
#define DEF_CMSD_SV_APL_IN              9       /* 業務電文中継(inbound)サーバ      */
#define DEF_CMSD_SV_APL_CTL             10      /* 制御電文中継(inbound)サーバ      */
#define DEF_CMSD_SV_MAX                 11      /* テーブル数                       */

/* タイマー情報 */
#define DEF_CMSD_FNC_SIGNON             0       /* 開局応答待ち                     */
#define DEF_CMSD_FNC_SIGNOFF            1       /* 閉局応答待ち                     */
#define DEF_CMSD_FNC_ECHO               2       /* エコーテスト応答待ち             */
#define DEF_CMSD_FNC_KEYREQ             3       /* 鍵交換依頼応答待ち               */
#define DEF_CMSD_FNC_KEYEXC             4       /* 鍵交換応答待ち                   */
#define DEF_CMSD_FNC_CUTOVER            5       /* カットオーバー依頼待ち           */
#define DEF_CMSD_FNC_SAF                6       /* SAF送信開始/停止応答待ち         */

/* サーバ管理テーブル管理フラグ */
#define DEF_CMSD_MNG_OFF                '0'     /* 管理機能無し                     */
#define DEF_CMSD_MNG_ON                 '1'     /* 管理機能有り                     */

/* サーバ管理テーブル管理単位 */
#define DEF_CMSD_MNG_IF                 'I'     /* インタフェース単位               */
#define DEF_CMSD_MNG_ST                 'S'     /* ステーション単位                 */
#define DEF_CMSD_MNG_NONE               ' '     /* 対象外                           */

/* 障害電文通知作成依頼振分先MTI */
#define DEF_GFQSW_ILLEGAL_MTI           "ZZZZ"

/* ビットマップ展開・組立て */
#define DEF_CMSD_FFDMTI_MAX             8       /* NW制御電文最大MTI数              */
#define DEF_CMSD_FFDBIT_MAX             32      /* NW制御電文MTI単位さだいビット数  */
#define DEF_CMSD_FFDFMT_MAX             2       /* 固定フォーマットバージョン数     */
                                                /* 固定フォーマットインデックス数   */
#define DEF_CMSD_FFDIDX_MAX             (DEF_CMSD_FFDMTI_MAX * DEF_CMSD_FFDFMT_MAX)
                                                /* 固定フォーマット数               */
#define DEF_CMSD_FFD_MAX                (DEF_CMSD_FFDMTI_MAX * DEF_CMSD_FFDBIT_MAX * DEF_CMSD_FFDFMT_MAX)

/* 固定フォーマット */
#define DEF_CMSD_FFMT_SIZE_MAX          12000   /* 固定フォーマット最大長 */

/* 検索単位区分 */
#define DEF_CMSD_UNIT_INTERFACE         1       /* インタフェース単位               */
#define DEF_CMSD_UNIT_STATION           2       /* ステーション単位                 */
#define DEF_CMSD_KEY_LCN                1       /* GFP内部LCN                       */
#define DEF_CMSD_KEY_MATCH              2       /* 要求/応答マッチングキー          */
#define DEF_CMSD_KEY_TIMER              3       /* タイマー情報キー                 */

/* 検索結果 */
#define DEF_CMSD_FILE_NORMAL            0       /* 正常                             */
#define DEF_CMSD_FILE_EOF               1       /* EOF                              */
#define DEF_CMSD_FILE_ERROR             2       /* エラー                           */

/* サーバ処理結果(受信電文通知) */
#define DEF_CMSD_STS_SENDDATA           0       /* 正常(応答電文あり)               */
#define DEF_CMSD_STS_NORMAL             1       /* 正常(応答電文なし)               */
#define DEF_CMSD_STS_ERRORRESPONSE      2       /* エラー応答                       */
#define DEF_CMSD_STS_FAULTTEXT          3       /* 障害電文                         */

#define DEF_CMSD_STS_SENDDATAERROR      7       /* 電文送信エラー                   */
#define DEF_CMSD_STS_IFERROR            8       /* インタフェースエラー             */
#define DEF_CMSD_STS_PATHSENDERROR      9       /* PATHSENDエラー                   */

/* サーバ処理結果(要求電文作成) */
#define DEF_CMSD_STS_REQANDRES          4       /* 正常(要求電文あり、応答電文あり) */
#define DEF_CMSD_STS_REQONLY            5       /* 正常(要求電文あり、応答電文なし) */
#define DEF_CMSD_STS_NOREQ              6       /* 正常(要求電文なし) */

/* タイマー制御 */
#define DEF_IPC_IFCD_TIMER_ADD_REQ      "TM01"  /* タイマー発行要求                 */
#define DEF_IPC_IFCD_TIMER_ADD_RSP      "TM02"  /* タイマー発行応答                 */
#define DEF_IPC_IFCD_TIMER_CAN_REQ      "TC01"  /* タイマーキャンセル要求           */
#define DEF_IPC_IFCD_TIMER_CAN_RSP      "TC02"  /* タイマーキャンセル応答           */

#define DEF_TIMER_NOENTRY               ' '     /* タイマー未登録                   */
#define DEF_TIMER_ENTRY_PRIMARY         'P'     /* プライマリータイマー登録         */
#define DEF_TIMER_ENTRY_BACKUP          'B'     /* バックアップタイマー制御         */

/* 管理階層識別 */
#define DEF_MNG_LYR_INTERFACE           'I'     /* インタフェース単位               */
#define DEF_MNG_LYR_STATION             'S'     /* ステーション単位                 */
#define DEF_MNG_LYR_CONNECTION          'C'     /* コネクション単位                 */

/* エラー出力タイプ */
#define DEF_CMSD_ELG_RECVQUE            1       /* 受信キュー                       */
#define DEF_CMSD_ELG_SENDQUE            2       /* 送信キュー                       */
#define DEF_CMSD_ELG_SENDRES            3       /* 送信応答電文                     */
#define DEF_CMSD_ELG_SENDREQ            4       /* 送信要求電文                     */

/* 内部エラーコード */
//#define DEF_NERR_CMSD_SMK_RSP_TIMEOUT       "SCDI001"   /* 仕向応答待ちタイムアウト                   */
//#define DEF_NERR_CMSD_SMK_RSP_MCH_ERR       "SCDI002"   /* 仕向応答マッチングエラー                   */
//#define DEF_NERR_CMSD_CTL_DISCARD           "SCDI003"   /* 制御電文破棄                               */
//#define DEF_NERR_CMSD_CTL_SEND_ERR          "SCDI004"   /* 制御電文送信エラー                         */
//#define DEF_NERR_CMSD_BITMAP_DEC_ERR        "SCDI005"   /* ビットマップ展開エラー                     */
//#define DEF_NERR_CMSD_BITMAP_ENC_ERR        "SCDI006"   /* ビットマップ結合エラー                     */
//#define DEF_NERR_CMSD_TIMER_ERR             "SCDI007"   /* タイマー発行エラー                         */
//#define DEF_NERR_CMSD_TIMER_CNSL_ERR        "SCDI008"   /* タイマー解除エラー                         */

/****************************************************************************/
/*   TYPEDEF定義                                                            */
/****************************************************************************/

/* 個別ファイル情報 */
typedef struct __t_kbt_file_data
{
    char    GFMTL_file_name[ZSYS_VAL_LEN_FILENAME+1];   /* 制御電文管理ファイル名(物理)           */
    short   GFMTL_file_no;                              /* 制御電文管理ファイル番号               */
    char    GFQSW_file_name[ZSYS_VAL_LEN_FILENAME+1];   /* 受信電文振分先設定ファイル名(物理)     */
    short   GFQSW_file_no;                              /* 受信電文振分先設定ファイル番号         */
    char    GFELI_file_name[ZSYS_VAL_LEN_FILENAME+1];   /* 制御電文エレメント情報ファイル名(物理) */
    short   GFELI_file_no;                              /* 制御電文エレメント情報ファイル番号     */
} t_kbt_file_data;

/* サーバ管理テーブル */
typedef struct __server_tbl_def
{
    char                logicalname[12+1];      /* サーバクラス論理ID   */
    char                domainname[8+1];        /* ドメイン名           */
    char                monname[16+1];          /* PATHMON名            */
    char                scname[16+1];           /* サーバクラス名       */
    short               send_len;               /* 送信IPC長            */
    short               recv_len;               /* 受信IPC長            */
} server_tbl_def;

/* 制御電文管理管理情報 */
typedef struct __ctl_info_dev
{
    short               mng_layer;              /* 制御電文管理単位             */
    short               server_idx;             /* サーバ管理テーブル番号       */
    short               server_sts;             /* サーバ処理ステータス         */
    char                internal_err[7];        /* 内部エラーコード             */
    char                req_gfp_lcn[15];        /* GFP内部LCN                   */
    char                matching_key[50];       /* マッチングキー               */
    short               matching_key_len;       /* マッチングキー長             */
    char                timer_key[20];          /* タイマーキー                 */
    char                timer_entry_kbn;        /* タイマー制御登録区分         */
    char                yobi[19];               /* 予備                         */
    short               msg_off;                /* 電文開始位置                 */
    short               mti_off;                /* MTIオフセット                */
    short               mti_len;                /* MTI項目長                    */
    short               bitmap_off;             /* ビットマップオフセット       */

    struct {                                    /* 電文ログKEY                  */
        struct {                                /* トランザクションID           */
            char            gfp_lcn[15];        /* GFP内部LCN                   */
            char            denbun_keitai;      /* 電文形態                     */
        } tran_id;
        char                denbun_shubetu;     /* 電文種別                     */
        char                re_send_num[3];     /* 再送回数                     */
    } denbun_log_key;

    struct {                                    /* 受信コネクション論理ID       */
        char                site_id;
        char                nw_id;
        char                grp_id[5];
        char                if_id[5];
        char                station_id[6];
        char                connect_id[6];
    } connection_lid;

    struct {
        unsigned short      ipc_len;            /* 受信IPC長                    */
        char                mti[4];             /* MTI                          */
        control_kind_def    ctlkind;            /* 制御電文種別                 */
        unsigned short      qfile_len;          /* 受信キューレコード長         */
        unsigned short      qdata_len;          /* 受信キュー電文長             */
    } qrecv_info;

    struct {
        unsigned short      ipc_len;            /* 送信IPC長                    */
        char                mti[4];             /* MTI                          */
        control_kind_def    ctlkind;            /* 制御電文種別                 */
        unsigned short      qfile_len;          /* 送信キューレコード長         */
        unsigned short      qdata_len;          /* 送信キュー電文長             */
    } qsend_info;

    struct {
        char                mti[4];             /* MTI                          */
        control_kind_def    ctlkind;            /* 制御電文種別                 */
    } pserv_info;

} ctl_info_def;

/* フォーマット変換情報 */
typedef struct __format_info_def
{
    size_t      ffmt_len;           /* 固定フォーマット長       */
    size_t      iso8583_len;        /* ISO8583フォーマット長    */
} format_info_def;

/****************************************************************************/
/*   プロトタイプ関数宣言                                                   */
/****************************************************************************/
short CMSD_get_branch_info(void);
short CMSD_get_serverclass_names(void);
short CMSD_get_iso_elements(void);
void CMSD_inbound_request(void);
void CMSD_inbound_response(short);
void CMSD_recv_request(void);
void CMSD_recv_request_send_error(void);
void CMSD_recv_response(void);
void CMSD_command_request(void);
void CMSD_command_request_send_error(short);
void CMSD_command_response(short);
void CMSD_request_send_error(void);
void CMSD_response_send_error(void);
void CMSD_response_timeout(void);
short CMSD_server_recv_notice(short, char *);
short CMSD_server_edit_request(short);
short CMSD_outbound_text(short);
void CMSD_outbound_reject(void);
short CMSD_timer_entry(char);
short CMSD_timer_cancel(void);
short CMSD_pathsend(short, char*, short);
short CMSD_errorlog(short, char *, unsigned short);
short CMSD_deploy_bitmap(void);
short CMSD_create_bitmap(void);
short CMSD_read_netfile(short, char*, char*);
short CMSD_read_controlfile(short);
short CMSD_write_controlfile(void);
short CMSD_update_controlfile(void);
void CMSD_unlock_controlfile(void);

#endif /* _GFPCVX70_H */
