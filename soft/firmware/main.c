#include "ch.h"
#include "hal.h"

#include "link.h"
#include "message.h"
#include "main.h"
#include "sanity.h"
#include "board.h"
#include "eeprom.h"
#include "i2c_local.h"
#include "sensors.h"

/* heap for (link threads) OR (shell thread)*/
MemoryHeap LinkThdHeap;
static uint8_t link_thd_buf[LINK_THD_HEAP_SIZE + sizeof(stkalign_t)];

/* ���������������� ���� EEPROM */
EepromFileStream EepromFile;

uint32_t GlobalFlags;

/*
 * Application entry point.
 */
int main(void) {
  halInit();
  chSysInit();

  /* ������������� ���� ��� ������ ������ ���� */
  chHeapInit(&LinkThdHeap, (uint8_t *)MEM_ALIGN_NEXT(link_thd_buf), LINK_THD_HEAP_SIZE);

  GlobalFlags = 0;

  EepromOpen(&EepromFile);

  MsgInit();
  MavInit();
  LinkInit();
  SanityControlInit();
  I2CInitLocal();
  SensorsInit();

  while (TRUE) {
    chThdSleepMilliseconds(666);
  }
  return 0;
}


