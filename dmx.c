#include "headers/dmx.h"

void* startDMX(void * params)
{
    // Iniziliazzazione del vettore universare
    for (int i = 0; i < 513; ++i)
        dmxUniverse[i] = 0;

    printf("Opening serial port...\n");
    // Apertura porta seriale
    int serial_port = open("/dev/cu.usbserial-A50285BI", O_WRONLY);

    // Controllo errori
    if (serial_port < 0) {
        printf("Error %i from open: %s\nClosing serial port thread\n", errno, strerror(errno));
        return NULL;
    }

    printf("Opened port...\n");

    // Create new termios struc, we call it 'tty' for convention
    // No need for "= {0}" at the end as we'll immediately write the existing
    // config to this struct
    
    // Creazione di una termios struct, ci scriveremo la configurazione esistente.
    struct termios tty;

    // Read in existing settings, and handle any error
    // Lettura di configurazioni già esistenti.
    if(tcgetattr(serial_port, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\nClosing serial port thread\n", errno, strerror(errno));
        return NULL;
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag |= CSTOPB;  // Set stop field, two stop bits used in communication
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    tty.c_cc[VTIME] = 0;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    // Save tty settings, also checking for error
    // Salvataggio impostazioni tty.
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\nClosing serial port thread\n", errno, strerror(errno));
        return NULL;
    }

    //Imposto il baud rate a basso livello perchè la liberia OSX è differente da quella linux.
    speed_t speed = (speed_t)250000;
    //ioctl(serial_port, IOSSIOSPEED, &speed); //@TODO Su windows il termine IOSSISPEED non funziona poiché viene dalla libreria IOKit/serial/ioss.h (bisogna scaricare la libreria online)
                                                //https://developer.apple.com/documentation/iokit
    ioctl(serial_port, TIOCSBRK); //Start break
    while(1)
    {
        usleep(100);
        ioctl(serial_port, TIOCCBRK); //Stop break

        write(serial_port, dmxUniverse, sizeof(dmxUniverse));

        tcflush(serial_port, TCIOFLUSH);

        ioctl(serial_port, TIOCSBRK); //Start break

        usleep(18000);
    }

    printf("Serial port closing...\n");
    close(serial_port);
    printf("Serial port closed...\n");

    return NULL;
}
