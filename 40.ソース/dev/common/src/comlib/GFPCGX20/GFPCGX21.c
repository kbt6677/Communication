/******************************************************************************
*                                                                             *
*                               ＜GFP通信制御＞                               *
*                                                                             *
*                         ＜共通(ビットマップ展開・組立て)＞                  *
*                                                                             *
*        VERSION                               :＜1.0.0＞                     *
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
#include <stdio.h>
#include <stdlib.h> nolist
#include <string.h> nolist
#include <limits.h> nolist
/* USER HEADER */
#define __LIBRARY_COMPILE
#include <GFPCGX20.h> nolist  // public header
#include <GFPCGX21.h> nolist  // impliment header.

/****************************************************************************/
/*   内部関数定義                                                           */
/****************************************************************************/
void set_value_encode_error(ISO8583object_t * obj, ffd_def * ffd);
void set_length_encode_error(ISO8583object_t * obj, ffd_def * ffd, ffd_header_def * ffd_header);
void set_var_value_encode_error(ISO8583object_t * obj, ffd_def * ffd,ffd_header_def * ffd_header);
void set_rawdata_buffer_overflow_error(ISO8583object_t * obj, ffd_def * ffd);
bool check_data_length_format(ISO8583object_t *obj,ffd_def *ffd,size_t length);

/****************************************************************************/
/*  FUNCTION        : com_btm_render_ISO8583_message                        */
/*  CALLING SEQ.    : bool com_btm_render_ISO8583_message                   */
/*                           (ISO8583object_t *obj,                         */
/*                            char *iso8583_rawdata_buffer,                 */
/*                            size_t iso8583_rawdata_buffer_len,            */
/*                            const char *fixformat)                        */
/*                                                                          */
/*  ARGUMENT        : obj [in,out]                                          */
/*                      MTI検索済のISO8583object_t構造体ポインタ            */
/*                  : iso8583_rawdata_buffer [out]                          */
/*                      ISO8583メッセージ出力用バッファ                     */
/*                  : iso8583_rawdata_buffer_len [in]                       */
/*                      出力バッファ長                                      */
/*                  : fixformat [in]                                        */
/*                      固定フォーマットデータ                              */
/*                                                                          */
/*  RETURN CODE     : true  - 正常                                          */
/*                  : false - エラー有                                      */
/*                                                                          */
/*  DESCRIPTION     : ISO8583メッセージ組立て                               */
/*                    固定フォーマットをISO8583メッセージ形式に変換する     */
/****************************************************************************/
bool com_btm_render_ISO8583_message(ISO8583object_t * obj, char * iso8583_rawdata_buffer,size_t iso8583_rawdata_buffer_len,const char * fixformat)
{
    ffd_def         *ffd;                         // 固定フォーマットポインタ
    size_t          ffd_index;                    // 固定フォーマットループ用カウンタ
    ffd_header_def * ffd_header;                  // 固定長フォーマットエレメント
    bool            result;                       // 処理結果
    size_t          bitmap_length;

    /// 1.ISO8583object_tの初期化チェックを行う。
    if(obj->m_ffd_index_rec == NULL)
    {
        //ISO8583context_tの未初期化エラー
        CLR_ERR_INFO(obj);
        SET_ERR_INFO_ERCD(obj, GFPCGX20_ER_UNINITIALIZED);
        SET_ERR_INFO_DESC(obj,"ISO8583object_t is not initialized.");
        SET_ERR_INFO_DMP(obj,obj,sizeof(ISO8583object_t));
        return false;
    }
    /// 2.ISO8583object_tの初期化、バッファの割当てを行う。
    obj->m_iso8583BufferLength = iso8583_rawdata_buffer_len;
    obj->m_iso8583RawDataBuffer = (char *)iso8583_rawdata_buffer;
    obj->m_ptr = (char *)iso8583_rawdata_buffer;
    obj->m_fixFormatBuffer = (char *)fixformat;

    /// 3.bitmapの作成
    /// 固定フォーマットを一巡しm_flg_existが真のビットを1にする。
    ffd = obj->m_ffd_rec;
    memset(&(obj->m_bitmap), 0,sizeof(bitmap_t));
    for(ffd_index = 0; ffd_index < obj->m_ffd_index_rec->m_ffd_count; ffd_index++)
    {
        ffd_header = (ffd_header_def*)(obj->m_fixFormatBuffer + ffd->fix_fmt_info.m_tbl_start_offset);
        if (ffd_header->m_flg_exist)
        {
            if(set_bitmap(obj, ffd->pri_key.m_de_num, 1))
                return false;
        }
        ffd++;
    }
    bitmap_length = get_bitmap_length(&(obj->m_bitmap));
    /// 4.ISO8583メッセージバッファ長チェック
    if(rawdata_buffer_check(obj, bitmap_length))
    {
        set_rawdata_buffer_overflow_error(obj, ffd);
        return false;
    }
    /// 5.ISO8583メッセージ出力バッファにビットマップを出力し、バッファをビットマップ長だけ進める。
    memmove(obj->m_ptr,&(obj->m_bitmap.m_bitmap_data),bitmap_length);
    obj->m_ptr += bitmap_length;

    /// 6.データエレメント出力
    ffd = obj->m_ffd_rec;
    for(ffd_index = 0; ffd_index < obj->m_ffd_index_rec->m_ffd_count; ffd_index++)
    {
        ffd_header = (ffd_header_def*)(obj->m_fixFormatBuffer + ffd->fix_fmt_info.m_tbl_start_offset);
        if (ffd_header->m_flg_exist)
        {
            result = write_raw_data_element(obj,ffd,ffd_header);
            if(result) return false;
        }
        ffd++;
    }
    return true;
}

/****************************************************************************/
/*  FUNCTION        : com_btm_get_ISO8583_message_length                    */
/*  CALLING SEQ.    : size_t com_btm_get_ISO8583_message_length             */
/*                           (ISO8583object_t *obj)                         */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      ISO8583object_t構造体ポインタ                       */
/*                                                                          */
/*  RETURN CODE     : 作成されたISO8583メッセージ長（バイト数）             */
/*                                                                          */
/*  DESCRIPTION     : 作成されたISO8583メッセージ長を返す                   */
/***************************************************************************/
size_t com_btm_get_ISO8583_message_length(ISO8583object_t *obj)
{
    return obj->m_ptr - obj->m_iso8583RawDataBuffer;
}

/****************************************************************************/
/*  FUNCTION        : set_bitmap                                            */
/*  CALLING SEQ.    : bool set_bitmap                                       */
/*                           (ISO8583object_t *obj,                         */
/*                            long de_number,                               */
/*                            long val)                                     */
/*                                                                          */
/*  ARGUMENT        : obj [in,out]                                          */
/*                      MTI検索済のISO8583object_t構造体ポインタ            */
/*                  : de_number [in]                                        */
/*                      対象データエレメント番号                            */
/*                  : val [in]                                              */
/*                      設定値（0:OFF, 0以外:ON）                           */
/*                                                                          */
/*  RETURN CODE     : false - 正常                                          */
/*                  : true  - エラー有                                      */
/*                                                                          */
/*  DESCRIPTION     : Set the bitmap object                                 */
/****************************************************************************/
bool set_bitmap(ISO8583object_t *obj,long de_number,long val)
{
    int byte_index;
    int bit_shift_index;
    /// 次ビットマップのインジケータまたは最小、最大を弾く
    if(de_number == E_2ND_BITMAP_IND ||
       de_number == E_3RD_BITMAP_IND ||
       de_number == E_4TH_BITMAP_IND ||
       de_number < E_MIN_BITMAP_NUM || de_number > E_MAX_BITMAP_NUM)
       {
            CLR_ERR_INFO(obj);
            SET_ERR_INFO_FFDKEY(obj,obj->m_ffd_index_rec->ffd_key);
            SET_ERR_INFO_DEN(obj,de_number);
            SET_ERR_INFO_ERCD(obj,GFPCGX20_ER_INVALID_ELEMENT_NUMBER);
            SET_ERR_INFO_DESC(obj, "Invalid element number.")
            return true;
       }
    byte_index = (de_number - 1) / CHAR_BIT;
    bit_shift_index = (de_number - 1) % CHAR_BIT + 1;

    if(val == 0)
    {
        /// bitをOFFにする場合
        obj->m_bitmap.m_bitmap_data.c.c_24[byte_index] &= ~(1 << (CHAR_BIT - bit_shift_index));
        if(de_number >= E_MIN_2ND_BITMAP_RANGE && de_number <= E_MAX_2ND_BITMAP_RANGE) //2nd bitmapの操作
        {
            /// DE66～DE128の場合、2nd bitmap及び3rd bitmapがともに0ならば、ビット1を0にする。
            if(obj->m_bitmap.m_bitmap_data.l.l_3 == 0 &&
               obj->m_bitmap.m_bitmap_data.l.l_2 == 0 )
            {
                obj->m_bitmap.m_bitmap_data.c.c_24[E_2ND_BITMAP_BYTE_INDEX] &= 0x7F;
                obj->m_bitmap.m_bitmap2 = false;
            }
        }
        if(de_number >= E_MIN_3RD_BITMAP_RANGE && de_number <= E_MAX_BITMAP_NUM) //3rd bitmapの操作
        {
            /// DE130～DE192の場合、3rd bitmapが0ならば、bit 65を0にする。
            if(obj->m_bitmap.m_bitmap_data.l.l_3 == 0 )
            {
                obj->m_bitmap.m_bitmap_data.c.c_24[E_3RD_BITMAP_BYTE_INDEX] &= 0x7F;
                obj->m_bitmap.m_bitmap3 = false;
            }
            /// さらに、2nd bitmapが0ならば、bit 1も0にする。
            if(obj->m_bitmap.m_bitmap_data.l.l_2 == 0)
            {
                obj->m_bitmap.m_bitmap_data.c.c_24[E_2ND_BITMAP_BYTE_INDEX] &= 0x7F;
                obj->m_bitmap.m_bitmap2 = false;
            }
        }
    }
    else
    {
        /// bitをONにする場合
        obj->m_bitmap.m_bitmap_data.c.c_24[byte_index] |= (1 << (CHAR_BIT - bit_shift_index));
        if(de_number >= E_MIN_2ND_BITMAP_RANGE && de_number <= E_MAX_2ND_BITMAP_RANGE) //2nd bitmapの操作
        {
            //// DE66～128 ならば bit 1をON 、bitmap長は2
            obj->m_bitmap.m_bitmap_data.c.c_24[E_2ND_BITMAP_BYTE_INDEX] |= 0x80;
            obj->m_bitmap.m_bitmap2 = true;
        }
        if(de_number >= E_MIN_3RD_BITMAP_RANGE && de_number <= E_MAX_BITMAP_NUM) //3rd bitmapの操作
        {
            /// DE130からDE192ならば、bit0及び、bit65を1 bitmap長は3
            obj->m_bitmap.m_bitmap_data.c.c_24[E_2ND_BITMAP_BYTE_INDEX] |= 0x80;
            obj->m_bitmap.m_bitmap_data.c.c_24[E_3RD_BITMAP_BYTE_INDEX] |= 0x80;
            obj->m_bitmap.m_bitmap2 = true;
            obj->m_bitmap.m_bitmap3 = true;
        }
    }
    return false;
}

/****************************************************************************/
/*  FUNCTION        : write_raw_data_element                                */
/*  CALLING SEQ.    : bool write_raw_data_element                           */
/*                           (ISO8583object_t *obj,                         */
/*                            ffd_def *ffd,                                 */
/*                            ffd_header_def *ffd_header)                   */
/*                                                                          */
/*  ARGUMENT        : obj [in,out]                                          */
/*                      初期化済のISO8583object_t構造体ポインタ             */
/*                  : ffd [in]                                              */
/*                      書込対象の固定フォーマット定義構造体ポインタ        */
/*                  : ffd_header [in]                                       */
/*                      書込対象の固定フォーマットレコード構造体ポインタ    */
/*                                                                          */
/*  RETURN CODE     : false - 正常                                          */
/*                  : true  - エラー有                                      */
/*                                                                          */
/*  DESCRIPTION     : データエレメント書込                                  */
/*                    バッファ終端に固定フォーマットから                    */
/*                    組立てたデータエレメントの書込を行う                 */
/****************************************************************************/
bool write_raw_data_element(ISO8583object_t *obj,ffd_def *ffd,ffd_header_def *ffd_header)
{
    transcoder_error_t transcode_result;          ///< 変換処理結果(正常/異常)
    size_t             transcode_value_length;    ///< 変換後データ長
    switch(ffd->raw_msg_fmt_info.m_raw_length_type)
    {
        case FMT_LEN_FIX:
            /// 固定長データ部の書込
            /// m_raw_data_max_len(文字数)と書込データ(文字数)が一致しない場合はエラー
            if(ffd->raw_msg_fmt_info.m_raw_data_max_len != ffd_header->m_fixvalue_length)
            {
                set_value_encode_error(obj, ffd);
                return true;
            }
            transcode_result = ffd->raw_msg_fmt_info.m_raw_data_encoder(obj,
                                                    obj->m_ptr,
                                                    ffd->raw_msg_fmt_info.m_raw_data_max_len,//文字数
                                                    get_ffd_data_area_ptr(ffd_header),
                                                    ffd_header->m_fixvalue_length, //文字数
                                                    &transcode_value_length);      //transcode_value_length = バイト数
            switch (transcode_result)
            {
                case E_TRANSCODE_BUFFER_ERROR:
                    set_rawdata_buffer_overflow_error(obj, ffd);
                    return true;
                case E_TRANSCODE_ERROR:
                     //現在提供されているトランスコーダーではこのエラーは発生しない
                    set_value_encode_error(obj, ffd);
                    return true;
                default:
                    break;
            }
            ///変換結果の文字数が電文フォーマット情報.最大データ長(固定長フィールド長)と一致するかチェックする。
            ///BCD用ラウンドワーク
            ///文字コードがBCD以外の場合はtranscode_value_lengthがm_raw_data_max_lenと等しい
            ///transcode_value_lengthとm_fixvalue_lengthが一致しない場合はbcdコードなので、
            ///BCDの場合はtranscode_value_lengthが((m_raw_data_max_len+1)/2)に等しいこと。
            if(transcode_value_length != ffd->raw_msg_fmt_info.m_raw_data_max_len
                && (ffd_header->m_fixvalue_length != transcode_value_length && transcode_value_length != (ffd->raw_msg_fmt_info.m_raw_data_max_len + 1) /2))
            {
                set_value_encode_error(obj, ffd);
                return true;
            }
            obj->m_ptr += transcode_value_length;
            break;
        default:
        {
            //変換後の実際のデータ長が不明なので最大長までバッファを用意しておく
            char value_work_buffer[ffd->raw_msg_fmt_info.m_raw_data_max_len];
            /* 制御電文エレメントが正しく定義されているべきなので、ここでチェックはしない。
               電文フォーマット情報の最大データ長を越えればエラーになる。
            ///データ長形式のチェック(LVAR～LLVARの範囲に収まっていること。
            if(check_data_length_format(obj,ffd,ffd_header->m_fixvalue_length))
                return true;
            */
            ///データ長部の書込
            transcode_result = ffd->raw_msg_fmt_info.m_raw_length_encoder(
                                obj,
                                obj->m_ptr,
                                ffd->raw_msg_fmt_info.m_raw_length_size,
                                ffd_header->m_fixvalue_length);   //BCDの場合書込データ長(byte)が1/2(切上)になるが
                                                                       //FIXフォーマットから文字数の変わるフィールドは存在しないため
                                                                       //そのままデータ長として書込む
            switch (transcode_result)
            {
                case E_TRANSCODE_BUFFER_ERROR:
                    set_rawdata_buffer_overflow_error(obj, ffd);
                    return true;
                case E_TRANSCODE_ERROR:
                    set_length_encode_error(obj, ffd,ffd_header);
                    return true;
                default:
                    break;
            }
            obj->m_ptr += ffd->raw_msg_fmt_info.m_raw_length_size;

            /// データ部を作成する。
            transcode_result = ffd->raw_msg_fmt_info.m_raw_data_encoder(
                    obj,
                    value_work_buffer,
                    sizeof(value_work_buffer),
                    get_ffd_data_area_ptr(ffd_header),
                    ffd_header->m_fixvalue_length,   //文字数
                    &transcode_value_length);        //書込んだバイト数
                    //※ BCDの場合、m_fixvalue_length文字のASCIIコードをBCD変換し、read_len byte(文字数の1/2切上)電文データを書込む
            switch (transcode_result)
            {
                case E_TRANSCODE_BUFFER_ERROR:
                    set_rawdata_buffer_overflow_error(obj, ffd);
                    return true;
                case E_TRANSCODE_ERROR:
                    //現在提供されているトランスコーダーではこのエラーは発生しない
                    set_var_value_encode_error(obj, ffd, ffd_header);
                    return true;
                default:
                    break;
            }
            //データ部の書込
            memmove(obj->m_ptr, value_work_buffer, transcode_value_length);
            obj->m_ptr += transcode_value_length;
        }
    }
    return false;
}

/****************************************************************************/
/*  FUNCTION        : set_rawdata_buffer_overflow_error                     */
/*  CALLING SEQ.    : void set_rawdata_buffer_overflow_error                */
/*                           (ISO8583object_t *obj,                         */
/*                            ffd_def *ffd)                                 */
/*                                                                          */
/*  ARGUMENT        : obj [in,out]                                          */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : ffd [in]                                              */
/*                      固定フォーマット定義構造体ポインタ                  */
/*                                                                          */
/*  RETURN CODE     : void                                                  */
/*                                                                          */
/*  DESCRIPTION     : Set the rawdata buffer overflow error object          */
/*                    ISO8583メッセージ生成時に、出力バッファ長を           */
/*                    超過した場合のエラー情報を設定する                   */
/****************************************************************************/
void set_rawdata_buffer_overflow_error(ISO8583object_t * obj, ffd_def * ffd)
{
    CLR_ERR_INFO(obj);
    SET_ERR_INFO_FFDKEY(obj,obj->m_ffd_index_rec->ffd_key);
    SET_ERR_INFO_DEN(obj,ffd->pri_key.m_de_num);
    SET_ERR_INFO_OFS(obj, obj->m_ptr - obj->m_iso8583RawDataBuffer);
    SET_ERR_INFO_ERCD(obj,GFPCGX20_ER_RAW_BUFFER_OVERFLOW);
    SET_ERR_INFO_DESC(obj,"ISO8583message rawdata overflow.")
    SET_ERR_INFO_DMP(obj,obj->m_iso8583RawDataBuffer,obj->m_iso8583BufferLength);
}

/****************************************************************************/
/*  FUNCTION        : set_value_encode_error                                */
/*  CALLING SEQ.    : void set_value_encode_error                           */
/*                           (ISO8583object_t *obj,                         */
/*                            ffd_def *ffd)                                 */
/*                                                                          */
/*  ARGUMENT        : obj [in,out]                                          */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : ffd [in]                                              */
/*                      固定フォーマット定義構造体ポインタ                  */
/*                                                                          */
/*  RETURN CODE     : void                                                  */
/*                                                                          */
/*  DESCRIPTION     : Set the value encode error object                     */
/*                    固定フォーマットからのデータエレメント変換時に        */
/*                    値のエンコードに失敗した場合のエラー情報を設定する    */
/****************************************************************************/
void set_value_encode_error(ISO8583object_t * obj, ffd_def * ffd)
{
    CLR_ERR_INFO(obj) ;
    SET_ERR_INFO_FFDKEY(obj,obj->m_ffd_index_rec->ffd_key);
    SET_ERR_INFO_DEN(obj,ffd->pri_key.m_de_num);
    SET_ERR_INFO_OFS(obj,(long)ffd->fix_fmt_info.m_tbl_start_offset);
    SET_ERR_INFO_ERCD(obj,GFPCGX20_ER_F2R_VALUE_TRANS_ERR);
    SET_ERR_INFO_DESC(obj, "Fixformat value encode error.")
    SET_ERR_INFO_DMP(obj,(obj->m_fixFormatBuffer + ffd->fix_fmt_info.m_tbl_start_offset),
                     ffd->fix_fmt_info.m_data_area_size + sizeof(ffd_header_def));

}

/****************************************************************************/
/*  FUNCTION        : set_var_value_encode_error                            */
/*  CALLING SEQ.    : void set_var_value_encode_error                       */
/*                           (ISO8583object_t *obj,                         */
/*                            ffd_def *ffd,                                 */
/*                            ffd_header_def *ffd_header)                   */
/*                                                                          */
/*  ARGUMENT        : obj [in,out]                                          */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : ffd [in]                                              */
/*                      固定フォーマット定義構造体ポインタ                  */
/*                  : ffd_header [in]                                       */
/*                      固定フォーマットデータのヘッダ構造体ポインタ        */
/*                                                                          */
/*  RETURN CODE     : void                                                  */
/*                                                                          */
/*  DESCRIPTION     : Set the var value encode error object                 */
/*                    可変長データエレメントのエンコードに失敗した場合に    */
/*                    エラー情報を設定する                                  */
/****************************************************************************/
void set_var_value_encode_error(ISO8583object_t * obj, ffd_def * ffd, ffd_header_def * ffd_header)
{
    CLR_ERR_INFO(obj) ;
    SET_ERR_INFO_FFDKEY(obj,obj->m_ffd_index_rec->ffd_key);
    SET_ERR_INFO_DEN(obj,ffd->pri_key.m_de_num);
    SET_ERR_INFO_OFS(obj,(long)ffd->fix_fmt_info.m_tbl_start_offset);
    SET_ERR_INFO_ERCD(obj,GFPCGX20_ER_F2R_VALUE_TRANS_ERR);
    SET_ERR_INFO_DESC(obj, "Fixformat value encode error.")
    SET_ERR_INFO_DMP(obj,ffd_header,
                     ffd_header->m_fixvalue_length + sizeof(ffd_header_def));
}

/****************************************************************************/
/*  FUNCTION        : set_length_encode_error                               */
/*  CALLING SEQ.    : void set_length_encode_error                          */
/*                           (ISO8583object_t *obj,                         */
/*                            ffd_def *ffd,                                 */
/*                            ffd_header_def *ffd_header)                   */
/*                                                                          */
/*  ARGUMENT        : obj [in,out]                                          */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : ffd [in]                                              */
/*                      固定フォーマット定義構造体ポインタ                  */
/*                  : ffd_header [in]                                       */
/*                      固定フォーマットデータのヘッダ構造体ポインタ        */
/*                                                                          */
/*  RETURN CODE     : void                                                  */
/*                                                                          */
/*  DESCRIPTION     : Set the length encode error object                    */
/*                    データエレメント長のエンコードに失敗した場合の        */
/*                    エラー情報を設定する                                  */
/****************************************************************************/
void set_length_encode_error(ISO8583object_t * obj, ffd_def * ffd, ffd_header_def * ffd_header)
{
    CLR_ERR_INFO(obj) ;
    SET_ERR_INFO_FFDKEY(obj,obj->m_ffd_index_rec->ffd_key);
    SET_ERR_INFO_DEN(obj,ffd->pri_key.m_de_num);
    SET_ERR_INFO_OFS(obj,(long)ffd->fix_fmt_info.m_tbl_start_offset);
    SET_ERR_INFO_ERCD(obj,GFPCGX20_ER_F2R_LENGTH_TRANS_ERR);
    SET_ERR_INFO_DESC(obj, "Fixformat length encode error.")
    SET_ERR_INFO_DMP(obj,ffd_header,
                     ffd_header->m_fixvalue_length + sizeof(ffd_header_def));
}

/****************************************************************************/
/*  FUNCTION        : check_data_length_format                              */
/*  CALLING SEQ.    : bool check_data_length_format                         */
/*                           (ISO8583object_t *obj,                         */
/*                            ffd_def *ffd,                                 */
/*                            size_t length)                                */
/*                                                                          */
/*  ARGUMENT        : obj [in,out]                                          */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : ffd [in]                                              */
/*                      チェック対象の固定フォーマット定義構造体ポインタ    */
/*                  : length [in]                                           */
/*                      データエレメントの長さ（文字数）                    */
/*                                                                          */
/*  RETURN CODE     : true  - 長さ超過エラーあり                            */
/*                  : false - フォーマット内に収まっている                  */
/*                                                                          */
/*  DESCRIPTION     : 可変長データエレメントの長さが                        */
/*                    定義されたLnVARフォーマットの上限を超えていないか     */
/*                    を検証する                                            */
/****************************************************************************/
bool check_data_length_format(ISO8583object_t *obj,ffd_def *ffd,size_t length)
{
    switch(ffd->raw_msg_fmt_info.m_raw_length_type)
    {
        case FMT_LEN_LVAR:
            if(length <= 9)
                return false;
            break;
        case FMT_LEN_LLVAR:
            if(length <= 99)
                return false;
            break;
        case FMT_LEN_LLLVAR:
            if(length <= 999)
                return false;
            break;
        case FMT_LEN_LLLLVAR:
            if(length <= 9999)
                return false;
            break;
        default:
            return false;
    }

    CLR_ERR_INFO(obj);
    SET_ERR_INFO_FFDKEY(obj,ffd->pri_key.ffd_key);
    SET_ERR_INFO_DEN(obj,ffd->pri_key.m_de_num);
    SET_ERR_INFO_OFS(obj,(long)ffd->fix_fmt_info.m_tbl_start_offset) ;
    SET_ERR_INFO_ERCD(obj,GFPCGX20_ER_VAR_ELEMENT_LENGTH_ERROR) ;
    SET_ERR_INFO_DESC(obj,"Data element length exceed variable data element format(LnVAR).");
    SET_ERR_INFO_DMP(obj,obj->m_ptr,obj->m_ffd_index_rec->m_fix_format_length - ffd->fix_fmt_info.m_tbl_start_offset);

    return true;
}
