/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSN40                                    */
/*        FUNCTION          ････ NW個別(電文種別判定[NYCE])                  */
/*                                                                           */
/*        AUTHER            ････ HAS H.Mizuno                                */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025-07-31                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  Mizuno     2025/07/31 (J0680)新規作成                               */
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
#include "msg_NY.h"

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
    fixedform_nyce_0800_def  *ffd_0800 = (fixedform_nyce_0800_def*)fixdata;
    fixedform_nyce_0810_def  *ffd_0810 = (fixedform_nyce_0810_def*)fixdata;
    memset(ctlkind, 0x20, 4);

    /* 制御電文種別判定(MTI:0800) */
    if (memcmp(mti, DEF_NY_MTI_0800_REQ, strlen(DEF_NY_MTI_0800_REQ))==0) {
        if (ffd_0800->b070_nw_mng_code.ffd_header.m_flg_exist) {
            /* 開局要求               */
            if (memcmp(ffd_0800->b070_nw_mng_code.ffd_data, DEF_NY_F70_INFOCODE_071_SON, strlen(DEF_NY_F70_INFOCODE_071_SON))==0) {
                ctlkind[0] = DEF_CTLFNC_CNT_STS_ECH;    // 1:局状態・エコー制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_CNT_OPN;        // 1:開局(サインオン)
                ctlkind[3] = DEF_CTLINT_NORMAL;         // 1:通常
                return DEF_MSJ_NORMAL;
            } 
            /* 閉局要求               */
            if (memcmp(ffd_0800->b070_nw_mng_code.ffd_data, DEF_NY_F70_INFOCODE_072_SOF, strlen(DEF_NY_F70_INFOCODE_072_SOF))==0) {
                ctlkind[0] = DEF_CTLFNC_CNT_STS_ECH;    // 1:局状態・エコー制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_CNT_CLS;        // 2:閉局(サインオフ)
                ctlkind[3] = DEF_CTLINT_NORMAL;         // 1:通常
                return DEF_MSJ_NORMAL;
            } 
            /* エコーテスト要求       */
            if (memcmp(ffd_0800->b070_nw_mng_code.ffd_data, DEF_NY_F70_INFOCODE_371_ECH, strlen(DEF_NY_F70_INFOCODE_371_ECH))==0) {
                ctlkind[0] = DEF_CTLFNC_CNT_STS_ECH;    // 1:局状態・エコー制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_ECH_SND;        // 3:エコーテスト
                ctlkind[3] = DEF_CTLINT_NORMAL;         // 1:通常
                return DEF_MSJ_NORMAL;
            } 
            /* 鍵交換要求             */
            if (memcmp(ffd_0800->b070_nw_mng_code.ffd_data, DEF_NY_F70_INFOCODE_171_KEY, strlen(DEF_NY_F70_INFOCODE_171_KEY))==0) {
                ctlkind[0] = DEF_CTLFNC_KEY_EXCH;       // 2:鍵交換制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_KEY_EXC;        // 5:鍵交換
                ctlkind[3] = DEF_CTLINT_NORMAL;         // 1:通常
                return DEF_MSJ_NORMAL;
            } 
            /* カットオーバー開始要求 */
            if (memcmp(ffd_0800->b070_nw_mng_code.ffd_data, DEF_NY_F70_INFOCODE_271_EODBEG, strlen(DEF_NY_F70_INFOCODE_271_EODBEG))==0) {
                ctlkind[0] = DEF_CTLFNC_CUT_OVER;       // 3:カットオーバー制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_CUT_START;      // 6:EOD開始
                ctlkind[3] = DEF_CTLINT_NORMAL;         // 1:通常
                return DEF_MSJ_NORMAL;
            } 
            /* カットオーバー終了要求 */
            if (memcmp(ffd_0800->b070_nw_mng_code.ffd_data, DEF_NY_F70_INFOCODE_272_EODEND, strlen(DEF_NY_F70_INFOCODE_272_EODEND))==0) {
                ctlkind[0] = DEF_CTLFNC_CUT_OVER;       // 3:カットオーバー制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_CUT_END;        // 7:EOD終了
                ctlkind[3] = DEF_CTLINT_NORMAL;         // 1:通常
                return DEF_MSJ_NORMAL;
            } 
            /* 不明                   */
            return DEF_MSJ_ERROR;
        } else {
            /* 不明 */
            return DEF_MSJ_ERROR;
        }
    }
    /* その他MTI */
    return DEF_MSJ_ERROR;
} /* end of NWM_MSJ */

