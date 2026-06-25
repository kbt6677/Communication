/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCGXC0                                    */
/*        FUNCTION          ････ 共通モジュール・ヘッダー                    */
/*                               サーバー停止判定モジュール                  */
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
#ifndef _GFPCGXC0_H
#define _GFPCGXC0_H

#define   DEF_COM_STP_RTN_OK_NO_STOP   0             /* 戻り値 正常(停止無)     */
#define   DEF_COM_STP_RTN_OK_STOP      1             /* 戻り値 正常(停止有)     */
#define   DEF_COM_STP_RTN_NG_TBL_FULL -1             /* 戻り値 異常(TBL FULL)   */


/* typedef定義 */
#pragma fieldalign shared2 __COM_STP_arg_1
typedef struct __COM_STP_arg_1
{
   short     ancst_handle[10];
   struct
     {
      char    node_name[8];
      short   node_open_count;
      short   cpu_open_count[16];
     } openers[10];
} COM_STP_arg_1_def;


/* プロトタイプ宣言 */
void  COM_STP_INIT (COM_STP_arg_1_def *);
short COM_STP_JUDGE(COM_STP_arg_1_def *, char *);

#endif

