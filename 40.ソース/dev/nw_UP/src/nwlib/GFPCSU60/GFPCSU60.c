/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSU60                                    */
/*        FUNCTION          ････ NW個別(電文ヘッダ電文長編集）               */
/*        AUTHER            ････ HAS Matsumoto                               */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-09-16                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  Matsumoto  2025/09/16 (J0680)新規作成                               */
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
#include <stdio.h>
#include <string.h> nolist

/* COMMON HEADER   */
#include  "msg_UP.h"
#include "NWM_HDL.h"
#include  "vproc.h"

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  NWM_HDL                                         */
/*  DESCRIPTION     : 電文ヘッダ電文長編集[VisaNet]                          */
/*                                                                           */
/*  CALLING SEQ.    : short  NWM_HDL()                                       */
/*  ARGUMENT        : *telegram_p      : 電文格納域（バッファ）アドレス      */
/*                    length           : ISO8583電文長                       */
/*                                                                           */
/*  RETURN CODE     : 0:正常終了  9:異常終了                                 */
/*****************************************************************************/
short NWM_HDL(char *telegram_p,short length)
{
    char     wkbuf[10];
    MSG_HEADER_UNIONPAY_def *msghead_p;

    msghead_p = (MSG_HEADER_UNIONPAY_def *)telegram_p;

    /* ISO8583電文長+MessageHeader長をASCII文字4桁に変換して設定 */
    memset(wkbuf, 0x00, sizeof(wkbuf));
    sprintf(wkbuf, "%04d", (length + MSG_HEADER_UNIONPAY_def_Size));
    memcpy(msghead_p->mh_tot_len, wkbuf, sizeof(msghead_p->mh_tot_len));

    return(DEF_HDL_NORMAL);
}


