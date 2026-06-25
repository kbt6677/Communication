/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSJH0                                    */
/*        FUNCTION          ････ NW個別モジュール                            */
/*                               電文送信時局状態判定                        */
/*        AUTHER            ････ HAS T.Sugisaki                              */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2024-12-04                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  T.Sugisaki 2024/12/04 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
#ifndef _NWM_NTE_H
#define _NWM_NTE_H

// 送信電文種別長
#define DEF_SEND_DEN_TYP_LEN    2

// 
#define DEF_HDE_SEND_OK         1    // 送信可
#define DEF_HDE_SEND_NG         0    // 送信不可
#define DEF_HDE_SEND_TYP_ERR    2    // 電文種別異常
#define DEF_HDE_SEND_CEN_ERR    3    // 局状態異常

/* プロトタイプ宣言 */
short NWM_NTE(char, char,char *);

#endif

