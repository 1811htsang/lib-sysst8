/** ANCHOR - Implementation of Fatal Code Return (FCR) in UEDP
 * @file sysst8_fcr.c
 * @author Hai Minh
 * @version 0.1
 * @date 2026-08-01
 * @copyright MIT License
 */
#include <stddef.h>
#include "sysst8_conf.h"
#include "sysst8_fcr.h"

/** ANCHOR - Bảng mã lỗi nghiêm trọng của lõi UEDP
 * @attention Đây là bảng "tĩnh" (static const), không cần khởi tạo runtime.
 *            Mỗi module trong lõi UEDP chỉ nên có tối đa 256 mã lỗi con (0x00 -> 0xFF),
 *            xem SYSST8_FCR_MOD_* trong sysst8_fcr.h để biết dải mã module tương ứng.
 * @attention Entry cuối cùng (SYSST8_FCR_UNKNOWN) LUÔN phải tồn tại và được đặt cuối bảng,
 *            dùng làm fallback khi sysst8_fcr_lookup() không tìm thấy mã lỗi tương ứng.
 */
static const sysst8_fcr_entry_t g_fcr_table[] = {

  // [SM] (FSM/TSM)
  { SYSST8_FCR_SM_INVALID_TRANS, "SM invalid transition", SYSST8_FCR_SEV_ERROR, SYSST8_FCR_ACT_LOG_ONLY },
  { SYSST8_FCR_SM_NULL_HANDLER, "SM null state handler", SYSST8_FCR_SEV_FATAL, SYSST8_FCR_ACT_SYS_PANIC },

  // Fallback - LUÔN đặt cuối cùng
  { SYSST8_FCR_UNKNOWN, "Unknown FCR code", SYSST8_FCR_SEV_FATAL, SYSST8_FCR_ACT_SYS_PANIC }
};

#define SYSST8_FCR_TABLE_SIZE   (sizeof(g_fcr_table) / sizeof(g_fcr_table[0]))

const sysst8_fcr_entry_t* sysst8_fcr_lookup(sysst8_fcr_code_t code) {
  for (uint16_t i = 0; i < (uint16_t)SYSST8_FCR_TABLE_SIZE; i++) {
    if (g_fcr_table[i].code == code) {
      return &g_fcr_table[i];
    }
  }

  // Không tìm thấy -> trả về entry fallback (luôn là phần tử cuối bảng)
  return &g_fcr_table[SYSST8_FCR_TABLE_SIZE - 1];
}

void sysst8_fcr_raise(sysst8_fcr_code_t code, const char* file, uint32_t line, const char* extra_msg) {
  const sysst8_fcr_entry_t* entry = sysst8_fcr_lookup(code);

  // 1. Ghi log trước khi xử lý hành động, để đảm bảo dấu vết lỗi
  //    luôn được lưu lại kể cả khi hành động tiếp theo là SYS_PANIC/SYS_RESET
  sysst8_log((extra_msg != NULL) ? extra_msg : entry->desc);

  // 2. Thực hiện hành động xử lý tương ứng với entry tra được trong bảng
  switch (entry->action) {
    case SYSST8_FCR_ACT_LOG_ONLY:
      // NOTE - Không can thiệp thêm, chỉ ghi log ở bước 1
      // REVIEW - Bổ sung itnlog ở đây
      break;

    case SYSST8_FCR_ACT_SYS_RESET:
      sysst8_reset();
      break;

    case SYSST8_FCR_ACT_SYS_PANIC:
    default:
      sysst8_fatal(file, line, entry->desc);
      break;
  }
}
