
/* Следит за состоянием здоровья органов.*/

#include "ch.h"
#include "hal.h"

#include "message.h"
#include "main.h"
#include "link.h"

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern Mailbox tolink_mb;
extern mavlink_system_t mavlink_system_struct;

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
/* указатель на Idle поток. Оттуда мы будем брать данные для расчета загрузки проца */
static Thread *IdleThread_p = NULL;

/* переменные для оценки загруженности процессора */
static uint32_t last_sys_ticks = 0;
static uint32_t last_idle_ticks = 0;

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

/**
 * посылает heartbeat пакеты и моргает светодиодиком
 */
static WORKING_AREA(SanityControlThreadWA, 256);
static msg_t SanityControlThread(void *arg) {
  chRegSetThreadName("Sanity");
  (void)arg;

  mavlink_heartbeat_t mavlink_heartbeat_struct;
  Mail heartbeat_mail = {NULL, MAVLINK_MSG_ID_HEARTBEAT, NULL};

  mavlink_heartbeat_struct.autopilot = MAV_AUTOPILOT_GENERIC;
  mavlink_heartbeat_struct.custom_mode = 0;

  while (TRUE) {
    palSetPad(IOPORT3, GPIOC_LED);
    chThdSleepMilliseconds(950);

    if (heartbeat_mail.payload == NULL){
      mavlink_heartbeat_struct.type           = mavlink_system_struct.type;
      mavlink_heartbeat_struct.base_mode      = mavlink_system_struct.mode;
      mavlink_heartbeat_struct.system_status  = mavlink_system_struct.state;
      heartbeat_mail.payload = &mavlink_heartbeat_struct;
      chMBPost(&tolink_mb, (msg_t)&heartbeat_mail, TIME_IMMEDIATE);

      palClearPad(IOPORT3, GPIOC_LED); /* blink*/
      chThdSleepMilliseconds(50);
    }
  }
  return 0;
}

/**
 * @brief   Return number of system ticks since last call.
 * @note    Function modifies the last value itself.
 *
 * @param[in] last      pointer to the value containing last call time
 * @return              The number of ticks.
 *
 * @api
 */
systime_t GetTimeInterval(systime_t *last){
  systime_t t = 0;

  if (chTimeNow() >= *last)
    t = chTimeNow() - *last;
  else /* произошло переполнение */
    t = chTimeNow() + (0xFFFFFFFF - *last);
  /* refresh last value */
  *last = chTimeNow();
  return t;
}

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */
void SanityControlInit(void){

  IdleThread_p = chSysGetIdleThread();

  chThdCreateStatic(SanityControlThreadWA,
          sizeof(SanityControlThreadWA),
          NORMALPRIO,
          SanityControlThread,
          NULL);
}

/**
 * Рассчитывает загрузку проца.
 * Возвращает десятые доли процента.
 */
uint16_t get_cpu_load(void){

  uint32_t i, s; /* "мгновенные" значения количества тиков idle, system */

  /* получаем мгновенное значение счетчика из Idle */
  if (chThdGetTicks(IdleThread_p) >= last_idle_ticks)
    i = chThdGetTicks(IdleThread_p) - last_idle_ticks;
  else /* произошло переполнение */
    i = chThdGetTicks(IdleThread_p) + (0xFFFFFFFF - last_idle_ticks);
  /* обновляем счетчик */
    last_idle_ticks = chThdGetTicks(IdleThread_p);

  /* получаем мгновенное значение счетчика из системы */
  s = GetTimeInterval(&last_sys_ticks);

  return ((s - i) * 1000) / s;
}




