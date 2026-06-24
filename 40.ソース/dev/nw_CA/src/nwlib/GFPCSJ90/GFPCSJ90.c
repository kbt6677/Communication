/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSJ90                                    */
/*        FUNCTION          ････ NW個別(カットオーバー個別処理[CARDNET])     */
/*                                                                           */
/*                               CARDNETのカットーバー個別処理を行う。       */
/*                                                                           */
/*        AUTHER            ････ HAS                                         */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-03-31                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS        2025/03/31 新規作成                                      */
/*                                                                           */
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>   nolist
#include <stdlib.h>   nolist
#include <ctype.h>    nolist
#include <tal.h>      nolist
#include <cextdecs.h> nolist

#include "GFPCSJ90.h"                /* NW個別(カットオーバー個別処理)ヘッダ */
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
/*                    9：精査エラー（破棄）                                  */
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
    short iresult;
//  db_gfnwi_def *pst_nwinf_grp = (db_gfnwi_def *)pch_nwinf_grp;
    db_gfnwi_def *pst_nwinf_if  = (db_gfnwi_def *)pch_nwinf_if;

    /* C401 制御電文処理要求 */
    cr401_def                   *pst_cr401_inf = (cr401_def *)pch_msg;
    /* システム間インターフェース・CARDNET・電文ヘッダ */
    msg_cardnet_def             *pst_biz_msg;
    MSG_HEADER_CARDNET_def      *pst_cdnet_hdr;
    fixedform_cardnet_1804_def  *pst_fxd_1804_col;

    NWM_CTO_cnn_unq_inf_def *db_gfnws_nw_inf = NULL;
    NWM_CTO_cnn_unq_inf_def *db_gfnws_st_inf = NULL;

    db_gfnws_nw_inf = (NWM_CTO_cnn_unq_inf_def *)((db_gfnws_def *)pch_cninf_nw)->dst_unq_info;

    db_gfnws_st_inf = (NWM_CTO_cnn_unq_inf_def *)((db_gfnws_def *)pch_cninf_st)->dst_unq_info;

    /* -----------------------------------------------------------------------*/
    /* 初期処理                                                               */
    /* -----------------------------------------------------------------------*/
    /* C401 制御電文処理要求 */
    pst_biz_msg =   (void *)pst_cr401_inf->data_bu.message_text;
    pst_cdnet_hdr = &pst_biz_msg->header;
    pst_fxd_1804_col = (void *)&pst_biz_msg->ffd;

    /* -----------------------------------------------------------------------*/
    /* 電文精査・ヘッダ部                                                     */
    /* -----------------------------------------------------------------------*/
    /* 共通制御・ヘッダータイプ */
    if(memcmp(pst_cdnet_hdr->ctrl_hdr_type, DEF_CA_CMHD_TYPE_F1, strlen(DEF_CA_CMHD_TYPE_F1)) != 0){
        /* エラー発生ヘッダ番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_CTRLHD_TYPE,
               strlen(DEF_CHK_ERR_CTRLHD_TYPE));
         return DEF_IPC_ERRCD_FAILMSG;
    }
    /* 共通制御・全体電文長 */
    memset(buf, 0x00, sizeof(buf));
    iresult = NWM_CTO_BCD2CHAR(pst_cdnet_hdr->ctrl_msg_len, buf, sizeof(pst_cdnet_hdr->ctrl_msg_len));
    if(iresult != 0){
        /* エラー発生ヘッダ番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_TOTAL_LEN,
               strlen(DEF_CHK_ERR_TOTAL_LEN));
         return DEF_IPC_ERRCD_FAILMSG;
    }

    /* 共通制御・差出センターID */
    if(memcmp(pst_cdnet_hdr->ctrl_src_id, db_gfnws_nw_inf->dst_center_id, sizeof(pst_cdnet_hdr->ctrl_src_id)) !=0) {
        /* エラー発生ヘッダ番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_DST_CENT_ID,
               strlen(DEF_CHK_ERR_DST_CENT_ID));
         return DEF_IPC_ERRCD_FAILMSG;
    }

    /* 共通制御・宛先センターID */
    if(memcmp(pst_cdnet_hdr->ctrl_dst_id, db_gfnws_st_inf->src_center_id, sizeof(pst_cdnet_hdr->ctrl_dst_id)) !=0) {
        /* エラー発生ヘッダ番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_SRC_CENT_ID,
               strlen(DEF_CHK_ERR_SRC_CENT_ID));
         return DEF_IPC_ERRCD_FAILMSG;
    }

    /* 共通制御・送信日時 */
    memset(buf, 0x00, sizeof(buf));
    NWM_CTO_BCD2CHAR(pst_cdnet_hdr->ctrl_snd_time, buf, sizeof(pst_cdnet_hdr->ctrl_snd_time));
    if(NWM_CTO_datechk(buf, sizeof(pst_cdnet_hdr->ctrl_snd_time) * 2) != 0){
        /* エラー発生ヘッダ番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_SEND_TIME,
               strlen(DEF_CHK_ERR_SEND_TIME));
         return DEF_IPC_ERRCD_FAILMSG;
    }
    /* 共通制御・モードフラグ */
    memset(buf, 0x00, sizeof(buf));
    NWM_CTO_BCD2CHAR(&pst_cdnet_hdr->ctrl_mode_flg, buf, sizeof(pst_cdnet_hdr->ctrl_mode_flg));
    if(buf[0] != *pst_nwinf_if->dst_unq_info){
        /* エラー発生ヘッダ番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_MODE_FLG,
               strlen(DEF_CHK_ERR_MODE_FLG));
         return DEF_IPC_ERRCD_FAILMSG;
    }
    /* 業務共通・ヘッダータイプ */
    if(memcmp(pst_cdnet_hdr->bh_hdr_type, DEF_CA_APHD_TYPE_A1, strlen(DEF_CA_APHD_TYPE_A1)) != 0){
        /* エラー発生ヘッダ番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_HD_TYPE,
               strlen(DEF_CHK_ERR_HD_TYPE));
         return DEF_IPC_ERRCD_FAILMSG;
    }
    /* 業務共通・電文種別コード */
    if(memcmp(pst_cdnet_hdr->bh_msg_type, DEF_CA_APHD_MSGCODE_C804_REQ, strlen(DEF_CA_APHD_MSGCODE_C804_REQ)) != 0){
        /* エラー発生ヘッダ番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_MSG_TYPE,
               strlen(DEF_CHK_ERR_MSG_TYPE));
         return DEF_IPC_ERRCD_FAILMSG;
    }
    /* 業務共通・カット対象日付 */
    memset(buf, 0x00, sizeof(buf));
    NWM_CTO_BCD2CHAR(pst_cdnet_hdr->bh_cut_date, buf, sizeof(pst_cdnet_hdr->bh_cut_date));
    if(NWM_CTO_datechk(buf, sizeof(pst_cdnet_hdr->bh_cut_date) * 2) != 0){
        /* エラー発生ヘッダ番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_CUT_DATE,
               strlen(DEF_CHK_ERR_CUT_DATE));
         return DEF_IPC_ERRCD_FAILMSG;
    }
    // カット日付保存
    memcpy(pch_cut_date, buf, sizeof(pst_cdnet_hdr->bh_cut_date) * 2);

    /* 業務共通・BODY部電文長 */
    memset(buf, 0x00, sizeof(buf));
    iresult = NWM_CTO_BCD2CHAR(pst_cdnet_hdr->bh_body_len, buf, sizeof(pst_cdnet_hdr->bh_body_len));
    if(iresult != 0){
        /* エラー発生ヘッダ番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_BODY_LEN,
               strlen(DEF_CHK_ERR_BODY_LEN));
         return DEF_IPC_ERRCD_FAILMSG;
    }

    /* -----------------------------------------------------------------------*/
    /* 電文精査・データ部                                                     */
    /* -----------------------------------------------------------------------*/
    /* MTI */
    if(memcmp(pst_biz_msg->mti, DEF_NWM_CTO_CTLMSG_REQ, strlen(DEF_NWM_CTO_CTLMSG_REQ)) != 0){
        /* エラー発生ヘッダ番号設定 */
        memcpy(pch_err_bit,
               DEF_CHK_ERR_MTI,
               strlen(DEF_CHK_ERR_MTI));
         return DEF_IPC_ERRCD_FAILMSG;
    }

    if(pst_fxd_1804_col->b11_system_audit_number.ffd_header.m_flg_exist != 0){
        /* システムトレースオーディットナンバー */
        if(NWM_CTO_numchk(pst_fxd_1804_col->b11_system_audit_number.ffd_data,
                          sizeof(pst_fxd_1804_col->b11_system_audit_number.ffd_data)) != 0){
            /* エラー発生BIT番号設定 */
            memcpy(pch_err_bit,
                   DEF_CHK_ERR_BIT_11,
                   strlen(DEF_CHK_ERR_BIT_11));
            return DEF_IPC_ERRCD_FAILMSG;
        }
    }

    if(pst_fxd_1804_col->b12_local_tran_time.ffd_header.m_flg_exist != 0){
        /* 現地取引日時 */
        snprintf(buf, sizeof(buf), "20%12.12s", pst_fxd_1804_col->b12_local_tran_time.ffd_data);
        if(NWM_CTO_datechk(buf,strlen(buf)) != 0){
            /* エラー発生BIT番号設定 */
            memcpy(pch_err_bit,
                   DEF_CHK_ERR_BIT_12,
                   strlen(DEF_CHK_ERR_BIT_12));
            return DEF_IPC_ERRCD_FAILMSG;
        }
    }

    if(pst_fxd_1804_col->b24_function_code.ffd_header.m_flg_exist != 0){
        /* ファンクションコード */
        if(memcmp(pst_fxd_1804_col->b24_function_code.ffd_data, DEF_CA_F24_FUNCTION_821_CUT, strlen(DEF_CA_F24_FUNCTION_821_CUT)) != 0){
            /* エラー発生BIT番号設定 */
            memcpy(pch_err_bit,
                   DEF_CHK_ERR_BIT_24,
                   strlen(DEF_CHK_ERR_BIT_24));
            return DEF_IPC_ERRCD_FAILMSG;
        }
    }else{
            /* エラー発生BIT番号設定 */
            memcpy(pch_err_bit,
                   DEF_CHK_ERR_BIT_24,
                   strlen(DEF_CHK_ERR_BIT_24));
            return DEF_IPC_ERRCD_FAILMSG;
    }

    if(pst_fxd_1804_col->b28_scrutiny_date.ffd_header.m_flg_exist != 0){
        /* 精査日 */
        snprintf(buf,sizeof(buf),"20%-6.6s", pst_fxd_1804_col->b28_scrutiny_date.ffd_data);
        if(NWM_CTO_datechk(buf, 8) != 0){
            /* エラー発生BIT番号設定 */
            memcpy(pch_err_bit,
                   DEF_CHK_ERR_BIT_28,
                   strlen(DEF_CHK_ERR_BIT_28));
            return DEF_IPC_ERRCD_FAILMSG;
        }
    }

    return 0;

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
    char                        datetime_hex[64];   /* COM_UNQ用バッファ */
    char                        szbuf[32];          /* 編集用バッファ */

    /* C401 制御電文処理要求 */
    cr401_def                    *pst_req_cr401_inf = (cr401_def *)psh_reqmsg;
    /* システム間インターフェース・CARDNET・電文ヘッダ */
    MSG_HEADER_CARDNET_def      *pst_req_cdnet_hdr;
    msg_cardnet_def             *pst_req_biz_msg;
    fixedform_cardnet_1804_def  *pst_req_fxd_1804_col;

    /* C401 制御電文処理要求 */
    cr401_def                    *pst_rsp_cr401_inf = (cr401_def *)pch_rspmsg;
    /* システム間インターフェース・CARDNET・電文ヘッダ */
    MSG_HEADER_CARDNET_def      *pst_rsp_cdnet_hdr;
    msg_cardnet_def             *pst_rsp_biz_msg;
    fixedform_cardnet_1814_def  *pst_rsp_fxd_1814_col;

    // 電文長演算
    *psh_len = (short)(
                 ((char *)pst_rsp_cr401_inf->data_bu.message_text - pch_rspmsg)
               + sizeof(MSG_HEADER_CARDNET_def)
               + sizeof(pst_rsp_biz_msg->mti)
               + sizeof(fixedform_cardnet_1814_def) );

    /* -----------------------------------------------------------------------*/
    /* 初期処理                                                               */
    /* -----------------------------------------------------------------------*/
    /* 要求 */
    /* C401 制御電文処理要求 */
    pst_req_biz_msg =   (void *)&pst_req_cr401_inf->data_bu;
    pst_req_cdnet_hdr = &pst_req_biz_msg->header;
    pst_req_fxd_1804_col = (void *)&pst_req_biz_msg->ffd;

    /* 応答 */
    pst_rsp_biz_msg =   (void *)&pst_rsp_cr401_inf->data_bu;
    pst_rsp_cdnet_hdr = &pst_rsp_biz_msg->header;
    pst_rsp_fxd_1814_col = (void *)&pst_rsp_biz_msg->ffd;

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
    if(memcmp(pch_inn_errcd, DEF_NWM_CTO_MSG_FMT_NG, strlen(DEF_NWM_CTO_MSG_FMT_NG)) == 0){
        // 電文精査異常
        pst_rsp_cr401_inf->common_header.error_code = DEF_IPC_ERRCD_FAILMSG;
    }

    /* 内部エラーコード*/
    memcpy(pst_rsp_cr401_inf->common_header.internal_error_code,
           pch_inn_errcd,
           strlen(pch_inn_errcd));

    /* データ長                     */
    pst_rsp_cr401_inf->common_header.control_data_length =  sizeof(MSG_HEADER_CARDNET_def)
                                                          + sizeof(pst_rsp_biz_msg->mti)
                                                          + sizeof(fixedform_cardnet_1814_def);

    /* 応答種別                     */
    if(memcmp(pst_req_cr401_inf->control_info.request_kind,
                DEF_CTLREQ_HISIMUKE_ERROR,
                strlen(DEF_CTLREQ_HISIMUKE_ERROR))!=0){
        // 通常
        // 電文あり
        memcpy(pst_rsp_cr401_inf->control_info.response_kind,
               DEF_CTLRSP_SEND,
               strlen(DEF_CTLRSP_SEND));
    }else{
        // 被仕向応答送信不可
        // 電文なし
        memcpy(pst_rsp_cr401_inf->control_info.response_kind,
               DEF_CTLRSP_NOSEND,
               strlen(DEF_CTLRSP_NOSEND));
    }

    /* 制御電文種別                 */
    memcpy(&pst_rsp_cr401_inf->control_info.control_kind,
           DEF_NWM_CTO_RSP_TYPE,
           strlen(DEF_NWM_CTO_RSP_TYPE));

    /* 電文種別                 */
    pst_rsp_cr401_inf->control_info.denbun_log_key.denbun_shubetu =
                pst_rsp_cr401_inf->control_info.control_kind.ctl_text_kbn;

    /* MTI                      */
    memcpy(pst_rsp_cr401_inf->control_info.mti,
           DEF_NWM_CTO_CTLMSG_RES,
           strlen(DEF_NWM_CTO_CTLMSG_RES));

    /* -----------------------------------------------------------------------*/
    /* 電文編集・ヘッダ部                                                     */
    /* -----------------------------------------------------------------------*/
    /* 共通制御・ヘッダータイプ */
    memcpy(pst_rsp_cdnet_hdr->ctrl_hdr_type, DEF_CA_CMHD_TYPE_F1, strlen(DEF_CA_CMHD_TYPE_F1));

    /* 共通制御・全体電文長 */
    memset(pst_rsp_cdnet_hdr->ctrl_msg_len, 0x00, strlen(pst_rsp_cdnet_hdr->ctrl_msg_len));

    /* 共通制御・差出センターID */
    memcpy(pst_rsp_cdnet_hdr->ctrl_src_id,
           pst_req_cdnet_hdr->ctrl_dst_id,
           sizeof(pst_rsp_cdnet_hdr->ctrl_src_id));

    /* 共通制御・宛先センターID */
    memcpy(pst_rsp_cdnet_hdr->ctrl_dst_id,
           pst_req_cdnet_hdr->ctrl_src_id,
           sizeof(pst_rsp_cdnet_hdr->ctrl_dst_id));

    /* 加盟店契約会社コード */
    memset(pst_rsp_cdnet_hdr->ctrl_merch_code, '0', sizeof(pst_rsp_cdnet_hdr->ctrl_merch_code));

    /* 共通制御・送信日時 */
    memset(szbuf, 0x00, sizeof(szbuf));
    memset(datetime_hex, 0x00, sizeof(datetime_hex));
    COM_UNQ((COM_UNQ_arg_1_def *)szbuf, datetime_hex);
    NWM_CTO_CHAR2BCD(pst_rsp_cdnet_hdr->ctrl_snd_time, szbuf, sizeof(pst_rsp_cdnet_hdr->ctrl_snd_time));

    /* 共通制御・モードフラグ */
    pst_rsp_cdnet_hdr->ctrl_mode_flg = pst_req_cdnet_hdr->ctrl_mode_flg ;

    /* 共通制御・予備 */
    memset(pst_rsp_cdnet_hdr->ctrl_filler, ' ', sizeof(pst_rsp_cdnet_hdr->ctrl_filler));;

    /* 業務共通・ヘッダータイプ */
    memcpy(pst_rsp_cdnet_hdr->bh_hdr_type, DEF_CA_APHD_TYPE_A1, strlen(DEF_CA_APHD_TYPE_A1));

    /* 業務共通・電文種別コード */
    memcpy(pst_rsp_cdnet_hdr->bh_msg_type, DEF_CA_APHD_MSGCODE_C814_RSP, strlen(DEF_CA_APHD_MSGCODE_C814_RSP));

    /* 業務共通・電文認証値 */
    memset(pst_rsp_cdnet_hdr->bh_auth_val, 0x00, sizeof(pst_rsp_cdnet_hdr->bh_auth_val));;

    /* 業務共通・チェックディジット */
    memset(&pst_rsp_cdnet_hdr->bh_chk_digit, 0x00, sizeof(pst_rsp_cdnet_hdr->bh_chk_digit));;

    /* 業務共通・仕向区分 */
    pst_rsp_cdnet_hdr->bh_dst_type = pst_req_cdnet_hdr->bh_dst_type;

    /* 業務共通・カット対象日付 */
    memcpy(pst_rsp_cdnet_hdr->bh_cut_date,
           pst_req_cdnet_hdr->bh_cut_date,
           sizeof(pst_rsp_cdnet_hdr->bh_cut_date));

    /* 業務共通・BODY部電文長 */
    memset(pst_rsp_cdnet_hdr->bh_body_len, 0x00, sizeof(pst_rsp_cdnet_hdr->bh_body_len));

    /* 業務共通・カードネット取引識別 */
    memcpy(pst_rsp_cdnet_hdr->bh_cardnet_id, pst_rsp_cdnet_hdr->bh_cardnet_id, sizeof(pst_rsp_cdnet_hdr->bh_cardnet_id));
    /* 業務共通・カードネット取引通番 */
    memcpy(pst_rsp_cdnet_hdr->bh_cardnet_seq, pst_rsp_cdnet_hdr->bh_cardnet_seq, sizeof(pst_rsp_cdnet_hdr->bh_cardnet_seq));
    /* 業務共通・カードネット使用域 */
    memcpy(pst_rsp_cdnet_hdr->bh_cardnet_area, pst_rsp_cdnet_hdr->bh_cardnet_area, sizeof(pst_rsp_cdnet_hdr->bh_cardnet_area));
    /* 業務共通・予備 */
    memset(pst_rsp_cdnet_hdr->bh_filler, ' ', sizeof(pst_rsp_cdnet_hdr->bh_filler));

    /* -----------------------------------------------------------------------*/
    /* 電文編集・データ部                                                     */
    /* -----------------------------------------------------------------------*/
    /* MTI */
    memcpy(pst_rsp_biz_msg->mti, DEF_NWM_CTO_CTLMSG_RES, strlen(DEF_NWM_CTO_CTLMSG_RES));

    // 以降は基本的にフラグが有効な場合に要求データコピーで対応する。
    // F11.システムトレースオーディットナンバー
    NWM_CTO_MCR_CPY_1814(pst_rsp_fxd_1814_col->b11_system_audit_number,
                         pst_req_fxd_1804_col->b11_system_audit_number);

    // F12.現地取引日時
    NWM_CTO_MCR_CPY_1814(pst_rsp_fxd_1814_col->b11_system_audit_number,
                         pst_req_fxd_1804_col->b11_system_audit_number);

    /* システムトレースオーディットナンバー */
    NWM_CTO_MCR_CPY_1814(pst_rsp_fxd_1814_col->b12_local_tran_time,
                         pst_req_fxd_1804_col->b12_local_tran_time);

    // F24.ファンクションコード
    NWM_CTO_MCR_CPY_1814(pst_rsp_fxd_1814_col->b24_function_code,
                         pst_req_fxd_1804_col->b24_function_code);

    // F28.精査日
    NWM_CTO_MCR_CPY_1814(pst_rsp_fxd_1814_col->b28_scrutiny_date,
                         pst_req_fxd_1804_col->b28_scrutiny_date);

    // F39.アクションコード
    pst_rsp_fxd_1814_col->b39_action_code.ffd_header.m_flg_exist = 1;
    pst_rsp_fxd_1814_col->b39_action_code.ffd_header.m_fixvalue_length =
        sizeof(pst_rsp_fxd_1814_col->b39_action_code.ffd_data);
    if(memcmp(pch_inn_errcd, DEF_NWM_CTO_CEN_STS_NG, strlen(DEF_NWM_CTO_CEN_STS_NG)) == 0){
        // 局状態異常
        memcpy(pst_rsp_fxd_1814_col->b39_action_code.ffd_data,
               DEF_NWM_CTO_ACT_INSP_NG,
               strlen(DEF_NWM_CTO_ACT_INSP_NG));
    }else{
        // その他（通常）
        memcpy(pst_rsp_fxd_1814_col->b39_action_code.ffd_data,
               DEF_NWM_CTO_ACT_INSP_OK,
               strlen(DEF_NWM_CTO_ACT_INSP_OK));
    }

    // F53.セキュリティ関連制御情報
    NWM_CTO_MCR_CPY_1814(pst_rsp_fxd_1814_col->b53_secure_ctl_info,
                         pst_req_fxd_1804_col->b53_secure_ctl_info);

    // F93.電文送信先センターID
    NWM_CTO_MCR_CPY_1814(pst_rsp_fxd_1814_col->b93_src_center_id,
                         pst_req_fxd_1804_col->b93_src_center_id);

    // F94.電文送信元センターID
    NWM_CTO_MCR_CPY_1814(pst_rsp_fxd_1814_col->b94_dst_center_id,
                         pst_req_fxd_1804_col->b94_dst_center_id);

    // F96.キーマネージメントデータ
    NWM_CTO_MCR_CPY_1814(pst_rsp_fxd_1814_col->b96_key_management_data,
                         pst_req_fxd_1804_col->b96_key_management_data);

    return;
}
/*****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_CTO_cst_check_req_rcv                       */
/*  CALLING SEQ.    : void NWM_CTO_cst_check_req_rcv(char,char *)            */
/*  ARGUMENT        : 1.pch_sta_sts     (I)   局状態                         */
/*                  : 2.pch_inner_errcd (I)   内部エラーコード               */
/*  RETURN CODE     :  0：局状態正常                                         */
/*                    -1：局状態異常                                         */
/*  DESCRIPTION     : カットオーバー電文精査                                 */
/*****************************************************************************/
short
NWM_CTO_cst_check_req_rcv(
    char         *pch_sta_sts,      /* 局状態 */
    char         *pch_inner_errcd   /* 内部エラーコード */
)
{
    /* 開局以外はエラー */
    if(memcmp(pch_sta_sts, DEF_STTE_STS_OPN, strlen(DEF_STTE_STS_OPN)) != 0){
        memcpy(pch_inner_errcd, DEF_NWM_CTO_CEN_STS_NG, strlen(DEF_NWM_CTO_CEN_STS_NG));
        return -1;
    }

    return 0;
}
/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CCUT_EMS_out                                    */
/*  CALLING SEQ.    : short NWM_CTO_numchk(char *,size_t)                    */
/*  ARGUMENT        : 1.col            (I)   検証対象日付文字列              */
/*                  : 2.ilen           (I)   文字列長(8:YMD/14:YMDhms)       */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 数字文字列チェック処理                                 */
/*****************************************************************************/
static short NWM_CTO_numchk(
    char   *col,
    size_t len)
{
    int i;

    for( i = 0; i < len; i++){
        if(isdigit(col[i]) == 0){
            return -1;
        }
    }
    return 0;
}

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CCUT_EMS_out                                    */
/*  CALLING SEQ.    : short NWM_CTO_datechk(char *,size_t)                   */
/*  ARGUMENT        : 1.col            (I)   検証対象日付文字列              */
/*                  : 2.ilen           (I)   文字列長(8:YMD/14:YMDhms)       */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 日付文字列チェック処理                                 */
/*****************************************************************************/
static short NWM_CTO_datechk(
    char      *col,
    size_t    ilen)
{
    char chBuff[32];
    char *pch_Ptr = col;
    int inum;

    /* 数値チェック */
    if(NWM_CTO_numchk(col, ilen) != 0){
        return -1;
    }

    /* 年 */
    memcpy(chBuff, pch_Ptr, 4);
    chBuff[4] = 0x00;
    inum = atoi(chBuff);
    if(atoi(chBuff) < 1900){
        return -1;
    }

    pch_Ptr += 4;

    /* 月 */
    memcpy(chBuff, pch_Ptr, 2);
    chBuff[2] = 0x00;     /* 以降2バイトのみ対象なので、NULL文字挿入不要 */
    inum = atoi(chBuff);
    if(inum < 1 || inum >12){
        return -1;
    }
    pch_Ptr += 2;

    /* 日 */
    memcpy(chBuff, pch_Ptr, 2);

    inum = atoi(chBuff);
    if(inum < 1 || inum > 31){
        return -1;
    }

    pch_Ptr += 2;

    if(ilen == 8 ){
        /* YYYYMMDDの場合ここで終了 */
        return 0;
    }

    /* 時 */
    memcpy(chBuff, pch_Ptr, 2);

    inum = atoi(chBuff);
    if(inum > 23){
        return -1;
    }

    pch_Ptr += 2;

    /* 分 */
    memcpy(chBuff, pch_Ptr, 2);

    inum = atoi(chBuff);
    if(inum > 59){
        return -1;
    }

    pch_Ptr += 2;

    if(ilen == 12 ){
        /* YYYYMMDDhhmmの場合ここで終了 */
        return 0;
    }

    /* 秒 */
    memcpy(chBuff, pch_Ptr, 2);

    inum = atoi(chBuff);
    if(inum > 59){
        return -1;
    }

    /* 正常終了 */
    return 0;
}
/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CCUT_EMS_out                                    */
/*  CALLING SEQ.    : short NWM_CTO_datechk(unsigned char *, char *,short )  */
/*  ARGUMENT        : 1.bcd_p          (I)   返還前BCD                       */
/*                  : 2.ascii_p        (O)   変換後文字列                    */
/*                  : 2.s_len          (I)   BCDサイズ                       */
/*  RETURN CODE     :  0 正常                                                 */
/*                  : -1 異常                                                 */
/*  DESCRIPTION     : BCD->ASCII変換処理                                     */
/*****************************************************************************/
static short NWM_CTO_BCD2CHAR(unsigned char *bcd_p, char *ascii_p,short s_len)
{
    short s_count;
    short a_indx;
    const char ToNUM_tbl[16] = {"0123456789******"};    /* ニューメリック変換テーブル */

    // BCDの上位下位のそれぞれは0～9の想定
    for (s_count = 0; s_count < s_len; s_count++) {
        a_indx = s_count * 2;
        ascii_p[a_indx]   = ToNUM_tbl[bcd_p[s_count] >> 4];
        if(ascii_p[a_indx] == '*'){
            return -1;
        }

        a_indx++;

        ascii_p[a_indx] = ToNUM_tbl[bcd_p[s_count] & 0x0f];
        if(ascii_p[a_indx] == '*'){
            return -1;
        }
    }
    return 0;
} /* end of NWM_CTO_BCD2CHAR */

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CCUT_EMS_out                                    */
/*  CALLING SEQ.    : short NWM_CTO_datechk(unsigned char *, char *,short )  */
/*  ARGUMENT        : 1.bcd_p          (O)   返還前BCD                       */
/*                  : 2.ascii_p        (I)   変換後文字列                    */
/*                  : 2.s_len          (I)   BCDサイズ                       */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : ASCII->BCD変換処理                                     */
/*****************************************************************************/
static void NWM_CTO_CHAR2BCD(unsigned char *bcd_p, char *ascii_p,short s_len)
{
    short s_count;
    // 数値文字列は0埋めの前提
    for (s_count = 0; s_count < s_len; s_count++) {
        bcd_p[s_count] = (unsigned char)(((ascii_p[s_count*2]   - '0' ) <<    4) +
                                         ((ascii_p[s_count*2+1] - '0' ) &  0x0f));
    }
} /* end of NWM_CTO_BCD2CHAR */

