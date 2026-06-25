/* GFPOGGZ3 データ変換モジュール */
#ifndef GFPOGGZ3_H
#define GFPOGGZ3_H

/* -------------------------------------------------------*/
/* 関数プロトタイプ宣言                                   */
/* -------------------------------------------------------*/

// 文字コード変換 EBCDIC ⇒ SJIS
void EBCDIC2SJIS(char *source_p, char *target_p, short source_len);

// 文字コード変換 EBCDICカナ ⇒ SJIS
void EBCDICKANA2SJIS(char *source_p, char *target_p, short source_len);

// 文字コード変換 EBCDIK ⇒ SJIS
void EBCDIK2SJIS(char *source_p, char *target_p, short source_len);

// 文字コード変換 SJIS ⇒ EBCDIC
void SJIS2EBCDIC(char *source_p, char *target_p, short source_len);

// 文字コード変換 SJIS ⇒ EBCDICカナ
void SJIS2EBCDICKANA(char *source_p, char *target_p, short source_len);

// 文字コード変換 SJIS ⇒ EBCDIK
void SJIS2EBCDIK(char *source_p, char *target_p, short source_len);

// データ変換 10進数 BCD ⇒ キャラクタ
void BCD2CHAR(unsigned char *source_p, char *target_p, short source_len);

// データ変換 10進数 キャラクタ ⇒ BCD
short CHAR2BCD(unsigned char *source_p, short *target_p);

// データ変換 16進数 Binary ⇒ キャラクタ
void HEX2CHAR(unsigned char *source_p, char *target_p, short source_len);

// データ変換 16進数 キャラクタ ⇒ Binary
short CHAR2HEX(const char *source_p, char *target_p, short source_len);

#endif
