#ifndef DMX_H
#define DMX_H

// Headers utilizzati
#include "structs.h"

// Serial port
#include <termios.h>
#include <sys/ioctl.h>
#include <IOKit/serial/ioss.h>


// Sezione dei metodi
void* startDmx(void * params);
void sendDmx(int serial_port);

#endif