# EMS解析レポート：[ECCE007] 共通モジュールエラー / NWM_ENC エラーコード=00009

## 0. 対象ログ

```
26-06-23 18:47:29 \CBX1.$TJG1      *TIS-JPN.3.V01         030810 E30810 99
                                    $E8AA1   ........ E 26/06/23 18:47:29:46
                                    GFP , COM NW=CA/    GFPCVX30 , $LCAD6
                                    [ECCE007]         ｷｮｳﾂｳﾓｼﾞｭｰﾙｴﾗｰ
                                    ｻｰﾊﾞｰｸﾗｽID=SCNMSDSI ﾓｼﾞｭｰﾙ=NWM_ENC
                                    ｴﾗｰｺｰﾄﾞ=00009
```

---

## 1. 結論（先に要点）

**受信電文（CARDNET/NW=CA、inbound）の認証（MAC）処理で、電文に埋め込まれた
KMAC（メッセージ認証鍵）のチェックディジットが、自局の鍵管理ファイル(GCKEY)に
登録されている KMAC 鍵のチェックディジット（現用 `key_info_01` / 旧 `key_info_02`
のどちら）とも一致しなかった**ため、復号・認証モジュール `NWM_ENC` が
リターンコード `9 (DEF_NWM_ENC_RTN_NG_DIGITS_KMAC = チェックディジットエラー(KMAC))`
を返し、電文振分(inbound)サーバ `GFPCVX30 (SCNMSDSI)` が
内部エラーコード `ECCE007（電文認証値判定エラー(KMAC)）` として EMS 出力したものです。

ポイントは、**「MAC値そのものの照合に失敗した（改ざん）」のではなく、その手前の
『どの鍵で照合すべきか』を特定する段階（鍵のチェックディジット照合）で外れている**
点です。実態は **送信側／受信側の KMAC 鍵の不一致（鍵の世代ずれ・鍵交換の取りこぼし・
鍵管理ファイルの登録誤り）**、または **電文ヘッダのチェックディジット領域の破損** が
最有力です。

---

## 2. ログ各フィールドの意味（デコード）

| ログ表記 | 意味 | 補足 |
|---|---|---|
| `\CBX1.$TJG1` | NonStop ノード名 `\CBX1` / プロセス `$TJG1` | EMS 収集・出力経路 |
| `*TIS-JPN.3.V01` | サブシステムID(SSID)／版数 | |
| `GFP , COM NW=CA/` | GFP通信制御サブシステム、共通(COM)、ネットワーク種別 **CA(CARDNET)** | |
| `GFPCVX30 , $LCAD6` | 出力プログラム `GFPCVX30`、プロセス名 `$LCAD6` | GFPCVX30 = 電文振分(inbound) 本体 |
| `[ECCE007]` | 内部エラーコード = `DEF_NERR_KMAC_HANTE_ERR`「電文認証値判定エラー(KMAC)」 | `errcd.h:89` |
| `ｷｮｳﾂｳﾓｼﾞｭｰﾙｴﾗｰ` | 「共通モジュールエラー」= イベント `DEF_EVT_COMMON_MOD_ERR (29230)` | `ems.h:78` |
| `ｻｰﾊﾞｰｸﾗｽID=SCNMSDSI` | サーバクラス `SCNMSDSI` = `DEF_SC_FURI_I`「電文振分(inbound)」 | `common.h:485` |
| `ﾓｼﾞｭｰﾙ=NWM_ENC` | 呼び出した共通モジュール = 暗号化・復号／認証モジュール | |
| `ｴﾗｰｺｰﾄﾞ=00009` | NWM_ENC のリターンコード `9` | 下記参照 |

> 補足：先頭行の `030810 / E30810 / 99` は NonStop EMS 側で採番されるイベント番号
> （EMSメッセージテンプレート側の採番）で、アプリ内部定数 `29230` とは別系統です。
> 本ソース一式（Cソース＋DB用DDL）にはEMSメッセージカタログ本体は含まれないため、
> 番号の対応はカタログ側で管理されています。判定にはアプリ側ロジックで十分です。

---

## 3. エラーコード `9` の定義

`dev/common/include/nwlib/NWM_ENC.h`

```c
#define  DEF_NWM_ENC_RTN_OK              0   /* 正常                              */
#define  DEF_NWM_ENC_RTN_OK_NONE         1   /* 正常(対象外/変換なし)             */
#define  DEF_NWM_ENC_RTN_NG_AUTHORI      2   /* 認証値チェックエラー(MAC不一致)   */
#define  DEF_NWM_ENC_RTN_NG_DIGITS       3   /* チェックディジットエラー          */
#define  DEF_NWM_ENC_RTN_NG_ATALLA       4   /* ATALLA(HSM)レスポンスエラー       */
#define  DEF_NWM_ENC_RTN_NG_IO           5   /* PATHSEND/ファイルIOエラー         */
#define  DEF_NWM_ENC_RTN_NG_PARAM        6   /* パラメータエラー                  */
#define  DEF_NWM_ENC_RTN_NG_KMAC         7   /* GCKEY KMAC エラー                 */
#define  DEF_NWM_ENC_RTN_NG_KC           8   /* GCKEY KC エラー                   */
#define  DEF_NWM_ENC_RTN_NG_DIGITS_KMAC  9   /* ★チェックディジットエラー(KMAC)  */
#define  DEF_NWM_ENC_RTN_NG_DIGITS_KC   10   /* チェックディジットエラー(KC)      */
```

---

## 4. 発生箇所と原因ロジック

### 4-1. 呼び出し元：`GFPCVX30`（電文振分inbound, SCNMSDSI）の復号処理

`dev/common/src/proc/GFPCVX30/GFPCVX30.c` `MDSI_decode()`（受信電文の復号・認証）

```c
s_ret = NWM_ENC(MDSI_DECODE, &arg2, &arg3, &arg4, &arg5, ch_module_id);   /* L3184 */
if ( s_ret >= 2 ) {
    switch(s_ret){
    case DEF_NWM_ENC_RTN_NG_AUTHORI:      /* 2  */ → ECCE007 (KMAC判定エラー)
    case DEF_NWM_ENC_RTN_NG_DIGITS_KMAC:  /* 9  */ → ECCE007 (KMAC判定エラー)   ★本件
    case DEF_NWM_ENC_RTN_NG_DIGITS_KC:    /* 10 */ → ECCE006 (KC復号エラー)
    default:                              /* 4,5等 */ → ECCE012 (電文復号処理エラー)
    }
    /* 共通モジュールエラーとしてEMS出力（@X=モジュール名, @E=リターンコード） */
    MDSI_message_output(DEF_EVT_COMMON_MOD_ERR, g_internal_error_code,
                        "@X@E", "NWM_ENC", s_ret);                          /* L3218 */
    return MDSI_RET_NG;
}
```

※ リターンコード `2` と `9` は **どちらも `ECCE007`** にマップされます。今回ログの
`エラーコード=00009` から、本件は **`9`（チェックディジット不一致）側**と特定できます。

### 4-2. 発生元：`NWM_ENC` 本体 `NWM_ENC_dec()`

`dev/nw_CA/src/nwlib/GFPCSJ10/GFPCSJ10.c`（NW個別モジュール：暗号化・復号(CARDNET)）

```c
/* 認証チェック対象の判定（電文にKMACチェックディジットがある場合のみ実施） */
if ((memcmp(before_msg_ptr->bh_chk_digit.bh_chk_digit_kmac, null_check_digit, 2) != 0)) {

    /* 鍵管理ファイルから KMAC 鍵レコードを取得 */
    err = NWM_ENC_gckey_read(DEF_NWM_ENC_KEY_KMAC, ...);
    if (err != 0) return(DEF_NWM_ENC_RTN_NG_IO);

    /* 受信電文中の KMAC チェックディジットを 4桁HEX 文字列化 */
    wk_short = (unsigned short *)&before_msg_ptr->bh_chk_digit.bh_chk_digit_kmac[0];
    sprintf(check_digit_kmac, "%04X", *wk_short);

    /* 鍵管理ファイルの現用鍵(key_info_01) / 旧鍵(key_info_02) のチェックディジットと照合 */
    if (memcmp(check_digit_kmac, gckey_o->key_info.key_info_01.check_digit, 4) == 0) {
        ...現用鍵を採用...
    } else {
        if (memcmp(check_digit_kmac, gckey_o->key_info.key_info_02.check_digit, 4) == 0) {
            ...旧鍵を採用...
        } else {
            return(DEF_NWM_ENC_RTN_NG_DIGITS_KMAC);   /* ★ ここで 9 を返す（L540） */
        }
    }
    /* ↑を通過した場合のみ、ATALLA(HSM)で実際のMAC値照合を実施 */
    /*   MAC値不一致なら return DEF_NWM_ENC_RTN_NG_AUTHORI (=2) */
}
```

つまり **コード `9` は、受信電文が名乗っている KMAC 鍵（チェックディジット）が、
自局の鍵管理ファイルに登録された KMAC 鍵2世代のいずれとも一致せず、
「照合に使う鍵を特定できなかった」段階のエラー**です。
ATALLA(HSM) での MAC 値照合そのものには到達していません。

### 4-3. 鍵管理ファイル(GCKEY)の鍵2世代

`dev/common/include/file.h` `db_gckey_def`

```c
struct { char key_value[74]; char check_digit[16]; char key_update_time[14]; ... } key_info_01; /* 現用 */
struct { char key_value[74]; char check_digit[16]; char key_update_time[14]; ... } key_info_02; /* 旧/次 */
```

鍵交換（サーバクラス `SCNCSKYX` 鍵交換制御）の移行期に新旧2世代を保持する設計のため、
照合は `key_info_01`（現用）→ `key_info_02`（もう一方）の順で行われます。

---

## 5. コード `2`（AUTHORI）との違い ＝ 切り分けの肝

| | コード 9（本件 DIGITS_KMAC） | コード 2（AUTHORI） |
|---|---|---|
| 失敗段階 | 鍵の特定（チェックディジット照合） | ATALLA(HSM)によるMAC値照合 |
| 意味 | 受信電文の鍵世代が自局の登録鍵と不一致 | 鍵は特定できたがMAC値が合致しない |
| 主因 | **鍵の世代ずれ／鍵交換不整合／GCKEY登録誤り／チェックディジット領域破損** | 電文本文の改ざん・破損、MAC算出条件相違 |
| EMSコード | ECCE007 | ECCE007（同じ） |

両者とも `ECCE007` で出力されるため、**`エラーコード=00009` か `00002` かで切り分ける**
必要があります。本件は `9` なので **鍵不一致系**です。

---

## 6. 想定原因と確認観点

発生可能性が高い順：

1. **送信側と受信側の KMAC 鍵の不一致（最有力）**
   - 鍵交換（KEY EXCHANGE）が片側のみ完了／取りこぼし、世代がずれている。
   - → 鍵管理ファイル(GCKEY)の該当局(`site/nw/grp/if/station`)レコードの
     `key_info_01 / key_info_02` の `check_digit` と、相手局が実際に使用した鍵の
     チェックディジットを突合。
2. **鍵管理ファイル(GCKEY)の登録内容誤り・未更新**
   - 直近の鍵更新時刻 `key_update_time` を確認。鍵更新運用の直後であれば移行不整合を疑う。
3. **対象局の鍵設定漏れ**
   - 当該 `interface_name / station_name`（`arg4`：`if_id` / `station_id`）に対する
     KMAC 鍵レコードが想定どおり登録されているか。
4. **受信電文ヘッダの破損・電文種別不整合**
   - `bh_chk_digit.bh_chk_digit_kmac` 領域が壊れている／本来 0x0000（認証対象外）で
     あるべき電文に値が入っている等。CARDNETヘッダ(80byte)のレイアウト相違も含む。

### 確認に有用なログ・データ
- 同時刻帯の `NW=CA` の他EMS、特に **鍵交換（KEY EXCHANGE / `SCNCSKYX`）** 関連イベント。
- 当該 **局 (station_id)** の電文ログ（`MDSI_log_output` 出力）と、ヘッダ80byteのダンプ。
- 鍵管理ファイル GCKEY の該当キー（site/nw/grp/if/station, key_type=KMAC）の
  `key_info_01.check_digit` / `key_info_02.check_digit` / `key_update_time`。

---

## 7. 関連改修履歴（参考）

`GFPCSJ10.c` 改版履歴に以下があり、認証値チェック周りに既知の不具合対応があります。
切り分け時は適用版数の確認を推奨します。

```
1.0  2024/10/01 (J0680) 新規作成
1.1  2025/04/22 認証値チェック不正対応（不具合No.57）   ← 認証(MAC)系の修正
1.2  2025/06/22 初期化処理(NWM_ENI) 分離
```

---

## 8. Pathway構成での裏付け（追加資料 `コンフィグ・オベイ_トレース0/`）

受領したPathway(PATHCOM)構成オベイファイルにより、本EMSログの出力主体が
ソース `GFPCVX30.c` であることが裏付けられます。

`コンフィグ・オベイ_トレース0/CCA11SKL`（NW=CA, グループG0001）抜粋：

```
SET   SERVER PARAM  PM-MY-SERVERCLASS-NAME "SCNMSDSI"        [* 電文振分(inbound) *]
SET   SERVER PARAM  SRV-LOGICAL-ID         "E-C-G0001-SCNMSDSI-0000"
SET   SERVER PARAM  MSG-MON-NAME           "$E8AMN"
SET   SERVER PROGRAM $GFPL01.ACOML.GFPCVJ30
ADD   SERVER SCNMSDSI
```

- **サーバクラス `SCNMSDSI`（電文振分inbound）の実体プログラムは `GFPCVJ30`** です。
  `GFPCVJ30` は共通ソース `GFPCVX30.c` を NW=CA(CARDNET) 向けにビルドしたもの
  （命名規則：共通の `X` がネットワーク識別子 `J(=CA)` に置換）。CA向けには
  CA個別の `NWM_ENC`（`dev/nw_CA/src/nwlib/GFPCSJ10`）がリンクされます。
  → EMSログの `GFP , COM NW=CA/ GFPCVX30`（共通ソース名で出力）と整合。
- `MSG-MON-NAME "$E8AMN"` 等の `$E8A*` 系プロセス名は、EMSログの `$E8AA1` /
  `$LCAD6` と同一サブシステム系統。
- 同様の SCNMSDSI 定義は `CCA12SKL`（G0001冗長）, `CCA21SKL`/`CCA22SKL`（G0002）
  にも存在し、東/西・グループ単位で多重化されています。

つまり、**「NW=CA の電文振分inbound（SCNMSDSI=GFPCVJ30）が、受信電文の復号・認証で
共通モジュール NWM_ENC を呼び、KMACチェックディジット不一致(9)で ECCE007 を出力した」**
という第1〜7章の結論が、実運用構成からも確認できます。

> 補足：本構成一式は PATHCOM の OBEY コマンドファイル（`SKL`=サーバ定義,
> `BEG`/`END`/`STA`=起動・停止・状態, `*cmd`=東西別コマンド 等）で、計216ファイル。
> ATALLA(HSM)や鍵管理ファイル(GCKEY)の所在はGFP個別パラメータ／別サーバ経由
> (PATHSEND)で解決されるため、本SCNMSDSIサーバ定義内には直接の鍵ファイルASSIGNはありません。

---

## 9. 参照ファイル一覧（本リポジトリ内）

| 役割 | パス |
|---|---|
| 出力元プロセス（電文振分inbound/SCNMSDSI） | `40.ソース/dev/common/src/proc/GFPCVX30/GFPCVX30.c` |
| NWM_ENC 本体（暗号化・復号/認証, CARDNET） | `40.ソース/dev/nw_CA/src/nwlib/GFPCSJ10/GFPCSJ10.c` |
| NWM_ENC リターンコード定義 | `40.ソース/dev/common/include/nwlib/NWM_ENC.h` |
| エラーコード定義(ECCE007) | `40.ソース/dev/common/include/errcd.h` |
| サーバクラス定義(SCNMSDSI) | `40.ソース/dev/common/include/common.h` |
| EMSイベント定義(共通モジュールエラー) | `40.ソース/dev/common/include/ems.h` |
| 鍵管理ファイル構造(GCKEY) | `40.ソース/dev/common/include/file.h` |
| Pathway構成(SCNMSDSI=GFPCVJ30 定義, NW=CA) | `コンフィグ・オベイ_トレース0/CCA11SKL` 他 |
