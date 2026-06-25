#ifndef _GFPCVXC0_H_
#define _GFPCVXC0_H_
/**
 * @brief GFPCVXC0.h 通知電文制御ヘッダーファイル
 *
 * @date 2025/04/10 新規作成 by HAS
 *
*/
/*   ｺﾝﾊﾟｲﾙｵﾌﾟｼｮﾝ                                                           */
/****************************************************************************/

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/

/* 内部エラーコード */
//#define DEF_NERR_NTC_RECV_SEISA             "SCDN001"   /* 障害電文通知精査エラー             */
//#define DEF_NERR_NTC_RESP_EDIT              "SCDN002"   /* 障害電文通知応答編集エラー         */
//#define DEF_NERR_NTC_RECV                   "SCDN003"   /* 障害電文通知受信                   */
//#define DEF_NERR_REQ_RECV                   "SCDN004"   /* 障害電文要求受信                   */

/* -------------------------------------------------------*/
/* 構造体のtypedef定義                                    */
/* -------------------------------------------------------*/
                                                        /* ---------------------------------- */
typedef struct __t_kbt_file_data_def                    /* 個別ファイル情報                   */
{                                                       /* ---------------------------------- */
    char   log_file_name[ZSYS_VAL_LEN_FILENAME+1];      /*  制御電文ログ名(物理)              */
    short  log_file_no;                                 /*  制御電文ログ番号                  */
} t_kbt_file_data_def;
//                                                        /* ---------------------------------- */
//typedef struct __fileio_def                             /* ファイルI/Oテーブル                */
//{                                                       /* ---------------------------------- */
//    lk_iocorem_arg_1_def arg_1;                         /*  機能名識別情報                    */
//    lk_iocorem_arg_2_def arg_2;                         /*  サブプログラムステータス情報      */
//    lk_iocorem_arg_3_def arg_3;                         /*  トレース情報                      */
//    lk_iocorem_arg_4_def arg_4;                         /*  ファイル情報                      */
//    lk_iocorem_arg_5_def arg_5;                         /*  入力情報                          */
//    lk_iocorem_arg_6_def arg_6;                         /*  出力情報                          */
//} fileio_def;
                                                        /* ---------------------------------- */
typedef struct __glmlg_pri_key_def                      /* 制御電文ログプライマリキー         */
{                                                       /* ---------------------------------- */
   char    part_id[2];                                  /*  パーティションID                  */
   char    lcn_id[15];                                  /*  LCN                               */
   char    s_h_kubun;                                   /*  仕向・被仕向区分                  */
   char    send_recv_id;                                /*  送受信識別                        */
} glmlg_pri_key_def;
                                                        /* ---------------------------------- */
typedef struct __ctrl_info_def                          /* 制御情報                           */
{                                                       /* ---------------------------------- */
   char                internal_error_code[7];          /*  内部エラーコード                  */
   control_kind_def    rcv_control_kind;                /*  受信制御電文種別                  */
   unsigned short      rcv_msg_len;                     /*  受信制御電文長                    */
   char                send_mti[4];                     /*  送信電文MTI                       */
   short               scrutiny_result;                 /*  通知電文精査結果                  */
} ctrl_info_def;


/* -------------------------------------------------------*/
/* 定数定義                                               */
/* -------------------------------------------------------*/

/* NW情報ファイル取得区分 */
#define     DEF_GFNWI_GET_GP               1                /* グループ単位 */
#define     DEF_GFNWI_GET_IF               2                /* インタフェース単位 */
#define     DEF_GFNWI_GET_ST               3                /* ステーション単位 */

/* NW情報ファイル */
#define     DEF_IF_ID_DEFAULT              "}}}}}"          /* インタフェース識別.指定なしALL"}" */
#define     DEF_STATION_ID_DEFAULT         "}}}}}}"         /* ステーション識別.指定なしALL"}"   */

/* 処理区分 */
#define     DEF_SMK_REP_RCV                13               /* 仕向応答電文受信 */
#define     DEF_HSMK_REQ_RCV               21               /* 被仕向要求電文受信 */
#define     DEF_HSMK_REP_SND               22               /* 被仕向応答電文送信 */
#define     DEF_HSMK_REP_SND_FUKA          23               /* 被仕向応答電文送信不可 */

/* 共通ファイルI/Oモジュール */
#define DEF_FILEIO_REWRITE          "REWRITE "

/* ------------------------------------------------------------------------------------ */
/* グローバルデータ                                                                     */
/* ------------------------------------------------------------------------------------ */
/* ファイルレコード */
t_kbt_file_data_def      g_kbt_file_data;               /* 個別ファイル情報             */
db_gfnwi_def             g_db_gfnwi;                    /* NW情報ファイルレコード       */
//fileio_def               g_glmlg_io;                    /* 電文ログファイルI/O情報      */
glmlg_pri_key_def        g_wk_glmlg_pri_key;            /* WKプライマリーキー           */
glmlg_pri_key_def        g_glmlg_pri_key;               /* 編集用プライマリーキー       */

ctrl_info_def            g_ctrl_info;                   /* 制御情報                     */

/* IPC */
common_header_def   *g_ipcreq_head;                     /* 要求IPCヘッダー      */
common_header_def   *g_ipcres_head;                     /* 応答IPCヘッダー      */
cr401_def           *g_c401;                            /* NW受信電文要求       */
cr401_def           *g_r401;                            /* NW受信電文応答       */


/* 関数のプロトタイプ宣言 */
void  CNTF_receive_req_data( void );                        /* 電文受信通知処理 */
short CNTF_validate_request( void );                      /* 電文精査処理 */
void  CNTF_validate_error( void );                        /* 電文精査エラー処理 */
void  CNTF_receive_notice( void );                          /* 通知型電文受信処理 */
void  CNTF_receive_request( void );                         /* 要求応答型要求電文受信処理 */
void  CNTF_receive_response( void );                        /* 要求応答型応答電文受信処理 */
void  CNTF_send_response_error( void );                     /* 応答電文送信不可処理 */
void  CNTF_get_network_info( short, char *,                 /* N/W情報ファイル取得処理 */
                             char *, db_gfnwi_def * );
void  CNTF_put_log( short, char *, short ,char *);          /* 制御電文ログ出力処理 */
void  CNTF_put_errlog( char, char * );                      /* エラー出力ログ出力処理 */


#endif /* _GFPCVXC0_H_ */
