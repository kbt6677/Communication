/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSA40                                    */
/*        FUNCTION          ････ NW個別(電文種別判定[AEGN])                  */
/*                                                                           */
/*        AUTHER            ････ HAS T.Sugisaki                              */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025-06-27                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  Sugisaki   2025/06/27 (J0680)新規作成                               */
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
#include "vproc.h"
#include "NWM_MSJ.h"
#include "GFPCSA40.h"

/****************************************************************************/
/*  FUNCTION        : 1.1.0  電文種別判定処理                               */
/*  CALLING SEQ.    : short NWM_MSJ(char*, char*, char*)                    */
/*  ARGUMENT        : 1.mti            (I)   MTI                            */
/*                  : 2.fixdata        (I)   固定フォーマットデータ         */
/*                  : 3.ctlkind        (O)   制御電文種別                   */
/*  RETURN CODE     : 0:正常 1:エラー                                       */
/*  DESCRIPTION     : NW個別(電文種別判定[AEGN])                            */
/****************************************************************************/
short   NWM_MSJ(char *mti, char *fixdata, char *ctlkind)
{
    fixedform_aegn_1804_def  *ffd_1804 = (fixedform_aegn_1804_def*)fixdata;
    fixedform_aegn_1814_def  *ffd_1814 = (fixedform_aegn_1814_def*)fixdata;
    memset(ctlkind, 0x20, 4);

    /* 制御電文種別判定(MTI:1804) */
    if (memcmp(mti, DEF_AX_MTI_1804_REQ, strlen(DEF_AX_MTI_1804_REQ))==0) {
        if (ffd_1804->b24_function_code.ffd_header.m_flg_exist) {
            /* 開局要求 */
            if (memcmp(ffd_1804->b24_function_code.ffd_data, DEF_AX_F24_FUNCTION_801_OPN, strlen(DEF_AX_F24_FUNCTION_801_OPN))==0) {
                ctlkind[0] = DEF_CTLFNC_CNT_STS_ECH;    // 1:局状態・エコー制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_CNT_OPN;        // 1:開局(サインオン)
                ctlkind[3] = DEF_CTLINT_NORMAL;         // 1:通常
                return DEF_MSJ_NORMAL;
            }
            /* エコーテスト要求 */
            if (memcmp(ffd_1804->b24_function_code.ffd_data, DEF_AX_F24_FUNCTION_831_ECH, strlen(DEF_AX_F24_FUNCTION_831_ECH))==0) {
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

    /* その他MTI */
    return DEF_MSJ_ERROR;
} /* end of NWM_MSJ */

