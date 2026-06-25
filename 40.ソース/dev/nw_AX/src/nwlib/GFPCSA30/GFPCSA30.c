/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSA30                                    */
/*        FUNCTION          ････ NW個別モジュール                            */
/*                               電文ヘッダ編集(AEGN)                        */
/*                                 指定された編集前電文に電文長を付与し      */
/*                                 編集後電文に設定を行う。                  */
/*        AUTHER            ････ HAS S.Kimura                                */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-06-10                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  S.Kimura   2025/07/24 (J0680)新規作成                               */
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
#include "vproc.h"

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
    MSG_COMMON_def  *msg_common = (MSG_COMMON_def *)msg_out->msg_out_addr;

    /* メッセージヘッダのMessage Lengthを追加 */
    msg_common->msg_data_len = msg_in->msg_in_len + sizeof(msg_common->msg_data_len);
    memcpy(msg_common->msg_data ,msg_in->msg_in_addr ,msg_in->msg_in_len);
    msg_out->msg_out_len = msg_common->msg_data_len;
    return;
}
/* end of NWM_HDE */
