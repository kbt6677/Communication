/**
 * @brief GFPCSJ70.h 
 *
 * @date 2025/03/06 新規作成 by HAS
 *
*/
/* STANDARD HEADER */

/* USER HEADER     */
#include "GFPCGX50.h"                        /* システム日時取得             */
#include "GFPCGX20.h"                        /* システム日時取得             */

#ifndef _GFPCSJ70_H_
#define _GFPCSJ70_H_

/* ------------------------------------------------------------------------------------------ */
/* Define 定義                                                                                */
/* ------------------------------------------------------------------------------------------ */
#define DEF_HDR_LEN                              0x2E     /* hdr_len  (46)                    */
#define DEF_HEAD_FLG                             0x01     /* Header Flag and Version          */
//#define DEF_MODE_00                              0x00     /* モードフラグ                     */
//#define DEF_MODE_10                              0x10     /* モードフラグ                     */
//#define DEF_MSG_TYPE_C804                       "C804"    /* 電文種別コード C804              */
//#define DEF_MSG_TYPE_C814                       "C814"    /* 電文種別コード C814              */

//#define DEF_MTI_STS_0800                        "0800"    /* MTI:0800                         */
//#define DEF_MTI_STS_0810                        "0810"    /* MTI:0810                         */
//#define DEF_MTI_STS_0820                        "0820"    /* MTI:0820                         */
//#define DEF_MTI_STS_0830                        "0830"    /* MTI:0830                         */
//#define DEF_FUNC_CODE_001                        "001"    /* Network Management Information Code  開局    */
//#define DEF_FUNC_CODE_002                        "002"    /* Network Management Information Code  閉局    */
//#define DEF_FUNC_CODE_301                        "301"    /* Network Management Information Code  エコー  */
//#define DEF_PROC_CODE_000000                  "000000"    /* プロセッシングコード             */

//#define DEF_COM_SDT_arg1_gmt                        1     /* 標準時                           */
//#define DEF_COM_SDT_arg1_jpn                        2     /* 日本時間                         */
//#define DEF_COM_SDT_arg1_chn                        3     /* 中国時間                         */
//#define DEF_COM_SDT_jpn_time              32400000000     /* 9時間（マイクロ秒)               */
//#define DEF_COM_SDT_chn_time              28800000000     /* 8時間（マイクロ秒)               */

//#define DEF_NERR_STS_NOMAL                  "0000000"     /* 正常                                       */
//#define DEF_NERR_HSMK_STS_ERR_OPN           "SCDJ002"     /* 被仕向開局処理中の局状態チェックエラー */
//#define DEF_NERR_HSMK_STS_ERR_CLS           "SCDJ006"     /* 被仕向閉局処理中の局状態チェックエラー */
//#define DEF_NERR_HSMK_STS_ERR_ECH           "SCDJ022"     /* 被仕向エコーテスト局状態チェックエラー */

#define DEF_ACT_NORMAL                          "00"      /* 正常                             */
                                                          /* 制御電文種別3桁目                */
//#define DEF_RSP_KYOKA                             'A'     /* 許可応答                         */
//#define DEF_RSP_NO_OPTION                         '0'     /* オプション無し                   */
//#define DEF_RSP_FORCE_EXECUTION                   '1'     /* 強制実行                         */
//#define DEF_RSP_STAT_UPDATE                       '2'     /* 状態更新                         */


//#define DEF_REQ_TYPE_SRSP                        "20"     /* 仕向応答                         */
//#define DEF_REQ_TYPE_SRSP_TOUT                   "30"     /* 仕向応答Timeout                  */
//#define DEF_REQ_TYPE_SREQ_SND_ERR                "40"     /* 仕向要求送信不可                 */

//#define DEF_CHK_ERR_CTR_TYPE               "Ctrl-Type"
//#define DEF_CHK_ERR_CTRLHD_TYPE       "Ctrl-Head-Type"
//#define DEF_CHK_ERR_TOTAL_LEN           "Total-Legnth"
//#define DEF_CHK_ERR_DST_CENT_ID        "Dst-Center-ID"
//#define DEF_CHK_ERR_SRC_CENT_ID        "Src-Center-ID"
//#define DEF_CHK_ERR_CTL_MERCH_CD      "Ctl-Merch-Code"
//#define DEF_CHK_ERR_SMK_KUBN           "Shimuke-Kubun"
//#define DEF_CHK_ERR_SEND_TIME              "Send-time"
//#define DEF_CHK_ERR_MODE_FLG               "Mode-Flag"
//#define DEF_CHK_ERR_HD_TYPE                "Head-Type"
//#define DEF_CHK_ERR_MSG_TYPE            "Message_Type"
//#define DEF_CHK_ERR_CUT_DATE                "Cut_Date"
//#define DEF_CHK_ERR_BODY_LEN             "Body_Length"
//#define DEF_CHK_ERR_CDNT_ID               "Cardnet_Id"
//#define DEF_CHK_ERR_CDNT_SEQ             "Cardnet_Seq"
#define DEF_CHK_ERR_HDR_LEN            "Header Length"
#define DEF_CHK_ERR_TTL_LEN     "Total Message Length"
#define DEF_CHK_ERR_HEAD_FLG             "Header Flag"
#define DEF_CHK_ERR_MTI                          "MTI"
#define DEF_CHK_ERR_BIT_007                   "BIT007"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_011                   "BIT011"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_033                   "BIT033"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_039                   "BIT039"     /* エラー発生BIT        */
//#define DEF_CHK_ERR_BIT_048                   "BIT048"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_053                   "BIT053"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_070                   "BIT070"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_096                   "BIT096"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_100                   "BIT100"     /* エラー発生BIT        */
//#define DEF_CHK_ERR_BIT_128                   "BIT128"     /* エラー発生BIT        */

//#define DEF_CST_CHK_OK                               0     /* 局状態チェックOK     */
//#define DEF_CST_CHK_NG                               1     /* 局状態チェックNG     */
//#define DEF_SEND_SRC_CODE_LEN                       11     /* 送信元識別コード長   */

#define DEF_BIT007_DATA_LEN                         10     /* b007_trans_date_time       */
#define DEF_BIT011_DATA_LEN                          6     /* b011_system_audit_number   */
#define DEF_BIT033_DATA_LEN                         11     /* b033_fowd_inst_id_code     */
#define DEF_BIT039_DATA_LEN                          2     /* b039_response_code         */
//#define DEF_BIT048_DATA_LEN                        512     /* b048_add_data_private      */
#define DEF_BIT053_DATA_LEN                         16     /* b053_security_ctl_info     */
#define DEF_BIT070_DATA_LEN                          3     /* b070_nw_mng_code           */
//#define DEF_BIT096_DATA_LEN                          8     /* b096_msg_security_code     */
#define DEF_BIT100_DATA_LEN                         11     /* b100_recv_inst_id_code     */
//#define DEF_BIT128_DATA_LEN                         08     /* b128_msg_auth_code         */



/* ------------------------------------------------------------------------------------------ */
/* 関数のプロトタイプ宣言                                                                     */
/* ------------------------------------------------------------------------------------------ */
short  NWM_STE_char2hex  (char *, char *, short );
short  NWM_bcd_check     (char *, short );
void   NWM_bcd_to_char   (char *, char *, short );
#endif

