/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSJ70                                    */
/*        FUNCTION          ････ 局状態・エコー制御                          */
/*                               開局・閉局・エコー制御のNW個別処理(VISANET) */
/*                               処理を行う                                  */
/*        AUTHER            ････ HAS K.F                                     */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-04-21                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS K.F    2025/04/21 新規作成                                      */
/*                                                                           */
/*****************************************************************************/
/* USER HEADER     */
#include "NWM_STE.h"                           /* NW個別(開局・閉局・エコー個別処理) */

#ifndef _GFPCVX70_H
#define _GFPCVX70_H

/*****************************************************************************/
/*   INCLUDE定義                                                             */
/*****************************************************************************/
#include "msg_VI.h"

/*****************************************************************************/
/* EFINE 定義                                                                */
/*****************************************************************************/
#define DEF_RTN_NORMAL            0          // 正常
#define DEF_RTN_ERRCD            -1          // 異常
#define DEF_RTN_CHKOK             0          // 正常（精査OK）
//#define DEF_RTN_CHKNG_REJECT      1          // 精査エラー（拒否応答）
//#define DEF_RTN_CHKNG_FAULT       2          // 精査エラー（障害電文通知）
//#define DEF_RTN_CHKNG_DISPOSAL    3          // 精査エラー（廃棄）
#define DEF_MSG_HEADER_LEN        0x16       // 制御電文固定部ヘッダ長(22)
#define DEF_UTC_DATE              1          // グリニッジ標準時
//#define DEF_AUTHOR_ANS            0          // 許可応答
//#define DEF_REJECT_ANS            1          // 拒否応答
//#define DEF_DISPOS_ANS            2          // 電文破棄
#define DEF_DEPT_OPEN             0x31       // 制御電文 開局
#define DEF_DEPT_CLOS             0x32       //          閉局
#define DEF_DEPT_ECHO             0x33       //          エコーテスト
//#define DEF_STATE_OPEN            "10"       // 局状態 開局
//#define DEF_STATE_CLOS            "90"       //        閉局
//#define DEF_STATE_OPENPROC        "11"       //        開局処理中
//#define DEF_STATE_CLOSPROC        "91"       //        開局処理中
#define DEF_STATE_SPACE           "  "       // 局状態スペース
#define DEF_STATE_IOPEN           10         // 局状態(数値型) 開局
#define DEF_STATE_ICLOS           90         //                閉局
#define DEF_STATE_IOPENPROC       11         //                開局処理中
#define DEF_STATE_ICLOSPROC       91         //                開局処理中
#define DEF_ANSTYPE_AUTHOR        0x41       // 許可応答 "A"
#define DEF_NONOPTION             0x30       // オプション無
#define DEF_ENFORCEXE             0x31       // 強制実行
#define DEF_STSUPONLY             0x32       // 状態更新のみ
//#define DEF_ACCEPT_TRN            0          // コマンド受付可(電文送信あり)
//#define DEF_ACCEPT_NON            1          // コマンド受付可(電文送信なし)
//#define DEF_NOT_ACCEPT            2          // コマンド受付不可
//#define DEF_MANEGFL_ISUP          0          // 管理ファイル更新あり
//#define DEF_MANEGFL_NOUP          1          // 管理ファイル更新無し
#define DEF_REQTYP_DESTANS        "20"       // 仕向け応答
#define DEF_REQTYP_ANSTIMO        "30"       // 仕向け応答タイムアウト
#define DEF_REQTYP_SENDNG         "40"       // 仕向け応答送信不可
#define DEF_DESTANS_OK            0          // 仕向け応答OK
#define DEF_DESTANS_NG            1          //
#define DEF_INSIDE_RSPCD          "00"       // 内部レスポンスコード 
#define DEF_CTLTYP_STRA           'A'       //
#define DEF_CTLTYP_STRB           'B'        //

                                             //
/* OUTPUT EMS MESSAGE */
#define DEF_ERREMS_HEADLEN             "Header-Len"        // ヘッダ長チェックエラー
#define DEF_ERREMS_MSGLEN              "Message-Len"       // メッセージ長チェックエラー
#define DEF_ERREMS_MTICODE             "Mti-CODE"          // MTIチェックエラー
#define DEF_ERREMS_B07_DATEFRM         "B07-Date-Format"   // BIT07 DATE FORMATエラー
#define DEF_ERREMS_B07_FALSE           "B07-Elm-False"     // BIT07 エレメントチェックエラー
#define DEF_ERREMS_B11_NUMERIC         "B11-Non-Numeric"   // BIT11 ニューメリックチェックエラー
#define DEF_ERREMS_B11_REQMISSMATCH    "B11-Req-Missmatch" // BIT11 要求電文固定フォーマット不一致
#define DEF_ERREMS_B11_FALSE           "B11-Elm-False"     // BIT11 エレメントチェックエラー
#define DEF_ERREMS_B39_FALSE           "B39-Elm-False"     // BIT39 エレメントチェックエラー
#define DEF_ERREMS_B70_FALSE           "B70-Elm-False"     // BIT70 エレメントチェックエラー

#endif

