/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSU90                                    */
/*        FUNCTION          ････ NW個別(カットオーバー個別処理[UnionPay])    */
/*                               カットオーバー個別処理[UnionPay]を行う。    */
/*                                                                           */
/*        AUTHER            ････ HAS                                         */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-09-18                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS        2025/09/18 新規作成                                      */
/*                                                                           */
/*****************************************************************************/
#include <stdio.h>    nolist
#include <string.h>   nolist

/* USER HEADER     */
#include "common.h"
#include "file.h"
#include "ipc.h"

#include "msg_UP.h"
#include "NWM_CTO.h"
#include "GFPCSU90.h"                /* NW個別(カットオーバー個別処理)ヘッダ */
#include "GFPCVXZ0.h"                /* 制御電文共通メイン処理ヘッダ */
#include "vproc.h"

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CCUT_EMS_out                                    */
/*  CALLING SEQ.    : void NWM_CTO_msg_check(char,db_gfnwi_def *             */
/*  ARGUMENT        : 1.psh_msg       (I)   NW情報レコード(グループ単位)     */
/*                  : 2.psh_nwinf     (I)   NW情報レコード(IF単位)           */
/*                  : 3.psh_nwinf     (I)   接続先固有情報レコード(NW単位)   */
/*                  : 4.psh_nwinf     (I)   接続先固有情報レコード(IF単位)   */
/*                  : 5.psh_nwinf     (I)   接続先固有情報レコード(STA単位)  */
/*                  : 6.psh_nwinf     (I)   接続先固有情報レコード(CNN単位)  */
/*                  : 7.pch_cut_date  (O)   カット日付                       */
/*                  : 8.pch_err_bit   (O)   エラー発生ビット                 */
/*  RETURN CODE     : 0：精査OK                                              */
/*                    1：精査エラー（拒否応答）                              */
/*                    8：精査エラー（障害電文通知）                          */
/*                    7：精査エラー（破棄）                                  */
/*  DESCRIPTION     : カットオーバー電文精査                                 */
/*****************************************************************************/
short
NWM_CTO_msg_check(
    char         *pch_msg,          /* 受信電文 */
    char         *pch_nwinf_grp,    /* NW情報レコード(グループ単位) */
    char         *pch_nwinf_if,     /* NW情報レコード(IF単位) */
    char         *pch_cninf_nw,     /* 接続先固有情報レコード(NW単位) */
    char         *pch_cninf_if,     /* 接続先固有情報レコード(インタフェース単位) */
    char         *pch_cninf_st,     /* 接続先固有情報レコード(ステーション単位) */
    char         *pch_cninf_cn,     /* 接続先固有情報レコード(コネクション単位) */
    char         *pch_cut_date,     /* カット日付 */
    char         *pch_err_bit       /* エラー発生ビット */
)
{
    char buf[32];
    short ls_result;

    /* C401 制御電文処理要求 */
    cr401_def                   *pst_cr401_inf = (cr401_def *)pch_msg;
    /* システム間インターフェース・CARDNET・電文ヘッダ */
    msg_unionpay_def            *pst_biz_msg;
    fixedform_unionpay_0820_def *pst_fxd_0820_col;
    db_gfnwi_def                *lp_nw_info_if;   /* NW情報ファイル(IF単位)   */
    nwi_unq_info_up_def         *lp_nwi_unq_info; /* NW情報固有情報           */

    /* -----------------------------------------------------------------------*/
    /* 初期処理                                                               */
    /* -----------------------------------------------------------------------*/
    /* C401 制御電文処理要求 */
    pst_biz_msg =   (msg_unionpay_def *)pst_cr401_inf->data_bu.message_text;
    pst_fxd_0820_col = (fixedform_unionpay_0820_def *)&pst_biz_msg->ffd;

    lp_nw_info_if    = (db_gfnwi_def*)pch_nwinf_if;
    lp_nwi_unq_info  = (nwi_unq_info_up_def*)&lp_nw_info_if->dst_unq_info;

    /* -----------------------------------------------------------------------*/
    /* 電文精査・データ部                                                     */
    /* -----------------------------------------------------------------------*/
    /* ヘッダー部電文長                      */
    if (pst_biz_msg->header.mh_hdr_len != MSG_HEADER_UNIONPAY_def_Size){
        /* エラー発生ヘッダ番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_HDR_LEN,
               strlen(DEF_CHK_ERR_HDR_LEN));
        /* 電文破棄 */
        return DEF_NWM_CTO_ERR_HAKI;
    }

    /* Total Message Length精査 */
    if (memcmp(pst_cr401_inf->control_info.denbun_len,
               pst_biz_msg->header.mh_tot_len,
               sizeof(pst_biz_msg->header.mh_tot_len)) != 0) {
        /* エラー発生ヘッダ番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_TOT_LEN,
               strlen(DEF_CHK_ERR_TOT_LEN));
        /* 電文破棄 */
        return DEF_NWM_CTO_ERR_HAKI;
    }

    /* モードフラグ → 設定値check */
    if ( lp_nwi_unq_info->mode_flg == DEF_MODE_FLG_OFF  ) {
        if( pst_biz_msg->header.mh_hdr_flg_ver != DEF_UN_MODE_01_HONBAN ){
            memcpy( pch_err_bit,
                    DEF_CHK_ERR_MODE_FLG,
                    strlen(DEF_CHK_ERR_MODE_FLG));
            return DEF_NWM_CTO_ERR_HAKI;
        }
    } else
    if ( lp_nwi_unq_info->mode_flg == DEF_MODE_FLG_ON  ) {
        if( pst_biz_msg->header.mh_hdr_flg_ver != DEF_UN_MODE_81_TEST ){
            memcpy( pch_err_bit,
                    DEF_CHK_ERR_MODE_FLG,
                    strlen(DEF_CHK_ERR_MODE_FLG));
            return DEF_NWM_CTO_ERR_HAKI;
        }
    }

    /* MTI */
    if(memcmp(pst_biz_msg->mti,
              DEF_UP_MTI_0820_REQ,
              strlen(DEF_UP_MTI_0820_REQ)) != 0){
        memcpy(pch_err_bit,
               DEF_CHK_ERR_MTI,
               strlen(DEF_CHK_ERR_MTI));
        return DEF_NWM_CTO_ERR_HAKI;
    }

    /* BITMAP部精査 */
    if(pst_fxd_0820_col->b007_trans_date_time.ffd_header.m_flg_exist != true){
        /* Transmission Date and Time */
        /* エラー発生BIT番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_BIT_007,
               strlen(DEF_CHK_ERR_BIT_007));
        return DEF_NWM_CTO_ERR_HAKI;
    }

    if(pst_fxd_0820_col->b011_system_audit_number.ffd_header.m_flg_exist != true){
        /* Systems Trace Audit Number */
        /* エラー発生BIT番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_BIT_011,
               strlen(DEF_CHK_ERR_BIT_011));
        return DEF_NWM_CTO_ERR_HAKI;
    }

    if(pst_fxd_0820_col->b033_fowd_inst_id_code.ffd_header.m_flg_exist != false){
        /* Security Related Control Information */
        /* エラー発生BIT番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_BIT_033,
               strlen(DEF_CHK_ERR_BIT_033));
        return DEF_NWM_CTO_ERR_HAKI;
    }

    if(pst_fxd_0820_col->b053_secur_ctl_info.ffd_header.m_flg_exist != false){
        /* Network Management Information Code */
        /* エラー発生BIT番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_BIT_053,
               strlen(DEF_CHK_ERR_BIT_053));
        return DEF_NWM_CTO_ERR_HAKI;
    }

    if(pst_fxd_0820_col->b070_nw_mng_code.ffd_header.m_flg_exist != true){
        /* Network Management Information Code */
        /* エラー発生BIT番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_BIT_070,
               strlen(DEF_CHK_ERR_BIT_070));
        return DEF_NWM_CTO_ERR_HAKI;
    }

    if(pst_fxd_0820_col->b100_recv_inst_id_code.ffd_header.m_flg_exist != true){
        /* Receiving Institution Identification Code */
        /* エラー発生BIT番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_BIT_100,
               strlen(DEF_CHK_ERR_BIT_100));
        return DEF_NWM_CTO_ERR_HAKI;
    }

    /* --- BIT007 ----------------- */
    memset(buf , DEF_NWM_CTO_NULL, sizeof(buf));
    memset(buf , DEF_NWM_CTO_ZERO, 4);
    memcpy(&buf[4],
           pst_fxd_0820_col->b007_trans_date_time.ffd_data,
           DEF_NWM_CTO_BIT_007_LENG);
    ls_result = CMIN_check_datetime ( buf );
    if ( ls_result != DEF_NWM_CTO_OK ) {
        /* エラー発生BIT番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_BIT_007,
               strlen(DEF_CHK_ERR_BIT_007));
        /* 電文破棄 */
        return DEF_NWM_CTO_ERR_HAKI;
    }

    /* --- BIT011 ----------------- */
    memset( buf, DEF_NWM_CTO_NULL, sizeof(buf));
    memcpy( buf,
            pst_fxd_0820_col->b011_system_audit_number.ffd_data,
            pst_fxd_0820_col->b011_system_audit_number.ffd_header.m_fixvalue_length);
    ls_result = CMIN_num_check ( buf );
    if ( ls_result != DEF_NWM_CTO_OK ) {
        /* エラー発生BIT番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_BIT_011,
               strlen(DEF_CHK_ERR_BIT_011));
        /* 電文破棄 */
        return DEF_NWM_CTO_ERR_HAKI;
    }

    /* --- BIT070 ----------------- */
    /* 精査不要 */

    /* --- BIT100 ----------------- */
    /* 精査不要 */

    /* 精査OK */
    return DEF_NWM_CTO_OK;

}
/*****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_CTO_msg_edit                                */
/*  CALLING SEQ.    : void NWM_CTO_msg_edit(char *,db_gfnwi_def *,char *,    */
/*                                          char *,char *,char *)            */
/*  ARGUMENT        : 1.psh_reqmsg      (I)   被仕向要求電文                 */
/*                  : 2.pch_nwinf_grp   (I)   NW情報レコード(グループ単位)   */
/*                  : 3.pch_nwinf_if    (I)   NW情報レコード(IF単位)         */
/*                  : 4.pch_cninf_nw    (I)   接続先固有情報(NW単位)         */
/*                  : 5.pch_cninf_if    (I)   接続先固有情報(IF単位)         */
/*                  : 6.pch_cninf_st    (I)   接続先固有情報(Station単位)    */
/*                  : 7.pch_cninf_cn    (I)   接続先固有情報(Connection単位) */
/*                  : 8.pch_inn_errcd   (I)   内部エラーコード               */
/*                  : 9.pch_rspmsg      (O)   応答メッセージ                 */
/*                  :10.psh_len         (O)   応答メッセージ長               */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : カットオーバー応答電文編集処理                         */
/*****************************************************************************/
void
NWM_CTO_msg_edit(
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
)
{
    /* C401 制御電文処理要求 */
    cr401_def                   *pst_req_cr401_inf = (cr401_def *)psh_reqmsg;
    /* システム間インターフェース・UnionPay・電文ヘッダ */
    msg_unionpay_def            *pst_req_biz_msg;
    fixedform_unionpay_0820_def *pst_req_fxd_0820_col;

    /* C401 制御電文処理要求 */
    cr401_def                   *pst_rsp_cr401_inf = (cr401_def *)pch_rspmsg;
    /* システム間インターフェース・UnionPay・電文ヘッダ */
    msg_unionpay_def            *pst_rsp_biz_msg;
    fixedform_unionpay_0830_def *pst_rsp_fxd_0830_col;

    // 電文長演算
    *psh_len = (short)(
                 sizeof(pst_rsp_cr401_inf->common_header)
               + sizeof(pst_rsp_cr401_inf->control_info)
               + sizeof(pst_rsp_biz_msg->header)
               + sizeof(pst_rsp_biz_msg->mti)
               + sizeof(fixedform_unionpay_0830_def) );

    /* -----------------------------------------------------------------------*/
    /* 初期処理                                                               */
    /* -----------------------------------------------------------------------*/
    /* 要求 */
    /* C401 制御電文処理要求 */
    pst_req_biz_msg      = (msg_unionpay_def *)&pst_req_cr401_inf->data_bu;
    pst_req_fxd_0820_col = (fixedform_unionpay_0820_def *)&pst_req_biz_msg->ffd;

    /* 応答 */
    pst_rsp_biz_msg      = (msg_unionpay_def *)&pst_rsp_cr401_inf->data_bu;
    pst_rsp_fxd_0830_col = (fixedform_unionpay_0830_def *)&pst_rsp_biz_msg->ffd;

    // 応答送信領域初期化
    memset(pch_rspmsg, 0x00, *psh_len);

    /* -----------------------------------------------------------------------*/
    /* 電文編集・CR401 NW電文受信                                             */
    /* -----------------------------------------------------------------------*/
    // 要求受信分の共通ヘッダ・制御電文情報を一旦コピー
    memcpy(pst_rsp_cr401_inf, pst_req_cr401_inf,
        sizeof(pst_req_cr401_inf->common_header) + sizeof(pst_req_cr401_inf->control_info));

    /* インターフェースコード */
    memcpy(pst_rsp_cr401_inf->common_header.interface_code,
           DEF_IPC_IFCD_NW_MSG_RSP,
           strlen(DEF_IPC_IFCD_NW_MSG_RSP));

    /* エラーコード*/
    if(memcmp(pch_inn_errcd,
              DEF_NWM_CTO_INTERR_OK,
              strlen(DEF_NWM_CTO_INTERR_OK)) == 0){
        pst_rsp_cr401_inf->common_header.error_code = DEF_IPC_ERRCD_OK;
    }
    else if(memcmp(pch_inn_errcd,
              DEF_NWM_CTO_MSG_FMT_NG,
              strlen(DEF_NWM_CTO_MSG_FMT_NG)) == 0){
        // 電文精査異常
        /* 電文破棄のため正常応答とする */
        pst_rsp_cr401_inf->common_header.error_code = DEF_IPC_ERRCD_OK;
    }
    else {
        // 電文精査異常
        pst_rsp_cr401_inf->common_header.error_code = DEF_IPC_ERRCD_NG;
    }

    /* 内部エラーコード*/
    memcpy(pst_rsp_cr401_inf->common_header.internal_error_code,
           pch_inn_errcd,
           sizeof(pst_rsp_cr401_inf->common_header.internal_error_code));

    /* 応答種別                     */
    if(memcmp(pst_req_cr401_inf->control_info.request_kind,
              DEF_CTLREQ_HISIMUKE_ERROR,
              strlen(DEF_CTLREQ_HISIMUKE_ERROR))!=0){
        // 通常
        if (memcmp(pch_inn_errcd,
                   DEF_NWM_CTO_MSG_FMT_NG,
                   strlen(DEF_NWM_CTO_MSG_FMT_NG)) == 0){
           // 電文破棄の場合、電文なし
           memcpy(pst_rsp_cr401_inf->control_info.response_kind,
                  DEF_CTLRSP_NOSEND,
                  strlen(DEF_CTLRSP_SEND));
        }
        else {
           // 電文あり
           memcpy(pst_rsp_cr401_inf->control_info.response_kind,
                  DEF_CTLRSP_SEND,
                  strlen(DEF_CTLRSP_SEND));
        }
    }else{
        // 被仕向応答送信不可
        // 電文なし
        memcpy(pst_rsp_cr401_inf->control_info.response_kind,
               DEF_CTLRSP_NOSEND,
               strlen(DEF_CTLRSP_NOSEND));
    }

    /* データ長                     */
    if (memcmp(pst_rsp_cr401_inf->control_info.response_kind,
               DEF_CTLRSP_NOSEND,
               strlen(DEF_CTLRSP_NOSEND)) == 0){
        /* 電文なし */
        pst_rsp_cr401_inf->common_header.control_data_length =  sizeof(pst_rsp_cr401_inf->control_info);
        *psh_len = (short)(
                     sizeof(pst_rsp_cr401_inf->common_header)
                   + sizeof(pst_rsp_cr401_inf->control_info) );
    }
    else{
        /* 電文あり */
        pst_rsp_cr401_inf->common_header.control_data_length =  sizeof(pst_rsp_cr401_inf->control_info)
                                                              + sizeof(pst_rsp_biz_msg->header)
                                                              + sizeof(pst_rsp_biz_msg->mti)
                                                              + sizeof(fixedform_unionpay_0830_def);
    }

    /* 制御電文種別                 */
    pst_rsp_cr401_inf->control_info.control_kind.req_res_kbn  = DEF_CTLMSG_RESPONSE;
    pst_rsp_cr401_inf->control_info.control_kind.int_proc_kbn = DEF_CTLINT_ALLOW;

    /* 電文種別                 */
    pst_rsp_cr401_inf->control_info.denbun_log_key.denbun_shubetu =
                pst_rsp_cr401_inf->control_info.control_kind.ctl_text_kbn;

    /* MTI                      */
    memcpy(pst_rsp_cr401_inf->control_info.mti,
           DEF_UP_MTI_0830_RSP,
           strlen(DEF_UP_MTI_0830_RSP));

    if (memcmp(pst_rsp_cr401_inf->control_info.response_kind,
               DEF_CTLRSP_SEND,
               strlen(DEF_CTLRSP_SEND)) == 0){
        /* -----------------------------------------------------------------------*/
        /* 電文編集・データ部                                                     */
        /* -----------------------------------------------------------------------*/
        /* MTI */
        memcpy(pst_rsp_biz_msg->mti, DEF_UP_MTI_0830_RSP, strlen(DEF_UP_MTI_0830_RSP));

        /* Message Header */
        memcpy((char *)&pst_rsp_biz_msg->header, (char *)&pst_req_biz_msg->header, sizeof(pst_req_biz_msg->header));
        // Destination ID
        memcpy(pst_rsp_biz_msg->header.mh_dst_id, pst_req_biz_msg->header.mh_src_id, sizeof(pst_req_biz_msg->header.mh_src_id));
        // Source ID
        memcpy(pst_rsp_biz_msg->header.mh_src_id, pst_req_biz_msg->header.mh_dst_id, sizeof(pst_req_biz_msg->header.mh_dst_id));

        // 以降は基本的にフラグが有効な場合に要求データコピーで対応する。
        // F7.Transmission Date and Time
        NWM_CTO_MCR_CPY_0830(pst_rsp_fxd_0830_col->b007_trans_date_time,
                             pst_req_fxd_0820_col->b007_trans_date_time);

        // F11.System Trace Audit Number
        NWM_CTO_MCR_CPY_0830(pst_rsp_fxd_0830_col->b011_system_audit_number,
                             pst_req_fxd_0820_col->b011_system_audit_number);

        // F39.アクションコード
        pst_rsp_fxd_0830_col->b039_response_code.ffd_header.m_flg_exist = 1;
        pst_rsp_fxd_0830_col->b039_response_code.ffd_header.m_fixvalue_length =
                      sizeof(pst_rsp_fxd_0830_col->b039_response_code.ffd_data);
        /* 正常応答のみ */
        memcpy(pst_rsp_fxd_0830_col->b039_response_code.ffd_data,
               DEF_NWM_CTO_ACT_INSP_OK,
               strlen(DEF_NWM_CTO_ACT_INSP_OK));

        // F70.Network Management Information Code
        NWM_CTO_MCR_CPY_0830(pst_rsp_fxd_0830_col->b070_nw_mng_code,
                             pst_req_fxd_0820_col->b070_nw_mng_code);

        // F100.Receiving Institution Identification Code
        NWM_CTO_MCR_CPY_0830(pst_rsp_fxd_0830_col->b100_recv_inst_id_code,
                             pst_req_fxd_0820_col->b100_recv_inst_id_code);
    }
    return;
}
/*****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_CTO_cst_check_req_rcv                       */
/*  CALLING SEQ.    : void NWM_CTO_cst_check_req_rcv(char,char *)            */
/*  ARGUMENT        : 1.pch_sta_sts     (I)   局状態                         */
/*                  : 2.pch_inner_errcd (I)   内部エラーコード               */
/*  RETURN CODE     :  0：局状態正常                                         */
/*                    -1：局状態異常                                         */
/*  DESCRIPTION     : 局状態精査                                             */
/*****************************************************************************/
short
NWM_CTO_cst_check_req_rcv(
    char         *pch_sta_sts,      /* 局状態 */
    char         *pch_inner_errcd   /* 内部エラーコード */
)
{
    /* すべての局状態が受付可能 */
    return 0;
}

