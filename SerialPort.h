#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__

#include <sys/types.h>                                                        
#include <sys/stat.h>                                                         
#include <fcntl.h>                                                            
#include <termios.h>                                                          
#include <stdio.h>                                                            
#include <string.h>                                                            
#include <stdlib.h>
#include <unistd.h>

#define FALSE 0                                                               
#define TRUE 1                                                                

int OpenPort(char *portdev, int brate);
int ReadPort(int fd, char *buf);

#endif /* __SERIALPORT_H__ */
