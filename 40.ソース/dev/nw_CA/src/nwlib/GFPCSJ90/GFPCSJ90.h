/**
 * @brief カットオーバー電文精査・電文編集・・ヘッダファイル
 *
 * @date 2025/03/18 新規作成 by tatsuya.sugisaki
*/
#ifndef _GFPCSJ90_H
#define _GFPCSJ90_H

// zsysc内ZSYS_VAL_LEN_PROCESSNAME未定義時のみINCLUDE
#include "zsysc" nolist

/* USER HEADER     */
#include "common.h"
#include "ems.h"
#include "file.h"
#include "ipc.h"

#include "msg_CA.h"
#include "NWM_CTO.h"
#include "GFPCGX90.h"

/* アクションコード */

/* アクションコード */

//#define DEF_NWM_CTO_HDR_TYPE_CTR        "F1"            /* 共通制御・ヘッダータイプ           */
//#define DEF_NWM_CTO_HDR_TYPE_BIZ        "A1"            /* 業務共通・ヘッダータイプ           */
//#define DEF_NWM_CTO_MSG_RCV_TYPE        "C804"          /* 業務共通・電文種別コード           */
//#define DEF_NWM_CTO_MSG_RSP_TYPE        "C814"          /* 業務共通・電文種別コード           */
//#define DEF_NWM_CTO_FNC_CD              "821"           /* ファンクションコード               */
#define DEF_NWM_CTO_ACT_INSP_NG           "910"
#define DEF_NWM_CTO_ACT_INSP_OK           "800"
#define DEF_NWM_CTO_CEN_STS_NG            "ECCK201"
#define DEF_NWM_CTO_MSG_FMT_NG            "ECCK200"

#define DEF_NWM_CTO_MTI_LEN               4
#define DEF_NWM_CTO_BIT_LEN               8
#define DEF_NWM_CTO_ERR_BIT_LEN           4

/* ヘッダーエラー */
#define DEF_CHK_ERR_CTR_TYPE                   "Ctrl-Type"
#define DEF_CHK_ERR_CTRLHD_TYPE           "Ctrl-Head-Type"
#define DEF_CHK_ERR_TOTAL_LEN               "Total-Legnth"
#define DEF_CHK_ERR_DST_CENT_ID            "Dst-Center-ID"
#define DEF_CHK_ERR_SRC_CENT_ID            "Src-Center-ID"
#define DEF_CHK_ERR_CTL_MERCH_CD          "Ctl-Merch-Code"
#define DEF_CHK_ERR_SMK_KUBN               "Shimuke-Kubun"
#define DEF_CHK_ERR_SEND_TIME                  "Send-time"
#define DEF_CHK_ERR_MODE_FLG                   "Mode-Flag"
#define DEF_CHK_ERR_HD_TYPE                    "Head-Type"
#define DEF_CHK_ERR_MSG_TYPE                "Message_Type"
#define DEF_CHK_ERR_CUT_DATE                    "Cut_Date"
#define DEF_CHK_ERR_BODY_LEN                 "Body_Length"
#define DEF_CHK_ERR_CDNT_ID                   "Cardnet_Id"
#define DEF_CHK_ERR_CDNT_SEQ                 "Cardnet_Seq"
#define DEF_CHK_ERR_CDNT_AREA               "Cardnet_Area"
#define DEF_CHK_ERR_MTI                              "MTI"

/* エラーBIT */
#define DEF_CHK_ERR_BIT_11                "BIT11"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_12                "BIT12"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_24                "BIT24"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_28                "BIT28"     /* エラー発生BIT        */

#define NWM_CTO_MCR_CPY_1814(dst,src) \
{\
    if(src.ffd_header.m_flg_exist != 0){\
        memcpy(&dst, &src, sizeof(dst));\
    }\
}

/* 接続先固有情報・CARDNET */
typedef struct __NWM_CTO_cnn_unq_inf_def
{
    char    dst_center_id[11];      // 接続先センタID
    char    src_center_id[11];      // 自センタID
    char    future_use[178];        // 予備
} NWM_CTO_cnn_unq_inf_def;

/*----------------------------------------------------------------------------*/
/* 非公開モジュール                                                           */
/*----------------------------------------------------------------------------*/
static short NWM_CTO_numchk(
    char   *col,
    size_t len
);

static short NWM_CTO_datechk(
    char      *col,
    size_t    ilen
);
static short NWM_CTO_BCD2CHAR(
    unsigned char *bcd_p,
    char *ascii_p,
    short s_len
);
static void NWM_CTO_CHAR2BCD(
    unsigned char *bcd_p,
    char *ascii_p,
    short s_len
);

#endif
