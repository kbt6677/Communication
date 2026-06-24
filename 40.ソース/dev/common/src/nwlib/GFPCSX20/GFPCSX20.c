/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSJ20                                    */
/*        FUNCTION          ････ NW個別モジュール                            */
/*                             鍵交換および暗号化・復号処理用初期処理(共通)  */
/*                               物理名情報ファイルよりATALLA振分サーバ情報  */
/*                               と鍵管理ファイル情報を取得し、              */
/*                                 鍵管理ファイルのオープンを行う            */
/*        AUTHER            ････ HAS T.Fukunaga                              */
/*        PROGRAM-CALL      ････ EMS出力モジュール                           */
/*                               IOモジュール                                */
/*        WRITTEN-DATE      ････ 2024-10-01                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  T.Fukunaga 2024/10/01 (J0680)新規作成                               */
/*  2.0  S.Kimura   2025/04/01 初期化処理を分離                              */
/*                                                                           */
/*****************************************************************************/
/****************************************************************************/
/*   ｺﾝﾊﾟｲﾙｵﾌﾟｼｮﾝ                                                           */
/****************************************************************************/
#pragma ENV COMMON

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <stdbool.h>  nolist
#include <stdio.h>    nolist
#include <stdlib.h>   nolist
#include <string.h>   nolist
#include <cextdecs.h> nolist
#include <tal.h>      nolist

/* USER HEADER     */
#include "common.h"
#include "ems.h"
#include "file.h"
#include "NWM_ENI.h"
#include "GFPCGXB0.h"
#include "vproc.h"
/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define  DEF_NWM_ENI_OWN_NODE      0           /* 自ノード                  */
#define  DEF_NWM_ENI_OTHER_NODE    1           /* 他ノード                  */

/****************************************************************************/
/*   内部関数宣言                                                           */
/****************************************************************************/
short NWM_ENI_gfphi_read(char
                       , char *
                       , NWM_ENI_arg_1_def *
                       , NWM_ENI_arg_4_def *
                       , COM_IOM_arg_5_def *
                       , COM_IOM_arg_6_def *
                       , char *
                       , oggz1in_def *
                       , ems_info_add *);
short NWM_ENI_gckey_open(NWM_ENI_arg_2_def *
                       , COM_IOM_arg_5_def *
                       , COM_IOM_arg_6_def *
                       , char *
                       , oggz1in_def *
                       , ems_info_add *);

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_ENI                                        */
/*  CALLING SEQ.    : short NWM_ENI(strcut *, struct *, struct *, struct *  */
/*                                , char *)                                 */
/*  ARGUMENT        : 1.node_type      (I)   鍵管理ファイル取得種別         */
/*                  : 2.key_file_info  (I/O) 鍵管理ファイル情報             */
/*                  : 3.key_file_info  (I/O) 鍵管理ファイル情報             */
/*                  : 4.atalla_srv_info(O)   PATHSEND結果情報               */
/*                  : 5.netwrok_info   (I)   ネットワーク特定情報           */
/*                  : 6.prog_id        (I)   モジュールID                   */
/*                  : 7.ems_info       (I)   EMS共通情報                    */
/*                  : 8.ems_add        (I)   EMS出力付加情報                */
/*  RETURN CODE     : 0 :正常                                               */
/*                  : -1:異常                                               */
/*  DESCRIPTION     : 暗号化・復号処理の初期化を行う                        */
/****************************************************************************/
short NWM_ENI(char node_type
            , NWM_ENI_arg_1_def *physical_info
            , NWM_ENI_arg_2_def *key_file_info_own
            , NWM_ENI_arg_2_def *key_file_info_other
            , NWM_ENI_arg_3_def *atalla_srv_info
            , NWM_ENI_arg_4_def *network_info
            , char *prog_id
            , oggz1in_def *ems_info
            , ems_info_add *ems_add)
{
    short err = 0;
    COM_IOM_arg_5_def in_fl_inf;               /* IOモジュール引数 */
    COM_IOM_arg_6_def out_fl_inf;              /* IOモジュール引数 */
    db_gfphi_def      *gfphi_o;                /* 物理名情報ファイルREADバッファ */
    char              wk_space[16];
    
    /* 初期化     */
    gfphi_o = (db_gfphi_def *)&out_fl_inf.rec_area[0];
    memset(wk_space,' ',sizeof(wk_space));
    /* ATALLA振分サーバ情報レコードの取得 */
    err = NWM_ENI_gfphi_read(DEF_NWM_ENI_OWN_NODE
                            ,DEF_SC_ATALLA
                            ,physical_info
                            ,network_info
                            ,&in_fl_inf
                            ,&out_fl_inf
                            , prog_id
                            , ems_info
                            , ems_add);
    if(err != 0 ){
        return(DEF_NWM_ENI_RTN_NG);
    }
    /* 引数． ATALLA振分サーバ情報の設定 */
    memset(atalla_srv_info->domain_name,' ',sizeof(atalla_srv_info->domain_name));
    memset(atalla_srv_info->server_name,' ',sizeof(atalla_srv_info->server_name));
    if(memcmp(gfphi_o->srv_cls_info.domain_name,wk_space,sizeof(gfphi_o->srv_cls_info.domain_name)) != 0) {
        memcpy(atalla_srv_info->domain_name ,gfphi_o->srv_cls_info.domain_name
            ,sizeof(gfphi_o->srv_cls_info.domain_name));
    } else {
        memcpy(atalla_srv_info->domain_name ,gfphi_o->srv_cls_info.pathmon_name
            ,sizeof(gfphi_o->srv_cls_info.pathmon_name));
    }
    memcpy(atalla_srv_info->server_name ,gfphi_o->srv_cls_info.srv_cls_name
        ,sizeof(atalla_srv_info->server_name));
    /* 鍵管理ファイル情報(自ノード)レコードの取得 */
    err = NWM_ENI_gfphi_read(DEF_NWM_ENI_OWN_NODE,
                             DEF_FL_KEY_MG
                            ,physical_info
                            ,network_info
                            ,&in_fl_inf
                            ,&out_fl_inf
                            , prog_id
                            , ems_info
                            , ems_add);
    if(err != 0 ){
        return(DEF_NWM_ENI_RTN_NG);
    }
    /* 鍵管理ファイルのオープン */
    err = NWM_ENI_gckey_open(key_file_info_own
                            ,&in_fl_inf
                            ,&out_fl_inf
                            , prog_id
                            , ems_info
                            , ems_add);
    if(err != 0 ){
        return(DEF_NWM_ENI_RTN_NG);
    }
    if ( node_type == DEF_NWM_ENI_OTHER_NODE_INC){
        /* 鍵管理ファイル情報(他ノード)レコードの取得 */
        err = NWM_ENI_gfphi_read(DEF_NWM_ENI_OTHER_NODE,
                                 DEF_FL_KEY_MG
                                ,physical_info
                                ,network_info
                                ,&in_fl_inf
                                ,&out_fl_inf
                                , prog_id
                                , ems_info
                                , ems_add);
        if(err != 0 ){
            return(DEF_NWM_ENI_RTN_NG);
        }
        /* 鍵管理ファイルのオープン */
        err = NWM_ENI_gckey_open(key_file_info_other
                                ,&in_fl_inf
                                ,&out_fl_inf
                                , prog_id
                                , ems_info
                                , ems_add);
        if(err != 0 ){
            return(DEF_NWM_ENI_RTN_NG);
        }
    }
    return(DEF_NWM_ENI_RTN_OK);
}
/* end of NWM_ENI */
/****************************************************************************/
/*  FUNCTION        : 1.1.0  NWM_ENI_gfphi_read                             */
/*  CALLING SEQ.    : short NWM_ENI_gfphi_read  (char *                     */
/*                                              ,strcut *                   */
/*                                              ,struct *                   */
/*                                              ,struct *                   */
/*                                              ,struct *                   */
/*                                              ,char * )                   */
/*  ARGUMENT        : 1.prc_file_kind  (I)   ATALLA/GCKEY                   */
/*                  : 2.physical_info  (I)   物理名情報ファイル             */
/*                  : 3.netwrok_info   (I)   ネットワーク特定情報           */
/*                  : 4.in_fl_inf      (I)   入力情報                       */
/*                  : 5.out_fl_inf     (O)   出力情報                       */
/*                  : 6.prog_id        (I)   モジュールID                   */
/*  RETURN CODE     : 0 :正常                                               */
/*                  : -1:異常                                               */
/*  DESCRIPTION     : 物理名情報ファイルの読み込みを行う                    */
/****************************************************************************/
short NWM_ENI_gfphi_read(char node_type
                       , char *prc_file_kind
                       , NWM_ENI_arg_1_def *physical_info
                       , NWM_ENI_arg_4_def *network_info
                       , COM_IOM_arg_5_def *in_fl_inf
                       , COM_IOM_arg_6_def *out_fl_inf
                       , char *prog_id
                       , oggz1in_def *ems_info
                       , ems_info_add *ems_add)
{
    char   subprog_sts[2];
    COM_IOM_arg_3_def trace_inf;
    COM_IOM_arg_4_def file_inf;
    db_gfphi_def      *gfphi_i;              /* 物理名情報ファイルPRI-KEY */

    /* 初期化 */
    gfphi_i = (db_gfphi_def *)&in_fl_inf->key_value[0];
    /* IOモジュール引数設定　トレース情報 */
    memset(trace_inf.prog_id      ,' ',sizeof(trace_inf.prog_id));
    memset(trace_inf.file_id      ,' ',sizeof(trace_inf.file_id));
    memset(trace_inf.file_name    ,' ',sizeof(trace_inf.file_name));
    memset(trace_inf.file_io_type ,' ',sizeof(trace_inf.file_io_type));
    memcpy(trace_inf.prog_id ,prog_id ,sizeof(trace_inf.prog_id));
    memcpy(trace_inf.file_id ,DEF_GFPHI ,sizeof(DEF_GFPHI)-1);
    memcpy(trace_inf.file_name ,physical_info->file_name
        ,sizeof(trace_inf.file_name));
    memcpy(trace_inf.file_io_type ,"READ",4);
    /* IOモジュール引数設定　ファイル情報 */
    memset(file_inf.file_id   ,' ',sizeof(file_inf.file_id));
    memset(file_inf.file_name ,' ',sizeof(file_inf.file_name));
    memcpy(file_inf.file_id ,DEF_GFPHI ,sizeof(DEF_GFPHI)-1);
    memcpy(file_inf.file_name ,physical_info->file_name ,sizeof(file_inf.file_name));
    file_inf.file_no = physical_info->file_no;
    /* IOモジュール引数設定　入力情報 */
    in_fl_inf->part_key_type      = 0x00;
    in_fl_inf->part_key_position  = 0x00;
    in_fl_inf->part_key_len       = 0x00;
    gfphi_i->pri_key.site_id = network_info->site_id;
    gfphi_i->pri_key.nw_id   = network_info->nw_id;
    memcpy(gfphi_i->pri_key.grp_id ,network_info->grp_id ,sizeof(gfphi_i->pri_key.grp_id));
    if(memcmp(prc_file_kind,DEF_SC_ATALLA,sizeof(prc_file_kind)) == 0){
        memcpy(gfphi_i->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind ,DEF_SC_ATALLA 
            ,sizeof(gfphi_i->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind));
        memset(gfphi_i->pri_key.srv_cls_key.srv_cls_id.srv_cls_num,'0'
            ,sizeof(gfphi_i->pri_key.srv_cls_key.srv_cls_id.srv_cls_num));
        memset(gfphi_i->pri_key.srv_cls_key.srv_cls_mlt_num,'0'
            ,sizeof(gfphi_i->pri_key.srv_cls_key.srv_cls_mlt_num));
        memset((char *)&gfphi_i->pri_key.prc_file_key ,'}' ,sizeof(gfphi_i->pri_key.prc_file_key));
    } else {
        if (node_type == DEF_NWM_ENI_OTHER_NODE){
            if (network_info->site_id == DEF_FURI_DST_TKY){
                gfphi_i->pri_key.site_id = DEF_FURI_DST_OSK;
            }
            else {
                gfphi_i->pri_key.site_id = DEF_FURI_DST_TKY;
            }
        }
        memset((char *)&gfphi_i->pri_key.srv_cls_key ,'}' ,sizeof(gfphi_i->pri_key.srv_cls_key));
        memcpy(gfphi_i->pri_key.prc_file_key.prc_file_id.prc_file_kind ,DEF_FL_KEY_MG
            ,sizeof(gfphi_i->pri_key.prc_file_key.prc_file_id.prc_file_kind));
        memset(gfphi_i->pri_key.prc_file_key.prc_file_id.prc_file_num,'0'
            ,sizeof(gfphi_i->pri_key.prc_file_key.prc_file_id.prc_file_num));
        memset(gfphi_i->pri_key.prc_file_key.prc_file_mlt_num,'0'
            ,sizeof(gfphi_i->pri_key.prc_file_key.prc_file_mlt_num));
    }
    memcpy(in_fl_inf->key_type ,DEF_COM_IOM_KEYTYPE_PRI ,sizeof(in_fl_inf->key_type));
    in_fl_inf->key_len            = (short)sizeof(gfphi_i->pri_key);
    in_fl_inf->compare_len        = (short)sizeof(gfphi_i->pri_key);
    in_fl_inf->positioning_mode   = DEF_COM_IOM_EXACT;
    in_fl_inf->lock_flg           = DEF_COM_IOM_NOLOCK;
    in_fl_inf->asc_desc_type      = DEF_COM_IOM_ASCEND;
    in_fl_inf->io_timer           = physical_info->io_timer;
    in_fl_inf->rec_len            = db_gfphi_def_Size;
    /* IOモジュール呼び出し */
    COM_IOM(DEF_COM_IOM_FUNC_STARTREAD
                , subprog_sts
                , &trace_inf
                , &file_inf
                , in_fl_inf
                , out_fl_inf );
    if(memcmp(subprog_sts ,DEF_COM_IOM_NO_ERR ,sizeof(subprog_sts)) == 0 ){
        return(DEF_NWM_ENI_RTN_OK);
    } else {
        /* EMS出力 */
        return(DEF_NWM_ENI_RTN_NG);
    }
} /* end of NWM_ENI_gfphi_read */
/****************************************************************************/
/*  FUNCTION        : 1.2.0  NWM_ENI_gckey_open                             */
/*  CALLING SEQ.    : short NWM_ENI_gckey_open  (struct *                   */
/*                                              ,struct *                   */
/*                                              ,struct *                   */
/*                                              ,char * )                   */
/*  ARGUMENT        : 1.key_file_info  (I/O) 鍵管理ファイル情報             */
/*                  : 2.in_fl_inf      (I)   入力情報                       */
/*                  : 3.out_fl_inf     (O)   出力情報                       */
/*                  : 4.prog_id        (I)   モジュールID                   */
/*  RETURN CODE     : 0 :正常                                               */
/*                  : -1:異常                                               */
/*  DESCRIPTION     : 物理名情報ファイルのオープンを行う                    */
/****************************************************************************/
short NWM_ENI_gckey_open(NWM_ENI_arg_2_def *key_file_info
                       , COM_IOM_arg_5_def *in_fl_inf
                       , COM_IOM_arg_6_def *out_fl_inf
                       , char *prog_id
                       , oggz1in_def *ems_info
                       , ems_info_add *ems_add)
{
    char   subprog_sts[2];
    COM_IOM_arg_3_def trace_inf;
    COM_IOM_arg_4_def file_inf;
    db_gfphi_def      *gfphi_o;              /* 物理名情報ファイルPRI-KEY */

    gfphi_o = (db_gfphi_def *)&out_fl_inf->rec_area[0];

    /* IOモジュール引数設定　トレース情報 */
    memset(trace_inf.prog_id      ,' ',sizeof(trace_inf.prog_id));
    memset(trace_inf.file_id      ,' ',sizeof(trace_inf.file_id));
    memset(trace_inf.file_name    ,' ',sizeof(trace_inf.file_name));
    memset(trace_inf.file_io_type ,' ',sizeof(trace_inf.file_io_type));
    memcpy(trace_inf.prog_id ,prog_id ,sizeof(trace_inf.prog_id));
    memcpy(trace_inf.file_id ,DEF_GCKEY ,sizeof(DEF_GCKEY)-1);
    memcpy(trace_inf.file_name ,gfphi_o->prc_file_info.prc_file_name
        ,sizeof(trace_inf.file_name));
    memcpy(trace_inf.file_io_type ,"OPEN" ,4);
    /* IOモジュール引数設定　ファイル情報 */
    memset(file_inf.file_id   ,' ',sizeof(file_inf.file_id));
    memset(file_inf.file_name ,' ',sizeof(file_inf.file_name));
    memcpy(file_inf.file_id ,DEF_GCKEY ,sizeof(DEF_GCKEY)-1);
    memcpy(file_inf.file_name ,gfphi_o->prc_file_info.prc_file_name
        ,sizeof(file_inf.file_name));
    /* IOモジュール引数設定　入力情報 */
    in_fl_inf->io_timer           = key_file_info->io_timer;
    /* IOモジュール呼び出し */
    COM_IOM(DEF_COM_IOM_FUNC_OPEN
                , subprog_sts
                , &trace_inf
                , &file_inf
                , in_fl_inf
                , out_fl_inf );
    /* 処理結果判定 */
    if(memcmp(subprog_sts ,DEF_COM_IOM_NO_ERR ,sizeof(subprog_sts)) != 0 ){
        /* EMS出力 */
        return(DEF_NWM_ENI_RTN_NG);
    }
    /* 引数．鍵管理ファイル情報の設定 */
    memcpy(key_file_info->file_id   ,file_inf.file_id   ,sizeof(key_file_info->file_id));
    memcpy(key_file_info->file_name ,file_inf.file_name ,sizeof(key_file_info->file_name));
    key_file_info->file_no = file_inf.file_no;
    return(DEF_NWM_ENI_RTN_OK);
} /* end of NWM_ENI_gckey_open */

