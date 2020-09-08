#ifndef DMX_H
#define DMX_H

/*
 *  Headers
 */

#include "structs.h"
//Serial port
#include <termios.h>
#include <sys/ioctl.h>
//#include <IOKit/serial/ioss.h>

/*
 *   Methods
 */

void* startDMX(void * params);
void sendDmx(int serial_port);

#endif