#include <pigpiod_if2.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSZ 1024
#define LED 27

int main(){
    printf("start user\n");
    int pi = pigpio_start(NULL, NULL);
    //gpio_write(pi, LED, 0);

    char buf[BUFSZ];
    int fid;
    int len;
    fid = open("/dev/smile0", O_RDWR);
    len = sprintf(buf, "Hello\n");
    len = write(fid, buf, len);
    write(1, buf, len);
    close(fid);
    return 0;
}
