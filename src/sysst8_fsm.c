/** ANCHOR - Triển khai các hàm và logic liên quan đến Finite State Machine (FSM) trong hệ thống UEDP
 * @file fsm.c
 * @author Shang Huang
 * @version 0.1
 * @date 2026-04-16
 * @copyright MIT License
 */
#include <stdint.h>
#include "sysst8_conf.h"
#include "sysst8_fcr.h"
#include "sysst8_fsm.h"

void sysst8_fsm_go_next(sysst8_fsm_t* me, state_handler target) {
  // Bảo vệ critical section để đảm bảo tính nhất quán khi thay đổi trạng thái của FSM
  sysst8_enter_critical();

  // Kiểm tra tính hợp lệ của con trỏ FSM và trạng thái mục tiêu
  if (!me || !target) {
    sysst8_exit_critical();
    SYSST8_FCR_RAISE_MSG(SYSST8_FCR_SM_NULL_HANDLER, "go_next: null fsm/target");
    return; 
  }

  // Gửi tín hiệu EXIT đến trạng thái hiện tại trước khi chuyển đổi
  if (me->state) {
    sysst8_msg_t exit_msg = SYSST8_FSM_SIG_EXIT; // Giả định có hàm tạo msg chuẩn
    me->state(&exit_msg);
  }

  // Lưu trạng thái hiện tại vào lịch sử trước khi chuyển đổi
  me->history[me->history_index] = me->state; // Lưu trạng thái hiện tại vào lịch sử
  me->history_index = (me->history_index + 1) % SYSST8_FSM_HIS_MAX; // Cập nhật chỉ số lịch sử, đảm bảo không vượt quá giới hạn
  if (me->history_count < SYSST8_FSM_HIS_MAX) {
    me->history_count++;
  }
  //NOTE - Sửa lỗi code gốc: Cần kiểm tra me->history_count trước khi tăng, tránh tràn số lượng lịch sử

  // Cập nhật trạng thái hiện tại của FSM thành trạng thái mục tiêu
  me->state = target; 

  // Gửi tín hiệu ENTRY đến trạng thái mục tiêu sau khi chuyển đổi
  sysst8_msg_t entry_msg = SYSST8_FSM_SIG_NTRY; // Giả định có hàm tạo msg chuẩn

  // Gửi tín hiệu ENTRY đến trạng thái mục tiêu
  target(&entry_msg);

  // Thoát critical section sau khi hoàn thành việc chuyển đổi trạng thái
  sysst8_exit_critical();
}


void sysst8_fsm_go_back(sysst8_fsm_t* me) {
  // Bảo vệ critical section để đảm bảo tính nhất quán khi thay đổi trạng thái của FSM
  sysst8_enter_critical();

  // Kiểm tra tính hợp lệ của con trỏ FSM
  if (!me || !me->state) {
    sysst8_exit_critical();
    SYSST8_FCR_RAISE_MSG(SYSST8_FCR_SM_NULL_HANDLER, "go_back: null fsm/state");
    return;
  }

  // Chưa có lịch sử để quay lại - đây là tình huống hợp lệ (không phải lỗi),
  // ví dụ FSM vừa init xong chưa từng go_next() lần nào - không raise FCR ở đây.
  if (me->history_count == 0) {
    sysst8_exit_critical();
    return;
  }

  // Cập nhật chỉ số lịch sử để trỏ đến trạng thái trước đó
  uint8_t prev_index = (me->history_index + SYSST8_FSM_HIS_MAX - 1) % SYSST8_FSM_HIS_MAX; // Tính chỉ số của trạng thái trước đó

  // Lấy trạng thái trước đó từ lịch sử
  state_handler previous_state = me->history[prev_index];
  //NOTE - Sửa lỗi code gốc: Cần kiểm tra previous_state trước khi sử dụng, tránh dereference NULL pointer

  // Đảm bảo rằng trạng thái trước đó không phải là NULL trước khi quay lại
  if (!previous_state) {
    // Sửa lỗi code gốc: Phải gọi sysst8_exit_critical() trước khi return để tránh treo hệ thống (Deadlock)
    sysst8_exit_critical();

    // Lịch sử có count > 0 nhưng slot bị NULL tức là hỏng bộ nhớ
    SYSST8_FCR_RAISE_MSG(SYSST8_FCR_SM_NULL_HANDLER, "go_back: corrupt history slot");
    return; 
  }

  // Gửi tín hiệu EXIT đến trạng thái hiện tại trước khi quay lại
  sysst8_msg_t exit_msg = SYSST8_FSM_SIG_EXIT; // Giả định có hàm tạo msg chuẩn
  me->state(&exit_msg);

  // Cập nhật trạng thái hiện tại của FSM thành trạng thái trước đó
  me->state = previous_state;
  me->history_index = prev_index; // Cập nhật chỉ số lịch sử sau khi quay lại
  me->history_count--; // Giảm số lượng trạng thái đã lưu trong lịch sử sau

  // Gửi tín hiệu ENTRY đến trạng thái trước đó sau khi quay lại
  sysst8_msg_t entry_msg = SYSST8_FSM_SIG_NTRY; // Giả định có hàm tạo msg chuẩn

  // Gửi tín hiệu ENTRY đến trạng thái trước đó
  previous_state(&entry_msg);

  // Thoát critical section sau khi hoàn thành việc quay lại trạng thái
  sysst8_exit_critical();
}

void sysst8_fsm_stay(sysst8_fsm_t* me) {
  // Bảo vệ critical section để đảm bảo tính nhất quán khi giữ nguyên trạng thái của FSM
  sysst8_enter_critical();

  // Kiểm tra tính hợp lệ của con trỏ FSM
  if (!me || !me->state) {
    sysst8_exit_critical();
    SYSST8_FCR_RAISE_MSG(SYSST8_FCR_SM_NULL_HANDLER, "stay: null fsm/state");
    return;
  }

  // Gửi tín hiệu STAY đến trạng thái hiện tại để thông báo rằng FSM sẽ giữ nguyên trạng thái
  sysst8_msg_t stay_msg = SYSST8_FSM_SIG_STAY; // Giả định có hàm tạo msg chuẩn
  me->state(&stay_msg);

  // Thoát critical section sau khi hoàn thành việc giữ nguyên trạng thái
  sysst8_exit_critical();
}
