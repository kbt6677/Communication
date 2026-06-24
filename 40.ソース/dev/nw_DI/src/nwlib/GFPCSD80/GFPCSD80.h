/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCSD80                                    */
/*        FUNCTION          ････ 鍵交換個別処理[Discover]                    */
/*                                                                           */
/*                               鍵交換制御のNW個別処理(Discover)の処理を行う*/
/*                                                                           */
/*        AUTHER            ････ ISYS N.Miki                                 */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-04-20                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  N.Miki     2025/04/30 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
/* -------------------------------------------------------*/
/* 定数定義                                               */
/* -------------------------------------------------------*/
#define DEF_ERR_BIT7               "0007"           // エラー発生ビット番号：7
#define DEF_ERR_BIT11              "0011"           // エラー発生ビット番号：11
#define DEF_ERR_BIT32              "0032"           // エラー発生ビット番号：32
#define DEF_ERR_BIT37              "0037"           // エラー発生ビット番号：37
#define DEF_ERR_BIT39              "0039"           // エラー発生ビット番号：39
#define DEF_ERR_BIT44              "0044"           // エラー発生ビット番号：44
#define DEF_ERR_BIT48              "0048"           // エラー発生ビット番号：48
#define DEF_ERR_BIT53              "0053"           // エラー発生ビット番号：53
#define DEF_ERR_BIT59              "0059"           // エラー発生ビット番号：59
#define DEF_ERR_BIT70              "0070"           // エラー発生ビット番号：70
#define DEF_ERR_BIT100             "0100"           // エラー発生ビット番号：100
#define DEF_ERR_BIT120             "0120"           // エラー発生ビット番号：120
#define DEF_ERR_BIT127             "0127"           // エラー発生ビット番号：127
#define DEF_ERR_BITSIZE            4                // エラー発生ビット番号サイズ
#define DEF_BIT39_00               "00"             // BIT39精査値00
#define DEF_BIT39_96               "96"             // BIT39精査値96
#define DEF_IN_ERR_CODE_OK         "0000000"        // 内部エラーコード：正常

#define DEF_BIT70_101              "101"            // BIT70精査値101
#define DEF_CHECK_DIGIT_SIZE_KB    6                // check_digitサイズ（TR-31 Key Block）
#define DEF_BIT53_DATA             "0004000100"     // bit53設定値
#define DEF_BIT53_CK_DI_OFSET      10               // bit53チェックデジットオフセット
#define DEF_BIT53_CK_DI_SIZE       6                // bit53チェックデジットサイズ

#define DEF_KEY_DS1                "DS1"
#define DEF_KEY_DS2                "DS2"
#define DEF_KEY_DS3                "DS3"
#define DEF_KEY_FMT_VA             "VA"             // Variant（ANSI X9.17）
#define DEF_KEY_FMT_KB             "KB"             // Key Block（ANSI X9 TR31 key Block）


/*===========================================================================*/
/*   プロトタイプ宣言                                                        */
/*===========================================================================*/
short   CMIN_check_datetime(char*);                 // 日付形式チェック
short   CMIN_get_day_of_year(char*, char*);         // 通算日算出処理  
bool    NWM_KYX_isdigit(char*,unsigned long);
