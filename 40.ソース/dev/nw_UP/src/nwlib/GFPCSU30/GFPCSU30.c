/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSU30                                    */
/*        FUNCTION          ････ NW個別モジュール                            */
/*                               電文ヘッダ編集(UnionPay)                    */
/*                                 指定された編集前電文を編集せずに          */
/*                                 編集後電文に設定を行う。                  */
/*        AUTHER            ････ HAS H.Mizuno                                */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-08-15                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  H.Mizuno   2025/08/15 (J0680)新規作成                               */
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
#include <stdbool.h>  nolist
#include <stdio.h>    nolist
#include <stdlib.h>   nolist
#include <string.h>   nolist
#include <cextdecs.h> nolist
#include <tal.h>      nolist

/* USER HEADER     */
#include "NWM_HDE.h"
/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
/****************************************************************************/
/*   内部関数宣言                                                           */
/****************************************************************************/

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_HDE                                        */
/*  CALLING SEQ.    : void   NWM_HDE(strcut *, struct *, short, short)      */
/*  ARGUMENT        : 1.msg_in         (I)   編集前電文情報                 */
/*                  : 2.msg_out        (O)   編集後電文情報                 */
/*                  : 3.start_position (I)   電文開始位置                   */
/*                  : 4.send_msg_type  (I)   送信電文種別                   */
/*  RETURN CODE     :                                                       */
/*  DESCRIPTION     : 電文ヘッダの編集を行う                                */
/****************************************************************************/
void  NWM_HDE(NWM_HDE_arg_1_def *msg_in
            , NWM_HDE_arg_2_def *msg_out
            , short              start_position
            , short              send_msg_type)
{
    char  wbuf[10];
typedef struct __msg_out_DEF
{
    char                    len[4];             // 電文長
    char                    msg_out_buf[2];    // 電文フォーマット
} msg_out_DEF;
    msg_out_DEF  *lp_msg_out;
    lp_msg_out  = (msg_out_DEF *)msg_out->msg_out_addr;

    snprintf(wbuf, sizeof(wbuf), "%04d", msg_in->msg_in_len);
    memcpy(lp_msg_out->len, wbuf,4);

    memcpy(lp_msg_out->msg_out_buf ,msg_in->msg_in_addr ,msg_in->msg_in_len);
    msg_out->msg_out_len = msg_in->msg_in_len + 4;
    return;
}
/* end of NWM_HDE */
