/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････                                             */
/*        FUNCTION          ････ CARDNET用メッセージヘッダー(共通ヘッダー)   */
/*        AUTHER            ････ HAS Matsumoto                               */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-02-07                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  Matsumoto  2025/02/07 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
#ifndef _msg_CA_H
#define _msg_CA_H

#include "GFPCGX20.h"

/* 制御電文 MTI */
#define DEF_CA_MTI_1804_REQ             "1804"      /* 制御電文要求                             */
#define DEF_CA_MTI_1814_RSP             "1814"      /* 制御電文応答                             */
#define DEF_CA_MTI_1644_NTC             "1644"      /* 制御電文通知                             */

/* 共通制御ヘッダ */
#define DEF_CA_CMHD_TYPE_F1             "F1"        /* ヘッダータイプ                           */

/* 業務共通ヘッダ */
#define DEF_CA_APHD_TYPE_A1             "A1"        /* ヘッダータイプ                           */
#define DEF_CA_APHD_MSGCODE_C804_REQ    "C804"      /* 電文種別コード: ネットワーク制御要求     */
#define DEF_CA_APHD_MSGCODE_C814_RSP    "C814"      /* 電文種別コード: ネットワーク制御応答     */
#define DEF_CA_APHD_MSGCODE_E644_NTC    "E644"      /* 電文種別コード: 汎用通知                 */
#define DEF_CA_APHD_MODE_00_HONBAN      0x00        /* モードフラグ: 本番モード                 */
#define DEF_CA_APHD_MODE_10_TEST        0x10        /* モードフラグ: 試験モード                 */
#define DEF_CA_APHD_SIMUKE_KBN_20       0x20        /* 仕向区分     */

/* (F24)ファンクションコード */
#define DEF_CA_F24_FUNCTION_801_OPN     "801"       /* 開局                                     */
#define DEF_CA_F24_FUNCTION_802_CLS     "802"       /* 閉局                                     */
#define DEF_CA_F24_FUNCTION_811_KEY     "811"       /* 鍵交換                                   */
#define DEF_CA_F24_FUNCTION_821_CUT     "821"       /* カットオーバー                           */
#define DEF_CA_F24_FUNCTION_831_ECH     "831"       /* エコーテスト                             */
#define DEF_CA_F24_FUNCTION_650_ERR     "650"       /* 障害通知                                 */

#define DEF_CA_CNTR_ID_LEN                11        /* センターIDレングス               */
#define DEF_CA_CTRL_MSG_LEN                2        /* 全体電文長レングス               */
#define DEF_CA_AUTH_VAL_LEN                4        /* 電文認証値レングス               */
#define DEF_CA_DIGIT_KC_LEN                2        /* 電文暗号化キー(Kc)レングス       */
#define DEF_CA_DIGIT_KMAC_LEN              2        /* 電文暗号化キー(Kmac)レングス     */
#define DEF_CA_SND_TIME_LEN                7        /* 送信日時レングス                 */
#define DEF_CA_CUT_DATE_LEN                4        /* カット対象日付レングス           */
#define DEF_CA_BODY_LEN                    2        /* BODY部電文長レングス             */
#define DEF_CA_MTI_LEN                     4        /* MTIレングス                      */
#define DEF_CA_CARDNET_ID_LEN              2        /* カードネット取引識別             */
#define DEF_CA_CARDNET_SEQ_LEN             6        /* カードネット取引通番             */
#define DEF_CA_CARDNET_AREA_LEN            2        /* カードネット使用域               */
#define DEF_CA_BIT11_DATA_LEN              6        /* システムオーディットナンバー     */
#define DEF_CA_BIT12_DATA_LEN             12        /* 現地取引日時                     */
#define DEF_CA_BIT24_DATA_LEN              3        /* ファンクションコード             */
#define DEF_CA_BIT39_DATA_LEN              3        /* アクションコード                 */

/* typedef定義 */
/* N/W情報ファイル・接続先固有情報 */
#pragma fieldalign shared2 __nwi_unq_info_ca
typedef struct __nwi_unq_info_ca
{
    char    mode_flg;
    char    future_use[99];
} nwi_unq_info_ca_def;
#define nwi_unq_info_ca_def_Size 100

/* 接続先固有情報ファイル・接続先固有情報 */
#pragma fieldalign shared2 __nws_unq_info_ca
typedef struct __nws_unq_info_ca
{
    char    dst_center_id[11];
    char    src_center_id[11];
    char    future_use1[178];
} nws_unq_info_ca_def;
#define nws_unq_info_ca_def_Size 200

/* システム間インターフェース・CARDNET・電文ヘッダ */
#pragma fieldalign shared2 __MSG_HEADER_CARDNET
typedef struct __MSG_HEADER_CARDNET
{
    /* 共通制御ヘッダ */
    char    ctrl_hdr_type   [2];    /* 共通制御・ヘッダータイプ       */
    char    ctrl_msg_len    [2];    /* 共通制御・全体電文長           */
    char    ctrl_src_id     [11];   /* 共通制御・差出センターID       */
    char    ctrl_dst_id     [11];   /* 共通制御・宛先センターID       */
    char    ctrl_merch_code [11];   /* 共通制御・加盟店契約会社コード */
    char    ctrl_snd_time   [7];    /* 共通制御・送信日時             */
    char    ctrl_mode_flg      ;    /* 共通制御・モードフラグ         */
    char    ctrl_filler     [2];    /* 共通制御・予備                 */

    /* 業務共通ヘッダ */
    char    bh_hdr_type     [2];    /* 業務共通・ヘッダータイプ       */
    char    bh_msg_type     [4];    /* 業務共通・電文種別コード       */
    char    bh_auth_val     [4];    /* 業務共通・電文認証値           */
    struct
    {
        char    bh_chk_digit_kc     [2];    /* 業務共通・チェックディジット・電文暗号化キー(KC) */
        char    bh_chk_digit_kmac   [2];    /* 業務共通・チェックディジット・電文認証キー(KMAC) */
    } bh_chk_digit;                         /* 業務共通・チェックディジット */
    char    bh_dst_type        ;    /* 業務共通・仕向区分             */
    char    bh_cut_date     [4];    /* 業務共通・カット対象日付       */
    char    bh_body_len     [2];    /* 業務共通・BODY部電文長         */
    char    bh_cardnet_id   [2];    /* 業務共通・カードネット取引識別 */
    char    bh_cardnet_seq  [6];    /* 業務共通・カードネット取引通番 */
    char    bh_cardnet_area [2];    /* 業務共通・カードネット使用域   */
    char    bh_filler       [2];    /* 業務共通・予備                 */
} MSG_HEADER_CARDNET_def;

#define MSG_HEADER_CARDNET_ctrl_Size 47
#define MSG_HEADER_CARDNET_bh_Size 33
#define MSG_HEADER_CARDNET_def_Size 80

/* 制御電文固定フォーマット[CARDNET] 1804 */
#pragma fieldalign shared2 __fixedform_cardnet_1804_def
typedef struct __fixedform_cardnet_1804_def
{
                                                // F11.システムトレースオーディットナンバー
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b11_system_audit_number;
                                                // F12.現地取引日時
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[12];
    } b12_local_tran_time;
                                                // F24.ファンクションコード
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[3];
        char                filler;
    } b24_function_code;
                                                // F28.精査日
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b28_scrutiny_date;
                                                // F53.セキュリティ関連制御情報
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[14];
    } b53_secure_ctl_info;
                                                // F93.電文送信先センターID
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[11];
        char                filler;
    } b93_src_center_id;
                                                // F94.電文送信元センターID
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[11];
        char                filler;
    } b94_dst_center_id;
                                                // F96.キーマネージメントデータ
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[19];
        char                filler;
    } b96_key_management_data;
} fixedform_cardnet_1804_def;

/* 制御電文固定フォーマット[CARDNET] 1814 */
#pragma fieldalign shared2 __fixedform_cardnet_1814_def
typedef struct __fixedform_cardnet_1814_def
{
                                                // F11.システムトレースオーディットナンバー
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b11_system_audit_number;
                                                // F12.現地取引日時
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[12];
    } b12_local_tran_time;
                                                // F24.ファンクションコード
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[3];
        char                filler;
    } b24_function_code;
                                                // F28.精査日
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b28_scrutiny_date;
                                                // F39.アクションコード
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[3];
        char                filler;
    } b39_action_code;
                                                // F53.セキュリティ関連制御情報
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[14];
    } b53_secure_ctl_info;
                                                // F93.電文送信先センターID
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[11];
        char                filler;
    } b93_src_center_id;
                                                // F94.電文送信元センターID
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[11];
        char                filler;
    } b94_dst_center_id;
                                                // F96.キーマネージメントデータ
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[19];
        char                filler;
    } b96_key_management_data;
} fixedform_cardnet_1814_def;

/* 制御電文固定フォーマット[CARDNET] 1644 */
#pragma fieldalign shared2 __fixedform_cardnet_1644_def
typedef struct __fixedform_cardnet_1644_def
{
                                                // F11.システムトレースオーディットナンバー
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b11_system_audit_number;
                                                // F12.現地取引日時
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[12];
    } b12_local_tran_time;
                                                // F24.ファンクションコード
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[3];
        char                filler;
    } b24_function_code;
                                                // F39.アクションコード
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[3];
        char                filler;
    } b39_action_code;
                                                // F72.通知レコード
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[340];
    } b72_notice_record;
                                                // F93.電文送信先センターID
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[11];
        char                filler;
    } b93_src_center_id;
                                                // F94.電文送信元センターID
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[11];
        char                filler;
    } b94_dst_center_id;
} fixedform_cardnet_1644_def;

/* 制御電文内部フォーマット[CARDNET] */
#pragma fieldalign shared2 __msg_cardnet_def
typedef struct __msg_cardnet_def
{
    MSG_HEADER_CARDNET_def  header;             // 共通制御ヘッダ&業務共通ヘッダ
    char                    mti[4];             // MTI
    char                    ffd;                // 電文固定フォーマット
} msg_cardnet_def;

#endif

