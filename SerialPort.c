#include "SerialPort.h"
#include "dbg.h"

int OpenPort(char *portdev, int brate) {
    int fd,c, res;
    struct termios newtio;
    char buf[255];

    DBGPRINT("---enter OpenPort---\n");
    // Open device for reading and writing and not as controlling tty        
    if ((fd = open(portdev, O_RDONLY | O_NDELAY | O_NOCTTY)) < 0) {
        perror(portdev);
        return -1;
    }
    fcntl(fd, F_SETFL, 0);

    /* clear struct for new port settings */
    bzero(&newtio, sizeof(newtio)); 
    // tcgetattr(fd, &newtio);
    // cfsetispeed(&newtio, brate);
    // cfsetospeed(&newtio, brate);

    /*                                                                            
    BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
    CRTSCTS : output hardware flow control (only used if the cable has
              all necessary lines. See sect. 7 of Serial-HOWTO)
    CS8     : 8n1 (8bit,no parity,1 stopbit)
    CLOCAL  : local connection, no modem contol
    CREAD   : enable receiving characters
    */
    newtio.c_cflag = brate;
    newtio.c_cflag &= ~PARENB;
    newtio.c_cflag &= ~CSTOPB;
    newtio.c_cflag &= ~CSIZE;
    newtio.c_cflag |= CS8;

    /*
    IGNPAR  : ignore bytes with parity errors
    ICRNL   : map CR to NL (otherwise a CR input on the other computer 
              will not terminate input) otherwise make device raw (no other 
              input processing)                       
    */
    newtio.c_iflag = IGNPAR;

    // Raw output.                                                                  
    newtio.c_oflag = 0;

    /*                                                                            
    ICANON  : enable canonical input disable all echo functionality, and don't 
              send signals to calling program
    */                                                                            
    newtio.c_lflag = ICANON;
                                                                              
    /*                                                                            
    initialize all control characters 
    default values can be found in /usr/include/termios.h, and are given in the 
    comments, but we don't need them here
    */                                                                            
    newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */
    newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
    newtio.c_cc[VERASE]   = 0;     /* del */
    newtio.c_cc[VKILL]    = 0;     /* @ */
    newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
    newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
    newtio.c_cc[VSWTC]    = 0;     /* '\0' */
    newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */
    newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
    newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
    newtio.c_cc[VEOL]     = 0;     /* '\0' */
    newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
    newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
    newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
    newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
    newtio.c_cc[VEOL2]    = 0;     /* '\0' */

    // now clean the line and activate the settings for the port             
    if (tcflush(fd, TCIFLUSH) < 0) {
        perror("tcflush");
        return -1;
    }

    if (tcsetattr(fd,TCSANOW,&newtio) < 0) {
        close(fd);
        perror("tcsetattr");
        return -1;
    }
    
    return fd;
}

int ReadPort(int fd, char *buf) {
    int res, i;

    DBGPRINT("---enter ReadPort---\n");
    i = 0;
    res = 0;
#if 0
    do {
        if ((res = read(fd,buf,255)) < 0) {
            i++;
            DBGPRINT("ReadPort sleep %d seconds.\n", i);
            sleep(1);
        }
    } while((i < 35) && (res < 0));
#else
    res = read(fd,buf,255);
#endif

    if (res < 0) return -1;

    if (res == 0) return -2;

    buf[res] = 0;

    return res;
}                                                                             

