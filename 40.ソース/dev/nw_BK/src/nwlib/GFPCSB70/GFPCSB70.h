/**
 * @brief GFPCSJ70.h 
 *
 * @date 2025/03/06 新規作成 by HAS
 *
*/

/* USER HEADER     */
#include "NWM_STE.h"                         /* 開局・閉局・エコー制御のNW個別処理(共通) */
#include "NWM_CTU.h"

#ifndef _GFPCSJ70_H_
#define _GFPCSJ70_H_

/* ------------------------------------------------------------------------------------------ */
/* Define 定義                                                                                */
/* ------------------------------------------------------------------------------------------ */
#define DEF_COM_SDT_arg1_gmt                        1     /* 標準時                           */
#define DEF_COM_SDT_arg1_jpn                        2     /* 日本時間                         */
#define DEF_COM_SDT_arg1_chn                        3     /* 中国時間                         */
#define DEF_COM_SDT_jpn_time              32400000000     /* 9時間（マイクロ秒)               */
#define DEF_COM_SDT_chn_time              28800000000     /* 8時間（マイクロ秒)               */

//#define DEF_NERR_STS_NOMAL                  "0000000"     /* 正常                                       */
//#define DEF_NERR_HSMK_STS_ERR_OPN           "SCDJ002"     /* 被仕向開局処理中の局状態チェックエラー */
//#define DEF_NERR_HSMK_STS_ERR_CLS           "SCDJ006"     /* 被仕向閉局処理中の局状態チェックエラー */
//#define DEF_NERR_HSMK_STS_ERR_ECH           "SCDJ022"     /* 被仕向エコーテスト局状態チェックエラー */

#define DEF_ACT_NORMAL                          "800"     /* 正常                             */
#define DEF_ACT_STS_ERR                         "910"     /* 局状態チェックエラー             */

#define DEF_RSP_KYOKA                             'A'     /* 許可応答                         */
//#define DEF_RSP_NO_OPTION                         '0'     /* オプション無し                   */
//#define DEF_RSP_FORCE_EXECUTION                   '1'     /* 強制実行                         */
//#define DEF_RSP_STAT_UPDATE                       '2'     /* 状態更新                         */


#define DEF_REQ_TYPE_SRSP                        "20"     /* 仕向応答                         */
#define DEF_REQ_TYPE_SRSP_TOUT                   "30"     /* 仕向応答Timeout                  */
#define DEF_REQ_TYPE_SREQ_SND_ERR                "40"     /* 仕向要求送信不可                 */

#define DEF_CHK_ERR_CTR_TYPE               "Ctrl-Type"
#define DEF_CHK_ERR_CTRLHD_TYPE       "Ctrl-Head-Type"
#define DEF_CHK_ERR_TOTAL_LEN           "Total-Legnth"
#define DEF_CHK_ERR_DST_CENT_ID        "Dst-Center-ID"
#define DEF_CHK_ERR_SRC_CENT_ID        "Src-Center-ID"
#define DEF_CHK_ERR_CTL_MERCH_CD      "Ctl-Merch-Code"
#define DEF_CHK_ERR_SMK_KUBN           "Shimuke-Kubun"
#define DEF_CHK_ERR_SEND_TIME              "Send-time"
#define DEF_CHK_ERR_MODE_FLG               "Mode-Flag"
#define DEF_CHK_ERR_HD_TYPE                "Head-Type"
#define DEF_CHK_ERR_MSG_TYPE            "Message_Type"
#define DEF_CHK_ERR_CUT_DATE                "Cut_Date"
#define DEF_CHK_ERR_BODY_LEN             "Body_Length"
#define DEF_CHK_ERR_CDNT_ID               "Cardnet_Id"
#define DEF_CHK_ERR_CDNT_SEQ             "Cardnet_Seq"
#define DEF_CHK_ERR_CDNT_AREA           "Cardnet_Area"
#define DEF_CHK_ERR_MTI                          "MTI"
#define DEF_CHK_ERR_BIT_02                     "BIT02"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_07                     "BIT07"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_11                     "BIT11"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_14                     "BIT14"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_33                     "BIT33"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_39                     "BIT39"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_63                     "BIT63"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_70                     "BIT70"     /* エラー発生BIT        */

#define DEF_CST_CHK_OK                               0     /* 局状態チェックOK     */
#define DEF_CST_CHK_NG                               1     /* 局状態チェックNG     */
#define DEF_BIT03_ALL_0                        "000000"    /* "000000"であること   */

#define DEF_RESP_CD_ALLOW_CHK                      "00"

                                        // 精査エラー設定
#define NWM_CTU_ERRORSET(cd)            memcpy(p_rslt_info->err_area,cd, strlen(cd))

/* ------------------------------------------------------------------------------------------ */
/* 関数のプロトタイプ宣言                                                                     */
/* ------------------------------------------------------------------------------------------ */
short  NWM_STE_char2hex  (char *, char *, short );
short  NWM_bcd_check     (char *, short );
void   NWM_bcd_to_char   (char *, char *, short );


short  CMIN_num_check (char *check_buf);                /* 数字文字チェック処理               */
short  CMIN_check_datetime(char*);                      /* 日付形式チェック                   */

// 文字変換ルーチン
//_cobol void EBCDIC2SJIS(char*, char*, short);
//_cobol void SJIS2EBCDIC(char*, char*, short);

#endif

