#ifndef DMX_H
#define DMX_H

/*
 *  Headers
 */

#include "structs.h"

/*
 *   Methods
 */

void* startDMX(void * params);
void sendDmx(int serial_port);

#endif