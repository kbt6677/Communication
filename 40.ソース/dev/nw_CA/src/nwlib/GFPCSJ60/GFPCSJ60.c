/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSJ60                                    */
/*        FUNCTION          ････ NW個別(電文ヘッダ電文長編集[CARDNET])       */
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
#include <stdio.h> nolist

/* USER HEADER     */
#include "vproc.h"
#include "NWM_HDL.h"
#include "GFPCSJ60.h"

short CHAR2BCD(unsigned char *, short len, unsigned char *);

/****************************************************************************/
/*  FUNCTION        : 1.1.0  電文ヘッダ電文長編集処理                       */
/*  CALLING SEQ.    : short NWM_HDL(char*, short)                           */
/*  ARGUMENT        : 1.textbuf        (I/O) 電文バッファ                   */
/*                  : 2.textlen        (I)   ISO8583電文長                  */
/*  RETURN CODE     : 0:正常 1:エラー                                       */
/*  DESCRIPTION     : NW個別(電文ヘッダ電文長編集[CARDNET])                 */
/****************************************************************************/
short NWM_HDL(char *textbuf, short textlen)
{
    msg_cardnet_def *nwmsg = (msg_cardnet_def*)textbuf;
    unsigned char   wkbuf[8];
    unsigned char   bodylen[8];
    unsigned char   wholelen[8];

    /* BODY部電文長 */
    memset(wkbuf, 0x00, sizeof(wkbuf));
    sprintf(wkbuf, "%04d", textlen);
    memset(bodylen, 0x00, sizeof(bodylen));
    if (!CHAR2BCD(wkbuf, sizeof(nwmsg->header.bh_body_len)*2, bodylen)) {
        return DEF_HDL_ERROR;
    }

    /* 全体電文長(ヘッダ部+BODY部) */
    memset(wkbuf, 0x00, sizeof(wkbuf));
    sprintf(wkbuf, "%04d", (textlen + MSG_HEADER_CARDNET_def_Size));
    memset(wholelen, 0x00, sizeof(wholelen));
    if (!CHAR2BCD(wkbuf, sizeof(nwmsg->header.ctrl_msg_len)*2, wholelen)) {
        return DEF_HDL_ERROR;
    }

    /* ヘッダ情報更新 */
    memcpy(nwmsg->header.bh_body_len, bodylen, sizeof(nwmsg->header.bh_body_len));
    memcpy(nwmsg->header.ctrl_msg_len, wholelen, sizeof(nwmsg->header.ctrl_msg_len));

    return DEF_HDL_NORMAL;
} /* end of NWM_HDL */

