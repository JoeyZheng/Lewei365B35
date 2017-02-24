#include <time.h>
#include "SerialPort.h"
#include "LeweiClient.h"

#define BRATE B57600
#define PORTDEV "/dev/ttyUSB0"                                              

//put your api key here,find it in lewei50.com->my account->account setting
//#define LW_USERKEY "YOUR_API_KEY_HERE\r\n"
////put your gateway number here,01 as default
#define LW_GATEWAY "02"
#ifndef LW_USERKEY

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

/* verify data and avoid sending the incorrect ones to Lewei50. */
int verifydata(int value[7]) {
    // 0.3um
    if (value[0] <= 0) return -1; 
    // PM2.5
    if ((value[1] < 0) || (value[1] > 1000)) return -2;
    // HCHO
    if (value[2] < 0) return -3;
    // CO2
    if (value[3] < 300) return -4;
    // Temperature
    if ((value[4] < 10) || (value[4] > 45)) return -5;
    // Humidity
    if ((value[5] < 0) || (value[5] > 99)) return -6;
    // Order
    if (value[6] <= 0) return -7;
    return 0;
} 
    

int main() {
    time_t timep;
    LeweiClient_t *lwc;
    char name[2], buf[255];
    int vnum, value[7], portfd, ret;
    char lwc_begin, sndretries, rptimes;

#ifndef LW_USERKEY
#define KEYLEN 40
    int fd;
    char *LW_USERKEY;

    /* get userkey from userkey file. */
    if ((fd = open("../userkey", O_RDONLY)) < 0) {
        perror("ERROR openning userkey file");
        return(-1);
    }
    LW_USERKEY = (char *)malloc(KEYLEN);
    memset(LW_USERKEY, 0, KEYLEN);
    if (read(fd, LW_USERKEY, KEYLEN) < 0) {
#ifndef LW_USERKEY
        free(LW_USERKEY);
#endif
        perror("ERROR read userkey");
        return(-1);
    }

    close(fd);
#endif

    /* Open port */
    if ((portfd = OpenPort(PORTDEV, BRATE)) > 0) {
        lwc = (LeweiClient_t *)malloc(sizeof(lwc));
        /* initial LWC env */
        if (LWCsetup(lwc, LW_USERKEY, LW_GATEWAY) >= 0) {
            /* looping to read data and send to Lewei50. */
            rptimes = 0;
            while(1) {
                ret = 0;
                bzero(buf, sizeof(buf));
                /* get data from USB port */
                if ((ret = ReadPort(portfd, buf)) <= 0) {
                    if (ret == -1) {
                        printf("Lose one B35 data by unknown reason.\n");
                        continue;
                    } else if (ret == -2) {
                        time(&timep);
                        printf("%s--->B35 may be disconnected! (tried %d times)\n", \
                                asctime(localtime(&timep)), ++rptimes);
                        if (rptimes >= 3) break;
                        close(portfd);
                        sleep(10);
                        if ((portfd = OpenPort(PORTDEV, BRATE)) > 0) 
                            continue;
                        else
                            break;
                    }
                    printf("return 0 from ReadPort\n");
                }
                rptimes = 0;
                ret = 0;
                sscanf(buf, "%d,%d,%d,%d,%d,%d,%d", &value[0], &value[1], 
                        &value[2], &value[3], &value[4], &value[5], &value[6]);
                if ((ret = verifydata(value)) < 0) {
                    printf("skip invalid data (%d).\n", ret);
                    continue;
                }

                ret = 0;
                for(vnum = 0; vnum < 7; vnum++) {
                    sprintf(name, "V%d", (vnum + 1));
                    if ((ret = LWCappend(lwc, name, value[vnum], vnum)) <= 0) break; 
                }
                if (ret <= 0) continue;

                /* send data to Lewei50 platform. */
                ret = 0;
                sndretries = 0;
                do {
                    if ((ret = LWCsend(lwc)) < 0) {
                        if (ret == -2) break;
                        sndretries++;
                        time(&timep);
                        printf("%s--->Failed sending data (%d):%s", \
                                asctime(localtime(&timep)), sndretries, buf);
                    } else break;
                } while (sndretries < 3);
                if (sndretries >= 3) break;
                printf("%s", buf);
            }
        } else {
            printf("Failed setup env.\n");
        }
        if (lwc->head != NULL) 
            free(lwc->head);
        if (lwc->user_data != NULL) 
            free(lwc->user_data);
        if (lwc != NULL) 
            free(lwc);
        close(portfd);
    }

#ifndef LW_USERKEY
    if (LW_USERKEY) 
        free(LW_USERKEY);
#endif
    printf("Exit!\n");
    return 0;
}
