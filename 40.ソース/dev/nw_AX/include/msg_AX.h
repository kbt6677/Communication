/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････                                             */
/*        FUNCTION          ････ AMEX用メッセージヘッダー(共通ヘッダー)      */
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
#ifndef _msg_AX_H
#define _msg_AX_H

#include "GFPCGX20.h"

/* 制御電文 MTI(ASCII) */
#define DEF_AX_MTI_1804_REQ                 "1804"      /* 制御電文要求             */
#define DEF_AX_MTI_1814_RSP                 "1814"      /* 制御電文応答             */

/* (F24)Function Code */
#define DEF_AX_F24_FUNCTION_801_OPN         "801"       /* サインオン               */
#define DEF_AX_F24_FUNCTION_802_CLS         "802"       /* サインオフ               */
#define DEF_AX_F24_FUNCTION_831_ECH         "831"       /* エコーテスト             */

#define DEF_AX_CNTR_ID_LEN                  11          /* センターIDレングス               */
#define DEF_AX_SEND_SRC_CODE_LEN            11          /* 送信元識別コード長   */
#define DEF_AX_BIT03_DATA_LEN                6          /* Processing Code                  */
#define DEF_AX_BIT11_DATA_LEN                6          /* システムオーディットナンバー     */
#define DEF_AX_BIT12_DATA_LEN               12          /* 現地取引日時                     */
#define DEF_AX_BIT24_DATA_LEN                3          /* Function Code                    */
#define DEF_AX_BIT33_DATA_LEN               11          /* Forwarding Institution Identification Code */
#define DEF_AX_BIT39_DATA_LEN                3          /* アクションコード                 */


/* typedef定義 */
/* N/W情報ファイル・接続先固有情報 */
#pragma fieldalign shared2 __nwi_unq_info_ax
typedef struct __nwi_unq_info_ax
{
    char    future_use[100];
} nwi_unq_info_ax_def;
#define nwi_unq_info_ax_def_Size 100

/* 接続先固有情報ファイル・接続先固有情報 */
#pragma fieldalign shared2 __nws_unq_info_ax
typedef struct __nws_unq_info_ax
{
    char    send_src_code[11];
    char    future_use1[189];
} nws_unq_info_ax_def;
#define nws_unq_info_ax_def_Size 200





/* 制御電文固定フォーマット[AEGN] 1804 */
#pragma fieldalign shared2 __fixedform_aegn_1804_def
typedef struct __fixedform_aegn_1804_def
{
                                                // F003.Processing Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b03_processing_code;
                                                // F011.Systems Trace Audit Number
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b11_system_audit_number;
                                                // F012.Date and Time, Local Transaction
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[12];
    } b12_local_tran_time;
                                                // F024.Function Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[3];
        char                filler;
    } b24_function_code;
                                                // F033.Forwarding Institution Identification Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[11];
        char                filler;
    } b33_forwd_inst_id;
                                                // F048.Stand-In Parameter Information
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[4];
    } b48_stand_in_param_info;
} fixedform_aegn_1804_def;

/* 制御電文固定フォーマット[AEGN] 1814 */
#pragma fieldalign shared2 __fixedform_aegn_1814_def
typedef struct __fixedform_aegn_1814_def
{
                                                // F003.Processing Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b03_processing_code;
                                                // F011.Systems Trace Audit Number
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b11_system_audit_number;
                                                // F012.Date and Time, Local Transaction
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[12];
    } b12_local_tran_time;
                                                // F024.Function Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[3];
        char                filler;
    } b24_function_code;
                                                // F033.Forwarding Institution Identification Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[11];
        char                filler;
    } b33_forwd_inst_id;
                                                // F039.Action Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[3];
        char                filler;
    } b39_action_code;
} fixedform_aegn_1814_def;

/* 制御電文内部フォーマット[AGEN??] */
#pragma fieldalign shared2 __msg_aegn_def
typedef struct __msg_aegn_def
{
    char                    mti[4];             // MTI
    char                    ffd;                // 電文固定フォーマット
} msg_aegn_def;

#endif

