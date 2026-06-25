/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSU80                                    */
/*        FUNCTION          ････ NW個別(鍵交換個別処理[UnionPay])            */
/*                               鍵交換個別処理[UnionPay])を行う。           */
/*                                                                           */
/*        AUTHER            ････ HAS                                         */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-09-10                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS        2025/09/10 新規作成                                      */
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
#include "GFPCVXZ0.h"                        /* 制御電文共通メイン処理ヘッダ */
#include "msg_UP.h"
#include "NWM_KYX.h"
#include "GFPCSU80.h"

/******************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_msg_check_req                            */
/*  CALLING SEQ.    : short NWM_KYX_msg_check_req(                            */
/*                                      char              *req_message        */
/*                                    , char              *p_rcv_data_len     */
/*                                    , db_gfnwi_def      *nwi_g              */
/*                                    , db_gfnwi_def      *nwi_i              */
/*                                    , char              *nws_n              */
/*                                    , char              *nws_i              */
/*                                    , char              *nws_s              */
/*                                    , char              *nws_c              */
/*                                    , db_gckey_def      *gckey              */
/*                                    , NWM_KYX_arg_1_def *NWM_KYX_arg_1)     */
/*  ARGUMENT        :  1.req_message    (I) 受信データ(要求電文)              */
/*                  :  2.p_rcv_data_len (I) 受信電文長                        */
/*                  :  3.nwi_g          (I) NW情報レコード(グループ単位)      */
/*                  :  4.nwi_i          (I) NW情報レコード(インタフェース単位)*/
/*                  :  5.nws_n          (I) 接続先固有情報(NW単位)            */
/*                  :  6.nws_i          (I) 接続先固有情報(インタフェース単位)*/
/*                  :  7.nws_s          (I) 接続先固有情報(ステーション単位)  */
/*                  :  8.nws_c          (I) 接続先固有情報(コネクション単位)  */
/*                  :  9.gckey          (I) 鍵管理情報レコード                */
/*                  : 10.NWM_KYX_arg_1  (O) 精査処理結果                      */
/*  RETURN CODE     : 0：精査OK                                               */
/*                  : 1：精査エラー(拒否応答)                                 */
/*                  : 3：精査エラー(電文破棄)                                 */
/*                  : 9：精査エラー(異常)                                     */
/*  DESCRIPTION     : 鍵交換電文精査(要求受信)                                */
/******************************************************************************/
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
    char   lc_wbuf[256];
    msg_unionpay_def             *rcv_msg_ctl;
    fixedform_unionpay_0800_def  *rcv_0800_msg;
    nwi_unq_info_up_def          *lp_nwi_unq_info; /* NW情報固有情報              */

    /* BIT48(KB)の型宣言 */
    typedef struct  __b048_sub_def
    {
        char   usage_identifier[2];
        char   key[144];
    } b048_sub_def;

    b048_sub_def                *b048_sub_tbl;
    lp_nwi_unq_info   = (nwi_unq_info_up_def*)nwi_i->dst_unq_info;

    /* ヘッダ部確認 */
    rcv_msg_ctl = (msg_unionpay_def*)req_message;

    /* ヘッダー部電文長                      */
    if (rcv_msg_ctl->header.mh_hdr_len != MSG_HEADER_UNIONPAY_def_Size){
        /* エラー発生ヘッダ番号設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_HEADER_HEAD_LEN_ERR,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    /* Total Message Length精査 */
    if (memcmp(p_rcv_data_len,
               rcv_msg_ctl->header.mh_tot_len,
               sizeof(rcv_msg_ctl->header.mh_tot_len)) != 0) {
        /* エラー発生ヘッダ番号設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_HEADER_TTL_LEN_ERR,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    /* モードフラグ → 設定値check */
    if ( lp_nwi_unq_info->mode_flg == DEF_MODE_FLG_OFF  ) {
        if( rcv_msg_ctl->header.mh_hdr_flg_ver != DEF_UN_MODE_01_HONBAN ){
            memcpy( NWM_KYX_arg_1->err_bit,
                    DEF_NWM_KYX_HEADER_MOD_FLG_ERR,
                    strlen(DEF_NWM_KYX_HEADER_MOD_FLG_ERR));
            return DEF_NWM_KYX_RTN_HAKI_MSG;
        }
    } else
    if ( lp_nwi_unq_info->mode_flg == DEF_MODE_FLG_ON  ) {
        if( rcv_msg_ctl->header.mh_hdr_flg_ver != DEF_UN_MODE_81_TEST ){
            memcpy( NWM_KYX_arg_1->err_bit,
                    DEF_NWM_KYX_HEADER_MOD_FLG_ERR,
                    strlen(DEF_NWM_KYX_HEADER_MOD_FLG_ERR));
            return DEF_NWM_KYX_RTN_HAKI_MSG;
        }
    }

    /* データ部確認 */
    /* MTIチェック */
    if (memcmp(rcv_msg_ctl->mti,
               DEF_UP_MTI_0800_KEYREQ,
               sizeof(rcv_msg_ctl->mti)) != 0){
        /* エラー発生ヘッダ番号設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_HEADER_MTI_ERR,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    rcv_0800_msg = (fixedform_unionpay_0800_def*)&rcv_msg_ctl->ffd;

    /* KEY長を0クリア */
    NWM_KYX_arg_1->key_leng = 0;

    /* --- BIT007 ----------------- */
    if (rcv_0800_msg->b007_trans_date_time.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_007,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    if (rcv_0800_msg->b007_trans_date_time.ffd_header.m_fixvalue_length !=
                                 DEF_NWM_KYX_BIT_007_LENG){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_007,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    memset ( lc_wbuf , DEF_BUF_NULL, sizeof(lc_wbuf));
    memset ( lc_wbuf , DEF_NWM_KYX_ZERO, 4);
    memcpy (&lc_wbuf[4], rcv_0800_msg->b007_trans_date_time.ffd_data, DEF_NWM_KYX_BIT_007_LENG);
    ls_result = CMIN_check_datetime ( lc_wbuf );
    if ( ls_result != DEF_NWM_KYX_RTN_OK ) {

        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_007,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    /* --- BIT011 ----------------- */
    if (rcv_0800_msg->b011_system_audit_number.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_011,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    if (rcv_0800_msg->b011_system_audit_number.ffd_header.m_fixvalue_length !=
                                 DEF_NWM_KYX_BIT_011_LENG){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_011,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    memset( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
    memcpy( lc_wbuf, rcv_0800_msg->b011_system_audit_number.ffd_data
                   , rcv_0800_msg->b011_system_audit_number.ffd_header.m_fixvalue_length);
    ls_result = CMIN_num_check ( lc_wbuf );
    if ( ls_result != DEF_NWM_KYX_RTN_OK ) {
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_011,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    /* --- BIT048 ----------------- */
    if (rcv_0800_msg->b048_add_data_private.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_048,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    if (rcv_0800_msg->b048_add_data_private.ffd_header.m_fixvalue_length < DEF_NWM_KYX_BIT_048_LENG) {
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_048,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    b048_sub_tbl = (b048_sub_def *)rcv_0800_msg->b048_add_data_private.ffd_data;

    if (memcmp(b048_sub_tbl->usage_identifier,
               DEF_NWM_KYX_BIT_048_USAGEID_KB,
               sizeof(b048_sub_tbl->usage_identifier)) != 0) {
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_048,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    /* KEY設定 */
    memcpy(NWM_KYX_arg_1->key
          , b048_sub_tbl->key
          , (rcv_0800_msg->b048_add_data_private.ffd_header.m_fixvalue_length - 2));
    NWM_KYX_arg_1->key_leng = (short)rcv_0800_msg->b048_add_data_private.ffd_header.m_fixvalue_length - 2;

    /* --- BIT053 ----------------- */
    if (rcv_0800_msg->b053_secur_ctl_info.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_053,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    if (rcv_0800_msg->b053_secur_ctl_info.ffd_header.m_fixvalue_length !=
                                 DEF_NWM_KYX_BIT_053_LENG){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_053,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    /* --- BIT070 ----------------- */
    if (rcv_0800_msg->b070_nw_mng_code.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_070,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    if (rcv_0800_msg->b070_nw_mng_code.ffd_header.m_fixvalue_length !=
                                 DEF_NWM_KYX_BIT_070_LENG){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_070,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    if (memcmp(&rcv_0800_msg->b070_nw_mng_code.ffd_data[0],
               DEF_NWM_KYX_BIT_070_101,
               DEF_NWM_KYX_BIT_070_KYETYPE_LENG) != 0) {
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_070,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    /* --- BIT096 ----------------- */
    if (rcv_0800_msg->b096_msg_security_code.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_096,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    if (rcv_0800_msg->b096_msg_security_code.ffd_header.m_fixvalue_length !=
                                 DEF_NWM_KYX_BIT_096_LENG){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_096,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    /* --- BIT100 ----------------- */
    if (rcv_0800_msg->b100_recv_inst_id_code.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_100,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    if (rcv_0800_msg->b100_recv_inst_id_code.ffd_header.m_fixvalue_length >
                                 DEF_NWM_KYX_BIT_100_LENG){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_100,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    /* --- BIT128 ----------------- */
    if (rcv_0800_msg->b128_msg_auth_code.ffd_header.m_flg_exist == true){
        if (rcv_0800_msg->b128_msg_auth_code.ffd_header.m_fixvalue_length !=
                                     DEF_NWM_KYX_BIT_128_LENG){
            /* エラー発生BIT設定 */
            memcpy(NWM_KYX_arg_1->err_bit,
                   DEF_NWM_KYX_BIT_128,
                   sizeof(NWM_KYX_arg_1->err_bit));
            /* 電文破棄 */
            return DEF_NWM_KYX_RTN_HAKI_MSG;
        }
    }

    /* チェックディジット長に0を設定 */
    NWM_KYX_arg_1->checkdigit_leng = 0;

    /* KEY種別 */
    memcpy(NWM_KYX_arg_1->key_kind,
           DEF_NWM_KYX_KIND_KPE,
           sizeof(NWM_KYX_arg_1->key_kind) );

    return DEF_NWM_KYX_RTN_OK;
}
/* end of NWM_KYX_msg_check_req */

/******************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_msg_check_rsp                            */
/*  CALLING SEQ.    : short NWM_KYX_msg_check_req(                            */
/*                                      short             key_data_kind       */
/*                                    , char              *req_message        */
/*                                    , char              *p_rcv_data_len     */
/*                                    , db_gfnwi_def      *nwi_g              */
/*                                    , db_gfnwi_def      *nwi_i              */
/*                                    , char              *nws_n              */
/*                                    , char              *nws_i              */
/*                                    , char              *nws_s              */
/*                                    , char              *nws_c              */
/*                                    , db_gckey_def      *gckey              */
/*                                    , NWM_KYX_arg_2_def *NWM_KYX_arg_2)     */
/*  ARGUMENT        :  1.key_data_kind  (I) 要求種別                          */
/*                  :  2.req_message    (I) 受信データ(応答電文)              */
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
    msg_unionpay_def            *rcv_msg_ctl;
    fixedform_unionpay_0830_def *rcv_0830_msg;
    char                         lc_wbuf[30];
    short                        ls_result      = 0;

    short                        wk_src_id_size = 0;
    char                        *wk_src_id_ptr;

    short                        nws_cnt        = 0;

    nwi_unq_info_up_def          *lp_nwi_unq_info; /* NW情報固有情報              */

    rcv_msg_ctl = (msg_unionpay_def*)req_message;

    lp_nwi_unq_info   = (nwi_unq_info_up_def*)nwi_i->dst_unq_info;

    /* ヘッダ部確認 */
    /* ヘッダー部電文長                      */
    if (rcv_msg_ctl->header.mh_hdr_len != MSG_HEADER_UNIONPAY_def_Size){
        /* エラー発生ヘッダ番号設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_HEADER_HEAD_LEN_ERR,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    /* Total Message Length精査 */
    if (memcmp(p_rcv_data_len,
               rcv_msg_ctl->header.mh_tot_len,
               sizeof(rcv_msg_ctl->header.mh_tot_len)) != 0) {
        /* エラー発生ヘッダ番号設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_HEADER_TTL_LEN_ERR,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    /* モードフラグ → 設定値check */
    if ( lp_nwi_unq_info->mode_flg == DEF_MODE_FLG_OFF  ) {
        if( rcv_msg_ctl->header.mh_hdr_flg_ver != DEF_UN_MODE_01_HONBAN ){
            memcpy( NWM_KYX_arg_2->err_bit,
                    DEF_NWM_KYX_HEADER_MOD_FLG_ERR,
                    strlen(DEF_NWM_KYX_HEADER_MOD_FLG_ERR));
            return DEF_NWM_KYX_RTN_HAKI_MSG;
        }
    } else
    if ( lp_nwi_unq_info->mode_flg == DEF_MODE_FLG_ON  ) {
        if( rcv_msg_ctl->header.mh_hdr_flg_ver != DEF_UN_MODE_81_TEST ){
            memcpy( NWM_KYX_arg_2->err_bit,
                    DEF_NWM_KYX_HEADER_MOD_FLG_ERR,
                    strlen(DEF_NWM_KYX_HEADER_MOD_FLG_ERR));
            return DEF_NWM_KYX_RTN_HAKI_MSG;
        }
    }

    /* データ部確認 */
    /* MTIチェック */
    if (memcmp(rcv_msg_ctl->mti,
               DEF_UP_MTI_0830_RSP,
               sizeof(rcv_msg_ctl->mti)) != 0){
        /* エラー発生ヘッダ番号設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_HEADER_MTI_ERR,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    rcv_0830_msg = (fixedform_unionpay_0830_def*)&rcv_msg_ctl->ffd;

    /* --- BIT007 ----------------- */
    if (rcv_0830_msg->b007_trans_date_time.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_007,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    if (rcv_0830_msg->b007_trans_date_time.ffd_header.m_fixvalue_length !=
                                 DEF_NWM_KYX_BIT_007_LENG){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_007,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    memset ( lc_wbuf , DEF_BUF_NULL, sizeof(lc_wbuf));
    memset ( lc_wbuf , DEF_NWM_KYX_ZERO, 4);
    memcpy (&lc_wbuf[4], rcv_0830_msg->b007_trans_date_time.ffd_data, DEF_NWM_KYX_BIT_007_LENG);
    ls_result = CMIN_check_datetime ( lc_wbuf );
    if ( ls_result != DEF_NWM_KYX_RTN_OK ) {
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_007,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    /* --- BIT011 ----------------- */
    if (rcv_0830_msg->b011_system_audit_number.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_011,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    if (rcv_0830_msg->b011_system_audit_number.ffd_header.m_fixvalue_length !=
                                 DEF_NWM_KYX_BIT_011_LENG){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_011,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    memset( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
    memcpy( lc_wbuf, rcv_0830_msg->b011_system_audit_number.ffd_data
                   , rcv_0830_msg->b011_system_audit_number.ffd_header.m_fixvalue_length);
    ls_result = CMIN_num_check ( lc_wbuf );
    if ( ls_result != DEF_NWM_KYX_RTN_OK ) {
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_011,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    /* --- BIT033 ----------------- */
    if (rcv_0830_msg->b033_fowd_inst_id_code.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_033,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    /* NW情報レコード(グループ単位)の接続先固有情報ファイルレコード登録単位確認 */
    memset(lc_wbuf, DEF_NWM_KYX_SPACE, sizeof(lc_wbuf));

    for(nws_cnt=0; nws_cnt<=3; nws_cnt++){
        if (nwi_g->gfnws_info.rec_unit[nws_cnt] == DEF_NWM_KYX_CHAR_ON){
            /* 接続先固有情報ファイルレコード確認 */
            switch(nws_cnt) {
            /* Source ID */
            case DEF_NWM_KYX_NWS_NW:
                if (nws_n[0] != DEF_NWM_KYX_SPACE){
                    memcpy(lc_wbuf,
                           nws_n,
                           DEF_NWM_KYX_BIT_033_LENG);
                }
                else{
                    continue;
                }
                break;
            case DEF_NWM_KYX_NWS_IF:
                if (nws_i[0] != DEF_NWM_KYX_SPACE){
                    memcpy(lc_wbuf,
                           nws_i,
                           DEF_NWM_KYX_BIT_033_LENG);
                }
                else{
                    continue;
                }
                break;
            case DEF_NWM_KYX_NWS_ST:
                if (nws_s[0] != DEF_NWM_KYX_SPACE){
                    memcpy(lc_wbuf,
                           nws_s,
                           DEF_NWM_KYX_BIT_033_LENG);
                }
                else{
                    continue;
                }
                break;
            case DEF_NWM_KYX_NWS_CN:
                if (nws_c[0] != DEF_NWM_KYX_SPACE){
                    memcpy(lc_wbuf,
                           nws_c,
                           DEF_NWM_KYX_BIT_033_LENG);
                }
                else{
                    continue;
                }
                break;
            }
            if (lc_wbuf[0] != DEF_NWM_KYX_SPACE){
                /* データ取得完了 */
                break;
            }
        }
    }

    /* データサイズ確認 */
    wk_src_id_ptr = strchr(lc_wbuf, DEF_NWM_KYX_SPACE);
    wk_src_id_size = (short)((unsigned long)wk_src_id_ptr - (unsigned long)lc_wbuf);

    if ( wk_src_id_size != rcv_0830_msg->b033_fowd_inst_id_code.ffd_header.m_fixvalue_length ) {
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_033,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    /* 仕向送信データと同値であることを確認 */
    if (memcmp(rcv_0830_msg->b033_fowd_inst_id_code.ffd_data,
               lc_wbuf,
               wk_src_id_size) != 0){
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_033,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    /* --- BIT039 ----------------- */
    if (rcv_0830_msg->b039_response_code.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_039,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    if (rcv_0830_msg->b039_response_code.ffd_header.m_fixvalue_length !=
                                 DEF_NWM_KYX_BIT_039_LENG){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_039,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    /* 応答種別確認 (00：正常) */
    if (memcmp(rcv_0830_msg->b039_response_code.ffd_data,
               DEF_NWM_ACTION_CODE_OK,
               2) != 0) {
        /* 拒否応答 */
        return DEF_NWM_KYX_RTN_NG_REJ;
    }

    /* --- BIT053 ----------------- */
    if (rcv_0830_msg->b053_secur_ctl_info.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_053,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    if (rcv_0830_msg->b053_secur_ctl_info.ffd_header.m_fixvalue_length !=
                                 DEF_NWM_KYX_BIT_053_LENG){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_053,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    /* --- BIT070 ----------------- */
    if (rcv_0830_msg->b070_nw_mng_code.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_070,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    if (rcv_0830_msg->b070_nw_mng_code.ffd_header.m_fixvalue_length !=
                                 DEF_NWM_KYX_BIT_070_LENG){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_070,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    memcpy(NWM_KYX_arg_2->key_kind,
           DEF_NWM_KYX_KIND_KPE,
           sizeof(NWM_KYX_arg_2->key_kind) );

    /* KEY長に0を設定 */
    NWM_KYX_arg_2->key_leng = 0;
    /* チェックディジット長に0を設定 */
    NWM_KYX_arg_2->checkdigit_leng = 0;

    return DEF_NWM_KYX_RTN_OK;
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
    msg_unionpay_def               *send_0820_msg;
    fixedform_unionpay_0820_def    *send_form_0820;

    COM_SDT_arg_2_def  COM_SDT_arg_2_data;       /* 日時取得用データ        */
    COM_SDT_arg_3_def  COM_SDT_arg_3_data;       /* 日時取得用データ        */
    long long          date_time;                /* 日時取得用データ        */

    short              nws_cnt        = 0;
    short              wk_src_id_size = 0;
    char               wk_src_id[12];
    char              *wk_src_id_ptr;

    nwi_unq_info_up_def          *lp_nwi_unq_info; /* NW情報固有情報              */

    /* BIT53の型宣言 */
    typedef struct  __t_rcv_b053_data
    {
        char   key_type;
        char   key_algorithm;
        char   key_format;
        char   reserved[13];
    } t_rcv_b053_data;

    t_rcv_b053_data   *rcv_b053_data;

    /* 送信バッファ設定 */
    send_0820_msg  = (msg_unionpay_def*)NWM_KYX_arg_4->message;
    send_form_0820 = (fixedform_unionpay_0820_def*)&send_0820_msg->ffd;

    lp_nwi_unq_info   = (nwi_unq_info_up_def*)nwi_i->dst_unq_info;

    /* 鍵交換依頼でなければエラー */
    if (key_data_kind != DEF_NWM_KYX_MSG_KIND_IRAIREQ){
        /* 異常終了 */
        return DEF_NWM_KYX_RTN_ERR;
    }

    /* 送信日時設定 */
    COM_SDT(DEF_COM_SDT_arg1_chn,
            &COM_SDT_arg_2_data,
            &COM_SDT_arg_3_data,
            &date_time);

    /* ヘッダー部設定 */
    /* Header Length設定 */
    send_0820_msg->header.mh_hdr_len = MSG_HEADER_UNIONPAY_def_Size;

    /* Header Flag and Version */
    /* MODE FLAG 設定 */
    if ( lp_nwi_unq_info->mode_flg == DEF_MODE_FLG_OFF  ) {
        send_0820_msg->header.mh_hdr_flg_ver = DEF_UN_MODE_01_HONBAN;
    } else {
        send_0820_msg->header.mh_hdr_flg_ver = DEF_UN_MODE_81_TEST;
    }

    /* Total Message Length */
    memset(send_0820_msg->header.mh_tot_len,
           DEF_NWM_KYX_ZERO,
           sizeof(send_0820_msg->header.mh_tot_len));

    /* Destination ID */
    memset(send_0820_msg->header.mh_dst_id,
           DEF_NWM_KYX_SPACE,
           sizeof(send_0820_msg->header.mh_dst_id));
    memcpy(send_0820_msg->header.mh_dst_id,
           DEF_UN_DST_ID,
           strlen(DEF_UN_DST_ID));

    /* Source ID */
    /* NW情報レコード(グループ単位)の接続先固有情報ファイルレコード登録単位確認 */
    for(nws_cnt=0; nws_cnt<=3; nws_cnt++){
        if (nwi_g->gfnws_info.rec_unit[nws_cnt] == DEF_NWM_KYX_CHAR_ON){
            /* 接続先固有情報ファイルレコード確認 */
            switch(nws_cnt) {
            /* Source ID */
            case DEF_NWM_KYX_NWS_NW:
                if (nws_n[0] != DEF_NWM_KYX_SPACE){
                    memcpy(send_0820_msg->header.mh_src_id,
                           nws_n,
                           sizeof(send_0820_msg->header.mh_src_id));
                }
                else{
                    continue;
                }
                break;
            case DEF_NWM_KYX_NWS_IF:
                if (nws_i[0] != DEF_NWM_KYX_SPACE){
                    memcpy(send_0820_msg->header.mh_src_id,
                           nws_i,
                           sizeof(send_0820_msg->header.mh_src_id));
                }
                else{
                    continue;
                }
                break;
            case DEF_NWM_KYX_NWS_ST:
                if (nws_s[0] != DEF_NWM_KYX_SPACE){
                    memcpy(send_0820_msg->header.mh_src_id,
                           nws_s,
                           sizeof(send_0820_msg->header.mh_src_id));
                }
                else{
                    continue;
                }
                break;
            case DEF_NWM_KYX_NWS_CN:
                if (nws_c[0] != DEF_NWM_KYX_SPACE){
                    memcpy(send_0820_msg->header.mh_src_id,
                           nws_c,
                           sizeof(send_0820_msg->header.mh_src_id));
                }
                else{
                    continue;
                }
                break;
            }
            /* 設定完了確認 */
            if (send_0820_msg->header.mh_src_id[0] != DEF_NWM_KYX_SPACE){
                break;
            }
        }
    }

    /* Reserved for Use */
    memset(send_0820_msg->header.mh_rsv_use,
           DEF_NWM_KYX_NULL,
           sizeof(send_0820_msg->header.mh_rsv_use));

    /* Batch Number */
    send_0820_msg->header.mh_bat_num = DEF_NWM_KYX_NULL;

    /* Transaction Information */
    memcpy(send_0820_msg->header.mh_tran_info,
           DEF_NWM_KYX_TRANS_INF,
           sizeof(send_0820_msg->header.mh_tran_info));

    /* User Information */
    send_0820_msg->header.mh_usr_info = DEF_NWM_KYX_NULL;

    /* Reject Code */
    memset(send_0820_msg->header.mh_rjct_code,
           DEF_NWM_KYX_ZERO,
           sizeof(send_0820_msg->header.mh_rjct_code));

    /* データ部設定 */
    /* MTI設定 */
    memcpy(send_0820_msg->mti,
           DEF_UP_MTI_0820_REQ,
           sizeof(send_0820_msg->mti));

    memcpy(NWM_KYX_arg_4->mti,
           DEF_UP_MTI_0820_REQ,
           sizeof(NWM_KYX_arg_4->mti));

    /* --- BIT007 ----------------- */
    send_form_0820->b007_trans_date_time.ffd_header.m_flg_exist       = true;
    send_form_0820->b007_trans_date_time.ffd_header.m_fixvalue_length = DEF_NWM_KYX_BIT_007_LENG;
    memcpy( send_form_0820->b007_trans_date_time.ffd_data
          ,&COM_SDT_arg_2_data.mm[0]    , DEF_NWM_KYX_BIT_007_LENG);  

    /* --- BIT011 ----------------- */
    send_form_0820->b011_system_audit_number.ffd_header.m_flg_exist       = true;
    send_form_0820->b011_system_audit_number.ffd_header.m_fixvalue_length = DEF_NWM_KYX_BIT_011_LENG;
    memcpy( send_form_0820->b011_system_audit_number.ffd_data
          , NWM_KYX_arg_3->sysytem_no , DEF_NWM_KYX_BIT_011_LENG);

    /* --- BIT033 ----------------- */
    send_form_0820->b033_fowd_inst_id_code.ffd_header.m_flg_exist       = true;
    memset(wk_src_id, DEF_NWM_KYX_SPACE, sizeof(wk_src_id));
    memcpy(wk_src_id,
           send_0820_msg->header.mh_src_id,
           DEF_NWM_KYX_BIT_033_LENG);
    /* データサイズ確認、設定 */
    wk_src_id_ptr = strchr(wk_src_id, DEF_NWM_KYX_SPACE);
    wk_src_id_size = (short)((unsigned long)wk_src_id_ptr - (unsigned long)wk_src_id);
    send_form_0820->b033_fowd_inst_id_code.ffd_header.m_fixvalue_length = wk_src_id_size;
    memcpy( send_form_0820->b033_fowd_inst_id_code.ffd_data,
            send_0820_msg->header.mh_src_id,
            DEF_NWM_KYX_BIT_033_LENG);

    /* --- BIT053 ----------------- */
    send_form_0820->b053_secur_ctl_info.ffd_header.m_flg_exist       = true;
    send_form_0820->b053_secur_ctl_info.ffd_header.m_fixvalue_length = DEF_NWM_KYX_BIT_053_LENG;
    rcv_b053_data = (t_rcv_b053_data*)send_form_0820->b053_secur_ctl_info.ffd_data;

    /* Key Type */
    rcv_b053_data->key_type      = DEF_NWM_KYX_KEY_PIK;

    /* Data Key Algorithm */
    rcv_b053_data->key_algorithm = DEF_NWM_KYX_SPACE;
    if (memcmp(gckey->key_info.key_use_alg,
               DEF_NWM_KYX_ALG_DS1,
               sizeof(gckey->key_info.key_use_alg)) == 0){
        rcv_b053_data->key_algorithm = DEF_NWM_KYX_ALG_DS1_NUM;
    }
    else if (memcmp(gckey->key_info.key_use_alg,
             DEF_NWM_KYX_ALG_DS2,
             sizeof(gckey->key_info.key_use_alg)) == 0){
        rcv_b053_data->key_algorithm = DEF_NWM_KYX_ALG_DS2_NUM;
    }
    else if (memcmp(gckey->key_info.key_use_alg,
             DEF_NWM_KYX_ALG_DS3,
             sizeof(gckey->key_info.key_use_alg)) == 0){
        rcv_b053_data->key_algorithm = DEF_NWM_KYX_ALG_DS3_NUM;
    }

    /* Key Format */
    if (memcmp(gckey->key_info.key_format,
               DEF_NWM_KYX_FMT_KB,
               sizeof(gckey->key_info.key_format)) == 0){
        rcv_b053_data->key_format = DEF_NWM_KYX_FMT_KB_NUM;
    }
    else {
        rcv_b053_data->key_format = DEF_NWM_KYX_FMT_VA_NUM;
    }

    /* Reserved */
    memset(rcv_b053_data->reserved,
           DEF_NWM_KYX_ZERO,
           sizeof(rcv_b053_data->reserved));

    /* --- BIT070 ----------------- */
    send_form_0820->b070_nw_mng_code.ffd_header.m_flg_exist       = true;
    send_form_0820->b070_nw_mng_code.ffd_header.m_fixvalue_length = DEF_NWM_KYX_BIT_070_LENG;
    memcpy( send_form_0820->b070_nw_mng_code.ffd_data
          , DEF_NWM_KYX_BIT_070_101 ,   DEF_NWM_KYX_BIT_070_LENG);

    /* --- BIT100 ----------------- */
    send_form_0820->b100_recv_inst_id_code.ffd_header.m_flg_exist         = false;
    send_form_0820->b100_recv_inst_id_code.ffd_header.m_fixvalue_length   = 0;
    memset( send_form_0820->b100_recv_inst_id_code.ffd_data
          ,DEF_BUF_SPACE    , DEF_NWM_KYX_BIT_100_LENG);

    /* 電文長設定 */
    NWM_KYX_arg_4->message_leng = MSG_HEADER_UNIONPAY_def_Size
                                + 4
                                + sizeof(fixedform_unionpay_0820_def);

    return DEF_NWM_KYX_RTN_OK;

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
/*                  :  9.ems_info_cmn (I) EMS出力共通情報                   */
/*                  : 10.ems_info_add (I) EMS出力付加情報                   */
/*                  : 11.NWM_KYX_arg_5(I) 設定データ                        */
/*                  : 12.NWM_KYX_arg_6(O) 電文情報                          */
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

    msg_unionpay_def               *send_0810_msg;
    msg_unionpay_def               *rcv_0800_msg;
    fixedform_unionpay_0810_def    *send_form_0810;
    fixedform_unionpay_0800_def    *rcv_0800_ffd;

    /* 送信バッファ設定 */
    send_0810_msg  = (msg_unionpay_def*)NWM_KYX_arg_6->message;
    rcv_0800_msg   = (msg_unionpay_def*)NWM_KYX_arg_6->rcv_message;
    send_form_0810 = (fixedform_unionpay_0810_def*)&send_0810_msg->ffd;
    rcv_0800_ffd   = (fixedform_unionpay_0800_def*)&rcv_0800_msg->ffd;

    /* リクエスト電文を初期化する */
    memset(send_form_0810, 0, sizeof(fixedform_unionpay_0810_def));

    /* ヘッダー部設定 */
    /* 要求電文のヘッダー部を応答電文にコピー */
    memcpy( &send_0810_msg->header,
            &rcv_0800_msg->header,
            sizeof(send_0810_msg->header));

    /* Header Length設定 */
    send_0810_msg->header.mh_hdr_len = MSG_HEADER_UNIONPAY_def_Size;

    /* Total Message Length */
    memset(send_0810_msg->header.mh_tot_len,
           DEF_NWM_KYX_ZERO,
           sizeof(send_0810_msg->header.mh_tot_len));

    /* Destination ID */
    memcpy(send_0810_msg->header.mh_dst_id,
           rcv_0800_msg->header.mh_src_id,
           sizeof(send_0810_msg->header.mh_dst_id));

    /* Source ID */
    memcpy(send_0810_msg->header.mh_src_id,
           rcv_0800_msg->header.mh_dst_id,
           sizeof(send_0810_msg->header.mh_src_id));

    /* データ部設定 */
    /* MTI設定 */
    memcpy(send_0810_msg->mti,
           DEF_UP_MTI_0810_KEYRSP,
           sizeof(send_0810_msg->mti));

    memcpy(NWM_KYX_arg_6->mti,
           DEF_UP_MTI_0810_KEYRSP,
           sizeof(NWM_KYX_arg_6->mti));

    /* --- BIT007 ----------------- */
    memcpy(&send_form_0810->b007_trans_date_time,
           &rcv_0800_ffd->b007_trans_date_time,
           sizeof(send_form_0810->b007_trans_date_time));

    /* --- BIT011 ----------------- */
    memcpy(&send_form_0810->b011_system_audit_number,
           &rcv_0800_ffd->b011_system_audit_number,
           sizeof(send_form_0810->b011_system_audit_number));

    /* --- BIT039 ----------------- */
    /* アクションコード設定 */
    if (memcmp(NWM_KYX_arg_5->err_code 
              ,DEF_NERR_NOMAL
              ,sizeof(NWM_KYX_arg_5->err_code))  == 0){
        /* 正常応答 */
        memcpy(send_form_0810->b039_response_code.ffd_data,
               DEF_NWM_ACTION_CODE_OK,
               sizeof(send_form_0810->b039_response_code.ffd_data));
    }
    else {
        /* 拒否応答 */
        memcpy(send_form_0810->b039_response_code.ffd_data,
               DEF_NWM_ACTION_CODE_KYOHI,
               sizeof(send_form_0810->b039_response_code.ffd_data));
    }
    send_form_0810->b039_response_code.ffd_header.m_flg_exist = true;
    send_form_0810->b039_response_code.ffd_header.m_fixvalue_length =
                      sizeof(send_form_0810->b039_response_code.ffd_data);

    /* --- BIT053 ----------------- */
    memcpy(&send_form_0810->b053_secur_ctl_info,
           &rcv_0800_ffd->b053_secur_ctl_info,
           sizeof(send_form_0810->b053_secur_ctl_info));

    /* --- BIT070 ----------------- */
    memcpy(&send_form_0810->b070_nw_mng_code,
           &rcv_0800_ffd->b070_nw_mng_code,
           sizeof(send_form_0810->b070_nw_mng_code));

    /* --- BIT100 ----------------- */
    memcpy(&send_form_0810->b100_recv_inst_id_code,
           &rcv_0800_ffd->b100_recv_inst_id_code,
           sizeof(send_form_0810->b100_recv_inst_id_code));

    /* --- BIT128 ----------------- */
    memcpy(&send_form_0810->b128_msg_auth_code,
           &rcv_0800_ffd->b128_msg_auth_code,
           sizeof(send_form_0810->b128_msg_auth_code));
    send_form_0810->b128_msg_auth_code.ffd_header.m_flg_exist         = false;

    NWM_KYX_arg_6->message_leng = MSG_HEADER_UNIONPAY_def_Size
                                + 4
                                + sizeof(fixedform_unionpay_0810_def);

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
        /* 開局以外は電文破棄を返却する */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
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
    /* UnionPayではチェック不要のため正常を返却する */
    return DEF_NWM_KYX_RTN_OK;
}
/* end of NWM_KYX_cst_check_rsp */


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
/*  FUNCTION        : 2.3.5  NWM_KYX_ASCII2SHORT                            */
/*  CALLING SEQ.    : void  NWM_KYX_ASCII2SHORT       (const char  *        */
/*                                                     ,short      *        */
/*                                                     ,short       )       */
/*  ARGUMENT        : 1.ascii_p        (I)   変換元バッファ                 */
/*                  : 2.short_p        (O)   変換先バッファ                 */
/*                  : 3.s_len          (I)   変換長                         */
/*  RETURN CODE     : なし                                                  */
/*  DESCRIPTION     : 16進文字->BINARY変換を行う                            */
/****************************************************************************/
void NWM_KYX_ASCII2SHORT(unsigned char *ascii_p, short *short_p, short s_len)
{
    short wk_save_data = 0;
    short loop_cnt     = 0;
    short wk_keta      = 1;

    for(loop_cnt=s_len-1; loop_cnt>=0; loop_cnt--){
        wk_save_data += ((ascii_p[loop_cnt]-'0') * wk_keta);
        wk_keta = wk_keta * 10;
    }
    *short_p = wk_save_data;

    return;
} /* end of NWM_KYX_ASCII2SHORT */

