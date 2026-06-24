/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････                                             */
/*        FUNCTION          ････ Unionpay用メッセージヘッダー(共通ヘッダー)  */
/*        AUTHER            ････ HAS Matsumoto                               */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-03-28                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  Matsumoto  2025/03/28 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
#ifndef _msg_UP_H
#define _msg_UP_H

#include "GFPCGX20.h"

/* 制御電文 MTI */
#define DEF_UP_MTI_0820_REQ                 "0820"      /* 制御電文要求         */
#define DEF_UP_MTI_0830_RSP                 "0830"      /* 制御電文応答         */
#define DEF_UP_MTI_0800_KEYREQ              "0800"      /* 鍵交換要求           */
#define DEF_UP_MTI_0810_KEYRSP              "0810"      /* 鍵交換要求           */

/* (F70)Network Management Information Code */
#define DEF_UP_F70_INFOCODE_001_OPN         "001"       /* 開局                 */
#define DEF_UP_F70_INFOCODE_002_CLS         "002"       /* 閉局                 */
#define DEF_UP_F70_INFOCODE_101_KEY         "101"       /* 鍵交換               */
#define DEF_UP_F70_INFOCODE_201_CUTBEG      "201"       /* カットオフ開始       */
#define DEF_UP_F70_INFOCODE_202_CUTEND      "202"       /* カットオフ終了       */
#define DEF_UP_F70_INFOCODE_301_ECH         "301"       /* エコーテスト         */

#define DEF_UP_CNTR_ID_LEN                  11          /* 送信先/送信元ID長    */
#define DEF_UP_BIT007_DATA_LEN              10          /* trans_date_time      */

#define DEF_UN_MODE_01_HONBAN               0x01        /* モードフラグ: 本番モード */
#define DEF_UN_MODE_81_TEST                 0x81        /* モードフラグ: 試験モード */

#define DEF_UN_DST_ID                       "00010344"  /* Destination ID       */

/* typedef定義 */
/* N/W情報ファイル・接続先固有情報 */
#pragma fieldalign shared2 __nwi_unq_info_up
typedef struct __nwi_unq_info_up
{
    char    mode_flg;
    char    future_use[99];
} nwi_unq_info_up_def;
#define nwi_unq_info_up_def_Size 100

/* 接続先固有情報ファイル・接続先固有情報 */
#pragma fieldalign shared2 __nws_unq_info_up
typedef struct __nws_unq_info_up
{
    char    send_src_code[11];
    char    future_use1[189];
} nws_unq_info_up_def;
#define nws_unq_info_up_def_Size 200

/* システム間インターフェース・UnionPay・電文ヘッダ */
#pragma fieldalign shared2 __MSG_HEADER_UNIONPAY
typedef struct __MSG_HEADER_UNIONPAY
{
    char    mh_hdr_len         ;    /* Header Length            */
    char    mh_hdr_flg_ver     ;    /* Header Flag and Version  */
    char    mh_tot_len      [4];    /* Total Message Length     */
    char    mh_dst_id       [11];   /* Destination ID           */
    char    mh_src_id       [11];   /* Source ID                */
    char    mh_rsv_use      [3];    /* Reserved for Use         */
    char    mh_bat_num         ;    /* Batch Number             */
    char    mh_tran_info    [8];    /* Transaction Information  */
    char    mh_usr_info        ;    /* User Information         */
    char    mh_rjct_code    [5];    /* Reject Code              */
} MSG_HEADER_UNIONPAY_def;

#define MSG_HEADER_UNIONPAY_def_Size 46


/* 制御電文固定フォーマット[unionpay] 820 */
#pragma fieldalign shared2 __fixedform_unionpay_0820_def
typedef struct __fixedform_unionpay_0820_def

{
                                                // Transmission Date and Time
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[10];
    } b007_trans_date_time;
                                                // System Trace Audit Number
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b011_system_audit_number;
                                                // Forwarding Institution Identification Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[11];
        char                filler;
    } b033_fowd_inst_id_code;
                                                // Security Related Control Information
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[16];
    } b053_secur_ctl_info;
                                                // Network Management Information Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[03];
        char                filler;
    } b070_nw_mng_code;
                                                // Receiving Institution Identification Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[11];
        char                filler;
    } b100_recv_inst_id_code;
} fixedform_unionpay_0820_def;

/* 制御電文固定フォーマット[unionpay] 830 */
#pragma fieldalign shared2 __fixedform_unionpay_0830_def
typedef struct __fixedform_unionpay_0830_def
{
                                                // Transmission Date and Time
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[10];
    } b007_trans_date_time;
                                                // System Trace Audit Number
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b011_system_audit_number;
                                                // Forwarding Institution Identification Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[11];
        char                filler;
    } b033_fowd_inst_id_code;
                                                // Response Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[2];
    } b039_response_code;
                                                // Security Related Control Information
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[16];
    } b053_secur_ctl_info;
                                                // Network Management Information Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[3];
        char                filler;
    } b070_nw_mng_code;
                                                // Receiving Institution Identification Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[11];
        char                filler;
    } b100_recv_inst_id_code;
} fixedform_unionpay_0830_def;

/* 制御電文固定フォーマット[unionpay] 800 */
#pragma fieldalign shared2 __fixedform_unionpay_0800_def
typedef struct __fixedform_unionpay_0800_def
{

                                                // Transmission Date and Time
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[10];
    } b007_trans_date_time;
                                                // System Trace Audit Number
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b011_system_audit_number;
                                                // Additional Data-Private
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[512];
    } b048_add_data_private;
                                                // Security Related Control Information
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[16];
    } b053_secur_ctl_info;
                                                // Network Management Information Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[3];
        char                filler;
    } b070_nw_mng_code;
                                                // Message Security Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[8];
    } b096_msg_security_code;
                                                // Receiving Institution Identification Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[11];
        char                filler;
    } b100_recv_inst_id_code;
                                                // Message Authentication Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[8];
    } b128_msg_auth_code;
} fixedform_unionpay_0800_def;

/* 制御電文固定フォーマット[unionpay] 810 */
#pragma fieldalign shared2 __fixedform_unionpay_0810_def
typedef struct __fixedform_unionpay_0810_def
{


                                                // Transmission Date and Time
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[10];
    } b007_trans_date_time;
                                                // System Trace Audit Number
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b011_system_audit_number;
                                                // Response Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[2];
    } b039_response_code;
                                                // Security Related Control Information
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[16];
    } b053_secur_ctl_info;
                                                // Network Management Information Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[3];
        char                filler;
    } b070_nw_mng_code;
                                                // Receiving Institution Identification Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[11];
        char                filler;
    } b100_recv_inst_id_code;
                                                // Message Authentication Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[8];
    } b128_msg_auth_code;
} fixedform_unionpay_0810_def;

/* 制御電文内部フォーマット[unionpay] */
#pragma fieldalign shared2 __msg_unionpay_def
typedef struct __msg_unionpay_def
{
    MSG_HEADER_UNIONPAY_def header;             // 共通制御ヘッダ&業務共通ヘッダ
    char                    mti[4];             // MTI
    char                    ffd;                // 電文固定フォーマット
} msg_unionpay_def;

// 注)Message Lengthは電文振分(inbound)で処理されます

#endif

