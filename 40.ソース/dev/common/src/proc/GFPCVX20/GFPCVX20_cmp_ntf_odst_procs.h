#pragma once
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
/* USER HEADER     */
#include <limit.h>
#include "GFPCVX20_fileIO.h" nolist
#include "GFPCVX20_io_mem.h" nolist
#include "GFPCVX20_thread_header.h" nolist
#ifdef _TANDEM_SOURCE
#ifndef __db_gflin_def__
#define __db_gflin_def__
#include <file.h(db_gflin)>  nolist
#endif
#ifndef __db_gfnwi_def__
#define __db_gfnwi_def__
#include <file.h(db_gfnwi)>  nolist
#endif
#ifndef __db_gfphi_def__
#define __db_gfphi_def__
#include <file.h(db_gfphi)>  nolist
#endif
#else
#ifndef __file_h__
#define __file_h__
#include <file.h> nolist
#endif
#endif

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define QUALIFIER_FOR_PSNMSDSO "#CONN"  // 電文振分(outbound)向けqualifier

/****************************************************************************/
/*   外部データ定義                                                         */
/****************************************************************************/
#pragma fieldalign shared2 __cmp_ntf_odst_procs_t
typedef struct __cmp_ntf_odst_procs_t
{
    size_t           out_dist_procs_count;  // 電文振分(outbound)プロセス数
    thread_object_t *odst_procs[CCC_MAX_outbound];
    io_mem_q_t       request_list;          // 通知待ちキュー
} cmp_ntf_odst_procs_t;

/****************************************************************************/
/*   関数定義                                                             */
/****************************************************************************/
thread_object_t *cncl_create_ntf_odst_procs(db_gfphi_def *out_dist_procs, size_t out_dist_procs_count);
short cncl_ntf_odst_procs_add_req(thread_object_t *thread_ntf_odst, thread_object_t *caller_transport,
                                  io_trace_buf_t *request, bool open_request_required);
void cncl_ntf_odst_procs_complete_callback(thread_object_t *ntf_odst_procs_thread, iobuf_page_t *request);
