/**
 * @brief カットオーバー電文精査・電文編集・ヘッダファイル
 *
 * @date 2025/03/18 新規作成 by tatsuya.sugisaki
*/
#ifndef _NWM_CTO_H
#define _NWM_CTO_H

#define DEF_NWM_CTO_INTERR_OK           "0000000"
#define DEF_NWM_CTO_CEN_STS_NG          "ECCK201"
#define DEF_NWM_CTO_MSG_FMT_NG          "ECCK200"
#define DEF_NWM_CTO_CTLMSG_REQ          "1804"
#define DEF_NWM_CTO_CTLMSG_RES          "1814"
#define DEF_NWM_CTO_RSP_TYPE            "326A"
#define DEF_NWM_CTO_RSP_ERR_TYPE        "326B"
#define DEF_NWM_CTO_RSP_BASE            "3261"

/* エラーコード */
#define DEF_NWM_CTO_ERR_HAKI            7            /* 精査エラー(破棄)      */

/*----------------------------------------------------------------------------*/
/* 公開モジュール                                                             */
/*----------------------------------------------------------------------------*/
short NWM_CTO_msg_check(
    char         *pch_msg,          /* 受信電文 */
    char         *pst_nwinf_grp,    /* NW情報レコード(グループ単位) */
    char         *pst_nwinf_intr,   /* NW情報レコード(IF単位) */
    char         *pch_cninf_nw,     /* 接続先固有情報レコード(NW単位) */
    char         *pch_cninf_if,     /* 接続先固有情報レコード(インタフェース単位) */
    char         *pch_cninf_st,     /* 接続先固有情報レコード(ステーション単位) */
    char         *pch_cninf_cn,     /* 接続先固有情報レコード(コネクション単位) */
    char         *pch_cut_date,     /* カット対象日付 */
    char         *pch_err_bit       /* エラー発生ビット */
);
void NWM_CTO_msg_edit(
    char         *psh_reqmsg,       /* 被仕向要求電文 */
    char         *pch_nwinf_grp,    /* NW情報レコード(グループ単位) */
    char         *pch_nwinf_if,     /* NW情報レコード(IF単位) */
    char         *pch_cninf_nw,     /* 接続先固有情報レコード(NW単位) */
    char         *pch_cninf_if,     /* 接続先固有情報レコード(インタフェース単位) */
    char         *pch_cninf_st,     /* 接続先固有情報レコード(ステーション単位) */
    char         *pch_cninf_cn,     /* 接続先固有情報レコード(コネクション単位) */
    char         *pch_inn_errcd,    /* 内部エラーコード */
    char         *pch_rspmsg,       /* 応答メッセージ */
    short        *psh_len           /* 応答メッセージ長 */
);
short NWM_CTO_cst_check_req_rcv(
    char         *pch_sta_sts,      /* 局状態 */
    char         *pch_inner_errcd   /* 内部エラーコード */
);

#endif
