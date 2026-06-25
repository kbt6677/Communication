#pragma once
/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                               *
 *                                                                             *
 *                         ＜共通(ビットマップ展開・組立て)＞                  *
 *                                                                             *
 *        VERSION                               :＜1.1.0＞                     *
 *                                                                             *
 *        CREATE DATE                           :＜作成日 2024-10-23＞         *
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
/*        WRITTEN-DATE      ････ 2024-10-23                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤  2024-10-23 新規作成                                      */
/*  1.1  ISYS 工藤  2025/06/09 CM-040(PCIPIN対応)                            */
/****************************************************************************/

/* ################### */
/* # INCLUDE         # */
/* ################### */
/* STANDARD HEADER */
#include <stddef.h> nolist
#ifndef __DUPLICATE_INCLUDE_GUARD_AQWSEDRFTGYH
#define __DUPLICATE_INCLUDE_GUARD_AQWSEDRFTGYH
#ifdef _TANDEM_SOURCE
#include <ZSYSC(filename_constant)> nolist
#include <ZSPIC(zspi_ddl_char8,zspi_ddl_uint)> nolist
#include <ZFILC(constants)> nolist
#else
#include <ZSYSC> nolist
#include <ZSPIC> nolist
#include <ZFILC> nolist
#endif
#endif
#include <stdbool.h> nolist
/* USER HEADER */
#include <GFPCGXB0.h> nolist

/****************************************************************************/
/*   定数定義                                                               */
/****************************************************************************/
// 制御電文エレメント情報ファイル読込I/Oタイマー
extern const long DEF_COM_BTM_GFELI_IO_TIMER;  // = -1L:無期限(10ms単位);

// GFPCGX20 エラーコード定義
typedef enum __GFPCGX20_error_code
{
    GFPCGX20_VALID                       = 0,   ///< 処理結果は正常
    GFPCGX20_ER_UNINITIALIZED            = 1,   ///< 初期化されていない。
    GFPCGX20_ER_UNDEFINED_MTI            = 2,   ///< MTIが定義されていない
    GFPCGX20_ER_RAW_BUFFER_UNDERFLOW     = 3,   ///< RAW BUFFER元データ長を超過した
    GFPCGX20_ER_INVALID_FORMAT           = 4,   ///< フォーマット不正
    GFPCGX20_ER_UNDEFINED_ELEMENT        = 5,   ///< 固定フォーマットの定義が存在しないデータエレメント
    GFPCGX20_ER_R2F_LENGTH_TRANS_ERR     = 6,   ///< データ長部のISO8583から固定フォーマットへの変換に失敗した
    GFPCGX20_ER_R2F_VALUE_TRANS_ERR      = 7,   ///< データ部のISO8583から固定フォーマットへの変換に失敗した
    GFPCGX20_ER_INVALID_ELEMENT_NUMBER   = 8,   ///< 不正なデータエレメント番号が指定された。
    GFPCGX20_ER_RAW_BUFFER_OVERFLOW      = 9,   ///< RAW BUFFER書込データ長を超過した
    GFPCGX20_ER_F2R_LENGTH_TRANS_ERR     = 10,  ///< データ長部の固定フォーマットからISO8583への変換に失敗した
    GFPCGX20_ER_F2R_VALUE_TRANS_ERR      = 11,  ///< データ部の固定フォーマットからISO8583への変換に失敗した
    GFPCGX20_ER_FILE_IO_ERROR            = 12,  ///< GFELI読込時のI/Oエラー
    GFPCGX20_ER_UNKNOWN_CODE_L           = 13,  ///< データ長部に対応する文字コード変換定義が存在しない.
    GFPCGX20_ER_UNKNOWN_CODE_V           = 14,  ///< データ部に対応する文字コード変換定義が存在しない.
    GFPCGX20_ER_INVALID_GFELI_REC        = 15,  ///< GFELIのレコード長不正.
    GFPCGX20_ER_INVALID_GFELI_COL        = 16,  ///< GFELIのカラムが正しくない
    GFPCGX20_ER_FIX_FMT_IDX_NUM_EXCEED   = 17,  ///< 固定フォーマットインデックスのレコード数超過
    GFPCGX20_ER_FIX_FMT_NUM_EXCEED       = 18,  ///< 固定フォーマットのレコード数超過
    GFPCGX20_ER_FIX_FMT_OFFSET_INC       = 19,  ///< 固定フォーマット定義のオフセット範囲が重複している。
    GFPCGX20_ER_VAR_ELEMENT_LENGTH_ERROR = 20,  ///< 固定フォーマットのデータ長が変換先の可変長データエレメントの
                                                ///< 長さを越えている。
    GFPCGX20_ER_R2F_4TH_BITMAP_EXIST     = 21,  ///< ISO8583メッセージに4th bitmapがある。
    GFPCGX20_ER_FIX_BUFFER_OVERFLOW      = 22,  ///< 固定フォーマットのデータエリアサイズを超過した
    GFPCGX20_ER_INVALID_PARAMETER        = 23,  ///< 不正なパラメータ指定
} GFPCGX20_error_code_t;
/****************************************************************************/
/*   ユーザー定義型                                                         */
/****************************************************************************/
// モジュールID
typedef char module_id_t[8 + 1];

// 論理ファイル名
typedef char filename_l_t[8 + 1];

// 物理ファイル名
typedef char filename_p_t[ZSYS_VAL_LEN_FILENAME + 1];

// サブプログラムステータスコード
typedef char sub_prog_sts_t[2 + 1];  // DEF_COM_IOM_XXX_ERR guardian file system error.

// MTI char mti_t[4] null終端無し
typedef char mti_t[4];

// Data element number. char denum_t[4] null終端無し
typedef char denum_t[3];

// エラーの詳細情報テキスト
typedef char error_info_desc_t[249 + 1];

// エラーダンプ
typedef char error_info_dump_t[250];

/****************************************************************************/
/*   構造体定義                                                             */
/****************************************************************************/
// エラー情報
#pragma fieldalign shared2 __error_info_t
typedef struct __error_info_t
{
    char                  m_MTI[sizeof(mti_t) + 1];           ///< 処理中のMTI
    long                  m_de_number;                        ///< 処理中にエラーになったデータエレメントの番号
    long                  m_currentOffset;                    ///< 処理中のメッセージオフセット
    GFPCGX20_error_code_t m_errCd;                            ///< エラー理由コード
    error_info_desc_t     m_description;                      ///< エラーの詳細説明
    error_info_dump_t     m_dump;                             ///< 処理中のデータダンプ
    char                  m_ipc_version;                      ///< 電文フォーマットバージョン番号
/**
 * @brief Guardian error 情報
 * @note @@struct __guardian_info_def
 */
#pragma fieldalign shared2 __guardian_info_def
    struct __guardian_info_def
    {
        short          guardian_errcode;                      ///< Guardian error code.
        char           err_proc[30 + 1];                      ///< エラーが発生したプロシージャ名
        char           file_name[ZSYS_VAL_LEN_FILENAME + 1];  ///< エラー対象の物理ファイル名
        sub_prog_sts_t sub_prog_sts;                          ///< サブプログラムステータス
    } m_guardian_error_Info;
} error_info_t;
typedef struct __guardian_info_def guardian_info_def;

#ifdef __LIBRARY_COMPILE
typedef struct __ffd_index_def ffd_index_def;
typedef struct __ffd_def       ffd_def;
#else
// ffd_index_defとffd_defの定義が変更された場合はSIZE_OF_FFD_INDEX及び、SIZE_OF_FFDも修正すること。
#define SIZE_OF_FFD_INDEX 18
#define SIZE_OF_FFD       46
#pragma fieldalign shared2 __ffd_index_def_opaque
typedef struct __ffd_index_def_opaque
{
    char opaque[SIZE_OF_FFD_INDEX];
} ffd_index_def;
#pragma fieldalign shared2 __ffd_def_opaque
typedef struct __ffd_def_opaque
{
    char opaque[SIZE_OF_FFD];
} ffd_def;
#endif
//  ISO8583コンテキスト ISO8583メッセージ組立・展開定義情報を保持する。
#pragma fieldalign shared2 __ISO8583context_t
typedef struct __ISO8583context_t
{
    ffd_index_def *m_ffd_index_tbl;      ///< 固定フォーマット定義インデックス配列
                                         ///< 固定フォーマット定義インデックスレコードを格納した配列
    ffd_def       *m_ffd_rec_tbl;        ///< 固定フォーマット定義配列
                                         ///<                固定フォーマットのレコードを格納した配列
    size_t         m_ffd_index_tbl_cnt;  ///< 固定フォーマット定義インデックス件数
                                         ///< 固定フォーマット定義インデックスの実際に読込んだ件数を格納する。
    bool           m_initialized;        ///< 初期化済FLG
                                         ///<                初期化が完了している状態で真。未初期化のコンテキストで
                                         ///<                処理を呼出した場合のエラーチェックに使う。
    char           m_module_id[8 + 1];   ///< モジュールID
                                         //                   モジュールIDを格納する。
    char           m_ipc_version;        ///< 電文フォーマットバージョン番号
    error_info_t   m_error_info;         ///< エラー情報
                                         //                   エラー情報(error_info_t)を参照
} ISO8583context_t;

// bitmap ビットマップ取り扱い用の共用帯と付加情報
#pragma fieldalign shared2 __bitmap_t
#pragma fieldalign shared2 __bitmap_data_t
#pragma fieldalign shared2 __bitmap_data_c
#pragma fieldalign shared2 __bitmap_data_ll
typedef struct __bitmap_t
{
    union __bitmap_data_t
    {
        struct __bitmap_data_c
        {
            char c_24[24];
        } c;               ///< 24byte
        struct __bitmap_data_ll
        {
            unsigned long long l_1;
            unsigned long long l_2;
            unsigned long long l_3;
        } l;               ///< 8byte(uint64)×3
    } m_bitmap_data;       ///< bitmap 1-3
    bool   m_bitmap2;      ///< 2nd bitmap有効
    bool   m_bitmap3;      ///< 3rd bitmap有効
    size_t m_current_bit;  ///< 現在のビット位置を保持
} bitmap_t;

// ISO8583メッセージオブジェクト ISO8583メッセージを組立・展開するオブジェクト。
#pragma fieldalign shared2 __ISO8583object_t
typedef struct __ISO8583object_t
{
    ffd_index_def *m_ffd_index_rec;         ///< 固定フォーマット定義インデックス 当該MTIの固定定義フォーマットインデックスへのポインタ
    ffd_def       *m_ffd_rec;               ///< 固定フォーマット定義配列            当該MTIの固定定義フォーマット配列へのポインタ
    char          *m_fixFormatBuffer;       ///< 固定フォーマットデータバッファ      固定フォーマットに展開したデータを格納する
    char          *m_iso8583RawDataBuffer;  ///< ISO8583RawDataバッファ              ISO8583RawDataを格納する
    size_t         m_iso8583BufferLength;   ///< ISO8584RawDataバッファ長            ISO8583RawDataを格納するバッファ長
    size_t         m_iso8583RawDataLength;  ///< ISO8583RawData長 ISO8583RawDataに格納されている現在のRawData長  <
                                            ///< 使用していない。削除する
    char          *m_ptr;                   ///< ISO8583RawData処理位置              ISO8583RawDataの現在処理位置
    bitmap_t       m_bitmap;                ///< bitmap 1-3
    char           m_ipc_version;           ///< 電文フォーマットバージョン番号
    error_info_t   m_error_info;            ///< エラー情報                          エラー情報(error_info_t)を参照
} ISO8583object_t;

// 固定フォーマットヘッダー 固定フォーマットデータの各エレメントの管理情報部
#pragma fieldalign shared2 __ffd_header_def
typedef struct __ffd_header_def
{
    bool   m_flg_exist;        ///< データエレメント有無 固定フィールドの当該データエレメントのデータの有無
    char   m_filler_1;
    size_t m_fixvalue_length;  ///< データ長 データエレメントが可変長である場合、実際に格納されているデータのバイト数
} ffd_header_def;

/****************************************************************************/
/*   関数定義                                                               */
/****************************************************************************/
bool com_btm_create_ISO8583_object(ISO8583context_t *iso8583_ctx, ISO8583object_t *iso8583obj, const mti_t mti);
size_t com_btm_get_fixformat_length(ISO8583object_t *obj);
bool com_btm_render_fixformat(ISO8583object_t *obj, char *fixformat, const char *iso8583_rawdata,
                              const size_t iso8583_rawdata_len);
bool com_btm_initial_ISO8583(ISO8583context_t *ctx, module_id_t mod_id, filename_l_t gfeli_name_l,
                             filename_p_t gfeli_name_p, char nw_id, ffd_index_def *ffd_index_buffer,
                             size_t ffd_index_count, ffd_def *ffd_buffer, size_t ffd_count);
bool com_btm_render_ISO8583_message(ISO8583object_t *obj, char *iso8583_rawdata_buffer,
                                    size_t iso8583_rawdata_buffer_len, const char *fixformat);
size_t com_btm_get_ISO8583_message_length(ISO8583object_t *obj);
bool com_btm_set_ipc_version(ISO8583context_t *ctx, char ipc_version);
size_t sizeof_ffd_opaque();
size_t sizeof_ffd_index_opaque();
