/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                               *
 *                                                                             *
 *                               ＜コネクション制御(クライアント)＞            *
 *                                                                             *
 *        VERSION                               :＜1.0.0＞                     *
 *                                                                             *
 *        CREATE DATE                           :＜作成日 2025/02/28＞         *
 *        CODED                                 :＜ISYS＞                      *
 *                                                                             *
 *        MODIFY DATE                           :＜修正日 yyyy／mm／dd＞       *
 *        CODED                                 :＜修正者＞                    *
 *                                              :＜修正概要＞                  *
 ******************************************************************************/
/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP制御                                     */
/*        PROGRAM-ID        ････ GFPCVX20                                    */
/*        FUNCTION          ････ コネクション制御(クライアント)              */
/*                                                                           */
/*        AUTHOR            ････ ISYS Kudo                                   */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025/02/28                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤   2025/02/28 新規作成                                     */
/*  1.1  ISYS 工藤   2025/04/28 GFNWI 2.01版対応                             */
/*       ISYS 工藤   2025/07/15 ファイル設計書の更新対応漏れをチェック       */
/****************************************************************************/
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
/* USER HEADER     */
#include <errcd.h> nolist
#include <common.h> nolist

#include "GFPCVX20_ems_trace.h" nolist
#include "GFPCVX20_util.h" nolist

/*
 * 主なチェック例:
 * nw_kubun: "CA"：CARDNET、"VI"：VisaNet、"BK"：Banknet、"AX"：AEGEN、"NY"：NYCE、"UP"：UnionPay、"JL"：J-LINK、"DI"：Discoverのいずれか
 * open_close_mng_lyr など: "C","I","S"," " のいずれか
 * data_len_start_lct : 1以上の数字
 * data_len_attribute : "BIN","BCD","ASC","EBC"
 * data_len_include_id: 'I' or 'O'
 * connect_nxt_prc_kind: "SO","DT"," " (スペース)
 * spc_data: 空白 or HEXコード(左詰スペース埋め)
 * (廃止)act_stb_id: 'A' or 'S'
 * 各種タイマー/カウンタ: 数字
 */
/*****************************************************************************/
/*  FUNCTION        :check_db_gfnwi_def                                     */
/*  CALLING SEQ.    :int check_db_gfnwi_def(const db_gfnwi_def *p)          */
/*  ARGUMENT        :p:db_gfnwi_def構造体へのポインタ(フィールドを検証)     */
/*  RETURN CODE     :0(正常終了),-1(エラーが見つかった場合)                 */
/*  DESCRIPTION     :db_gfnwi_def構造体に含まれる複数の項目を検証し         */
/*                  :不正がある場合はログ出力後-1を返す                     */
/****************************************************************************/
int cncl_chk_gfnwi_rec(const db_gfnwi_def *p)
{
    int retval = 0;
    /* nw_kubun */
    {
        static const char *kubunList[] = {DEF_NW_KUBUN_CARDNET /* "JC"->“CA” */, DEF_NW_KUBUN_VISANET /* "VI" */,
                                          DEF_NW_KUBUN_BANKNET /* "BA"->"BK" */, DEF_NW_KUBUN_AEGN /* "AE"->"AX" */,
                                          DEF_NW_KUBUN_NYCE /* "NY" */,          DEF_NW_KUBUN_UNIONPAY /* "UN"->"UP" */,
                                          DEF_NW_KUBUN_JLINK /* "JL" */,         DEF_NW_KUBUN_DISCOVER /* "DI" */};
        if (!checkStringInList(p->nw_id_info.nw_kubun, 2, kubunList, 8)) {
            cncl_ems_param_err("nw_kubun", 0, DEF_NERR_PRM_RD_ERR_INV);
            retval = -1;
        }
    }

    /* mng_lyr_info各項目: "C","I","S"," " のいずれか */
    {
        const char *validChars = "CIS ";
        if (!checkCharInSet(p->mng_lyr_info.open_close_mng_lyr, validChars)) {
            cncl_ems_param_err("open_close_mng_lyr", 0, DEF_NERR_PRM_RD_ERR_INV);
            retval = -1;
        }
        if (!checkCharInSet(p->mng_lyr_info.echo_test_mng_lyr, validChars)) {
            cncl_ems_param_err("echo_test_mng_lyr", 0, DEF_NERR_PRM_RD_ERR_INV);
            retval = -1;
        }
        if (!checkCharInSet(p->mng_lyr_info.cut_over_mng_lyr, validChars)) {
            cncl_ems_param_err("cut_over_mng_lyr", 0, DEF_NERR_PRM_RD_ERR_INV);
            retval = -1;
        }
        validChars = "IS ";
        if (!checkCharInSet(p->mng_lyr_info.key_cng_mng_lyr, validChars)) {
            cncl_ems_param_err("key_cng_mng_lyr", 0, DEF_NERR_PRM_RD_ERR_INV);
            retval = -1;
        }
        if (!checkCharInSet(p->mng_lyr_info.saf_send_mng_lyr, validChars)) {
            cncl_ems_param_err("saf_send_mng_lyr", 0, DEF_NERR_PRM_RD_ERR_INV);
            retval = -1;
        }
        if (!checkCharInSet(p->mng_lyr_info.connect_num_mng_lyr, validChars)) {
            cncl_ems_param_err("connect_num_mng_lyr", 0, DEF_NERR_PRM_RD_ERR_INV);
            retval = -1;
        }
    }


    /* denbun_item_lct_info */
    {
        /* data_len_start_lct, denbun_start_lct, mti_start_lct => 1以上の数字 */
        if (!checkNumericGreaterEqual1(p->denbun_item_lct_info.data_len_start_lct, 5)) {
            cncl_ems_param_err("data_len_start_lct", 0, DEF_NERR_PRM_RD_ERR_INV);
            retval = -1;
        }
        if (!checkNumericGreaterEqual1(p->denbun_item_lct_info.denbun_start_lct, 5)) {
            cncl_ems_param_err("denbun_start_lct", 0, DEF_NERR_PRM_RD_ERR_INV);
            retval = -1;
        }
        if (!checkNumericGreaterEqual1(p->denbun_item_lct_info.mti_start_lct, 5)) {
            cncl_ems_param_err("mti_start_lct", 0, DEF_NERR_PRM_RD_ERR_INV);
            retval = -1;
        }

        /* data_len_size => 1以上の数字 */
        if (!checkNumericGreaterEqual1(p->denbun_item_lct_info.data_len_size, 2)) {
            cncl_ems_param_err("data_len_size", 0, DEF_NERR_PRM_RD_ERR_INV);
            retval = -1;
        }

        /* data_len_attribute, mti_item_attribute => BIN, BCD, ASC, EBC */
        {
            static const char *attrList[] = {DEF_DATA_LEN_ATTR_BIN3, DEF_DATA_LEN_ATTR_BCD3, DEF_DATA_LEN_ATTR_ASC3, DEF_DATA_LEN_ATTR_EBC3};
            if (!checkStringInList(p->denbun_item_lct_info.data_len_attribute, 3, attrList, 4)) {
                cncl_ems_param_err("data_len_attribute", 0, DEF_NERR_PRM_RD_ERR_INV);
                retval = -1;
            }
            if (!checkStringInList(p->denbun_item_lct_info.mti_item_attribute, 3, attrList, 4)) {
                cncl_ems_param_err("mti_item_attribute", 0, DEF_NERR_PRM_RD_ERR_INV);
                retval = -1;
            }
        }
        /* data_len_include_id => 'I' or 'O' */
        if (p->denbun_item_lct_info.data_len_include_id != DEF_DATA_LEN_INCLUDE_IN && p->denbun_item_lct_info.data_len_include_id != DEF_DATA_LEN_INCLUDE_OUT) {
            cncl_ems_param_err("data_len_include_id", 0, DEF_NERR_PRM_RD_ERR_INV);
            retval = -1;
        }

        /* mti_item_len => 1以上の数字 */
        if (!checkNumericGreaterEqual1(p->denbun_item_lct_info.mti_item_len, 2)) {
            cncl_ems_param_err("mti_item_len", 0, DEF_NERR_PRM_RD_ERR_INV);
            retval = -1;
        }
    }

    /* connect_nxt_prc_info */
    {
        /* connect_nxt_prc_kind => "SO","DT"," " */
        /* 2バイト配列を文字列化して比較 */
        {
            static const char *kindList[] = {DEF_CONNECT_NEXT_OPN_SEND, DEF_CONNECT_NEXT_DATA_SEND, DEF_CONNECT_NEXT_NO};
            if (!checkStringInList(p->connect_nxt_prc_info.connect_nxt_prc_kind, 2, kindList, 3)) {
                cncl_ems_param_err("connect_nxt_prc_kind", 0, DEF_NERR_PRM_RD_ERR_INV);
                retval = -1;
            }
        }
        /* spc_data => 空白 or HEX */
        if (!checkSpcOrHex(p->connect_nxt_prc_info.spc_data, 100)) {
            cncl_ems_param_err("spc_data", 0, DEF_NERR_PRM_RD_ERR_INV);
            retval = -1;
        }
    }

    //未使用フィールドのチェックを廃止
    // /* env_set_info */
    // {
    //     if (p->env_set_info.act_stb_id != 'A'
    //         && p->env_set_info.act_stb_id != 'S'
    //         && p->env_set_info.act_stb_id != ' ') {
    //             cncl_ems_param_err("act_stb_id", 0, DEF_NERR_PRM_RD_ERR_INV);
    //             retval = -1;
    //     }
    // }

    /* 各種タイマー/カウンタ => 数字 */
    /* 例: connect_wait_tmr, send_wait_tmr, ...  */
    {
/* 簡単に checkDecimalRange(str, 8, 0, 99999999, false) 等も考えられる */
/* ここでは全部 isAllDigits(p->trans_cntrl_tmr_info.fieldName, 8) としてみる */
#define CHECK_TIMER_FIELD(fieldName)                                                   \
    if (!checkDecimalRange(p->trans_cntrl_tmr_info.fieldName, 8, 0, 99999999, true)) { \
        cncl_ems_param_err(#fieldName, 0, DEF_NERR_PRM_RD_ERR_INV);                    \
        retval = -1;                                                                     \
    }

        CHECK_TIMER_FIELD(connect_wait_tmr);
        CHECK_TIMER_FIELD(send_wait_tmr);
        CHECK_TIMER_FIELD(nxt_data_recv_wait);
        CHECK_TIMER_FIELD(non_comm_monitor);
        CHECK_TIMER_FIELD(line_fail_rtr_num_srt);
        CHECK_TIMER_FIELD(line_fail_rtr_num_lng);
        //CHECK_TIMER_FIELD(line_fail_rtr_num_lst);
        //CHECK_TIMER_FIELD(tmr_08);    //未使用フィールドのチェックを廃止
        //CHECK_TIMER_FIELD(tmr_09);    //未使用フィールドのチェックを廃止
        //CHECK_TIMER_FIELD(tmr_10);    //未使用フィールドのチェックを廃止
#undef CHECK_TIMER_FIELD
    }

    /* trans_cntrl_cnt_info の line_fail_rtr_num_srt / lng / lst なども同様にチェック */
    {
#define CHECK_CNT_FIELD(fieldName)                                                     \
    if (!checkDecimalRange(p->trans_cntrl_cnt_info.fieldName, 8, 0, 99999999, true)) { \
        cncl_ems_param_err(#fieldName, 0, DEF_NERR_PRM_RD_ERR_INV);                    \
        retval = -1;                                                                     \
    }

        CHECK_CNT_FIELD(line_fail_rtr_num_srt);
        CHECK_CNT_FIELD(line_fail_rtr_num_lng);
        //CHECK_CNT_FIELD(line_fail_rtr_num_lst);   //未使用フィールドのチェックを廃止
        //CHECK_CNT_FIELD(cnt_04);                  //未使用フィールドのチェックを廃止
        //CHECK_CNT_FIELD(cnt_05);                  //未使用フィールドのチェックを廃止
#undef CHECK_CNT_FIELD
    }

    /* cntrl_denbun_tmr_info */
    {
#define CHECK_DENBUN_TMR_FIELD(fieldName)                                               \
    if (!checkDecimalRange(p->cntrl_denbun_tmr_info.fieldName, 8, 0, 99999999, true)) { \
        cncl_ems_param_err(#fieldName, 0, DEF_NERR_PRM_RD_ERR_INV);                     \
        retval = -1;                                                                      \
    }
        //CHECK_DENBUN_TMR_FIELD(open_res_wait_tmr);        //未使用フィールドのチェックを廃止
        //CHECK_DENBUN_TMR_FIELD(close_res_wait_tmr);       //未使用フィールドのチェックを廃止
        //CHECK_DENBUN_TMR_FIELD(echo_test_res_wait_tmr);   //未使用フィールドのチェックを廃止
        //CHECK_DENBUN_TMR_FIELD(key_cng_req_wait_tmr);     //未使用フィールドのチェックを廃止
        //CHECK_DENBUN_TMR_FIELD(key_cng_res_wait_tmr);     //未使用フィールドのチェックを廃止
        //CHECK_DENBUN_TMR_FIELD(cut_over_res_wait_tmr);    //未使用フィールドのチェックを廃止
        //CHECK_DENBUN_TMR_FIELD(saf_start_end_res_tmr);    //未使用フィールドのチェックを廃止
        //CHECK_DENBUN_TMR_FIELD(tmr_08);                   //未使用フィールドのチェックを廃止
        //CHECK_DENBUN_TMR_FIELD(tmr_09);                   //未使用フィールドのチェックを廃止
        //CHECK_DENBUN_TMR_FIELD(tmr_10);                   //未使用フィールドのチェックを廃止
#undef CHECK_DENBUN_TMR_FIELD
    }

    /* cntrl_denbun_cnt_info */
    {
#define CHECK_DENBUN_CNT_FIELD(fieldName)                                               \
    if (!checkDecimalRange(p->cntrl_denbun_cnt_info.fieldName, 8, 0, 99999999, true)) { \
        cncl_ems_param_err(#fieldName, 0, DEF_NERR_PRM_RD_ERR_INV);                     \
        retval = -1;                                                                      \
    }
        //CHECK_DENBUN_CNT_FIELD(open_retry_num); //未使用フィールドのチェックを廃止
        //CHECK_DENBUN_CNT_FIELD(cnt_02);         //未使用フィールドのチェックを廃止
        //CHECK_DENBUN_CNT_FIELD(cnt_03);         //未使用フィールドのチェックを廃止
        //CHECK_DENBUN_CNT_FIELD(cnt_04);         //未使用フィールドのチェックを廃止
        //CHECK_DENBUN_CNT_FIELD(cnt_05);         //未使用フィールドのチェックを廃止
#undef CHECK_DENBUN_CNT_FIELD
    }

    /* 他、dst_unq_infoの中身の細かい仕様チェックなどがあれば追加 */

    return retval; /* result */
}
