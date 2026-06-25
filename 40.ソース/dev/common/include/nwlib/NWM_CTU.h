/**
 * @brief GFPCVXA0.h PATHWAYサーバ共通ヘッダーファイル
 *
 * @date 2025/03/06 新規作成 by HAS
 *
*/


#ifndef _NWM_CTU_H_
#define _NWM_CTU_H_

#include "GFPCGX50.h"
#include "GFPCGXB0.h"

#define     DEF_NWM_CTU_OPEN           "OPEN"          // ファイルアクセスイベント：OPEN
#define     DEF_NWM_CTU_START          "START"         // ファイルアクセスイベント：START
#define     DEF_NWM_CTU_READ           "READ"          // ファイルアクセスイベント：READ
#define     DEF_NWM_CTU_WRITE          "WRITE"         // ファイルアクセスイベント：WRITE
#define     DEF_NWM_CTU_UNLOCK         "UNLOCK"        // ファイルアクセスイベント：UNLOCK
#define     DEF_NWM_CTU_CLOSE          "CLOSE"         // ファイルアクセスイベント：CLOSE

#define     DEF_NWM_CTU_LOCK           1               // LOCKモード
#define     DEF_NWM_CTU_GYOM_ERR       'E'
#define     DEF_NWM_CTU_NERR_FILE_IO_ERR                "SCAA001"            /* ファイルIOエラー                           */

/* ------------------------------------------------------------------------------------------ */
/* ここからカット対象日付取得初期化処理のパラメータ                                           */
/* ------------------------------------------------------------------------------------------ */
                                                        /* ---------------------------------- */
typedef struct __t_NWM_CTU_INI_arg_1_def                /* 物理名情報ファイル情報             */
{                                                       /* ---------------------------------- */
    char           file_id[8];                          /*  ファイルID                        */
    char           file_name[47];                       /*  ファイル名                        */
    short          file_no;                             /*  ファイル番号                      */
    unsigned long  io_timer;                            /*  I/Oタイマー                       */
} NWM_CTU_INI_arg_1_def;
                                                        /* ---------------------------------- */
typedef struct __t_NWM_CTU_INI_arg_2_def                /* カット対象日付管理ファイル情報     */
{                                                       /* ---------------------------------- */
    char           file_id[8];                          /*  ファイルID                        */
    char           file_name[47];                       /*  ファイル名                        */
    short          file_no;                             /*  ファイル番号                      */
    unsigned long  io_timer;                            /*  I/Oタイマー                       */
} NWM_CTU_INI_arg_2_def;
                                                        /* ---------------------------------- */
typedef struct __t_NWM_CTU_INI_arg_3_def                /* ネットワーク特定情報               */
{                                                       /* ---------------------------------- */
    char           site_id;                             /*   サイト識別                       */
    char           nw_id;                               /*   N/W識別                          */
    char           grp_id[5];                           /*   グループ識別                     */
    char           if_id[5];                            /*   インタフェース識別               */
    char           station_id[6];                       /*   ステーション識別                 */
    char           connect_id[6];                       /*   コネクション識別                 */
} NWM_CTU_INI_arg_3_def;
                                                        /* ---------------------------------- */
typedef struct __t_NWM_CTU_INI_arg_4_def                /* モジュールID                       */
{                                                       /* ---------------------------------- */
    char           prcmlt_id[8];                        /*  モジュールID                      */
} NWM_CTU_INI_arg_4_def;
                                                        /* ---------------------------------- */
typedef struct __t_NWM_CTU_INI_arg_5_def                /* EMS出力共通情報                    */
{                                                       /* ---------------------------------- */
    char  subrcd;
    struct {                                            /*  運用監視端末出力情報              */
        char  proctimer[4];                             /*   プロセスI/Oタイマー              */
        char  uytrmmon[13];                             /*   運用監視端末出力PATHMON名        */
        char  uytrmmonlen[2];                           /*   運用監視端末出力PATHMON名長      */
        char  uytrmsrv[12];                             /*   運用監視端末出力サーバ名         */
        char  uytrmsrvlen[2];                           /*   運用監視端末出力サーバ名長       */
    } uytrminf;
    struct {                                            /*  EMS出力情報                       */
        char  rcd;                                      /*   リターンコード                   */
        char  msgid[5];                                 /*   メッセージID                     */
        struct {                                        /*   業務共通メッセージ               */
            char  msgttkb;                              /*    メッセージ通知区分              */
            char  emsgkinf_yobi1;                       /*    予備                            */
            char  sysnm[3];                             /*    システム名                      */
            char  emsgkinf_yobi2;                       /*    予備                            */
            char  srv_kbn[3];                           /*    サーバー分類                    */
            char  emsgkinf_yobi3;                       /*    予備                            */
            char  h_nw_kbn[2];                          /*    NW識別（被仕向）                */
            char  emsgkinf_yobi4;                       /*    予備                            */
            char  s_nw_kbn[2];                          /*    NW識別（仕向）                  */
            char  emsgkinf_yobi5;                       /*    予備                            */
            char  prgid[8];                             /*    メッセージ出力元プログラム名    */
            char  emsgkinf_yobi6;                       /*    予備                            */
            char  trmnm[8];                             /*    メッセージ出力元プロセス名      */
            char  emsgkinf_yobi7;                       /*    予備                            */
            char  inter_errcd[7];                       /*    GFP内部エラーコード             */
            char  emsgkinf_yobi8[6];                    /*    予備                            */
        } emsgkinf;
        struct {                                        /*   任意メッセージ                   */
            char  ktmg[40];                             /*    （項目名称不明）                */
            struct {                                    /*    任意メッセージテーブル ×10     */
                char  msgtbl_prm[40];                   /*     （項目名称不明）               */
                char  msgtbl_vl[80];                    /*     出力内容                       */
            } msgtbl[10];
        } emsnninf;
    } emsinf;
} NWM_CTU_INI_arg_5_def;
                                                        /* ---------------------------------- */
typedef struct __t_NWM_CTU_INI_arg_6_def                /* EMS出力付加情報                    */
{                                                       /* ---------------------------------- */
    char  srv_logical_id[8];                            /*  サーバークラス論理ID              */
    char  lcn[15];                                      /*  GFP内部LCN                        */
    char  connect[24];                                  /*  接続先(サイト識別コネクション識別)*/
} NWM_CTU_INI_arg_6_def;

/* ------------------------------------------------------------------------------------------ */
/* 関数のプロトタイプ宣言                                                                     */
/* ------------------------------------------------------------------------------------------ */

short NWM_CTU_INIT(
    NWM_CTU_INI_arg_1_def   *pst_phys_name_inf, // 物理名情報ファイル情報
    NWM_CTU_INI_arg_2_def   *pst_cut_file_inf,  // カット対象日付管理ファイル情報
    NWM_CTU_INI_arg_3_def   *pst_nw_inf,        // ネットワーク特定情報
    char                    *pch_module_id,     // モジュールID
    oggz1in_def             *pst_ems_cmn_inf,   // EMS共通情報
    NWM_CTU_INI_arg_6_def   *pst_ems_add_inf    // EMS追加情報
);

short NWM_CTU_GET(
    NWM_CTU_INI_arg_2_def   *pst_cut_file_inf,  // カット対象日付管理ファイル情報
    NWM_CTU_INI_arg_3_def   *pst_nw_inf,        // ネットワーク特定情報
    char                    *pch_module_id,     // モジュールID
    oggz1in_def             *pst_ems_cmn_inf,   // EMS共通情報
    NWM_CTU_INI_arg_6_def   *pst_ems_add_inf,   // EMS追加情報
    char                    *pst_db_gccut_inf   // カット対象日付管理レコード
);

short NWM_CTU_UPDATE(
    NWM_CTU_INI_arg_2_def   *pst_cut_file_inf,  // カット対象日付管理ファイル情報
    NWM_CTU_INI_arg_3_def   *pst_nw_inf,        // ネットワーク特定情報
    char                    *pch_module_id,     // モジュールID
    oggz1in_def             *pst_ems_cmn_inf,   // EMS共通情報
    NWM_CTU_INI_arg_6_def   *pst_ems_add_inf,   // EMS追加情報
    char                    *pch_cutdate        // カット日付
);
#endif
