/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSJ80                                    */
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
#ifndef _GFPCSJ80_H
#define _GFPCSJ80_H

/* 電文チェック用DEFINE */
#define  DEF_NWM_KYX_SPACE               ' '

//#define  DEF_NWM_KYX_HED_TYPE            "F1"       /* ヘッダータイプ       */
//#define  DEF_NWM_KYX_HED_GYOMU_TYPE      "A1"       /* 業務ヘッダータイプ   */
//#define  DEF_NWM_KYX_HED_MSG_KIND_REQ    "C804"     /* 電文種別コード       */
//#define  DEF_NWM_KYX_HED_MSG_KIND_RSP    "C814"     /* 電文種別コード       */
//#define  DEF_NWM_KYX_MTI_1804            "1804"     /* MTI                  */
//#define  DEF_NWM_KYX_mode_test_rcv       0x10       /* モードフラグ(試験)(受信電文)*/
//#define  DEF_NWM_KYX_mode_honban_rcv     0x00       /* モードフラグ(本番)(受信電文)*/
#define  DEF_NWM_KYX_mode_test_nw        '1'        /* モードフラグ(試験)(db_NW)*/
#define  DEF_NWM_KYX_mode_honban_nw      '0'        /* モードフラグ(本番)(db_NW)*/
#define  DEF_NWM_KYX_BITMAP_MAX          96         /* 確認対象BITMAP最大値 */
#define  DEF_NWM_KYX_BIT_11              "0011"     /* BIT番号              */
#define  DEF_NWM_KYX_BIT_12              "0012"     /* BIT番号              */
#define  DEF_NWM_KYX_BIT_24              "0024"     /* BIT番号              */
#define  DEF_NWM_KYX_BIT_53              "0053"     /* BIT番号              */
#define  DEF_NWM_KYX_BIT_93              "0093"     /* BIT番号              */
#define  DEF_NWM_KYX_BIT_94              "0094"     /* BIT番号              */
#define  DEF_NWM_KYX_BIT_96              "0096"     /* BIT番号              */
#define  DEF_NWM_KYX_BIT_11_ERR          "0011"     /* エラー発生BIT        */
#define  DEF_NWM_KYX_BIT_12_ERR          "0012"     /* エラー発生BIT        */
#define  DEF_NWM_KYX_BIT_24_ERR          "0024"     /* エラー発生BIT        */
#define  DEF_NWM_KYX_BIT_53_ERR          "0053"     /* エラー発生BIT        */
#define  DEF_NWM_KYX_BIT_93_ERR          "0093"     /* エラー発生BIT        */
#define  DEF_NWM_KYX_BIT_94_ERR          "0094"     /* エラー発生BIT        */
#define  DEF_NWM_KYX_BIT_96_ERR          "0096"     /* エラー発生BIT        */
#define  DEF_NWM_KYX_HEADER_901_ERR      "0901"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_HEADER_902_ERR      "0902"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_HEADER_903_ERR      "0903"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_HEADER_904_ERR      "0904"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_HEADER_905_ERR      "0905"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_HEADER_906_ERR      "0906"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_HEADER_907_ERR      "0907"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_HEADER_908_ERR      "0908"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_HEADER_909_ERR      "0909"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_HEADER_910_ERR      "0910"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_HEADER_911_ERR      "0911"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_HEADER_912_ERR      "0912"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_HEADER_913_ERR      "0913"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_HEADER_914_ERR      "0914"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_HEADER_915_ERR      "0915"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_HEADER_916_ERR      "0916"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_HEADER_917_ERR      "0917"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_HEADER_918_ERR      "0918"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_HEADER_919_ERR      "0919"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_HEADER_920_ERR      "0920"     /* エラー発生共通ヘッダ */
#define  DEF_NWM_KYX_BIT_11_LENG          6         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_12_LENG         12         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_24_LENG          3         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_53_LENG         14         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_93_LENG         15         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_94_LENG         13         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_96_LENG_MAC_11  11         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_96_LENG_MAC_19  19         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_96_LENG_KPE_08   8         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_96_LENG_KPE_16  16         /* BITごとデータ長      */
#define  DEF_NWM_KYX_BIT_96_ENC          "ENC"      /* BITごとデータ        */
#define  DEF_NWM_KYX_BIT_96_MAC          "MAC"      /* BITごとデータ        */
#define  DEF_NWM_KYX_BIT_96_KYETYPE_LENG 3          /* BITごとデータ        */
#define  DEF_NWM_KYX_BIT_53_KEYTYPE      "99"       /* KEY TYPE             */
#define  DEF_NWM_KYX_BIT_53_PINENC_01    "01"       /* PIN暗号化ロジック    */
#define  DEF_NWM_KYX_BIT_53_PINENC_02    "02"       /* PIN暗号化ロジック    */
#define  DEF_NWM_KYX_BIT_53_PINBLK       "01"       /* PINブロック形式      */
#define  DEF_NWM_KYX_BIT_53_KEYINDEX     "00"       /* KEY索引値            */
#define  DEF_NWM_KYX_BIT_53_AUTHINDEX    "00"       /* 認証索引値           */
#define  DEF_NWM_KYX_BIT_53_CHKDIGIT     "0000"     /* チェックディジット   */

#define  DEF_NWM_ERR_800                 "800"      /* 正常                 */
#define  DEF_NWM_ERR_910                 "910"      /* 局状態エラー                 */
#define  DEF_NWM_ERR_917                 "917"      /* 暗号化キー同期エラー(KMAC)   */
#define  DEF_NWM_ERR_919                 "919"      /* 暗号化キー同期エラー(KPE,KC) */
#define  DEF_NWM_ERR_904                 "904"      /* フォーマットエラー           */

#define DEF_NWM_FILEIO_TYPE_READ         "READ    "

/* プロトタイプ */
short NWM_KYX_check_datetime(char*);
//void  HEX2CHAR(unsigned char *, char *,short );
short NWM_KYX_CHAR2BCD(unsigned char *, char *, short);
short NWM_KYX_read_cutfile(file_info_gccut*, gflin_pkey_def*, db_gccut_def*,
                           oggz1in_def*, NWM_KYX_ems_add*);
void  NWM_KYX_ems_output(char*, char*, COM_IOM_arg_5_def*, COM_IOM_arg_6_def*,
                         gflin_pkey_def*, oggz1in_def*,NWM_KYX_ems_add*);
short NWM_KYX_CHAR2HEX(const char*, char*, short);
void  NWM_KYX_BCD2CHAR(unsigned char *, char *,short);
short NWM_KYX_SHORT2BCD(unsigned short, char *);

#endif

