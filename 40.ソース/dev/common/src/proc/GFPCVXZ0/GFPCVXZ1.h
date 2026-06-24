/**
 * @brief GFPCVXZ1.h PATHWAYサーバ共通ヘッダーファイル
 *                   グローバル定義/プロトタイプ宣言
 *
 * @date 2025/03/06 新規作成 by HAS
 *
*/

#ifndef _GFPCVXZ1_H_
#define _GFPCVXZ1_H_

/* ------------------------------------------------------------------------------------ */
/* グローバルデータ                                                                     */
/* ------------------------------------------------------------------------------------ */
t_myinfo_def             g_myinfo;                      /* 自プロセス情報               */
procinfo_def             g_procinfo;                    /* プロセス情報                 */
t_sendinfo_def           g_sendinfo;                    /* PATHSEND情報                 */
t_com_file_data          g_com_file_data;               /* 共通ファイル情報             */
t_iocomp_def             g_iocomp;                      /* I/O完了情報                  */
t_encdec_con             g_encdec;                      /* 暗号復号化情報               */
db_gfnwi_def             g_gfnwi_tbl[3];                /* NW情報ファイルレコード       */
COM_STP_arg_1_def        g_openersinfo;                 /* オープナー情報テーブル       */
oggz1in_def              g_cg010in_modle;               /* メッセージ出力               */
NWM_CTU_INI_arg_6_def    g_ems_add;                     /* EMS出力付加情報              */
COM_IOM_arg_3_def        g_com_iom_arg_3;               /* I/Oモジュール arg 3          */
COM_IOM_arg_4_def        g_com_iom_arg_4;               /* I/Oモジュール arg 4          */
COM_IOM_arg_5_def        g_com_iom_arg_5;               /* I/Oモジュール arg 5          */
COM_IOM_arg_6_def        g_com_iom_arg_6;               /* I/Oモジュール arg 6          */
COM_ERL_arg_1_def        g_com_erl_arg_1;               /* エラーログ出力 arg1          */
COM_ERL_arg_2_def        g_com_erl_arg_2;               /* エラーログ出力 arg2          */
COM_ERL_arg_3_def        g_com_erl_arg_3;               /* エラーログ出力 arg3          */
NWM_CTU_INI_arg_1_def    g_NWM_CTU_INI_arg_1;           /* カット対象日付取得 arg1      */
NWM_CTU_INI_arg_2_def    g_NWM_CTU_INI_arg_2;           /* カット対象日付取得 arg2      */
NWM_CTU_INI_arg_3_def    g_NWM_CTU_INI_arg_3;           /* カット対象日付取得 arg3      */
NWM_CTU_INI_arg_4_def    g_NWM_CTU_INI_arg_4;           /* カット対象日付取得 arg4      */
NWM_CTU_INI_arg_5_def    g_NWM_CTU_INI_arg_5;           /* カット対象日付取得 arg5      */
NWM_CTU_INI_arg_6_def    g_NWM_CTU_INI_arg_6;           /* カット対象日付取得 arg6      */
char                     g_ch_sub_prog_sts[2];          /*                              */
char                     g_recv_buf[DEF_BUF_LENGTH];    /* $RECEIVE I/O用バッファ       */
char                     g_resp_buf[DEF_BUF_LENGTH];    /* $RECEIVE I/O用バッファ       */
char                     g_send_buf[DEF_BUF_LENGTH];    /* PATHSEND用バッファ           */
char                     g_trc_buf[sizeof(lk_zac2001i_arg_1_def)]; /*                   */
char                    *gp_sys_msg = g_recv_buf;       /* システムッセージ             */
lk_zac2001r_arg_1_def   *gp_trc_st;                     /* トレース：開始               */
lk_zac2001r_arg_1_def   *gp_trc_ed;                     /* トレース：終了               */

unsigned short           g_recv_len;                    /* $RECEIVE受信レングス         */
char                     EXMYSRVCLSNAME[15];
char                     EXMYPROCNAME[6];
short                    EXTRACEMODE;
char                     EXTRACEFILENAME[47];
short                    EXTRACEFILENO;

/* 関数のプロトタイプ宣言 */
/* ------------------------------------------------------------------------------------------ */
/* 共通メイン関数                                                                             */
/* ------------------------------------------------------------------------------------------ */
static void CMIN_set_comiom_arg( char              *pch_pname       /* 物理ファイル名         */
                               , char              *pch_lname       /* 論理ファイル名         */
                               , short              sh_fie_no       /* ファイル番号           */
                               , char              *pch_rec_key     /* 読込みキー             */
                               , short              sh_lock         /* LOCK有無               */
                               , short              sh_key_len      /* キー長                 */
                               , short              sh_rec_len      /* レコード長             */
                               , COM_IOM_arg_3_def *ptrace_inf      /* トレース情報           */
                               , COM_IOM_arg_4_def *pfile_inf       /* ファイル情報           */
                               , COM_IOM_arg_5_def *pin_inf         /* 入力情報               */
                               , char              *pch_iotype);    /* ファイルI/Oタイプ      */
static void CMIN_spctub_cpy   ( char              *dst              /* コピー先               */
                              , char              *src              /* コピー元               */
                              , size_t            len  );           /* コピー長               */

#endif
