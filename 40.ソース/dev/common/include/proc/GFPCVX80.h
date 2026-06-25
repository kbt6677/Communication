/**
 * @brief GFPCVX80.h 
 *
 * @date 2025/03/06 新規作成 by HAS
 *
*/
/* STANDARD HEADER */

/* USER HEADER     */

#ifndef _GFPCVX80_H_
#define _GFPCVX80_H_

/* ------------------------------------------------------------------------------------------ */
/* Define 定義                                                                                */
/* ------------------------------------------------------------------------------------------ */
#define DEF_FL_CNCT_INFO                    "FLNGFNWS"    /* 接続先固有情報ファイル           */

                                                          /* 接続先固有情報TBLインデックス    */
#define DEF_FNWS_IDX_MAX                             4    /*  MAXレコード数                   */
#define DEF_FNWS_IDX_NETWORK                         0    /*  NW識別                          */
#define DEF_FNWS_IDX_INTERFACE                       1    /*  インタフェース識別              */
#define DEF_FNWS_IDX_STATION                         2    /*  ステーション識別                */
#define DEF_FNWS_IDX_CNNNECTION                      3    /*  コネクション識別                */

#define DEF_FNWS_KYOKU_SEND                        '1'    /*  開局送信要                      */
#define DEF_REQRSP_KBN_REQ                         '1'    /*  要求                            */
#define DEF_REQRSP_KBN_RSP                         '2'    /*  応答                            */

#define DEF_REPLY_CD_ERROR                           9    /* REPLYエラー                      */
/* ------------------------------------------------------------------------------------------ */
                                                          /* 処理結果種別                     */
#define DEF_RSP_TYPE_NORMAL                          0    /* 正常応答                         */
#define DEF_RSP_TYPE_KYOHI                           1    /* 拒否応答                         */
#define DEF_RSP_TYPE_OBST                            2    /* 障害通知                         */
#define DEF_RSP_TYPE_HAKI                            3    /* 破棄                             */
#define DEF_RSP_TYPE_ERROR                           4    /* エラー応答                       */
/* ------------------------------------------------------------------------------------------ */
//                                                     /* 開局・閉局・エコー局状態判定結果      */
//#define DEF_SSTS_RCV_ON_SND_ON                       0    /* コマンド受付可(電文送信あり)     */
//#define DEF_SSTS_RCV_ON_SND_OFF                      1    /* コマンド受付可(電文送信なし)     */
//#define DEF_SSTS_RCV_OFF                             2    /* コマンド受付不可                 */
/* ------------------------------------------------------------------------------------------ */
                                                          /* 制御電文ログ編集種別             */
#define DEF_GLMLG_RSTYP_RECV                         0    /* RECEIVEデータ                    */
#define DEF_GLMLG_RSTYP_REPL                         1    /* REPLYデータ                      */
                                                          /* 制御電文ログREAD種別             */
#define DEF_GLMLG_READ_REQ                           0    /* 受信電文                         */
#define DEF_GLMLG_READ_RSP                           1    /* 要求電文(仕向応答電文受信時)     */
                                                          /* 401:NW制御電文受信               */
#define DEF_SONF_H_RQ_RV                             0    /* 開閉局_被仕向要求受信            */
#define DEF_SONF_H_RQ_SD                             1    /* 開閉局_被仕向応答送信            */
#define DEF_SONF_S_RP_RV                             2    /* 開閉局_仕向応答受信              */
#define DEF_SONF_S_RP_TIMEOUT_RV                     3    /* 開閉局_仕向応答待ちTimeout受信   */
#define DEF_SONF_S_RQ_SNDERR_RV                      4    /* 開閉局_仕向要求送信不可受信      */
#define DEF_SONF_H_RP_SNDERR_RV                      5    /* 開閉局_被仕向応答送信不可受信    */
#define DEF_ECHO_H_RQ_RV                             6    /* エコー_被仕向要求受信            */
#define DEF_ECHO_H_RQ_SD                             7    /* エコー_被仕向要求送信            */
#define DEF_ECHO_S_RP_RV                             8    /* エコー_仕向応答受信              */
#define DEF_ECHO_S_RP_TIMEOUT_RV                     9    /* エコー_仕向応答待ちTimeout受信   */
#define DEF_ECHO_S_RQ_SNDERR_RV                     10    /* エコー_仕向要求送信不可受信      */
#define DEF_ECHO_H_RP_SNDERR_RV                     11    /* エコー_被仕向応答送信不可受信    */
                                                          /* 402:制御電文受信                 */
#define DEF_SONF_S_RQ_SD                            12    /* 開閉局_仕向要求送信              */
#define DEF_ECHO_S_RQ_SD                            13    /* エコー_仕向要求送信              */

#define DEF_STAN6                                 "03"    /* システム採番の通番区分           */
#define DEF_CHR_FLG_ON                              '1'   /* FLAG(char)ON                     */
#define DEF_ERR_ID_NAIBU                            '2'   /* エラー電文識別：2(内部エラー)    */

#define DEF_PSEND_DATA_MAX                       12000    /* PATHSEND MAX DATA LENGTH         */
#define DEF_SSTS_LEN                                2     /* 局状態桁数                       */
#define DEF_MTI_LEN                                 4     /* MTIレングス                      */
#define DEF_LCN_NO_LEN                             15     /* LCN Noレングス                   */
#define DEF_NAIBU_ERR_CD_LEN                        7     /* 内部エラーコード                 */

#define DEF_LCN_INIT_SPACE          "               "     /* LCN初期値                        */
#define DEF_NERR_INIT_SPACE                 "       "     /* 初期値                           */
//#define DEF_NERR_HSMK_SST_OPNING            "SCDJ002"     /* 局状態エラー(被仕向開局処理中)   */
//#define DEF_NERR_HSMK_SST_OPN               "SCDJ004"     /* 局状態エラー(被仕向開局)         */
//#define DEF_NERR_HSMK_SST_CLSING            "SCDJ006"     /* 局状態エラー(被仕向閉局処理中)   */
//#define DEF_NERR_HSMK_SST_CLS               "SCDJ008"     /* 局状態エラー(被仕向閉局)         */
//#define DEF_NERR_SMK_SST_OPNING             "SCDJ010"     /* 局状態エラー(仕向開局処理中)     */
//#define DEF_NERR_SMK_SST_CLSING             "SCDJ016"     /* 局状態エラー(仕向閉局処理中)     */
//#define DEF_NERR_SMK_SST_CTLINT_ERR         "SCDJ017"     /* 局状態エラー(仕向処理区分エラー) */
//#define DEF_NERR_HSMK_SST_ECHO              "SCDJ022"     /* 局状態エラー(被仕向エコー)       */
//#define DEF_NERR_SMK_SST_ECHO               "SCDJ024"     /* 局状態エラー(仕向エコー)         */
//#define DEF_NERR_SMK_SST_CLS                "SCDJ028"     /* 局状態エラー(仕向閉局)           */
//#define DEF_NERR_SYS_NO_MAKE_ERR            "SCDJ030"     /* システム採番生成失敗             */
//#define DEF_NERR_ERRLOG_OUTPUT_ERR          "SCDJ031"     /* エラーログ出力エラー             */
//#define DEF_NERR_SIGN_ON_RETRY_OVER         "SCDJ032"     /* 自動開局要求送信リトライオーバー */
//#define DEF_NERR_SIGN_ON_RETRY_ERR          "SCDJ033"     /* 自動開局要求送信リトライエラー   */

                                                          /* 自動開局判定結果                 */
#define DEF_CSTE_OPEN_NOAUTO                         0    /* 自動開局以外                     */
#define DEF_CSTE_OPEN_AUTO                           1    /* 自動開局                         */
                                                          /* 開局リトライ有無                 */
#define DEF_CSTE_RETRY_OFF                           0    /* 開局リトライ無し                 */
#define DEF_CSTE_RETRY_ON                            1    /* 開局リトライ有り                 */

#define DEF_CSTE_STS_SPACE                        "  "    /* 更新後局状態 スペース            */

/* ------------------------------------------------------------------------------------------ */
/* 構造体                                                                                     */
/* ------------------------------------------------------------------------------------------ */
typedef struct __t_kbt_file_data                          /* 個別ファイル情報                 */
{                                                         /* -------------------------------  */
    char   cnct_inf_fname[ZSYS_VAL_LEN_FILENAME+1];       /* 接続先固有情報ファイル物理名     */
    short  cnct_inf_fno;                                  /* 接続先固有情報ファイル番号       */
    char   stan_sts_fname[ZSYS_VAL_LEN_FILENAME+1];       /* 局状態管理ファイル物理名         */
    short  stan_sts_fno;                                  /* 局状態管理ファイル番号           */
    char   echo_mng_fname[ZSYS_VAL_LEN_FILENAME+1];       /* エコー状態管理ファイル物理名     */
    short  echo_mng_fno;                                  /* エコー状態管理ファイル番号       */
    char   ctrl_log_fname[ZSYS_VAL_LEN_FILENAME+1];       /* 制御電文ログファイル物理名       */
    short  ctrl_log_fno;                                  /* 制御電文ログファイル番号         */
} t_kbt_file_data;
                                                          /* -------------------------------  */
typedef struct __t_kbt_svrcls_data                        /* 送信ServerClass情報              */
{                                                         /* -------------------------------  */
    char   lcn_domain_name [ 8+1];                        /* GFP内部LCN採番サーバdomain名     */
    char   lcn_pathmon_name[16+1];                        /* GFP内部LCN採番サーバPATHMON名    */
    char   lcn_srvcls_name [16+1];                        /* GFP内部LCN採番サーバServerClass  */
    char   ser_domain_name [ 8+1];                        /* system採番生成サーバdomain名     */
    char   ser_pathmon_name[16+1];                        /* system採番生成サーバPATHMON名    */
    char   ser_srvcls_name [16+1];                        /* system採番生成サーバServerClass  */
    char   cmd_domain_name [ 8+1];                        /* コマンドサーバdomain名           */
    char   cmd_pathmon_name[16+1];                        /* コマンドサーバPATHMON名          */
    char   cmd_srvcls_name [16+1];                        /* コマンドサーバServerClass        */
} t_kbt_svrcls_data;
                                                          /* -------------------------------  */
typedef struct __t_gfnws_pri_key_def                      /* 接続先固有情報ファイルP-Key      */
{                                                         /* -------------------------------  */
    char   nw_id;                                         /* N/W識別                          */
    char   if_id[5];                                      /* インタフェース識別               */
    char   station_id[6];                                 /* ステーション識別                 */
    char   connect_id[6];                                 /* コネクション識別                 */
} t_gfnws_pkey_def;
                                                          /* -------------------------------  */
typedef struct __t_gcest_pri_key_def                      /* エコー状態管理ファイルP-Key      */
{                                                         /* -------------------------------  */
    char   site_id;                                       /* サイト識別                       */
    char   nw_id;                                         /* N/W識別                          */
    char   gp_id[5];                                      /* グループ識別                     */
    char   if_id[5];                                      /* インタフェース識別               */
    char   st_id[6];                                      /* ステーション識別                 */
    char   cn_id[6];                                      /* コネクション識別                 */
} t_gcest_pkey_def;

typedef struct __t_rcv_info_def                           /* 処理結果情報                     */
{
    char             ctrl_type[4];                        /* 制御電文種別                     */
    char             sys_no[6];                           /* システム通番                     */
    char             naibu_errcd[7];                      /* 内部エラーコード                 */
    char             new_stn_sts[2];                      /* 更新後局状態                     */
    char             lcn_no[15];                          /* LCN                              */
    short            rsp_result;                          /* 受信電文判定結果                 */
    short            denbun_len;                          /* 電文レングス                     */
    char             err_area[20];                        /* 精査エラー項目                   */
    char             mti[4];                              /* MTI                              */
    short            repl_data_len;                       /* REPLYデータレングス              */
    short            repl_code;                           /* REPLYコード                      */
} t_rcv_info_def;

/* システム採番接続情報 */
typedef struct __t_sys_no_data
{
    char          domain_name[8];
    char          pathmon_name[16];
    char          server_class[16];
    unsigned long pathsend_timer;
    short         retry_cnt;
} t_sys_no_data;

/* EMS出力付加情報 */
typedef struct __ems_info_add
{
   char     srv_logical_id[8];
   char     lcn[15];
   char     connect[24];
} ems_info_add;

/****************************************************************************/
/*   グローバルデータ                                                       */
/****************************************************************************/
t_sys_no_data                g_sys_no_data_pri;         /* システム採番接続情報(primary)      */
t_sys_no_data                g_sys_no_data_sec;         /* システム採番接続情報(secondary)    */

/* ------------------------------------------------------------------------------------------ */
/* 関数のプロトタイプ宣言                                                                     */
/* ------------------------------------------------------------------------------------------ */
#include "NWM_STE.h"                         // NW個別(開局・閉局・エコー個別処理)

#endif
