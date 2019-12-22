/* standard library headers */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

/* BG stack headers */
#include "bg_types.h"
#include "gecko_bglib.h"

/* Own header */
#include "app.h"
#include "dump.h"
#include "support.h"

// App booted flag
static bool appBooted = false;
static struct {
  char *name;
  uint32 advertising_interval;
  uint16 connection_interval, mtu; 
  bd_addr remote;
  uint8 advertise, connection;
} config = { .remote = { .addr = {0,0,0,0,0,0}},
	     .connection = 0xff,
	     .advertise = 1,
	     .name = NULL,
	     .advertising_interval = 160, // 100 ms
	     .connection_interval = 80, // 100 ms
	     .mtu = 23,
};
  
void parse_address(const char *fmt,bd_addr *address) {
  char buf[3];
  int octet;
  for(uint8 i = 0; i < 6; i++) {
    memcpy(buf,&fmt[3*i],2);
    buf[2] = 0;
    sscanf(buf,"%02x",&octet);
    address->addr[5-i] = octet;
  }
}

uint8 ad_flags(uint8 *buffer, uint flags) {
  if(0 == flags) return 0;
  buffer[0] = 2;
  buffer[1] = 1;
  buffer[2] = flags;
  return 3;
}

uint8 ad_name(uint8 *buffer, char *name) {
  uint8 len = strlen(name);
  buffer[0] = len + 1;
  buffer[1] = 9;
  memcpy(&buffer[2],(uint8*)&name[0],len);
  return len + 2;
}

uint8 ad_manufacturer(uint8 *buffer, uint8 *data, uint8 len) {
  buffer[0] = len + 3;
  buffer[1] = 0xff;
  buffer[2] = 0xff;
  buffer[3] = 0xff;
  memcpy(&buffer[4],data,len);
  return len + 4;
}

const char *getAppOptions(void) {
  return "";
}

void appOption(int option, const char *arg) {
  switch(option) {
  default:
    fprintf(stderr,"Unhandled option '-%c'\n",option);
    exit(1);
  }
}

void appInit(void) {
}

/***********************************************************************************************//**
 *  \brief  Event handler function.
 *  \param[in] evt Event pointer.
 **************************************************************************************************/
void appHandleEvents(struct gecko_cmd_packet *evt)
{
  if (NULL == evt) {
    return;
  }

  // Do not handle any events until system is booted up properly.
  if ((BGLIB_MSG_ID(evt->header) != gecko_evt_system_boot_id)
      && !appBooted) {
#if defined(DEBUG)
    printf("Event: 0x%04x\n", BGLIB_MSG_ID(evt->header));
#endif
    millisleep(50);
    return;
  }

  /* Handle events */
#ifdef DUMP
  switch (BGLIB_MSG_ID(evt->header)) {
  default:
    dump_event(evt);
  }
#endif
  switch (BGLIB_MSG_ID(evt->header)) {
  case gecko_evt_system_boot_id:
    appBooted = true;
    gecko_cmd_sm_delete_bondings();
    exit(0);
    break;

  default:
    break;
  }
}
