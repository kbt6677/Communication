/**
 * @brief GFPCVX80.h 
 *
 * @date 2025/03/06 新規作成 by HAS
 *
*/
/* STANDARD HEADER */

/* USER HEADER     */
#include "vproc.h"
#include "GFPCGX40.h"
#include "GFPCGX90.h"


#ifndef _GFPCVX81_H_
#define _GFPCVX81_H_

/* ------------------------------------------------------------------------------------------ */
/* グローバルデータ                                                                           */
/* ------------------------------------------------------------------------------------------ */
    t_kbt_file_data    g_kbt_file_data;                   /* 個別ファイル情報                 */
    t_kbt_svrcls_data  g_kbt_svrcls;                      /* 個別ServerClass情報              */
    db_gfnws_def       g_db_gfnws_tbl[4];                 /* 接続先固有情報ファイルテーブル   */
    t_gfnws_pkey_def   g_gfnws_pkey;                      /* 接続先固有情報ファイルPkey       */
    db_gcsst_def       g_gcsst;                           /* 局状態管理ファイルレコード       */
    db_gcest_def       g_gcest;                           /* エコー状態管理ファイルレコード   */
    t_gcest_pkey_def   g_gcest_pkey;                      /* エコー状態管理ファイルPKey       */
    db_glmlg_def       g_glmlg;                           /* 制御電文ログレコード             */
    COM_SDT_arg_2_def  g_com_sdt_arg_2;                   /* システム日時取得 arg2            */
    COM_SDT_arg_3_def  g_com_sdt_arg_3;                   /* システム日時取得 arg3            */
    short              g_son_retry;                       /* 開局リトライ有無                 */
/* ------------------------------------------------------------------------------------------ */
/* 関数のプロトタイプ宣言                                                                     */
/* ------------------------------------------------------------------------------------------ */
void    CSTE_handle_req_msg_c401   (t_rcv_info_def*);     /* NW電文受信要求処理               */
short    CSTE_sonof_hsm_req_rcv    (t_rcv_info_def*);     /* 開閉局_被仕向要求受信処理        */
short    CSTE_sonof_sim_rsp_rcv    (t_rcv_info_def*);     /* 開閉局_仕向応答受信処理          */
short    CSTE_sonof_sim_rsp_timeout(t_rcv_info_def*);     /* 開閉局_仕向応答待ちTimeout処理   */
short    CSTE_sonof_sim_req_snd_err(t_rcv_info_def*);     /* 開閉局_仕向要求送信不可処理      */
short    CSTE_sonof_hsm_rsp_snd_err(t_rcv_info_def*);     /* 開閉局_被仕向応答送信不可処理    */
short    CSTE_echo_hsm_req_rcv     (t_rcv_info_def*);     /* エコー_被仕向要求受信処理        */
short    CSTE_echo_sim_rsp_rcv     (t_rcv_info_def*);     /* エコー_仕向応答受信処理          */
short    CSTE_echo_sim_rsp_timeout (t_rcv_info_def*);     /* エコー_仕向応答待ちTimeout処理   */
short    CSTE_echo_sim_req_snd_err (t_rcv_info_def*);     /* エコー_仕向要求送信不可処理      */
short    CSTE_echo_hsm_rsp_snd_err (t_rcv_info_def*);     /* エコー_被仕向応答送信不可処理    */
short    CSTE_r401_edit            (t_rcv_info_def*);     /* NW電文受信応答IPC(R401)編集処理  */
void    CSTE_handle_req_msg_c402   (t_rcv_info_def*);     /* 制御電文作成要求処理             */
short    CSTE_sonof_sim_req_snd    (t_rcv_info_def*);     /* 開閉局_仕向要求送信処理          */
short    CSTE_echo_sim_req_snd     (t_rcv_info_def*);     /* エコー_仕向要求送信処理          */
short    CSTE_r402_edit            (t_rcv_info_def*);     /* 制御電文応答IPC(R402)編集処理    */
short    CSTE_is_auto_signon       (void);                /* 自動開局判定                     */
void    CSTE_send_signon           (t_rcv_info_def*);     /* 開局コマンド送信                 */
short   CSTE_read_gfnwi            (t_rcv_info_def*);     /* NW情報ファイル読込処理           */
short   CSTE_read_gfnws            (t_rcv_info_def*);     /* 接続先固有情報ファイル取得処理   */
short   CSTE_read_gfnws_io         (t_rcv_info_def*);     /* 接続先固有情報ファイルI/O処理    */
short   CSTE_read_gcsst            (short                 /* 局状態管理ファイル読込み処理     */
                                   ,t_rcv_info_def*);
short   CSTE_update_gcsst          (t_rcv_info_def*);     /* 局状態管理ファイル更新処理       */
short   CSTE_unlock_gcsst          (t_rcv_info_def*);     /* 局状態管理ファイルUNLOCK         */
short   CSTE_update_gcest          (t_rcv_info_def*);     /* エコー態管理ファイル更新処理     */
short   CSTE_read_glmlg            (short, short, char    /* 制御電文ログ取得処理             */
                                   ,t_rcv_info_def*);
short   CSTE_put_glmlg             (short                 /* 制御電文ログ出力処理             */
                                   ,t_rcv_info_def*);
short   CSTE_update_glmlg          (char, short           /* 制御電文ログ更新処理             */
                                   ,t_rcv_info_def*);
short   CSTE_get_lcn               (t_rcv_info_def*);     /* GFP内部LCN取得処理               */
short   CSTE_get_sysnum            (t_rcv_info_def*);     /* システム通番取得処理             */
void    CSTE_error_msg_out         (t_rcv_info_def*);     /* エラー出力ログ出力処理           */

#endif
