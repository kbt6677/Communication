#pragma section lk_zaa1601_arg_1
/* Definition LK-ZAA1601-ARG-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_zaa1601_arg_1
typedef struct __lk_zaa1601_arg_1
{
   unsigned short                  ic_data_len;
   char                            ic_data[340];
   char                            search_tag[4];
} lk_zaa1601_arg_1_def;
#define lk_zaa1601_arg_1_def_Size 346
#pragma section lk_zaa1601_arg_2
/* Definition LK-ZAA1601-ARG-2 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_zaa1601_arg_2
typedef struct __lk_zaa1601_arg_2
{
   unsigned short                  tlv_data_len;
   char                            tlv_data[340];
   unsigned short                  result_code;
} lk_zaa1601_arg_2_def;
#define lk_zaa1601_arg_2_def_Size 344
#pragma section lk_zab1401_arg_1
/* Definition LK-ZAB1401-ARG-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_zab1401_arg_1
typedef struct __lk_zab1401_arg_1
{
   char                            file_id[20];
} lk_zab1401_arg_1_def;
#define lk_zab1401_arg_1_def_Size 20
#pragma section lk_zab1401_arg_2
/* Definition LK-ZAB1401-ARG-2 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_zab1401_arg_2
typedef struct __lk_zab1401_arg_2
{
   char                            file_name[47];
} lk_zab1401_arg_2_def;
#define lk_zab1401_arg_2_def_Size 47
#pragma section lk_zac1201_arg_1
/* Definition LK-ZAC1201-ARG-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_zac1201_arg_1
typedef struct __lk_zac1201_arg_1
{
   char                            func_type[4];
} lk_zac1201_arg_1_def;
#define lk_zac1201_arg_1_def_Size 4
#pragma section lk_zac1201_arg_2
/* Definition LK-ZAC1201-ARG-2 created on 05/16/2024 at 11:43 */
#include <tnsint.h>
#pragma fieldalign shared2 __lk_zac1201_arg_2
typedef struct __lk_zac1201_arg_2
{
   __int32_t                       transaction_id;
} lk_zac1201_arg_2_def;
#define lk_zac1201_arg_2_def_Size 4
#pragma section lk_zac1201_arg_3
/* Definition LK-ZAC1201-ARG-3 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_zac1201_arg_3
typedef struct __lk_zac1201_arg_3
{
   short                           shori_result;
} lk_zac1201_arg_3_def;
#define lk_zac1201_arg_3_def_Size 2
#pragma section lk_iocorem_arg_1
/* Definition LK-IOCOREM-ARG-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_iocorem_arg_1
typedef struct __lk_iocorem_arg_1
{
   char                            func_type[4];
} lk_iocorem_arg_1_def;
#define lk_iocorem_arg_1_def_Size 4
#pragma section lk_iocorem_arg_2
/* Definition LK-IOCOREM-ARG-2 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_iocorem_arg_2
typedef struct __lk_iocorem_arg_2
{
   char                            sub_prog_sts[2];
} lk_iocorem_arg_2_def;
#define lk_iocorem_arg_2_def_Size 2
#pragma section lk_iocorem_arg_3
/* Definition LK-IOCOREM-ARG-3 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_iocorem_arg_3
typedef struct __lk_iocorem_arg_3
{
   char                            prog_id[8];
   char                            file_id[8];
   char                            file_name[47];
   char                            file_io_type[8];
} lk_iocorem_arg_3_def;
#define lk_iocorem_arg_3_def_Size 71
#pragma section lk_iocorem_arg_4
/* Definition LK-IOCOREM-ARG-4 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_iocorem_arg_4
typedef struct __lk_iocorem_arg_4
{
   char                            file_id[8];
   char                            file_name[47];
   char                            future_use;
   short                           file_no;
} lk_iocorem_arg_4_def;
#define lk_iocorem_arg_4_def_Size 58
#pragma section lk_iocorem_arg_5
/* Definition LK-IOCOREM-ARG-5 created on 05/16/2024 at 11:43 */
#include <tnsint.h>
#pragma fieldalign shared2 __lk_iocorem_arg_5
typedef struct __lk_iocorem_arg_5
{
   char                            part_key_type;
   char                            future_use;
   short                           part_key_position;
   short                           part_key_len;
   char                            key_value[150];
   char                            key_type[2];
   short                           key_len;
   short                           compare_len;
   short                           positioning_mode;
   char                            lock_flg;
   char                            asc_desc_type;
   __int32_t                       io_timer;
   __int32_t                       rec_len;
   char                            rec_area[20000];
} lk_iocorem_arg_5_def;
#define lk_iocorem_arg_5_def_Size 20094
#pragma section lk_iocorem_arg_6
/* Definition LK-IOCOREM-ARG-6 created on 05/16/2024 at 11:43 */
#include <tnsint.h>
#pragma fieldalign shared2 __lk_iocorem_arg_6
typedef struct __lk_iocorem_arg_6
{
   short                           guardian_errcode;
   char                            err_proc[30];
   char                            file_name[47];
   char                            future_use;
   __int32_t                       rec_len;
   char                            rec_area[20000];
} lk_iocorem_arg_6_def;
#define lk_iocorem_arg_6_def_Size 20084
#pragma section lk_iocoreq_arg_1
/* Definition LK-IOCOREQ-ARG-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_iocoreq_arg_1
typedef struct __lk_iocoreq_arg_1
{
   char                            func_type[4];
} lk_iocoreq_arg_1_def;
#define lk_iocoreq_arg_1_def_Size 4
#pragma section lk_iocoreq_arg_2
/* Definition LK-IOCOREQ-ARG-2 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_iocoreq_arg_2
typedef struct __lk_iocoreq_arg_2
{
   char                            sub_prog_sts[2];
} lk_iocoreq_arg_2_def;
#define lk_iocoreq_arg_2_def_Size 2
#pragma section lk_iocoreq_arg_3
/* Definition LK-IOCOREQ-ARG-3 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_iocoreq_arg_3
typedef struct __lk_iocoreq_arg_3
{
   char                            prog_id[8];
   char                            file_id[8];
   char                            file_name[47];
   char                            file_io_type[8];
} lk_iocoreq_arg_3_def;
#define lk_iocoreq_arg_3_def_Size 71
#pragma section lk_iocoreq_arg_4
/* Definition LK-IOCOREQ-ARG-4 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_iocoreq_arg_4
typedef struct __lk_iocoreq_arg_4
{
   char                            file_id[8];
   char                            file_name[47];
   char                            future_use;
   short                           file_no;
} lk_iocoreq_arg_4_def;
#define lk_iocoreq_arg_4_def_Size 58
#pragma section lk_iocoreq_arg_5
/* Definition LK-IOCOREQ-ARG-5 created on 05/16/2024 at 11:43 */
#include <tnsint.h>
#pragma fieldalign shared2 __lk_iocoreq_arg_5
typedef struct __lk_iocoreq_arg_5
{
   char                            key_value[150];
   short                           key_len;
   short                           compare_len;
   short                           positioning_mode;
   __int32_t                       io_timer;
   short                           rec_len;
   char                            rec_area[20000];
} lk_iocoreq_arg_5_def;
#define lk_iocoreq_arg_5_def_Size 20082
#pragma section lk_iocoreq_arg_6
/* Definition LK-IOCOREQ-ARG-6 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_iocoreq_arg_6
typedef struct __lk_iocoreq_arg_6
{
   short                           guardian_errcode;
   char                            err_proc[30];
   char                            file_name[47];
   char                            future_use;
   short                           rec_len;
   char                            rec_area[20000];
} lk_iocoreq_arg_6_def;
#define lk_iocoreq_arg_6_def_Size 20082
#pragma section lk_iofilem_arg_1
/* Definition LK-IOFILEM-ARG-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_iofilem_arg_1
typedef struct __lk_iofilem_arg_1
{
   char                            func_type[4];
} lk_iofilem_arg_1_def;
#define lk_iofilem_arg_1_def_Size 4
#pragma section lk_iofilem_arg_2
/* Definition LK-IOFILEM-ARG-2 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_iofilem_arg_2
typedef struct __lk_iofilem_arg_2
{
   char                            sub_prog_sts[2];
} lk_iofilem_arg_2_def;
#define lk_iofilem_arg_2_def_Size 2
#pragma section lk_iofilem_arg_3
/* Definition LK-IOFILEM-ARG-3 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_iofilem_arg_3
typedef struct __lk_iofilem_arg_3
{
   char                            prog_id[8];
} lk_iofilem_arg_3_def;
#define lk_iofilem_arg_3_def_Size 8
#pragma section lk_iofilem_arg_4
/* Definition LK-IOFILEM-ARG-4 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_iofilem_arg_4
typedef struct __lk_iofilem_arg_4
{
   char                            key_value[150];
   char                            key_type[2];
   char                            key_len[3];
   char                            compare_len[3];
   char                            positioning_mode;
   char                            lock_flg;
   char                            asc_desc_type;
   char                            io_timer[5];
   char                            future_use;
   char                            rec_len[5];
   char                            rec_area[20000];
} lk_iofilem_arg_4_def;
#define lk_iofilem_arg_4_def_Size 20092
#pragma section lk_iofilem_arg_5
/* Definition LK-IOFILEM-ARG-5 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_iofilem_arg_5
typedef struct __lk_iofilem_arg_5
{
   char                            guardian_errcode[4];
   char                            err_proc[30];
   char                            file_name[47];
   char                            rec_len[5];
   char                            rec_area[20000];
} lk_iofilem_arg_5_def;
#define lk_iofilem_arg_5_def_Size 20086
#pragma section lk_zac2001r_arg_1
/* Definition LK-ZAC2001R-ARG-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_zac2001r_arg_1
typedef struct __lk_zac2001r_arg_1
{
   char                            func_flg;
   struct
   {
      char                            prog_id[8];
      char                            file_id[8];
      char                            file_name[47];
      char                            file_io_type[8];
      char                            guardian_errcode[4];
      char                            shori_start_time[12];
      char                            shori_end_time[12];
   } trace_info;
   struct
   {
      char                            rec_len[5];
      char                            rec_area[27000];
   } data_info;
} lk_zac2001r_arg_1_def;
#define lk_zac2001r_arg_1_def_Size 27105
#pragma section lk_zac2001t_arg_1
/* Definition LK-ZAC2001T-ARG-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_zac2001t_arg_1
typedef struct __lk_zac2001t_arg_1
{
   char                            func_flg;
   struct
   {
      char                            prog_id[8];
      char                            file_id[8];
      char                            file_name[47];
      char                            file_io_type[8];
      char                            guardian_errcode[4];
      char                            shori_start_time[12];
      char                            shori_end_time[12];
   } trace_info;
} lk_zac2001t_arg_1_def;
#define lk_zac2001t_arg_1_def_Size 100
#pragma section lk_zac2001f_arg_1
/* Definition LK-ZAC2001F-ARG-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_zac2001f_arg_1
typedef struct __lk_zac2001f_arg_1
{
   char                            func_flg;
   struct
   {
      char                            prog_id[8];
      char                            file_id[8];
      char                            file_name[47];
      char                            file_io_type[8];
   } trace_info;
   struct
   {
      char                            key_type[2];
      char                            key_value[150];
      char                            next_revs_type[4];
      char                            guardian_errcode[4];
      char                            rec_len[5];
      char                            rec_area[20000];
      char                            shori_start_time[12];
      char                            shori_end_time[12];
   } data_info;
} lk_zac2001f_arg_1_def;
#define lk_zac2001f_arg_1_def_Size 20181
#pragma section lk_zac2001p_arg_1
/* Definition LK-ZAC2001P-ARG-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_zac2001p_arg_1
typedef struct __lk_zac2001p_arg_1
{
   char                            func_flg;
   struct
   {
      char                            prog_id[8];
      char                            file_id[8];
      char                            file_name[47];
      char                            file_io_type[8];
      char                            guardian_errcode[4];
      char                            shori_start_time[12];
      char                            shori_end_time[12];
   } trace_info;
   struct
   {
      char                            rec_len[5];
      char                            rec_area[27000];
   } data_info;
} lk_zac2001p_arg_1_def;
#define lk_zac2001p_arg_1_def_Size 27105
#pragma section lk_zac2001i_arg_1
/* Definition LK-ZAC2001I-ARG-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_zac2001i_arg_1
typedef struct __lk_zac2001i_arg_1
{
   char                            func_flg;
   struct
   {
      char                            prog_id[8];
      char                            file_id[8];
      char                            file_name[47];
      char                            file_io_type[8];
      char                            guardian_errcode[4];
      char                            shori_start_time[12];
      char                            shori_end_time[12];
   } trace_info;
   struct
   {
      char                            rec_len[5];
      char                            rec_area[27000];
   } data_info;
} lk_zac2001i_arg_1_def;
#define lk_zac2001i_arg_1_def_Size 27105
#pragma section lk_zac2001r9_arg_1
/* Definition LK-ZAC2001R9-ARG-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_zac2001r9_arg_1
typedef struct __lk_zac2001r9_arg_1
{
   char                            func_flg;
   struct
   {
      char                            prog_id[8];
      char                            file_id[8];
      char                            file_name[42];
      char                            split_pos[5];
      char                            file_io_type[8];
      char                            guardian_errcode[4];
      char                            shori_start_time[12];
      char                            shori_end_time[12];
   } trace_info;
   struct
   {
      char                            rec_len[5];
      char                            rec_area[54000];
   } data_info;
} lk_zac2001r9_arg_1_def;
#define lk_zac2001r9_arg_1_def_Size 54105
#pragma section lk_zac2001p9_arg_1
/* Definition LK-ZAC2001P9-ARG-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_zac2001p9_arg_1
typedef struct __lk_zac2001p9_arg_1
{
   char                            func_flg;
   struct
   {
      char                            prog_id[8];
      char                            file_id[8];
      char                            file_name[42];
      char                            split_pos[5];
      char                            file_io_type[8];
      char                            guardian_errcode[4];
      char                            shori_start_time[12];
      char                            shori_end_time[12];
   } trace_info;
   struct
   {
      char                            rec_len[5];
      char                            rec_area[54000];
   } data_info;
} lk_zac2001p9_arg_1_def;
#define lk_zac2001p9_arg_1_def_Size 54105
#pragma section lk_zac2001i9_arg_1
/* Definition LK-ZAC2001I9-ARG-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_zac2001i9_arg_1
typedef struct __lk_zac2001i9_arg_1
{
   char                            func_flg;
   struct
   {
      char                            prog_id[8];
      char                            file_id[8];
      char                            file_name[42];
      char                            split_pos[5];
      char                            file_io_type[8];
      char                            guardian_errcode[4];
      char                            shori_start_time[12];
      char                            shori_end_time[12];
   } trace_info;
   struct
   {
      char                            rec_len[5];
      char                            rec_area[54000];
   } data_info;
} lk_zac2001i9_arg_1_def;
#define lk_zac2001i9_arg_1_def_Size 54105

#pragma section lk_othchg_arg_1
/* Definition LK-OTHCHG-ARG-1 created on 05/16/2024 at 11:43 */
typedef struct __lk_othchg_arg_1
{
    char                           othchg_cvsrc_inbuf[999];
} lk_othchg_arg_1_def;
#define lk_othchg_arg_arg_1_def_Size 999
#pragma section lk_othchg_arg_2
/* Definition LK-OTHCHG-ARG-2 created on 05/16/2024 at 11:43 */
typedef struct __lk_othchg_arg_2
{
    char                           othchg_cvsrc_outbuf[999];
} lk_othchg_arg_2_def;
#pragma section lk_othchg_arg_3
/* Definition LK-OTHCHG-ARG-3 created on 05/16/2024 at 11:43 */
typedef struct __lk_othchg_arg_3
{
    short                          othchg_inbuf_chglen;
} lk_othchg_arg_3_def;
#define lk_othchg_arg_arg_3_def_Size 2

#pragma section lk_char2bcd_arg_1
/* Definition LK-CHAR2BCD-ARG-1 created on 05/16/2024 at 11:43 */
typedef struct __lk_char2bcd_arg_1
{
    char                           c2b_cvsrc_char_inbuf[300];
} lk_char2bcd_arg_1_def;
#define lk_char2bcd_arg_1_def_Size 300
#pragma section lk_char2bcd_arg_2
/* Definition LK-CHAR2BCD-ARG-2 created on 05/16/2024 at 11:43 */
typedef struct __lk_char2bcd_arg_2
{
    char                           c2b_cvsrc_bcd_inlen[3];
} lk_char2bcd_arg_2_def;
#define lk_char2bcd_arg_2_def_Size 3

#pragma section lk_char2bcd_arg_3
/* Definition LK-CHAR2BCD-ARG-3 created on 05/16/2024 at 11:43 */
typedef struct __lk_char2bcd_arg_3
{
    char                           c2b_cvsrc_bcd_outbuf[150];
} lk_char2bcd_arg_3_def;
#define lk_char2bcd_arg_3_def_Size 150

#pragma section lk_char2bcd_arg_4
/* Definition LK-CHAR2BCD-ARG-4 created on 05/16/2024 at 11:43 */
typedef struct __lk_char2bcd_arg_4
{
    char                           c2b_return_code;
} lk_char2bcd_arg_4_def;
#define lk_char2bcd_arg_4_def_Size 1

#pragma section lk_char2hex_arg_1
/* Definition LK-CHAR2HEX-ARG-1 created on 05/16/2024 at 11:43 */
typedef struct __lk_char2hex_arg_1
{
    char                           c2h_cvsrc_char_inbuf[80];
} lk_char2hex_arg_1_def;
#define lk_char2hex_arg_1_def_Size 80
#pragma section lk_char2hex_arg_2
/* Definition LK-CHAR2HEX-ARG-2 created on 05/16/2024 at 11:43 */
typedef struct __lk_char2hex_arg_2
{
    char                           c2h_cvsrc_hex_outbuf[40];
} lk_char2hex_arg_2_def;
#define lk_char2hex_arg_2_def_Size 40
#pragma section lk_char2hex_arg_3
/* Definition LK-CHAR2HEX-ARG-3 created on 05/16/2024 at 11:43 */
typedef struct __lk_char2hex_arg_3
{
    short                          c2h_inbuf_chglen;
} lk_char2hex_arg_3_def;
#define lk_char2hex_arg_3_def_Size 2
#pragma section lk_zaa1201_arg_1
/* Definition LK-ZAA1201-ARG-1 created on 05/16/2024 at 11:43 */
#include <tnsint.h>
#pragma fieldalign shared2 __lk_zaa1201_arg_1
typedef struct __lk_zaa1201_arg_1
{
   __uint32_t                      bef_data_len;
   char                            bef_data[340];
} lk_zaa1201_arg_1_def;
#define lk_zaa1201_arg_1_def_Size 344
#pragma section lk_zaa1201_arg_2
/* Definition LK-ZAA1201-ARG-2 created on 05/16/2024 at 11:43 */
#include <tnsint.h>
#pragma fieldalign shared2 __lk_zaa1201_arg_2
typedef struct __lk_zaa1201_arg_2
{
   __uint32_t                      aft_data_len;
   char                            aft_data[340];
} lk_zaa1201_arg_2_def;
#define lk_zaa1201_arg_2_def_Size 344
#pragma section lk_zaa1201_arg_3
/* Definition LK-ZAA1201-ARG-3 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_zaa1201_arg_3
typedef struct __lk_zaa1201_arg_3
{
   char                            ret_code;
} lk_zaa1201_arg_3_def;
#define lk_zaa1201_arg_3_def_Size 1
#pragma section lk_gfpogg40_arg_1
/* Definition LK-GFPOGG40-ARG-1 created on 05/16/2024 at 11:43 */
#include <tnsint.h>
#pragma fieldalign shared2 __lk_gfpogg40_arg_1
typedef struct __lk_gfpogg40_arg_1
{
   __uint32_t                      bef_data_len;
   char                            bef_data[340];
} lk_gfpogg40_arg_1_def;
#define lk_gfpogg40_arg_1_def_Size 344
#pragma section lk_gfpogg40_arg_2
/* Definition LK-GFPOGG40-ARG-2 created on 05/16/2024 at 11:43 */
#include <tnsint.h>
#pragma fieldalign shared2 __lk_gfpogg40_arg_2
typedef struct __lk_gfpogg40_arg_2
{
   __uint32_t                      aft_data_len;
   char                            aft_data[340];
} lk_gfpogg40_arg_2_def;
#define lk_gfpogg40_arg_2_def_Size 344
#pragma section lk_gfpogg40_arg_3
/* Definition LK-GFPOGG40-ARG-3 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg40_arg_3
typedef struct __lk_gfpogg40_arg_3
{
   char                            ret_code;
} lk_gfpogg40_arg_3_def;
#define lk_gfpogg40_arg_3_def_Size 1
#pragma section cns_zaa1601
/* Definition CNS-ZAA1601 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __cns_zaa1601
typedef struct __cns_zaa1601
{
   struct
   {
      char                            cns_result_normal_end;
      /*value is 0*/
      char                            cns_result_err;
      /*value is 1*/
   } cns_result_code;
} cns_zaa1601_def;
#define cns_zaa1601_def_Size 2
#pragma section cns_zac1201
/* Definition CNS-ZAC1201 created on 05/16/2024 at 11:43 */
#include <tnsint.h>
#pragma fieldalign shared2 __cns_zac1201
typedef struct __cns_zac1201
{
   struct
   {
      char                            cns_func_begin_tran[4];
      /*value is "BT  "*/
      char                            cns_func_end_tran[4];
      /*value is "ET  "*/
      char                            cns_func_abort_tran[4];
      /*value is "AT  "*/
      char                            cns_func_resume_tran[4];
      /*value is "RT  "*/
      char                            cns_func_resume_tran_clear[4];
      /*value is "RTC "*/
      char                            cns_func_tranid_get[4];
      /*value is "GTID"*/
      char                            cns_func_tranid_active[4];
      /*value is "RTTX"*/
   } cns_func_type;
   struct
   {
      __int32_t                       cns_shori_normal_end;
      /*value is 0*/
      __int32_t                       cns_shori_err_param;
      /*value is -1*/
   } cns_shori_result;
} cns_zac1201_def;
#define cns_zac1201_def_Size 36
#pragma section cns_iocorem
/* Definition CNS-IOCOREM created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __cns_iocorem
typedef struct __cns_iocorem
{
   struct
   {
      char                            cns_func_open[4];
      /*value is "OPEN"*/
      char                            cns_func_startread[4];
      /*value is "SRED"*/
      char                            cns_func_nextread[4];
      /*value is "NRED"*/
      char                            cns_func_add[4];
      /*value is "ADD "*/
      char                            cns_func_update[4];
      /*value is "UPDT"*/
      char                            cns_func_delete[4];
      /*value is "DELT"*/
      char                            cns_func_unlock[4];
      /*value is "ULOC"*/
      char                            cns_func_close[4];
      /*value is "CLOS"*/
   } cns_func_type;
   struct
   {
      char                            cns_normal_end[2];
      /*value is "00"*/
      char                            cns_err_open[2];
      /*value is "Z1"*/
      char                            cns_err_eof[2];
      /*value is "Z2"*/
      char                            cns_err_read[2];
      /*value is "Z3"*/
      char                            cns_err_write[2];
      /*value is "Z4"*/
      char                            cns_err_timeout[2];
      /*value is "Z5"*/
      char                            cns_err_discfull[2];
      /*value is "Z6"*/
      char                            cns_err_duplicate[2];
      /*value is "Z7"*/
      char                            cns_err_other[2];
      /*value is "Z9"*/
   } cns_sub_prog_sts;
   struct
   {
      char                            cns_fileio_open[8];
      /*value is "OPEN    "*/
      char                            cns_fileio_start[8];
      /*value is "START   "*/
      char                            cns_fileio_read[8];
      /*value is "READ    "*/
      char                            cns_fileio_write[8];
      /*value is "WRITE   "*/
      char                            cns_fileio_rewrite[8];
      /*value is "REWRITE "*/
      char                            cns_fileio_delete[8];
      /*value is "DELETE  "*/
      char                            cns_fileio_unlock[8];
      /*value is "UNLOCK  "*/
      char                            cns_fileio_close[8];
      /*value is "CLOSE   "*/
   } cns_fileio_type;
   struct
   {
      char                            cns_part_key_none;
      /*value is 0*/
      char                            cns_part_key_fix;
      /*value is 1*/
      char                            cns_part_key_mod;
      /*value is 2*/
   } cns_part_key_type;
   struct
   {
      char                            cns_key_primary[2];
      /*value is "10"*/
      char                            cns_key_altkey1[2];
      /*value is "01"*/
      char                            cns_key_altkey2[2];
      /*value is "02"*/
      char                            cns_key_altkey3[2];
      /*value is "03"*/
      char                            cns_key_altkey4[2];
      /*value is "04"*/
      char                            cns_key_altkey5[2];
      /*value is "05"*/
      char                            cns_key_altkey6[2];
      /*value is "06"*/
      char                            cns_key_altkey7[2];
      /*value is "07"*/
      char                            cns_key_altkey8[2];
      /*value is "08"*/
      char                            cns_key_altkey9[2];
      /*value is "09"*/
   } cns_key_type;
   char                            filler_0;
   struct
   {
      short                           cns_position_approximate;
      /*value is 0*/
      short                           cns_position_generic;
      /*value is 1*/
      short                           cns_position_exact;
      /*value is 2*/
   } cns_positioning_mode;
   struct
   {
      char                            cns_lock_none;
      /*value is 0*/
      char                            cns_lock_on;
      /*value is 1*/
      char                            cns_lock_off;
      /*value is 2*/
   } cns_lock_flg;
   struct
   {
      char                            cns_asc;
      /*value is 0*/
      char                            cns_desc;
      /*value is 1*/
   } cns_asc_desc_type;
} cns_iocorem_def;
#define cns_iocorem_def_Size 149
#pragma section cns_iofilem
/* Definition CNS-IOFILEM created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __cns_iofilem
typedef struct __cns_iofilem
{
   struct
   {
      char                            cns_func_open[4];
      /*value is "OPEN"*/
      char                            cns_func_startread[4];
      /*value is "SRED"*/
      char                            cns_func_nextread[4];
      /*value is "NRED"*/
      char                            cns_func_add[4];
      /*value is "ADD "*/
      char                            cns_func_update[4];
      /*value is "UPDT"*/
      char                            cns_func_delete[4];
      /*value is "DELT"*/
      char                            cns_func_unlock[4];
      /*value is "ULOC"*/
      char                            cns_func_close[4];
      /*value is "CLOS"*/
      char                            cns_func_readadd[4];
      /*value is "RADD"*/
      char                            cns_func_readupdate[4];
      /*value is "RUPD"*/
   } cns_func_type;
   struct
   {
      char                            cns_normal_end[2];
      /*value is "00"*/
      char                            cns_err_open[2];
      /*value is "Z1"*/
      char                            cns_err_eof[2];
      /*value is "Z2"*/
      char                            cns_err_read[2];
      /*value is "Z3"*/
      char                            cns_err_write[2];
      /*value is "Z4"*/
      char                            cns_err_timeout[2];
      /*value is "Z5"*/
      char                            cns_err_discfull[2];
      /*value is "Z6"*/
      char                            cns_err_duplicate[2];
      /*value is "Z7"*/
      char                            cns_err_other[2];
      /*value is "Z9"*/
   } cns_sub_prog_sts;
   struct
   {
      char                            cns_key_primary[2];
      /*value is "10"*/
      char                            cns_key_altkey1[2];
      /*value is "01"*/
      char                            cns_key_altkey2[2];
      /*value is "02"*/
      char                            cns_key_altkey3[2];
      /*value is "03"*/
      char                            cns_key_altkey4[2];
      /*value is "04"*/
      char                            cns_key_altkey5[2];
      /*value is "05"*/
      char                            cns_key_altkey6[2];
      /*value is "06"*/
      char                            cns_key_altkey7[2];
      /*value is "07"*/
      char                            cns_key_altkey8[2];
      /*value is "08"*/
      char                            cns_key_altkey9[2];
      /*value is "09"*/
   } cns_key_type;
   struct
   {
      char                            cns_position_approximate;
      /*value is 0*/
      char                            cns_position_generic;
      /*value is 1*/
      char                            cns_position_exact;
      /*value is 2*/
   } cns_positioning_mode;
   struct
   {
      char                            cns_lock_none;
      /*value is 0*/
      char                            cns_lock_on;
      /*value is 1*/
      char                            cns_lock_off;
      /*value is 2*/
   } cns_lock_flg;
   struct
   {
      char                            cns_asc;
      /*value is 0*/
      char                            cns_desc;
      /*value is 1*/
   } cns_asc_desc_type;
} cns_iofilem_def;
#define cns_iofilem_def_Size 86
#pragma section cns_zac2001
/* Definition CNS-ZAC2001 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __cns_zac2001
typedef struct __cns_zac2001
{
   struct
   {
      char                            cns_func_start;
      /*value is 0*/
      char                            cns_func_ems_out;
      /*value is 1*/
      char                            cns_func_end;
      /*value is 2*/
   } cns_func_flg;
   struct
   {
      char                            cns_fileio_open[8];
      /*value is "OPEN    "*/
      char                            cns_fileio_start[8];
      /*value is "START   "*/
      char                            cns_fileio_read[8];
      /*value is "READ    "*/
      char                            cns_fileio_write[8];
      /*value is "WRITE   "*/
      char                            cns_fileio_rewrite[8];
      /*value is "REWRITE "*/
      char                            cns_fileio_delete[8];
      /*value is "DELETE  "*/
      char                            cns_fileio_unlock[8];
      /*value is "UNLOCK  "*/
      char                            cns_fileio_close[8];
      /*value is "CLOSE   "*/
      char                            cns_fileio_begin_tran[8];
      /*value is "B-T     "*/
      char                            cns_fileio_end_tran[8];
      /*value is "E-T     "*/
      char                            cns_fileio_resume_tran[8];
      /*value is "R-T     "*/
      char                            cns_fileio_abort_tran[8];
      /*value is "A-T     "*/
   } cns_fileio_type;
   struct
   {
      char                            cns_next[4];
      /*value is "NEXT"*/
      char                            cns_revs[4];
      /*value is "REVS"*/
   } cns_next_revs_type;
} cns_zac2001_def;
#define cns_zac2001_def_Size 107
#pragma section db_altrc
/* Record DB-ALTRC created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __db_altrc
typedef struct __db_altrc
{
   struct
   {
      char                            shori_yyyymmdd[8];
      char                            shori_hhmmss[6];
      char                            milli_micro_second[6];
      char                            process_name[8];
   } pri_key;
   struct
   {
      char                            file_id[8];
      char                            file_name[47];
      char                            shori_func_name[32];
      char                            module_id[8];
      char                            file_io_type[8];
      char                            guardian_errcode[4];
      char                            start_time[12];
      char                            shori_end_time[12];
      char                            shori_end[8];
   } trace_info;
   struct
   {
      char                            key_type[2];
      char                            key_value[150];
      char                            order_read_sitei[4];
      char                            record_len[5];
      char                            record_content[27000];
   } data_info;
   char                            future_use[245];
} db_altrc_def;
#define db_altrc_def_Size 27493
#pragma section lk_gfpogg09_io_1
/* Definition LK-GFPOGG09-IO-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg09_io_1
typedef struct __lk_gfpogg09_io_1
{
   unsigned short                  ic_data_len;
   char                            ic_data[256];
   char                            search_tag[2];
   unsigned short                  tlv_data_len;
   char                            tlv_data[256];
} lk_gfpogg09_io_1_def;
#define lk_gfpogg09_io_1_def_Size 518
#pragma section lk_gfpogg09_io_2
/* Definition LK-GFPOGG09-IO-2 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg09_io_2
typedef struct __lk_gfpogg09_io_2
{
   short                           return_code;
} lk_gfpogg09_io_2_def;
#define lk_gfpogg09_io_2_def_Size 2
#pragma section lk_gfpogg11_io_1
/* Definition LK-GFPOGG11-IO-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg11_io_1
typedef struct __lk_gfpogg11_io_1
{
   char                            datetime_format;
   char                            base_date[8];
   char                            rtn_datetime[20];
} lk_gfpogg11_io_1_def;
#define lk_gfpogg11_io_1_def_Size 29
#pragma section lk_gfpogg11_io_2
/* Definition LK-GFPOGG11-IO-2 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg11_io_2
typedef struct __lk_gfpogg11_io_2
{
   short                           return_code;
} lk_gfpogg11_io_2_def;
#define lk_gfpogg11_io_2_def_Size 2
#pragma section lk_gfpogg12_io_1
/* Definition LK-GFPOGG12-IO-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg12_io_1
typedef struct __lk_gfpogg12_io_1
{
   char                            base_datetime[20];
   char                            calc_unit;
   char                            interval[5];
   char                            add_sub_type;
   char                            rtn_datetime[20];
} lk_gfpogg12_io_1_def;
#define lk_gfpogg12_io_1_def_Size 47
#pragma section lk_gfpogg12_io_2
/* Definition LK-GFPOGG12-IO-2 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg12_io_2
typedef struct __lk_gfpogg12_io_2
{
   short                           return_code;
} lk_gfpogg12_io_2_def;
#define lk_gfpogg12_io_2_def_Size 2
#pragma section lk_gfpogg13_io_1
/* Definition LK-GFPOGG13-IO-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg13_io_1
typedef struct __lk_gfpogg13_io_1
{
   char                            datetime_1[20];
   char                            datetime_2[20];
   char                            compa_unit;
   char                            rtn_interval[8];
} lk_gfpogg13_io_1_def;
#define lk_gfpogg13_io_1_def_Size 49
#pragma section lk_gfpogg13_io_2
/* Definition LK-GFPOGG13-IO-2 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg13_io_2
typedef struct __lk_gfpogg13_io_2
{
   short                           return_code;
} lk_gfpogg13_io_2_def;
#define lk_gfpogg13_io_2_def_Size 2
#pragma section lk_gfpogg14_io_1
/* Definition LK-GFPOGG14-IO-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg14_io_1
typedef struct __lk_gfpogg14_io_1
{
   char                            chk_date[8];
} lk_GFPoGG14_io_1_def;
#define lk_gfpogg14_io_1_def_Size 8
#pragma section lk_gfpogg14_io_2
/* Definition LK-GFPOGG14-IO-2 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg14_io_2
typedef struct __lk_gfpogg14_io_2
{
   short                           return_code;
} lk_GFPoGG14_io_2_def;
#define lk_gfpogg14_io_2_def_Size 2
#pragma section lk_gfpogg06_io_1
/* Definition LK-GFPOGG06-IO-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg06_io_1
typedef struct __lk_gfpogg06_io_1
{
   char                            iso_field_info[2500];
   char                            iso_msg[9999];
   char                            field_info[1024];
} lk_gfpogg06_io_1_def;
#define lk_gfpogg06_io_1_def_Size 13523
#pragma section lk_gfpogg06_io_2
/* Definition LK-GFPOGG06-IO-2 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg06_io_2
typedef struct __lk_gfpogg06_io_2
{
   char                            return_code;
} lk_gfpogg06_io_2_def;
#define lk_gfpogg06_io_2_def_Size 1
#pragma section lk_gfpogg07_io
/* Definition LK-GFPOGG07-IO created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg07_io
typedef struct __lk_gfpogg07_io
{
   char                            tlv_data[6000];
   char                            tlv_data_len[4];
   char                            tag_name[30];
   char                            tag_name_len[2];
   char                            len_digits[4];
   char                            data_type;
   char                            value_len[5];
   char                            value_buf[2000];
} lk_gfpogg07_io_def;
#define lk_gfpogg07_io_def_Size 8048
#pragma section lk_gfpogg08_io
/* Definition LK-GFPOGG08-IO created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg08_io
typedef struct __lk_gfpogg08_io
{
   char                            tlv_data[6000];
   char                            tlv_data_len[4];
   char                            tag_name[30];
   char                            tag_name_len[2];
   char                            len_digits[4];
   char                            data_type;
   char                            value_len[5];
   char                            value_buf[2000];
} lk_gfpogg08_io_def;
#define lk_gfpogg08_io_def_Size 8048
#pragma section lk_gfpogg10_io_1
/* Definition LK-GFPOGG10-IO-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg10_io_1
typedef struct __lk_gfpogg10_io_1
{
   char                            ascii_char[128];
   char                            ascii_char_len[3];
   char                            change_metod;
   char                            hex_data[16];
   char                            hex_data_len[2];
} lk_gfpogg10_io_1_def;
#define lk_gfpogg10_io_1_def_Size 150
#pragma section lk_gfpogg10_io_2
/* Definition LK-GFPOGG10-IO-2 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg10_io_2
typedef struct __lk_gfpogg10_io_2
{
   char                            return_code;
} lk_gfpogg10_io_2_def;
#define lk_gfpogg010_io_2_def_Size 1
#pragma section lk_gfpogg15_io_1
/* Definition LK-GFPOGG15-IO-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg15_io_1
typedef struct __lk_gfpogg15_io_1
{
   char                            tag_name[4];
   char                            contents_buf[12000];
   char                            contetns_buf_len[6];
   char                            if_buf_len[6];
   char                            if_buf[32767];
} lk_gfpogg15_io_1_def;
#define lk_gfpogg15_io_1_def_Size 44016
#pragma section lk_gfpogg15_io_2
/* Definition LK-GFPOGG15-IO-2 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg15_io_2
typedef struct __lk_gfpogg15_io_2
{
   char                            return_code;
} lk_gfpogg15_io_2_def;
#define lk_gfpogg15_io_2_def_Size 1
#pragma section exgfp1qput
/* Definition EXGFP1QPUT created on 10/15/2024 at 14:36 */
#pragma fieldalign shared2 __exqueinfo
/**/
typedef struct __exqueinfo
{
   short                           file_num;
   struct
   {
      char                            file_name[47];
   } file_info[5];
} exqueinfo_def;
#define exqueinfo_def_Size 238
#pragma section lk_gfpogg42_arg_1
/* Definition LK-GFPOGG42-ARG-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg42_arg_1
typedef struct __lk_gfpogg42_arg_1
{
    char                           de48_data[999];
} lk_gfpogg42_arg_1_def;
#define lk_gfpogg42_arg_1_def_Size 999

#pragma section lk_gfpogg42_arg_2
/* Definition LK-GFPOGG42-ARG-2 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg42_arg_2
typedef struct __lk_gfpogg42_arg_2
{
    short                          de48_len;
} lk_gfpogg42_arg_2_def;
#define lk_gfpogg42_arg_2_def_Size 2

#pragma section lk_gfpogg42_arg_3
/* Definition LK-GFPOGG42-ARG-3 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg42_arg_3
typedef struct __lk_gfpogg42_arg_3
{
    char                           se_id[2];
} lk_gfpogg42_arg_3_def;
#define lk_gfpogg42_arg_3_def_Size 2

#pragma section lk_gfpogg42_arg_4
/* Definition LK-GFPOGG42-ARG-4 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg42_arg_4
typedef struct __lk_gfpogg42_arg_4
{
    short                          se_len;
} lk_gfpogg42_arg_4_def;
#define lk_gfpogg42_arg_4_def_Size 2

#pragma section lk_gfpogg42_arg_5
/* Definition LK-GFPOGG42-ARG-5 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg42_arg_5
typedef struct __lk_gfpogg42_arg_5
{
    char                           se_data[99];
} lk_gfpogg42_arg_5_def;
#define lk_gfpogg42_arg_5_def_Size 99
#pragma section lk_gfpogg43_arg_1
/* Definition LK-GFPoGG43-ARG-1 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg43_arg_1
typedef struct __lk_gfpogg43_arg_1
{
   char                            func_flg;
   char                            io_timer[5];
   char                            sub_prog_sts[2];
   short                           guardian_errcode;
   char                            file_name[47];
   char                            add_info[30];
   char                            deid[3];
   char                            mti[4];
   struct
   {
      char                            i_data_lll[3];
      char                            i_data[999];
   } i_data_lv;
   struct
   {
      char                            o_data_lll[3];
      char                            o_data[999];
   } o_data_lv;
} lk_gfpogg43_arg_1_def;
#define lk_gfpogg43_arg_1_def_Size 2098
/* Definition LK-GFPoGG43 created on 05/16/2024 at 11:43 */
#pragma fieldalign shared2 __lk_gfpogg43
typedef struct __lk_gfpogg43
{
   struct
   {
      char                            cns_mjcnv_func_init;
      /*value is 0*/
      char                            cns_mjcnv_func_request;
      /*value is 1*/
      char                            cns_mjcnv_func_response;
      /*value is 2*/
   } cns_mjcnv_func_flg;
   struct
   {
      char                            cns_mjcnv_normal_end[2];
      /*value is "00"*/
      char                            cns_mjcnv_err_assign[2];
      /*value is "Z1"*/
      char                            cns_mjcnv_err_open[2];
      /*value is "Z2"*/
      char                            cns_mjcnv_err_read[2];
      /*value is "Z3"*/
      char                            cns_mjcnv_err_item[2];
      /*value is "Z4"*/
      char                            cns_mjcnv_err_param[2];
      /*value is "Z5"*/
      char                            cns_mjcnv_err_other[2];
      /*value is "Z9"*/
   } cns_mjcnv_sub_prog_sts;
} lk_gfpogg43_def;
#define lk_gfpogg43_def_Size 17
