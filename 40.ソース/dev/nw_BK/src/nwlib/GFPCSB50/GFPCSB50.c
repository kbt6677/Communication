/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSB50                                    */
/*        FUNCTION          ････ NW個別(要求応答マッチングキー生成[BANKNET]) */
/*                                                                           */
/*        AUTHER            ････ HAS T.Sugisaki                              */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025-07-02                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  Sugisaki   2025/07/02 (J0680)新規作成                               */
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
#include "GFPCSB50.h"

/****************************************************************************/
/*  FUNCTION        : 1.1.0  要求応答マッチングキー生成処理                 */
/*  CALLING SEQ.    : short NWM_MKM(                                        */
/*                     char*, queue_data_def*, char*, char*, short, short*) */
/*  ARGUMENT        : 1.mtiinfo        (I)   MTI                            */
/*                  : 2.queinfo        (I)   キューファイル電文情報         */
/*                  : 3.fixdata        (I)   固定フォーマットデータ         */
/*                  : 4.matchinfo      (O)   マッチングキー情報             */
/*                  : 5.matchinfo_size (I)   マッチングキー最大長           */
/*                  : 6.matchinfo_len  (O)   マッチングキー長               */
/*  RETURN CODE     : 0:正常 1:エラー                                       */
/*  DESCRIPTION     : NW個別(要求応答マッチングキー生成[BANKNET])           */
/****************************************************************************/
short NWM_MKM(
    char            *mtiinfo, 
    queue_data_def  *queinfo, 
    char            *fixdata, 
    char            *matchinfo,
    short           matchinfo_size,
    short           *matchinfo_len)
{
    fixedform_banknet_0800_def  *ffd_0800 = (fixedform_banknet_0800_def*)fixdata;
    fixedform_banknet_0810_def  *ffd_0810 = (fixedform_banknet_0810_def*)fixdata;
    mkm_matchkey_def            *matchkey = (mkm_matchkey_def*)matchinfo;

    /* マッチングキー最大長チェック */
    if (matchinfo_size < sizeof(mkm_matchkey_def)) {
        return DEF_MKM_ERROR;
    }

    memset(matchinfo, 0x20, matchinfo_size);
    *matchinfo_len = 0;

    if (memcmp(mtiinfo, DEF_BK_MTI_0800_REQ, strlen(DEF_BK_MTI_0800_REQ))==0) {
        /* 要求応答マッチングキー生成(要求電文受信) */
        if ((ffd_0800->b007_trans_date_time.ffd_header.m_flg_exist) &&
            (ffd_0800->b011_system_audit_number.ffd_header.m_flg_exist) &&
            (ffd_0800->b070_nw_mng_code.ffd_header.m_flg_exist)) {
            memcpy(matchkey->mti, mtiinfo, sizeof(matchkey->mti));
            memcpy(matchkey->trans_date_time, 
                ffd_0800->b007_trans_date_time.ffd_data, 
                ffd_0800->b007_trans_date_time.ffd_header.m_fixvalue_length);
            memcpy(matchkey->system_audit_number, 
                ffd_0800->b011_system_audit_number.ffd_data, 
                ffd_0800->b011_system_audit_number.ffd_header.m_fixvalue_length);
            memcpy(matchkey->nw_mng_code, 
                ffd_0800->b070_nw_mng_code.ffd_data, 
                ffd_0800->b070_nw_mng_code.ffd_header.m_fixvalue_length);
            *matchinfo_len = sizeof(mkm_matchkey_def);
            return DEF_MKM_NORMAL;
        } else {
            return DEF_MKM_ERROR;
        }
    }else if (memcmp(mtiinfo, DEF_BK_MTI_0810_RSP, strlen(DEF_BK_MTI_0810_RSP))==0) {
        /* 要求応答マッチングキー生成(応答電文受信) */
        if ((ffd_0810->b007_trans_date_time.ffd_header.m_flg_exist) &&
            (ffd_0810->b011_system_audit_number.ffd_header.m_flg_exist) &&
            (ffd_0810->b070_nw_mng_code.ffd_header.m_flg_exist)) {
            memcpy(matchkey->mti, DEF_BK_MTI_0800_REQ, sizeof(matchkey->mti));
            memcpy(matchkey->trans_date_time, 
                ffd_0810->b007_trans_date_time.ffd_data, 
                ffd_0810->b007_trans_date_time.ffd_header.m_fixvalue_length);
            memcpy(matchkey->system_audit_number, 
                ffd_0810->b011_system_audit_number.ffd_data, 
                ffd_0810->b011_system_audit_number.ffd_header.m_fixvalue_length);
            memcpy(matchkey->nw_mng_code, 
                ffd_0810->b070_nw_mng_code.ffd_data, 
                ffd_0810->b070_nw_mng_code.ffd_header.m_fixvalue_length);
            *matchinfo_len = sizeof(mkm_matchkey_def);
            return DEF_MKM_NORMAL;
        } else {
            return DEF_MKM_ERROR;
        }
    }

    return DEF_MKM_ERROR;
} /* end of NWM_MKM */

