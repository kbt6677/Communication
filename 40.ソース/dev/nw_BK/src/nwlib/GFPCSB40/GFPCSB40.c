/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSB40                                    */
/*        FUNCTION          ････ NW個別(電文種別判定[BANKNET])               */
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
#include "common.h"
#include "GFPCSB40.h"
#include "vproc.h"

/****************************************************************************/
/*  FUNCTION        : 1.1.0  電文種別判定処理                               */
/*  CALLING SEQ.    : short NWM_MSJ(char*, char*, char*)                    */
/*  ARGUMENT        : 1.mti            (I)   MTI                            */
/*                  : 2.fixdata        (I)   固定フォーマットデータ         */
/*                  : 3.ctlkind        (O)   制御電文種別                   */
/*  RETURN CODE     : 0:正常 1:エラー                                       */
/*  DESCRIPTION     : NW個別(電文種別判定[BANKNET])                         */
/****************************************************************************/
short   NWM_MSJ(char *mti, char *fixdata, char *ctlkind)
{
    fixedform_banknet_0800_def  *ffd_0800 = (fixedform_banknet_0800_def*)fixdata;
    fixedform_banknet_0810_def  *ffd_0810 = (fixedform_banknet_0810_def*)fixdata;
    memset(ctlkind, 0x20, 4);

    /* 制御電文種別判定(MTI:0800) */
    if (memcmp(mti, DEF_BK_MTI_0800_REQ, strlen(DEF_BK_MTI_0800_REQ))==0) {
    	// BIT70設定確認
        if (ffd_0800->b070_nw_mng_code.ffd_header.m_flg_exist) {
            /* エコーテスト要求 */
            if (memcmp(ffd_0800->b070_nw_mng_code.ffd_data, DEF_BK_F70_INFOCODE_270_ECH, strlen(DEF_BK_F70_INFOCODE_270_ECH))==0) {
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

