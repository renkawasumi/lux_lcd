#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>

static unsigned char *buf;
static int len;

static int smile_open(struct inode *inode, struct file *filp){
    dev_t devid = inode->i_rdev;
    printk("DEV[%ld] %d:%d\n", inode->i_ino, imajor(inode), iminor(inode));
    printk("process %d\n", current->pid);
    return 0;
}

static int smile_release(struct inode *inode, struct file *filp){
    printk("close, pos%lld\n", filp->f_pos);
    printk("process: %d\n", current->pid);
    return 0;
}

static ssize_t smile_read(struct file *filp, char __user *ubuf, size_t count, loff_t *pos){
    printk("smile_read\n");
    printk("Read(%d)\n", (int)count);
    printk("process:%d\n", current->pid);
    printk("userland_addr:%p\n", ubuf);
    printk("pos:%lld\n", *pos);
    int r;
    r = simple_read_from_buffer(ubuf, count, pos, buf, len);
    printk("    ---> %lld\n", *pos);
    return r;
}

static ssize_t smile_write(struct file *filp, const char __user *ubuf, size_t count, loff_t *pos){
    printk("smile_write\n");
    printk("Write(%d)\n", (int)count);
    printk("process:%d\n", current->pid);
    printk("userland_addr:%p\n", ubuf);
    printk("pos:%lld\n", *pos);
    int r;
    r = simple_write_to_buffer(buf, PAGE_SIZE, pos, ubuf, count);
    len = count;
    printk("    ---> %lld\n", *pos);
    return r;
}

loff_t smile_lseek(struct file *filp, loff_t pos, int whence){
    printk("smile_lseek\n");
    printk("Lseek(%lld)\n", pos);
    return filp->f_pos = pos;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = smile_open,
    .release = smile_release,
    .read = smile_read,
    .write = smile_write,
    .llseek = smile_lseek,
    .unlocked_ioctl = NULL,
};

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
    char *title = "Title";
    int i;
    for(i = 0; i < strlen(title); i++){
    	lcd_buf[1] = title[i];
	    i2c_master_send(my_lcd, lcd_buf, 2);
    }
 
    while(!kthread_should_stop()) {// 停止を指示されたらループを抜ける
		lcd_buf[0] = 0x00;
    	lcd_buf[1] = 0x40 + 0x80;
    	i2c_master_send(my_lcd, lcd_buf, 2);
	
		lcd_buf[0] = 0x40;
		char *music_name = "Hoge";
    	for(i = 0; i < strlen(music_name); i++){
    		lcd_buf[1] = music_name[i];
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
    printk("LOAD LCD (o_o)/\n");
    if(register_chrdev(77, "smile-drv", &fops)){
        return -EBUSY;
    }
    buf = kmalloc(PAGE_SIZE, GFP_KERNEL);

    mytask = kthread_run(smile_thread, NULL, "ksmile");//スレッド実行
    return IS_ERR(mytask)? -ENOMEM : 0; 
}

void cleanup_module(void)
{
    kthread_stop(mytask);   // カーネルスレッドに停止を指示
    kfree(buf);
    unregister_chrdev(77, "smile-drv");
    printk("UNLOAD LCD m(x_x)m\n");
}

MODULE_AUTHOR("b1801631@planet.kanazawa-it.ac.jp");
MODULE_LICENSE("GPL");
