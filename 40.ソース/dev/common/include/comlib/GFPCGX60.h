/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCGX60                                    */
/*        FUNCTION          ････ 共通モジュール・ヘッダー                    */
/*                               IOタグ生成・解析モジュール                  */
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
/****************************************************************************/
#ifndef _GFPCGX60_H
#define _GFPCGX60_H


/* プロトタイプ宣言 */
short COM_TGM(unsigned short, unsigned short,   unsigned short  , unsigned long *);
void  COM_TGA(unsigned long,  unsigned short *, unsigned short *, unsigned short *);

#endif

