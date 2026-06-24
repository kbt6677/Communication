/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSV40                                    */
/*        FUNCTION          ････ NW個別(電文種別判定[VISANET])               */
/*                                                                           */
/*        AUTHER            ････ HAS K.F                                     */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025-04-17                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  K.F        2025/04/17 新規作成                                      */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*   ｺﾝﾊﾟｲﾙｵﾌﾟｼｮﾝ                                                            */
/*****************************************************************************/
#pragma ENV COMMON

/*****************************************************************************/
/*   INCLUDE定義                                                             */
/*****************************************************************************/
/* SYSTEM HEADER   */
#include <stdio.h> nolist
#include <stdlib.h> nolist
#include <string.h> nolist
#include <stdbool.h> nolist

/* COMMON HEADER   */
#include "common.h"
#include "ems.h"
#include "file.h"

/* USER HEADER     */
#include "msg_VI.h"
#include "GFPCSV40.h"
#include "vproc.h"

/*****************************************************************************/
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
    fixedform_visanet_0800_def  *ffd_0800 = (fixedform_visanet_0800_def*)fixdata;
    fixedform_visanet_0810_def  *ffd_0810 = (fixedform_visanet_0810_def*)fixdata;
    ctltext_type_def            *ctltype  = (ctltext_type_def *)ctlkind;
    memset((char *)ctltype, 0x20, 4);

    /* 制御電文種別判定(MTI:1804) */
    if (memcmp(mti, DEF_VI_MTI_0800_REQ, strlen(DEF_VI_MTI_0800_REQ))==0) {
        if (ffd_0800->b70.ffd_header.m_flg_exist == true) {
            /* 開局要求 */
            if (memcmp(ffd_0800->b70.ffd_data, DEF_VI_F70_INFOCODE_071_SON, strlen(DEF_VI_F70_INFOCODE_071_SON)) == 0) {
                ctlkind[0] = DEF_CTLFNC_CNT_STS_ECH;    // 1:局状態・エコー制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_CNT_OPN;        // 1:開局(サインオン)
                ctlkind[3] = DEF_CTLINT_NORMAL;         // 1:通常
                return DEF_MSJ_NORMAL;
            }
            /* 閉局要求 */
            if (memcmp(ffd_0800->b70.ffd_data, DEF_VI_F70_INFOCODE_072_SOF, strlen(DEF_VI_F70_INFOCODE_072_SOF)) == 0) {
                ctlkind[0] = DEF_CTLFNC_CNT_STS_ECH;    // 1:局状態・エコー制御
                ctlkind[1] = DEF_CTLMSG_REQUEST;        // 1:要求
                ctlkind[2] = DEF_CTLTXT_CNT_CLS;        // 2:閉局(サインオフ)
                ctlkind[3] = DEF_CTLINT_NORMAL;         // 1:通常
                return DEF_MSJ_NORMAL;
            }
            /* エコーテスト要求 */
            if (memcmp(ffd_0800->b70.ffd_data, DEF_VI_F70_INFOCODE_301_ECH, strlen(DEF_VI_F70_INFOCODE_301_ECH)) == 0) {
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

