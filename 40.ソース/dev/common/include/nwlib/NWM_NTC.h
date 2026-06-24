/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSxC0                                    */
/*        FUNCTION          ････ NW個別(通知電文個別処理)                    */
/*        AUTHER            ････ HAS Yumoto                                  */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-04-14                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS 湯本   2025/04/14 (通知電文個別処理(CARDNET))新規作成           */
/*                                                                           */
/*****************************************************************************/
#ifndef _NWM_NTC_H
#define _NWM_NTC_H

/* 通知電文精査（NWM_NTC_msg_check）戻り値 */
#define     DEF_RTN_RECEIVE_NOTICE         1                /* 通知電文       */
#define     DEF_RTN_RECEIVE_REQUEST        2                /* 要求応答型要求電文 */
#define     DEF_RTN_RECEIVE_RESPONSE       3                /* 要求応答型応答電文 */
#define     DEF_RTN_ABNORMAL_SCRUTINY      9                /* 精査異常       */

/* 通知電文編集（NWM_NTC_msg_edit）戻り値 */
#define     DEF_RTN_NO_RESPONSE_MSG        1                /* 応答電文なし   */
#define     DEF_RTN_WITH_RESPONSE_MSG      2                /* 応答電文あり   */
#define     DEF_RTN_EDIT_ERROR             9                /* 編集エラー     */

/* プロトタイプ宣言 */
short NWM_NTC_msg_check(char *, char *, short, short, db_gfnwi_def *);  /* 通知電文精査 */
short NWM_NTC_msg_edit(char *, short, db_gfnwi_def *,                   /* 通知電文編集 */
                       char *, short, short *, char * );

#endif
