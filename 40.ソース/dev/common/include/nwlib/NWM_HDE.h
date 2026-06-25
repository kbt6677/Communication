/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSJ30                                    */
/*        FUNCTION          ････ NW個別モジュール・ヘッダー                  */
/*                               電文ヘッダ編集(CARDNET)                     */
/*        AUTHER            ････ HAS T.Fukunaga                              */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2024-10-01                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  T.Fukunaga 2024/10/01 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
#ifndef _NWM_HDE_H
#define _NWM_HDE_H


/* typedef定義 */
#pragma fieldalign shared2 __NWM_HDE_arg_1
typedef struct __NWM_HDE_arg_1
{
   char    *msg_in_addr;
   short    msg_in_len;
} NWM_HDE_arg_1_def;

#pragma fieldalign shared2 __NWM_HDE_arg_2
typedef struct __NWM_HDE_arg_2
{
   char    *msg_out_addr;
   short    msg_out_len;
} NWM_HDE_arg_2_def;

typedef struct __MSG_COMMON
{
    short   msg_data_len;    /* 電文長     */
    char    msg_data[10000]; /* 電文       */
} MSG_COMMON_def;

/* プロトタイプ宣言 */
void  NWM_HDE(NWM_HDE_arg_1_def *
	        , NWM_HDE_arg_2_def *
	        , short
	        , short );

#endif

