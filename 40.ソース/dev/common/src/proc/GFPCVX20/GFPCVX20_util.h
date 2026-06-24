#pragma once
/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                               *
 *                                                                             *
 *                               ＜コネクション制御(クライアント)＞            *
 *                                                                             *
 *        VERSION                               :＜1.0.0＞                     *
 *                                                                             *
 *        CREATE DATE                           :＜作成日 2025/02/04＞         *
 *        CODED                                 :＜ISYS＞                      *
 *                                                                             *
 *        MODIFY DATE                           :＜修正日 yyyy／mm／dd＞       *
 *        CODED                                 :＜修正者＞                    *
 *                                              :＜修正概要＞                  *
 ******************************************************************************/
/*****************************************************************************/
/*****                    <<     head PROGRAM      >>                    *****/
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
/*        WRITTEN-DATE      ････ 2025/02/04                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤   2025/02/04 新規作成                                     */
/****************************************************************************/
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <stdbool.h> nolist
#include <stddef.h> nolist
/* USER HEADER     */

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#ifdef _TANDEM_SOURCE
#define DUMMY
#else
#define DUMMY 0
#endif
#define no_param DUMMY

/****************************************************************************/
/*   外部データ定義                                                         */
/****************************************************************************/
typedef short ts_128_t[16];
typedef char  datetime20_t[20 + 1];
typedef char  datetime_hex_t[16 + 1];

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

/****************************************************************************/
/*   関数定義                                                               */
/****************************************************************************/
long str2ul_c(const char *num_str, size_t len, size_t *r_len, short *s_err);
void cncl_rm_trspc_wn(char *dt, int len);
void cobolization(char *dst, const char *src, size_t dst_length);
void inter_pret_ts_unique(ts_128_t ts, datetime20_t datetime20, datetime_hex_t datetime_hex);
int hexbin_encoder(char *dst, size_t dst_len, const char *src, size_t src_len, size_t *write_len);

/* 文字列が '0'～'9' のみで構成されているかチェック */
bool isAllDigits(const char *str, int length);

/* 文字列がヘキサ文字(0-9, A-F, a-f)のみで構成されているかチェック */
bool isAllHex(const char *str, int length);

/*
 * 数値の範囲チェック
 *  - 長さ length分の文字列を数値として解釈し、[minVal, maxVal] の範囲かどうかを確認
 *  - 全部スペースの場合は許容するかなどは呼び出し元で要件に応じて実装を切り替える
 */
bool checkDecimalRange(const char *str, int length, int minVal, int maxVal, bool allowSpace);

/*
 * IPv4アドレス形式チェック
 *   - 例: "192.168.0.1", "127.0.0.1"
 *   - 全部スペースの場合は許容するかどうかは要件次第
 */
bool checkIPv4Address(const char *str, int length, bool allowEmpty);

/*
 * 更新日付 "YYYYMMDD" 形式のチェック
 *   - 簡易的に「数字8桁」かどうかをチェックする
 *   - 実際にカレンダー上有効か(例: 20230229 は本来2月29日ではない)などは必要に応じて別途実装
 */
bool checkDateYYYYMMDD(const char *str);

/*
 * 先頭が特定文字かどうか、後続が英数字のみかどうか 等のチェック
 *  例: 先頭'$'で始まり、以降は英数字
 */
bool checkStartCharAndAlnum(const char *str, int length, char startChar);

/*
 * 指定文字列が候補リストのいずれかに合致するか確認
 *  (例) "JC","VI","BA","AE","NY","UN","JL","DI"など
 */
bool checkStringInList(const char *str, int length, const char *candidateList[], int candidateCount);

/*
 * 文字列が英字で始まり、その後ろが英数字のみ、等のチェック (可変長)
 */
bool checkAlphabetStartAndAlnumBody(const char *str, int length, int minLen, int maxLen);

/*
 * 数値 or スペース しか許可しないなどの場合
 * (port_num_src はスペースまたは0～65535 など)
 */
bool checkPortSpec(const char *str, int length, bool allowSpace);

/* 1以上の数字かどうか */
bool checkNumericGreaterEqual1(const char *str, int length);

/*
 * spc_data 等、HEX or スペース(パディング) を想定する場合
 * 左詰/右詰の管理は場合によっては呼び出し元がやる必要あり
 */
bool checkSpcOrHex(const char *str, int length);

/*
 * 1文字が指定のリスト('C','I','S',' ')のいずれかかどうか
 */
bool checkCharInSet(char c, const char *set);

size_t strnlen_isys(const char *s, size_t maxlen);

void binhex_encoder(const char *bin, size_t bin_len,
                    char *hex_str, size_t hex_str_len);
