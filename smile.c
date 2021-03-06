#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>

#include <linux/sched.h>
static char lux = 't';
static struct kobject *kobj01;

static ssize_t                    // 書き込んだ文字列の長さ
xyz_show(struct kobject *kobj,    // 注目しているカーネルオブジェクト
         struct kobj_attribute *attr,    // 注目している属性
         char *buf                // この番地にユーザランドに渡す情報を書く
){
    printk("PID:%d SHOW %s:%s:%c\n", current->pid, kobj->name, attr->attr.name, lux);    
    return sprintf(buf, "%c\n", lux); 
}
 
static ssize_t                    // 読みだした文字列の長さ
xyz_store(struct kobject *kobj,    // 注目しているカーネルオブジェクト
          struct kobj_attribute *attr,    // 注目している属性
          const char *buf,        // ユーザランドから渡される文字列の場所
          size_t count            // ユーザランドから渡された文字列の長さ
){
    printk("PID:%d STORE %s:%s:%c-->", current->pid, kobj->name, attr->attr.name, lux); 
    sscanf(buf, "%c", &lux);
    printk("%c\n", lux); 
    return count;
}
 
static struct kobj_attribute myatt =__ATTR(xyz, 0660, xyz_show, xyz_store);

static struct i2c_board_info info_lcd = { // 雛形を書き換え
    .type = "aqm0802a", // 名前
    .addr = 0x3e,   // アドレス
    .flags = I2C_CLIENT_WAKE,
};

static struct i2c_board_info info_lux = { // 雛形を書き換え
    .type = "bh1750", // 名前
    .addr = 0x23,   // アドレス
    .flags = I2C_CLIENT_WAKE,
};

static int smile_thread(void *num)
{
    static struct i2c_client *my_lcd;
	static struct i2c_client *my_lux;
    struct i2c_adapter *adap;

    adap = i2c_get_adapter(1);  // I2Cバス1のアダプタ情報を参照
    if(adap == NULL)
        goto ERR01;
    printk("(o_o) adapter[%s]\n", adap->name);

    my_lcd = i2c_new_client_device(adap, &info_lcd);
    if(my_lcd == NULL)
        goto ERR02;
    printk("(^_^) device[%s:%02x]\n", my_lcd->name, my_lcd->addr);
	
	my_lux = i2c_new_client_device(adap, &info_lux);
    if(my_lux == NULL)
        goto ERR02;
    printk("(^_^) device[%s:%02x]\n", my_lux->name, my_lux->addr);


    unsigned char lcd_buf[2] = {};
    lcd_buf[0] = 0x00; 
    //----- I2C通信開始 -----------     
    lcd_buf[1] = 0x38;
    i2c_master_send(my_lcd, lcd_buf, 2);// Function set
    msleep(1);
    lcd_buf[1] = 0x39;
    i2c_master_send(my_lcd, lcd_buf, 2);// Function set
    msleep(1);
    lcd_buf[1] = 0x14;
    i2c_master_send(my_lcd, lcd_buf, 2);// Internal OSC frequency
    msleep(1);
    lcd_buf[1] = 0x73;
    i2c_master_send(my_lcd, lcd_buf, 2); // Contrast set
    msleep(1);
    lcd_buf[1] = 0x56;
    i2c_master_send(my_lcd, lcd_buf, 2); // 
    msleep(1);
    lcd_buf[1] = 0x6c;
    i2c_master_send(my_lcd, lcd_buf, 2); // 
    msleep(1);
    lcd_buf[1] = 0x38;
    i2c_master_send(my_lcd, lcd_buf, 2); // 
    msleep(1);
    lcd_buf[1] = 0x01;
    i2c_master_send(my_lcd, lcd_buf, 2); // clear
    msleep(1);
    lcd_buf[1] = 0x0c;
    i2c_master_send(my_lcd, lcd_buf, 2); // display on 
    msleep(1);

	char dat[2];
	int lx, cnt;
	i2c_master_send(my_lux, "\x01", 1); // power on
	msleep(100);
	i2c_master_send(my_lux, "\x10", 1); // 
 
    lcd_buf[0] = 0x40;
    char *title = "Lux";
    int i;
    for(i = 0; i < strlen(title); i++){
    	lcd_buf[1] = title[i];
	    i2c_master_send(my_lcd, lcd_buf, 2);
    }
 
    while(!kthread_should_stop()) {// 停止を指示されたらループを抜ける
		lcd_buf[0] = 0x00;
    	lcd_buf[1] = 0x40 + 0x80;
    	i2c_master_send(my_lcd, lcd_buf, 2);
	
		cnt = i2c_master_recv(my_lux, dat, 2);	
		lx = (dat[0] * 256 + dat[1]) * 1000 * 6 / 5;
		lcd_buf[0] = 0x40;
		printk("lx:%d\n", lx);
		if(lx <= 10000){
			lux = 't';
		}
		else{
			lux = 'f';
		}
		char lcd_lux[32];
    	sprintf(lcd_lux, "%d.%d", lx / 1000, lx % 1000);
        for(i = 0; i < strlen(lcd_lux); i++){
	    	lcd_buf[1] = lcd_lux[i];
            i2c_master_send(my_lcd, lcd_buf, 2);
        }
		msleep(200);
    }	
	i2c_master_send(my_lux, "\x00", 1); // power off
	i2c_unregister_device(my_lux);  // I2Cデバイス登録を解除 
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
	int ret;
    printk("LOAD LCD (o_o)/\n");
    struct kobject *kobj00; // モジュール（/sys/module/smile/）のkobj
	kobj00 = &((THIS_MODULE->mkobj).kobj);
	printk("Module Name: %s\n", kobj00->name); // 確認
	// myport を /sys/module/smile/に追加
	kobj01 = kobject_create_and_add("myport", kobj00);
	if(kobj01==NULL)
         return -ENOMEM;
 	// xyz を /sys/module/smile/myportに追加        
	ret = sysfs_create_file( kobj01, &myatt.attr);
	if (ret) {
			kobject_put(kobj01);
			printk("(x_x) ERROR\n");
			return -EINVAL;
	}

    mytask = kthread_run(smile_thread, NULL, "ksmile");//スレッド実行
    return IS_ERR(mytask)? -ENOMEM : 0; 
}

void cleanup_module(void)
{
    kthread_stop(mytask);   // カーネルスレッドに停止を指示
    sysfs_remove_file(kobj01, &myatt.attr);
    kobject_put(kobj01);
    printk("UNLOAD LCD m(x_x)m\n");
}

MODULE_AUTHOR("b1801631@planet.kanazawa-it.ac.jp");
MODULE_LICENSE("GPL");
