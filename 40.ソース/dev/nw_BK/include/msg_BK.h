/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････                                             */
/*        FUNCTION          ････ Banknet用メッセージヘッダー(共通ヘッダー)   */
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
#ifndef _msg_BK_H
#define _msg_BK_H

#include "GFPCGX20.h"

/* 制御電文 MTI(ASCII) */
#define DEF_BK_MTI_0800_REQ                 "0800"      /* 制御電文要求             */
#define DEF_BK_MTI_0810_RSP                 "0810"      /* 制御電文応答             */

/* (F70)Network Management Information Code */
#define DEF_BK_F70_INFOCODE_061_SON         "061"       /* サインオン               */
#define DEF_BK_F70_INFOCODE_062_SOF         "062"       /* サインオフ               */
#define DEF_BK_F70_INFOCODE_270_ECH         "270"       /* エコーテスト             */

#define DEF_BK_MTI_LEN                        4         /* MTIレングス              */
#define DEF_BK_BIT002_DATA_LEN               19         /* pan                      */
#define DEF_BK_BIT007_DATA_LEN               10         /* trans_date_time          */
#define DEF_BK_BIT011_DATA_LEN                6         /* system_audit_number      */
#define DEF_BK_BIT020_DATA_LEN                3         /* pan_country              */
#define DEF_BK_BIT033_DATA_LEN                6         /* forwd_inst_id            */
#define DEF_BK_BIT039_DATA_LEN                2         /* response_code            */
#define DEF_BK_BIT044_DATA_LEN               25         /* add_rsp_data             */
#define DEF_BK_BIT053_DATA_LEN               16         /* security_ctl_info        */
#define DEF_BK_BIT063_DATA_LEN               50         /* network_data             */
#define DEF_BK_BIT070_DATA_LEN                3         /* nw_mng_code              */
#define DEF_BK_BIT094_DATA_LEN                7         /* service_indicator        */
#define DEF_BK_BIT096_DATA_LEN                8         /* msg_security_code        */
#define DEF_BK_BIT127_DATA_LEN              100         /* private_data             */


/* typedef定義 */
/* N/W情報ファイル・接続先固有情報 */
#pragma fieldalign shared2 __nwi_unq_info_bk
typedef struct __nwi_unq_info_bk
{
    char    future_use[100];
} nwi_unq_info_bk_def;
#define nwi_unq_info_bk_def_Size 100

/* 接続先固有情報ファイル・接続先固有情報 */
#pragma fieldalign shared2 __nws_unq_info_bk
typedef struct __nws_unq_info_bk
{
    char    send_src_code[6];       // 送信元識別コード
    char    service_indicator[7];   // サービスインジケーター
    char    msg_sec_code[8];        // メッセージセキュリティコード
    char    grp_signon_id[5];       // グループサインオンID
    char    future_use1[174];       // 予備
} nws_unq_info_bk_def;
#define nws_unq_info_bk_def_Size 200

#define MSG_HEADER_BANKNET_ctrl_Size 47
#define MSG_HEADER_BANKNET_bh_Size 33
#define MSG_HEADER_BANKNET_def_Size 80


/* 制御電文固定フォーマット[BANKNET] 0800 */
#pragma fieldalign shared2 __fixedform_banknet_0800_def
typedef struct __fixedform_banknet_0800_def
{
                                                // F02. Primary Account Number (PAN)
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[19];
        char                filler;
    } b002_pan;
                                                // F07. Tranzaction Data and Time
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[10];
    } b007_trans_date_time;
                                                // F11. システムトレースオーディットナンバー
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b011_system_audit_number;
                                                // F20. Primary Account Number (PAN) Country Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[3];
        char                filler;
    } b020_pan_country;
                                                // F33. Forwarding Institution ID Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b033_forwd_inst_id;
                                                // F53. Security-Related Control Information
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[16];
    } b053_security_ctl_info;
                                                // F63. V.I.P. Private-Use Fields
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[50];
    } b063_network_data;
                                                // F70. Network Management Information Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[3];
        char                filler;
    } b070_nw_mng_code;
                                                // F94. Service Indicator
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[7];
        char                filler;
    } b094_service_indicator;
                                                // F96. Message Security Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[8];
    } b096_msg_security_code;
                                                // F127. Private Data
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[100];
    } b127_private_data;

} fixedform_banknet_0800_def;

/* 制御電文固定フォーマット[BANKNET] 0810 */
#pragma fieldalign shared2 __fixedform_banknet_0810_def
typedef struct __fixedform_banknet_0810_def
{
                                                // F02. Primary Account Number (PAN)
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[19];
        char                filler;
    } b002_pan;
                                                // F07. Tranzaction Data and Time
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[10];
    } b007_trans_date_time;
                                                // F11. システムトレースオーディットナンバー
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b011_system_audit_number;
                                                // F33. Forwarding Institution ID Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b033_forwd_inst_id;
                                                // F39. Response Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[2];
    } b039_response_code;
                                                // F44. Additional Response Data
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[25];
        char                filler;
    } b044_add_rsp_data;
                                                // F63. V.I.P. Private-Use Fields
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[50];
    } b063_network_data;
                                                // F70. Network Management Information Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[3];
        char                filler;
    } b070_nw_mng_code;
                                                // F127. Private Data
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[100];
    } b127_private_data;

} fixedform_banknet_0810_def;


/* 制御電文内部フォーマット[BANKNET] */
typedef struct __msg_cardnet_def
{
    char                    mti[4];             // MTI
    char                    ffd;                // 電文固定フォーマット
} msg_banknet_def;

#endif

