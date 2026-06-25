/**
 * @brief GFPCVX80.h 
 *
 * @date 2025/03/06 新規作成 by HAS
 *
*/
/* STANDARD HEADER */

/* USER HEADER     */

#ifndef _GFPCVX82_H_
#define _GFPCVX82_H_

/* ------------------------------------------------------------------------------------------ */
/* グローバルデータ                                                                           */
/* ------------------------------------------------------------------------------------------ */
extern t_kbt_file_data    g_kbt_file_data;                /* 個別ファイル情報                 */
extern t_kbt_svrcls_data  g_kbt_svrcls;                   /* 個別ServerClass情報              */
extern db_gfnws_def       g_db_gfnws_tbl[4];              /* 接続先固有情報ファイルテーブル   */
extern t_gfnws_pkey_def   g_gfnws_pkey;                   /* 接続先固有情報ファイルPkey       */
extern db_gcsst_def       g_gcsst;                        /* 局状態管理ファイルレコード       */
extern db_gcest_def       g_gcest;                        /* エコー状態管理ファイルレコード   */
extern t_gcest_pkey_def   g_gcest_pkey;                   /* エコー状態管理ファイルPKey       */
extern db_glmlg_def       g_glmlg;                        /* 制御電文ログレコード             */
extern t_rcv_info_def     g_rcv_info;                     /* 受信電文情報                     */
extern COM_SDT_arg_2_def  g_com_sdt_arg_2;                /* システム日時取得 arg2            */
extern COM_SDT_arg_3_def  g_com_sdt_arg_3;                /* システム日時取得 arg3            */
#endif
