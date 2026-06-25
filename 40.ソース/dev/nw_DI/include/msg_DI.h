/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････                                             */
/*        FUNCTION          ････ Discover用メッセージヘッダー(共通ヘッダー)  */
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
#ifndef _msg_DI_H
#define _msg_DI_H


/*****************************************************************************/
/*   DEFINE定義                                                              */
/*****************************************************************************/
#define DEF_NW_MOVE_MODE_I                  'I'         // ネットワーク情報動作モード:I(GFP as Issuer)
#define DEF_NW_MOVE_MODE_A                  'A'         // ネットワーク情報動作モード:I(GFP as Acquirer)

#ifndef DEF_MTI_0800
#define DEF_MTI_0800                        "0800"      // MTI:0800
#endif
#ifndef DEF_MTI_0810
#define DEF_MTI_0810                        "0810"      // MTI:0810
#endif
#ifndef DEF_MTI_0820
#define DEF_MTI_0820                        "0820"      // MTI:0820
#endif
#ifndef DEF_MTI_0830
#define DEF_MTI_0830                        "0830"      // MTI:0830
#endif

#define DEF_MTI_SIZE                          4
#define DEF_BIT7_SET_SIZE                    10
#define DEF_BIT11_SET_SIZE                    6
#define DEF_BIT32_SET_SIZE                   11
#define DEF_BIT37_SET_SIZE                   12
#define DEF_BIT39_SET_SIZE                    2
#define DEF_BIT44_SET_SIZE                   99
#define DEF_BIT48_SET_SIZE                   32
#define DEF_BIT53_SET_SIZE                   16
#define DEF_BIT59_SET_SIZE                  100
#define DEF_BIT70_SET_SIZE                    3
#define DEF_BIT100_SET_SIZE                  11
#define DEF_BIT120_SET_SIZE                 999
#define DEF_BIT127_SET_SIZE                   5
#define DEF_PIN_ENC_SET_SIZE                  2

#define DEF_BIT127_SET_DATA               "03141"     // bit127設定値

#define DEF_BIT70_061                       "061"       // 固定フォーマットbit70：061
#define DEF_BIT70_062                       "062"       // 固定フォーマットbit70：062
#define DEF_BIT70_101                       "101"       // 固定フォーマットbit70：101
#define DEF_BIT39_N1                        "N1"        // 固定フォーマットbit39：N1
#define DEF_BIT39_N2                        "N2"        // 固定フォーマットbit39：N2
#define DEF_BIT39_N3                        "N3"        // 固定フォーマットbit39：N3
#define DEF_BIT53_04                        "04"        // 固定フォーマットbit53：53



/* typedef定義 */
/* 制御電文内部フォーマット[Discover] */
#pragma fieldalign shared2 __msg_discover_def
typedef struct __msg_discover_def
{
    char                    mti[DEF_MTI_SIZE];  // MTI
    char                    ffd;                // 電文固定フォーマット
} msg_discover_def;


#pragma fieldalign shared2 __bit7_def
typedef struct __bit7_def
{
    bool                flag;
    char                filler1;
    unsigned long       data_len;
    char                data[DEF_BIT7_SET_SIZE];
}bit7_def;
#pragma fieldalign shared2 __bit11_def
typedef struct __bit11_def
{
    bool                flag;
    char                filler1;
    unsigned long       data_len;
    char                data[DEF_BIT11_SET_SIZE];
}bit11_def;
#pragma fieldalign shared2 __bit32_def
typedef struct __bit32_def
{
    bool                flag;
    char                filler1;
    unsigned long       data_len;
    char                data[DEF_BIT32_SET_SIZE];
    char                filler2;
}bit32_def;
#pragma fieldalign shared2 __bit37_def
typedef struct __bit37_def
{
    bool                flag;
    char                filler1;
    unsigned long       data_len;
    char                data[DEF_BIT37_SET_SIZE];
}bit37_def;
#pragma fieldalign shared2 __bit39_def
typedef struct __bit39_def
{
    bool                flag;
    char                filler1;
    unsigned long       data_len;
    char                data[DEF_BIT39_SET_SIZE];
}bit39_def;
#pragma fieldalign shared2 __bit44_def
typedef struct __bit44_def
{
    bool                flag;
    char                filler1;
    unsigned long       data_len;
    char                data[DEF_BIT44_SET_SIZE];
    char                filler2;
}bit44_def;
#pragma fieldalign shared2 __bit48_def
typedef struct __bit48_def
{
    bool                flag;
    char                filler1;
    unsigned long       data_len;
    char                data[DEF_BIT48_SET_SIZE];
}bit48_def;
#pragma fieldalign shared2 __bit53_def
typedef struct __bit53_def
{
    bool                flag;
    char                filler1;
    unsigned long       data_len;
    char                data[DEF_BIT53_SET_SIZE];
}bit53_def;
#pragma fieldalign shared2 __bit59_def
typedef struct __bit59_def
{
    bool                flag;
    char                filler1;
    unsigned long       data_len;
    char                data[DEF_BIT59_SET_SIZE];
}bit59_def;
#pragma fieldalign shared2 __bit70_def
typedef struct __bit70_def
{
    bool                flag;
    char                filler1;
    unsigned long       data_len;
    char                data[DEF_BIT70_SET_SIZE];
    char                filler2;
}bit70_def;
#pragma fieldalign shared2 __bit100_def
typedef struct __bit100_def
{
    bool                flag;
    char                filler1;
    unsigned long       data_len;
    char                data[DEF_BIT100_SET_SIZE];
    char                filler2;
}bit100_def;
#pragma fieldalign shared2 __bit120_def
typedef struct __bit120_def
{
    bool                flag;
    char                filler1;
    unsigned long       data_len;
    char                data[DEF_BIT120_SET_SIZE];
    char                filler2;
}bit120_def;
#pragma fieldalign shared2 __bit127_def
typedef struct __bit127_def
{
    bool                flag;
    char                filler1;
    unsigned long       data_len;
    char                data[DEF_BIT127_SET_SIZE];
    char                filler2;
}bit127_def;


/* MTI0800固定フォーマット */
#pragma fieldalign shared2 __mti_0800
typedef struct __mti_0800
{
    bit7_def    bit7;
    bit11_def   bit11;
    bit32_def   bit32;
    bit37_def   bit37;
    bit39_def   bit39;
    bit44_def   bit44;
    bit48_def   bit48;
    bit53_def   bit53;
    bit59_def   bit59;
    bit70_def   bit70;
    bit100_def  bit100;
    bit120_def  bit120;
    bit127_def  bit127;
}MTI_0800;

/* MTI0810固定フォーマット */
#pragma fieldalign shared2 __mti_0810
typedef struct __mti_0810
{
    bit7_def    bit7;
    bit11_def   bit11;
    bit32_def   bit32;
    bit37_def   bit37;
    bit39_def   bit39;
    bit44_def   bit44;
    bit48_def   bit48;
    bit53_def   bit53;
    bit59_def   bit59;
    bit70_def   bit70;
    bit100_def  bit100;
    bit120_def  bit120;
    bit127_def  bit127;
}MTI_0810;

/* MTI0820固定フォーマット */
#pragma fieldalign shared2 __mti_0820
typedef struct __mti_0820
{
    bit7_def    bit7;
    bit11_def   bit11;
    bit32_def   bit32;
    bit37_def   bit37;
    bit39_def   bit39;
    bit44_def   bit44;
    bit48_def   bit48;
    bit53_def   bit53;
    bit59_def   bit59;
    bit70_def   bit70;
    bit100_def  bit100;
    bit120_def  bit120;
    bit127_def  bit127;
}MTI_0820;

/* MTI0830固定フォーマット */
#pragma fieldalign shared2 __mti_0830
typedef struct __mti_0830
{
    bit7_def    bit7;
    bit11_def   bit11;
    bit32_def   bit32;
    bit37_def   bit37;
    bit39_def   bit39;
    bit44_def   bit44;
    bit48_def   bit48;
    bit53_def   bit53;
    bit59_def   bit59;
    bit70_def   bit70;
    bit100_def  bit100;
    bit120_def  bit120;
    bit127_def  bit127;
}MTI_0830;

/* 接続先固有情報ファイル・接続先固有情報 */
#pragma fieldalign shared2 __nws_unq_info_di
typedef struct __nws_unq_info_di
{
    char    acq_code[11];
    char    recv_code[11];
    char    future_use1[178];
} nws_unq_info_di_def;
#define nws_unq_info_di_def_Size 200

/* NW情報ファイル・接続先固有情報 */
typedef struct __dst_unq_info_di
{
    char    move_mode;
    char    future_use[99];
} dst_unq_info_discover;
#define dst_unq_info_di_def_Size 100

#endif

