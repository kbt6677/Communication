/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSX00                                    */
/*        FUNCTION          ････ NW個別モジュール・ヘッダー                  */
/*                               ヘッダーレイアウトチェック処理              */
/*        AUTHER            ････ HAS T.Fukunaga                              */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2024-10-01                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  T.Fukunaga 2024/10/01 (J0680)新規作成                               */
/*  1.1  S.Kimura   2025/01/27 (J0680)VisaNet用定義追加                      */
/*                                                                           */
/*****************************************************************************/
#ifndef _NWM_MTI_H
#define _NWM_MTI_H

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define  NWM_DATA_START_POSI         1               /* 電文開始位置        */

#define  DEF_NWM_MTI_RTN_GENERAL     1               /* RETURN 一般電文     */
#define  DEF_NWM_MTI_RTN_REJECT      2               /* RETURN リジェクト   */
#define  DEF_NWM_MTI_RTN_HEARTBEAT   3               /* RETURN ハートビート */
#define  DEF_NWM_MTI_RTN_IDLE        4               /* RETURN アイドル     */
#define  DEF_NWM_MTI_RTN_INVALID    -1               /* RETURN 電文不正     */

/* typedef定義 */

/* プロトタイプ宣言 */
short NWM_MTI(char *,short,short,short *,char *,short *);

#endif

