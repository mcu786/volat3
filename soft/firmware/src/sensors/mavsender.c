#include "ch.h"
#include "hal.h"

#include "mavsender.h"
#include "param.h"
#include "message.h"
#include "main.h"
#include "sanity.h"
#include "sensors.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern Mailbox tolink_mb;
extern RawData raw_data;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static float *T_tlm;

static mavlink_mpiovd_sensors_raw_t     mpiovd_sensors_raw_struct;
static mavlink_mpiovd_sensors_scaled_t  mpiovd_sensors_scaled_struct;

/*
 ******************************************************************************
 * PROTOTYPES
 ******************************************************************************
 */

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

/**
 *
 */
static WORKING_AREA(TLM_SenderThreadWA, 128);
static msg_t TLM_SenderThread(void *arg) {
  chRegSetThreadName("TLM_Sender");
  (void)arg;

  Mail raw_mail    = {NULL, MAVLINK_MSG_ID_MPIOVD_SENSORS_RAW, NULL};
  Mail scaled_mail = {NULL, MAVLINK_MSG_ID_MPIOVD_SENSORS_SCALED, NULL};

  while (TRUE) {
    chThdSleepMilliseconds(200);

    if (raw_mail.payload == NULL){
      mpiovd_sensors_raw_struct.sec = TIME_BOOT_MS;
      mpiovd_sensors_raw_struct.analog01 = ((uint16_t)raw_data.temp_tmp75) / 256;
      mpiovd_sensors_raw_struct.engine_uptime = chTimeNow();

      raw_mail.payload = &mpiovd_sensors_raw_struct;
      chMBPost(&tolink_mb, (msg_t)&raw_mail, TIME_IMMEDIATE);
    }

    if (scaled_mail.payload == NULL){
      mpiovd_sensors_scaled_struct.sec = TIME_BOOT_MS;
      scaled_mail.payload = &mpiovd_sensors_scaled_struct;
      chMBPost(&tolink_mb, (msg_t)&scaled_mail, TIME_IMMEDIATE);
    }

  }
  return 0;
}

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

/**
 *
 */
void MavSenderInit(void){
  T_tlm = ValueSearch("T_tlm");

  chThdCreateStatic(TLM_SenderThreadWA,
          sizeof(TLM_SenderThreadWA),
          LINK_THREADS_PRIO - 1,
          TLM_SenderThread,
          NULL);
}


