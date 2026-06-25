/**
 * @brief GFPCVXZ2.h PATHWAYサーバ共通ヘッダーファイル
 *                   グローバル定義/プロトタイプ宣言
 *
 * @date 2025/03/06 新規作成 by HAS
 *
*/

#ifndef _GFPCVXZ2_H_
#define _GFPCVXZ2_H_

/* ------------------------------------------------------------------------------------ */
/* グローバルデータ                                                                     */
/* ------------------------------------------------------------------------------------ */
extern t_myinfo_def             g_myinfo;                      /* 自プロセス情報               */
extern procinfo_def             g_procinfo;                    /* プロセス情報                 */
extern t_sendinfo_def           g_sendinfo;                    /* PATHSEND情報                 */
extern t_com_file_data          g_com_file_data;               /* 共通ファイル情報             */
extern t_iocomp_def             g_iocomp;                      /* I/O完了情報                  */
extern t_encdec_con             g_encdec;                      /* 暗号復号化情報               */
extern db_gfnwi_def             g_gfnwi_tbl[3];                /* NW情報ファイルレコード       */
extern COM_STP_arg_1_def        g_openersinfo;                 /* オープナー情報テーブル       */
extern oggz1in_def              g_cg010in_modle;               /* メッセージ出力               */
extern NWM_CTU_INI_arg_6_def    g_ems_add;                     /* EMS出力付加情報              */
extern COM_IOM_arg_3_def        g_com_iom_arg_3;               /* I/Oモジュール arg 3          */
extern COM_IOM_arg_4_def        g_com_iom_arg_4;               /* I/Oモジュール arg 4          */
extern COM_IOM_arg_5_def        g_com_iom_arg_5;               /* I/Oモジュール arg 5          */
extern COM_IOM_arg_6_def        g_com_iom_arg_6;               /* I/Oモジュール arg 6          */
extern COM_ERL_arg_1_def        g_com_erl_arg_1;               /* エラーログ出力 arg1          */
extern COM_ERL_arg_2_def        g_com_erl_arg_2;               /* エラーログ出力 arg2          */
extern COM_ERL_arg_3_def        g_com_erl_arg_3;               /* エラーログ出力 arg3          */
extern NWM_CTU_INI_arg_1_def    g_NWM_CTU_INI_arg_1;           /* カット対象日付取得 arg1      */
extern NWM_CTU_INI_arg_2_def    g_NWM_CTU_INI_arg_2;           /* カット対象日付取得 arg2      */
extern NWM_CTU_INI_arg_3_def    g_NWM_CTU_INI_arg_3;           /* カット対象日付取得 arg3      */
extern NWM_CTU_INI_arg_4_def    g_NWM_CTU_INI_arg_4;           /* カット対象日付取得 arg4      */
extern NWM_CTU_INI_arg_5_def    g_NWM_CTU_INI_arg_5;           /* カット対象日付取得 arg5      */
extern NWM_CTU_INI_arg_6_def    g_NWM_CTU_INI_arg_6;           /* カット対象日付取得 arg6      */
extern char                     g_ch_sub_prog_sts[2];          /*                              */
extern char                     g_recv_buf[DEF_BUF_LENGTH];    /* $RECEIVE I/O用バッファ       */
extern char                     g_resp_buf[DEF_BUF_LENGTH];    /* $RECEIVE I/O用バッファ       */
extern char                     g_send_buf[DEF_BUF_LENGTH];    /* PATHSEND用バッファ           */
extern char                     g_trc_buf[sizeof(lk_zac2001i_arg_1_def)]; /*                   */
extern char                    *gp_sys_msg;                    /* システムッセージ             */
extern lk_zac2001r_arg_1_def   *gp_trc_st;                     /* トレース：開始               */
extern lk_zac2001r_arg_1_def   *gp_trc_ed;                     /* トレース：終了               */

unsigned short                  g_recv_len;                    /* $RECEIVE受信レングス         */
extern char                     EXMYSRVCLSNAME[15];
extern char                     EXMYPROCNAME[6];
extern short                    EXTRACEMODE;
extern char                     EXTRACEFILENAME[47];
extern short                    EXTRACEFILENO;

#endif
