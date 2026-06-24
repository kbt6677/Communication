/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                               *
 *                                                                             *
 *                               ＜コネクション制御(クライアント)＞            *
 *                                                                             *
 *        VERSION                               :＜1.0.0＞                     *
 *                                                                             *
 *        CREATE DATE                           :＜作成日 2025/03/01＞         *
 *        CODED                                 :＜ISYS＞                      *
 *                                                                             *
 *        MODIFY DATE                           :＜修正日 yyyy／mm／dd＞       *
 *        CODED                                 :＜修正者＞                    *
 *                                              :＜修正概要＞                  *
 ******************************************************************************/
/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP制御                                     */
/*        PROGRAM-ID        ････ GFPCVX20                                    */
/*        FUNCTION          ････ コネクション制御(クライアント)              */
/*                                                                           */
/*        AUTHOR            ････ ISYS Kudo                                   */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025/03/01                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤   2025/03/01 新規作成                                     */
/****************************************************************************/
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <ctype.h>  nolist
#include <errno.h>  nolist
#include <limits.h> nolist
#include <stdbool.h> nolist
#include <stddef.h> nolist
#include <stdio.h>  nolist
#include <string.h> nolist
#ifdef _TANDEM_SOURCE
#include <tal.h> nolist
#include <cextdecs.h(TS_UNIQUE_CONVERT_TO_JULIAN_,CONVERTTIMESTAMP,INTERPRETTIMESTAMP)> nolist
#else
#include <cextdecs.h> nolist
#include <tal.h> nolist
#endif
/* USER HEADER     */
#include "GFPCVX20_util.h" nolist

/*****************************************************************************/
/*  FUNCTION        :str2ul_c                                                */
/*  CALLING SEQ.    :long str2ul_c(const char *num_str, size_t len,          */
/*                   size_t *r_len, short *s_err)                            */
/*  ARGUMENT        :num_str :数字文字列                                     */
/*                  :len     :文字列の長さ                                   */
/*                  :r_len   :実際に読み取った文字数を格納                   */
/*                  :s_err   :エラー状態を返すポインタ                       */
/*  RETURN CODE     :変換した数値, LONG_MAX(エラー時)                        */
/*  DESCRIPTION     :文字列をlongに変換し、数値とエラー情報を返す            */
/*****************************************************************************/
long str2ul_c(const char *num_str, size_t len, size_t *r_len, short *s_err)
{
    long val = 0;
    bool ws_flg = false;
    *s_err = 0;
    *r_len = 0;
    if (len > 10) {
        *s_err = ERANGE;
        return LONG_MAX;
    }
    while (len--) {
        if (isspace(*num_str) && !ws_flg) {
            ;
        } else {
            if (isspace(*num_str) && ws_flg) break;
            if (!isdigit(*num_str)) {
                if (!isspace(*num_str)) {
                    *s_err = EINVAL;
                }
                return val;
            }
            val    = val * 10 + *num_str - '0';
            ws_flg = true;
        }
        num_str++;
        *r_len += 1;
    }
    return val;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_rm_trspc_wn                                        */
/*  CALLING SEQ.    :void cncl_rm_trspc_wn(char *dt, int len)                */
/*  ARGUMENT        :dt :文字列バッファ                                      */
/*                  :len:バッファ長                                          */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :文字列末尾の空白とヌル終端を取り除く                    */
/*****************************************************************************/
void cncl_rm_trspc_wn(char *dt, int len)
{
    int ix;
    for (ix = len; ix > 0; ix--) {
        if (isspace(dt[ix - 1]) || dt[ix - 1] == 0x00) {
            dt[ix - 1] = 0x00;
        } else
            return;
    }
}
/*****************************************************************************/
/*  FUNCTION        :cobolization                                            */
/*  CALLING SEQ.    :void cobolization(char *dst, const char *src,           */
/*                   size_t dst_length)                                      */
/*  ARGUMENT        :dst       :出力先バッファ                               */
/*                  :src       :入力文字列                                   */
/*                  :dst_length:出力先バッファのサイズ                       */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :srcをCOBOL的に右寄せ・空白埋めしてdstに格納             */
/*****************************************************************************/
void cobolization(char *dst, const char *src, size_t dst_length)
{
    size_t src_len = strlen(src);
    memset(dst, ' ', dst_length);
    memmove(dst, src, src_len > dst_length ? dst_length : src_len);
}
/*****************************************************************************/
/*  FUNCTION        :inter_pret_ts_unique                                    */
/*  CALLING SEQ.    :void inter_pret_ts_unique(ts_128_t ts_128,              */
/*                   datetime20_t datetime20, datetime_hex_t datetime_hex)   */
/*  ARGUMENT        :ts_128      :128bitタイムスタンプ                       */
/*                  :datetime20  :日付時刻文字列(出力用)                     */
/*                  :datetime_hex:16進数文字列(出力用)                       */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :ts_128をユリウス暦に変換し、日時文字列とHEX文字列を返す */
/*                  :                                                        */
/*****************************************************************************/
void inter_pret_ts_unique(ts_128_t ts_128, datetime20_t datetime20, datetime_hex_t datetime_hex)
{
    short     date_and_time[8];
    long long julian;
    long long lct;
    /* ユリウス暦タイムスタンプ(64bit)の取得 */
    TS_UNIQUE_CONVERT_TO_JULIAN_(ts_128, &julian);
    /* 日本時間(LCT)へ変換 */
    lct = CONVERTTIMESTAMP(julian, no_param, no_param, no_param);
    /* グレゴリオ日時形式に変換 */
    INTERPRETTIMESTAMP(lct, (short _near *)date_and_time);
    /*グレゴリオ日時形式を第1アーギュメント・日付に テキスト形式(20byte)で設定 */
    snprintf(datetime20, sizeof(datetime20_t), "%04d%02d%02d%02d%02d%02d%03d%03d", date_and_time[0], date_and_time[1],
             date_and_time[2], date_and_time[3], date_and_time[4], date_and_time[5], date_and_time[6],
             date_and_time[7]);
    /* ユニークタイムスタンプ（後半64bit)を16進数文字列に変換し
                                   第2アーギュメント・タイスタンプに設定 */
    snprintf((char *)datetime_hex, sizeof(datetime_hex_t), "%04x%04x%04x%04x", ts_128[4], ts_128[5], ts_128[6],
             ts_128[7]);
}
/*****************************************************************************/
/*  FUNCTION        :hextoint                                                */
/*  CALLING SEQ.    :char hextoint(int c)                                    */
/*  ARGUMENT        :c:変換対象の文字                                        */
/*  RETURN CODE     :0~15(16進数変換), -1(非16進文字時)                     */
/*  DESCRIPTION     :文字を16進数として解釈し数値化する                      */
/*****************************************************************************/
int hextoint(int c)
{
    if (isdigit(c)) return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
/*****************************************************************************/
/*  FUNCTION        :hexbin_encoder                                          */
/*  CALLING SEQ.    :int hexbin_encoder(char *dst, size_t dst_len,           */
/*                   const char *src, size_t src_len, size_t *write_len)     */
/*  ARGUMENT        :dst      :変換後のバイナリ格納先                        */
/*                  :dst_len  :dstのバッファサイズ                           */
/*                  :src      :元の16進文字列                                */
/*                  :src_len  :16進文字列の長さ                              */
/*                  :write_len:出力データのバイト数                          */
/*  RETURN CODE     :0(成功),EINVAL,ERANGE等(エラー)                         */
/*  DESCRIPTION     :16進文字列をバイナリに変換し、dstに書き込む             */
/*  UPDATE          :2025/03/17    refs #745                                 */
/*****************************************************************************/
int hexbin_encoder(char *dst, size_t dst_len, const char *src, size_t src_len, size_t *write_len)
{
    // 前後の空白を除去するためのポインタ
    const char *start        = src;
    const char *end          = src + src_len - 1;

    /// 前後の空白をスキップ
    while (start <= end && isspace((unsigned char)*start)) start++;
    while (end >= start && isspace((unsigned char)*end)) end--;

    /// 数字以外の文字が含まれているかチェック
    for (const char *p = start; p <= end; p++) {
        if (!isxdigit((unsigned char)*p)) {
            return EINVAL;  // 数字以外の文字が含まれている場合はエラー
        }
    }

    /// 変換対象の文字列の長さ
    size_t num_len = end - start + 1;

    /// 変換後のデータ長を計算（奇数桁の場合は+1）
    *write_len     = (num_len + 1) / 2;

    /// 変換後のデータ長がdst_byte_lenを超える場合はエラー
    if (*write_len > dst_len) {
        return ERANGE;
    }
    /// 16進数変換
    /// 奇数桁の補完処理
    /// 奇数桁の場合右詰めで先頭に0を補完する(VISA仕様)
    if (num_len % 2 != 0) {
        dst[0] = (char)hextoint(start[0]);
        start++;
        dst++;
        num_len--;
    }
    for (size_t i = 0; i < num_len; i++) {
        int digit = hextoint(start[i]);
        if (digit == -1) {
            // 有効な16進文字でなければエラー
            return EINVAL;
        }
        if ((i) % 2 == 0) {
            dst[i / 2] = (char)(digit << 4);  // 上位4ビットに格納
        } else {
            dst[i / 2] |= (char)digit;        // 下位4ビットに格納
        }
    }
    return 0;
}

size_t strnlen_isys(const char *s, size_t maxlen)
{
   const char *end = (const char *)memchr(s, '\0', maxlen);
   return end ? (size_t)(end - s) : maxlen;
}
/*****************************************************************************/
/*  FUNCTION        :binhex_encoder                                          */
/*  CALLING SEQ.    :void binhex_encoder(                                    */
/*                   const char *bin, size_t bin_len,                        */
/*                   char *hex_str, size_t hex_str_len)                      */
/*  ARGUMENT        :bin       : バイナリデータ                              */
/*                   bin_len   : バイナリデータのバイト長                    */
/*                   hex_str   : 16進数文字列の格納先                        */
/*                   hex_str_len: 格納先バッファ長(NULL終端分を含む)         */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :バイナリデータを16進数(大文字表記)文字列に変換し、      */
/*                   hex_strに格納する。                                     */
/*                   ・出力は必ずNULL終端される                              */
/*                   ・格納先バッファが不足する場合は収まる範囲まで出力      */
/*****************************************************************************/
void binhex_encoder(const char *bin, size_t bin_len,
                    char *hex_str, size_t hex_str_len)
{
    static const char HEX[16] = "0123456789ABCDEF";

    if (hex_str_len == 0) {
        // 出力先に何も書けない
        return;
    }

    size_t out_cap = hex_str_len - 1; // NUL 終端ぶんを除いた最大文字数
    size_t o = 0;                     // 出力位置
    size_t i = 0;                     // 入力位置

    // 2文字（=1バイトぶん）がまだ入る間だけ書く
    while (i < bin_len && (o + 2) <= out_cap) {
        unsigned char b = (unsigned char)bin[i++];
        hex_str[o++] = HEX[b >> 4];
        hex_str[o++] = HEX[b & 0x0F];
    }

    // かならず NUL 終端
    hex_str[o] = '\0';
}
