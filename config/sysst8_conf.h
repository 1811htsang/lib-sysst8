#ifndef __SYSST8_CONFIG_H__
	#define __SYSST8_CONFIG_H__

  /** ANCHOR - Khai báo thư viện sử dụng
	 * @attention Người dùng bổ sung BSP ở đây để triển khai các hàm bên dưới
	 */
	#include <stdint.h>

  // ANCHOR - Số lượng trạng thái tối đa trong lịch sử của FSM
	#define SYSST8_FSM_HIS_MAX (4u)

  /** ANCHOR - Khai báo kiểu dữ liệu để quản lý tin nhắn trong hệ thống SYSST8
	 * @attention Người dùng tự định nghĩa các mã tin nhắn (message IDs) trong phạm vi 0x20 -> 0x2F,
   * 						để tránh xung đột với các mã tin nhắn nội bộ của SYSST8 (0x10 -> 0x1F).
   * @note SYSST8_FSM_SIG_INIT = 0x20u là mã tin nhắn đặc biệt được sử dụng để khởi tạo FSM,
   *       nên ưu tiên giữ đặt tên theo mẫu SYSST8_FSM_SIG_* và không cần khai báo thêm giá trị đi kèm.
	 */
  typedef enum sysst8_msg_t {
    SYSST8_FSM_SIG_INIT = 0x20u,
    SYSST8_FSM_SIG_NEXT = 0x21u,
    SYSST8_FSM_SIG_BACK = 0x22u,
    SYSST8_FSM_SIG_STAY = 0x23u,
    SYSST8_FSM_SIG_EXIT = 0x24u,
    SYSST8_FSM_SIG_NTRY = 0x25u,
    SYSST8_FSM_SIG_USER = 0x26u, // Mã tin nhắn do người dùng tự định nghĩa và sửa đổi
  } sysst8_msg_t;

  inline void sysst8_enter_critical(void) {
    // NOTE - Tự người dùng định nghĩa
  }

  inline void sysst8_exit_critical(void) {
    // NOTE - Tự người dùng định nghĩa
  }

  inline void sysst8_log(const char* msg) {
    // NOTE - Tự người dùng định nghĩa
  }

  inline void sysst8_reset(void) {
    // NOTE - Tự người dùng định nghĩa
  }

  inline void sysst8_fatal(const char* file, uint32_t line, const char* msg) {
    // NOTE - Tự người dùng định nghĩa
  }

#endif // __SYSST8_CONFIG_H__