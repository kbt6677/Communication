/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSN80                                    */
/*        FUNCTION          ････ NW個別(鍵交換個別処理[NYCE])                */
/*                               鍵交換個別処理[NYCE])を行う。               */
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
#ifndef _GFPCSN80_H
#define _GFPCSN80_H

/* 電文チェック用DEFINE */
//#define  DEF_NWM_KYX_SPACE               ' '

//#define  DEF_NWM_KYX_BITMAP_MAX          96         /* 確認対象BITMAP最大値 */
#define  DEF_NWM_KYX_BIT_007             "0007"     /* BIT番号              */
#define  DEF_NWM_KYX_BIT_011             "0011"     /* BIT番号              */
#define  DEF_NWM_KYX_BIT_039             "0039"     /* BIT番号              */
#define  DEF_NWM_KYX_BIT_070             "0070"     /* BIT番号              */
#define  DEF_NWM_KYX_BIT_096             "0096"     /* BIT番号              */
#define  DEF_NWM_KYX_BIT_125             "0125"     /* BIT番号              */
//#define  DEF_NWM_KYX_BIT_007_ERR         "0007"     /* エラー発生BIT        */
//#define  DEF_NWM_KYX_BIT_011_ERR         "0011"     /* エラー発生BIT        */
//#define  DEF_NWM_KYX_BIT_039_ERR         "0039"     /* エラー発生BIT        */
//#define  DEF_NWM_KYX_BIT_070_ERR         "0070"     /* エラー発生BIT        */
//#define  DEF_NWM_KYX_BIT_096_ERR         "0096"     /* エラー発生BIT        */
//#define  DEF_NWM_KYX_BIT_125_ERR         "0125"     /* エラー発生BIT        */
#define  DEF_NWM_KYX_HEADER_MTI_ERR      "0901"     /* エラー発生共通ヘッダ(MTI) */
#define  DEF_NWM_KYX_BIT_007_LENG        10         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_011_LENG         6         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_039_LENG         2         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_070_LENG         3         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_096_LENG         8         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_125_LENG       256         /* BITごとデータ長      */

#define  DEF_NWM_KYX_BIT_070_171         "171"      /* BITごとデータ(BIT70設定値)      */
#define  DEF_NWM_KYX_BIT_070_191         "191"      /* BITごとデータ(BIT70設定値)      */
#define  DEF_NWM_KYX_BIT_070_KYETYPE_LENG 3         /* BITごとデータ(BIT70データ長)    */
#define  DEF_NWM_KYX_BIT_125_SUB_TAG_01  "01"       /* BITごとデータ(BIT125設定値：TAG番号) */
#define  DEF_NWM_KYX_BIT_125_SUB_TAG_02  "02"       /* BITごとデータ(BIT125設定値：TAG番号) */
#define  DEF_NWM_KYX_BIT_125_SUBTAG_LEN   2         /* BITごとデータ(BIT125サブタグ長) */
#define  DEF_NWM_KYX_BIT_125_PRITAG_KP   "KP"       /* BITごとデータ(BIT125設定値：KP) */

#define  DEF_NWM_ERR_ACTION_CODE_00     "00"       /* レスポンスアクションコード(正常応答)  */
#define  DEF_NWM_ERR_ACTION_CODE_76     "76"       /* レスポンスアクションコード(拒否応答)  */

//#define  DEF_NWM_FILEIO_TYPE_READ       "READ    "

#define  DEF_NWM_KYX_MAX_CKDIGIT_LEN    32         /* チェックディジット長制限値 */
#define  DEF_NWM_KYX_MAX_KEY_LEN        256        /* キー長制限値               */

#define  DEF_NWM_KYX_MSG_KIND_REQRSP     1         /* 鍵交換電文種別(精査/局状態判定) 鍵交換依頼応答         */
#define  DEF_NWM_KYX_MSG_KIND_KEYRSP     2         /* 鍵交換電文種別(精査/局状態判定) GFP契機鍵交換応答      */
#define  DEF_NWM_KYX_MSG_KIND_IRAIREQ    1         /* 鍵交換電文種別(編集) 鍵交換依頼時の要求電文作成        */
#define  DEF_NWM_KYX_MSG_KIND_KEYREQ     2         /* 鍵交換電文種別(編集) GFP契機鍵交換要求時の要求電文作成 */

#define  DEF_NWM_KYX_RTN_OK              0         /* 戻り値 正常                      */
#define  DEF_NWM_KYX_RTN_NG_REJ          1         /* 戻り値 拒否応答                  */
#define  DEF_NWM_KYX_RTN_FAULT_MSG       2         /* 戻り値 障害電文通知              */
#define  DEF_NWM_KYX_RTN_HAKI_MSG        3         /* 戻り値 電文破棄                  */
#define  DEF_NWM_KYX_RTN_ERR             9         /* 戻り値 異常                      */

#define  DEF_NWM_KYX_RTN_NG             -1

#define  DEF_NWM_KYX_KIND_KPE           "KPE "
//#define  DEF_NWM_KYX_KIND_KMAC          "KMAC"
//#define  DEF_NWM_KYX_KIND_KC            "KC  "

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

#endif

