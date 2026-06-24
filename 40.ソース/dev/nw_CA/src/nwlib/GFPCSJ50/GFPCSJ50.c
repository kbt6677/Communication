/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSJ50                                    */
/*        FUNCTION          ････ NW個別(要求応答マッチングキー生成[CARDNET]) */
/*                                                                           */
/*        AUTHER            ････ HAS M.Matsumoto                             */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025-04-01                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  Matsumoto  2025/04/01 (J0680)新規作成                               */
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
#include "GFPCSJ50.h"

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
/*  DESCRIPTION     : NW個別(要求応答マッチングキー生成[CARDNET])           */
/****************************************************************************/
short NWM_MKM(
    char            *mtiinfo, 
    queue_data_def  *queinfo, 
    char            *fixdata, 
    char            *matchinfo,
    short           matchinfo_size,
    short           *matchinfo_len)
{
    fixedform_cardnet_1804_def  *ffd_1804 = (fixedform_cardnet_1804_def*)fixdata;
    fixedform_cardnet_1814_def  *ffd_1814 = (fixedform_cardnet_1814_def*)fixdata;
    mkm_matchkey_def            *matchkey = (mkm_matchkey_def*)matchinfo;
    MSG_HEADER_CARDNET_def      *msg_header;

    /* マッチングキー最大長チェック */
    if (matchinfo_size < DEF_MKM_MATCHKEY_LEN) {
        return DEF_MKM_ERROR;
    }
    msg_header = (MSG_HEADER_CARDNET_def *)queinfo->denbun;
    memset(matchinfo, 0x20, matchinfo_size);
    *matchinfo_len = 0;

    /* 要求応答マッチングキー生成(要求電文受信) */
    if (memcmp(mtiinfo, DEF_CA_MTI_1804_REQ, strlen(DEF_CA_MTI_1804_REQ))==0) {
        if ((ffd_1804->b11_system_audit_number.ffd_header.m_flg_exist) &&
            (ffd_1804->b12_local_tran_time.ffd_header.m_flg_exist) &&
            (ffd_1804->b24_function_code.ffd_header.m_flg_exist)) {
            memcpy(matchkey->ctrl_src_id, msg_header->ctrl_src_id, sizeof(matchkey->ctrl_src_id));
            memcpy(matchkey->mti, DEF_CA_MTI_1804_REQ, sizeof(matchkey->mti));
            memcpy(matchkey->system_audit_number, 
                ffd_1804->b11_system_audit_number.ffd_data, 
                ffd_1804->b11_system_audit_number.ffd_header.m_fixvalue_length);
            memcpy(matchkey->local_tran_time, 
                ffd_1804->b12_local_tran_time.ffd_data, 
                ffd_1804->b12_local_tran_time.ffd_header.m_fixvalue_length);
            memcpy(matchkey->function_code, 
                ffd_1804->b24_function_code.ffd_data, 
                ffd_1804->b24_function_code.ffd_header.m_fixvalue_length);
            *matchinfo_len = DEF_MKM_MATCHKEY_LEN;
            return DEF_MKM_NORMAL;
        } else {
            return DEF_MKM_ERROR;
        }
    }

    /* 要求応答マッチングキー生成(応答電文受信) */
    if (memcmp(mtiinfo, DEF_CA_MTI_1814_RSP, strlen(DEF_CA_MTI_1814_RSP))==0) {
        if ((ffd_1814->b11_system_audit_number.ffd_header.m_flg_exist) &&
            (ffd_1814->b12_local_tran_time.ffd_header.m_flg_exist) &&
            (ffd_1814->b24_function_code.ffd_header.m_flg_exist)) {
            memcpy(matchkey->ctrl_src_id, msg_header->ctrl_dst_id, sizeof(matchkey->ctrl_src_id));
            memcpy(matchkey->mti, DEF_CA_MTI_1804_REQ, sizeof(matchkey->mti));
            memcpy(matchkey->system_audit_number, 
                ffd_1814->b11_system_audit_number.ffd_data, 
                ffd_1814->b11_system_audit_number.ffd_header.m_fixvalue_length);
            memcpy(matchkey->local_tran_time, 
                ffd_1814->b12_local_tran_time.ffd_data, 
                ffd_1814->b12_local_tran_time.ffd_header.m_fixvalue_length);
            memcpy(matchkey->function_code, 
                ffd_1814->b24_function_code.ffd_data, 
                ffd_1814->b24_function_code.ffd_header.m_fixvalue_length);
            *matchinfo_len = DEF_MKM_MATCHKEY_LEN;
            return DEF_MKM_NORMAL;
        } else {
            return DEF_MKM_ERROR;
        }
    }

    return DEF_MKM_ERROR;
} /* end of NWM_MKM */

