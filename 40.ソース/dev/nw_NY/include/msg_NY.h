/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････                                             */
/*        FUNCTION          ････ NYCE用メッセージヘッダー(共通ヘッダー)      */
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
#ifndef _msg_NY_H
#define _msg_NY_H

#include "GFPCGX20.h"

/* 制御電文 MTI(ASCII) */
#define  DEF_NY_MTI_0800_REQ                "0800"      /* 制御電文要求             */
#define  DEF_NY_MTI_0810_RSP                "0810"      /* 制御電文応答             */

/* (F70)Network Management Information Code */
#define  DEF_NY_F70_INFOCODE_061_SON        "061"       /* サインオン from Issuer to NYCE   */
#define  DEF_NY_F70_INFOCODE_062_SOF        "062"       /* サインオフ                       */
#define  DEF_NY_F70_INFOCODE_071_SON        "071"       /* サインオン from NYCE to Issuer   */
#define  DEF_NY_F70_INFOCODE_072_SOF        "072"       /* サインオフ                       */
#define  DEF_NY_F70_INFOCODE_171_KEY        "171"       /* 鍵交換 from NYCE to Issuer       */
#define  DEF_NY_F70_INFOCODE_191_KEY        "191"       /* 鍵交換依頼 from Issuer to NYCE   */
#define  DEF_NY_F70_INFOCODE_271_EODBEG     "271"       /* EOD開始 from NYCE to Issuer      */
#define  DEF_NY_F70_INFOCODE_272_EODEND     "272"       /* EOD終了 from NYCE to Issuer      */
#define  DEF_NY_F70_INFOCODE_361_ECH        "361"       /* エコーテスト from Issuer to NYCE */
#define  DEF_NY_F70_INFOCODE_371_ECH        "371"       /* エコーテスト from NYCE to Issuer */

/* typedef定義 */
/* N/W情報ファイル・接続先固有情報 */
#pragma fieldalign shared2 __nwi_unq_info_ny
typedef struct __nwi_unq_info_ny
{
    char    future_use[100];
} nwi_unq_info_ny_def;
#define nwi_unq_info_ny_def_Size 100

/* 接続先固有情報ファイル・接続先固有情報 */
#pragma fieldalign shared2 __nws_unq_info_ny
typedef struct __nws_unq_info_ny
{
    char    msg_sec_code[8];
    char    future_use1[192];
} nws_unq_info_ny_def;
#define nws_unq_info_ny_def_Size 200


/* 制御電文固定フォーマット[NYCE] 800 */
#pragma fieldalign shared2 __fixedform_nyce_0800_def
typedef struct __fixedform_nyce_0800_def
{
                                                // F007.Transmission Date and Time
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[10];
    } b007_trans_date_time;
                                                // F011.Systems Trace Audit Number
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b011_system_audit_number;
                                                // F070.Network Management Information Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[3];
        char                filler;
    } b070_nw_mng_code;
                                                // F096.Message Security Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[8];
    } b096_msg_security_code;
                                                // F125.Network Management Information
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[259];
        char                filler;
    } b125_nw_mng_info;
} fixedform_nyce_0800_def;


/* 制御電文固定フォーマット[NYCE] 810 */
#pragma fieldalign shared2 __fixedform_nyce_0810_def
typedef struct __fixedform_nyce_0810_def
{
                                                // F007.Transmission Date and Time
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[10];
    } b007_trans_date_time;
                                                // F011.Systems Trace Audit Number
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b011_system_audit_number;
                                                // F039.Response Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[2];
    } b039_response_code;
                                                // F070.Network Management Information Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[3];
        char                filler;
    } b070_nw_mng_code;
} fixedform_nyce_0810_def;

/* 制御電文内部フォーマット[NYCE] */
#pragma fieldalign shared2 __msg_nyce_def
typedef struct __msg_nyce_def
{
    char                    mti[4];             // MTI
    char                    ffd;                // 電文固定フォーマット
} msg_nyce_def;



#endif

