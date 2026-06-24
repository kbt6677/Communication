/* SCHEMA PRODUCED DATE - TIME :11/12/2024 - 18:59:05 */
#pragma section timer_issue_rq
/* Definition TIMER-ISSUE-RQ created on 11/12/2024 at 18:59 */
#pragma fieldalign shared2 __timer_issue_rq
/**/
typedef struct __timer_issue_rq
{
   char                            unique_msg_id[4];
   char                            process_result_code[4];
   struct
   {
      struct
      {
         char                            numbering_system_kbn;
         char                            location;
         char                            filler_1;
         char                            year_y;
         char                            mdh[3];
         char                            time_mmss[4];
         char                            gfp_lcn_serial_num[4];
      } gfp_lcn;
      char                            msg_form;
   } transaction_id;
   char                            timer_exp_yymmddhhmmsscc[16];
   char                            queue_pathmon_primary[16];
   char                            queue_server_primary[16];
   char                            queue_pathmon_backup[16];
   char                            queue_server_backup[16];
} timer_issue_rq_def;
#define timer_issue_rq_def_Size 104
#pragma section timer_issue_resp
/* Definition TIMER-ISSUE-RESP created on 11/12/2024 at 18:59 */
#pragma fieldalign shared2 __timer_issue_resp
typedef struct __timer_issue_resp
{
   char                            unique_msg_id[4];
   char                            process_result_code[4];
   char                            timer_ctrl_addr1[4];
   char                            timer_ctrl_cpu1[2];
   char                            timer_ctrl_pin1[2];
   char                            timer_ctrl_addr2[4];
   char                            timer_ctrl_cpu2[2];
   char                            timer_ctrl_pin2[2];
} timer_issue_resp_def;
#define timer_issue_resp_def_Size 24
#pragma section timer_cancel_rq
/* Definition TIMER-CANCEL-RQ created on 11/12/2024 at 18:59 */
#pragma fieldalign shared2 __timer_cancel_rq
typedef struct __timer_cancel_rq
{
   char                            unique_msg_id[4];
   char                            process_result_code[4];
   struct
   {
      struct
      {
         char                            numbering_system_kbn;
         char                            location;
         char                            filler_1;
         char                            year_y;
         char                            mdh[3];
         char                            time_mmss[4];
         char                            gfp_lcn_serial_num[4];
      } gfp_lcn;
      char                            msg_form;
   } transaction_id;
   char                            timer_ctrl_addr1[4];
   char                            timer_ctrl_cpu1[2];
   char                            timer_ctrl_pin1[2];
   char                            timer_ctrl_addr2[4];
   char                            timer_ctrl_cpu2[2];
   char                            timer_ctrl_pin2[2];
} timer_cancel_rq_def;
#define timer_cancel_rq_def_Size 40
#pragma section timer_cancel_resp
/* Definition TIMER-CANCEL-RESP created on 11/12/2024 at 18:59 */
#pragma fieldalign shared2 __timer_cancel_resp
typedef struct __timer_cancel_resp
{
   char                            unique_msg_id[4];
   char                            process_result_code[4];
} timer_cancel_resp_def;
#define timer_cancel_resp_def_Size 8
#pragma section timer_addition_rq
/* Definition TIMER-ADDITION-RQ created on 11/12/2024 at 18:59 */
#include <tnsint.h>
#pragma fieldalign shared2 __timer_addition_rq
typedef struct __timer_addition_rq
{
   char                            unique_msg_id[4];
   char                            process_result_code[4];
   short                           setting_count;
   struct
   {
      struct
      {
         struct
         {
            char                            numbering_system_kbn;
            char                            location;
            char                            filler_1;
            char                            year_y;
            char                            mdh[3];
            char                            time_mmss[4];
            char                            gfp_lcn_serial_num[4];
         } gfp_lcn;
         char                            msg_form;
      } transaction_id;
      char                            timer_exp_yymmddhhmmsscc[16];
      char                            queue_pathmon_primary[16];
      char                            queue_server_primary[16];
      char                            queue_pathmon_backup[16];
      char                            queue_server_backup[16];
      __int32_t                       primary_memory_address;
   } timer_addition_info[100];
} timer_addition_rq_def;
#define timer_addition_rq_def_Size 10010
#pragma section timer_addition_resp
/* Definition TIMER-ADDITION-RESP created on 11/12/2024 at 18:59 */
#include <tnsint.h>
#pragma fieldalign shared2 __timer_addition_resp
typedef struct __timer_addition_resp
{
   char                            unique_msg_id[4];
   char                            process_result_code[4];
   short                           setting_count;
   struct
   {
      struct
      {
         struct
         {
            char                            numbering_system_kbn;
            char                            location;
            char                            filler_1;
            char                            year_y;
            char                            mdh[3];
            char                            time_mmss[4];
            char                            gfp_lcn_serial_num[4];
         } gfp_lcn;
         char                            msg_form;
      } transaction_id;
      __int32_t                       backup_memory_address;
      __int32_t                       primary_memory_address;
   } timer_addition_result[100];
} timer_addition_resp_def;
#define timer_addition_resp_def_Size 2410
#pragma section timer_delete_rq
/* Definition TIMER-DELETE-RQ created on 11/12/2024 at 18:59 */
#include <tnsint.h>
#pragma fieldalign shared2 __timer_delete_rq
typedef struct __timer_delete_rq
{
   char                            unique_msg_id[4];
   char                            process_result_code[4];
   short                           setting_count;
   struct
   {
      struct
      {
         struct
         {
            char                            numbering_system_kbn;
            char                            location;
            char                            filler_1;
            char                            year_y;
            char                            mdh[3];
            char                            time_mmss[4];
            char                            gfp_lcn_serial_num[4];
         } gfp_lcn;
         char                            msg_form;
      } transaction_id;
      __int32_t                       primary_memory_address;
      __int32_t                       backup_memory_address;
   } timer_delete_info[100];
} timer_delete_rq_def;
#define timer_delete_rq_def_Size 2410
#pragma section timer_delete_resp
/* Definition TIMER-DELETE-RESP created on 11/12/2024 at 18:59 */
#include <tnsint.h>
#pragma fieldalign shared2 __timer_delete_resp
typedef struct __timer_delete_resp
{
   char                            unique_msg_id[4];
   char                            process_result_code[4];
   short                           setting_count;
   struct
   {
      __int32_t                       primary_memory_address;
   } timer_delete_result[100];
} timer_delete_resp_def;
#define timer_delete_resp_def_Size 410
