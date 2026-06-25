/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSU80                                    */
/*        FUNCTION          ････ NW個別(鍵交換個別処理[UnionPay])            */
/*                               鍵交換個別処理[UnionPay])を行う。           */
/*                                                                           */
/*        AUTHER            ････ HAS                                         */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-09-10                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS        2025/09/10 新規作成                                      */
/*                                                                           */
/*****************************************************************************/
#ifndef _GFPCSU80_H
#define _GFPCSU80_H

/* 電文チェック用DEFINE */
#define  DEF_NWM_KYX_SPACE               ' '
#define  DEF_NWM_KYX_ZERO                '0'
#define  DEF_NWM_KYX_NULL                0
#define  DEF_NWM_KYX_CHAR_ON             '1'

//#define  DEF_NWM_KYX_mode_test_rcv       0x10       /* モードフラグ(試験)(受信電文)*/
//#define  DEF_NWM_KYX_mode_test_nw        '1'        /* モードフラグ(試験)(db_NW)*/
//#define  DEF_NWM_KYX_mode_honban_nw      '0'        /* モードフラグ(本番)(db_NW)*/
#define  DEF_NWM_KYX_BITMAP_MAX          96         /* 確認対象BITMAP最大値 */
#define  DEF_NWM_KYX_BIT_007             "0007"     /* エラー発生BIT番号      */
#define  DEF_NWM_KYX_BIT_011             "0011"     /* エラー発生BIT番号      */
#define  DEF_NWM_KYX_BIT_033             "0033"     /* エラー発生BIT番号      */
#define  DEF_NWM_KYX_BIT_039             "0039"     /* エラー発生BIT番号      */
#define  DEF_NWM_KYX_BIT_048             "0048"     /* エラー発生BIT番号      */
#define  DEF_NWM_KYX_BIT_053             "0053"     /* エラー発生BIT番号      */
#define  DEF_NWM_KYX_BIT_070             "0070"     /* エラー発生BIT番号      */
#define  DEF_NWM_KYX_BIT_096             "0096"     /* エラー発生BIT番号      */
#define  DEF_NWM_KYX_BIT_100             "0100"     /* エラー発生BIT番号      */
#define  DEF_NWM_KYX_BIT_128             "0128"     /* エラー発生BIT番号      */
//#define  DEF_NWM_KYX_BIT_007_ERR         "0007"     /* エラー発生BIT        */
//#define  DEF_NWM_KYX_BIT_011_ERR         "0011"     /* エラー発生BIT        */
//#define  DEF_NWM_KYX_BIT_033_ERR         "0033"     /* エラー発生BIT        */
//#define  DEF_NWM_KYX_BIT_039_ERR         "0039"     /* エラー発生BIT        */
//#define  DEF_NWM_KYX_BIT_048_ERR         "0048"     /* エラー発生BIT        */
//#define  DEF_NWM_KYX_BIT_053_ERR         "0053"     /* エラー発生BIT        */
//#define  DEF_NWM_KYX_BIT_070_ERR         "0070"     /* エラー発生BIT        */
//#define  DEF_NWM_KYX_BIT_096_ERR         "0096"     /* エラー発生BIT        */
//#define  DEF_NWM_KYX_BIT_100_ERR         "0100"     /* エラー発生BIT        */
//#define  DEF_NWM_KYX_BIT_128_ERR         "0128"     /* エラー発生BIT        */
#define  DEF_NWM_KYX_HEADER_MTI_ERR      "0901"     /* エラー発生共通ヘッダ(MTI)                  */
#define  DEF_NWM_KYX_HEADER_HEAD_LEN_ERR "0902"     /* エラー発生共通ヘッダ(ヘッダー部電文長)     */
#define  DEF_NWM_KYX_HEADER_TTL_LEN_ERR  "0903"     /* エラー発生共通ヘッダ(Total Message Length) */
#define  DEF_NWM_KYX_HEADER_MOD_FLG_ERR  "0904"     /* エラー発生共通ヘッダ(モードフラグ)         */
#define  DEF_NWM_KYX_BIT_007_LENG        10         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_011_LENG         6         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_033_LENG        11         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_039_LENG         2         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_048_LENG         2         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_053_LENG        16         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_070_LENG         3         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_096_LENG         8         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_100_LENG        13         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_128_LENG         8         /* BITごとデータ長      */

#define  DEF_NWM_KYX_BIT_048_USAGEID_KB  "KB"       /* BITごとデータ        */
#define  DEF_NWM_KYX_BIT_070_101         "101"      /* BITごとデータ        */
#define  DEF_NWM_KYX_BIT_070_KYETYPE_LENG 3         /* BITごとデータ        */

#define  DEF_NWM_ACTION_CODE_OK           "00"      /* レスポンスアクションコード(正常応答) */
#define  DEF_NWM_ACTION_CODE_KYOHI        "A7"      /* レスポンスアクションコード(拒否応答) */

//#define  DEF_NWM_KYX_MTI_0800           "0800"
//#define  DEF_NWM_KYX_MTI_0810           "0810"
//#define  DEF_NWM_KYX_MTI_0820           "0820"
//#define  DEF_NWM_KYX_MTI_0830           "0830"

//#define  DEF_NWM_KYX_MAX_KEY_LEN        256        /* キー長制限値               */

#define  DEF_NWM_KYX_TRANS_INF          "00000000" /* Transaction Information    */
#define  DEF_NWM_KYX_KEY_PIK            '1'        /* Key Type                   */
#define  DEF_NWM_KYX_ALG_DS1            "DS1"      /* Data Key Algorithm         */
#define  DEF_NWM_KYX_ALG_DS2            "DS2"      /* Data Key Algorithm         */
#define  DEF_NWM_KYX_ALG_DS3            "DS3"      /* Data Key Algorithm         */
#define  DEF_NWM_KYX_ALG_DS1_NUM        '0'        /* Data Key Algorithm 番号    */
#define  DEF_NWM_KYX_ALG_DS2_NUM        '6'        /* Data Key Algorithm 番号    */
#define  DEF_NWM_KYX_ALG_DS3_NUM        '7'        /* Data Key Algorithm 番号    */
#define  DEF_NWM_KYX_FMT_VA             "VA"       /* Data Key Format            */
#define  DEF_NWM_KYX_FMT_KB             "KB"       /* Data Key Format            */
#define  DEF_NWM_KYX_FMT_VA_NUM         '0'        /* Data Key Format 番号       */
#define  DEF_NWM_KYX_FMT_KB_NUM         '1'        /* Data Key Format 番号       */
//#define  DEF_NWM_KYX_DST_ID             "00010344   "

#define  DEF_NWM_KYX_NWS_NW             0          /* 接続先固有情報ファイルレコード配列番号 */
#define  DEF_NWM_KYX_NWS_IF             1          /* 接続先固有情報ファイルレコード配列番号 */
#define  DEF_NWM_KYX_NWS_ST             2          /* 接続先固有情報ファイルレコード配列番号 */
#define  DEF_NWM_KYX_NWS_CN             3          /* 接続先固有情報ファイルレコード配列番号 */

/* プロトタイプ */
void  NWM_KYX_ASCII2SHORT(unsigned char*, short *, short);

#endif

