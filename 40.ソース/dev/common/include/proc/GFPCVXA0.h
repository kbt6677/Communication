/**
 * @brief GFPCVXA0.h PATHWAYサーバ共通ヘッダーファイル
 *
 * @date 2025/03/06 新規作成 by HAS
 *
*/


#ifndef _GFPCVXA0_H_
#define _GFPCVXA0_H_

/********** define ***********/
/* openファイル種別 */
#define DEF_FLTYPE_NWIF                         1       /* NW情報ファイル                     */
#define DEF_FLTYPE_SSTM                         2       /* 局状態管理(自サイト)ファイル       */
#define DEF_FLTYPE_SSTO                         3       /* 局状態管理(他サイト)ファイル       */
#define DEF_FLTYPE_ECHO                         4       /* エコー状態管理ファイル             */
#define DEF_FLTYPE_CMMG                         5       /* 制御電文管理ファイル               */
#define DEF_FLTYPE_TOHN                         6       /* 東阪振分け比率設定ファイル         */

/* 内部エラーコード */
#define DEF_INN_ERR_CD_NORMAL           "0000000"       /* 内部エラーコード・正常             */

#define DEF_PROG_ID                     "GFPCVXA0"      /* プログラムID                       */

#define DEF_CT_OVR_KEY_LEN              24              /* カットオーバーキー長 */
#define  DEF_COM_PSD_EMS_MSGTTKB        "E"             /* EMS エラー */

#define DEF_CCUT_BUF_LENGTH             3200            /*   受信Buffer Length                */
#define DEF_CCUT_STATUS_LEN             2

#define DEF_CCUT_STA_STS_KEY_LEN        19              /* 局状態キー長                       */

#define DEF_REC_UNIT_FLG_CNT             4              /* 接続先固有情報ファイルレコード登録単位件数 */
#define DEF_REC_UNIT_IDX_NW              0              /* 接続先固有情報ファイルレコード登録単位NW */
#define DEF_REC_UNIT_FLG_IF              1              /* 接続先固有情報ファイルレコード登録単位IF */
#define DEF_REC_UNIT_FLG_ST              2              /* 接続先固有情報ファイルレコード登録単位STATION */
#define DEF_REC_UNIT_FLG_CNN             3              /* 接続先固有情報ファイルレコード登録単位CONNECTION */
#define DEF_IPC_FAULT_MSG                8              /* 障害電文 */

#define DEF_CUT_OVER_MNG_LYR_CNCT       'C'
                                                        /* 単純memcpy（転送先のサイズがわかる場合限定 */
#define MCR_CCUT_CPY(dst, src)          memcpy(dst,src,sizeof(dst));
#define DEF_CCUT_SND_TYPE               'H'             /* 被仕向 */
#define DEF_CCUT_SND_IDF                '1'             /* 要求電文受信 */
#define DEF_CCUT_OK                      0
#define DEF_CCUT_NG                      -1
#define DEF_GCSST_MSG_LEN                      424
#define DEF_CCUT_RCV_ID                  '1'
#define DEF_CCUT_SND_ID                  '2'

#define DEF_CCUT_MTI_UPD                  0
#define DEF_CCUT_MTI_REQ                  1

#define DEF_INN_ERR_GFNW_NW           "ECCK401"       /* 内部エラーコード・正常             */
#define DEF_INN_ERR_GFNW_IF           "ECCK411"       /* 内部エラーコード・正常             */
#define DEF_INN_ERR_GFNW_ST           "ECCK421"       /* 内部エラーコード・正常             */
#define DEF_INN_ERR_GFNW_CNN          "ECCK431"       /* 内部エラーコード・正常             */

#define DEF_STATE_STS_LEN              2

/********** グローバルデータ ***********/
oggz1in_def              g_cmn_ems_info;                /* EMS出力共通情報 */
NWM_CTU_INI_arg_6_def    g_add_ems_info;                /* EMS出力付加情報 */

                                                            // 接続先固有情報レコード
                                                            //    コネクション単位以外は起動時に定義する
db_gfnws_def             gst_gfnws_inf[DEF_REC_UNIT_FLG_CNT];
                                                            // 接続先固有情報ファイルレコード登録単位
char                     gch_gfnws_rec_unit[DEF_REC_UNIT_FLG_CNT];
COM_IOM_arg_3_def        g_GFNWS_tace_inf;                  // 接続先固有情報・トレース情報
COM_IOM_arg_4_def        g_GFNWS_file_inf;                  // 接続先固有情報・ファイル情報
COM_IOM_arg_5_def        g_GFNWS_in_inf;                    // 接続先固有情報・入力情報
char                     g_GFNWS_key[sizeof(db_gfnws_def)]; // 接続先固有情報・検索キー

COM_IOM_arg_3_def        g_GCSST_tace_inf;                  // 局状態情報・トレース情報
COM_IOM_arg_4_def        g_GCSST_file_inf;                  // 局状態情報・ファイル情報
COM_IOM_arg_5_def        g_GCSST_in_inf;                    // 局状態情報・入力情報

COM_IOM_arg_3_def        g_GFNWI_tace_inf;                  // NW情報情報・ステーション単位・トレース情報
COM_IOM_arg_4_def        g_GFNWI_file_inf;                  // NW情報情報・ステーション単位・ファイル情報
COM_IOM_arg_5_def        g_GFNWI_in_inf;                    // NW情報情報・ステーション単位・入力情報
char                     g_GFNWI_key[sizeof(db_gfnwi_def)]; // NW情報情報・ステーション単位・検索キー

COM_IOM_arg_3_def        g_GLMLG_tace_inf;                  // 制御電文ログファイル・トレース情報
COM_IOM_arg_4_def        g_GLMLG_file_inf;                  // 制御電文ログファイル・ファイル情報
COM_IOM_arg_5_def        g_GLMLG_in_inf;                    // 制御電文ログファイル・入力情報

db_gcsst_def            g_GCSST_rec;                        // 局状態情報レコード

/* 関数のプロトタイプ宣言 */


short CCUT_cutover( cr401_def *cr401msg
                   , char     *c_inner_errcd
                   , char     *c_cut_date);

static void  ccut_stb_cpy(void *dst, void *src, size_t len);
static short CCUT_kbt_ini_GCSST();
static short CCUT_kbt_ini_GFNWS();
static short CCUT_kbt_ini_GFNWI();
static short CCUT_kbt_ini_GLMLG();
static void  CCUT_ems_arg_ini();
static void set_comiom_arg(
    char                    *pch_pname,         // 物理ファイル名
    char                    *pch_lname,         // 論理ファイル名
    short                    sh_fie_no,         // ファイル番号
    char                    *pch_rec_key,       // 読込みキー
    short                    sh_lock,           // LOCK有無
    short                    sh_key_len,        // キー長
    short                    sh_rec_len,        // レコード長
    COM_IOM_arg_3_def       *ptrace_inf,        // トレース情報
    COM_IOM_arg_4_def       *pfile_inf,         // ファイル情報
    COM_IOM_arg_5_def       *pin_inf            // 入力情報
);
static short CCUT_get_cnct_gfnws(
    cr401_def    *pst_rec_rcv,
    char         *rec_unit,
    db_gfnws_def *pch_cninf_if,
    db_gfnws_def *pch_cninf_st,
    db_gfnws_def *pch_cninf_cn,
    char         *intrlr_err_code
);
static short  CCUT_read_gfnwi(
    char *c_inner_errcd
);
static void CCUT_edit_gfnws_key(
    COM_IOM_arg_5_def   *pst_in_inf,
    cr401_def           *pst_rec_rcv
);
static short CCUT_put_log(
    char             send_recv_id,
    cr401_def       *pcr401msg,
    db_gfnwi_def    *pstgfnwi,
    char            *c_inner_errcd,
    short           len
);
static short CCUT_update_log(
    cr401_def       *pcr401msg,
    db_gfnwi_def    *pstgfnwi,
    char            *c_inner_errcd,
    short           len
);
static void CCUT_make_glmlg(
    char             send_recv_id,
    cr401_def       *pcr401msg,
    db_gfnwi_def    *pstgfnwi,
    char            *c_inner_errcd,
    short           len,
    db_glmlg_def    *pst_glmlg
);
static void CCUT_msg_edit(
    short        sh_mti_flg,        /* MTI更新フラグ */
    char         *psh_reqmsg,       /* 被仕向要求電文 */
    char         *pch_inn_errcd,    /* 内部エラーコード */
    char         *pch_rspmsg,       /* 応答メッセージ */
    short        *psh_len           /* 応答メッセージ長 */
);
static void CCUT_put_errlog( char * );

#endif
