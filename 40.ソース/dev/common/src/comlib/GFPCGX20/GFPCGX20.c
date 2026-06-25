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
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP制御                                     */
/*        PROGRAM-ID        ････ GFPCGX20                                    */
/*        FUNCTION          ････ 共通(ビットマップ展開・組立て)              */
/*                                                                           */
/*        AUTHER            ････ ISYS Kudo                                   */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2024-10-24                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤  2025/06/09 新規作成                                      */
/*  1.1  ISYS 工藤  2025/06/09 CM-040(PCIPIN対応)                            */
/****************************************************************************/

/* ################### */
/* # INCLUDE         # */
/* ################### */
/* STANDARD HEADER */
#include <limits.h> nolist
#include <stdbool.h> nolist
#include <stdlib.h> nolist
#include <string.h> nolist
/* USER HEADER */
#define __LIBRARY_COMPILE
#include <GFPCGX20.h> nolist  // public header
#include <GFPCGX21.h> nolist  // impliment header.
#include "vproc.h"    nolist  // vproc

/****************************************************************************/
/*   内部関数定義                                                           */
/****************************************************************************/
void set_rawdata_buffer_underflow_error(ISO8583object_t * obj, const ffd_def * ffd);
void set_fix_buffer_overflow_error(ISO8583object_t * obj, const ffd_def * ffd);
void set_length_decode_error(ISO8583object_t * obj, const ffd_def * ffd);
void set_value_decode_error(ISO8583object_t * obj, const ffd_def * ffd);

/****************************************************************************/
/*  FUNCTION        : com_btm_create_ISO8583_object                         */
/*  CALLING SEQ.    : bool com_btm_create_ISO8583_object                    */
/*                           (ISO8583context_t *iso8583_ctx,                */
/*                            ISO8583object_t *iso8583obj ,                 */
/*                            const mti_t mti)                              */
/*                                                                          */
/*  ARGUMENT        : iso8583_ctx [in,out] 初期化済のISO8583context_tを渡す。*/
/*                    エラー時はこの変数にエラー情報を返す。                */
/*                  : iso8583obj  [in,out] ISO8583object_tをNULL埋めして渡すこと。*/
/*                  : mti [in] MTIをASCII形式で渡す                         */
/*  RETURN CODE     : true: 処理結果正常, false: 処理結果エラー             */
/*  DESCRIPTION     : ISO8583メッセージオブジェクトを初期化する。           */
/*  MTIを指定して当該メッセージフォーマット処理用のオブジェクトを作成する。 */
/*  エラー時はISO8583context_tにエラー情報が返る。                          */
/****************************************************************************/
bool com_btm_create_ISO8583_object(ISO8583context_t *iso8583_ctx,ISO8583object_t *iso8583obj ,const mti_t mti)
{
    ffd_index_def   * ffd_index;
    if(!(iso8583_ctx->m_initialized)){
        //ISO8583context_tの未初期化エラー
        CLR_ERR_INFO(iso8583_ctx);
        SET_ERR_INFO_ERCD(iso8583_ctx, GFPCGX20_ER_UNINITIALIZED);
        SET_ERR_INFO_DESC(iso8583_ctx,"ISO8583context_t is not initialized.");
        return false;
    }
    /// MTIに対応する固定フォーマットの検索
    ffd_index = search_index(iso8583_ctx, mti);
    if(!ffd_index){
        //対応する固定フォーマット無し
        CLR_ERR_INFO(iso8583_ctx);
        SET_ERR_INFO_MTI(iso8583_ctx, mti);
        SET_ERR_INFO_ERCD(iso8583_ctx, GFPCGX20_ER_UNDEFINED_MTI);
        SET_ERR_INFO_DESC(iso8583_ctx,"fixformat definition not found.");
        return false;
    }
    /// ISO8583object_tの初期化
    memset(iso8583obj, 0, sizeof(ISO8583object_t));
    iso8583obj->m_ffd_index_rec = ffd_index;
    iso8583obj->m_ffd_rec = iso8583_ctx->m_ffd_rec_tbl + ffd_index->m_ffd_pos;
    iso8583obj->m_ipc_version = iso8583_ctx->m_ipc_version;
    return true;
}
/****************************************************************************/
/*  FUNCTION        : com_btm_get_fixed_format_length                       */
/*  CALLING SEQ.    : size_t com_btm_get_fixed_format_length                */
/*                           (ISO8583object_t *obj)                         */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      初期化済のISO8583object_t                           */
/*                                                                          */
/*  RETURN CODE     : 固定フォーマットを格納するのに必要なメモリ長          */
/*                                                                          */
/*  DESCRIPTION     : 固定フォーマット長取得                                */
/*                    指定したMTIの固定フォーマット全体を格納するのに       */
/*                    必要なメモリサイズを返す。                            */
/****************************************************************************/
size_t com_btm_get_fixformat_length(ISO8583object_t * obj)
{
    return obj->m_ffd_index_rec->m_fix_format_length;
}

/****************************************************************************/
/*  FUNCTION        : com_btm_create_fixed_format                           */
/*  CALLING SEQ.    : bool com_btm_create_fixed_format                      */
/*                           (ISO8583object_t *obj,                         */
/*                            char    *fixformat,                           */
/*                            const char    *iso8583_rawdata,               */
/*                            const size_t iso8583_rawdata_len)             */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      初期化済のISO8583object_t                           */
/*                  : fixformat [out]                                       */
/*                      固定フォーマットを格納するバッファ                 */
/*                  : iso8583_rawdata [in]                                  */
/*                      ISO8583メッセージ raw dataを格納する。(bitmap以降の部分)*/
/*                  : iso8583_rawdata_len [in]                              */
/*                      iso8583_rawdataのメッセージ長                       */
/*                                                                          */
/*  RETURN CODE     : true  - 正常終了                                      */
/*                  : false - エラー有                                      */
/*                                                                          */
/*  DESCRIPTION     : 固定フォーマット作成                                  */
/*                    ISO8583メッセージを固定フォーマットに出力する。       */
/****************************************************************************/
bool com_btm_render_fixformat(ISO8583object_t *obj, char * fixformat, const char * iso8583_rawdata, const size_t iso8583_rawdata_len)
{
    bitmap_t        bitmap;
    const ffd_def   *ffd;
    int             de_number;
    int             bit_value;
    size_t          ffd_count;

    /// ISO8583object_tがMTIで初期化されていない場合はエラーを返す。
    if(!(obj->m_ffd_index_rec))
    {
        CLR_ERR_INFO(obj);
        SET_ERR_INFO_ERCD(obj,GFPCGX20_ER_UNINITIALIZED) ;
        SET_ERR_INFO_DESC(obj,"ISO8583object_t is not initialized.");
        SET_ERR_INFO_DMP(obj,obj,sizeof(ISO8583object_t));
        return false;
    }
    /// 2.ISO8583object_tを初期化し、引数からバッファを割当てる。
    memset(&bitmap, 0, sizeof(bitmap_t));
    obj->m_iso8583BufferLength = iso8583_rawdata_len;
    obj->m_iso8583RawDataBuffer = (char *)iso8583_rawdata;
    obj->m_ptr = (char *)iso8583_rawdata;
    obj->m_fixFormatBuffer = fixformat;

    /// 3.bitmapを取得する。
    if(get_bitmap(obj))
    {
        //bitmap不正
        return false;
    }

    /// 4.bitmapの先頭から順にbitを検査し、bitが1の場合は固定フォーマットを検索し当該データエレメントを出力する。
    ffd = obj->m_ffd_rec;
    ffd_count = obj->m_ffd_index_rec->m_ffd_count;
    while(get_next_bit(&(obj->m_bitmap), &de_number,&bit_value))
    {
        /// 存在しないデータエレメントと次bitmapのインジケータはスキップする。
        if(bit_value == 0  ||
           de_number == E_2ND_BITMAP_IND  ||
           de_number == E_3RD_BITMAP_IND ||
           de_number == E_4TH_BITMAP_IND ) continue;
        ffd = find_ffd(ffd, &ffd_count, de_number);
        if(ffd == NULL)
        {
            ///5.未定義のビットマップが存在する場合はエラーを返す。
            CLR_ERR_INFO(obj);
            SET_ERR_INFO_FFDKEY(obj,obj->m_ffd_index_rec->ffd_key);
            SET_ERR_INFO_DEN(obj, de_number);
            SET_ERR_INFO_OFS(obj, obj->m_ptr - obj->m_iso8583RawDataBuffer);
            SET_ERR_INFO_ERCD(obj,GFPCGX20_ER_UNDEFINED_ELEMENT);
            SET_ERR_INFO_DESC(obj,"Undefined data element.");
            SET_ERR_INFO_DMP(obj,&(obj->m_bitmap.m_bitmap_data),get_bitmap_length(&(obj->m_bitmap)));
            return false;
        }
        else
        {
            ///6.固定フォーマット出力を行う。
            if(write_fix_element(obj, ffd))
                return false;
        }
        ffd++;
    }
    return true;
}
/****************************************************************************/
/*  FUNCTION        : comp_mti                                              */
/*  CALLING SEQ.    : int comp_mti                                          */
/*                           (const void *lhs,                              */
/*                            const void *rhs)                              */
/*                                                                          */
/*  ARGUMENT        : lhs [in]                                              */
/*                      ffd_key_t 検索対象のMTI4byte                        */
/*                  : rhs [in]                                              */
/*                        ffd_index_def IPCバージョンと先頭4byteのMTIと     */
/*                        比較する。                                        */
/*                                                                          */
/*  RETURN CODE     : int                                                   */
/*                                                                          */
/*  DESCRIPTION     : search_index関数の比較関数                            */
/****************************************************************************/

int comp_mti(const void * lhs, const void * rhs)
{
    const ffd_key_t      *ffdkey = lhs;
    const ffd_index_def  *index = rhs;
    return memcmp(ffdkey, &(index->ffd_key), sizeof(ffd_key_t));
}
/****************************************************************************/
/*  FUNCTION        : search_index                                          */
/*  CALLING SEQ.    : ffd_index_def* search_index                           */
/*                           (ISO8583context_t *iso8583_ctx,                */
/*                            const mti_t mti_key)                          */
/*                                                                          */
/*  ARGUMENT        : iso8583_ctx [in]                                      */
/*                      初期化済の @@ISO8583context_t オブジェクト          */
/*                  : mti_key [in]                                          */
/*                      検索対象となるMTIキー                               */
/*  RETURN CODE     : ffd_index_def*                                        */
/*                                                                          */
/*  DESCRIPTION     : 固定フォーマット定義インデックステーブル検索          */
/*                    MTIをキーとして固定フォーマット定義インデックス       */
/*                    テーブルからレコードを検索する                        */
/*                    IPCバージョンはiso8583_ctxが持っている                */
/****************************************************************************/
ffd_index_def* search_index(ISO8583context_t *iso8583_ctx, const mti_t mti_key)
{
    ffd_key_t ffdkey;
    memset(&ffdkey, 0, sizeof(ffd_key_t));
    ffdkey.m_ipc_version = iso8583_ctx->m_ipc_version;
    memmove(&ffdkey.m_mti_id, mti_key, sizeof(mti_t));

    return bsearch(&ffdkey,iso8583_ctx->m_ffd_index_tbl,
                 iso8583_ctx->m_ffd_index_tbl_cnt,
                 sizeof(ffd_index_def), comp_mti);
}

/****************************************************************************/
/*  FUNCTION        : write_fix_element                                     */
/*  CALLING SEQ.    : bool write_fix_element                                */
/*                           (ISO8583object_t *obj,                         */
/*                            const ffd_def *ffd)                           */
/*                                                                          */
/*  ARGUMENT        : obj [in,out]                                          */
/*                      ISO8583object_t                                     */
/*                  : ffd [in]                                              */
/*                      出力対象データエレメントの固定フォーマット定義      */
/*                                                                          */
/*  RETURN CODE     : true  - エラー有                                      */
/*                  : false - 正常                                          */
/*                                                                          */
/*  DESCRIPTION     : 固定フォーマットデータエレメント書き出し              */
/****************************************************************************/
bool write_fix_element(ISO8583object_t *obj,const ffd_def *ffd)
{
    ffd_header_def * ffd_header;
    size_t read_len;                    //ISO8583メッセージから読み出したバイト数
    size_t fix_data_length;             //固定フォーマットに書込むデータ長
    transcoder_error_t transcode_result;

    /// 1.固定フォーマット定義からオフセットを取得し、固定フォーマットバッファの書き出し位置を特定する。
    ffd_header = (ffd_header_def *)(obj->m_fixFormatBuffer + ffd->fix_fmt_info.m_tbl_start_offset);

    switch(ffd->raw_msg_fmt_info.m_raw_length_type )
    {
        case FMT_LEN_FIX:
            /// 固定長の場合は固定長フォーマットのm_data_area_size=m_raw_data_max_len(文字数)
            /// BCDの場合バイト数は(m_raw_data_max_len+1)/2になる。
            // memo: m_data_area_size=m_raw_data_max_lenであることは、GFELIの定義に依存するのでチェック不可能
            // TODO:com_btm_initial_ISO8583の整合性チェックに追加するか？
            fix_data_length = ffd->raw_msg_fmt_info.m_raw_data_max_len; //文字数が返る。
            break;
        default:
            transcode_result = ffd->fix_fmt_info.m_fix_length_decoder(obj,
                                        obj->m_ptr,
                                   ffd->raw_msg_fmt_info.m_raw_length_size,
                                               &fix_data_length); //文字数が返る
            switch(transcode_result)
            {
                case E_TRANSCODE_BUFFER_ERROR:
                    // 電文データ超過
                    set_rawdata_buffer_underflow_error(obj,ffd);
                    return true;
                case E_TRANSCODE_ERROR:
                    //length変換エラー
                    set_length_decode_error(obj, ffd);
                    return true;
                default:
                    break;
            }
            obj->m_ptr += ffd->raw_msg_fmt_info.m_raw_length_size;
            break;
    }
    /// データ部書込
    transcode_result = ffd->fix_fmt_info.m_fix_data_decoder(
                        obj,get_ffd_data_area_ptr(ffd_header),
                        ffd->fix_fmt_info.m_data_area_size,
                            obj->m_ptr,
                        fix_data_length, //文字数を指定する。
                        &read_len);    //処理されたバイト数を返す。
                        //※ BCDの場合、fix_data_length文字のASCIIコードを出力し、read_len byte電文データを読込む
    switch(transcode_result)
    {
        case E_TRANSCODE_BUFFER_ERROR:
            set_rawdata_buffer_underflow_error(obj,ffd);
            return true;
        case E_TRANSCODE_FIX_BUFFER_ERROR:
            set_fix_buffer_overflow_error(obj, ffd);
            return true;
        case E_TRANSCODE_ERROR:
        // 現在提供されているトランスコーダはこのエラーを返さない。
            set_value_decode_error(obj, ffd);
            return true;
        default:
            break;
    }
    ffd_header->m_fixvalue_length = fix_data_length;
    ffd_header->m_flg_exist = true;
    obj->m_ptr += read_len;
    return false;
}
/****************************************************************************/
/*  FUNCTION        : get_next_bit                                          */
/*  CALLING SEQ.    : bool get_next_bit                                     */
/*                           (bitmap_t *bitmap,                             */
/*                            int *de_number,                               */
/*                            int *bit_value)                               */
/*                                                                          */
/*  ARGUMENT        : bitmap [in,out]                                       */
/*                      ビットマップ情報                                    */
/*                  : de_number [out]                                       */
/*                      読み出したデータエレメント番号                      */
/*                  : bit_value [out]                                       */
/*                      読み出したビットの値                                 */
/*                                                                          */
/*  RETURN CODE     : true  - 正常                                          */
/*                  : false - ビットマップ読出完了                          */
/*                                                                          */
/*  DESCRIPTION     : Get the next bit object                               */
/*                    ビットマップを先頭から順に読み出すイテレータ          */
/****************************************************************************/
bool get_next_bit(bitmap_t * bitmap, int * de_number, int *bit_value)
{
    int byte_index;
    int bit_index;
    size_t bitmap_length = get_bitmap_length(bitmap);
    /// カレントビットがビットマップ終端ならばfalseを返す。
    if(bitmap->m_current_bit >= bitmap_length * CHAR_BIT)
    {
        *de_number = *bit_value = -1;
        return false;
    }
    /// カレントビットの番号+1がデータエレメント番号
    *de_number = (int)(bitmap->m_current_bit + 1);

    /// カレントビットの値を読出し、bit_valueに返す。
    byte_index = (int)(bitmap->m_current_bit / CHAR_BIT);
    bit_index = (int)(bitmap->m_current_bit % CHAR_BIT + 1);
    *bit_value = (int)((bitmap->m_bitmap_data.c.c_24[byte_index]) >> (CHAR_BIT - bit_index) & 1);
    /// カレントビットをインクリメントする。
    bitmap->m_current_bit++;
    return true;
}
/****************************************************************************/
/*  FUNCTION        : get_bitmap_length                                     */
/*  CALLING SEQ.    : size_t get_bitmap_length                              */
/*                           (bitmap_t *bitmap)                             */
/*                                                                          */
/*  ARGUMENT        : bitmap [in]                                           */
/*                      ビットマップ                                        */
/*                                                                          */
/*  RETURN CODE     : bitmap長（バイト数）                                  */
/*                                                                          */
/*  DESCRIPTION     : Get the bitmap length object                          */
/****************************************************************************/
size_t get_bitmap_length(bitmap_t *bitmap)
{
    size_t length = E_1ST_BITMAP_BYTE_LENGTH;
    if(bitmap->m_bitmap2)
    {
        length = E_2ND_BITMAP_BYTE_LENGTH;
        if(bitmap->m_bitmap3) length = E_3RD_BITMAP_BYTE_LENGTH;
    }
    return length;
}
/****************************************************************************/
/*  FUNCTION        : find_ffd                                              */
/*  CALLING SEQ.    : const ffd_def* find_ffd                               */
/*                           (const ffd_def *start,                         */
/*                            size_t *count,                                */
/*                            const int de_number)                          */
/*                                                                          */
/*  ARGUMENT        : start [in]                                            */
/*                      固定フォーマット検索開始位置                        */
/*                  : count [in,out]                                        */
/*                      固定フォーマット定義の残り件数                      */
/*                  : de_number [in]                                        */
/*                      検索対象データエレメント                            */
/*                                                                          */
/*  RETURN CODE     : const ffd_def*                                        */
/*                      対象データエレメントの固定フォーマット定義          */
/*                                                                          */
/*  DESCRIPTION     : 固定フォーマット検索                                  */
/****************************************************************************/
const ffd_def * find_ffd(const ffd_def * start, size_t *count ,const int de_number)
{
    while(*count > 0)
    {
        (*count) --;
        if(start->pri_key.m_de_num == de_number)
        {
           return start;
        }
        start ++;
    }
    return NULL;
}

/****************************************************************************/
/*  FUNCTION        : get_bitmap                                            */
/*  CALLING SEQ.    : bool get_bitmap                                       */
/*                           (ISO8583object_t *obj)                         */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      初期済でバッファ割当て済のISO8583object_t           */
/*                                                                          */
/*  RETURN CODE     : true  - 異常                                          */
/*                  : false - 正常                                          */
/*                                                                          */
/*  DESCRIPTION     : Get the bitmap object                                 */
/****************************************************************************/
bool get_bitmap(ISO8583object_t *obj)
{
    char * tmp;
    size_t bitmap_length_byte;
    // bitmap長判定
    if((obj->m_iso8583RawDataBuffer[E_2ND_BITMAP_BYTE_INDEX] & 0x80)) // 2nd bitmapの有無
    {
        obj->m_bitmap.m_bitmap2 = true;
        if((obj->m_iso8583RawDataBuffer[E_3RD_BITMAP_BYTE_INDEX] & 0x80)) // 3nd bitmapの有無
        {
            obj->m_bitmap.m_bitmap3 = true;
            if((obj->m_iso8583RawDataBuffer[E_4TH_BITMAP_BYTE_INDEX] & 0x80)) // 4th bitmapの有無
            {
                //4th bitmapはサポートしない
                CLR_ERR_INFO(obj);
                SET_ERR_INFO_FFDKEY(obj,obj->m_ffd_index_rec->ffd_key);
                SET_ERR_INFO_OFS(obj, obj->m_ptr - obj->m_iso8583RawDataBuffer);
                SET_ERR_INFO_ERCD(obj,GFPCGX20_ER_R2F_4TH_BITMAP_EXIST);
                SET_ERR_INFO_DESC(obj,"ISO8583message have 4th bitmap.")
                SET_ERR_INFO_DMP(obj,obj->m_ptr,obj->m_iso8583BufferLength);
                return true;
    }   }   }
    bitmap_length_byte = get_bitmap_length(&(obj->m_bitmap));
    memmove(&(obj->m_bitmap), obj->m_ptr, bitmap_length_byte);
    obj->m_bitmap.m_current_bit = 0;
    tmp = obj->m_ptr + bitmap_length_byte;
#pragma clang diagnostic push
//cspell:disable
#pragma clang diagnostic ignored "-Wsign-compare"
//cspell:enable
    if(tmp - obj->m_iso8583RawDataBuffer > obj->m_iso8583BufferLength)
    {
        CLR_ERR_INFO(obj);
        SET_ERR_INFO_FFDKEY(obj,obj->m_ffd_index_rec->ffd_key);
        SET_ERR_INFO_OFS(obj, obj->m_ptr - obj->m_iso8583RawDataBuffer);
        SET_ERR_INFO_ERCD(obj,GFPCGX20_ER_RAW_BUFFER_UNDERFLOW);
        SET_ERR_INFO_DESC(obj,"ISO8583message rawdata underflow.")
        SET_ERR_INFO_DMP(obj,obj->m_ptr,obj->m_iso8583BufferLength);
        return true;
    }
#pragma clang diagnostic pop
    obj->m_ptr = tmp;
    return false;
}

/****************************************************************************/
/*  FUNCTION        : set_rawdata_buffer_underflow_error                    */
/*  CALLING SEQ.    : void set_rawdata_buffer_underflow_error               */
/*                           (ISO8583object_t *obj,                         */
/*                            const ffd_def *ffd)                           */
/*                                                                          */
/*  ARGUMENT        : obj [in,out]                                          */
/*                      ISO8583object_t                                     */
/*                  : ffd [in]                                              */
/*                      ffd_def                                              */
/*                                                                          */
/*  RETURN CODE     : void                                                  */
/*                                                                          */
/*  DESCRIPTION     : Set the rawdata buffer underflow error object         */
/*                    GFPCGX20_ER_RAW_BUFFER_UNDERFLOW                      */
/*                    (ISO8583メッセージデータ超過)エラー情報設定           */
/*                    固定フォーマット展開時に電文の処理位置が              */
/*                    ISO8583メッセージ長を超えた。                         */
/****************************************************************************/

void set_rawdata_buffer_underflow_error(ISO8583object_t * obj, const ffd_def * ffd)
{
        CLR_ERR_INFO(obj);
        SET_ERR_INFO_FFDKEY(obj,obj->m_ffd_index_rec->ffd_key);
        SET_ERR_INFO_DEN(obj,ffd->pri_key.m_de_num);
        SET_ERR_INFO_OFS(obj, obj->m_ptr - obj->m_iso8583RawDataBuffer);
        SET_ERR_INFO_ERCD(obj,GFPCGX20_ER_RAW_BUFFER_UNDERFLOW);
        SET_ERR_INFO_DESC(obj,"ISO8583message rawdata underflow.")
        SET_ERR_INFO_DMP(obj,obj->m_ptr,obj->m_iso8583BufferLength -(obj->m_ptr - obj->m_iso8583RawDataBuffer));
}
/****************************************************************************/
/*  FUNCTION        : set_fix_buffer_overflow_error                         */
/*  CALLING SEQ.    : void set_fix_buffer_overflow_error                    */
/*                           (ISO8583object_t *obj,                         */
/*                            const ffd_def *ffd)                           */
/*                                                                          */
/*  ARGUMENT        : obj [in,out]                                          */
/*                      ISO8583object_t                                     */
/*                  : ffd [in]                                              */
/*                      ffd_def                                              */
/*                                                                          */
/*  RETURN CODE     : void                                                  */
/*                                                                          */
/*  DESCRIPTION     : Set the fix buffer overflow error object              */
/*                    GFPCGX20_ER_FIX_BUFFER_OVERFLOW                       */
/*                    (固定フォーマットデータエリアサイズ超過)エラー情報設定*/
/*                    固定フォーマットにデータを書込む際、                   */
/*                    データエリアサイズを超過した。                        */
/****************************************************************************/
void set_fix_buffer_overflow_error(ISO8583object_t * obj, const ffd_def * ffd)
{
        CLR_ERR_INFO(obj);
        SET_ERR_INFO_FFDKEY(obj,obj->m_ffd_index_rec->ffd_key);
        SET_ERR_INFO_DEN(obj,ffd->pri_key.m_de_num);
        SET_ERR_INFO_OFS(obj, obj->m_ptr - obj->m_iso8583RawDataBuffer);
        SET_ERR_INFO_ERCD(obj,GFPCGX20_ER_FIX_BUFFER_OVERFLOW);
        SET_ERR_INFO_DESC(obj,"ISO8583message fix data_area_size overflow.")
        SET_ERR_INFO_DMP(obj,obj->m_ptr,obj->m_iso8583BufferLength -(obj->m_ptr - obj->m_iso8583RawDataBuffer));
}
/****************************************************************************/
/*  FUNCTION        : set_length_decode_error                               */
/*  CALLING SEQ.    : void set_length_decode_error                          */
/*                           (ISO8583object_t *obj,                         */
/*                            const ffd_def *ffd)                           */
/*                                                                          */
/*  ARGUMENT        : obj [in,out]                                          */
/*                      ISO8583object_t                                     */
/*                  : ffd [in]                                              */
/*                      ffd_def                                              */
/*                                                                          */
/*  RETURN CODE     : void                                                  */
/*                                                                          */
/*  DESCRIPTION     : Set the length decode error object                    */
/*                    GFPCGX20_ER_R2F_LENGTH_TRANS_ERR                      */
/*                    固定フォーマット展開時に可変データ長データエレメントの*/
/*                    データエレメント長のデコードに失敗した               */
/****************************************************************************/
void set_length_decode_error(ISO8583object_t * obj, const ffd_def * ffd)
{
    ///length変換エラー
    CLR_ERR_INFO(obj) ;
    SET_ERR_INFO_FFDKEY(obj,obj->m_ffd_index_rec->ffd_key);
    SET_ERR_INFO_DEN(obj,ffd->pri_key.m_de_num);
    SET_ERR_INFO_OFS(obj,obj->m_ptr - obj->m_iso8583RawDataBuffer);
    SET_ERR_INFO_ERCD(obj,GFPCGX20_ER_R2F_LENGTH_TRANS_ERR);
    SET_ERR_INFO_DESC(obj, "var data element length transcode error.")
    SET_ERR_INFO_DMP(obj,obj->m_ptr, obj->m_iso8583BufferLength - obj->m_error_info.m_currentOffset);
}
/****************************************************************************/
/*  FUNCTION        : set_value_decode_error                                */
/*  CALLING SEQ.    : void set_value_decode_error                           */
/*                           (ISO8583object_t *obj,                         */
/*                            const ffd_def *ffd)                           */
/*                                                                          */
/*  ARGUMENT        : obj [in,out]                                          */
/*                      ISO8583object_t                                     */
/*                  : ffd [in]                                              */
/*                      ffd_def                                              */
/*                                                                          */
/*  RETURN CODE     : void                                                  */
/*                                                                          */
/*  DESCRIPTION     : Set the value decode error object                     */
/*                    GFPCGX20_ER_R2F_VALUE_TRANS_ERR                       */
/*                    固定フォーマット展開時に可変データ長データエレメントの*/
/*                    データ部のデコードに失敗した                          */
/****************************************************************************/
void set_value_decode_error(ISO8583object_t * obj, const ffd_def * ffd)
{
    ///length変換エラー
    CLR_ERR_INFO(obj);
    SET_ERR_INFO_FFDKEY(obj,obj->m_ffd_index_rec->ffd_key);
    SET_ERR_INFO_DEN(obj,ffd->pri_key.m_de_num);
    SET_ERR_INFO_OFS(obj,obj->m_ptr - obj->m_iso8583RawDataBuffer);
    SET_ERR_INFO_ERCD(obj,GFPCGX20_ER_R2F_VALUE_TRANS_ERR);
    SET_ERR_INFO_DESC(obj, "data element value transcode error.")
    SET_ERR_INFO_DMP(obj,obj->m_ptr, obj->m_iso8583BufferLength - obj->m_error_info.m_currentOffset);
}
/****************************************************************************/
/*  FUNCTION        : get_ffd_data_area_ptr                                 */
/*  CALLING SEQ.    : void* get_ffd_data_area_ptr                           */
/*                           (ffd_header_def *ffd_header)                   */
/*                                                                          */
/*  ARGUMENT        : ffd_header [in]                                       */
/*                      ffd_templateのヘッダ構造体                          */
/*                                                                          */
/*  RETURN CODE     : void*                                                 */
/*                      ffd_templateの次の1バイトの先頭アドレス             */
/*                                                                          */
/*  DESCRIPTION     : Get the ffd data area ptr object                      */
/*                    ffd_templateの次の1バイトの先頭アドレスを返す         */
/****************************************************************************/
void * get_ffd_data_area_ptr(ffd_header_def *ffd_header)
{
    return  (void *)(++ffd_header);
}
