/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSX80                                    */
/*        FUNCTION          ････ 鍵交換制御                                  */
/*                                                                           */
/*                               制御電文共通となる常駐プロセス制御を行う。  */
/*                                                                           */
/*        AUTHER            ････ HAS                                         */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-03-30                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS        2025/03/06 新規作成                                      */
/*                                                                           */
/*****************************************************************************/
#ifndef _GFPCSX80_H
#define _GFPCSX80_H

#define  DEF_NWM_KYX_MSG_KIND_REQRSP     1               /* 鍵交換電文種別(精査/局状態判定) 鍵交換依頼応答                    */
#define  DEF_NWM_KYX_MSG_KIND_KEYRSP     2               /* 鍵交換電文種別(精査/局状態判定) GFP契機鍵交換応答                 */
#define  DEF_NWM_KYX_MSG_KIND_IRAIREQ    1               /* 鍵交換電文種別(編集) 鍵交換依頼時の要求電文作成        */
#define  DEF_NWM_KYX_MSG_KIND_KEYREQ     2               /* 鍵交換電文種別(編集) GFP契機鍵交換要求時の要求電文作成 */

#define  DEF_NWM_KYX_RTN_OK              0               /* 戻り値 正常                      */
#define  DEF_NWM_KYX_RTN_NG_REJ          1               /* 戻り値 拒否応答                  */
#define  DEF_NWM_KYX_RTN_FAULT_MSG       2               /* 戻り値 障害電文通知              */
#define  DEF_NWM_KYX_RTN_HAKI_MSG        3               /* 戻り値 電文破棄                  */
#define  DEF_NWM_KYX_RTN_ERR             9               /* 戻り値 異常                      */

#define  DEF_NWM_KYX_RTN_NG             -1

#define  DEF_NWM_KYX_KIND_KPE           "KPE "
#define  DEF_NWM_KYX_KIND_KMAC          "KMAC"
#define  DEF_NWM_KYX_KIND_KC            "KC  "

#define  DEF_NWM_KYX_IPC_1804           "1804"
#define  DEF_NWM_KYX_IPC_1814           "1814"

/* 内部エラーコード */
//#define DEF_NERR_NOMAL                    "0000000"   /* 正常                                       */
//#define DEF_NERR_HSMK_SEISA_ERR           "SCDK001"   /* 被仕向鍵交換要求精査エラー                 */
//#define DEF_NERR_HSMK_STATION_ST_ERR      "SCDK002"   /* 被仕向鍵交換局状態不正                     */
//#define DEF_NERR_HSMK_DECODE_ERR          "SCDK003"   /* 被仕向鍵交換鍵復号エラー                   */
//#define DEF_NERR_HSMK_LCN_GET_ERR         "SCDK004"   /* 被仕向鍵交換LCN取得エラー                  */
//#define DEF_NERR_HSMK_TOHAN_DOUKI_ERR     "SCDK005"   /* 被仕向鍵交換東阪鍵同期エラー               */
//#define DEF_NERR_SMK_SEISA_ERR            "SCDK006"   /* 仕向鍵交換要求精査エラー                   */
//#define DEF_NERR_SMK_STATION_ST_ERR       "SCDK007"   /* 仕向鍵交換局状態チェックエラー             */
//#define DEF_NERR_SMK_KEY_ENC_ERR          "SCDK008"   /* 仕向鍵交換鍵生成エラー                     */
//#define DEF_NERR_SMK_LCN_GET_ERR          "SCDK009"   /* 仕向鍵交換LCN取得エラー                    */
//#define DEF_NERR_SMK_TO                   "SCDK010"   /* 仕向鍵交換応答待ちタイムアウト             */
//#define DEF_NERR_SMK_REJECT_RSP           "SCDK011"   /* 仕向鍵交換拒否応答受信                     */
//#define DEF_NERR_CHK_DIGIT_ERR            "SCDK013"   /* チェックデジットエラー                     */
//#define DEF_NERR_MSG_MAKE_ERR             "SCDK014"   /* 電文作成失敗                               */
//#define DEF_NERR_MSG_CHECK_ERR            "SCDK015"   /* 電文精査エラー                             */
//#define DEF_NERR_SYS_NO_MAKE_ERR          "SCDK016"   /* システム採番生成失敗                       */
//#define DEF_NERR_ATALLA_RSP_ERR           "SCDK017"   /* ATALLAレスポンスエラー                     */
//#define DEF_NERR_CTL_LOG_OUTPUT_ERR       "SCDK018"   /* 制御電文ログ出力エラー                     */
//#define DEF_NERR_HISIMUKE_REQ_CHECK_ERR   "SCDK021"   /* 被仕向要求精査エラー                       */
//#define DEF_NERR_SIMUKE_REQ_CHECK_ERR     "SCDK022"   /* 仕向要求精査エラー                         */
//#define DEF_NERR_SIMUKE_RSP_CHECK_ERR     "SCDK023"   /* 仕向応答精査エラー                         */

/* typedef定義 */
#pragma fieldalign shared2 __NWM_KYX_arg_1_def
typedef struct __NWM_KYX_arg_1_def {
    char    key_kind[4];
    short   key_leng;
    char    *key;
    char    key_usage[2];
    short   checkdigit_leng;
    char    *checkdigit;
    char    err_bit[4];
} NWM_KYX_arg_1_def;

#pragma fieldalign shared2 __NWM_KYX_arg_2_def
typedef struct __NWM_KYX_arg_2_def {
    char    key_kind[4];
    short   key_leng;
    char    *key;
    char    key_usage[2];
    short   checkdigit_leng;
    char    *checkdigit;
    char    err_bit[4];
} NWM_KYX_arg_2_def;

#pragma fieldalign shared2 __NWM_KYX_arg_3_def
typedef struct __NWM_KYX_arg_3_def {
    char             denbun_kind[4];
    gflin_pkey_def   connection_lid;
    short            checkdigit_leng;
    char             *checkdigit;
    char             sysytem_no[6];
    char             key_kind[4];
    short            key_leng;
    char             *key;
    char             err_code[7];
} NWM_KYX_arg_3_def;

#pragma fieldalign shared2 __NWM_KYX_arg_4_def
typedef struct __NWM_KYX_arg_4_def {
    short    message_leng;
    char     *message;
    char     mti[4];
} NWM_KYX_arg_4_def;

#pragma fieldalign shared2 __NWM_KYX_arg_5_def
typedef struct __NWM_KYX_arg_5_def {
    char             denbun_kind[4];
    gflin_pkey_def   connection_lid;
    short            checkdigit_leng;
    char             *checkdigit;
    char             sysytem_no[6];
    char             key_kind[4];
    short            key_leng;
    char             *key;
    char             err_code[7];
} NWM_KYX_arg_5_def;

#pragma fieldalign shared2 __NWM_KYX_arg_6_def
typedef struct __NWM_KYX_arg_6_def {
    short    rcv_message_leng;
    char     *rcv_message;
    short    message_leng;
    char     *message;
    char     mti[4];
} NWM_KYX_arg_6_def;

#pragma fieldalign shared2 __NWM_KYX_ems_add
typedef struct __NWM_KYX_ems_add {
    char            srv_logical_id[16];
    char            lcn[15];
} NWM_KYX_ems_add;

/* カット対象日付管理ファイル情報 */
#pragma fieldalign shared2 __file_info_gccut
typedef struct __file_info_gccut {
    char    file_id[8];             /* ファイルID         */
    char    file_name[48];          /* ファイル名         */
    short   file_no;                /* ファイル番号       */
    long    io_timer;               /* I/Oタイマー        */
} file_info_gccut;

/* プロトタイプ宣言 */
/* 鍵交換電文精査(要求受信) */
short NWM_KYX_msg_check_req(char*                  /* 受信データ(要求電文)                */
                          , char*                  /* 受信電文長                          */
                          , db_gfnwi_def*          /* NW情報レコード(グループ単位)        */
                          , db_gfnwi_def*          /* NW情報レコード(インタフェース単位)  */
                          , char*                  /* 接続先固有情報(NW単位)              */
                          , char*                  /* 接続先固有情報(インタフェース単位)  */
                          , char*                  /* 接続先固有情報(ステーション単位)    */
                          , char*                  /* 接続先固有情報(コネクション単位)    */
                          , db_gckey_def*          /* 鍵管理情報レコード                  */
                          , NWM_KYX_arg_1_def*);   /* 精査処理結果                        */

/* 鍵交換電文精査(応答受信) */
short NWM_KYX_msg_check_rsp(short                  /* 要求種別                            */
                          , char*                  /* 受信データ(応答電文)                */
                          , char*                  /* 受信電文長                          */
                          , db_gfnwi_def*          /* NW情報レコード(グループ単位)        */
                          , db_gfnwi_def*          /* NW情報レコード(インタフェース単位)  */
                          , char*                  /* 接続先固有情報(NW単位)              */
                          , char*                  /* 接続先固有情報(インタフェース単位)  */
                          , char*                  /* 接続先固有情報(ステーション単位)    */
                          , char*                  /* 接続先固有情報(コネクション単位)    */
                          , db_gckey_def*          /* 鍵管理情報レコード                  */
                          , NWM_KYX_arg_2_def*);   /* 精査処理結果                        */

/* 鍵交換電文編集(要求電文) */
short NWM_KYX_msg_edit_req(short                   /* 要求種別                            */
                         , db_gfnwi_def*           /* NW情報レコード(グループ単位)        */
                         , db_gfnwi_def*           /* NW情報レコード(インタフェース単位)  */
                         , char*                   /* 接続先固有情報(NW単位)              */
                         , char*                   /* 接続先固有情報(インタフェース単位)  */
                         , char*                   /* 接続先固有情報(ステーション単位)    */
                         , char*                   /* 接続先固有情報(コネクション単位)    */
                         , db_gckey_def*           /* 鍵管理情報レコード                  */
                         , oggz1in_def*            /* EMS出力共通情報                     */
                         , NWM_KYX_ems_add*        /* EMS出力付加情報                     */
                         , NWM_KYX_arg_3_def*      /* 設定データ                          */
                         , NWM_KYX_arg_4_def*);    /* 電文情報                            */

/* 鍵交換電文編集(応答電文) */
short NWM_KYX_msg_edit_rsp(db_gfnwi_def*           /* NW情報レコード(グループ単位)        */
                         , db_gfnwi_def*           /* NW情報レコード(インタフェース単位)  */
                         , char*                   /* 接続先固有情報(NW単位)              */
                         , char*                   /* 接続先固有情報(インタフェース単位)  */
                         , char*                   /* 接続先固有情報(ステーション単位)    */
                         , char*                   /* 接続先固有情報(コネクション単位)    */
                         , db_gckey_def*           /* 鍵管理情報レコード                  */
                         , file_info_gccut*        /* カット対象日付管理ファイル情報      */
                         , oggz1in_def*            /* EMS出力共通情報                     */
                         , NWM_KYX_ems_add*        /* EMS出力付加情報                     */
                         , NWM_KYX_arg_5_def*      /* 設定データ                          */
                         , NWM_KYX_arg_6_def*);    /* 電文情報                            */

/* 鍵交換局状態チェック(要求受信) */
short NWM_KYX_cst_check_req(char*);                 /* 局状態                              */

/* 鍵交換局状態チェック(応答受信) */
short NWM_KYX_cst_check_rsp(short                  /* 鍵交換電文種別                      */
                          , char*);                /* 局状態                              */

#endif

