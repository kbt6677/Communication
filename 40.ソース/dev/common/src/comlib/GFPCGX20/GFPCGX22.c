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
*        MODIFY DATE                           :＜修正日 YYYY/MM/DD＞         *
*        CODED                                 :＜修正者＞                    *
*                                              :＜修正概要＞                  *
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
/****************************************************************************/

/* ################### */
/* # INCLUDE         # */
/* ################### */
/* STANDARD HEADER */
#include <ctype.h> nolist
#include <stdio.h> nolist
#include <errno.h> nolist
#include <stdlib.h> nolist
#include <string.h> nolist
#include <common.h> nolist
/* USER HEADER */
#include <GFPOGGZ3_encode.h> nolist
#define __LIBRARY_COMPILE
#include <GFPCGX20.h> nolist  // public header
#include <GFPCGX21.h> nolist  // impliment header.

/****************************************************************************/
/*   内部関数定義                                                           */
/****************************************************************************/

//データ長部展開用
transcoder_error_t BIN_len_decoder(ISO8583object_t * obj, char * raw_length,size_t raw_length_size,size_t *val);
transcoder_error_t BCD_len_decoder(ISO8583object_t * obj, char * raw_length,size_t raw_length_size,size_t *val);
transcoder_error_t ASCII_len_decoder(ISO8583object_t * obj, char * raw_length,size_t raw_length_size,size_t *val);
transcoder_error_t EBCDIC_len_decoder(ISO8583object_t * obj, char * raw_length,size_t raw_length_size,size_t *val);

//データ長部組立用
transcoder_error_t BIN_len_encoder(ISO8583object_t * obj, char * raw_length,size_t raw_length_size,size_t val);
transcoder_error_t BCD_len_encoder(ISO8583object_t * obj, char * raw_length,size_t raw_length_size,size_t val);
transcoder_error_t ASCII_len_encoder(ISO8583object_t * obj, char * raw_length,size_t raw_length_size,size_t val);
transcoder_error_t EBCDIC_len_encoder(ISO8583object_t * obj, char * raw_length,size_t raw_length_size,size_t val);

//データ部展開用
transcoder_error_t null_decoder(ISO8583object_t * obj, char * dst,size_t dst_len,const char * src,size_t src_len,size_t * write_len);
transcoder_error_t BCD_decoder(ISO8583object_t * obj, char * dst,size_t dst_len,const char * src,size_t src_len,size_t * write_len);
transcoder_error_t ASCII_decoder(ISO8583object_t * obj, char * dst,size_t dst_len,const char * src,size_t src_len,size_t * write_len);
transcoder_error_t EBCDIC_decoder(ISO8583object_t * obj, char * dst,size_t dst_len,const char * src,size_t src_len,size_t * write_len);
transcoder_error_t JIS8_decoder(ISO8583object_t * obj, char * dst,size_t dst_len,const char * src,size_t src_len,size_t * write_len);

//データ部組立用
transcoder_error_t null_encoder(ISO8583object_t * obj, char * dst,size_t dst_len,const char * src,size_t src_len,size_t * write_len);
transcoder_error_t BCD_encoder(ISO8583object_t * obj, char * dst,size_t dst_len,const char * src,size_t src_len,size_t * write_len);
transcoder_error_t ASCII_encoder(ISO8583object_t * obj, char * dst,size_t dst_len,const char * src,size_t src_len,size_t * write_len);
transcoder_error_t EBCDIC_encoder(ISO8583object_t * obj, char * dst,size_t dst_len,const char * src,size_t src_len,size_t * write_len);
transcoder_error_t JIS8_encoder(ISO8583object_t * obj, char * dst,size_t dst_len,const char * src,size_t src_len,size_t * write_len);

/****************************************************************************/
/*   定数定義                                                               */
/****************************************************************************/

//データ長部展開組立用
length_transcoder_set_def
    ASCII_LEN_TRANSCODER  = { ASCII_len_decoder , ASCII_len_encoder},  ///< ASCIIコードのデータ長部変換メソッド
    BCD_LEN_TRANSCODER    = { BCD_len_decoder   , BCD_len_encoder},    ///< BCDコードのデータ長部変換メソッド
    EBCDIC_LEN_TRANSCODER = { EBCDIC_len_decoder, EBCDIC_len_encoder}, ///< EBCDICコードのデータ長部変換メソッド
    BIN_LEN_TRANSCODER    = { BIN_len_decoder   , BIN_len_encoder};    ///< バイナリのデータ長部変換メソッド

//データ部展開組立用
transcoder_set_def
//  ASCII_TRANSCODER = {null_decoder ,null_encoder },
    BCD_TRANSCODER   = {BCD_decoder   ,BCD_encoder },         ///< BCDコードのデータ部変換メソッド
    EBCDIC_TRANSCODER = {EBCDIC_decoder,EBCDIC_encoder },     ///< BCDコードのデータ部変換メソッド
//  JIS8_TRANSCODER  = {null_decoder  ,null_encoder },
    NULL_TRANSCODER  = {null_decoder  ,null_encoder };        ///< その他(変換無)のデータ部変換メソッド

/****************************************************************************/
/*  FUNCTION        : rawdata_buffer_check                                  */
/*  CALLING SEQ.    : bool rawdata_buffer_check                             */
/*                           (ISO8583object_t *obj,                         */
/*                            size_t data_length)                           */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : data_length [in]                                      */
/*                      書き込もうとしているデータ長（バイト数）            */
/*                                                                          */
/*  RETURN CODE     : true  - バッファオーバーの可能性あり                  */
/*                  : false - 問題なし                                      */
/*                                                                          */
/*  DESCRIPTION     : ISO8583出力バッファの残領域が                         */
/*                    指定データ長を格納できるかを検査する                  */
/****************************************************************************/
bool rawdata_buffer_check(ISO8583object_t * obj, size_t data_length)
{
    if(obj->m_ptr + data_length > obj->m_iso8583RawDataBuffer + obj->m_iso8583BufferLength)
        return true;
    return false;
}

/* ################### */
/* # データ長部展開  # */
/* ################### */

/****************************************************************************/
/*  FUNCTION        : BIN_len_decoder                                       */
/*  CALLING SEQ.    : transcoder_error_t BIN_len_decoder                    */
/*                           (ISO8583object_t *obj,                         */
/*                            char *raw_length,                             */
/*                            size_t raw_length_size,                       */
/*                            size_t *val)                                  */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : raw_length [in]                                       */
/*                      データ長部（バイナリ形式）                          */
/*                  : raw_length_size [in]                                  */
/*                      データ長部のバイト数（1～8バイト）                  */
/*                  : val [out]                                             */
/*                      デコードされたデータ長（文字数）                    */
/*                                                                          */
/*  RETURN CODE     : E_TRANSCODE_VALID           - 正常終了                */
/*                  : E_TRANSCODE_BUFFER_ERROR     - バッファオーバー検出   */
/*                  : E_TRANSCODE_ERROR            - サポート外の長さなど   */
/*                                                                          */
/*  DESCRIPTION     : データ長部展開（Binary(整数) → Binary(整数)整数）    */
/*                    バイナリ形式のデータ長部を数値にデコードする          */
/****************************************************************************/
transcoder_error_t BIN_len_decoder(ISO8583object_t * obj,char * raw_length,size_t raw_length_size,size_t *val)
{

    char buffer[8];

    /// rawdata bufferからデータ長部を読み出せるかチェック
    if(rawdata_buffer_check(obj,raw_length_size))
    {
        return E_TRANSCODE_BUFFER_ERROR;
    }

    memset(buffer, 0 ,sizeof(buffer));
    switch(raw_length_size)
    {
        case 1:
            *val = (size_t)*raw_length;
            break;
        case 2:
            *val = (size_t)*(unsigned short*)raw_length;
            break;
        case 3:
        case 4:
            memmove(buffer + (sizeof(unsigned long) - raw_length_size), raw_length, raw_length_size);
            *val = (size_t)*(unsigned long*)buffer;
            break;
        case 5:
        case 6:
        case 7:
        case 8:
            memmove(buffer + (sizeof(unsigned long long) - raw_length_size), raw_length, raw_length_size);
            *val = (size_t)*(unsigned long*)buffer;
            break;
        default:
            return E_TRANSCODE_ERROR;
    }
    return E_TRANSCODE_VALID;
}

/****************************************************************************/
/*  FUNCTION        : BCD_len_decoder                                       */
/*  CALLING SEQ.    : transcoder_error_t BCD_len_decoder                    */
/*                           (ISO8583object_t *obj,                         */
/*                            char *raw_length,                             */
/*                            size_t raw_length_size,                       */
/*                            size_t *val)                                  */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : raw_length [in]                                       */
/*                      データ長部（BCD形式）                               */
/*                  : raw_length_size [in]                                  */
/*                      データ長部のバイト数                                */
/*                  : val [out]                                             */
/*                      デコードされたデータ長（文字数）                    */
/*                                                                          */
/*  RETURN CODE     : E_TRANSCODE_VALID         - 正常終了                  */
/*                  : E_TRANSCODE_BUFFER_ERROR   - バッファオーバー検出     */
/*                  : E_TRANSCODE_ERROR          - BCD不正（0～9以外含む）  */
/*                                                                          */
/*  DESCRIPTION     : データ長部展開（BCD → Binary整数）                   */
/*                    BCD形式で格納されたデータ長情報を数値に変換する       */
/****************************************************************************/
transcoder_error_t BCD_len_decoder(ISO8583object_t * obj,char * raw_length,size_t raw_length_size,size_t *val)
{
    size_t result = 0;
    unsigned char high,low;
    size_t i;

    /// rawdata bufferからデータ長部を読み出せるかチェック
    if(rawdata_buffer_check(obj,raw_length_size))
    {
        return E_TRANSCODE_BUFFER_ERROR;
    }

    for (i = 0; i < raw_length_size; i++)
    {
        high = (char)(((raw_length[i]) >> 4) & 0x0f);
        low = (char)((raw_length[i]) & 0x0f);
        if(high > 9 || low > 9) return E_TRANSCODE_ERROR;
        result = result * 100 + high * 10 + low;
    }
    *val = result;
    return E_TRANSCODE_VALID;
}
/****************************************************************************/
/*  FUNCTION        : ASCII_len_decoder                                     */
/*  CALLING SEQ.    : transcoder_error_t ASCII_len_decoder                  */
/*                           (ISO8583object_t *obj,                         */
/*                            char *raw_length,                             */
/*                            size_t raw_length_size,                       */
/*                            size_t *val)                                  */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : raw_length [in]                                       */
/*                      データ長部（ASCII形式）                             */
/*                  : raw_length_size [in]                                  */
/*                      データ長部のバイト数                                */
/*                  : val [out]                                             */
/*                      デコードされたデータ長（文字数）                    */
/*                                                                          */
/*  RETURN CODE     : E_TRANSCODE_VALID         - 正常終了                  */
/*                  : E_TRANSCODE_BUFFER_ERROR   - バッファオーバー検出     */
/*                  : E_TRANSCODE_ERROR          - 変換失敗（非数値含む等） */
/*                                                                          */
/*  DESCRIPTION     : データ長部展開（ASCII → 整数）                       */
/*                    ASCII形式の数値文字列を整数としてデコードする         */
/****************************************************************************/
transcoder_error_t ASCII_len_decoder(ISO8583object_t * obj,char * raw_length,size_t raw_length_size,size_t *val)
{
    char buffer[raw_length_size+1];
    char *epos;
    size_t retval;
     /// rawdata bufferからデータ長部を読み出せるかチェック
    if(rawdata_buffer_check(obj,raw_length_size))
    {
        return E_TRANSCODE_BUFFER_ERROR;
    }
    memset(buffer, 0, sizeof(buffer));
    memmove(buffer,raw_length,raw_length_size);
    retval = strtoul(buffer, &epos, 10);
    if(errno == ERANGE ||
#ifdef EINVAL
       errno == EINVAL ||
#endif
       *epos != '\0') return E_TRANSCODE_ERROR;
        *val = retval;
    return E_TRANSCODE_VALID;
}

/****************************************************************************/
/*  FUNCTION        : EBCDIC_len_decoder                                    */
/*  CALLING SEQ.    : transcoder_error_t EBCDIC_len_decoder                 */
/*                           (ISO8583object_t *obj,                         */
/*                            char *raw_length,                             */
/*                            size_t raw_length_size,                       */
/*                            size_t *val)                                  */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : raw_length [in]                                       */
/*                      データ長部（EBCDIC形式）                            */
/*                  : raw_length_size [in]                                  */
/*                      データ長部のバイト数                                */
/*                  : val [out]                                             */
/*                      デコードされたデータ長（文字数）                    */
/*                                                                          */
/*  RETURN CODE     : E_TRANSCODE_VALID         - 正常終了                  */
/*                  : E_TRANSCODE_BUFFER_ERROR   - バッファオーバー検出     */
/*                  : E_TRANSCODE_ERROR          - 変換失敗（非数値含む等） */
/*                                                                          */
/*  DESCRIPTION     : データ長部展開（EBCDIC → 整数）                      */
/*                    EBCDIC形式の数値文字列をSJISへ変換し、整数として解釈 */
/****************************************************************************/
transcoder_error_t EBCDIC_len_decoder(ISO8583object_t * obj,char * raw_length,size_t raw_length_size,size_t *val)
{
    char buffer[raw_length_size+1];
    char *epos;
    size_t retval;

    /// rawdata bufferからデータ長部を読み出せるかチェック
    if(rawdata_buffer_check(obj,raw_length_size))
    {
        return E_TRANSCODE_BUFFER_ERROR;
    }
    memset(buffer, 0, sizeof(buffer));
    EBCDIC2SJIS(raw_length, buffer, (short)raw_length_size);
    retval = strtoul(buffer, &epos, 10);
    if(errno == ERANGE ||
#ifdef EINVAL
       errno == EINVAL ||
#endif
       *epos != '\0') return E_TRANSCODE_ERROR;
       *val = retval;
    return E_TRANSCODE_VALID;
}

/****************************************************************************/
/*  FUNCTION        : getByteSize                                           */
/*  CALLING SEQ.    : unsigned int getByteSize                              */
/*                           (size_t num)                                   */
/*                                                                          */
/*  ARGUMENT        : num [in]                                              */
/*                      対象の数値（ビット数に変換する対象）                */
/*                                                                          */
/*  RETURN CODE     : unsigned int                                          */
/*                      num を表現するために必要なバイト数                  */
/*                                                                          */
/*  DESCRIPTION     : 指定された数値を2進数で表現する際に                   */
/*                    必要となるバイト数を計算して返す                      */
/****************************************************************************/
unsigned int getByteSize(size_t num) {
    if (num == 0) return 1; // 0は1バイト

    unsigned int bits = 0;
    while (num > 0) {
        num >>= 1; // 右に1ビットシフト
        bits++;
    }

    return (bits + 7) / 8; // ビット数を8で割って切り上げ
}

/* ################### */
/* # データ長部組立  # */
/* ################### */

/****************************************************************************/
/*  FUNCTION        : BIN_len_encoder                                       */
/*  CALLING SEQ.    : transcoder_error_t BIN_len_encoder                    */
/*                           (ISO8583object_t *obj,                         */
/*                            char *raw_length,                             */
/*                            size_t raw_length_size,                       */
/*                            size_t val)                                   */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : raw_length [out]                                      */
/*                      データ長の出力先                                     */
/*                  : raw_length_size [in]                                  */
/*                      データ長の出力サイズ（バイト）                      */
/*                  : val [in]                                              */
/*                      出力対象のデータ長（整数値）                        */
/*                                                                          */
/*  RETURN CODE     : E_TRANSCODE_VALID          - 正常終了                 */
/*                  : E_TRANSCODE_BUFFER_ERROR    - 出力先バッファ不足など   */
/*                                                                          */
/*  DESCRIPTION     : データ長部組立て（無変換）                            */
/*                    指定された数値を、出力サイズに従い                    */
/*                    ネットワークバイトオーダーでそのまま出力する          */
/*                                                                          */
/*  NOTE            : 整数が変換元・変換先ともにネットワークバイトオーダー  */
/*                    であることが前提                                       */
/****************************************************************************/

transcoder_error_t BIN_len_encoder(ISO8583object_t * obj,char * raw_length,size_t raw_length_size,size_t val)
{
    unsigned long long buffer = val;
    /// rawdata bufferにデータ長部を書き出せるかチェック
    if(rawdata_buffer_check(obj,raw_length_size))
    {
        return E_TRANSCODE_BUFFER_ERROR;
    }
    /// エンコード対象がraw_length_sizeのバイト数を超えるときエラー
    if(getByteSize(val) > raw_length_size) return E_TRANSCODE_BUFFER_ERROR;
    memmove(raw_length, ((char *)&buffer) + sizeof(unsigned long long) - raw_length_size, raw_length_size);
    return E_TRANSCODE_VALID;
}

/****************************************************************************/
/*  FUNCTION        : BCD_len_encoder                                       */
/*  CALLING SEQ.    : transcoder_error_t BCD_len_encoder                    */
/*                           (ISO8583object_t *obj,                         */
/*                            char *raw_length,                             */
/*                            size_t raw_length_size,                       */
/*                            size_t val)                                   */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : raw_length [out]                                      */
/*                      BCDコードの出力先バッファ                           */
/*                  : raw_length_size [in]                                  */
/*                      出力バッファのサイズ（バイト）                      */
/*                  : val [in]                                              */
/*                      変換対象となる整数値                                */
/*                                                                          */
/*  RETURN CODE     : E_TRANSCODE_VALID          - 正常終了                 */
/*                  : E_TRANSCODE_BUFFER_ERROR    - バッファオーバー検出     */
/*                  : E_TRANSCODE_ERROR           - 出力長不足（変換不可）   */
/*                                                                          */
/*  DESCRIPTION     : データ長組立て（INT → BCD）変換                       */
/*                    指定された整数値をBCD形式に変換し、                   */
/*                    出力バッファに格納する                                */
/****************************************************************************/
transcoder_error_t BCD_len_encoder(ISO8583object_t * obj,char * raw_length,size_t raw_length_size,size_t val)
{
    size_t temp_val = val;
    size_t bcd_byte_len = 0;

    /// rawdata bufferにデータ長部を書き出せるかチェック
    if(rawdata_buffer_check(obj,raw_length_size))
    {
        return E_TRANSCODE_BUFFER_ERROR;
    }

    /// BCD表現の長さを計算します
    while (temp_val > 0) {
        bcd_byte_len++;
        temp_val /= 10;
    }

    /// 各bcd桁には半分のバイトが必要なので、切り上げる必要があります
    bcd_byte_len = (bcd_byte_len + 1) / 2;

    /// BCD変換後データ長さがraw_length_size以下かチェックする。
    if (bcd_byte_len > raw_length_size) {
        return E_TRANSCODE_ERROR;
    }

    /// Temp_valを元の値にリセットします
    temp_val = val;

    /// ゼロで初期化します
    memset(raw_length, 0, raw_length_size);

    /// 6.BCDに変換し、末尾からraw_lengthに保存します
    for (size_t i = 0; i < bcd_byte_len; i++) {
        raw_length[raw_length_size - 1 - i] = (char)((temp_val % 10) | ((temp_val / 10 % 10) << 4));
        temp_val /= 100;
    }

    return E_TRANSCODE_VALID;
}

/****************************************************************************/
/*  FUNCTION        : ASCII_len_encoder                                     */
/*  CALLING SEQ.    : transcoder_error_t ASCII_len_encoder                  */
/*                           (ISO8583object_t *obj,                         */
/*                            char *raw_length,                             */
/*                            size_t raw_length_size,                       */
/*                            size_t val)                                   */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : raw_length [out]                                      */
/*                      ASCIIコードの出力先バッファ                         */
/*                  : raw_length_size [in]                                  */
/*                      出力バッファのサイズ（バイト）                      */
/*                  : val [in]                                              */
/*                      変換対象の整数値                                    */
/*                                                                          */
/*  RETURN CODE     : E_TRANSCODE_VALID          - 正常終了                 */
/*                  : E_TRANSCODE_BUFFER_ERROR    - バッファオーバー検出    */
/*                  : E_TRANSCODE_ERROR           - 桁数オーバー（変換不可）*/
/*                                                                          */
/*  DESCRIPTION     : データ長組立て（INT → ASCII）変換                    */
/*                    指定された整数値をASCIIコードで                       */
/*                    ゼロ埋め右詰に書式化して出力する                      */
/****************************************************************************/
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
transcoder_error_t ASCII_len_encoder(ISO8583object_t * obj,char * raw_length,size_t raw_length_size,size_t val)
{
    size_t number = val;
    size_t number_of_digits = 0;
    char   buffer[21];

    /// rawdata bufferにデータ長部を書き出せるかチェック
    if(rawdata_buffer_check(obj,raw_length_size))
    {
        return E_TRANSCODE_BUFFER_ERROR;
    }
    /// Find a number of digits.
    if(number == 0)
    {
        number_of_digits = 1;
    }
    while(number)
    {
        number /=10;
        number_of_digits++;
    }
    /// Number of digits exceeded.
    if(number_of_digits > raw_length_size)
        return E_TRANSCODE_ERROR;
    /// right justified, raw_length_size digits padded with zero.
    snprintf(buffer, sizeof(buffer), "%0.*ld", raw_length_size, val);
    memmove(raw_length,buffer,raw_length_size);
    return E_TRANSCODE_VALID;
}
#pragma clang diagnostic pop:

/****************************************************************************/
/*  FUNCTION        : EBCDIC_len_encoder                                    */
/*  CALLING SEQ.    : transcoder_error_t EBCDIC_len_encoder                 */
/*                           (ISO8583object_t *obj,                         */
/*                            char *raw_length,                             */
/*                            size_t raw_length_size,                       */
/*                            size_t val)                                   */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : raw_length [out]                                      */
/*                      EBCDICコードの出力先バッファ                        */
/*                  : raw_length_size [in]                                  */
/*                      出力バッファのサイズ（バイト）                      */
/*                  : val [in]                                              */
/*                      変換対象の整数値                                    */
/*                                                                          */
/*  RETURN CODE     : E_TRANSCODE_VALID          - 正常終了                 */
/*                  : E_TRANSCODE_BUFFER_ERROR   - バッファオーバー検出     */
/*                  : E_TRANSCODE_ERROR          - 桁数オーバー（変換不可） */
/*                                                                          */
/*  DESCRIPTION     : データ長組立て（INT → EBCDIC）変換                   */
/*                    指定された整数値をEBCDIC形式で                        */
/*                    ゼロ埋め右詰にフォーマットして出力する                */
/****************************************************************************/
transcoder_error_t EBCDIC_len_encoder(ISO8583object_t * obj,char * raw_length,size_t raw_length_size,size_t val)
{
    size_t number = val;
    size_t number_of_digits = 0;
    char   buffer[20];

    /// rawdata bufferにデータ長部を書き出せるかチェック
    if(rawdata_buffer_check(obj,raw_length_size))
    {
        return E_TRANSCODE_BUFFER_ERROR;
    }
    /// EBCDICコードの0(0x0f)でbufferを初期化する。20はuint64の最大桁数。NULL終端不要
    memset(buffer,0xf0,sizeof(buffer));
    while(number)
    {
        number_of_digits++;
        /// 指定桁数raw_length_sizeを越える場合はエラーを返す。
        if(raw_length_size < number_of_digits) return E_TRANSCODE_ERROR;
        /// EBCDICコードに変換して末尾から格納していく。
        buffer[raw_length_size - number_of_digits ] = (char)(0xF0 | (number % 10));
        number /= 10;
    }
    /// 処理結果が正常ならば返却して正常終了する。
    memmove(raw_length, buffer , raw_length_size);
    return E_TRANSCODE_VALID;
}

/* ################### */
/* # データ部展開用  # */
/* ################### */

/****************************************************************************/
/*  FUNCTION        : BCD_decoder                                           */
/*  CALLING SEQ.    : transcoder_error_t BCD_decoder                        */
/*                           (ISO8583object_t *obj,                         */
/*                            char *dst,                                    */
/*                            size_t dst_len,                               */
/*                            const char *src,                              */
/*                            size_t src_len,                               */
/*                            size_t *read_len)                             */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : dst [out]                                             */
/*                      ASCII変換後の文字列格納先                           */
/*                  : dst_len [in]                                          */
/*                      変換後の出力バッファ長（文字数）                    */
/*                  : src [in]                                              */
/*                      変換元のBCDデータ                                   */
/*                  : src_len [in]                                          */
/*                      BCDの桁数（バイト数ではなく桁数）                   */
/*                  : read_len [out]                                        */
/*                      処理に使用した入力バイト数（= (src_len+1)/2）       */
/*                                                                          */
/*  RETURN CODE     : E_TRANSCODE_VALID            - 正常終了               */
/*                  : E_TRANSCODE_BUFFER_ERROR     - 入力バッファ不足       */
/*                  : E_TRANSCODE_FIX_BUFFER_ERROR - 出力バッファ不足       */
/*                                                                          */
/*  DESCRIPTION     : データ部展開（BCD → ASCII）                          */
/*                    BCDコーディングされたデータをASCIIに変換し            */
/*                    結果をdstに格納する。奇数桁の場合は先頭0を除去。      */
/*                    不正値（0x0～0x9以外）は'*'（0x2a）に置換する。       */
/****************************************************************************/
transcoder_error_t BCD_decoder(ISO8583object_t * obj,char * dst,size_t dst_len,const char * src,size_t src_len,size_t * read_len)
{

    char   buffer[src_len + src_len % 2];
    size_t src_byte_len = (src_len + 1) /2;

    /// rawdata bufferからデータ長部を読み出せるかチェック
    if(rawdata_buffer_check(obj,src_byte_len))
    {
        return E_TRANSCODE_BUFFER_ERROR;
    }

    if(dst_len < src_len) return E_TRANSCODE_FIX_BUFFER_ERROR;

    ///src_lenが奇数桁の場合、ASCII変換後先頭の0を取り除く必要があるため、変換後バッファ末尾から
    ///src_len byte固定フォーマットバッファに書込む
    BCD2CHAR((unsigned char*)src, buffer, (short)src_byte_len);
    memmove(dst,buffer + sizeof(buffer) - src_len ,src_len);
    /// rawdata bufferから読み出したバイト数を返す。
    *read_len = src_byte_len;

    return E_TRANSCODE_VALID;
}

/****************************************************************************/
/*  FUNCTION        : EBCDIC_decoder                                        */
/*  CALLING SEQ.    : transcoder_error_t EBCDIC_decoder                     */
/*                           (ISO8583object_t *obj,                         */
/*                            char *dst,                                    */
/*                            size_t dst_len,                               */
/*                            const char *src,                              */
/*                            size_t src_len,                               */
/*                            size_t *read_len)                             */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : dst [out]                                             */
/*                      EBCDIC→ASCII変換後の文字列格納先                   */
/*                  : dst_len [in]                                          */
/*                      出力バッファ長（バイト）                            */
/*                  : src [in]                                              */
/*                      入力EBCDIC文字列                                    */
/*                  : src_len [in]                                          */
/*                      入力EBCDIC文字列の長さ（バイト）                    */
/*                  : read_len [out]                                        */
/*                      処理されたEBCDICバイト長（src_lenと等しい）         */
/*                                                                          */
/*  RETURN CODE     : E_TRANSCODE_VALID            - 正常終了               */
/*                  : E_TRANSCODE_BUFFER_ERROR     - 入力バッファ不足       */
/*                  : E_TRANSCODE_FIX_BUFFER_ERROR - 出力バッファ不足       */
/*                                                                          */
/*  DESCRIPTION     : データ部展開（EBCDIC → ASCII）                       */
/*                    EBCDIC形式の文字列をASCII（SJIS）に変換し、           */
/*                    dstに格納する。read_lenには処理バイト数を格納する。   */
/****************************************************************************/
transcoder_error_t EBCDIC_decoder(ISO8583object_t * obj,char * dst,size_t dst_len,const char * src,size_t src_len,size_t * read_len)
{
    /// rawdata bufferからデータ長部を読み出せるかチェック
    if(rawdata_buffer_check(obj,src_len))
    {
        return E_TRANSCODE_BUFFER_ERROR;
    }
    if(dst_len < src_len) return E_TRANSCODE_FIX_BUFFER_ERROR;
    EBCDIK2SJIS((char *)src, dst, (short)src_len);
    *read_len = src_len;
    return E_TRANSCODE_VALID;
}

/****************************************************************************/
/*  FUNCTION        : null_decoder                                          */
/*  CALLING SEQ.    : transcoder_error_t null_decoder                       */
/*                           (ISO8583object_t *obj,                         */
/*                            char *dst,                                    */
/*                            size_t dst_len,                               */
/*                            const char *src,                              */
/*                            size_t src_len,                               */
/*                            size_t *read_len)                             */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : dst [out]                                             */
/*                      固定フォーマットデータ格納先バッファ                */
/*                  : dst_len [in]                                          */
/*                      出力バッファ長（バイト）                            */
/*                  : src [in]                                              */
/*                      入力データ（変換対象電文）                          */
/*                  : src_len [in]                                          */
/*                      入力データ長（バイト）                              */
/*                  : read_len [out]                                        */
/*                      処理されたバイト数（src_lenと等しい）               */
/*                                                                          */
/*  RETURN CODE     : E_TRANSCODE_VALID            - 正常終了               */
/*                  : E_TRANSCODE_BUFFER_ERROR     - 入力バッファ不足       */
/*                  : E_TRANSCODE_FIX_BUFFER_ERROR - 出力バッファ不足       */
/*                                                                          */
/*  DESCRIPTION     : データ部展開（変換なし）                              */
/*                    入力データをそのまま固定フォーマットに出力する        */
/****************************************************************************/
transcoder_error_t null_decoder(ISO8583object_t * obj,char * dst,size_t dst_len,const char * src,size_t src_len,size_t * read_len)
{
    /// rawdata bufferからデータ長部を読み出せるかチェック
    if(rawdata_buffer_check(obj,src_len))
    {
        return E_TRANSCODE_BUFFER_ERROR;
    }
    if(dst_len < src_len) return E_TRANSCODE_FIX_BUFFER_ERROR;
    memmove(dst, src, src_len);
    *read_len = src_len;
    return E_TRANSCODE_VALID;
}

/* ################### */
/* # データ部組立用  # */
/* ################### */

/****************************************************************************/
/*  FUNCTION        : BCD_encoder                                           */
/*  CALLING SEQ.    : transcoder_error_t BCD_encoder                        */
/*                           (ISO8583object_t *obj,                         */
/*                            char *dst,                                    */
/*                            size_t dst_len,                               */
/*                            const char *src,                              */
/*                            size_t src_len,                               */
/*                            size_t *write_len)                            */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : dst [out]                                             */
/*                      BCD変換後のデータ格納領域                           */
/*                  : dst_len [in]                                          */
/*                      変換後の桁数（バイト数ではない）                    */
/*                  : src [in]                                              */
/*                      固定フォーマットのASCII文字列                       */
/*                  : src_len [in]                                          */
/*                      ASCII文字列の長さ（バイト）                         */
/*                  : write_len [out]                                       */
/*                      書き込んだバイト数（実際のBCDバイト数）             */
/*                                                                          */
/*  RETURN CODE     : E_TRANSCODE_VALID           - 正常終了                */
/*                  : E_TRANSCODE_ERROR           - 非数字文字を検出        */
/*                  : E_TRANSCODE_BUFFER_ERROR    - 出力バッファ長不足      */
/*                                                                          */
/*  DESCRIPTION     : データ部組立（ASCII → BCD）                          */
/*                    ASCII形式の数値文字列をBCD形式に変換する。            */
/*                    ・前後の空白は無視                                    */
/*                    ・奇数桁の場合は先頭に0を補完して右詰で格納（VISA仕様）*/
/****************************************************************************/
transcoder_error_t BCD_encoder(ISO8583object_t *obj, char *dst, size_t dst_len, const char *src, size_t src_len, size_t *write_len)
{
    // 前後の空白を除去するためのポインタ
    const char *start = src;
    const char *end = src + src_len - 1;

    //dst buffの最大長
    size_t dst_byte_len = (dst_len + 1) / 2;

    /// 前後の空白をスキップ
    while (start <= end && isspace((unsigned char)*start)) start++;
    while (end >= start && isspace((unsigned char)*end)) end--;

    /// 数字以外の文字が含まれているかチェック
    for (const char *p = start; p <= end; p++) {
        if (!isdigit((unsigned char)*p)) {
            return E_TRANSCODE_ERROR; // 数字以外の文字が含まれている場合はエラー
        }
    }

    /// 変換対象の文字列の長さ
    size_t num_len = end - start + 1;

    /// 変換後のデータ長を計算（奇数桁の場合は+1）
    *write_len = (num_len + 1) / 2;

    /// 変換後のデータ長がdst_byte_lenを超える場合はエラー
    if (*write_len > dst_byte_len)
    {
        return E_TRANSCODE_BUFFER_ERROR;
    }
    if(rawdata_buffer_check(obj,*write_len))
    {
        return E_TRANSCODE_BUFFER_ERROR;
    }
    /// BCD変換
    /// 奇数桁の補完処理
    /// 奇数桁の場合右詰めで先頭に0を補完する(VISA仕様)
    if (num_len % 2 != 0) {
        dst[0] = (start[0] - '0');
        start++;
        dst++;
        num_len--;
    }
    for (size_t i=0; i < num_len; i++) {
        unsigned char digit = start[i] - '0';
        if ((i) % 2 == 0) {
            dst[i / 2] = (char)(digit << 4); // 上位4ビットに格納
        } else {
            dst[i / 2] |= digit; // 下位4ビットに格納
        }
    }
    return E_TRANSCODE_VALID;
}

/****************************************************************************/
/*  FUNCTION        : EBCDIC_encoder                                        */
/*  CALLING SEQ.    : transcoder_error_t EBCDIC_encoder                     */
/*                           (ISO8583object_t *obj,                         */
/*                            char *dst,                                    */
/*                            size_t dst_len,                               */
/*                            const char *src,                              */
/*                            size_t src_len,                               */
/*                            size_t *write_len)                            */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : dst [out]                                             */
/*                      変換先のEBCDICコード格納バッファ                    */
/*                  : dst_len [in]                                          */
/*                      出力バッファの長さ（バイト）                        */
/*                  : src [in]                                              */
/*                      入力データ（JIS8/SJIS）                             */
/*                  : src_len [in]                                          */
/*                      入力データの長さ（バイト）                          */
/*                  : write_len [out]                                       */
/*                      実際に書き込んだバイト数（= src_len）               */
/*                                                                          */
/*  RETURN CODE     : E_TRANSCODE_VALID           - 正常終了                */
/*                  : E_TRANSCODE_BUFFER_ERROR    - 出力バッファ長不足      */
/*                                                                          */
/*  DESCRIPTION     : データ部組立（JIS8 → EBCDIC）変換                    */
/*                    入力されたSJIS文字列をEBCDICに変換し、                */
/*                    指定バッファに出力する                                */
/****************************************************************************/
transcoder_error_t EBCDIC_encoder(ISO8583object_t * obj,char * dst,size_t dst_len,const char * src,size_t src_len,size_t * write_len)
{
   /// rawdata bufferにデータ長部を書き出せるかチェック
    if(rawdata_buffer_check(obj,src_len))
    {
        return E_TRANSCODE_BUFFER_ERROR;
    }
    if(dst_len < src_len) return E_TRANSCODE_BUFFER_ERROR;
    SJIS2EBCDIK((char *)src, dst, (short)src_len);
    *write_len = src_len;
    return E_TRANSCODE_VALID;
}

/****************************************************************************/
/*  FUNCTION        : null_encoder                                          */
/*  CALLING SEQ.    : transcoder_error_t null_encoder                       */
/*                           (ISO8583object_t *obj,                         */
/*                            char *dst,                                    */
/*                            size_t dst_len,                               */
/*                            const char *src,                              */
/*                            size_t src_len,                               */
/*                            size_t *write_len)                            */
/*                                                                          */
/*  ARGUMENT        : obj [in]                                              */
/*                      ISO8583object_t構造体ポインタ                       */
/*                  : dst [out]                                             */
/*                      変換先データ格納バッファ                            */
/*                  : dst_len [in]                                          */
/*                      出力バッファの長さ（バイト）                        */
/*                  : src [in]                                              */
/*                      変換元データ                                        */
/*                  : src_len [in]                                          */
/*                      変換元データの長さ（バイト）                        */
/*                  : write_len [out]                                       */
/*                      書き込んだデータ長（バイト）                        */
/*                                                                          */
/*  RETURN CODE     : E_TRANSCODE_VALID           - 正常終了                */
/*                  : E_TRANSCODE_BUFFER_ERROR     - バッファオーバー        */
/*                                                                          */
/*  DESCRIPTION     : データ部組立（変換なし）                              */
/*                    入力データをそのまま出力バッファにコピーする          */
/****************************************************************************/
transcoder_error_t null_encoder(ISO8583object_t * obj,char * dst,size_t dst_len,const char * src,size_t src_len,size_t * write_len)
{
       /// rawdata bufferにデータ長部を書き出せるかチェック
    if(rawdata_buffer_check(obj,src_len))
    {
        return E_TRANSCODE_BUFFER_ERROR;
    }
    if(dst_len < src_len) return E_TRANSCODE_BUFFER_ERROR;
    memmove(dst, src, src_len);
    *write_len = src_len;
    return E_TRANSCODE_VALID;
}

/****************************************************************************/
/*  FUNCTION        : transcode_fault                                       */
/*  CALLING SEQ.    : bool transcode_fault                                  */
/*                           (const char data_area_attribute)               */
/*                                                                          */
/*  ARGUMENT        : data_area_attribute [in]                              */
/*                      データ部/データ長属性                               */
/*                                                                          */
/*  RETURN CODE     : true  - 許可されていない属性（変換不可）              */
/*                  : false - 許可された属性（変換可能）                    */
/*                                                                          */
/*  DESCRIPTION     : データ部/データ長属性が許可された変換対象かを判定する */
/*                    BIN, BCD, ASC, EBC, JIS のいずれかでなければ true を返す */
/****************************************************************************/
bool transcode_fault(const char data_area_attribute)
{
    const char transcode_pool[] = {DEF_DATA_AREA_ATTR_BIN, DEF_DATA_AREA_ATTR_BCD,
                               DEF_DATA_AREA_ATTR_ASC, DEF_DATA_AREA_ATTR_EBC,
                               DEF_DATA_AREA_ATTR_JIS,'\0'};

    return strchr(transcode_pool,data_area_attribute) == NULL ? true : false;
}

/****************************************************************************/
/*  FUNCTION        : nw_id_fault                                           */
/*  CALLING SEQ.    : bool nw_id_fault                                      */
/*                           (const char nw_id)                             */
/*                                                                          */
/*  ARGUMENT        : nw_id [in]                                            */
/*                      チェック対象のネットワーク識別子（1バイト）         */
/*                                                                          */
/*  RETURN CODE     : true  - 無効なネットワーク識別子                      */
/*                  : false - 有効なネットワーク識別子                      */
/*                                                                          */
/*  DESCRIPTION     : ネットワーク識別子が定義済みの識別子プールに含まれるか*/
/*                    を判定する。GFP/VISA/MASTER/AMEX などが対象。         */
/****************************************************************************/
bool nw_id_fault(const char nw_id ) {

    const char nw_id_pool[] = {
        DEF_NW_ID_GFP,  DEF_NW_ID_VISA,     DEF_NW_ID_MASTER,
        DEF_NW_ID_AMEX, DEF_NW_ID_DISCOVER, DEF_NW_ID_JCN,
        DEF_NW_ID_NYCE, DEF_NW_ID_JLink,    DEF_NW_ID_UnionPay,'\0'};
    return strchr(nw_id_pool,nw_id) == NULL ? true : false;
}

/****************************************************************************/
/*  FUNCTION        : query_value_transcoder                                */
/*  CALLING SEQ.    : bool query_value_transcoder                           */
/*                           (const char nw_id,                             */
/*                            const char data_area_attribute,               */
/*                            const char code_change_need,                  */
/*                            transcoder_set_def *transcoder_set)           */
/*                                                                          */
/*  ARGUMENT        : nw_id [in]                                            */
/*                      ネットワーク識別子                                  */
/*                  : data_area_attribute [in]                              */
/*                      データ部の変換種別（BIN/BCD/ASC/EBC/JIS）           */
/*                  : code_change_need [in]                                 */
/*                      コード変換の要否（'0'で変換不要）                   */
/*                  : transcoder_set [out]                                  */
/*                      取得した変換処理メソッド構造体                      */
/*                                                                          */
/*  RETURN CODE     : true  - 正常に取得成功                                */
/*                  : false - 入力値不正または対応なし                      */
/*                                                                          */
/*  DESCRIPTION     : データ部の展開／組立に使用する                        */
/*                    トランスコーダメソッドをネットワーク識別子と         */
/*                    データ属性に基づいて選択・設定する                   */
/*                    コード変換不要であればNULL_TRANSCODERを返す           */
/****************************************************************************/
bool query_value_transcoder(const char nw_id, const char data_area_attribute, const char code_change_need , transcoder_set_def * transcoder_set)
{
    /// 1．コード変換不要 ならば null_transcoder
    if(code_change_need == '0')
    {
        *transcoder_set = NULL_TRANSCODER;
        return true;
    }

    ///2.NW識別子と変換コード種別の範囲チェック
    if (nw_id_fault(nw_id) || transcode_fault(data_area_attribute)) return false;

    switch(data_area_attribute)
    {
        case DEF_DATA_AREA_ATTR_BIN:
        case DEF_DATA_AREA_ATTR_ASC:
        case DEF_DATA_AREA_ATTR_JIS:
            *transcoder_set = NULL_TRANSCODER;
            break;
        case DEF_DATA_AREA_ATTR_BCD:
            *transcoder_set = BCD_TRANSCODER;
            break;
        case DEF_DATA_AREA_ATTR_EBC:
            switch(nw_id)
            {
                case DEF_NW_ID_GFP:
                case DEF_NW_ID_JCN:
                case DEF_NW_ID_AMEX:
                case DEF_NW_ID_DISCOVER:
                case DEF_NW_ID_MASTER:
                case DEF_NW_ID_VISA:
                case DEF_NW_ID_NYCE:
                case DEF_NW_ID_UnionPay:
                case DEF_NW_ID_JLink:
                default:
                    *transcoder_set = EBCDIC_TRANSCODER;
            }
            break;
        default:
            return false;
    }
    return true;
}

/****************************************************************************/
/*  FUNCTION        : query_length_transcoder                               */
/*  CALLING SEQ.    : bool query_length_transcoder                          */
/*                           (const char nw_id,                             */
/*                            const char data_area_attribute,               */
/*                            length_transcoder_set_def *transcoder_set)    */
/*                                                                          */
/*  ARGUMENT        : nw_id [in]                                            */
/*                      ネットワーク識別子                                  */
/*                  : data_area_attribute [in]                              */
/*                      データエリアの属性（BIN1/BCD1/ASC1/EBC1）           */
/*                  : transcoder_set [out]                                  */
/*                      対応するデータ長トランスコーダメソッド構造体        */
/*                                                                          */
/*  RETURN CODE     : true  - メソッド取得成功                              */
/*                  : false - 入力値不正または未対応の属性                  */
/*                                                                          */
/*  DESCRIPTION     : データ長部に対応する組立／展開トランスコーダを選択    */
/*                    指定されたネットワークIDとデータ属性に基づき、        */
/*                    適切な処理メソッドをtranscoder_setに設定する          */
/****************************************************************************/
 #pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
bool query_length_transcoder(const char nw_id, const char data_area_attribute,length_transcoder_set_def * transcoder_set)
{

    ///2.NW識別子と変換コード種別の範囲チェック
    if (nw_id_fault(nw_id) || transcode_fault(data_area_attribute)) return false;

    switch(data_area_attribute)
    {
        case DEF_DATA_LEN_ATTR_BIN1:
            *transcoder_set = BIN_LEN_TRANSCODER;
            break;
        case DEF_DATA_LEN_ATTR_BCD1:
            *transcoder_set = BCD_LEN_TRANSCODER;
            break;
        case DEF_DATA_LEN_ATTR_ASC1:
            *transcoder_set = ASCII_LEN_TRANSCODER;
            break;
        case DEF_DATA_LEN_ATTR_EBC1:
            *transcoder_set = EBCDIC_LEN_TRANSCODER;
            break;
        default:
            return false;
    }
    return true;
}
#pragma clang diagnostic pop
