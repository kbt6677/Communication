/* GFPOGGZ4 トレース出力モジュール */
#ifndef GFPOGGZ4_H
#define GFPOGGZ4_H

#include "DDLCCLC.h(lk_zac2001i_arg_1)" nolist // IPCトレース
#include "DDLCCLC.h(lk_zac2001p_arg_1)" nolist // PATHSENDトレース
#include "DDLCCLC.h(lk_zac2001r_arg_1)" nolist // $RECEIVE完了トレース
#include "DDLCCLC.h(lk_zac2001t_arg_1)" nolist // TMFトレース
#include "DDLCCLC.h(lk_zac2001f_arg_1)" nolist // ファイルトレース

/* -------------------------------------------------------*/
/* 関数プロトタイプ宣言                                   */
/* -------------------------------------------------------*/

// トレース出力
short TRACEOUT(char*);

/* -------------------------------------------------------*/
/* 定数定義                                               */
/* -------------------------------------------------------*/

// 機能フラグ
#define     DEF_TRACE_FUNC_INI          '0'             // 初期処理
#define     DEF_TRACE_FUNC_OUT          '1'             // 出力処理
#define     DEF_TRACE_FUNC_END          '2'             // 終了処理

#endif
