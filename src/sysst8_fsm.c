/** ANCHOR - Triển khai các hàm và logic liên quan đến Finite State Machine (FSM) trong hệ thống UEDP
 * @file fsm.c
 * @author Shang Huang
 * @version 0.1
 * @date 2026-04-16
 * @copyright MIT License
 */
#include <stdint.h>
#include <stdbool.h>
#include "sysst8_conf.h"
#include "sysst8_fcr.h"
#include "sysst8_fsm.h"

static void internal_fsm_enqueue(sysst8_fsm_t* me, sysst8_msg_t msg);
static bool internal_fsm_dequeue(sysst8_fsm_t* me, sysst8_msg_t* msg);

void sysst8_fsm_go_next(sysst8_fsm_t* me, state_handler target) {
  // NOTE - Bảo vệ critical section để đảm bảo tính nhất quán khi thay đổi trạng thái của FSM
  sysst8_enter_critical();

  // NOTE - Kiểm tra tính hợp lệ của con trỏ FSM và trạng thái mục tiêu
  bool is_valid_param = (me != NULL) && (target != NULL);
  if (!is_valid_param) {
    sysst8_exit_critical();
    SYSST8_FCR_RAISE_MSG(SYSST8_FCR_SM_NULL_HANDLER, "go_next: null fsm/target");
    return;
  }

  // NOTE - Chỉ gửi EXIT và lưu lịch sử khi đã có trạng thái hiện tại, tránh đưa con trỏ NULL vào history
  bool has_current_state = (me->state != NULL);
  if (has_current_state) {
    sysst8_msg_t exit_msg = SYSST8_FSM_SIG_EXIT;
    me->state(exit_msg);

    me->history[me->history_index] = me->state;
    me->history_index = (me->history_index + 1) % SYSST8_FSM_HIS_MAX;
    if (me->history_count < SYSST8_FSM_HIS_MAX) me->history_count++;
  }

  me->state = target;

  // NOTE - ENTRY được đưa vào hàng đợi để fsm_dispatch() xử lý, không gọi trực tiếp trong critical section
  internal_fsm_enqueue(me, SYSST8_FSM_SIG_NTRY);

  sysst8_exit_critical();
}

void sysst8_fsm_go_back(sysst8_fsm_t* me) {
  sysst8_enter_critical();

  bool is_valid_param = (me != NULL) && (me->state != NULL);
  if (!is_valid_param) {
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

  uint8_t prev_index = (me->history_index + SYSST8_FSM_HIS_MAX - 1) % SYSST8_FSM_HIS_MAX;
  state_handler previous_state = me->history[prev_index];

  if (!previous_state) {
    sysst8_exit_critical();
    SYSST8_FCR_RAISE_MSG(SYSST8_FCR_SM_NULL_HANDLER, "go_back: corrupt history slot");
    return;
  }

  sysst8_msg_t exit_msg = SYSST8_FSM_SIG_EXIT;
  me->state(exit_msg);

  me->state = previous_state;
  me->history_index = prev_index;
  me->history_count--;

  internal_fsm_enqueue(me, SYSST8_FSM_SIG_NTRY);

  sysst8_exit_critical();
}

void sysst8_fsm_stay(sysst8_fsm_t* me) {
  sysst8_enter_critical();

  if (!me || !me->state) {
    sysst8_exit_critical();
    SYSST8_FCR_RAISE_MSG(SYSST8_FCR_SM_NULL_HANDLER, "stay: null fsm/state");
    return;
  }

  internal_fsm_enqueue(me, SYSST8_FSM_SIG_STAY);

  sysst8_exit_critical();
}

// NOTE - Hàm nội bộ không tự bảo vệ critical section, được gọi bên trong critical section của hàm public
static void internal_fsm_enqueue(sysst8_fsm_t* me, sysst8_msg_t msg) {
  if (me->msg_q_count >= SYSST8_FSM_MSG_Q_MAX) {
    SYSST8_FCR_RAISE_MSG(SYSST8_FCR_SM_MSGQ_TRY_FULL, "enqueue: message queue full");
    return;
  }

  me->msg_q[me->msg_q_tail] = msg;
  me->msg_q_tail = (me->msg_q_tail + 1) % SYSST8_FSM_MSG_Q_MAX;
  me->msg_q_count++;
}

static bool internal_fsm_dequeue(sysst8_fsm_t* me, sysst8_msg_t* msg) {
  sysst8_enter_critical();

  if (me->msg_q_count == 0) {
    sysst8_exit_critical();
    return false;
  }

  *msg = me->msg_q[me->msg_q_head];
  me->msg_q_head = (me->msg_q_head + 1) % SYSST8_FSM_MSG_Q_MAX;
  me->msg_q_count--;

  sysst8_exit_critical();
  return true;
}

void sysst8_fsm_dispatch(sysst8_fsm_t* me) {
  if (me == NULL || me->state == NULL) {
    SYSST8_FCR_RAISE(SYSST8_FCR_SM_NULL_HANDLER); // FSM chưa được sysst8_fsm_init() hoặc chưa có go_next()
    return;
  }

  // Hàng đợi rỗng là trạng thái bình thường khi main loop poll, không phải lỗi
  sysst8_msg_t msg;
  if (internal_fsm_dequeue(me, &msg)) {
    me->state(msg);
  }
}
