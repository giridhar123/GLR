#include "headers/dmx.h"
#include "headers/sharedVariables.h"

void* startDMX(void * params)
{
    char * port = (char *) params;
    printf("Opening serial port...%s\n" , port);

    // Cerco uno slot libero nell'array contenente i nomi delle porte seriali
    int ThreadNumber = -1;

    for (int i = 0; i < N_THREADS; ++i)
    {
        if (DmxName[i] == NULL)
        {
            ThreadNumber = i;
            break;
        }
    }

    if (ThreadNumber == -1)
    {
        printf("There are no free slot for a new thread.\n");
        return NULL;
    }

    DmxOpen[ThreadNumber] = 1;
    DmxName[ThreadNumber] = malloc(sizeof(char) * strlen(port));
    DmxName[ThreadNumber] = strcpy(DmxName[ThreadNumber], port);
   
    // Apertura porta seriale
    printf("THREAD: %d\nApertura porta seriale...: %s\n", ThreadNumber, port);
    int serial_port = open(port, O_WRONLY);
    
    // Controllo errori
    if (serial_port < 0) {
        printf("Error %i from open: %s\nClosing serial port thread\n", errno, strerror(errno));
        return NULL;
    }

    printf("Opened port...\n");
    
    // Creazione di una termios struct, ci scriveremo la configurazione esistente.
    struct termios tty;

    // Lettura di configurazioni già esistenti e gestione eventuali errori
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

    // Salvataggio impostazioni tty e gestione eventuali errori
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\nClosing serial port thread\n", errno, strerror(errno));
        return NULL;
    }

    //Imposto il baud rate a basso livello perchè la liberia OSX è differente da quella linux.
    speed_t speed = (speed_t)250000;
    ioctl(serial_port, IOSSIOSPEED, &speed); // Su windows il termine IOSSISPEED non funziona poiché viene dalla libreria IOKit/serial/ioss.h (bisogna scaricare la libreria online)
                                                //https://developer.apple.com/documentation/iokit
    ioctl(serial_port, TIOCSBRK); // Start break
    while(DmxOpen[ThreadNumber])
    {
        usleep(100);
        sendDmx(serial_port);
        usleep(18000);
    }

    myFree(DmxName[ThreadNumber]);
    DmxName[ThreadNumber] = NULL;
    printf("Serial port closing...\n");
    close(serial_port);
    printf("Serial port closed...\n");
  
    return NULL;
}

void sendDmx(int serial_port)
{
     // Stop break
    ioctl(serial_port, TIOCCBRK);

    // Invio il vettore da 512 + canale 0
    write(serial_port, dmxUniverse, sizeof(dmxUniverse));

    // Faccio il flush della serialport per poi riscrivere.
    tcflush(serial_port, TCIOFLUSH);
    
    // Start break
    ioctl(serial_port, TIOCSBRK); 
}
