#include "sysst8_conf.h"

void sysst8_enter_critical(void) {
  // NOTE - Tự người dùng định nghĩa
}

void sysst8_exit_critical(void) {
  // NOTE - Tự người dùng định nghĩa
}

void sysst8_log(const char* msg) {
  // NOTE - Tự người dùng định nghĩa
}

void sysst8_reset(void) {
  // NOTE - Tự người dùng định nghĩa
}

void sysst8_fatal(const char* file, uint32_t line, const char* msg) {
  // NOTE - Tự người dùng định nghĩa
}

void sysst8_tim_hw_start(uint8_t hw_ch, uint32_t period_us, bool periodic) {
  // NOTE - Tự người dùng định nghĩa
}

void sysst8_tim_hw_stop(uint8_t hw_ch) {
  // NOTE - Tự người dùng định nghĩa
}