#include<linux/module.h>
#include<linux/kthread.h>
#include<linux/delay.h>
#include<linux/i2c.h>

static struct i2c_board_info info_lcd = { // 雛形を書き換え
    .type = "lcd", // 名前
    .addr = 0x3e,   // アドレス
    .flags = I2C_CLIENT_WAKE,
};

static int smile_thread(void *num)
{
    static struct i2c_client *my_lcd;
    struct i2c_adapter *adap;

    adap = i2c_get_adapter(1);  // I2Cバス1のアダプタ情報を参照
    if(adap == NULL)
        goto ERR01;
    printk("(o_o) adapter[%s]\n", adap->name);

    // my_lcd = i2c_new_device(adap, &info_lux);   // I2Cデバイス登録
    // my_lcd = i2c_acpi_new_device(&info_lux, 1, 1);
    my_lcd = i2c_new_client_device(adap, &info_lcd);
    if(my_lcd == NULL)
        goto ERR02;
    printk("(^_^) device[%s:%02x]\n", my_lcd->name, my_lcd->addr);

    unsigned char buf[2] = {};
    buf[0] = 0x00; 
    //----- I2C通信開始 -----------     
    buf[1] = 0x38;
    i2c_master_send(my_lcd, buf, 2);// Function set
    msleep(1);
    buf[1] = 0x39;
    i2c_master_send(my_lcd, buf, 2);// Function set
    msleep(1);
    buf[1] = 0x14;
    i2c_master_send(my_lcd, buf, 2);// Internal OSC frequency
    msleep(1);
    buf[1] = 0x73;
    i2c_master_send(my_lcd, buf, 2); // Contrast set
    msleep(1);
    buf[1] = 0x56;
    i2c_master_send(my_lcd, buf, 2); // 
    msleep(1);
    buf[1] = 0x6c;
    i2c_master_send(my_lcd, buf, 2); // 
    msleep(1);
    buf[1] = 0x38;
    i2c_master_send(my_lcd, buf, 2); // 
    msleep(1);
    buf[1] = 0x01;
    i2c_master_send(my_lcd, buf, 2); // clear
    msleep(1);
    buf[1] = 0x0c;
    i2c_master_send(my_lcd, buf, 2); // display on 
    msleep(1);

    buf[0] = 0x40;
    
    char lcd_lux[32];
    sprintf(lcd_lux, "%d", 5);
    int i;
    for(i = 0; i < strlen(lcd_lux); i++){
        buf[1] = lcd_lux[i];
        i2c_master_send(my_lcd, buf, 2);
    }

    /*char *lcd_str = "12";
    int i;
    for(i = 0; i < strlen(lcd_str); i++){
        buf[1] = lcd_str[i];
        i2c_master_send(my_lcd, buf, 2);
    }*/
    

    /*while(!kthread_should_stop()) {// 停止を指示されたらループを抜ける
        buf[0] = 0x40;
        buf[1] = kstrtol("CPU", NULL, 16);
        i2c_master_send(my_lcd, buf, 2);
    }*/

    i2c_unregister_device(my_lcd);  // I2Cデバイス登録を解除 
ERR02: 
    i2c_put_adapter(adap);  // アダプタの参照終了を通達
ERR01:
    while(!kthread_should_stop())
        msleep(10); 
    return 0;
}

static struct task_struct *mytask = NULL;

int init_module(void)
{
    printk("LOAD LCD (o_o)/\n");
    mytask = kthread_run(smile_thread, NULL, "ksmile");//スレッド実行
    return IS_ERR(mytask)? -ENOMEM : 0; 
}

void cleanup_module(void)
{
    kthread_stop(mytask);   // カーネルスレッドに停止を指示
    printk("UNLOAD LCD m(x_x)m\n");
}

MODULE_AUTHOR("b1801631@planet.kanazawa-it.ac.jp");
MODULE_LICENSE("GPL");
