/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSU40                                    */
/*        FUNCTION          ････ NW個別(電文種別判定[UnionPay])              */
/*                                                                           */
/*        AUTHER            ････ HAS T.Hashimoto                             */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025-06-27                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0             2025/06/27 (J0680)新規作成                               */
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
#include "msg_UP.h"

/****************************************************************************/
/*  FUNCTION        : 1.1.0  電文種別判定処理                               */
/*  CALLING SEQ.    : short NWM_MSJ(char*, char*, char*)                    */
/*  ARGUMENT        : 1.mti            (I)   MTI                            */
/*                  : 2.fixdata        (I)   固定フォーマットデータ         */
/*                  : 3.ctlkind        (O)   制御電文種別                   */
/*  RETURN CODE     : 0:正常 1:エラー                                       */
/*  DESCRIPTION     : 電文種別の判定を行い、制御電文種別を決定し返却する    */
/****************************************************************************/
short   NWM_MSJ(char *mti, char *fixdata, char *ctlkind)
{
    fixedform_unionpay_0820_def  *ffd_0820 = (fixedform_unionpay_0820_def *)fixdata;
    fixedform_unionpay_0800_def  *ffd_0800 = (fixedform_unionpay_0800_def *)fixdata;
    memset(ctlkind, 0x20, 4);

    /* 制御電文種別判定(MTI:0820) */
    if (memcmp(mti, DEF_UP_MTI_0820_REQ, strlen(DEF_UP_MTI_0820_REQ))==0) {
        if (ffd_0820->b070_nw_mng_code.ffd_header.m_flg_exist) {
            /* 開局要求 */
            if (memcmp(ffd_0820->b070_nw_mng_code.ffd_data, DEF_UP_F70_INFOCODE_001_OPN, strlen(DEF_UP_F70_INFOCODE_001_OPN))==0) {
                ctlkind[0] = DEF_CTLFNC_CNT_STS_ECH;    // 1:局状態・エコー制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_CNT_OPN;        // 1:開局(サインオン)
                ctlkind[3] = DEF_CTLINT_NORMAL;         // 1:通常
                return DEF_MSJ_NORMAL;
            }
            /* 閉局要求 */
            if (memcmp(ffd_0820->b070_nw_mng_code.ffd_data, DEF_UP_F70_INFOCODE_002_CLS, strlen(DEF_UP_F70_INFOCODE_002_CLS))==0) {
                ctlkind[0] = DEF_CTLFNC_CNT_STS_ECH;    // 1:局状態・エコー制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_CNT_CLS;        // 2:閉局(サインオフ)
                ctlkind[3] = DEF_CTLINT_NORMAL;         // 1:通常
                return DEF_MSJ_NORMAL;
            }
            /* エコーテスト要求 */
            if (memcmp(ffd_0820->b070_nw_mng_code.ffd_data, DEF_UP_F70_INFOCODE_301_ECH, strlen(DEF_UP_F70_INFOCODE_301_ECH))==0) {
                ctlkind[0] = DEF_CTLFNC_CNT_STS_ECH;    // 1:局状態・エコー制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_ECH_SND;        // 3:エコーテスト
                ctlkind[3] = DEF_CTLINT_NORMAL;         // 1:通常
                return DEF_MSJ_NORMAL;
            }
            /* カットオーバー開始要求 */
            if (memcmp(ffd_0820->b070_nw_mng_code.ffd_data, DEF_UP_F70_INFOCODE_201_CUTBEG, strlen(DEF_UP_F70_INFOCODE_201_CUTBEG))==0) {
                ctlkind[0] = DEF_CTLFNC_CUT_OVER;       // 3:カットオーバー制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_CUT_START;      // 6:カットオーバー
                ctlkind[3] = DEF_CTLINT_NORMAL;         // 1:通常
                return DEF_MSJ_NORMAL;
            }
            /* カットオーバー終了要求 */
            if (memcmp(ffd_0820->b070_nw_mng_code.ffd_data, DEF_UP_F70_INFOCODE_202_CUTEND, strlen(DEF_UP_F70_INFOCODE_202_CUTEND))==0) {
                ctlkind[0] = DEF_CTLFNC_CUT_OVER;       // 3:カットオーバー制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_CUT_END;        // 7:カットオーバー
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

    /* 制御電文種別判定(MTI:0800) */
    if (memcmp(mti, DEF_UP_MTI_0800_KEYREQ, strlen(DEF_UP_MTI_0800_KEYREQ))==0) {
        if (ffd_0800->b070_nw_mng_code.ffd_header.m_flg_exist) {
            if (memcmp(ffd_0800->b070_nw_mng_code.ffd_data, DEF_UP_F70_INFOCODE_101_KEY, strlen(DEF_UP_F70_INFOCODE_101_KEY))==0) {
                ctlkind[0] = DEF_CTLFNC_KEY_EXCH;       // 2:鍵交換制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_KEY_EXC;        // 5:鍵交換
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

