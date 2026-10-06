/**
 * @file sysst8_fcr.h
 * @author Hai Minh
 * @brief Fatal Code Return (FCR) - Declaration for error codes and handling in SYSST8 core
 * @version 0.1
 * @date 2026-08-01
 * @copyright MIT License
 */
#ifndef __SYSST8_FCR_H__
  #define __SYSST8_FCR_H__

  // ANCHOR - Khai báo thư viện sử dụng
  #include <stdint.h>

  /** ANCHOR - Định nghĩa kiểu dữ liệu cho mã lỗi FCR
   * @attention Mã lỗi FCR được thiết kế theo encoding tương tự các dải tín hiệu khác
   *            trong SYSST8 (xem [HES] Heximal Encoding Signals ở arch-design.md):
   *            byte cao (bit 15-8) là mã MODULE (module nào phát sinh lỗi),
   *            byte thấp (bit 7-0) là mã SUB-CODE (lỗi cụ thể trong module đó).
   *            Nhờ vậy 1 module có thể có tối đa 256 mã lỗi con khác nhau.
   */
  typedef uint16_t sysst8_fcr_code_t;

  /** ANCHOR - Khai báo dải mã MODULE cho FCR
   * @note MOD = ITC (Internal Trouble Code)
   */
  #define SYSST8_FCR_MOD_SM       (0x30u) // Module máy trạng thái (sysst8_fsm / sysst8_tsm)
  #define SYSST8_FCR_MOD_APP      (0x31u) // Dành riêng cho tầng ứng dụng tự khai báo mã lỗi
  #define SYSST8_FCR_MOD_UNK      (0x32u) // Module không xác định / fallback

  /** ANCHOR - Macro tiện lợi để ghép mã MODULE và SUB-CODE thành 1 sysst8_fcr_code_t
   * @param mod Mã module, xem các hằng số SYSST8_FCR_MOD_*
   * @param sub Mã lỗi con trong module đó (0x00 -> 0xFF)
   */
  #define SYSST8_FCR_CODE(mod, sub)   ((sysst8_fcr_code_t)(((uint16_t)(mod) << 8) | (uint16_t)(sub)))

  /** ANCHOR - Bảng mã lỗi nghiêm trọng của lõi SYSST8
   * @attention Đây KHÔNG phải danh sách đầy đủ - người dùng có thể bổ sung thêm
   *            mã lỗi riêng cho tầng ứng dụng bằng cách dùng SYSST8_FCR_CODE(SYSST8_FCR_MOD_APP, x)
   *            và tự đăng ký entry tương ứng nếu cần (xem sysst8_fcr_raise()).
   * @note FCR = DTC (Diagnostic Trouble Code) - Mã lỗi chẩn đoán nghiêm trọng, dùng để báo cáo lỗi
   */

  // State Machine (SM) - Mã lỗi liên quan đến FSM/TSM
  #define SYSST8_FCR_SM_INVALID_TRANS       SYSST8_FCR_CODE(SYSST8_FCR_MOD_SM, 0x00) // Không tìm thấy transition hợp lệ cho tín hiệu hiện tại trong TSM
  #define SYSST8_FCR_SM_NULL_HANDLER        SYSST8_FCR_CODE(SYSST8_FCR_MOD_SM, 0x01) // Con trỏ hàm state hiện tại của FSM là NULL

  // Fallback
  #define SYSST8_FCR_UNKNOWN                SYSST8_FCR_CODE(SYSST8_FCR_MOD_UNK, 0xFF) // Mã lỗi không tra được trong bảng (không có entry tương ứng)

  // APP - Mã lỗi do tầng ứng dụng tự định nghĩa, người dùng có thể khai báo thêm

  /** ANCHOR - Định nghĩa mức độ nghiêm trọng của một mã lỗi FCR
   * @param SYSST8_FCR_SEV_WARN Chỉ cảnh báo, hệ thống vẫn tiếp tục hoạt động bình thường
   * @param SYSST8_FCR_SEV_ERROR Lỗi có ảnh hưởng cục bộ, cần can thiệp ở mức tác vụ/module
   * @param SYSST8_FCR_SEV_FATAL Lỗi nghiêm trọng, có thể ảnh hưởng tới toàn bộ hệ thống
   */
  typedef enum sysst8_fcr_severity_t {
    SYSST8_FCR_SEV_WARN = 0,
    SYSST8_FCR_SEV_ERROR,
    SYSST8_FCR_SEV_FATAL
  } sysst8_fcr_severity_t;

  /** ANCHOR - Định nghĩa hành động xử lý tương ứng khi một mã lỗi FCR được raise
   * @param SYSST8_FCR_ACT_LOG_ONLY Chỉ ghi log, không can thiệp vào luồng chạy
   * @param SYSST8_FCR_ACT_SYS_RESET Gọi pal_sys_reset() để khởi động lại toàn bộ hệ thống
   * @param SYSST8_FCR_ACT_SYS_PANIC Gọi pal_sys_fatal() (SYSST8_PANIC) để dừng hệ thống ngay lập tức
   */
  typedef enum sysst8_fcr_action_t {
    SYSST8_FCR_ACT_LOG_ONLY = 0,
    SYSST8_FCR_ACT_SYS_RESET,
    SYSST8_FCR_ACT_SYS_PANIC
  } sysst8_fcr_action_t;

  /** ANCHOR - Khai báo 1 dòng trong bảng mã lỗi FCR
   * @param code Mã lỗi FCR (xem SYSST8_FCR_CODE)
   * @param desc Mô tả ngắn gọn về lỗi, dùng khi ghi log hoặc panic
   * @param severity Mức độ nghiêm trọng của lỗi
   * @param action Hành động xử lý tương ứng khi lỗi này được raise
   */
  typedef struct sysst8_fcr_entry_t {
    sysst8_fcr_code_t code;
    const char* desc;
    sysst8_fcr_severity_t severity;
    sysst8_fcr_action_t action;
  } sysst8_fcr_entry_t;

  /** ANCHOR - Tra bảng mã lỗi FCR để lấy thông tin (mô tả, mức độ, hành động) tương ứng
   * @param code Mã lỗi FCR cần tra
   * @return const sysst8_fcr_entry_t* Con trỏ đến entry tương ứng trong bảng,
   *         hoặc entry SYSST8_FCR_UNKNOWN nếu không tìm thấy code trong bảng
   */
  const sysst8_fcr_entry_t* sysst8_fcr_lookup(sysst8_fcr_code_t code);

  /** ANCHOR - Raise (báo cáo) một lỗi nghiêm trọng FCR
   * @param code Mã lỗi FCR cần raise (xem các hằng số SYSST8_FCR_*)
   * @param file Tên tệp phát sinh lỗi (thường truyền __FILE__)
   * @param line Số dòng phát sinh lỗi (thường truyền __LINE__)
   * @param extra_msg Thông tin bổ sung (có thể NULL), sẽ được nối thêm vào log
   * @note Hàm này sẽ:
   *       1. Tra bảng qua sysst8_fcr_lookup() để lấy mô tả, mức độ và hành động.
   *       2. Ghi log qua SYSST8_itnlog_log() với tag ITNLOG_TAG_FCR, mức độ log
   *          tương ứng với severity (WARN/ERROR -> ITNLOG_LEVEL_WARN/ERROR,
   *          FATAL -> ITNLOG_LEVEL_FATAL).
   *       3. Thực hiện hành động xử lý tương ứng (action) - xem SYSST8_fcr_action_t.
   * @attention Với action = SYSST8_FCR_ACT_SYS_PANIC, hàm này KHÔNG return
   *            (pal_sys_fatal thường sẽ abort()/reset hệ thống).
   */
  void sysst8_fcr_raise(sysst8_fcr_code_t code, const char* file, uint32_t line, const char* extra_msg);

  // ANCHOR - Macro tiện lợi để gọi sysst8_fcr_raise() với thông tin file/line tự động điền
  #define SYSST8_FCR_RAISE(code) sysst8_fcr_raise((code), __FILE__, __LINE__, '\0')
  #define SYSST8_FCR_RAISE_MSG(code, extra) sysst8_fcr_raise((code), __FILE__, __LINE__, (extra))

#endif // __SYSST8_FCR_H__
