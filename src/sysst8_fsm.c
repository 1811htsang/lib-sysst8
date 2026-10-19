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

static void internal_fsm_enqueue(sysst8_msg_t msg);
static sysst8_msg_t internal_fsm_dequeue(void);

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

  // NOTE - Gửi tín hiệu EXIT đến trạng thái hiện tại trước khi chuyển đổi
  bool has_current_state = (me->state != NULL);
  if (has_current_state) {
    sysst8_msg_t exit_msg = SYSST8_FSM_SIG_EXIT;
    me->state(exit_msg);
  }

  // NOTE - Lưu trạng thái hiện tại vào lịch sử trước khi chuyển đổi
  me->history[me->history_index] = me->state; // Lưu trạng thái hiện tại vào lịch sử
  me->history_index = (me->history_index + 1) % SYSST8_FSM_HIS_MAX; // Cập nhật chỉ số lịch sử, đảm bảo không vượt quá giới hạn
  if (me->history_count < SYSST8_FSM_HIS_MAX) me->history_count++;
  //NOTE - Sửa lỗi code gốc: Cần kiểm tra me->history_count trước khi tăng, tránh tràn số lượng lịch sử

  // NOTE - Cập nhật trạng thái hiện tại của FSM thành trạng thái mục tiêu
  me->state = target; 

  internal_fsm_enqueue(SYSST8_FSM_SIG_NTRY); // Gửi tín hiệu ENTRY đến trạng thái mục tiêu thông qua hàng đợi

  // NOTE - Không gọi trực tiếp target(entry_msg) ở đây mà đưa việc thực thi vào hàng đợi để fsm_dispatch() xử lý, 
  // đảm bảo tính tuần tự và tránh deadlock trong critical section

  // Thoát critical section sau khi hoàn thành việc chuyển đổi trạng thái
  sysst8_exit_critical();
}

void sysst8_fsm_go_back(sysst8_fsm_t* me) {
  // Bảo vệ critical section để đảm bảo tính nhất quán khi thay đổi trạng thái của FSM
  sysst8_enter_critical();

  // Kiểm tra tính hợp lệ của con trỏ FSM
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
  me->state(exit_msg);

  // Cập nhật trạng thái hiện tại của FSM thành trạng thái trước đó
  me->state = previous_state;
  me->history_index = prev_index; // Cập nhật chỉ số lịch sử sau khi quay lại
  me->history_count--; // Giảm số lượng trạng thái đã lưu trong lịch sử sau

  internal_fsm_enqueue(SYSST8_FSM_SIG_NTRY); // Gửi tín hiệu ENTRY đến trạng thái trước đó thông qua hàng đợi

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

  // Gửi tín hiệu STAY đến trạng thái hiện tại thông qua hàng đợi
  internal_fsm_enqueue(SYSST8_FSM_SIG_STAY);

  // Thoát critical section sau khi hoàn thành việc giữ nguyên trạng thái
  sysst8_exit_critical();
}

// Mảng lưu trữ các tin nhắn được gửi đến FSM để thực hiện dispatch
static sysst8_msg_t g_msg_q[SYSST8_FSM_MSG_Q_MAX] = {0}; // Khởi tạo mảng tin nhắn với giá trị mặc định 0

static uint8_t g_msg_q_head = 0; // Chỉ số đầu của hàng đợi tin nhắn
static uint8_t g_msg_q_tail = 0; // Chỉ số cuối của hàng đợi tin nhắn
static uint8_t g_msg_q_count = 0; // Số lượng tin nhắn hiện có trong hàng đợi

static void internal_fsm_enqueue(sysst8_msg_t msg) {
  // Bảo vệ critical section để đảm bảo tính nhất quán khi thêm tin nhắn vào hàng đợi
  sysst8_enter_critical();

  // Kiểm tra xem hàng đợi có đầy hay không
  if (g_msg_q_count >= SYSST8_FSM_MSG_Q_MAX) {
    sysst8_exit_critical();
    SYSST8_FCR_RAISE_MSG(SYSST8_FCR_SM_MSGQ_TRY_FULL, "enqueue: message queue full");
    return;
  }

  // Thêm tin nhắn vào hàng đợi tại vị trí tail
  g_msg_q[g_msg_q_tail] = msg;
  g_msg_q_tail = (g_msg_q_tail + 1) % SYSST8_FSM_MSG_Q_MAX; // Cập nhật chỉ số tail, đảm bảo vòng tròn
  g_msg_q_count++; // Tăng số lượng tin nhắn hiện có trong hàng đợi

  // Thoát critical section sau khi hoàn thành việc thêm tin nhắn
  sysst8_exit_critical();
}

static sysst8_msg_t internal_fsm_dequeue(void) {
  // Bảo vệ critical section để đảm bảo tính nhất quán khi lấy tin nhắn từ hàng đợi
  sysst8_enter_critical();

  // Kiểm tra xem hàng đợi có rỗng hay không
  if (g_msg_q_count == 0) {
    sysst8_exit_critical();
    SYSST8_FCR_RAISE_MSG(SYSST8_FCR_SM_MSGQ_TRY_EMPTY, "dequeue: message queue empty");
    return SYSST8_FSM_SIG_INIT; // Trả về giá trị mặc định khi hàng đợi rỗng
  }

  // Lấy tin nhắn từ hàng đợi tại vị trí head
  sysst8_msg_t msg = g_msg_q[g_msg_q_head];
  g_msg_q_head = (g_msg_q_head + 1) % SYSST8_FSM_MSG_Q_MAX; // Cập nhật chỉ số head, đảm bảo vòng tròn
  g_msg_q_count--; // Giảm số lượng tin nhắn hiện có trong hàng đợi

  // Thoát critical section sau khi hoàn thành việc lấy tin nhắn
  sysst8_exit_critical();

  return msg; // Trả về tin nhắn đã lấy từ hàng đợi
}

void sysst8_fsm_dispatch(sysst8_fsm_t* me) {
  // Lấy tin nhắn từ hàng đợi để xử lý
  sysst8_msg_t msg = internal_fsm_dequeue();
  bool is_valid_msg = (msg >= SYSST8_FSM_SIG_INIT && msg <= SYSST8_FSM_SIG_MAX);
  bool is_valid_fsm = (me != NULL && me->state != NULL);
  if (is_valid_fsm && is_valid_msg) {
    me->state(msg); 
    // NOTE - Do chuyển state đã được thực hiện trước khi gửi tin nhắn vào hàng đợi, 
    // nên không cần gọi go_next() ở đây nữa, chỉ cần dispatch tin nhắn đến state hiện tại
  } else if (!is_valid_fsm || !is_valid_msg) {
    SYSST8_FCR_RAISE(SYSST8_FCR_SM_NULL_HANDLER); // FSM chưa được sysst8_fsm_init() hoặc bị go_next(NULL)
  }
}