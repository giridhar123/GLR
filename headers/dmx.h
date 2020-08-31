#ifndef DMX_H
#define DMX_H

/*
 *  Headers
 */

// Basic headers
#include <stdio.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <string.h>
#include <errno.h> // Error integer and strerror() function
#include <unistd.h> // write(), read(), close()

//Serial port
#include <termios.h>
#include <sys/ioctl.h>
//#include <IOKit/serial/ioss.h>

/*
 *  Variables
 */

unsigned char dmxUniverse[513];


/*
*   Methods
 */

void* startDMX(void * params);

#endif