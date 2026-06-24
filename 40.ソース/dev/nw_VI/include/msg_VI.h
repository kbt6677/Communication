/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････                                             */
/*        FUNCTION          ････ VisaNet用メッセージヘッダー(共通ヘッダー)   */
/*                                        制御電文固定フォーマット（追記）   */
/*        AUTHER            ････ HAS S.Kimura                                */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-01-29                                  */
/*        UPDATE            ････ 2025-04-17                                  */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  S.Kimura   2025/01/29 (J0680)新規作成                               */
/*  1.1  K.F        2025/04/17 制御電文固定フォーマット宣言追加              */
/*                                                                           */
/*****************************************************************************/
#ifndef _msg_VI_H
#define _msg_VI_H

/* 制御電文 MTI(ASCII) */
#define DEF_VI_MTI_0800_REQ                "0800"      /* 制御電文要求             */
#define DEF_VI_MTI_0810_RSP                "0810"      /* 制御電文応答             */

/* 制御電文 MTI(BCD) */
#define DEF_VI_MTI_BCD_0800                0x0800      /* 制御電文要求             */
#define DEF_VI_MTI_BCD_0810                0x0810      /* 制御電文応答             */

/* VMLH Message Format and Platform */
#define DEF_VI_VMLH_FORM_DT_00             0x00        /* business message         */
#define DEF_VI_VMLH_FORM_HB_20             0x20        /* session control message  */
#define DEF_VI_VMLH_LENG_ZERO              0x00        /* 電文長 0                 */

/* Message Header: Message Format and Platform */
#define DEF_VI_HD_FLGFRM_DT_01             0x01        /* メッセージヘッダー       */
#define DEF_VI_HD_FLGFRM_RJ_81             0x81        /* リジェクト電文           */

/* Message Header: Text Format */
#define DEF_VI_HD_TXTFRM_02                0x02        /* Text Format              */

/* (F70)Network Management Information Code (固定フォーマット) */
#define DEF_VI_F70_INFOCODE_071_SON        "0071"      /* サインオン               */
#define DEF_VI_F70_INFOCODE_072_SOF        "0072"      /* サインオフ               */
#define DEF_VI_F70_INFOCODE_078_SAFBEG     "0078"      /* SAF送信開始              */
#define DEF_VI_F70_INFOCODE_079_SAFEND     "0079"      /* SAF送信開始              */
#define DEF_VI_F70_INFOCODE_301_ECH        "0301"      /* エコーテスト             */

/* データサイズDEFINE */
#define MSG_HEADER_NOMALDATA_LENG   26        /* VMLH長 + 一般電文ヘッダ長    */
#define MSG_HEADER_REJDATA_LENG     30        /* VMLH長 + リジェクトヘッダ長  */
#define MSG_HEADER_MTI_LENG         28        /* VMLH長 + 一般電文ヘッダ長 + MTIデータ長 */

/* typedef定義                 */

/* N/W情報ファイル・接続先固有情報 */
#pragma fieldalign shared2 __nwi_unq_info_vi
typedef struct __nwi_unq_info_vi
{
    char    future_use[100];
} nwi_unq_info_vi_def;
#define nwi_unq_info_vi_def_Size 100

/* 接続先固有情報ファイル・接続先固有情報 */
#pragma fieldalign shared2 __nws_unq_info_vi
typedef struct __nws_unq_info_vi
{
    char    visa_station_id[6];
    char    future_use1[194];
} nws_unq_info_vi_def;
#define nws_unq_info_vi_def_Size 200

/* システム間インターフェース・電文ヘッダ VMLH */
#pragma fieldalign shared2 __MSG_VMLH_VISA
typedef struct __MSG_VMLH_VISA
{
    short   vmlh_msg_len       ;    /* Message Length                   */
    char    vmlh_rsv           ;    /* Reserved                         */
    char    vmlh_msg_fmt_plt   ;    /* Message Format and Platform      */
} MSG_VMLH_VISA_def;
#define MSG_VMLH_VISA_def_Size 4

/* システム間インターフェース・VISA・電文ヘッダ */
#pragma fieldalign shared2 __MSG_HEADER_VISA
typedef struct __MSG_HEADER_VISA
{
    char    mh_hdr_len         ;    /* Header Length                    */
    char    mh_hdr_flg_fmt     ;    /* Header Flag and Format           */
    char    mh_txt_fmt         ;    /* Text Format                      */
    char    mh_tot_len      [2];    /* Total Message Length             */
    char    mh_dst_id       [3];    /* Destination Station ID           */
    char    mh_src_id       [3];    /* Source Station ID                */
    char    mh_rnd_trip        ;    /* Round Trip Control Information   */
    char    mh_vip_flg      [2];    /* V.I.P. Flags                     */
    char    mh_msg_sts_flg  [3];    /* Message Status Flags             */
    char    mh_bat_num         ;    /* Batch Number (not used)          */
    char    mh_visa_use     [3];    /* Reserved for Visa Use            */
    char    mh_usr_info        ;    /* User Information                 */
} MSG_HEADER_VISA_def;
#define MSG_HEADER_VISA_def_Size 22

/* システム間インターフェースリジェクト電文メッセージヘッダ */
#pragma fieldalign shared2 __MSG_HEADER_REJECT_VISA
typedef struct __MSG_HEADER_REJECT_VISA
{
    char    mh_hdr_len         ;    /* Header Length                    */
    char    mh_hdr_flg_fmt     ;    /* Header Flag and Format           */
    char    mh_txt_fmt         ;    /* Text Format                      */
    char    mh_tot_len      [2];    /* Total Message Length             */
    char    mh_dst_id       [3];    /* Destination Station ID           */
    char    mh_src_id       [3];    /* Source Station ID                */
    char    mh_rnd_trip        ;    /* Round Trip Control Information   */
    char    mh_vip_flg      [2];    /* V.I.P. Flags                     */
    char    mh_msg_sts_flg  [3];    /* Message Status Flags             */
    char    mh_bat_num         ;    /* Batch Number (not used)          */
    char    mh_visa_use     [3];    /* Reserved for Visa Use            */
    char    mh_usr_info        ;    /* User Information                 */
    char    mh_bit_map      [2];    /* Bitmap                           */
    char    mh_bit_map_rej  [2];    /* Bitmap, Reject Data Group        */
} MSG_HEADER_REJECT_VISA_def;
#define MSG_HEADER_REJECT_VISA_def_Size 26

/* 制御電文種別                                    */
#pragma fieldalign shared2 __ctltext_type_def
typedef struct __ctltext_type_def
{
    char    kinou_kbn;              /* 電文機能区分                   */
    char    dir_kbn;                /* 要求応答区分                   */
    char    msg_kbn;                /* 制御電文区分                   */
    char    prc_kbn;                /* 内部処理区分                   */
} ctltext_type_def;

/* システム間インターフェース・VISANET・電文ヘッダ */
#pragma fieldalign shared2 __MSG_HEADER_VISANET_DEF
typedef struct __MSG_HEADER_VISANET_DEF
{
    /* 共通制御ヘッダ */
    char    ctrl_hdr_type   [2];    /* 共通制御・ヘッダータイプ       */
    char    ctrl_msg_len    [2];    /* 共通制御・全体電文長           */
    char    ctrl_src_id     [11];   /* 共通制御・差出センターID       */
    char    ctrl_dst_id     [11];   /* 共通制御・宛先センターID       */
    char    ctrl_merch_code [11];   /* 共通制御・加盟店契約会社コード */
    char    ctrl_snd_time   [7];    /* 共通制御・送信日時             */
    char    ctrl_mode_flg      ;    /* 共通制御・モードフラグ         */
    char    ctrl_filler     [2];    /* 共通制御・予備                 */

    /* 業務共通ヘッダ */
    char    bh_hdr_type     [2];    /* 業務共通・ヘッダータイプ       */
    char    bh_msg_type     [4];    /* 業務共通・電文種別コード       */
    char    bh_auth_val     [4];    /* 業務共通・電文認証値           */
    struct
    {
        char    bh_chk_digit_kc     [2];    /* 業務共通・チェックディジット・電文暗号化キー(KC) */
        char    bh_chk_digit_kmac   [2];    /* 業務共通・チェックディジット・電文認証キー(KMAC) */
    } bh_chk_digit;                         /* 業務共通・チェックディジット */
    char    bh_dst_type        ;    /* 業務共通・仕向区分             */
    char    bh_cut_date     [4];    /* 業務共通・カット対象日付       */
    char    bh_body_len     [2];    /* 業務共通・BODY部電文長         */
    char    bh_visanet_id   [2];    /* 業務共通・VISAネット取引識別   */
    char    bh_visanet_seq  [6];    /* 業務共通・VISAネット取取引通番 */
    char    bh_visanet_area [2];    /* 業務共通・VISAネット取使用域   */
    char    bh_filler       [2];    /* 業務共通・予備                 */
} MSG_HEADER_VISANET_def;

#define MSG_HEADER_VISANET_ctrl_Size 47
#define MSG_HEADER_VISANET_bh_Size 33
#define MSG_HEADER_VISANET_def_Size 80

/* 固定フォーマットデータ エレメント管理情報部 */
#pragma fieldalign shared2 __ffd_header_def
typedef struct __ffd_header_def
{
    bool      m_flg_exist;                      // エレメント情報有
    char      m_dmy1;                           // 予備
    size_t    m_fixvalue_length;                // データ長
} ffd_header_def;

/* 制御電文固定フォーマット[VISANET] 0800 */
#pragma fieldalign shared2 __fixedform_visanet_0800_def
typedef struct __fixedform_visanet_0800_def
{
                                                // F07. Tranzaction Data and Time
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[10];
    } b07;
                                                // F11. システムトレースオーディットナンバー
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b11;
                                                // F37. Retrieval Reference Number
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[12];
    } b37;
                                                // F63. V.I.P. Private-Use Fields
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[255];
        char                m_fiiller_1;
    } b63;
                                                // F70. Network Management Information Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[4];
    } b70;

} fixedform_visanet_0800_def;

/* 制御電文固定フォーマット[VISANET] 0810 */
#pragma fieldalign shared2 __fixedform_visanet_0810_def
typedef struct __fixedform_visanet_0810_def
{
                                                // F07. Tranzaction Data and Time
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[10];
    } b07;
                                                // F11. システムトレースオーディットナンバー
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[6];
    } b11;
                                                // F37. Retrieval Reference Number
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[12];
    } b37;
                                                // F39. Response Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[2];
    } b39;
                                                // F63. V.I.P. Private-Use Fields
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[255];
        char                m_fiiller_1;
    } b63;
                                                // F70. Network Management Information Code
    struct
    {
        ffd_header_def      ffd_header;
        char                ffd_data[4];
    } b70;

} fixedform_visanet_0810_def;

/* 制御電文内部フォーマット[VISANET] */
#pragma fieldalign shared2 __msg_visanet_def
typedef struct __msg_visanet_def
{
    MSG_HEADER_VISA_def  header;                // 共通制御ヘッダ&業務共通ヘッダ
    char                 mti[2];                // MTI
    char                 ffd;                   // 電文固定フォーマット
} msg_visanet_def;


typedef struct __file_info_gfphi                // 物理名情報ファイル情報
{                                               // ----------------------
    char           file_id[8];                  //  ファイルID
    char           file_name[48];               //  ファイル名
    short          file_no;                     //  ファイル番号
    unsigned long  io_timer;                    //  I/Oタイマー
} file_info_gfphi;
                                                // ----------------------
typedef struct __file_info_gccut                // カット対象日付管理ファイル情報
{                                               // ----------------------
    char           file_id[8];                  //  ファイルID
    char           file_name[48];               //  ファイル名
    short          file_no;                     //  ファイル番号
    unsigned long  io_timer;                    //  I/Oタイマー
} file_info_gccut;
                                                // -----------------------
typedef struct __network_info                   // ネットワーク特定情報
{                                               // -----------------------
    char           site_id;                     //   サイト識別
    char           nw_id;                       //   N/W識別
    char           grp_id[5];                   //   グループ識別
    char           if_id[5];                    //   インタフェース識別
    char           station_id[6];               //   ステーション識別
    char           connect_id[6];               //   コネクション識別
} network_info;

#endif

