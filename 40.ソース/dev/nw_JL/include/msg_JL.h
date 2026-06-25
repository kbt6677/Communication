/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････                                             */
/*        FUNCTION          ････ J-Link用メッセージヘッダー(共通ヘッダー)    */
/*        AUTHER            ････ HAS S.Kimura                                */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-01-29                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  N.miki     2025/01/29 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
#ifndef _msg_JL_H
#define _msg_JL_H

/* 制御電文内部フォーマット[J-Link] */
#pragma fieldalign shared2 __msg_jlink_def
typedef struct __msg_jlink_def
{
    char                    mti[4];             // MTI
    char                    ffd;               // 電文固定フォーマット
} msg_jlink_def;

/* MTI0800固定フォーマット */
#pragma fieldalign shared2 __mti_0800
typedef struct __mti_0800
{
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[10];
    }bit7;
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[6];
    }bit11;
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[11];
        char                filler2;
    }bit33;
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[8];
    }bit53;
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[3];
        char                filler2;
    }bit70;
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[8];
    }bit96;
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[11];
        char                filler2;
    }bit100;
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[96];
    }bit105;
}MTI_0800;
/* MTI0810固定フォーマット */
#pragma fieldalign shared2 __mti_0810
typedef struct __mti_0810
{
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[10];
    }bit7;
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[6];
    }bit11;
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[11];
        char                filler2;
    }bit33;
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[2];
    }bit39;
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[8];
    }bit53;
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[3];
        char                filler2;
    }bit70;
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[8];
    }bit96;
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[11];
        char                filler2;
    }bit100;
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[96];
    }bit105;
}MTI_0810;
/* MTI0620固定フォーマット */
#pragma fieldalign shared2 __mti_0620
typedef struct __mti_0620
{
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[10];
    }bit7;
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[6];
    }bit11;
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[255];
        char                filler2;
    }bit48;
}MTI_0620;
/* MTI0630固定フォーマット */
#pragma fieldalign shared2 __mti_0630
typedef struct __mti_0630
{
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[10];
    }bit7;
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[6];
    }bit11;
    struct
    {
        bool                flag;
        char                filler1;
        unsigned long       data_len;
        char                data[255];
        char                filler2;
    }bit48;
}MTI_0630;

/* 接続先固有情報ファイルフォーマット(J-Link) */
#pragma fieldalign shared2 __dst_unq_info_JL
typedef struct __dst_unq_info_JL
{
    char                forwarding_inst_id[11];    /* 送信元識別コード */
    char                receiving_inst_id[11];     /* 受信機関識別コード */
    char                filler[178];               /* 予備 */
} dst_unq_info_JL;

#endif
