#ifndef __SYSST8_CONFIG_H__
	#define __SYSST8_CONFIG_H__

  /** ANCHOR - Khai báo thư viện sử dụng
   * @attention Người dùng bổ sung BSP ở đây để triển khai các hàm bên dưới
   */
	#include <stdint.h>
  #include <stdbool.h>

  // ANCHOR - Số lượng trạng thái tối đa trong lịch sử của FSM
	#define SYSST8_FSM_HIS_MAX (4u)

  // ANCHOR - Khai báo số lượng tín hiệu tối đa
  #define SYSST8_FSM_SIG_MAX (0x2Fu)

  // ANCHOR - Khai báo số lượng tin nhắn tối đa trong hàng đợi (queue) của FSM
  #define SYSST8_FSM_MSG_Q_MAX (8u)

  /** ANCHOR - Khai báo kiểu dữ liệu để quản lý tin nhắn trong hệ thống SYSST8
   * @attention Người dùng tự định nghĩa các mã tin nhắn (message IDs) trong phạm vi 0x20 -> 0x2F,
   * 						để tránh xung đột với các mã tin nhắn nội bộ của SYSST8 (0x10 -> 0x1F).
   * @note SYSST8_FSM_SIG_INIT = 0x20u là mã tin nhắn đặc biệt được sử dụng để khởi tạo FSM,
   *       nên ưu tiên giữ đặt tên theo mẫu SYSST8_FSM_SIG_* và không cần khai báo thêm giá trị đi kèm.
   */
  typedef enum sysst8_msg_t {
    SYSST8_FSM_SIG_INIT = 0x20u,
    SYSST8_FSM_SIG_STAY = 0x21u,
    SYSST8_FSM_SIG_EXIT = 0x22u,
    SYSST8_FSM_SIG_NTRY = 0x23u
  } sysst8_msg_t;

  /** ANCHOR - Khai báo prototype các hàm cần triển khai để SYSST8 hoạt động
   * @attention Người dùng tự định nghĩa các hàm này trong file sysst8_conf.c để triển khai các chức năng liên quan đến hệ thống.
   */
  void sysst8_enter_critical(void);
  void sysst8_exit_critical(void);
  void sysst8_log(const char* msg);
  void sysst8_reset(void);
  void sysst8_fatal(const char* file, uint32_t line, const char* msg);

  /** ANCHOR - Prototype BSP cho timer phần cứng, dùng bởi sysst8_tim
   * @attention Người dùng ánh xạ `hw_ch` sang ngoại vi timer thực tế và cấu hình ngắt,
   *            khi hết hạn cần gọi sysst8_tim_irq() tương ứng với kênh đó.
   * @param hw_ch chỉ số kênh timer phần cứng
   * @param period_us chu kỳ tính theo micro giây
   * @param periodic true nếu lặp lại liên tục, false nếu chỉ hết hạn một lần
   */
  void sysst8_tim_hw_start(uint8_t hw_ch, uint32_t period_us, bool periodic);
  void sysst8_tim_hw_stop(uint8_t hw_ch);

#endif // __SYSST8_CONFIG_H__
