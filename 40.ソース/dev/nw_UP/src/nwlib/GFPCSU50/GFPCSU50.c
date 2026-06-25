/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSU50                                    */
/*        FUNCTION          ････ NW個別(要求応答マッチングキー生成[UnionPay])*/
/*                                                                           */
/*        AUTHER            ････ HAS T.Hashimoto                             */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025-06-27                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0             2025/04/01 (J0680)新規作成                               */
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
#include <string.h> nolist
#include <stdlib.h> nolist

#include "file.h(queue_data)"

/* USER HEADER     */
#include "vproc.h"
#include "NWM_MKM.h"
#include "GFPCSU50.h"

/****************************************************************************/
/*  FUNCTION        : 1.1.0  要求応答マッチングキー生成処理                 */
/*  CALLING SEQ.    : short NWM_MKM(                                        */
/*                     char*, queue_data_def*, char*, char*, short, short*) */
/*  ARGUMENT        : 1.mtiinfo        (I)   MTI                            */
/*                  : 2.queinfo        (I)   キューファイル電文情報         */
/*                  : 3.fixdata        (I)   固定フォーマットデータ         */
/*                  : 4.matchinfo      (O)   マッチングキー情報             */
/*                  : 5.matchinfo_size (O)   マッチングキー最大長           */
/*                  : 6.matchinfo_len  (O)   マッチングキー長               */
/*  RETURN CODE     : 0:正常 1:エラー                                       */
/*  DESCRIPTION     : 要求電文、応答電文のマッチングキーを生成する          */
/****************************************************************************/
short NWM_MKM(
    char            *mtiinfo,
    queue_data_def  *queinfo,
    char            *fixdata,
    char            *matchinfo,
    short           matchinfo_size,
    short           *matchinfo_len)
{
    fixedform_unionpay_0820_def  *ffd_0820 = (fixedform_unionpay_0820_def *)fixdata;
    fixedform_unionpay_0830_def  *ffd_0830 = (fixedform_unionpay_0830_def *)fixdata;

    mkm_matchkey_def            *matchkey = (mkm_matchkey_def*)matchinfo;
//  MSG_HEADER_CARDNET_def      *msg_header;

    /* マッチングキー最大長チェック */
    if (matchinfo_size < DEF_MKM_MATCHKEY_LEN) {
        return DEF_MKM_ERROR;
    }
//  msg_header = (MSG_HEADER_CARDNET_def *)queinfo->denbun;
    memset(matchinfo, 0x20, matchinfo_size);
    *matchinfo_len = 0;

    /* 要求応答マッチングキー生成(要求電文受信) */
    if (memcmp(mtiinfo, DEF_UP_MTI_0820_REQ, strlen(DEF_UP_MTI_0820_REQ))==0) {
        if ((ffd_0820->b007_trans_date_time.ffd_header.m_flg_exist) &&
            (ffd_0820->b011_system_audit_number.ffd_header.m_flg_exist) &&
            (ffd_0820->b070_nw_mng_code.ffd_header.m_flg_exist)) {
            memcpy(matchkey->mti, DEF_UP_MTI_0820_REQ, sizeof(matchkey->mti));
            memcpy(matchkey->Transmission_Time,
                ffd_0820->b007_trans_date_time.ffd_data,
                ffd_0820->b007_trans_date_time.ffd_header.m_fixvalue_length);
            memcpy(matchkey->system_audit_number,
                ffd_0820->b011_system_audit_number.ffd_data,
                ffd_0820->b011_system_audit_number.ffd_header.m_fixvalue_length);
            memcpy(matchkey->nw_mng_code,
                ffd_0820->b070_nw_mng_code.ffd_data,
                ffd_0820->b070_nw_mng_code.ffd_header.m_fixvalue_length);
            *matchinfo_len = DEF_MKM_MATCHKEY_LEN;
            return DEF_MKM_NORMAL;
        } else {
            return DEF_MKM_ERROR;
        }
    }

    /* 要求応答マッチングキー生成(応答電文受信) */
    if (memcmp(mtiinfo, DEF_UP_MTI_0830_RSP, strlen(DEF_UP_MTI_0830_RSP))==0) {
        if ((ffd_0830->b007_trans_date_time.ffd_header.m_flg_exist) &&
            (ffd_0830->b011_system_audit_number.ffd_header.m_flg_exist) &&
            (ffd_0830->b070_nw_mng_code.ffd_header.m_flg_exist)) {
            memcpy(matchkey->mti, DEF_UP_MTI_0820_REQ, sizeof(matchkey->mti));
            memcpy(matchkey->Transmission_Time,
                ffd_0830->b007_trans_date_time.ffd_data,
                ffd_0830->b007_trans_date_time.ffd_header.m_fixvalue_length);
            memcpy(matchkey->system_audit_number,
                ffd_0830->b011_system_audit_number.ffd_data,
                ffd_0830->b011_system_audit_number.ffd_header.m_fixvalue_length);
            memcpy(matchkey->nw_mng_code,
                ffd_0830->b070_nw_mng_code.ffd_data,
                ffd_0830->b070_nw_mng_code.ffd_header.m_fixvalue_length);
            *matchinfo_len = DEF_MKM_MATCHKEY_LEN;
            return DEF_MKM_NORMAL;
        } else {
            return DEF_MKM_ERROR;
        }
    }

    return DEF_MKM_ERROR;
} /* end of NWM_MKM */

