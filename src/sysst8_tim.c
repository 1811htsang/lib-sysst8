/** ANCHOR - Triển khai wrapper timer phần cứng cho SYSST8
 * @file sysst8_tim.c
 * @version 0.1
 * @copyright MIT License
 */
#include <stddef.h>
#include "sysst8_conf.h"
#include "sysst8_fcr.h"
#include "sysst8_tim.h"

void sysst8_tim_init(sysst8_tim_t* me, uint8_t hw_ch, sysst8_tim_handler on_expire) {
  if (me == NULL || on_expire == NULL) {
    SYSST8_FCR_RAISE_MSG(SYSST8_FCR_TIM_NULL_PARAM, "tim_init: null tim/handler");
    return;
  }

  me->hw_ch = hw_ch;
  me->mode = SYSST8_TIM_MODE_ONESHOT;
  me->running = false;
  me->on_expire = on_expire;
}

void sysst8_tim_start(sysst8_tim_t* me, uint32_t period_us, sysst8_tim_mode_t mode) {
  if (me == NULL || me->on_expire == NULL) {
    SYSST8_FCR_RAISE_MSG(SYSST8_FCR_TIM_NULL_PARAM, "tim_start: null tim/handler");
    return;
  }

  if (period_us == 0u) {
    SYSST8_FCR_RAISE_MSG(SYSST8_FCR_TIM_INVALID_PERIOD, "tim_start: period is zero");
    return;
  }

  sysst8_enter_critical();
  me->mode = mode;
  me->running = true;
  sysst8_tim_hw_start(me->hw_ch, period_us, mode == SYSST8_TIM_MODE_PERIODIC);
  sysst8_exit_critical();
}

void sysst8_tim_stop(sysst8_tim_t* me) {
  if (me == NULL) {
    SYSST8_FCR_RAISE_MSG(SYSST8_FCR_TIM_NULL_PARAM, "tim_stop: null tim");
    return;
  }

  sysst8_enter_critical();
  me->running = false;
  sysst8_tim_hw_stop(me->hw_ch);
  sysst8_exit_critical();
}

void sysst8_tim_irq(sysst8_tim_t* me) {
  if (me == NULL || me->on_expire == NULL) {
    SYSST8_FCR_RAISE_MSG(SYSST8_FCR_TIM_NULL_PARAM, "tim_irq: null tim/handler");
    return;
  }

  // NOTE - Ngắt có thể đến sau khi stop() đã gọi, khi đó bỏ qua để không callback thừa
  if (!me->running) {
    return;
  }

  if (me->mode == SYSST8_TIM_MODE_ONESHOT) {
    me->running = false;
  }

  me->on_expire();
}

bool sysst8_tim_is_running(const sysst8_tim_t* me) {
  return (me != NULL) && me->running;
}
