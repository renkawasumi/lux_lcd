#include <pigpiod_if2.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSZ 1024
#define LED 27

int main(){
    printf("start user\n");
    int fd, r, w;
    char buff;
    char *path = "/sys/module/smile/myport/xyz";
    //char *path = "/home/pi/test.txt"; 
    while(1){
        fd = open(path, O_RDWR, 0666);
        if(fd < 0){
            printf("OPEN_ERROR\n");
            return -1;
        }
        r = read(fd, &buff, 1);
        if(r < 0){
            printf("READ ERROR\n");
            return -1;
        }
        //buff[r]='\0';
        printf("r:%d buff:%c\n", r, buff);
        if(buff == 't'){
            printf("GOOD\n");
        }
        else{
            printf("NOT\n");
        }
        if(close(fd) < 0){
            printf("CLOSE ERROR\n");
            return -1;
        }
    }

    /*while(1){
        r = read(fd, buff, 100);
        if(r < 0){
            printf("READ ERROR\n");
            return -1;
        }
        if(lseek(fd, sizeof(0), SEEK_SET) < 0){
            printf("LSEEK ERROR\n");
            return -1;
        }
        buff[r]='\0';
        printf("r:%d buff:%s\n", r, buff);
    }*/
    if(close(fd) < 0){
        printf("CLOSE ERROR\n");
        return -1;
    }

    /*int pi = pigpio_start(NULL, NULL);
    gpio_write(pi, LED, 0);*/

    /*char buf[BUFSZ];
    char buf2[BUFSZ];
    int fid;
    int len;
    fid = open("/dev/smile0", O_RDWR);
    len = sprintf(buf, "Hello\n");
    len = write(fid, buf, len);
    write(1, buf, len);
    lseek(fid, 0, 0);
    len = read(fid, buf2, BUFSZ);
    write(1, buf2, len);
    close(fid);*/
    return 0;
}
