/******************************************************************************
*                                                                             *
*                               ＜GFP通信制御＞                               *
*                                                                             *
*                         ＜共通(ビットマップ展開・組立て)＞                  *
*                                                                             *
*        VERSION                               :＜1.1.0＞                     *
*                                                                             *
*        CREATE DATE                           :＜作成日 2024-10-31＞         *
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
/*        WRITTEN-DATE      ････ 2024-10-31                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤  2024-10-31 新規作成                                      */
/*  1.1  ISYS 工藤  2025/06/09 CM-040(PCIPIN対応)                            */
/****************************************************************************/

/* ################### */
/* # INCLUDE         # */
/* ################### */
/* STANDARD HEADER */
#include <GFPCGXB0.h> nolist
#include <assert.h> nolist
#include <limits.h> nolist
#include <ctype.h>  nolist
#include <errno.h>  nolist
#include <stdio.h>  nolist
#include <stdlib.h> nolist
#include <string.h> nolist
#include <stdint.h> nolist
/* USER HEADER */
#define __LIBRARY_COMPILE
#include <GFPCGX20.h> nolist  // public header
#include <GFPCGX21.h> nolist  // impliment header.
#include <GFPCGXB0.h> nolist  // IOM(共通I/Oモジュール)
#ifdef _TANDEM_SOURCE
#include <file.h(db_gfeli)> nolist      // DDL
#else
#ifdef __MAKE_DEPEND__
#include <file.h> nolist      // DDL
#else
#include <gfeli.h>
#endif
#endif

/****************************************************************************/
/*   定数定義                                                               */
/****************************************************************************/

#define DEF_COM_BTM_TRACE_IO_OPEN                 "OPEN"
#define DEF_COM_BTM_TRACE_IO_START                "START"
#define DEF_COM_BTM_TRACE_IO_READ                 "READ"
#define DEF_COM_BTM_TRACE_IO_WRITE                "WRITE"
#define DEF_COM_BTM_TRACE_IO_REWRITE              "REWRITE"
#define DEF_COM_BTM_TRACE_IO_DELETE               "DELETE"
#define DEF_COM_BTM_TRACE_IO_UNLOCK               "UNLOCK"
#define DEF_COM_BTM_TRACE_IO_CLOSE                "CLOSE"
#define DEF_COM_BTM_IO_POSMODE_GEN                1

/**
 * 制御電文エレメント情報ファイル読込I/Oタイマー
 * msでタイマーを指定する。-1:無期限
 */
const long DEF_COM_BTM_GFELI_IO_TIMER = -1L;

/*ISO8583context_tのデフォルト電文フォーマットバージョン ’1’:通常 固定 */
const char DEF_COM_BTM_DEFAULT_IPC_VERSION = '1';

/****************************************************************************/
/*   構造体定義                                                             */
/****************************************************************************/
//共通I/Oモジュール呼出しに必要なデータのセット(共通I/Oモジュールの引数セット)
#pragma fieldalign shared2 __iom_params_def
typedef struct __iom_params_def
{
    module_id_t       mod_id;                     ///< I   トレース用モジュールID
    filename_l_t      gfeli_name_l;               ///< I   制御電文エレメント情報ファイル論理名
    filename_p_t      gfeli_name_p;               ///< I   制御電文エレメント情報ファイル物理名
    char              nw_id;                      ///< I   NW識別子(検索キー)
    sub_prog_sts_t     arg2;                      ///< O   サブプログラムステータス(ファイルシステムエラー)
    COM_IOM_arg_3_def  arg3;                      ///< I   トレース情報
    COM_IOM_arg_4_def  arg4;                      ///< I/O ファイル情報
    COM_IOM_arg_5_def  arg5;                      ///< I   入力情報(主として読込関連)
    COM_IOM_arg_6_def  arg6;                      ///< O   出力情報(主として書込関連)
} iom_params_def;

/****************************************************************************/
/*   内部関数定義                                                           */
/****************************************************************************/
void iom_error_info_set(ISO8583context_t *ctx, iom_params_def * io_params);
short gfeli_file_open(iom_params_def * io_params);
short gfeli_file_read_first(iom_params_def * io_params);
short gfeli_file_read_next(iom_params_def * io_params);
short gfeli_file_close(iom_params_def * io_params);
char* trim(char* str, size_t buffer_length) ;
size_t transcode_numeric(const char *buffer,size_t length, bool *error_flg);
bool set_ffd_rec_tbl_numeric_column(ISO8583context_t *ctx, ffd_def *ffd, db_gfeli_def * gfeli_rec);
bool set_ffd_rec_tbl(ISO8583context_t *ctx, ffd_def *ffd, db_gfeli_def * gfeli_rec);
bool fixformat_attribute_validation(ISO8583context_t *ctx,ffd_index_def * ffd_index,ffd_def *ffd);

/****************************************************************************/
/*  FUNCTION        : com_btm_initial_ISO8583                               */
/*  CALLING SEQ.    : bool com_btm_initial_ISO8583                          */
/*                           (ISO8583context_t *ctx,                        */
/*                            module_id_t mod_id,                           */
/*                            filename_l_t gfeli_name_l,                    */
/*                            filename_p_t gfeli_name_p,                    */
/*                            char nw_id,                                   */
/*                            ffd_index_def *ffd_index_buffer,              */
/*                            size_t ffd_index_count,                       */
/*                            ffd_def *ffd_buffer,                          */
/*                            size_t ffd_count)                             */
/*                                                                          */
/*  ARGUMENT        : ctx [out]                                             */
/*                      ISO8583コンテキスト                                 */
/*                  : mod_id [in]                                           */
/*                      モジュールID                                        */
/*                  : gfeli_name_l [in]                                     */
/*                      制御電文エレメント情報論理ファイル名                */
/*                  : gfeli_name_p [in]                                     */
/*                      制御電文エレメント情報ファイル名                    */
/*                  : nw_id [in]                                            */
/*                      NW識別                                               */
/*                  : ffd_index_buffer [in]                                 */
/*                      固定フォーマット定義インデックス配列               */
/*                  : ffd_index_count [in]                                  */
/*                      固定フォーマット定義インデックス配列最大レコード数  */
/*                  : ffd_buffer [in]                                       */
/*                      固定フォーマット定義配列                            */
/*                  : ffd_count [in]                                        */
/*                      固定フォーマット定義最大レコード数                  */
/*                                                                          */
/*  RETURN CODE     : true  - 正常                                          */
/*                  : false - 初期化失敗                                    */
/*                                                                          */
/*  DESCRIPTION     : ISO8583結合展開モジュール初期化                      */
/*                    ISO8583結合展開モジュールの初期化を行う。            */
/*                    ブランド種別をひとつ指定して制御電文エレメント情報   */
/*                    ファイルのレコードを取得する。プロセス起動後1度      */
/*                    呼べば良い。割当てるメモリはスコープ内で有効であること*/
/****************************************************************************/
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
bool com_btm_initial_ISO8583(
                    ISO8583context_t*             ctx,
                    module_id_t                   mod_id,
                    filename_l_t		          gfeli_name_l,
                    filename_p_t		          gfeli_name_p,
                    char 		                  nw_id,
                    ffd_index_def*	              ffd_index_buffer,
                    size_t		                  ffd_index_count,
                    ffd_def*	                  ffd_buffer,
                    size_t		                  ffd_count
)
{
    short           com_iom_result;
    iom_params_def  io_params;
    db_gfeli_def    *gfeli_rec;
    short           *gfeli_read_len;
    ffd_index_def   *ffd_index;
    ffd_def         *ffd;
    error_info_desc_t error_info_desc;
    bool          result = true;
    bool          holder;

    //コード整合性チェック
    //__ffd_index_defと__ffd_index_def_opaqueのレコード長が一致しないときABENDする。
    assert( sizeof(ffd_index_def) == sizeof_ffd_index_opaque() );
    //__ffd_defと__ffd_def_opaqueのレコード長が一致しないときABENDする。
    assert( sizeof(ffd_def) == sizeof_ffd_opaque() );

    /// 1.ctxの初期化
    ctx->m_ffd_index_tbl = ffd_index_buffer;
    ctx->m_ffd_rec_tbl = ffd_buffer;
    memmove(ctx->m_module_id , mod_id, sizeof(ctx->m_module_id)-1);
    memset(&(ctx->m_error_info), 0, sizeof(error_info_t));
    ctx->m_ipc_version = DEF_COM_BTM_DEFAULT_IPC_VERSION;

    /// 2.制御電文読込情報の初期化
    memset(&io_params, 0,sizeof(iom_params_def));
    // モジュールID
    strncpy(io_params.mod_id, mod_id,sizeof(module_id_t)-1);
    // 論理ファイル名
    strncpy(io_params.gfeli_name_l,gfeli_name_l,sizeof(filename_p_t)-1);
    /// 物理ファイル名
    strncpy(io_params.gfeli_name_p,gfeli_name_p,sizeof(filename_p_t)-1);
    /// 検索キー値
    io_params.nw_id = nw_id;

    gfeli_rec = (db_gfeli_def *)io_params.arg6.rec_area;
    gfeli_read_len = &(io_params.arg6.rec_len);

    com_iom_result = gfeli_file_open(&io_params);
    if(com_iom_result)
    {
        CLR_ERR_INFO(ctx);
        SET_ERR_INFO_ERCD(ctx,GFPCGX20_ER_FILE_IO_ERROR) ;
        snprintf(error_info_desc,sizeof(error_info_desc_t),
        "Open error at File:%.*s", ZSYS_VAL_LEN_FILENAME,
                io_params.arg4.file_name);
        SET_ERR_INFO_DESC(ctx, error_info_desc);
        iom_error_info_set(ctx, &io_params);
        return false;
    }

    com_iom_result = gfeli_file_read_first(&io_params);
    if(com_iom_result)
    {
        CLR_ERR_INFO(ctx);
        SET_ERR_INFO_ERCD(ctx,GFPCGX20_ER_FILE_IO_ERROR) ;
        snprintf(error_info_desc,sizeof(error_info_desc_t),
        "Read start error at File:%.*s", ZSYS_VAL_LEN_FILENAME,
                io_params.arg4.file_name);
        SET_ERR_INFO_DESC(ctx, error_info_desc);
        iom_error_info_set(ctx,&io_params);
        gfeli_file_close(&io_params);
        return false;
    }

    ffd_index = ctx->m_ffd_index_tbl;
    ffd = ctx->m_ffd_rec_tbl;
    while(io_params.arg6.guardian_errcode == ZFIL_ERR_OK)
    {
        if(*gfeli_read_len < sizeof(db_gfeli_def))
        {
            // レコード長不正
            CLR_ERR_INFO(ctx) ;
            SET_ERR_INFO_FFDKEY(ctx,(*(ffd_key_t *)&(gfeli_rec->pri_key.ipc_id)));
            SET_ERR_INFO_DEN(ctx,transcode_numeric(gfeli_rec->pri_key.bit_id,sizeof(denum_t),&holder));
            SET_ERR_INFO_ERCD(ctx,GFPCGX20_ER_INVALID_GFELI_REC);
            SET_ERR_INFO_DESC(ctx,"Invalid length of record retrieve from GFELI.");
            SET_ERR_INFO_DMP(ctx,gfeli_rec,sizeof(db_gfeli_def));
            gfeli_file_close(&io_params);
            return false;
        }
        if((ffd - ctx->m_ffd_rec_tbl + 1 ) > ffd_count)
        {
            // 固定フォーマットバッファオーバーフロー
            CLR_ERR_INFO(ctx);
            SET_ERR_INFO_FFDKEY(ctx,(*(ffd_key_t *)&(gfeli_rec->pri_key.ipc_id)));
            SET_ERR_INFO_DEN(ctx,transcode_numeric(gfeli_rec->pri_key.bit_id,sizeof(denum_t),&holder));
            SET_ERR_INFO_ERCD(ctx,GFPCGX20_ER_FIX_FMT_NUM_EXCEED);
            SET_ERR_INFO_DESC(ctx,"number of fixformat record has exceeded.");
            SET_ERR_INFO_DMP(ctx,gfeli_rec,sizeof(db_gfeli_def));
            gfeli_file_close(&io_params);
            return false;
        }
        if(memcmp(&(ffd_index->ffd_key), &(gfeli_rec->pri_key.ipc_id),sizeof(ffd_key_t) - 1 )!=0)
        {
            if(ffd_index->ffd_key.m_ipc_version != '\0')
            {
                //作成完了した固定フォーマットをチェックする。
                result = fixformat_attribute_validation(ctx, ffd_index, ctx->m_ffd_rec_tbl + ffd_index->m_ffd_pos);
                if(result)
                {
                    gfeli_file_close(&io_params);
                    return false;
                }
                ffd_index++;
            }
            if((ffd_index - ctx->m_ffd_index_tbl + 1) > ffd_index_count )
            {
                // 固定フォーマットインデックスバッファオーバーフロー
                CLR_ERR_INFO(ctx);
                SET_ERR_INFO_FFDKEY(ctx,(*(ffd_key_t *)&(gfeli_rec->pri_key.ipc_id)));
                SET_ERR_INFO_DEN(ctx,transcode_numeric(gfeli_rec->pri_key.bit_id,sizeof(denum_t),&holder));
                SET_ERR_INFO_ERCD(ctx,GFPCGX20_ER_FIX_FMT_IDX_NUM_EXCEED);
                SET_ERR_INFO_DESC(ctx,"number of fixformat index record has exceeded.");
                SET_ERR_INFO_DMP(ctx,gfeli_rec,sizeof(db_gfeli_def));
                gfeli_file_close(&io_params);
                return false;
            }
            //固定フォーマットインデックスを作成する。
            memmove(&(ffd_index->ffd_key), &(gfeli_rec->pri_key.ipc_id),sizeof(ffd_key_t) - 1);
            ffd_index->m_ffd_pos = ffd - ctx->m_ffd_rec_tbl;
            ctx->m_ffd_index_tbl_cnt++;
        }
        //固定フォーマットを作成する。
        result = set_ffd_rec_tbl(ctx, ffd, gfeli_rec);
        if(result)
        {
            gfeli_file_close(&io_params);
            return false;
        }
        //固定フォーマットインデックス更新
        ffd_index->m_ffd_count++;
        ffd++;
        com_iom_result = gfeli_file_read_next(&io_params);
    }
    if(io_params.arg6.guardian_errcode == ZFIL_ERR_EOF)
    {
        //作成完了した固定フォーマットをチェックする。
        result = fixformat_attribute_validation(ctx, ffd_index, ctx->m_ffd_rec_tbl + ffd_index->m_ffd_pos);
        if(result)
        {
            gfeli_file_close(&io_params);
            return false;
        }
    }
    else{
        CLR_ERR_INFO(ctx);
        SET_ERR_INFO_ERCD(ctx,GFPCGX20_ER_FILE_IO_ERROR);
        snprintf(error_info_desc,sizeof(error_info_desc_t),
        "Read next error at File:%.*s", ZSYS_VAL_LEN_FILENAME,
                io_params.arg4.file_name);
        SET_ERR_INFO_DESC(ctx, error_info_desc);
        SET_ERR_INFO_DMP(ctx,ffd,sizeof(ffd_def));
        iom_error_info_set(ctx,&io_params);
        gfeli_file_close(&io_params);
        return false;
    }
    com_iom_result = gfeli_file_close(&io_params);
    if(com_iom_result)
    {
        CLR_ERR_INFO(ctx);
        SET_ERR_INFO_ERCD(ctx,GFPCGX20_ER_FILE_IO_ERROR) ;
        snprintf(error_info_desc,sizeof(error_info_desc_t),
        "Close error at File:%.*s", ZSYS_VAL_LEN_FILENAME,
                io_params.arg4.file_name);
        SET_ERR_INFO_DESC(ctx, error_info_desc);
        iom_error_info_set(ctx,&io_params);
        gfeli_file_close(&io_params);
        return false;
    }
    //初期化済
    ctx->m_initialized = true;
    return true;
}
#pragma clang diagnostic pop

/****************************************************************************/
/*  FUNCTION        : COBOLization                                          */
/*  CALLING SEQ.    : void COBOLization                                     */
/*                           (char *dst,                                    */
/*                            const char *src,                              */
/*                            size_t length)                                */
/*                                                                          */
/*  ARGUMENT        : dst [out]                                             */
/*                      target buffer                                       */
/*                  : src [in]                                              */
/*                      source string                                       */
/*                  : length [in]                                           */
/*                      target length                                       */
/*                                                                          */
/*  RETURN CODE     : void                                                  */
/*                                                                          */
/*  DESCRIPTION     : adjust length with padding and left justify           */
/****************************************************************************/
void COBOLization(char * dst,const char * src,size_t length)
{
    char justify_buffer[length+1];
    snprintf (justify_buffer, sizeof(justify_buffer), "%-*s",(int)length,src);
    memmove(dst, justify_buffer, length);
}

/****************************************************************************/
/*  FUNCTION        : chk_ascii_printable                                   */
/*  CALLING SEQ.    : bool chk_ascii_printable(int c)                       */
/*                                                                          */
/*  ARGUMENT        : c [in]                                                */
/*                      判定対象のキャラクターコード                        */
/*                                                                          */
/*  RETURN CODE     : true  - ascii printable                               */
/*                  : false - not ascii printable                           */
/*                                                                          */
/*  DESCRIPTION     : キャラクターがascii printable                         */
/*                    (空白(32)～~(126))の範囲で有る場合に真を返す。        */
/****************************************************************************/
bool chk_ascii_printable(int c)
{
    if(c < 32 || c > 126) {
        return false;
    }
    return true;
}

/****************************************************************************/
/*  FUNCTION        : com_btm_set_ipc_version                               */
/*  CALLING SEQ.    : bool com_btm_set_ipc_version                          */
/*                           (ISO8583context_t *ctx,                        */
/*                            char ipc_version)                             */
/*                                                                          */
/*  ARGUMENT        : ctx [in]                                              */
/*                      ISO8583context_t構造体ポインタ                      */
/*                  : ipc_version [in]                                      */
/*                      電文フォーマットバージョン                          */
/*                                                                          */
/*  RETURN CODE     : true  - 正常                                          */
/*                  : false - パラメータ不正                                */
/*                                                                          */
/*  DESCRIPTION     : ISO8583context_tに電文フォーマットバージョンを        */
/*                    指定する。                                            */
/****************************************************************************/
bool com_btm_set_ipc_version(ISO8583context_t *ctx, char ipc_version)
{
    // ↓最適化されるはず。
    if (!(ctx->m_initialized)) {
        // ISO8583context_tの未初期化エラー
        CLR_ERR_INFO(ctx);
        SET_ERR_INFO_ERCD(ctx, GFPCGX20_ER_UNINITIALIZED);
        SET_ERR_INFO_DESC(ctx, "ISO8583context_t is not initialized.");
        return false;
    }
    // ↓最適化されるはず。
    if (!chk_ascii_printable(ipc_version)) {
        ffd_key_t ffd_key;
        memset(&ffd_key,0 ,sizeof(ffd_key_t));
        ffd_key.m_ipc_version = ipc_version;
        CLR_ERR_INFO(ctx);
        SET_ERR_INFO_DESC(ctx, "Invalid ipc_version");
        SET_ERR_INFO_ERCD(ctx, GFPCGX20_ER_INVALID_PARAMETER);
        SET_ERR_INFO_FFDKEY(ctx, ffd_key);
        return false;
    }
    ctx->m_ipc_version = ipc_version;
    return true;
}

/****************************************************************************/
/*  FUNCTION        : set_ffd_rec_tbl_numeric_column                        */
/*  CALLING SEQ.    : bool set_ffd_rec_tbl_numeric_column                   */
/*                           (ISO8583context_t *ctx,                        */
/*                            ffd_def *ffd,                                 */
/*                            db_gfeli_def *gfeli_rec)                      */
/*                                                                          */
/*  ARGUMENT        : ctx [in]                                              */
/*                      ISO8583コンテキスト構造体ポインタ                   */
/*                  : ffd [in]                                              */
/*                      固定フォーマット定義構造体ポインタ                  */
/*                  : gfeli_rec [in]                                        */
/*                      制御電文エレメント情報レコード構造体ポインタ        */
/*                                                                          */
/*  RETURN CODE     : true  - 異常値検出                                    */
/*                  : false - 正常                                          */
/*                                                                          */
/*  DESCRIPTION     : Set the ffd rec tbl numeric column object             */
/****************************************************************************/
bool set_ffd_rec_tbl_numeric_column(ISO8583context_t *ctx, ffd_def *ffd, db_gfeli_def * gfeli_rec)
{
    bool error_flg;

    /// 電文フォーマットバージョン ファイル設計書の属性指定がXなのでascii printableのうち空白を除く範囲であること
    if(!chk_ascii_printable(gfeli_rec->pri_key.ipc_id) || gfeli_rec->pri_key.ipc_id == ' ')
    {
        CLR_ERR_INFO(ctx);
        SET_ERR_INFO_DESC(ctx,"Invalid field value at gfeli::ipc_id");
        return true;
    }
    /// ビット番号
    ffd->pri_key.m_de_num = (long)transcode_numeric(gfeli_rec->pri_key.bit_id, sizeof(gfeli_rec->pri_key.bit_id), &error_flg);
    /// ビット番号の範囲チェック
    if(error_flg == false && (ffd->pri_key.m_de_num < 2 || ffd->pri_key.m_de_num > 192)) error_flg = true;
    if(error_flg)
    {
        CLR_ERR_INFO(ctx);
        SET_ERR_INFO_DESC(ctx,"Invalid field value at gfeli::bit_id");
        return true;
    }
    /// データ長形式
    switch(gfeli_rec->msg_fmt_info.data_len_type)
    {
        case '0':
            ffd->raw_msg_fmt_info.m_raw_length_type = FMT_LEN_FIX;
            break;
        case '1':
            ffd->raw_msg_fmt_info.m_raw_length_type = FMT_LEN_LVAR;
            break;
        case '2':
            ffd->raw_msg_fmt_info.m_raw_length_type = FMT_LEN_LLVAR;
            break;
        case '3':
            ffd->raw_msg_fmt_info.m_raw_length_type = FMT_LEN_LLLVAR;
            break;
        case '4':
            ffd->raw_msg_fmt_info.m_raw_length_type = FMT_LEN_LLLLVAR;
            break;
        default:
            CLR_ERR_INFO(ctx);
            SET_ERR_INFO_DESC(ctx,"Invalid field value at gfeli::msg_fmt_info.data_len_type");
            return true;
    }
    /// データ長サイズ
    ffd->raw_msg_fmt_info.m_raw_length_size = transcode_numeric(&(gfeli_rec->msg_fmt_info.data_len_size), sizeof(gfeli_rec->msg_fmt_info.data_len_size), &error_flg);
    ///データ長サイズが非数字または、データ長形式が固定長でない場合にデータ長サイズが0(ないし空白)だった場合はエラーとする。
    if(error_flg || (ffd->raw_msg_fmt_info.m_raw_length_type != FMT_LEN_FIX && ffd->raw_msg_fmt_info.m_raw_length_size == 0))
    {
        CLR_ERR_INFO(ctx);
        SET_ERR_INFO_DESC(ctx,"Invalid field value at gfeli::msg_fmt_info.data_len_size");
        return true;
    }
    /// 最大データ長
    ffd->raw_msg_fmt_info.m_raw_data_max_len = transcode_numeric(gfeli_rec->msg_fmt_info.data_max_len, sizeof(gfeli_rec->msg_fmt_info.data_max_len), &error_flg);; ;
     if(error_flg)
    {
        CLR_ERR_INFO(ctx);
        SET_ERR_INFO_DESC(ctx,"Invalid field value at gfeli::msg_fmt_info.data_max_len");
        return true;
    }
    /// テーブル開始オフセット
    ffd->fix_fmt_info.m_tbl_start_offset = transcode_numeric(gfeli_rec->fix_fmt_info.tbl_start_offset, sizeof(gfeli_rec->fix_fmt_info.tbl_start_offset), &error_flg);; ;
    if(error_flg)
    {
        CLR_ERR_INFO(ctx);
        SET_ERR_INFO_DESC(ctx,"Invalid field value at gfeli::fix_fmt_info.tbl_start_offset");
        return true;
    }
    /// データエリアサイズ
    ffd->fix_fmt_info.m_data_area_size = transcode_numeric(gfeli_rec->fix_fmt_info.data_area_size, sizeof(gfeli_rec->fix_fmt_info.data_area_size), &error_flg);
    if(error_flg)
    {
        CLR_ERR_INFO(ctx);
        SET_ERR_INFO_DESC(ctx,"Invalid field value at gfeli::fix_fmt_info.data_area_size");
        return true;
    }
    return false;
}

/****************************************************************************/
/*  FUNCTION        : set_ffd_rec_tbl                                       */
/*  CALLING SEQ.    : bool set_ffd_rec_tbl                                  */
/*                           (ISO8583context_t *ctx,                        */
/*                            ffd_def *ffd,                                 */
/*                            db_gfeli_def *gfeli_rec)                      */
/*                                                                          */
/*  ARGUMENT        : ctx [in]                                              */
/*                      ISO8583context構造体ポインタ                        */
/*                  : ffd [out]                                             */
/*                      固定フォーマットを格納する構造体ポインタ            */
/*                  : gfeli_rec [in]                                        */
/*                      制御電文エレメント情報ファイルのレコード構造体ポインタ*/
/*                                                                          */
/*  RETURN CODE     : true  - 異常値検出                                    */
/*                  : false - 正常                                          */
/*                                                                          */
/*  DESCRIPTION     : Set the ffd rec tbl object                            */
/****************************************************************************/
bool set_ffd_rec_tbl(ISO8583context_t *ctx, ffd_def *ffd, db_gfeli_def * gfeli_rec)
{
    bool result;
    transcoder_set_def    transcoder_set;
    length_transcoder_set_def length_transcoder_set;

    memset(&length_transcoder_set, 0,sizeof(length_transcoder_set));
    memmove(&(ffd->pri_key.ffd_key), &(gfeli_rec->pri_key.ipc_id), sizeof(ffd_key_t) - 1);

    ///数値属性のカラムのチェックと処理を行う。
    result = set_ffd_rec_tbl_numeric_column(ctx, ffd, gfeli_rec);
    if(result){
        SET_ERR_INFO_FFDKEY(ctx,(*(ffd_key_t *)&(gfeli_rec->pri_key.ipc_id)));
        SET_ERR_INFO_DEN(ctx, (long)transcode_numeric(gfeli_rec->pri_key.bit_id, sizeof(denum_t), &result));
        SET_ERR_INFO_ERCD(ctx,GFPCGX20_ER_INVALID_GFELI_COL);
        SET_ERR_INFO_DMP(ctx,gfeli_rec,sizeof(db_gfeli_def));
        return true;
    }
    // 修正: 固定長フォーマットのデータ長部変換値指定は不定であると仮定する。
    if(ffd->raw_msg_fmt_info.m_raw_length_type != FMT_LEN_FIX)
    {
        ///フィールドが固定長でなければデータ長属性の変換方法を検索する。
        result = query_length_transcoder(gfeli_rec->pri_key.nw_id,
                    gfeli_rec->msg_fmt_info.data_len_attribute,
                        &length_transcoder_set);
        if(!result)
        {
            CLR_ERR_INFO(ctx);
            SET_ERR_INFO_FFDKEY(ctx,(*(ffd_key_t *)&(gfeli_rec->pri_key.ipc_id)));
            SET_ERR_INFO_DEN(ctx, (long)transcode_numeric(gfeli_rec->pri_key.bit_id, sizeof(denum_t), &result));
            SET_ERR_INFO_ERCD(ctx,GFPCGX20_ER_UNKNOWN_CODE_L);
            SET_ERR_INFO_DESC(ctx, "length transcoder not found.")
            SET_ERR_INFO_DMP(ctx,gfeli_rec,sizeof(db_gfeli_def));
            return true;
        }
    }
    ffd->fix_fmt_info.m_fix_length_decoder = length_transcoder_set.decoder;
    ffd->raw_msg_fmt_info.m_raw_length_encoder = length_transcoder_set.encoder;

    ///データ部の変換方法を検索する。
    result = query_value_transcoder(gfeli_rec->pri_key.nw_id,
                gfeli_rec->msg_fmt_info.data_area_attribute,
                   gfeli_rec->fix_fmt_info.code_change_need,
                   &transcoder_set);
    if(!result)
    {
        CLR_ERR_INFO(ctx);
        SET_ERR_INFO_FFDKEY(ctx,(*(ffd_key_t *)&(gfeli_rec->pri_key.ipc_id)));
        SET_ERR_INFO_DEN(ctx, (long)transcode_numeric(gfeli_rec->pri_key.bit_id, sizeof(denum_t), &result));
        SET_ERR_INFO_ERCD(ctx,GFPCGX20_ER_UNKNOWN_CODE_V);
        SET_ERR_INFO_DESC(ctx, "value transcoder not found.")
        SET_ERR_INFO_DMP(ctx,gfeli_rec,sizeof(db_gfeli_def));
        return true;
    }
    ffd->fix_fmt_info.m_fix_data_decoder = transcoder_set.decoder;
    ffd->raw_msg_fmt_info.m_raw_data_encoder = transcoder_set.encoder;
    return false;
}

/****************************************************************************/
/*  FUNCTION        : gfeli_file_open                                       */
/*  CALLING SEQ.    : short gfeli_file_open                                 */
/*                           (iom_params_def *io_params)                    */
/*                                                                          */
/*  ARGUMENT        : io_params [in,out]                                    */
/*                      入出力パラメータ構造体ポインタ                      */
/*                                                                          */
/*  RETURN CODE     : 0     - 処理結果正常                                  */
/*                  : その他 - エラー有                                     */
/*                                                                          */
/*  DESCRIPTION     : 共通IOモジュールを呼出し制御エレメント情報ファイルを  */
/*                    OPENする                                              */
/****************************************************************************/
short gfeli_file_open(iom_params_def * io_params)
{
    short           com_iom_result;
    /// 1.トレース情報の初期化(OPEN時1回のみのフィールド)
    COBOLization(io_params->arg3.prog_id, io_params->mod_id, sizeof(io_params->arg3.prog_id));
    //↓はIOMモジュール内でやっているのを確認した。
    // strncpy(io_params->arg3.file_id,io_params->gfeli_name_l,sizeof(filename_l_t)-1);
    // strncpy(io_params->arg3.file_name,io_params->gfeli_name_p,sizeof(filename_p_t)-1);
    /// 2. IO固有のトレースに必要な情報
    COBOLization(io_params->arg3.file_io_type, DEF_COM_BTM_TRACE_IO_OPEN, sizeof(io_params->arg3.file_io_type));
    /// 3.ファイル名の設定
    COBOLization(io_params->arg4.file_id, io_params->gfeli_name_l, sizeof(filename_l_t)-1);
    COBOLization(io_params->arg4.file_name,io_params->gfeli_name_p,sizeof(filename_p_t)-1);
    /// 4.サブプログラムステータスを初期化
    memset(io_params->arg2, ' ',sizeof(sub_prog_sts_t));
    /// 5.I/Oコアモジュール呼出し"OPEN"
    com_iom_result = COM_IOM(DEF_COM_IOM_FUNC_OPEN
                   , io_params->arg2
	               , &(io_params->arg3)
	               , &(io_params->arg4)
	               , &(io_params->arg5)
	               , &(io_params->arg6));
    return com_iom_result;
}

/****************************************************************************/
/*  FUNCTION        : gfeli_file_read_first                                 */
/*  CALLING SEQ.    : short gfeli_file_read_first                           */
/*                           (iom_params_def *io_params)                    */
/*                                                                          */
/*  ARGUMENT        : io_params [in,out]                                    */
/*                      入出力パラメータ構造体ポインタ                      */
/*                                                                          */
/*  RETURN CODE     : 0     - 処理結果正常                                  */
/*                  : その他 - エラー有                                     */
/*                                                                          */
/*  DESCRIPTION     : 制御電文エレメント情報ファイルの先頭レコード読込処理  */
/****************************************************************************/
short gfeli_file_read_first(iom_params_def * io_params)
{
    short           com_iom_result;
    /// 1. IO固有のトレースに必要な情報
    memset(io_params->arg3.file_io_type, 0,sizeof(io_params->arg3.file_io_type));
    COBOLization(io_params->arg3.file_io_type, DEF_COM_BTM_TRACE_IO_START, sizeof(io_params->arg3.file_io_type));
    /// 2.ファイル情報設定
    io_params->arg5.key_value[0] = io_params->nw_id;
    memmove(io_params->arg5.key_type,DEF_COM_IOM_KEYTYPE_PRI,sizeof(io_params->arg5.key_type));
    io_params->arg5.key_len = 1;
    io_params->arg5.compare_len = 1;
    io_params->arg5.positioning_mode = DEF_COM_BTM_IO_POSMODE_GEN;
    io_params->arg5.io_timer = DEF_COM_BTM_GFELI_IO_TIMER;
    io_params->arg5.rec_len = sizeof(db_gfeli_def);
    /// 3.サブプログラムステータスを初期化
    memset(io_params->arg2, 0,sizeof(sub_prog_sts_t));
    /// 4.I/Oコアモジュール呼出し"OPEN"
    com_iom_result = COM_IOM(DEF_COM_IOM_FUNC_STARTREAD
                   , io_params->arg2
	               , &(io_params->arg3)
	               , &(io_params->arg4)
	               , &(io_params->arg5)
	               , &(io_params->arg6));
    return com_iom_result;
}

/****************************************************************************/
/*  FUNCTION        : gfeli_file_read_next                                  */
/*  CALLING SEQ.    : short gfeli_file_read_next                            */
/*                           (iom_params_def *io_params)                    */
/*                                                                          */
/*  ARGUMENT        : io_params [in,out]                                    */
/*                      入出力パラメータ構造体ポインタ                      */
/*                                                                          */
/*  RETURN CODE     : 0     - 処理結果正常                                  */
/*                  : その他 - エラー有                                     */
/*                                                                          */
/*  DESCRIPTION     : 制御電文エレメント情報ファイルの次レコード読込処理    */
/****************************************************************************/
short gfeli_file_read_next(iom_params_def * io_params)
{
    short           com_iom_result;
    /// 1. IO固有のトレースに必要な情報
    memset(io_params->arg3.file_io_type, 0,sizeof(io_params->arg3.file_io_type));
    COBOLization(io_params->arg3.file_io_type, DEF_COM_BTM_TRACE_IO_READ, sizeof(io_params->arg3.file_io_type));
    /// 3.サブプログラムステータスを初期化
    memset(io_params->arg2, 0,sizeof(sub_prog_sts_t));
    /// 4.I/Oコアモジュール呼出し"OPEN"
    com_iom_result = COM_IOM(DEF_COM_IOM_FUNC_NEXTREAD
                   , io_params->arg2
	               , &(io_params->arg3)
	               , &(io_params->arg4)
	               , &(io_params->arg5)
	               , &(io_params->arg6));
    return com_iom_result;
}

/****************************************************************************/
/*  FUNCTION        : gfeli_file_close                                      */
/*  CALLING SEQ.    : short gfeli_file_close                                */
/*                           (iom_params_def *io_params)                    */
/*                                                                          */
/*  ARGUMENT        : io_params [in,out]                                    */
/*                      入出力パラメータ構造体ポインタ                      */
/*                                                                          */
/*  RETURN CODE     : 0     - 処理結果正常                                  */
/*                  : その他 - エラー有                                     */
/*                                                                          */
/*  DESCRIPTION     : 制御電文エレメント情報ファイルのクローズ処理          */
/****************************************************************************/
short gfeli_file_close(iom_params_def * io_params)
{
    short           com_iom_result;
    /// 1. IO固有のトレースに必要な情報
    memset(io_params->arg3.file_io_type, 0,sizeof(io_params->arg3.file_io_type));
    COBOLization(io_params->arg3.file_io_type, DEF_COM_BTM_TRACE_IO_CLOSE, sizeof(io_params->arg3.file_io_type));
    /// 3.サブプログラムステータスを初期化
    memset(io_params->arg2, 0,sizeof(sub_prog_sts_t));
    /// 4.I/Oコアモジュール呼出し"OPEN"
    com_iom_result =  COM_IOM(DEF_COM_IOM_FUNC_CLOSE
                              , io_params->arg2
	                          , &(io_params->arg3)
	                          , &(io_params->arg4)
	                          , &(io_params->arg5)
	                          , &(io_params->arg6));
    return com_iom_result;
}

/****************************************************************************/
/*  FUNCTION        : iom_error_info_set                                    */
/*  CALLING SEQ.    : void iom_error_info_set                               */
/*                           (ISO8583context_t *ctx,                        */
/*                            iom_params_def *io_params)                    */
/*                                                                          */
/*  ARGUMENT        : ctx [in]                                              */
/*                      ISO8583context構造体ポインタ                        */
/*                  : io_params [in]                                        */
/*                      入出力パラメータ構造体ポインタ                      */
/*                                                                          */
/*  RETURN CODE     : void                                                  */
/*                                                                          */
/*  DESCRIPTION     : I/O処理エラー情報をISO8583context_tに設定する         */
/****************************************************************************/
void iom_error_info_set(ISO8583context_t *ctx, iom_params_def * io_params)
{
    memset(&(ctx->m_error_info.m_guardian_error_Info), 0, sizeof(guardian_info_def));
    ctx->m_error_info.m_guardian_error_Info.guardian_errcode = io_params->arg6.guardian_errcode;
    strncpy(ctx->m_error_info.m_guardian_error_Info.err_proc ,io_params->arg6.err_proc, sizeof(io_params->arg6.err_proc));
    strncpy(ctx->m_error_info.m_guardian_error_Info.file_name,io_params->arg6.file_name, ZSYS_VAL_LEN_FILENAME);
    strncpy(ctx->m_error_info.m_guardian_error_Info.sub_prog_sts,io_params->arg2,sizeof(sub_prog_sts_t) - 1);
}

/****************************************************************************/
/*  FUNCTION        : transcode_numeric                                     */
/*  CALLING SEQ.    : size_t transcode_numeric                              */
/*                           (const char *buffer,                           */
/*                            size_t length,                                */
/*                            bool *error_flg)                              */
/*                                                                          */
/*  ARGUMENT        : buffer [in]                                           */
/*                      数値文字列データ                                    */
/*                  : length [in]                                           */
/*                      bufferの長さ                                        */
/*                  : error_flg [out]                                       */
/*                      エラー発生時にtrueが設定されるフラグポインタ        */
/*                                                                          */
/*  RETURN CODE     : size_t                                                */
/*                      数値変換された値、失敗時は SIZE_MAX                 */
/*                                                                          */
/*  DESCRIPTION     : 数値文字列を符号なし整数に変換し、失敗時はフラグを設定 */
/****************************************************************************/
size_t transcode_numeric(const char *buffer,size_t length, bool *error_flg)
{
    char transcode_buffer[length + 1];
    char *transcode_ptr;
    char *epos;
    size_t retval = SIZE_MAX;
    *error_flg = false;
    memmove(transcode_buffer, buffer,length);
    transcode_buffer[length] = '\0';
    transcode_ptr = trim(transcode_buffer, length);
    retval = strtoul(transcode_ptr, &epos, 10);
    if(errno == ERANGE ||
#ifdef EINVAL
       errno == EINVAL ||
#endif
       *epos != '\0') *error_flg = true;
    return retval;
}

/****************************************************************************/
/*  FUNCTION        : trim                                                  */
/*  CALLING SEQ.    : char* trim                                            */
/*                           (char *str,                                    */
/*                            size_t buffer_length)                         */
/*                                                                          */
/*  ARGUMENT        : str [in]                                              */
/*                      対象の文字列ポインタ                                */
/*                  : buffer_length [in]                                    */
/*                      バッファの長さ                                      */
/*                                                                          */
/*  RETURN CODE     : char*                                                 */
/*                      先頭と末尾の空白およびNULL文字を除去した文字列への  */
/*                      ポインタ                                            */
/*                                                                          */
/*  DESCRIPTION     : 文字列の先頭および末尾にある空白やNULL文字を除去する */
/****************************************************************************/
char* trim(char* str, size_t buffer_length)
{
    if (str == NULL || buffer_length == 0) {
        return NULL;
    }

    // 先頭の空白とNULLを取り除く
    char* start = str;
    while (*start != '\0' && (isspace((unsigned char)*start) || *start == '\0')) {
        start++;
    }

    // 末尾の空白とNULLを取り除く
    char* end = str + buffer_length - 1; // buffer_length - 1 は NULL 終端の位置
    while (end > start && (isspace((unsigned char)*end) || *end == '\0')) {
        end--;
    }

    // 新しい終端を設定
    *(end + 1) = '\0';

    return start;
}

/****************************************************************************/
/*  FUNCTION        : fixformat_attribute_validation                        */
/*  CALLING SEQ.    : bool fixformat_attribute_validation                   */
/*                           (ISO8583context_t *ctx,                        */
/*                            ffd_index_def *ffd_index,                     */
/*                            ffd_def *ffd)                                 */
/*                                                                          */
/*  ARGUMENT        : ctx [in,out]                                          */
/*                      ISO8583context構造体ポインタ                        */
/*                  : ffd_index [in,out]                                    */
/*                      固定フォーマットインデックス構造体ポインタ          */
/*                  : ffd [in]                                              */
/*                      固定フォーマット定義配列ポインタ                    */
/*                                                                          */
/*  RETURN CODE     : true  - 属性不整合エラーあり                          */
/*                  : false - 正常                                          */
/*                                                                          */
/*  DESCRIPTION     : 固定フォーマット属性の整合性検証処理                  */
/*                    ・各データエリアのオフセットが昇順であることを確認    */
/*                    ・前のデータエリアと重複していないことを確認          */
/*                    ・エラー時にはISO8583context_tにエラー情報を設定      */
/****************************************************************************/
//#define __DEBUG_GFPCGX20
//↑optimize 0でも何故かsizeof(ffd_header_def)とffdの参照が最適化されてしまうため、
// デバッグで内容を参照したい場合のコード。
bool fixformat_attribute_validation(ISO8583context_t *ctx,ffd_index_def * ffd_index,ffd_def *ffd)
{
    size_t          ffd_count;
    #ifdef __DEBUG_GFPCGX20
    size_t current, prev,ffd_header_def_size = sizeof(ffd_header_def);
    #endif
    for(ffd_count = 0; ffd_count < ffd_index->m_ffd_count; ffd_count++)
    {
        /// 1.テーブル開始オフセットが昇順であること。
        /// 2.オフセット開始位置が一つまえの固定フォーマットのデータエリアと重ならないこと。
        if(ffd_count > 0)
        {
            #ifdef __DEBUG_GFPCGX20
            current = ffd[ffd_count].fix_fmt_info.m_tbl_start_offset;
            prev = ffd[ffd_count - 1].fix_fmt_info.m_tbl_start_offset
                + (sizeof(ffd_header_def))
                + ffd[ffd_count - 1].fix_fmt_info.m_data_area_size;
            #endif
            if(ffd[ffd_count].fix_fmt_info.m_tbl_start_offset <
                ffd[ffd_count - 1].fix_fmt_info.m_tbl_start_offset
                + (sizeof(ffd_header_def))
                + ffd[ffd_count - 1].fix_fmt_info.m_data_area_size)
            {
                CLR_ERR_INFO(ctx);
                SET_ERR_INFO_FFDKEY(ctx,ffd[ffd_count].pri_key.ffd_key);
                SET_ERR_INFO_DEN(ctx, ffd[ffd_count].pri_key.m_de_num);
                SET_ERR_INFO_ERCD(ctx,GFPCGX20_ER_FIX_FMT_OFFSET_INC);
                SET_ERR_INFO_DESC(ctx, "Offset overlaps with previous data area.")
                SET_ERR_INFO_DMP(ctx,&(ffd[ffd_count]),sizeof(ffd_def));
                return true;
            }
        }
    }
    ffd_index->m_fix_format_length = ffd[ffd_index->m_ffd_count-1].fix_fmt_info.m_tbl_start_offset
                                    + (ffd[ffd_index->m_ffd_count-1].fix_fmt_info.m_data_area_size + 1) / 2 * 2
                                    + sizeof(ffd_header_def);
    return false;
}
