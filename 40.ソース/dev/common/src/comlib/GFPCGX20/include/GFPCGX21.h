#pragma once
#ifdef __LIBRARY_COMPILE
/******************************************************************************
*                                                                             *
*                               ＜GFP通信制御＞                               *
*                                                                             *
*                         ＜共通(ビットマップ展開・組立て)＞                  *
*                                                                             *
*        VERSION                               :＜1.1.0＞                     *
*                                                                             *
*        CREATE DATE                           :＜作成日 2024-10-24＞         *
*        CODED                                 :＜ISYS＞                      *
*                                                                             *
*        MODIFY DATE                           :＜修正日 2025/06/09＞         *
*        CODED                                 :＜修正者＞ ISYS Kudo          *
*                                              :＜修正概要＞CM-040(PCIPIN対応 *
******************************************************************************/
/*****************************************************************************/
/*****                    <<     header PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP制御                                     */
/*        PROGRAM-ID        ････ GFPCGX20                                    */
/*        FUNCTION          ････ 共通(ビットマップ展開・組立て)              */
/*                                                                           */
/*        AUTHOR            ････ ISYS Kudo                                   */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2024-10-24                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤  2024-10-24 新規作成                                      */
/*  1.1  ISYS 工藤  2025/06/09 CM-040(PCIPIN対応)                            */
/****************************************************************************/

/* ################### */
/* # INCLUDE         # */
/* ################### */
/* STANDARD HEADER */
#include <stddef.h> nolist
/* USER HEADER */
#include <GFPCGX20.h> nolist

/****************************************************************************/
/*   マクロ定義                                                             */
/****************************************************************************/
/****************************************************************************/
/*  MACRO           : CLR_ERR_INFO                                          */
/*                                                                          */
/*  PARAMETER       : obj                                                   */
/*                      ISO8583context_t または ISO8583object_t のポインタ  */
/*                                                                          */
/*  DESCRIPTION     : エラー情報クリアマクロ                                */
/*                    ISO8583context_t または ISO8583object_t の            */
/*                    エラー情報を初期化する                                */
/****************************************************************************/
#define CLR_ERR_INFO(obj)                     memset(&(obj)->m_error_info, 0, sizeof(error_info_t))

/****************************************************************************/
/*  MACRO           : SET_ERR_INFO_MTI                                      */
/*                                                                          */
/*  PARAMETER       : obj                                                   */
/*                      ISO8583context_t または ISO8583object_t のポインタ */
/*                  : mti                                                   */
/*                      MTI                                                 */
/*                                                                          */
/*  DESCRIPTION     : エラー情報設定マクロ                                  */
/*                    エラー情報にMTIを設定する                             */
/****************************************************************************/
#define SET_ERR_INFO_MTI(obj, mti)            (obj)->m_error_info.m_ipc_version = (obj)->m_ipc_version; \
                                              strncpy((obj)->m_error_info.m_MTI,(mti),sizeof(mti_t))

/****************************************************************************/
/*  MACRO           : SET_ERR_INFO_FFDKEY                                   */
/*                                                                          */
/*  PARAMETER       : obj                                                   */
/*                      ISO8583context_t または ISO8583object_t のポインタ */
/*                  : ffd_key                                               */
/*                      固定フォーマット定義キー                            */
/*                                                                          */
/*  DESCRIPTION     : エラー情報設定マクロ                                  */
/*                    エラー情報にMTIと電文フォーマットバージョンを設定する */
/****************************************************************************/
#define SET_ERR_INFO_FFDKEY(obj, ffd_key)     (obj)->m_error_info.m_ipc_version = (ffd_key).m_ipc_version; \
                                              strncpy((obj)->m_error_info.m_MTI,(ffd_key.m_mti_id),sizeof(mti_t))

/****************************************************************************/
/*  MACRO           : SET_ERR_INFO_DEN                                      */
/*                                                                          */
/*  PARAMETER       : obj                                                   */
/*                      ISO8583context_t または ISO8583object_t のポインタ */
/*                  : den                                                   */
/*                      データエレメント番号（int）                         */
/*                                                                          */
/*  DESCRIPTION     : エラー情報設定マクロ                                  */
/*                    データエレメント番号をエラー情報に設定する           */
/****************************************************************************/
#define SET_ERR_INFO_DEN(obj,den)             (obj)->m_error_info.m_de_number = (long)(den);

/****************************************************************************/
/*  MACRO           : SET_ERR_INFO_OFS                                      */
/*                                                                          */
/*  PARAMETER       : obj                                                   */
/*                      ISO8583context_t または ISO8583object_t のポインタ  */
/*                  : ofs                                                   */
/*                      編集中の現在位置をバッファ先頭からのオフセットで示す*/
/*                                                                          */
/*  DESCRIPTION     : エラー情報設定マクロ                                  */
/*                    処理中のオフセットをエラー情報に設定する              */
/****************************************************************************/
#define SET_ERR_INFO_OFS(obj,ofs)             (obj)->m_error_info.m_currentOffset = (ofs);

/****************************************************************************/
/*  MACRO           : SET_ERR_INFO_ERCD                                     */
/*                                                                          */
/*  PARAMETER       : obj                                                   */
/*                      ISO8583context_t または ISO8583object_t のポインタ  */
/*                  : error_code                                            */
/*                      エラーコード                                        */
/*                                                                          */
/*  DESCRIPTION     : エラー情報設定マクロ                                  */
/*                    エラー種別コードをエラー情報に設定する                */
/****************************************************************************/
#define SET_ERR_INFO_ERCD(obj,error_code)     (obj)->m_error_info.m_errCd = (error_code);

/****************************************************************************/
/*  MACRO           : SET_ERR_INFO_DESC                                     */
/*                                                                          */
/*  PARAMETER       : obj                                                   */
/*                      ISO8583context_t または ISO8583object_t のポインタ */
/*                  : description                                           */
/*                      エラーの説明文                                      */
/*                                                                          */
/*  DESCRIPTION     : エラー情報設定マクロ                                  */
/*                    エラーの説明文をエラー情報に設定する                 */
/****************************************************************************/
#define SET_ERR_INFO_DESC(obj,description)    strncpy((obj)->m_error_info.m_description, (description), sizeof(error_info_desc_t));

/****************************************************************************/
/*  MACRO           : SET_ERR_INFO_DMP                                      */
/*                                                                          */
/*  PARAMETER       : obj                                                   */
/*                      ISO8583context_t または ISO8583object_t のポインタ */
/*                  : dump                                                  */
/*                      エラー発生箇所へのポインタ                          */
/*                  : length                                                */
/*                      エラーデータ長                                      */
/*                                                                          */
/*  DESCRIPTION     : エラー情報設定マクロ                                  */
/*                    エラー発生部分のバイナリダンプをエラー情報に設定する */
/****************************************************************************/
#define SET_ERR_INFO_DMP(obj,dump,length)     memmove((obj)->m_error_info.m_dump, (dump), sizeof(error_info_dump_t) < (length) ? sizeof(error_info_dump_t) : (length) );

// transcoderエラーコード
typedef enum __transcoder_error_t
{
    E_TRANSCODE_VALID,                            ///< 文字コード変換正常
    E_TRANSCODE_BUFFER_ERROR,                     ///< RAWバッファエラー
    E_TRANSCODE_FIX_BUFFER_ERROR,                 ///< FIX FORMATバッファオーバー
    E_TRANSCODE_ERROR                             ///< 文字コード変換不正
} transcoder_error_t;

/****************************************************************************/
/*  FUNCTION        : fn_transcoder_t                                       */
/*  CALLING SEQ.    : transcoder_error_t (*fn_transcoder_t)                 */
/*                           (ISO8583object_t *obj,                         */
/*                            char *dst,                                    */
/*                            size_t dst_len,                               */
/*                            const char *src,                              */
/*                            size_t src_len,                               */
/*                            size_t *write_len)                            */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      ISO8583object_t 構造体ポインタ                      */
/*                  : dst [out]                                             */
/*                      出力バッファ                                        */
/*                  : dst_len [in]                                          */
/*                      出力対象バッファ長                                  */
/*                  : src [in]                                              */
/*                      変換元データ                                        */
/*                  : src_len [in]                                          */
/*                      変換元データ長（文字数）                            */
/*                  : write_len [out]                                       */
/*                      変換後データ長（バイト）                            */
/*                                                                          */
/*  RETURN CODE     : transcoder_error_t                                    */
/*                      変換処理のエラーコード                              */
/*                                                                          */
/*  DESCRIPTION     : データ部組立/展開メソッド                             */
/****************************************************************************/
typedef transcoder_error_t (*fn_transcoder_t)(ISO8583object_t * obj, char * dst,size_t dst_len,const char * src,size_t src_len,size_t * write_len);

/****************************************************************************/
/*  FUNCTION        : fn_len_encoder_t                                      */
/*  CALLING SEQ.    : transcoder_error_t (*fn_len_encoder_t)               */
/*                           (ISO8583object_t *obj,                         */
/*                            char *raw_length,                             */
/*                            size_t raw_length_size,                       */
/*                            size_t val)                                   */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      ISO8583object_t 構造体ポインタ                      */
/*                  : raw_length [out]                                      */
/*                      ISO8583メッセージのデータエレメント長部の先頭       */
/*                  : raw_length_size [in]                                  */
/*                      データエレメント長部のバイト数                      */
/*                  : val [in]                                              */
/*                      データエレメントの長さ                              */
/*                                                                          */
/*  RETURN CODE     : transcoder_error_t                                    */
/*                      エンコード処理結果のエラーコード                    */
/*                                                                          */
/*  DESCRIPTION     : データ長部組立メソッド                                */
/****************************************************************************/
typedef transcoder_error_t (*fn_len_encoder_t)(ISO8583object_t * obj, char * raw_length,size_t raw_length_size,size_t val);

/****************************************************************************/
/*  FUNCTION        : fn_len_decoder_t                                      */
/*  CALLING SEQ.    : transcoder_error_t (*fn_len_decoder_t)               */
/*                           (ISO8583object_t *obj,                         */
/*                            char *raw_length,                             */
/*                            size_t raw_length_size,                       */
/*                            size_t *val)                                  */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      ISO8583object_t 構造体ポインタ                      */
/*                  : raw_length [in]                                       */
/*                      ISO8583メッセージデータエレメント長部の先頭         */
/*                  : raw_length_size [in]                                  */
/*                      データエレメント長部のバイト数                      */
/*                  : val [out]                                             */
/*                      デコードされたデータエレメント長（文字数）         */
/*                                                                          */
/*  RETURN CODE     : transcoder_error_t                                    */
/*                      デコード処理結果のエラーコード                      */
/*                                                                          */
/*  DESCRIPTION     : データ長部展開メソッド                                */
/****************************************************************************/
typedef transcoder_error_t (*fn_len_decoder_t)(ISO8583object_t * obj, char * raw_length,size_t raw_length_size,size_t *val);

//データエレメント長形式
typedef enum __raw_length_type
{
    FMT_LEN_FIX     = 0,                          ///< 固定長(データ長フィールドなし)
    FMT_LEN_LVAR    = 1,                          ///< "1"：可変長(L)
    FMT_LEN_LLVAR   = 2,                          ///< "2"：可変長(LL)
    FMT_LEN_LLLVAR  = 3,                          ///< "3"：可変長(LLL)
    FMT_LEN_LLLLVAR = 4                           ///< "4"：可変長(LLLL)
} raw_length_type;

//ビットマップインジケータビット定義
typedef enum __bitmap_marks
{
    E_2ND_BITMAP_IND = 1,                         ///< 2nd bitmapの存在有無を示すビット番号
    E_MIN_BITMAP_NUM = 2,                         ///< データエレメント番号の最小値
    E_3RD_BITMAP_IND = 65,                        ///< 3rd bitmapの存在有無を示すビット番号
    E_MIN_2ND_BITMAP_RANGE = 66,                  ///< 2nd bitmapの最小データエレメント番号
    E_MAX_2ND_BITMAP_RANGE = 128,                 ///< 2nd bitmapの最大データエレメント番号
    E_4TH_BITMAP_IND = 129,                       ///< 4th bitmapの存在有無を示すビット番号
    E_MIN_3RD_BITMAP_RANGE = 130,                 ///< 3rd bitmapの最小データエレメント番号
    E_MAX_BITMAP_NUM = 192                        ///< データエレメント番号の最大値
}bitmap_marks;

//ビットマップバイトインデックス
typedef enum __bitmap_byte_index
{
    E_2ND_BITMAP_BYTE_INDEX = 0,                  ///< 2nd bitmapの存在有無を示すビットを含むバイトインデックス
    E_3RD_BITMAP_BYTE_INDEX = 8,                  ///< 3rd bitmapの存在有無を示すビットを含むバイトインデックス
    E_4TH_BITMAP_BYTE_INDEX = 16,                 ///< 4th bitmapの存在有無を示すビットを含むバイトインデックス
} bitmap_byte_index;

//ビットマップデータ長
typedef enum __bitmap_byte_length
{
    E_1ST_BITMAP_BYTE_LENGTH = 8,                 ///< 1st bitmapのみのビットマップデータ長
    E_2ND_BITMAP_BYTE_LENGTH = 16,                ///< 2nd bitmapを含む場合のビットマップデータ長
    E_3RD_BITMAP_BYTE_LENGTH = 24,                ///< 3rd bitmapを含む場合のビットマップデータ長
} bitmap_byte_length;

//固定フォーマット定義キー
#pragma fieldalign shared2 __ffd_key_t
struct __ffd_key_t
{
    char            m_ipc_version;                ///< 電文フォーマットバージョン(P-Key)
    mti_t           m_mti_id;                     ///< MTI (P-Key)                  JIS8形式のMTI
    char            m_filler;
};
#ifndef __ffd_key_t__
#define __ffd_key_t__
typedef struct __ffd_key_t ffd_key_t;
#endif


#pragma fieldalign shared2 __ffd_index_def
//固定フォーマット定義インデックス
// ffd_index_defとffd_defの定義が変更された場合はSIZE_OF_FFD_INDEX及び、SIZE_OF_FFDも修正すること。
struct __ffd_index_def
{
    ffd_key_t       ffd_key;                      ///< 電文フォーマットバージョン(P-Key) + MTI (P-Key)の復号キー
    size_t          m_ffd_pos;                    ///< 固定フォーマット定義開始位置 当該固定フォーマットの開始位置
    size_t          m_ffd_count;                  ///< 固定フォーマット定義件数     当該固定フォーマットの件数
    size_t          m_fix_format_length;          ///< 固定フォーマット長           当該固定フォーマットデータ全長
} ;

#pragma fieldalign shared2 __ffd_def
#pragma fieldalign shared2 __pri_key_def
#pragma fieldalign shared2 __raw_msg_fmt_info_def
#pragma fieldalign shared2 __fix_fmt_info_def
// 固定フォーマット定義  固定フォーマット定義データのレコード定義
// ffd_index_defとffd_defの定義が変更された場合はSIZE_OF_FFD_INDEX及び、SIZE_OF_FFDも修正すること。
struct __ffd_def
{
    struct __pri_key_def
    {
        ffd_key_t           ffd_key;              ///< 電文フォーマットバージョン(P-Key) + MTI (P-Key)の復号キー
        long                m_de_num;             ///< データエレメント番号               データエレメント(ビット番号)
    } pri_key;                                    ///< 主キー定義
    struct __raw_msg_fmt_info_def
    {
        raw_length_type     m_raw_length_type;    /**< データ長形式
                                                        電文上のデータ長フィールドの形式
                                                        "0"：固定長(データ長フィールドなし)
                                                        "1"：可変長(L) "2"：可変長(LL) "3"：可変長(LLL)
                                                        "4"：可変長(LLLL) */
        size_t              m_raw_length_size;    ///< データ長サイズ                     データ長部を格納する領域のバイト数
        fn_len_encoder_t    m_raw_length_encoder; ///< データ長組立用コード変換メソッド
        fn_transcoder_t     m_raw_data_encoder;   ///< データ部組立用コード変換メソッド
        size_t              m_raw_data_max_len;   ///< 最大データ長 (rawデータの最大長または、データ長形式固定長時のデータ長)
    }raw_msg_fmt_info;                            ///< 電文フォーマット情報(raw data定義)
    struct __fix_fmt_info_def
    {
        size_t              m_tbl_start_offset;   ///< テーブル開始オフセット             固定フォーマットの当該データエレメントの開始位置
        size_t              m_data_area_size;     ///< データエリアサイズ                 固定フォーマットのデータ領域の最大(または固定)データ長
        fn_len_decoder_t    m_fix_length_decoder; ///< データ長展開用コード変換メソッド
        fn_transcoder_t     m_fix_data_decoder;   ///< データ部展開用コード変換メソッド
    }fix_fmt_info;                                ///< 固定フォーマット情報
} ;
//固定フォーマット定義::主キー定義
typedef struct      __pri_key_def                 pri_key_def;

//固定フォーマット定義::電文フォーマット情報
typedef struct      __raw_msg_fmt_info_def        raw_msg_fmt_info_def;

//固定フォーマット定義::固定フォーマット情報
typedef struct      __fix_fmt_info_def            fix_fmt_info_def;

// データ部変換(展開/組立)メソッドペア
// decoderでraw dataからfix formatに、encoderでfix formatからraw dataに変換する
#pragma fieldalign shared2 __transcoder_set_def
typedef struct __transcoder_set_def
{
    fn_transcoder_t decoder;                     ///< データエリアをISO8583メッセージから固定フォーマットに展開する。
    fn_transcoder_t encoder;                     ///< データエリアを固定フォーマットからISO8583メッセージに組立てる。
} transcoder_set_def;

// データ長部変換(展開/組立)メソッドペア
// decoderでraw dataからfix formatに、encoderでfix formatからraw dataに変換する
#pragma fieldalign shared2 __transcoder_set_def
typedef struct __length_transcoder_set_def
{
    fn_len_decoder_t decoder;                 ///< ISO8583メッセージのraw dataのデータ長部を固定フォーマットに展開する。
    fn_len_encoder_t encoder;                 ///< 固定フォーマットのデータ長部をISO8583メッセージのraw dataに組立てる
} length_transcoder_set_def;

/****************************************************************************/
/*   内部関数定義                                                           */
/****************************************************************************/
ffd_index_def* search_index(ISO8583context_t *iso8583_ctx, const mti_t mti_key);
int comp_mti(const void * lhs, const void * rhs);
bool get_bitmap(ISO8583object_t *obj);
const ffd_def * find_ffd(const ffd_def * start, size_t *count ,const int de_number);
bool get_next_bit(bitmap_t * bitmap, int * de_number, int *bit_value);
size_t get_bitmap_length(bitmap_t *bitmap);
bool rawdata_buffer_check(ISO8583object_t * obj, size_t data_length);
bool write_fix_element(ISO8583object_t *obj, const ffd_def *ffd);
bool set_bitmap(ISO8583object_t *obj,long de_number,long val);
bool rawdata_buffer_overflow(ISO8583object_t * obj, size_t write_length);
bool write_raw_data_element(ISO8583object_t *obj,ffd_def *ffd,ffd_header_def *ffd_template);
bool query_value_transcoder(const char nw_id, const char data_area_attribute, const char code_change_need , transcoder_set_def * transcoder_set);
bool query_length_transcoder(const char nw_id, const char data_area_attribute,length_transcoder_set_def *transcoder_set);
void * get_ffd_data_area_ptr(ffd_header_def *ffd_template);
#endif
