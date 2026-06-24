/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSJ80                                    */
/*        FUNCTION          ････ NW個別(鍵交換個別処理[CARDNET])             */
/*                               鍵交換個別処理[CARDNET])を行う。            */
/*                                                                           */
/*        AUTHER            ････ HAS                                         */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-04-06                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS        2025/03/06 新規作成                                      */
/*                                                                           */
/*****************************************************************************/
/****************************************************************************/
/*   ｺﾝﾊﾟｲﾙｵﾌﾟｼｮﾝ                                                           */
/****************************************************************************/
#pragma ENV COMMON

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <stdbool.h>  nolist
#include <stdio.h>    nolist
#include <stdlib.h>   nolist
#include <string.h>   nolist
#include <cextdecs.h> nolist
#include <tal.h>      nolist

/* USER HEADER     */
#include "vproc.h"
#include "common.h"
#include "file.h(db_glnlg)"
#include "file.h(db_gckey)"
#include "file.h(db_gfnwi)"
#include "file.h(db_gfnws)"
#include "file.h(db_gccut)"
#include "ipc.h"
#include "ems.h"
#include "errcd.h"
#include "msg_CA.h"
#include "GFPOGGZ3_encode.h"
#include "GFPCGX50.h"
#include "NWM_KYX.h"
#include "GFPCSJ80.h"

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_msg_check_req                           */
/*  CALLING SEQ.    : short NWM_KYX_msg_check_req(                           */
/*                                      char              *req_message       */
/*                                    , char              *p_rcv_data_len    */
/*                                    , db_gfnwi_def      *nwi_g             */
/*                                    , db_gfnwi_def      *nwi_i             */
/*                                    , char              *nws_n             */
/*                                    , char              *nws_i             */
/*                                    , char              *nws_s             */
/*                                    , char              *nws_c             */
/*                                    , db_gckey_def      *gckey             */
/*                                    , NWM_KYX_arg_1_def *NWM_KYX_arg_1)    */
/*  ARGUMENT        : 1.req_message    (I) 受信データ(要求電文)              */
/*                  : 2.p_rcv_data_len (I) 受信電文長                        */
/*                  : 3.nwi_g          (I) NW情報レコード(グループ単位)      */
/*                  : 4.nwi_i          (I) NW情報レコード(インタフェース単位)*/
/*                  : 5.nws_n          (I) 接続先固有情報(NW単位)            */
/*                  : 6.nws_i          (I) 接続先固有情報(インタフェース単位)*/
/*                  : 7.nws_s          (I) 接続先固有情報(ステーション単位)  */
/*                  : 8.nws_c          (I) 接続先固有情報(コネクション単位)  */
/*                  : 9.gckey          (I) 鍵管理情報レコード                */
/*                  : 10.NWM_KYX_arg_1 (O) 精査処理結果                      */
/*  RETURN CODE     : 0：精査OK                                              */
/*                  : 1：精査エラー(拒否応答)                                */
/*                  : 3：精査エラー(電文破棄)                                */
/*                  : 9：精査エラー(異常)                                    */
/*  DESCRIPTION     : 鍵交換電文精査(要求受信)                               */
/*****************************************************************************/
short NWM_KYX_msg_check_req(char              *req_message
                          , char              *p_rcv_data_len
                          , db_gfnwi_def      *nwi_g
                          , db_gfnwi_def      *nwi_i
                          , char              *nws_n
                          , char              *nws_i
                          , char              *nws_s
                          , char              *nws_c
                          , db_gckey_def      *gckey
                          , NWM_KYX_arg_1_def *NWM_KYX_arg_1)
{
    short ls_result = 0;
    short loop_cnt  = 0;
    MSG_HEADER_CARDNET_def     *rcv_msg;
    msg_cardnet_def            *rcv_msg_ctl;
    nws_unq_info_ca_def        *nws_data;
    nwi_unq_info_ca_def    *nwi_unq_info;
    fixedform_cardnet_1804_def *rcv_1804_msg;
    int    set_data = 0;
    int    ffd_cnt  = 0;
    char   ch_text[64+1];
    char   ascii_time[16];

    rcv_msg     = (MSG_HEADER_CARDNET_def*)req_message;
    rcv_msg_ctl = (msg_cardnet_def*)rcv_msg;

    /* NW情報ファイル・接続先固有情報設定 */
    nwi_unq_info  = (nwi_unq_info_ca_def*)nwi_i->dst_unq_info;

    memset(NWM_KYX_arg_1, DEF_NWM_KYX_SPACE, sizeof(NWM_KYX_arg_1));

    /* ヘッダ部確認 */
    /* ヘッダータイプ                  */
    if (memcmp(rcv_msg->ctrl_hdr_type,
               DEF_CA_CMHD_TYPE_F1,
               sizeof(rcv_msg->ctrl_hdr_type)) != 0){
        /* エラー発生ヘッダ番号設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_HEADER_901_ERR,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }
    /* 全体電文長                      */
    for(loop_cnt=0; loop_cnt<2; loop_cnt++){
        /* BCDチェック */
        set_data = rcv_msg->ctrl_msg_len[loop_cnt] & 0x0f;
        if (set_data > 9){
            /* エラー発生ヘッダ番号設定 */
            memcpy(NWM_KYX_arg_1->err_bit,
                   DEF_NWM_KYX_HEADER_902_ERR,
                   sizeof(NWM_KYX_arg_1->err_bit));
            /* 障害電文通知設定 */
            return DEF_NWM_KYX_RTN_FAULT_MSG;
        }
        set_data = ((rcv_msg->ctrl_msg_len[loop_cnt] >> 4) & 0x0f);
        if (set_data > 9){
            /* エラー発生ヘッダ番号設定 */
            memcpy(NWM_KYX_arg_1->err_bit,
                   DEF_NWM_KYX_HEADER_903_ERR,
                   sizeof(NWM_KYX_arg_1->err_bit));
            /* 障害電文通知設定 */
            return DEF_NWM_KYX_RTN_FAULT_MSG;
        }
    }

    /* 差出センターID                  */
    /* 接続先固有情報(NW単位)         */
    nws_data = (nws_unq_info_ca_def*)nws_n;
    if (memcmp(nws_data->dst_center_id,
               rcv_msg->ctrl_src_id,
               sizeof(nws_data->dst_center_id))!=0){
            /* エラー発生ヘッダ番号設定 */
            memcpy(NWM_KYX_arg_1->err_bit,
                   DEF_NWM_KYX_HEADER_904_ERR,
                   sizeof(NWM_KYX_arg_1->err_bit));
            /* 障害電文通知設定 */
            return DEF_NWM_KYX_RTN_FAULT_MSG;
    }

    /* 宛先センターID                  */
    /* 接続先固有情報(ステーション単位)         */
    nws_data = (nws_unq_info_ca_def*)nws_s;
    if (memcmp(nws_data->src_center_id,
               rcv_msg->ctrl_dst_id,
               sizeof(nws_data->src_center_id))!=0){
        /* エラー発生ヘッダ番号設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_HEADER_905_ERR,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }

    /* 送信日時                        */
    memset(ascii_time, '0', sizeof(ascii_time));
    NWM_KYX_BCD2CHAR(rcv_msg->ctrl_snd_time, ascii_time, 7);
    ls_result = NWM_KYX_check_datetime(ascii_time);
    if (ls_result != DEF_NWM_KYX_RTN_OK){
        /* エラー発生ヘッダ番号設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_HEADER_906_ERR,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }

    /* モードフラグ                    */
    if (((rcv_msg->ctrl_mode_flg == DEF_CA_APHD_MODE_10_TEST) &&
         (nwi_unq_info->mode_flg == DEF_NWM_KYX_mode_test_nw)   ) ||
        ((rcv_msg->ctrl_mode_flg == DEF_CA_APHD_MODE_00_HONBAN) &&
         (nwi_unq_info->mode_flg == DEF_NWM_KYX_mode_honban_nw))  ){
        /* モード正常 */
    }
    else{
        /* エラー発生ヘッダ番号設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_HEADER_907_ERR,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }

    /* 業務共通ヘッダータイプ */
    if (memcmp(rcv_msg->bh_hdr_type,
               DEF_CA_APHD_TYPE_A1,
               sizeof(rcv_msg->bh_hdr_type)) != 0){
        /* エラー発生ヘッダ番号設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_HEADER_908_ERR,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }

    /* 電文種別コード                  */
    if (memcmp(rcv_msg->bh_msg_type,
               DEF_CA_APHD_MSGCODE_C804_REQ,
               sizeof(rcv_msg->bh_msg_type)) != 0){
        /* エラー発生ヘッダ番号設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_HEADER_909_ERR,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }

    /* カット対象日付                  */
    memset(ascii_time, '0', sizeof(ascii_time));
    NWM_KYX_BCD2CHAR(rcv_msg->bh_cut_date, ascii_time, 4);
    ls_result = NWM_KYX_check_datetime(ascii_time);
    if (ls_result != DEF_NWM_KYX_RTN_OK){
        /* エラー発生ヘッダ番号設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_HEADER_912_ERR,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }

    /* BODY部電文長                    */
    for(loop_cnt=0; loop_cnt<2; loop_cnt++){
        /* BCDチェック */
        set_data = rcv_msg->bh_body_len[loop_cnt] & 0x0f;
        if (set_data > 9){
            /* エラー発生ヘッダ番号設定 */
            memcpy(NWM_KYX_arg_1->err_bit,
                   DEF_NWM_KYX_HEADER_913_ERR,
                   sizeof(NWM_KYX_arg_1->err_bit));
            /* 障害電文通知設定 */
            return DEF_NWM_KYX_RTN_FAULT_MSG;
        }
        set_data = ((rcv_msg->bh_body_len[loop_cnt] >> 4) & 0x0f);
        if (set_data > 9){
            /* エラー発生ヘッダ番号設定 */
            memcpy(NWM_KYX_arg_1->err_bit,
                   DEF_NWM_KYX_HEADER_914_ERR,
                   sizeof(NWM_KYX_arg_1->err_bit));
            /* 障害電文通知設定 */
            return DEF_NWM_KYX_RTN_FAULT_MSG;
        }
    }

    /* データ部確認 */
    /* MTIチェック */
    if (memcmp(rcv_msg_ctl->mti,
               DEF_CA_MTI_1804_REQ,
               sizeof(rcv_msg_ctl->mti)) != 0){
        /* エラー発生ヘッダ番号設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_HEADER_915_ERR,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }

    rcv_1804_msg = (fixedform_cardnet_1804_def*)&rcv_msg_ctl->ffd;

    if (rcv_1804_msg->b11_system_audit_number.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_11,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }
    if (rcv_1804_msg->b11_system_audit_number.ffd_header.m_fixvalue_length >
                                 DEF_NWM_KYX_BIT_11_LENG){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_11,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }
    for(loop_cnt=0; loop_cnt<DEF_NWM_KYX_BIT_11_LENG; loop_cnt++){
        /* NUMERICチェック */
        if ((rcv_1804_msg->b11_system_audit_number.ffd_data[loop_cnt] < '0') ||
            (rcv_1804_msg->b11_system_audit_number.ffd_data[loop_cnt] > '9')){
            /* エラー発生BIT設定 */
            memcpy(NWM_KYX_arg_1->err_bit,
                   DEF_NWM_KYX_BIT_11,
                   sizeof(NWM_KYX_arg_1->err_bit));
            /* 障害電文通知設定 */
            return DEF_NWM_KYX_RTN_FAULT_MSG;
        }
    }

    if (rcv_1804_msg->b12_local_tran_time.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_12,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }
    if (rcv_1804_msg->b12_local_tran_time.ffd_header.m_fixvalue_length !=
                                 DEF_NWM_KYX_BIT_12_LENG){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_12,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }

    memset(ascii_time, '0', sizeof(ascii_time));
    ascii_time[0]='2';
    ascii_time[1]='0';
    memcpy(&ascii_time[2], rcv_1804_msg->b12_local_tran_time.ffd_data, 12);
    ls_result = NWM_KYX_check_datetime(ascii_time);
    if (ls_result != DEF_NWM_KYX_RTN_OK){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_12,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }

    if (rcv_1804_msg->b24_function_code.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_24,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }
    if (rcv_1804_msg->b24_function_code.ffd_header.m_fixvalue_length !=
                                 DEF_NWM_KYX_BIT_24_LENG){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_24,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }

    if (rcv_1804_msg->b93_src_center_id.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_93,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }

    if (rcv_1804_msg->b94_dst_center_id.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_94,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }
    if (rcv_1804_msg->b94_dst_center_id.ffd_header.m_fixvalue_length >
                                 DEF_NWM_KYX_BIT_94_LENG){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_94,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }

    if (rcv_1804_msg->b96_key_management_data.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_96,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }

    if ((rcv_1804_msg->b96_key_management_data.ffd_header.m_fixvalue_length == DEF_NWM_KYX_BIT_96_LENG_MAC_11) ||
        (rcv_1804_msg->b96_key_management_data.ffd_header.m_fixvalue_length == DEF_NWM_KYX_BIT_96_LENG_MAC_19)) {
        /* KC/KMAC */
        /* KEY識別判定 */
        if (memcmp(&rcv_1804_msg->b96_key_management_data.ffd_data[0],
                   DEF_NWM_KYX_BIT_96_ENC,
                   DEF_NWM_KYX_BIT_96_KYETYPE_LENG) == 0) {
            /* KEY種別設定 */
            memcpy(NWM_KYX_arg_1->key_kind,
                   DEF_NWM_KYX_KIND_KC,
                   sizeof(NWM_KYX_arg_1->key_kind) );
        }
        else if (memcmp(&rcv_1804_msg->b96_key_management_data.ffd_data[0],
                   DEF_NWM_KYX_BIT_96_MAC,
                   DEF_NWM_KYX_BIT_96_KYETYPE_LENG) == 0) {
            /* KEY種別設定 */
            memcpy(NWM_KYX_arg_1->key_kind,
                   DEF_NWM_KYX_KIND_KMAC,
                   sizeof(NWM_KYX_arg_1->key_kind) );
        }
        else {
            /* エラー発生BIT設定 */
            memcpy(NWM_KYX_arg_1->err_bit,
                   DEF_NWM_KYX_BIT_96,
                   sizeof(NWM_KYX_arg_1->err_bit));
            /* 障害電文通知設定 */
            return DEF_NWM_KYX_RTN_FAULT_MSG;
        }
    }
    else if ((rcv_1804_msg->b96_key_management_data.ffd_header.m_fixvalue_length == DEF_NWM_KYX_BIT_96_LENG_KPE_08) ||
             (rcv_1804_msg->b96_key_management_data.ffd_header.m_fixvalue_length == DEF_NWM_KYX_BIT_96_LENG_KPE_16)) {
        /* KPE */
        /* KEY種別設定 */
        memcpy(NWM_KYX_arg_1->key_kind,
               DEF_NWM_KYX_KIND_KPE,
               sizeof(NWM_KYX_arg_1->key_kind) );
    }
    else {
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_96,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 障害電文通知設定 */
        return DEF_NWM_KYX_RTN_FAULT_MSG;
    }

    if (memcmp(NWM_KYX_arg_1->key_kind,
               DEF_NWM_KYX_KIND_KPE,
               sizeof(NWM_KYX_arg_1->key_kind)) == 0) {
        /* KPEの場合bit53精査 */
        if (rcv_1804_msg->b53_secure_ctl_info.ffd_header.m_flg_exist != true){
            /* エラー発生BIT設定 */
            memcpy(NWM_KYX_arg_1->err_bit,
                   DEF_NWM_KYX_BIT_53,
                   sizeof(NWM_KYX_arg_1->err_bit));
            /* 障害電文通知設定 */
            return DEF_NWM_KYX_RTN_FAULT_MSG;
        }

        if (rcv_1804_msg->b53_secure_ctl_info.ffd_header.m_fixvalue_length !=
                                     DEF_NWM_KYX_BIT_53_LENG){
            /* エラー発生BIT設定 */
            memcpy(NWM_KYX_arg_1->err_bit,
                   DEF_NWM_KYX_BIT_53,
                   sizeof(NWM_KYX_arg_1->err_bit));
            /* 障害電文通知設定 */
            return DEF_NWM_KYX_RTN_FAULT_MSG;
        }
        /* KEY TYPEチェック */
        ffd_cnt = 0;
        if (memcmp(&rcv_1804_msg->b53_secure_ctl_info.ffd_data[ffd_cnt],
                   DEF_NWM_KYX_BIT_53_KEYTYPE,
                   2) != 0){
            /* エラー発生BIT設定 */
            memcpy(NWM_KYX_arg_1->err_bit,
                   DEF_NWM_KYX_BIT_53,
                   sizeof(NWM_KYX_arg_1->err_bit));
            /* 障害電文通知設定 */
            return DEF_NWM_KYX_RTN_FAULT_MSG;
        }
        /* PIN暗号化ロジックチェック */
        ffd_cnt += 2;
        if ((memcmp(&rcv_1804_msg->b53_secure_ctl_info.ffd_data[ffd_cnt],
                   DEF_NWM_KYX_BIT_53_PINENC_01,
                   2) != 0) &&
            (memcmp(&rcv_1804_msg->b53_secure_ctl_info.ffd_data[ffd_cnt],
                   DEF_NWM_KYX_BIT_53_PINENC_02,
                   2) != 0)){
            /* エラー発生BIT設定 */
            memcpy(NWM_KYX_arg_1->err_bit,
                   DEF_NWM_KYX_BIT_53,
                   sizeof(NWM_KYX_arg_1->err_bit));
            /* 障害電文通知設定 */
            return DEF_NWM_KYX_RTN_FAULT_MSG;
        }
        /* PINブロック形式チェック */
        ffd_cnt += 2;
        if (memcmp(&rcv_1804_msg->b53_secure_ctl_info.ffd_data[ffd_cnt],
                   DEF_NWM_KYX_BIT_53_PINBLK,
                   2) != 0){
            /* エラー発生BIT設定 */
            memcpy(NWM_KYX_arg_1->err_bit,
                   DEF_NWM_KYX_BIT_53,
                   sizeof(NWM_KYX_arg_1->err_bit));
            /* 障害電文通知設定 */
            return DEF_NWM_KYX_RTN_FAULT_MSG;
        }
        /* KEY索引値チェック */
        ffd_cnt += 2;
        if (memcmp(&rcv_1804_msg->b53_secure_ctl_info.ffd_data[ffd_cnt],
                   DEF_NWM_KYX_BIT_53_KEYINDEX,
                   2) != 0){
            /* エラー発生BIT設定 */
            memcpy(NWM_KYX_arg_1->err_bit,
                   DEF_NWM_KYX_BIT_53,
                   sizeof(NWM_KYX_arg_1->err_bit));
            /* 障害電文通知設定 */
            return DEF_NWM_KYX_RTN_FAULT_MSG;
        }
        /* 認証索引値チェック */
        ffd_cnt += 2;
        if (memcmp(&rcv_1804_msg->b53_secure_ctl_info.ffd_data[ffd_cnt],
                   DEF_NWM_KYX_BIT_53_AUTHINDEX,
                   2) != 0){
            /* エラー発生BIT設定 */
            memcpy(NWM_KYX_arg_1->err_bit,
                   DEF_NWM_KYX_BIT_53,
                   sizeof(NWM_KYX_arg_1->err_bit));
            /* 障害電文通知設定 */
            return DEF_NWM_KYX_RTN_FAULT_MSG;
        }
        /* チェックディジットチェック */
        ffd_cnt += 2;
        if (memcmp(&rcv_1804_msg->b53_secure_ctl_info.ffd_data[ffd_cnt],
                   DEF_NWM_KYX_BIT_53_CHKDIGIT,
                   4) == 0){
            /* エラー発生BIT設定 */
            memcpy(NWM_KYX_arg_1->err_bit,
                   DEF_NWM_KYX_BIT_53,
                   sizeof(NWM_KYX_arg_1->err_bit));
            /* 障害電文通知設定 */
            return DEF_NWM_KYX_RTN_FAULT_MSG;
        }
        /* KEY値 バイナリー ⇒ ASCII変換 */
        memset(ch_text, 0, sizeof(ch_text));
        HEX2CHAR(&rcv_1804_msg->b96_key_management_data.ffd_data[0],
                 ch_text,
                 (short)(rcv_1804_msg->b96_key_management_data.ffd_header.m_fixvalue_length) );
        memcpy(NWM_KYX_arg_1->key,
               ch_text,
               (rcv_1804_msg->b96_key_management_data.ffd_header.m_fixvalue_length * 2 ));
        NWM_KYX_arg_1->key_leng = (char)((rcv_1804_msg->b96_key_management_data.ffd_header.m_fixvalue_length) * 2 );
        /* チェックディジット設定 */
        memcpy(NWM_KYX_arg_1->checkdigit,
               &rcv_1804_msg->b53_secure_ctl_info.ffd_data[10],
               4 );
        NWM_KYX_arg_1->checkdigit_leng = 4;
        /* KEY種別設定 */
        memcpy(NWM_KYX_arg_1->key_kind,
               DEF_NWM_KYX_KIND_KPE,
               4 );
    }
    else {
        /* KC/KMAC */
        /* KC/KMACの場合bit53がないことを確認 */
        if (rcv_1804_msg->b53_secure_ctl_info.ffd_header.m_flg_exist == true){
            /* エラー発生BIT設定 */
            memcpy(NWM_KYX_arg_1->err_bit,
                   DEF_NWM_KYX_BIT_53,
                   sizeof(NWM_KYX_arg_1->err_bit));
            /* 障害電文通知設定 */
            return DEF_NWM_KYX_RTN_FAULT_MSG;
        }

        /* KEY値 バイナリー ⇒ ASCII変換 */
        memset(ch_text, 0, sizeof(ch_text));
        HEX2CHAR(&rcv_1804_msg->b96_key_management_data.ffd_data[3],
                 ch_text,
                 (short)(rcv_1804_msg->b96_key_management_data.ffd_header.m_fixvalue_length -3) );
        memcpy(NWM_KYX_arg_1->key,
               ch_text,
               (short)((rcv_1804_msg->b96_key_management_data.ffd_header.m_fixvalue_length -3) * 2 ));
        NWM_KYX_arg_1->key_leng = (char)((rcv_1804_msg->b96_key_management_data.ffd_header.m_fixvalue_length -3) * 2 );
        /* チェックディジット バイナリー ⇒ ASCII変換 */
        memset(ch_text, 0, sizeof(ch_text));
        if (memcmp(NWM_KYX_arg_1->key_kind,
                   DEF_NWM_KYX_KIND_KC,
                   sizeof(NWM_KYX_arg_1->key_kind)) == 0) {
            /* KC */
            /* チェックディジット(暗号化キー)  */
            if ((rcv_msg->bh_chk_digit.bh_chk_digit_kc[0] == NULL) ||
                (rcv_msg->bh_chk_digit.bh_chk_digit_kc[1] == NULL)){
                /* 障害電文通知設定 */
                /* エラー発生ヘッダ番号設定 */
                memcpy(NWM_KYX_arg_1->err_bit,
                       DEF_NWM_KYX_HEADER_910_ERR,
                       sizeof(NWM_KYX_arg_1->err_bit));
                /* 拒否応答設定 */
                return DEF_NWM_KYX_RTN_NG_REJ;
            }

            HEX2CHAR(rcv_msg->bh_chk_digit.bh_chk_digit_kc,
                     ch_text,
                     2 );
        }
        else{
            /* KMAC */
            /* チェックディジット(認証キー)    */
            if ((rcv_msg->bh_chk_digit.bh_chk_digit_kmac[0] == NULL) ||
                (rcv_msg->bh_chk_digit.bh_chk_digit_kmac[1] == NULL)){
                /* エラー発生ヘッダ番号設定 */
                memcpy(NWM_KYX_arg_1->err_bit,
                       DEF_NWM_KYX_HEADER_911_ERR,
                       sizeof(NWM_KYX_arg_1->err_bit));
                /* 拒否応答設定 */
                return DEF_NWM_KYX_RTN_NG_REJ;
            }

            HEX2CHAR(rcv_msg->bh_chk_digit.bh_chk_digit_kmac,
                     ch_text,
                     2 );
        }
        memcpy(NWM_KYX_arg_1->checkdigit,
               ch_text,
               4 );
        NWM_KYX_arg_1->checkdigit_leng = 4;
    }

    return DEF_NWM_KYX_RTN_OK;
}
/* end of NWM_KYX_msg_check_req */

/******************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_msg_check_rsp                            */
/*  CALLING SEQ.    : short NWM_KYX_msg_check_req(                            */
/*                                      short             key_data_kind       */
/*                                    , char              *req_message        */
/*                                    , char              *p_rcv_data_len    */
/*                                    , db_gfnwi_def      *nwi_g              */
/*                                    , db_gfnwi_def      *nwi_i              */
/*                                    , char              *nws_n              */
/*                                    , char              *nws_i              */
/*                                    , char              *nws_s              */
/*                                    , char              *nws_c              */
/*                                    , db_gckey_def      *gckey              */
/*                                    , NWM_KYX_arg_2_def *NWM_KYX_arg_2)     */
/*  ARGUMENT        :  1.key_data_kind  (I) 要求種別                          */
/*                  :  2.req_message    (I) 受信データ(要求電文)              */
/*                  :  3.p_rcv_data_len (I) 受信電文長                        */
/*                  :  4.nwi_g          (I) NW情報レコード(グループ単位)      */
/*                  :  5.nwi_i          (I) NW情報レコード(インタフェース単位)*/
/*                  :  6.nws_n          (I) 接続先固有情報(NW単位)            */
/*                  :  7.nws_i          (I) 接続先固有情報(インタフェース単位)*/
/*                  :  8.nws_s          (I) 接続先固有情報(ステーション単位)  */
/*                  :  9.nws_c          (I) 接続先固有情報(コネクション単位)  */
/*                  : 10.gckey          (I) 鍵管理情報レコード                */
/*                  : 11.NWM_KYX_arg_2  (O) 精査処理結果                      */
/*  RETURN CODE     : 0：精査OK                                               */
/*                  : 1：精査エラー(拒否応答)                                 */
/*                  : 3：精査エラー(電文破棄)                                 */
/*                  : 9：精査エラー(異常)                                     */
/*  DESCRIPTION     : 鍵交換電文精査(応答受信)                                */
/******************************************************************************/
short NWM_KYX_msg_check_rsp(short             key_data_kind
                          , char              *req_message
                          , char              *p_rcv_data_len
                          , db_gfnwi_def      *nwi_g
                          , db_gfnwi_def      *nwi_i
                          , char              *nws_n
                          , char              *nws_i
                          , char              *nws_s
                          , char              *nws_c
                          , db_gckey_def      *gckey
                          , NWM_KYX_arg_2_def *NWM_KYX_arg_2)
{
    /* CARDNETでは動作しない */
    return DEF_NWM_KYX_RTN_ERR;
}
/* end of NWM_KYX_msg_check_rsp */

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_msg_edit_req                           */
/*  CALLING SEQ.    : short NWM_KYX_msg_edit_req(                           */
/*                                      short             key_data_kind     */
/*                                    , db_gfnwi_def      *nwi_g            */
/*                                    , db_gfnwi_def      *nwi_i            */
/*                                    , char              *nws_n            */
/*                                    , char              *nws_i            */
/*                                    , char              *nws_s            */
/*                                    , char              *nws_c            */
/*                                    , db_gckey_def      *gckey            */
/*                                    , oggz1in_def       *ems_cmn          */
/*                                    , NWM_KYX_ems_add   *ems_add          */
/*                                    , NWM_KYX_arg_3_def *NWM_KYX_arg_3    */
/*                                    , NWM_KYX_arg_4_def *NWM_KYX_arg_4)   */
/*  ARGUMENT        :  1.key_data_kind(I) 要求種別                          */
/*                  :  2.nwi_g        (I) NW情報レコード(グループ単位)      */
/*                  :  3.nwi_i        (I) NW情報レコード(インタフェース単位)*/
/*                  :  4.nws_n        (I) 接続先固有情報(NW単位)            */
/*                  :  5.nws_i        (I) 接続先固有情報(インタフェース単位)*/
/*                  :  6.nws_s        (I) 接続先固有情報(ステーション単位)  */
/*                  :  7.nws_c        (I) 接続先固有情報(コネクション単位)  */
/*                  :  8.gckey        (I) 鍵管理情報レコード                */
/*                  :  9.ems_info_cmn (I) EMS出力共通情報                   */
/*                  : 10.ems_info_add (I) EMS出力付加情報                   */
/*                  : 11.NWM_KYX_arg_3(I) 設定データ                        */
/*                  : 12.NWM_KYX_arg_4(O) 電文情報                          */
/*  RETURN CODE     : 0：精査OK                                             */
/*                  : 1：精査エラー(拒否応答)                               */
/*                  : 3：精査エラー(電文破棄)                               */
/*                  : 9：精査エラー(異常)                                   */
/*  DESCRIPTION     : 鍵交換電文編集(要求電文)                              */
/****************************************************************************/
short NWM_KYX_msg_edit_req(short              key_data_kind
                         , db_gfnwi_def       *nwi_g
                         , db_gfnwi_def       *nwi_i
                         , char               *nws_n
                         , char               *nws_i
                         , char               *nws_s
                         , char               *nws_c
                         , db_gckey_def       *gckey
                         , oggz1in_def        *ems_cmn
                         , NWM_KYX_ems_add    *ems_add
                         , NWM_KYX_arg_3_def  *NWM_KYX_arg_3
                         , NWM_KYX_arg_4_def  *NWM_KYX_arg_4)
{
    /* CARDNETでは動作しない */
    return DEF_NWM_KYX_RTN_ERR;
}
/* end of NWM_KYX_msg_edit_req */

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_msg_edit_rsp                           */
/*  CALLING SEQ.    : short NWM_KYX_msg_edit_rsp(                           */
/*                                      db_gfnwi_def      *nwi_g            */
/*                                    , db_gfnwi_def      *nwi_i            */
/*                                    , char              *nws_n            */
/*                                    , char              *nws_i            */
/*                                    , char              *nws_s            */
/*                                    , char              *nws_c            */
/*                                    , db_gckey_def      *gckey            */
/*                                    , db_gccut_def      *gccut            */
/*                                    , oggz1in_def       *ems_cmn          */
/*                                    , NWM_KYX_ems_add   *ems_add          */
/*                                    , NWM_KYX_arg_3_def *NWM_KYX_arg_3    */
/*                                    , NWM_KYX_arg_4_def *NWM_KYX_arg_4)   */
/*  ARGUMENT        :  1.req_message  (I) 受信データ(要求電文)              */
/*                  :  2.nwi_g        (I) NW情報レコード(グループ単位)      */
/*                  :  3.nwi_i        (I) NW情報レコード(インタフェース単位)*/
/*                  :  4.nws_n        (I) 接続先固有情報(NW単位)            */
/*                  :  5.nws_i        (I) 接続先固有情報(インタフェース単位)*/
/*                  :  6.nws_s        (I) 接続先固有情報(ステーション単位)  */
/*                  :  7.nws_c        (I) 接続先固有情報(コネクション単位)  */
/*                  :  8.gckey        (I) 鍵管理情報レコード                */
/*                  :  9.gccut        (I) カット対象日付レコード            */
/*                  : 10.ems_info_cmn (I) EMS出力共通情報                   */
/*                  : 11.ems_info_add (I) EMS出力付加情報                   */
/*                  : 12.NWM_KYX_arg_5(I) 設定データ                        */
/*                  : 13.NWM_KYX_arg_6(O) 電文情報                          */
/*  RETURN CODE     : 0：精査OK                                             */
/*                  : 1：精査エラー(拒否応答)                               */
/*                  : 3：精査エラー(電文破棄)                               */
/*                  : 9：精査エラー(異常)                                   */
/*  DESCRIPTION     : 鍵交換電文編集(応答電文)                              */
/****************************************************************************/
short NWM_KYX_msg_edit_rsp(db_gfnwi_def       *nwi_g
                         , db_gfnwi_def       *nwi_i
                         , char               *nws_n
                         , char               *nws_i
                         , char               *nws_s
                         , char               *nws_c
                         , db_gckey_def       *gckey
                         , file_info_gccut    *gccut_info
                         , oggz1in_def        *ems_cmn
                         , NWM_KYX_ems_add    *ems_add
                         , NWM_KYX_arg_5_def  *NWM_KYX_arg_5
                         , NWM_KYX_arg_6_def  *NWM_KYX_arg_6)
{

    msg_cardnet_def                *send_1814_msg;
    msg_cardnet_def                *rcv_1804_msg;
    nwi_unq_info_ca_def            *nwi_unq_info;
    fixedform_cardnet_1814_def     *send_form_1814;
    fixedform_cardnet_1804_def     *rcv_1804_ffd;

    COM_SDT_arg_2_def  COM_SDT_arg_2_data;       /* 日時取得用データ        */
    COM_SDT_arg_3_def  COM_SDT_arg_3_data;       /* 日時取得用データ        */
    long long          date_time;                /* 日時取得用データ        */
    char               bcd_data[64];
    short              s_ret = 0;
    db_gccut_def       gccut;
    char               check_digit_kc_kmac[2];  /* チェックディジット KMAC/KC  */
    char               bcd_time[7+1];

    /* リクエスト電文を作成電文にリクエスト電文長分コピーする */
    memcpy(NWM_KYX_arg_6->message,
           NWM_KYX_arg_6->rcv_message,
           NWM_KYX_arg_6->rcv_message_leng);

    /* 送信バッファ設定 */
    send_1814_msg  = (msg_cardnet_def*)NWM_KYX_arg_6->message;
    rcv_1804_msg   = (msg_cardnet_def*)NWM_KYX_arg_6->rcv_message;
    send_form_1814 = (fixedform_cardnet_1814_def*)&send_1814_msg->ffd;
    rcv_1804_ffd   = (fixedform_cardnet_1804_def*)&rcv_1804_msg->ffd;

    /* NW情報ファイル・接続先固有情報設定 */
    nwi_unq_info  = (nwi_unq_info_ca_def*)nwi_i->dst_unq_info;

    /* ヘッダータイプ設定 */
    memcpy(send_1814_msg->header.ctrl_hdr_type,
           DEF_CA_CMHD_TYPE_F1,
           sizeof(send_1814_msg->header.ctrl_hdr_type));

    /* 全体電文長設定 */
    memset(send_1814_msg->header.ctrl_msg_len, 0, sizeof(send_1814_msg->header.ctrl_msg_len));

    /* 差出センターID設定 */
    memcpy(send_1814_msg->header.ctrl_src_id,
           rcv_1804_msg->header.ctrl_dst_id,
           sizeof(send_1814_msg->header.ctrl_src_id));

    /* 宛先センターID設定 */
    memcpy(send_1814_msg->header.ctrl_dst_id,
           rcv_1804_msg->header.ctrl_src_id,
           sizeof(send_1814_msg->header.ctrl_dst_id));

    /* 送信日時設定 */
    COM_SDT(DEF_COM_SDT_arg1_jpn,
            &COM_SDT_arg_2_data,
            &COM_SDT_arg_3_data,
            &date_time);
   /* BCD変換 */
   memset(bcd_time, 0, sizeof(bcd_time));
   s_ret =  NWM_KYX_CHAR2BCD((char*)&COM_SDT_arg_2_data,
                             bcd_time,
                             14);

    memcpy(send_1814_msg->header.ctrl_snd_time,
           bcd_time,
           sizeof(send_1814_msg->header.ctrl_snd_time));

    /* モードフラグ設定 */
    if (nwi_unq_info->mode_flg == '1'){
        send_1814_msg->header.ctrl_mode_flg = 0x10;
    }
    else{
        send_1814_msg->header.ctrl_mode_flg = 0x00;
    }
    /* 予備にスペースを設定 */
    memset(send_1814_msg->header.ctrl_filler,
           DEF_NWM_KYX_SPACE,
           sizeof(send_1814_msg->header.ctrl_filler));

    /* 業務共通 ヘッダータイプ設定 */
    memcpy(send_1814_msg->header.bh_hdr_type,
           DEF_CA_APHD_TYPE_A1,
           sizeof(send_1814_msg->header.bh_hdr_type));

    /* 業務共通 電文種別コード設定 */
    memcpy(send_1814_msg->header.bh_msg_type,
           DEF_CA_APHD_MSGCODE_C814_RSP,
           sizeof(send_1814_msg->header.bh_msg_type));

    /* 業務共通 電文認証値設定 */
    memset(send_1814_msg->header.bh_auth_val,
           0,
           sizeof(send_1814_msg->header.bh_auth_val));

    /* 業務共通 チェックディジット(KC)初期化 */
    memset(send_1814_msg->header.bh_chk_digit.bh_chk_digit_kc,
           0,
           sizeof(send_1814_msg->header.bh_chk_digit.bh_chk_digit_kc));
    /* 業務共通 チェックディジット(KMAC)初期化 */
    memset(send_1814_msg->header.bh_chk_digit.bh_chk_digit_kmac,
           0,
           sizeof(send_1814_msg->header.bh_chk_digit.bh_chk_digit_kmac));

    /* 業務共通 チェックディジット */
    /* チェックディジットのバイナリ変換 */
    memset(check_digit_kc_kmac, 0, sizeof(check_digit_kc_kmac));
    s_ret = NWM_KYX_CHAR2HEX(NWM_KYX_arg_5->checkdigit,
                             check_digit_kc_kmac, 4);
    if (s_ret == DEF_NWM_KYX_RTN_OK){
        if (memcmp(NWM_KYX_arg_5->key_kind,
                   DEF_KEY_TYPE_KC,
                   sizeof(NWM_KYX_arg_5->key_kind)) == 0){
            /* KC */
            memcpy(send_1814_msg->header.bh_chk_digit.bh_chk_digit_kc,
                   check_digit_kc_kmac,
                   2);
        } else if (memcmp(NWM_KYX_arg_5->key_kind,
                   DEF_KEY_TYPE_KMAC,
                   sizeof(NWM_KYX_arg_5->key_kind)) == 0){
            /* 業務共通 チェックディジット(KMAC) */
            memcpy(send_1814_msg->header.bh_chk_digit.bh_chk_digit_kmac,
                   check_digit_kc_kmac,
                   2);
        }
    }

    /* カット対象日付        */
    memset(send_1814_msg->header.bh_cut_date,
           0x00,
           sizeof(send_1814_msg->header.bh_cut_date));

    s_ret = NWM_KYX_read_cutfile(gccut_info,
                                 &NWM_KYX_arg_5->connection_lid,
                                 &gccut,
                                 ems_cmn,
                                 ems_add);
    if ( s_ret == DEF_NWM_KYX_RTN_OK ){
        /* BCD変換 */
        memset(bcd_data, 0, sizeof(bcd_data));
        s_ret =  NWM_KYX_CHAR2BCD(gccut.cut_date_info.cut_date,
                                  bcd_data,
                                  sizeof(gccut.cut_date_info.cut_date));
        if ( s_ret == DEF_NWM_KYX_RTN_OK ){
            memcpy(send_1814_msg->header.bh_cut_date,
                   bcd_data,
                   sizeof(send_1814_msg->header.bh_cut_date));
        }
    }

    /* BODY部電文長 */
    memset(send_1814_msg->header.bh_body_len, 0, sizeof(send_1814_msg->header.bh_body_len));

    /* 予備                  */
    memset(send_1814_msg->header.bh_filler,
           DEF_NWM_KYX_SPACE,
           sizeof(send_1814_msg->header.bh_filler));

    /* MTI設定 */
    memcpy(send_1814_msg->mti,
           DEF_NWM_KYX_IPC_1814,
           sizeof(send_1814_msg->mti));

    memcpy(NWM_KYX_arg_6->mti,
           DEF_NWM_KYX_IPC_1814,
           sizeof(NWM_KYX_arg_6->mti));

    /* F11.システムトレースオーディットナンバー設定 */
    memcpy(&send_form_1814->b11_system_audit_number,
           &rcv_1804_ffd->b11_system_audit_number,
           sizeof(send_form_1814->b11_system_audit_number));

    /* F12.現地取引日時設定 */
    memcpy(&send_form_1814->b12_local_tran_time,
           &rcv_1804_ffd->b12_local_tran_time,
           sizeof(send_form_1814->b12_local_tran_time));

    /* F24.現地取引日時設定 */
    memcpy(&send_form_1814->b24_function_code,
           &rcv_1804_ffd->b24_function_code,
           sizeof(send_form_1814->b24_function_code));

    /* F28.精査日 */
    memcpy(&send_form_1814->b28_scrutiny_date,
           &rcv_1804_ffd->b28_scrutiny_date,
           sizeof(send_form_1814->b28_scrutiny_date));

    /* F53.セキュリティ関連制御情報 */
    memcpy(&send_form_1814->b53_secure_ctl_info,
           &rcv_1804_ffd->b53_secure_ctl_info,
           sizeof(send_form_1814->b53_secure_ctl_info));

    /* F93.電文送信先センターID */
    memcpy(&send_form_1814->b93_src_center_id,
           &rcv_1804_ffd->b93_src_center_id,
           sizeof(send_form_1814->b93_src_center_id));

    /* F94.電文送信元センターID */
    memcpy(&send_form_1814->b94_dst_center_id,
           &rcv_1804_ffd->b94_dst_center_id,
           sizeof(send_form_1814->b94_dst_center_id));

    /* F96.キーマネージメントデータ */
    memcpy(&send_form_1814->b96_key_management_data,
           &rcv_1804_ffd->b96_key_management_data,
           sizeof(send_form_1814->b96_key_management_data));

    /* アクションコード設定 */
    if (NWM_KYX_arg_5->err_code[0] == '0'){
        /* 正常応答 */
        memcpy(send_form_1814->b39_action_code.ffd_data,
               DEF_NWM_ERR_800,
               sizeof(send_form_1814->b39_action_code.ffd_data));
    }
    else {
        /* 拒否応答 */
        if (memcmp(NWM_KYX_arg_5->err_code,
                   DEF_NERR_HSMK_STATION_ST_ERR,
                   sizeof(NWM_KYX_arg_5->err_code)) == 0){
            /* 局状態チェックエラー */
            memcpy(send_form_1814->b39_action_code.ffd_data,
                   DEF_NWM_ERR_910,
                   sizeof(send_form_1814->b39_action_code.ffd_data));
        }
        else if ((memcmp(NWM_KYX_arg_5->err_code,
                   DEF_NERR_CHK_DIGIT_ERR,
                   sizeof(NWM_KYX_arg_5->err_code)) == 0) ||
                 (memcmp(NWM_KYX_arg_5->err_code,
                   DEF_NERR_ATALLA_RSP_ERR,
                   sizeof(NWM_KYX_arg_5->err_code)) == 0) ||
                 (memcmp(NWM_KYX_arg_5->err_code,
                   DEF_NERR_HSMK_DECODE_ERR,
                   sizeof(NWM_KYX_arg_5->err_code)) == 0)){
            /* チェックディジットチェックエラー */
            /* Atalla振分結果エラー */
            if (memcmp(NWM_KYX_arg_5->key_kind,
                       DEF_NWM_KYX_KIND_KMAC,
                       sizeof(NWM_KYX_arg_5->key_kind)) == 0){
               /* KMAC */
               memcpy(send_form_1814->b39_action_code.ffd_data,
                      DEF_NWM_ERR_917,
                      sizeof(send_form_1814->b39_action_code.ffd_data));
            }
            else {
               /* KC / KPE */
               memcpy(send_form_1814->b39_action_code.ffd_data,
                      DEF_NWM_ERR_919,
                      sizeof(send_form_1814->b39_action_code.ffd_data));
            }
        }
        else {
            /* フォーマットエラー */
            memcpy(send_form_1814->b39_action_code.ffd_data,
                   DEF_NWM_ERR_904,
                   sizeof(send_form_1814->b39_action_code.ffd_data));
        }
    }

    send_form_1814->b39_action_code.ffd_header.m_flg_exist = true;
    send_form_1814->b39_action_code.ffd_header.m_fixvalue_length =
                      sizeof(send_form_1814->b39_action_code.ffd_data);
    send_form_1814->b39_action_code.filler = DEF_NWM_KYX_SPACE;

    NWM_KYX_arg_6->message_leng = sizeof(MSG_HEADER_CARDNET_def) + 4 + sizeof(fixedform_cardnet_1814_def);

    return DEF_NWM_KYX_RTN_OK;

}
/* end of NWM_KYX_msg_edit_rsp */


/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_cst_check_req                          */
/*  CALLING SEQ.    : short NWM_KYX_cst_check_req(char *station_st)         */
/*  ARGUMENT        :  1.station_st   (I) 局状態                            */
/*  RETURN CODE     : 0：精査OK                                             */
/*                  : 1：精査エラー(拒否応答)                               */
/*                  : 3：精査エラー(電文破棄)                               */
/*                  : 9：精査エラー(異常)                                   */
/*  DESCRIPTION     : 鍵交換局状態チェック(要求受信)                        */
/****************************************************************************/
short NWM_KYX_cst_check_req(char *station_st)
{
    /* 局状態確認 */
    if (memcmp(station_st,
               DEF_STTE_STS_OPN,
               sizeof(DEF_STTE_STS_OPN)) != 0){
        /* 開局以外は拒否応答を返却する */
        return DEF_NWM_KYX_RTN_NG_REJ;
    }

    return DEF_NWM_KYX_RTN_OK;

}
/* end of NWM_KYX_cst_check_req */


/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_cst_check_rsp                          */
/*  CALLING SEQ.    : short NWM_KYX_cst_check_rsp(                          */
/*                                      short      key_data_kind            */
/*                                      char       *station_st)             */
/*  ARGUMENT        : 1.station_st    (I) 局状態                            */
/*  ARGUMENT        : 2.key_data_kind (I) 要求種別                          */
/*  RETURN CODE     : 0：精査OK                                             */
/*                  : 1：精査エラー(拒否応答)                               */
/*                  : 3：精査エラー(電文破棄)                               */
/*                  : 9：精査エラー(異常)                                   */
/*  DESCRIPTION     : 鍵交換局状態チェック(応答受信)                        */
/****************************************************************************/
short NWM_KYX_cst_check_rsp(short   key_data_kind
                          , char    *station_st)
{
    /* CARDNETでは動作しないためダミー関数 */
    return DEF_NWM_KYX_RTN_ERR;
}
/* end of NWM_KYX_cst_check_rsp */


/****************************************************************************/
/*  FUNCTION        : 4.0.0  NWM_KYX_check_datetime                         */
/*  CALLING SEQ.    : short NWM_KYX_check_datetime(char *date_time)         */
/*  ARGUMENT        : 1.day_time  (I)日時(文字列)  YYYYMMDDhhmmss           */
/*  RETURN CODE     : 0:正常 -1:異常                                        */
/*  DESCRIPTION     : 日付形式チェック                                      */
/****************************************************************************/
short NWM_KYX_check_datetime(char  *date_time)
{
    short  ts[8];
    short  get_ts[16];
    char   day_time_save[4+1];
    short  errmask      = 0;
    short  ts_cnt       = 0;
    short  day_cnt      = 4;
    short  loop_cnt     = 0;
    long long timestamp = 0;

    memset(ts,            0, sizeof(ts));
    memset(day_time_save, 0, sizeof(day_time_save));
    memset(get_ts,        0, sizeof(get_ts));

    memcpy(day_time_save, date_time, 4);
    ts[ts_cnt] = (short)atoi(day_time_save);
    memset(day_time_save, 0, sizeof(day_time_save));

    for(loop_cnt=1; loop_cnt<6 ; loop_cnt++){
        memcpy(day_time_save, &date_time[day_cnt], 2);
        ts[loop_cnt] = (short)atoi(day_time_save);
        day_cnt+=2;
    }

    /* YYYYが0の場合はYYYYを設定する */
    if (ts[0] == 0){
        TIME(get_ts);
        ts[0] = get_ts[0];
    }
 
    /* 日時チェック */
    timestamp = COMPUTETIMESTAMP(ts,&errmask);
    if ((errmask != DEF_NWM_KYX_RTN_OK) ||
        (timestamp == 0 )) {
        /* 異常終了 */
        return DEF_NWM_KYX_RTN_NG;
    }

    return DEF_NWM_KYX_RTN_OK;

} /* end of NWM_KYX_check_datetime */

/*********************************************************************************/
/*  FUNCTION        : 4.0.0  NWM_KYX_read_cutfile                                */
/*  CALLING SEQ.    : short NWM_KYX_read_cutfile(file_info_gccut *gccut_info,    */
/*                                               gflin_pkey_def  *connection_lid,*/
/*                                               db_gccut_def    *set_db,        */
/*                                               oggz1in_def     *ems_cmn,       */
/*                                               NWM_KYX_ems_add *ems_add)       */
/*  ARGUMENT        : 1.gccut_info     (I)キー情報                               */
/*                  : 2.connection_lid (I)センターID                             */
/*                  : 2.set_db         (O)DB設定領域                             */
/*  RETURN CODE     : 0:正常 -1:異常                                             */
/*  DESCRIPTION     : カット対象日付ファイル読み込み処理                         */
/*********************************************************************************/
short NWM_KYX_read_cutfile(file_info_gccut    *gccut_info,
                           gflin_pkey_def     *connection_lid,
                           db_gccut_def       *set_db,
                           oggz1in_def        *ems_cmn,
                           NWM_KYX_ems_add    *ems_add)
{
    short           s_result;
    db_gccut_def    *db_gccut;
    char            filename[48+1];
    char            ch_sub_prog_sts[2];
    short           cnt = 0;

    COM_IOM_arg_3_def        com_iom_arg_3;    /* I/Oモジュール arg 3          */
    COM_IOM_arg_4_def        com_iom_arg_4;    /* I/Oモジュール arg 4          */
    COM_IOM_arg_5_def        com_iom_arg_5;    /* I/Oモジュール arg 5          */
    COM_IOM_arg_6_def        com_iom_arg_6;    /* I/Oモジュール arg 6          */

    db_gccut = (db_gccut_def *)&com_iom_arg_6.rec_area;

    /* 共通I/Oモジュール情報初期化 */
    memset(ch_sub_prog_sts, 0x00, sizeof(ch_sub_prog_sts));
    memset(&com_iom_arg_3 , 0x00, sizeof(com_iom_arg_3));
    memset(&com_iom_arg_4 , 0x00, sizeof(com_iom_arg_4));
    memset(&com_iom_arg_5 , 0x00, sizeof(com_iom_arg_5));
    memset(&com_iom_arg_6 , 0x00, sizeof(com_iom_arg_6));

    /* トレース情報 */
    memcpy(com_iom_arg_3.file_id, DEF_GCCUT, strlen(DEF_GCCUT));
    memcpy(com_iom_arg_3.file_name, gccut_info->file_name, sizeof(com_iom_arg_3.file_name));
    memcpy(com_iom_arg_3.file_io_type, DEF_NWM_FILEIO_TYPE_READ, strlen(DEF_NWM_FILEIO_TYPE_READ));

    /* ファイル情報 */
    memcpy(com_iom_arg_4.file_id, gccut_info->file_id, strlen(gccut_info->file_id));
    memcpy(com_iom_arg_4.file_name, gccut_info->file_name, sizeof(com_iom_arg_4.file_name));
    com_iom_arg_4.file_no = gccut_info->file_no;

    /* 入力情報 */
    com_iom_arg_5.part_key_type     = DEF_COM_IOM_PARTITION_KEY_NOT;
    com_iom_arg_5.part_key_position = 0;
    com_iom_arg_5.part_key_len      = 0;
    memcpy(com_iom_arg_5.key_value  , (char *)&db_gccut->pri_key, sizeof(db_gccut->pri_key));
    memcpy(com_iom_arg_5.key_type   , DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    com_iom_arg_5.key_len           = sizeof(db_gccut->pri_key);
    com_iom_arg_5.compare_len       = sizeof(db_gccut->pri_key);
    com_iom_arg_5.positioning_mode  = DEF_COM_IOM_EXACT;
    com_iom_arg_5.lock_flg          = DEF_COM_IOM_NOLOCK;
    com_iom_arg_5.asc_desc_type     = DEF_COM_IOM_ASCEND;
    com_iom_arg_5.io_timer          = gccut_info->io_timer;
    com_iom_arg_5.rec_len           = db_gccut_def_Size;

    /* 検索キー */
    memcpy((char*)&db_gccut->pri_key, (char*)connection_lid, sizeof(db_gccut->pri_key));
    memset(db_gccut->pri_key.connect_id, DEF_NWM_KYX_SPACE, sizeof(db_gccut->pri_key.connect_id));

    memcpy(com_iom_arg_5.key_value, (char *)&db_gccut->pri_key, sizeof(db_gccut->pri_key));

    /* 共通I/Oモジュール */
    s_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                       , ch_sub_prog_sts
                       , &com_iom_arg_3
                       , &com_iom_arg_4
                       , &com_iom_arg_5
                       , &com_iom_arg_6);

    if (s_result != DEF_NWM_KYX_RTN_OK) {
        memset(filename, 0, sizeof(filename));
        memcpy(filename, gccut_info->file_name, sizeof(gccut_info->file_name));
        for(cnt=0; cnt<48; cnt++){
            if (filename[cnt] == DEF_NWM_KYX_SPACE){
                filename[cnt] = 0;
                break;
            }
        }
        NWM_KYX_ems_output(DEF_COM_IOM_FUNC_STARTREAD,
                           filename,
                           &com_iom_arg_5,
                           &com_iom_arg_6,
                           connection_lid,
                           ems_cmn,
                           ems_add);

        return DEF_NWM_KYX_RTN_NG;
    }

    /* レコード情報取得 */
    memcpy((char *)set_db, (char *)db_gccut, db_gccut_def_Size);

    return DEF_NWM_KYX_RTN_OK;

} /* NWM_KYX_read_cutfile */

/****************************************************************************/
/*  FUNCTION        : 2.3.3  NWM_KYX_ems_output                             */
/*  CALLING SEQ.    : void NWM_KYX_ems_output (char *,                      */
/*                                      char *,                             */
/*                                      char *,                             */
/*                                      COM_IOM_arg_5_def *,                */
/*                                      COM_IOM_arg_6_def *,                */
/*                                      COM_EMS_arg_1_def *)                */
/*  ARGUMENT        : 1.func_type      (I)   アクション                     */
/*                  : 2.filename       (I)   ファイル情報                   */
/*                  : 3.cnt_id         (I)   センターID                     */
/*                  : 4.in_fl_inf      (I)   入力情報                       */
/*                  : 5.out_fl_inf     (O)   ATALLAレスポンステーブル       */
/*                  : 6.ems_info       (I)   EMS出力情報                    */
/*  RETURN CODE     : 無し                                                  */
/*  DESCRIPTION     : ファイルI/OエラーのEMS出力を行う                      */
/****************************************************************************/
void  NWM_KYX_ems_output(
    char              *func_type,
    char              *filename,
    COM_IOM_arg_5_def *in_fl_inf,
    COM_IOM_arg_6_def *out_fl_inf,
    gflin_pkey_def    *connection_lid,
    oggz1in_def       *ems_cmn,
    NWM_KYX_ems_add   *ems_add)
{
    oggz1in_def               ems;            /* EMS出力構造体                */
    char text[256];

    memcpy((char*)&ems,(char*)ems_cmn, sizeof(oggz1in_def));

    // EMS情報設定
    memset(&ems, 0x20, sizeof(ems));     //領域初期化
    // 運用監視端末出力サーバI/Oエラー
    ems.subrcd      = '1';

    // EMS出力情報・メッセージID
    snprintf(text, sizeof(text), "%05.05d", DEF_EVT_FILE_IO_ERR);
    memcpy(ems.emsinf.msgid, text, strlen(text));

    // 任意メッセージ・メッセージ要求元の任意パラメータI  ・サーバークラス論理ID (8バイト)
    memcpy(ems.emsinf.emsnninf.msgtbl[0].msgtbl_vl, ems_add->srv_logical_id, sizeof(ems_add->srv_logical_id));

    // 任意メッセージ・メッセージ要求元の任意パラメータII ・LCN (15バイト)
    memcpy(ems.emsinf.emsnninf.msgtbl[1].msgtbl_vl, ems_add->lcn, sizeof(ems_add->lcn));

    // 任意メッセージ・メッセージ要求元の任意パラメータIII・センターID (19バイト)
    memcpy(ems.emsinf.emsnninf.msgtbl[2].msgtbl_vl, connection_lid, sizeof(connection_lid));

    // 任意メッセージ・メッセージ要求元の任意パラメータIV・アクション (19バイト)
    memcpy(ems.emsinf.emsnninf.msgtbl[3].msgtbl_vl, func_type, strlen(func_type));

    // 任意メッセージ・メッセージ要求元の任意パラメータV ・キー (40バイト)
    memcpy(ems.emsinf.emsnninf.msgtbl[4].msgtbl_vl, in_fl_inf->key_value, in_fl_inf->part_key_len);

    // 任意メッセージ・メッセージ要求元の任意パラメータVI・キー エラーコード (5バイト)
    snprintf(text, sizeof(text), "%05.05d", out_fl_inf->guardian_errcode);
    memcpy(ems.emsinf.emsnninf.msgtbl[5].msgtbl_vl, text, strlen(text));

    /* EMS出力モジュールの呼び出し */
    GFPOGGZ1(&ems);

    return;
}/* end of NWM_KYX_ems_output */

/****************************************************************************/
/*  FUNCTION        : 2.3.4  NWM_ENC_SHORT2BCD                               */
/*  CALLING SEQ.    : short NWM_ENC_CHAR2BCD       (unsigned char  *        */
/*                                                 ,short          *)       */
/*  ARGUMENT        : 1.ascii_p        (I)   変換元バッファ                 */
/*                  : 2.len            (O)   変換した値                     */
/*  RETURN CODE     : 0 :正常                                               */
/*                    1 :異常                                               */
/*  DESCRIPTION     : 10進数字->BCD変換を行う                               */
/****************************************************************************/
short NWM_KYX_SHORT2BCD(unsigned short short_data, char *bcd_data)
{
    unsigned short  bcd             = 0;
    unsigned short  short_data_save = short_data;
    int             shift           = 0;

    while(short_data_save > 0){
       /* 4ビットシフトそて、下位ビットに10進数の1桁を格納 */
       bcd |= (short_data_save % 10) << (shift * 4);
       short_data_save /= 10;
       shift++;
    }
    memcpy(bcd_data, (char *)&bcd, sizeof(bcd));

    return DEF_NWM_KYX_RTN_OK;
} /* end of NWM_KYX_SHORT2BCD */

/****************************************************************************/
/*  FUNCTION        : 2.3.4  NWM_KYX_CHAR2BCD                               */
/*  CALLING SEQ.    : short NWM_KYX_CHAR2BCD       (unsigned char  *        */
/*                                                 ,char           *        */
/*                                                 ,short          *)       */
/*  ARGUMENT        : 1.ascii_p        (I)   変換元バッファ                 */
/*                  : 2.bcd_p          (O)   変換した値                     */
/*                  : 3.len            (O)   変換するデータ長               */
/*  RETURN CODE     : 0 :正常                                               */
/*                    1 :異常                                               */
/*  DESCRIPTION     : 10進文字->BCD変換を行う                               */
/****************************************************************************/
short NWM_KYX_CHAR2BCD(unsigned char *ascii_p, char *bcd_p, short len)
{
    short sValue   = 0;
    short sbcd_cnt = 0;

    for (sValue=0; sValue<len; sValue++) {
        if (ascii_p[sValue] < '0' || ascii_p[sValue] > '9') {
            return DEF_NWM_KYX_RTN_NG;
        }
        else {
            if ((sValue % 2) == 1){
                bcd_p[sbcd_cnt] = (char)((ascii_p[sValue-1] & 0x0f) <<  4 |
                                         (ascii_p[sValue]   & 0x0f));
                sbcd_cnt++;

            }
        }
    }

   return DEF_NWM_KYX_RTN_OK;
} /* end of NWM_KYX_CHAR2BCD */

/****************************************************************************/
/*  FUNCTION        : 2.3.5  NWM_KYX_CHAR2HEX                               */
/*  CALLING SEQ.    : short NWM_KYX_CHAR2HEX       (const char     *        */
/*                                                 ,char           *        */
/*                                                 ,short           )       */
/*  ARGUMENT        : 1.ascii_p        (I)   変換元バッファ                 */
/*                  : 2.hex_p          (O)   変換先バッファ                 */
/*                  : 3.s_len          (I)   変換長                         */
/*  RETURN CODE     : 0 :正常                                               */
/*                    1 :異常                                               */
/*  DESCRIPTION     : 16進文字->BINARY変換を行う                            */
/****************************************************************************/
short NWM_KYX_CHAR2HEX(const char *ascii_p, char *hex_p, short s_len)
{
    short var,base,half,iix,oix;

    base = (short)(s_len & 0x01);
    for (iix = 0,oix = 0,var = 0;iix < s_len;iix++) {

        if        (ascii_p[iix] >= '0' && ascii_p[iix] <= '9') {
           half = ascii_p[iix] - '0';
        } else if (ascii_p[iix] >= 'A' && ascii_p[iix] <= 'F') {
           half = ascii_p[iix] - 'A' + 10;
        } else if (ascii_p[iix] >= 'a' && ascii_p[iix] <= 'f') {
           half = ascii_p[iix] - 'a' + 10;
        } else {
          memset(hex_p,0x20,s_len/2);
          return DEF_NWM_KYX_RTN_NG;
        }
        if (base == 0) {
            var = half;
            base = 1;
        } else {
            var = (short)((var << 4) + half);
            hex_p[oix] = (char)var;
            oix++;
            base = 0;
        }
    }
    return DEF_NWM_KYX_RTN_OK;

} /* end of NWM_KYX_CHAR2HEX */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  NWM_KYX_BCD2CHAR                               */
/*  CALLING SEQ.    : void MDSI_BCD2CHAR ( unsigned char *, char*, short)   */
/*  ARGUMENT        : 1. bcd_p       (I) BCDデータ                          */
/*                  : 2. bcd_p       (O) ASCIIデータ                        */
/*  RETURN CODE     : なし                                                  */
/*  DESCRIPTION     : BSD⇒CHAR変換                                         */
/****************************************************************************/
void NWM_KYX_BCD2CHAR(unsigned char *bcd_p, char *ascii_p,short s_len)
{
    short s_count;
    const char ToNUM_tbl[16] = {"0123456789******"};    /* ニューメリック変換テーブル */

    for (s_count = 0; s_count < s_len; s_count++) {
        ascii_p[s_count*2]   = ToNUM_tbl[bcd_p[s_count] >> 4];
        ascii_p[s_count*2+1] = ToNUM_tbl[bcd_p[s_count] & 0x0f];
    }
} /* end of NWM_KYX_BCD2CHAR */


