/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCGXA0                                    */
/*        FUNCTION          ････ 共通モジュール・ヘッダー                    */
/*                               トランザクション管理モジュール              */
/*                                                                           */
/*        AUTHER            ････ HAS T.Fukunaga                              */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2024-10-01                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  T.Fukunaga 2024/10/01 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
#ifndef _GFPCGXA0_H
#define _GFPCGXA0_H

#define DEF_COM_TMF_BEGIN     "BT  "
#define DEF_COM_TMF_END       "ET  "
#define DEF_COM_TMF_ABORT     "AT  "
#define DEF_COM_TMF_RESUME    "RT  "
#define DEF_COM_TMF_CLEAR     "RTC "
#define DEF_COM_TMF_GET       "GTID"
#define DEF_COM_TMF_ACTIVE    "RTTX"

/* typedef定義 */

/* プロトタイプ宣言 */
short COM_TMF(char *, long *, char *);

#endif

