/**
 * @file sysst8_tim.h
 * @brief Hardware timer wrapper for SYSST8 - one instance per hardware timer channel
 * @version 0.1
 * @copyright MIT License
 */
#ifndef __SYSST8_TIM_H__
  #define __SYSST8_TIM_H__

  // ANCHOR - Khai báo thư viện sử dụng
  #include <stdint.h>
  #include <stdbool.h>

  /** ANCHOR - Định nghĩa kiểu hàm callback được gọi khi timer hết hạn
   * @attention Hàm này được gọi trong ngữ cảnh ngắt (ISR), cần ngắn gọn và không block.
   */
  typedef void (*sysst8_tim_handler)(void);

  /** ANCHOR - Chế độ hoạt động của timer
   * @param SYSST8_TIM_MODE_ONESHOT Hết hạn một lần rồi dừng
   * @param SYSST8_TIM_MODE_PERIODIC Lặp lại liên tục theo chu kỳ period_us
   */
  typedef enum sysst8_tim_mode_t {
    SYSST8_TIM_MODE_ONESHOT = 0,
    SYSST8_TIM_MODE_PERIODIC
  } sysst8_tim_mode_t;

  /** ANCHOR - Cấu trúc quản lý một kênh timer phần cứng
   * @param hw_ch Chỉ số kênh timer phần cứng, được BSP ánh xạ sang ngoại vi thực tế
   * @param mode Chế độ hoạt động hiện tại
   * @param running Cờ cho biết timer đang chạy hay không
   * @param on_expire Hàm callback được gọi khi timer hết hạn
   */
  typedef struct sysst8_tim_t {
    uint8_t hw_ch;
    sysst8_tim_mode_t mode;
    bool running;
    sysst8_tim_handler on_expire;
  } sysst8_tim_t;

  /** ANCHOR - Khởi tạo một instance timer gắn với kênh phần cứng
   * @param me chỉ instance timer cần khởi tạo
   * @param hw_ch chỉ số kênh timer phần cứng
   * @param on_expire chỉ hàm callback khi timer hết hạn
   */
  void sysst8_tim_init(sysst8_tim_t* me, uint8_t hw_ch, sysst8_tim_handler on_expire);

  /** ANCHOR - Bắt đầu timer với chu kỳ và chế độ cho trước, nếu đang chạy thì nạp lại
   * @param me chỉ instance timer
   * @param period_us chu kỳ tính theo micro giây, phải > 0
   * @param mode chế độ ONESHOT hoặc PERIODIC
   */
  void sysst8_tim_start(sysst8_tim_t* me, uint32_t period_us, sysst8_tim_mode_t mode);

  /** ANCHOR - Dừng timer phần cứng
   * @param me chỉ instance timer
   * @attention Không đảm bảo callback đang chạy trong ISR đã kết thúc khi hàm này return.
   */
  void sysst8_tim_stop(sysst8_tim_t* me);

  /** ANCHOR - Hàm xử lý ngắt timer, cần được gọi từ ISR của kênh phần cứng tương ứng
   * @param me chỉ instance timer
   */
  void sysst8_tim_irq(sysst8_tim_t* me);

  /** ANCHOR - Kiểm tra timer có đang chạy hay không
   * @param me chỉ instance timer
   * @return true nếu timer đang chạy
   */
  bool sysst8_tim_is_running(const sysst8_tim_t* me);

#endif // __SYSST8_TIM_H__
