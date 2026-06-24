/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCGX90                                    */
/*        FUNCTION          ････ 共通モジュール・ヘッダー                    */
/*                               ユニーク日時取得モジュール                  */
/*                                                                           */
/*        AUTHER            ････ HAS T.Fukunaga                              */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2024-10-01                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  T.Fukunaga 2024/10/01 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
#ifndef _GFPCGX90_H
#define _GFPCGX90_H

/* typedef定義 */
#pragma fieldalign shared2 __COM_UNQ_arg_1
typedef struct __COM_UNQ_arg_1
{
   char    yyyy[4];
   char    mm[2];
   char    dd[2];
   char    hh[2];
   char    md[2];
   char    ss[2];
   char    ms[3];
   char    cc[3];
} COM_UNQ_arg_1_def;


/* プロトタイプ宣言 */
short COM_UNQ(COM_UNQ_arg_1_def *, char *);

#endif

