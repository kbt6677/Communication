/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSN80                                    */
/*        FUNCTION          ････ NW個別(鍵交換個別処理[NYCE])                */
/*                               鍵交換個別処理[NYCE])を行う。               */
/*                                                                           */
/*        AUTHER            ････ HAS                                         */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-08-20                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS        2025/08/20 新規作成                                      */
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
#include "msg_NY.h"
#include "NWM_KYX.h"
#include "GFPCSN80.h"

/*******************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_msg_check_req                             */
/*  CALLING SEQ.    : short NWM_KYX_msg_check_req(                             */
/*                                      char              *req_message         */
/*                                    , char              *p_rcv_data_len      */
/*                                    , db_gfnwi_def      *nwi_g               */
/*                                    , db_gfnwi_def      *nwi_i               */
/*                                    , char              *nws_n               */
/*                                    , char              *nws_i               */
/*                                    , char              *nws_s               */
/*                                    , char              *nws_c               */
/*                                    , db_gckey_def      *gckey               */
/*                                    , NWM_KYX_arg_1_def *NWM_KYX_arg_1)      */
/*  ARGUMENT        :  1.req_message    (I) 受信データ(要求電文)               */
/*                  :  2.p_rcv_data_len (I) 受信電文長                         */
/*                  :  3.nwi_g          (I) NW情報レコード(グループ単位)       */
/*                  :  4.nwi_i          (I) NW情報レコード(インタフェース単位) */
/*                  :  5.nws_n          (I) 接続先固有情報(NW単位)             */
/*                  :  6.nws_i          (I) 接続先固有情報(インタフェース単位) */
/*                  :  7.nws_s          (I) 接続先固有情報(ステーション単位)   */
/*                  :  8.nws_c          (I) 接続先固有情報(コネクション単位)   */
/*                  :  9.gckey          (I) 鍵管理情報レコード                 */
/*                  : 10.NWM_KYX_arg_1  (O) 精査処理結果                       */
/*  RETURN CODE     : 0：精査OK                                                */
/*                  : 1：精査エラー(拒否応答)                                  */
/*                  : 3：精査エラー(電文破棄)                                  */
/*                  : 9：精査エラー(異常)                                      */
/*  DESCRIPTION     : 鍵交換電文精査(要求受信)                                 */
/*******************************************************************************/
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
    msg_nyce_def               *rcv_msg_ctl;
    fixedform_nyce_0800_def    *rcv_0800_msg;
    char   lc_wbuf[256];
    short  ls_bit125_flg;
    short  Sub_Tag_Len;
    short  all_Sub_Tag_Len = 0;

typedef struct  __b125_sub_tag_def
    {
        char            Sub_Tag_ID[2];
        char            Sub_Tag_Length[3];
        char            Sub_Tag_Data[1];
    } b125_sub_tag_def;

typedef struct
    {
        char                Primary_Tag_ID[2];
        char                Primary_Tag_Length[3];
        b125_sub_tag_def    sub_tag[2];
    } b125_fdd_data_def;
    b125_fdd_data_def *b125_fdd_data;
    char              *wpch_search_wk;
    b125_sub_tag_def  *wpch_search_wk_125;
    short             b125_kp_flg;
    short             b125_pri_tag_leng;
    short             b125_tag_leng_int = 0;

    rcv_msg_ctl = (msg_nyce_def*)req_message;

    /* データ部確認 */
    /* MTIチェック */
    if (memcmp(rcv_msg_ctl->mti,
               DEF_NY_MTI_0800_REQ,
               sizeof(rcv_msg_ctl->mti)) != 0){
        /* エラー発生ヘッダ番号設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_HEADER_MTI_ERR,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    rcv_0800_msg = (fixedform_nyce_0800_def*)&rcv_msg_ctl->ffd;

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
    lc_wbuf[0] = '0';
    lc_wbuf[1] = '0';
    lc_wbuf[2] = '0';
    lc_wbuf[3] = '0';
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
               DEF_NWM_KYX_BIT_070_171,
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
    /* --- BIT125 ----------------- */
    if (rcv_0800_msg->b125_nw_mng_info.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_125,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    if (rcv_0800_msg->b125_nw_mng_info.ffd_header.m_fixvalue_length >
                                 DEF_NWM_KYX_BIT_125_LENG){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_125,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    b125_kp_flg        = DEF_FLAG_OFF;
    b125_pri_tag_leng  = 0;
    b125_fdd_data      = (b125_fdd_data_def *)&rcv_0800_msg->b125_nw_mng_info.ffd_data[0];

    while( b125_pri_tag_leng < rcv_0800_msg->b125_nw_mng_info.ffd_header.m_fixvalue_length ) {
        b125_fdd_data = (b125_fdd_data_def *)&rcv_0800_msg->b125_nw_mng_info.ffd_data[b125_pri_tag_leng];
        if (memcmp(b125_fdd_data->Primary_Tag_ID,
                   DEF_NWM_KYX_BIT_125_PRITAG_KP,
                   sizeof(b125_fdd_data->Primary_Tag_ID)) != 0){
            memset(lc_wbuf , DEF_BUF_NULL, sizeof(lc_wbuf));
            memcpy(lc_wbuf ,
                   b125_fdd_data->Primary_Tag_Length,
                   sizeof(b125_fdd_data->Primary_Tag_Length));
            b125_tag_leng_int = (short)atoi(lc_wbuf);
            b125_pri_tag_leng += (b125_tag_leng_int + 5);
        }
        else {
            /* KPあり */
            b125_kp_flg = DEF_FLAG_ON;
            memset(lc_wbuf , DEF_BUF_NULL, sizeof(lc_wbuf));
            memcpy(lc_wbuf ,
                   b125_fdd_data->Primary_Tag_Length,
                   sizeof(b125_fdd_data->Primary_Tag_Length));
            b125_tag_leng_int = (short)atoi(lc_wbuf);
            break;
        }
    }

    wpch_search_wk     = (char *)&b125_fdd_data->sub_tag;
    wpch_search_wk_125 = (b125_sub_tag_def *)&b125_fdd_data->sub_tag;

    /* KP有無確認 */
    if (b125_kp_flg != DEF_FLAG_ON) {
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_125,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    ls_bit125_flg = 0;
    /*------------------------------------------------*/
    /*    TAGの検索                                   */
    /*------------------------------------------------*/
    all_Sub_Tag_Len = 0;
    while( b125_tag_leng_int > all_Sub_Tag_Len ) {
        if( memcmp( wpch_search_wk_125->Sub_Tag_ID, DEF_NWM_KYX_BIT_125_SUB_TAG_01,  DEF_NWM_KYX_BIT_125_SUBTAG_LEN ) == 0 ) {
            /*------------------------------------------------*/
            /*    TAG01検出時                                 */
            /*------------------------------------------------*/
            memset( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
            memcpy(lc_wbuf , wpch_search_wk_125->Sub_Tag_Length,sizeof(wpch_search_wk_125->Sub_Tag_Length));
            ls_result = CMIN_num_check ( lc_wbuf );
            if ( ls_result != DEF_NWM_KYX_RTN_OK ) {
                memcpy(NWM_KYX_arg_1->err_bit,
                       DEF_NWM_KYX_BIT_125,
                       sizeof(NWM_KYX_arg_1->err_bit));
                /* 電文破棄 */
                return DEF_NWM_KYX_RTN_HAKI_MSG;
            }
            Sub_Tag_Len = (short)atoi(lc_wbuf);

            if (Sub_Tag_Len > DEF_NWM_KYX_MAX_KEY_LEN){
                /* データ長制限値を超えていた場合は制限値までをコピーする */
                memcpy(NWM_KYX_arg_1->key
                      , wpch_search_wk_125->Sub_Tag_Data
                      , DEF_NWM_KYX_MAX_KEY_LEN);
                NWM_KYX_arg_1->key_leng = DEF_NWM_KYX_MAX_KEY_LEN;
            }
            else {
                memcpy(NWM_KYX_arg_1->key
                      , wpch_search_wk_125->Sub_Tag_Data
                      , Sub_Tag_Len);
                NWM_KYX_arg_1->key_leng = Sub_Tag_Len;
            }

            wpch_search_wk = wpch_search_wk + 5 + Sub_Tag_Len;
            wpch_search_wk_125 = (b125_sub_tag_def *)wpch_search_wk;
            all_Sub_Tag_Len = all_Sub_Tag_Len + 5 + Sub_Tag_Len;

            if (( ls_bit125_flg  == 0) ||
               ( ls_bit125_flg  == 7))
            {
                ls_bit125_flg = ls_bit125_flg + 5;
            }
            else{
                memcpy(NWM_KYX_arg_1->err_bit,
                       DEF_NWM_KYX_BIT_125,
                       sizeof(NWM_KYX_arg_1->err_bit));
                /* 電文破棄 */
                return DEF_NWM_KYX_RTN_HAKI_MSG;
            }
        } else 
        if( memcmp( wpch_search_wk_125->Sub_Tag_ID, DEF_NWM_KYX_BIT_125_SUB_TAG_02,  DEF_NWM_KYX_BIT_125_SUBTAG_LEN ) == 0 ) {
            /*------------------------------------------------*/
            /*    TAG02検出時                                 */
            /*------------------------------------------------*/
            memset ( lc_wbuf , DEF_BUF_NULL, sizeof(lc_wbuf));
            memcpy(lc_wbuf , wpch_search_wk_125->Sub_Tag_Length,sizeof(wpch_search_wk_125->Sub_Tag_Length));
            ls_result = CMIN_num_check ( lc_wbuf );
            if ( ls_result != DEF_NWM_KYX_RTN_OK ) {
                memcpy(NWM_KYX_arg_1->err_bit,
                       DEF_NWM_KYX_BIT_125,
                       sizeof(NWM_KYX_arg_1->err_bit));
                /* 電文破棄 */
                return DEF_NWM_KYX_RTN_HAKI_MSG;
            }
            Sub_Tag_Len = (short)atoi(lc_wbuf);

            if (Sub_Tag_Len > DEF_NWM_KYX_MAX_CKDIGIT_LEN){
                /* データ長制限値を超えていた場合は制限値までをコピーする */
                memcpy(NWM_KYX_arg_1->checkdigit
                      , wpch_search_wk_125->Sub_Tag_Data
                      , DEF_NWM_KYX_MAX_CKDIGIT_LEN);
                NWM_KYX_arg_1->checkdigit_leng = DEF_NWM_KYX_MAX_CKDIGIT_LEN;
            }
            else {
                memcpy(NWM_KYX_arg_1->checkdigit
                      , wpch_search_wk_125->Sub_Tag_Data
                      , Sub_Tag_Len);
                NWM_KYX_arg_1->checkdigit_leng = Sub_Tag_Len;
            }

            wpch_search_wk = wpch_search_wk + 5 + Sub_Tag_Len;
            wpch_search_wk_125 = (b125_sub_tag_def *)wpch_search_wk;
            all_Sub_Tag_Len = all_Sub_Tag_Len + 5 + Sub_Tag_Len;

            if (( ls_bit125_flg  == 0) ||
               ( ls_bit125_flg  == 5))
            {
                ls_bit125_flg = ls_bit125_flg + 7;
            }
            else{
                memcpy(NWM_KYX_arg_1->err_bit,
                       DEF_NWM_KYX_BIT_125,
                       sizeof(NWM_KYX_arg_1->err_bit));
                /* 電文破棄 */
                return DEF_NWM_KYX_RTN_HAKI_MSG;
             }

        } else {
            memset ( lc_wbuf , DEF_BUF_NULL, sizeof(lc_wbuf));
            memcpy(lc_wbuf , wpch_search_wk_125->Sub_Tag_Length,sizeof(wpch_search_wk_125->Sub_Tag_Length));
            Sub_Tag_Len = (short)atoi(lc_wbuf);
            all_Sub_Tag_Len = all_Sub_Tag_Len + 5 + Sub_Tag_Len;
            wpch_search_wk = wpch_search_wk + 5 + Sub_Tag_Len;
            wpch_search_wk_125 = (b125_sub_tag_def *)wpch_search_wk;

        }
        if ( ls_bit125_flg  == 12) {
            /* データ取得完了 */
            break;
        }
    } /* while end */
     
    if ( ls_bit125_flg  != 12) 
    {
        memcpy(NWM_KYX_arg_1->err_bit,
               DEF_NWM_KYX_BIT_125,
               sizeof(NWM_KYX_arg_1->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

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
     msg_nyce_def              *rcv_msg_ctl;
    fixedform_nyce_0810_def    *rcv_0810_msg;
    char                        lc_wbuf[30];
    short                       ls_result = 0;

    rcv_msg_ctl = (msg_nyce_def*)req_message;

    /* データ部確認 */
    /* MTIチェック */
    if (memcmp(rcv_msg_ctl->mti,
               DEF_NY_MTI_0810_RSP,
               sizeof(rcv_msg_ctl->mti)) != 0){
        /* エラー発生ヘッダ番号設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_HEADER_MTI_ERR,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    rcv_0810_msg = (fixedform_nyce_0810_def*)&rcv_msg_ctl->ffd;

    /* --- BIT007 ----------------- */
    if (rcv_0810_msg->b007_trans_date_time.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_007,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    if (rcv_0810_msg->b007_trans_date_time.ffd_header.m_fixvalue_length !=
                                 DEF_NWM_KYX_BIT_007_LENG){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_007,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    memset ( lc_wbuf , DEF_BUF_NULL, sizeof(lc_wbuf));
    lc_wbuf[0] = '0';
    lc_wbuf[1] = '0';
    lc_wbuf[2] = '0';
    lc_wbuf[3] = '0';
    memcpy (&lc_wbuf[4], rcv_0810_msg->b007_trans_date_time.ffd_data, DEF_NWM_KYX_BIT_007_LENG);
    ls_result = CMIN_check_datetime ( lc_wbuf );
    if ( ls_result != DEF_NWM_KYX_RTN_OK ) {

        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_007,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    /* --- BIT011 ----------------- */
    if (rcv_0810_msg->b011_system_audit_number.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_011,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    if (rcv_0810_msg->b011_system_audit_number.ffd_header.m_fixvalue_length !=
                                 DEF_NWM_KYX_BIT_011_LENG){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_011,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    /* --- BIT039 ----------------- */
    if (rcv_0810_msg->b039_response_code.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_039,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    if (rcv_0810_msg->b039_response_code.ffd_header.m_fixvalue_length !=
                                 DEF_NWM_KYX_BIT_039_LENG){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_039,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }
    /* 応答種別確認 (00：正常) */
    if (memcmp(rcv_0810_msg->b039_response_code.ffd_data,
               DEF_NWM_ERR_ACTION_CODE_00,
               2) != 0) {
        /* 拒否応答 */
        return DEF_NWM_KYX_RTN_NG_REJ;
    }
    /* --- BIT070 ----------------- */
    if (rcv_0810_msg->b070_nw_mng_code.ffd_header.m_flg_exist != true){
        /* エラー発生BIT設定 */
        memcpy(NWM_KYX_arg_2->err_bit,
               DEF_NWM_KYX_BIT_070,
               sizeof(NWM_KYX_arg_2->err_bit));
        /* 電文破棄 */
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    if (rcv_0810_msg->b070_nw_mng_code.ffd_header.m_fixvalue_length !=
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
/*  RETURN CODE     : 0 :正常                                               */
/*                  : -1:異常                                               */
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


    msg_nyce_def                   *send_0800_msg;
    fixedform_nyce_0800_def        *send_form_0800;

    COM_SDT_arg_2_def  COM_SDT_arg_2_data;       /* 日時取得用データ        */
    COM_SDT_arg_3_def  COM_SDT_arg_3_data;       /* 日時取得用データ        */
    long long          date_time;                /* 日時取得用データ        */

    /* 送信バッファ設定 */
    send_0800_msg  = (msg_nyce_def*)NWM_KYX_arg_4->message;
    send_form_0800 = (fixedform_nyce_0800_def*)&send_0800_msg->ffd;

    /* 鍵交換依頼でなければエラー */
    if (key_data_kind != DEF_NWM_KYX_MSG_KIND_IRAIREQ){
        /* 異常終了 */
        return DEF_NWM_KYX_RTN_ERR;
    }

    /* 送信日時設定 */
    COM_SDT(DEF_COM_SDT_arg1_gmt,
            &COM_SDT_arg_2_data,
            &COM_SDT_arg_3_data,
            &date_time);

    /* MTI設定 */
    memcpy(send_0800_msg->mti,
           DEF_NY_MTI_0800_REQ,
           sizeof(send_0800_msg->mti));

    memcpy(NWM_KYX_arg_4->mti,
           DEF_NY_MTI_0800_REQ,
           sizeof(NWM_KYX_arg_4->mti));

    /* --- BIT007 ----------------- */
    send_form_0800->b007_trans_date_time.ffd_header.m_flg_exist       = true;
    send_form_0800->b007_trans_date_time.ffd_header.m_fixvalue_length = DEF_NWM_KYX_BIT_007_LENG;
    memcpy( send_form_0800->b007_trans_date_time.ffd_data
          ,&COM_SDT_arg_2_data.mm[0]    , DEF_NWM_KYX_BIT_007_LENG);  
    /* --- BIT011 ----------------- */
    send_form_0800->b011_system_audit_number.ffd_header.m_flg_exist       = true;
    send_form_0800->b011_system_audit_number.ffd_header.m_fixvalue_length = DEF_NWM_KYX_BIT_011_LENG;
    memcpy( send_form_0800->b011_system_audit_number.ffd_data
          , NWM_KYX_arg_3->sysytem_no , DEF_NWM_KYX_BIT_011_LENG);
    /* --- BIT070 ----------------- */
    send_form_0800->b070_nw_mng_code.ffd_header.m_flg_exist       = true;
    send_form_0800->b070_nw_mng_code.ffd_header.m_fixvalue_length = DEF_NWM_KYX_BIT_070_LENG;
    memcpy( send_form_0800->b070_nw_mng_code.ffd_data
          , DEF_NWM_KYX_BIT_070_191 ,   DEF_NWM_KYX_BIT_070_LENG);
    /* --- BIT096 ----------------- */
    send_form_0800->b096_msg_security_code.ffd_header.m_flg_exist         = false;
    send_form_0800->b096_msg_security_code.ffd_header.m_fixvalue_length   = 0;
    memset( send_form_0800->b096_msg_security_code.ffd_data
          ,DEF_BUF_SPACE    , DEF_NWM_KYX_BIT_096_LENG);
    /* --- BIT125 ----------------- */
    send_form_0800->b125_nw_mng_info.ffd_header.m_flg_exist               = false;
    send_form_0800->b125_nw_mng_info.ffd_header.m_fixvalue_length         = 0;
    memset( send_form_0800->b125_nw_mng_info.ffd_data
          ,DEF_BUF_SPACE    , DEF_NWM_KYX_BIT_125_LENG);

    NWM_KYX_arg_4->message_leng =  4 + sizeof(fixedform_nyce_0800_def);

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
/*  RETURN CODE     : 0 :正常                                               */
/*                  : -1:異常                                               */
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

    msg_nyce_def                   *send_0810_msg;
    msg_nyce_def                   *rcv_0800_msg;
    fixedform_nyce_0810_def        *send_form_0810;
    fixedform_nyce_0800_def        *rcv_0800_ffd;

    /* 送信バッファ設定 */
    send_0810_msg  = (msg_nyce_def*)NWM_KYX_arg_6->message;
    rcv_0800_msg   = (msg_nyce_def*)NWM_KYX_arg_6->rcv_message;
    send_form_0810 = (fixedform_nyce_0810_def*)&send_0810_msg->ffd;
    rcv_0800_ffd   = (fixedform_nyce_0800_def*)&rcv_0800_msg->ffd;

    /* リクエスト電文を初期化する */
    memset(send_form_0810, 0, sizeof(fixedform_nyce_0810_def));

    /* MTI設定 */
    memcpy(send_0810_msg->mti,
           DEF_NY_MTI_0810_RSP,
           sizeof(send_0810_msg->mti));

    memcpy(NWM_KYX_arg_6->mti,
           DEF_NY_MTI_0810_RSP,
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
               DEF_NWM_ERR_ACTION_CODE_00,
               sizeof(send_form_0810->b039_response_code.ffd_data));
    }
    else {
        /* 拒否応答 */
        memcpy(send_form_0810->b039_response_code.ffd_data,
               DEF_NWM_ERR_ACTION_CODE_76,
               sizeof(send_form_0810->b039_response_code.ffd_data));
    }
    send_form_0810->b039_response_code.ffd_header.m_flg_exist = true;
    send_form_0810->b039_response_code.ffd_header.m_fixvalue_length =
                      sizeof(send_form_0810->b039_response_code.ffd_data);

    /* --- BIT070 ----------------- */
    memcpy(&send_form_0810->b070_nw_mng_code,
           &rcv_0800_ffd->b070_nw_mng_code,
           sizeof(send_form_0810->b070_nw_mng_code));
    NWM_KYX_arg_6->message_leng =  4 + sizeof(fixedform_nyce_0810_def);

    return DEF_NWM_KYX_RTN_OK;

}
/* end of NWM_KYX_msg_edit_rsp */


/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_cst_check_req                          */
/*  CALLING SEQ.    : short NWM_KYX_cst_check_req(char *station_st)         */
/*  ARGUMENT        :  1.station_st   (I) 局状態                            */
/*  RETURN CODE     : 0 :正常                                               */
/*                  : -1:異常                                               */
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
/*  RETURN CODE     : 0 :正常                                               */
/*                  : -1:異常                                               */
/*  DESCRIPTION     : 鍵交換局状態チェック(応答受信)                        */
/****************************************************************************/
short NWM_KYX_cst_check_rsp(short   key_data_kind
                          , char    *station_st)
{
    /* NYCEではチェック不要のため正常を返却する */
    return DEF_NWM_KYX_RTN_OK;
}
/* end of NWM_KYX_cst_check_rsp */

