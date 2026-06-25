/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSJ40                                    */
/*        FUNCTION          ････ NW個別(電文種別判定[CARDNET])               */
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

/* USER HEADER     */
#include "common.h"
#include "NWM_MSJ.h"
#include "GFPCSJ40.h"
#include "vproc.h"

/****************************************************************************/
/*  FUNCTION        : 1.1.0  電文種別判定処理                               */
/*  CALLING SEQ.    : short NWM_MSJ(char*, char*, char*)                    */
/*  ARGUMENT        : 1.mti            (I)   MTI                            */
/*                  : 2.fixdata        (I)   固定フォーマットデータ         */
/*                  : 3.ctlkind        (O)   制御電文種別                   */
/*  RETURN CODE     : 0:正常 1:エラー                                       */
/*  DESCRIPTION     : NW個別(電文種別判定[CARDNET])                         */
/****************************************************************************/
short   NWM_MSJ(char *mti, char *fixdata, char *ctlkind)
{
    fixedform_cardnet_1804_def  *ffd_1804 = (fixedform_cardnet_1804_def*)fixdata;
    fixedform_cardnet_1814_def  *ffd_1814 = (fixedform_cardnet_1814_def*)fixdata;
    fixedform_cardnet_1644_def  *ffd_1644 = (fixedform_cardnet_1644_def*)fixdata;
    memset(ctlkind, 0x20, 4);

    /* 制御電文種別判定(MTI:1804) */
    if (memcmp(mti, DEF_CA_MTI_1804_REQ, strlen(DEF_CA_MTI_1804_REQ))==0) {
        if (ffd_1804->b24_function_code.ffd_header.m_flg_exist) {
            /* 開局要求 */
            if (memcmp(ffd_1804->b24_function_code.ffd_data, DEF_CA_F24_FUNCTION_801_OPN, strlen(DEF_CA_F24_FUNCTION_801_OPN))==0) {
                ctlkind[0] = DEF_CTLFNC_CNT_STS_ECH;    // 1:局状態・エコー制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_CNT_OPN;        // 1:開局(サインオン)
                ctlkind[3] = DEF_CTLINT_NORMAL;         // 1:通常
                return DEF_MSJ_NORMAL;
            }
            /* 閉局要求 */
            if (memcmp(ffd_1804->b24_function_code.ffd_data, DEF_CA_F24_FUNCTION_802_CLS, strlen(DEF_CA_F24_FUNCTION_802_CLS))==0) {
                ctlkind[0] = DEF_CTLFNC_CNT_STS_ECH;    // 1:局状態・エコー制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_CNT_CLS;        // 2:閉局(サインオフ)
                ctlkind[3] = DEF_CTLINT_NORMAL;         // 1:通常
                return DEF_MSJ_NORMAL;
            }
            /* 鍵交換要求 */
            if (memcmp(ffd_1804->b24_function_code.ffd_data, DEF_CA_F24_FUNCTION_811_KEY, strlen(DEF_CA_F24_FUNCTION_811_KEY))==0) {
                ctlkind[0] = DEF_CTLFNC_KEY_EXCH;       // 2:鍵交換制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_KEY_EXC;        // 5:鍵交換
                ctlkind[3] = DEF_CTLINT_NORMAL;         // 1:通常
                return DEF_MSJ_NORMAL;
            }
            /* カットオーバー要求 */
            if (memcmp(ffd_1804->b24_function_code.ffd_data, DEF_CA_F24_FUNCTION_821_CUT, strlen(DEF_CA_F24_FUNCTION_821_CUT))==0) {
                ctlkind[0] = DEF_CTLFNC_CUT_OVER;       // 3:カットオーバー制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_CUT_START;      // 6:カットオーバー
                ctlkind[3] = DEF_CTLINT_NORMAL;         // 1:通常
                return DEF_MSJ_NORMAL;
            }
            /* エコーテスト要求 */
            if (memcmp(ffd_1804->b24_function_code.ffd_data, DEF_CA_F24_FUNCTION_831_ECH, strlen(DEF_CA_F24_FUNCTION_831_ECH))==0) {
                ctlkind[0] = DEF_CTLFNC_CNT_STS_ECH;    // 1:局状態・エコー制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_ECH_SND;        // 3:エコーテスト
                ctlkind[3] = DEF_CTLINT_NORMAL;         // 1:通常
                return DEF_MSJ_NORMAL;
            }
            /* 不明 */
            return DEF_MSJ_ERROR;
        } else {
            /* 不明 */
            return DEF_MSJ_ERROR;
        }
    }

    /* 制御電文種別判定(MTI:1644) */
    if (memcmp(mti, DEF_CA_MTI_1644_NTC, strlen(DEF_CA_MTI_1644_NTC))==0) {
        if (ffd_1644->b24_function_code.ffd_header.m_flg_exist) {
            /* 障害電文通知 */
            if (memcmp(ffd_1804->b24_function_code.ffd_data, DEF_CA_F24_FUNCTION_650_ERR, strlen(DEF_CA_F24_FUNCTION_650_ERR))==0) {
                ctlkind[0] = DEF_CTLFNC_NTF_MSG;        // 5:通知電文制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_FAL;            // A:障害電文
                ctlkind[3] = DEF_CTLINT_NORMAL;         // 1:通常
                return DEF_MSJ_NORMAL;
            }
            /* 不明 */
            return DEF_MSJ_ERROR;
        } else {
            /* 不明 */
            return DEF_MSJ_ERROR;
        }
    }

    /* その他MTI */
    return DEF_MSJ_ERROR;
} /* end of NWM_MSJ */

