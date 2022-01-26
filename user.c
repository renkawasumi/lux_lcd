#include <pigpiod_if2.h>
#include <stdio.h>

#define LED 27

int main(){
    printf("pigpio\n");
    int pi = pigpio_start(NULL, NULL);
    gpio_write(pi, LED, 0);
    return 0;
}
