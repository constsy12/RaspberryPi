#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/delay.h>

#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/types.h>
#include <linux/ioport.h>
#include <linux/pid.h>
#include <linux/sched.h>

#include <asm/io.h>
#include <asm/delay.h>
#include <asm/irq.h>

#include <mach/map.h>
#include <mach/gpio.h>

#include <asm/gpio.h>
#include <mach/regs-gpio.h>

#include <plat/gpio-cfg.h>
#include <asm/unistd.h>


// Device Address
#define TACT 	0x0F 
#define LED	0x0A
#define FND	0x0B
#define CLCD	0x0C

#define ADDR_PIN_L_0(x)           EXYNOS4212_GPM0(x+1)
#define ADDR_PIN_L_1(x)           EXYNOS4212_GPM1(x)

#define ADDR_PIN_H_1(x)           EXYNOS4212_GPM1(x+1)

#define SEL(x)		ADDR_PIN_H_1(x)

/*gpio set */
#define DATA_PIN_2(x)             EXYNOS4212_GPM2(x)
#define DATA_PIN_3(x)             EXYNOS4212_GPM3(x)
#define DATA_PIN_4(x)             EXYNOS4212_GPM4(x)
#define DATA_PIN_0(x)             EXYNOS4212_GPM0(x)

#define RS                      ADDR_PIN_H_1(0)
#define WR                      ADDR_PIN_H_1(1)
#define EN                      ADDR_PIN_H_1(2)

#define MAX	10

MODULE_LICENSE("GPL");

int device_major = 0;	// major num is auto dynamic allocation
int result;
int front = 0;
int check = 0;

typedef struct QUEUE{
	int index;
	char str[30];
	char ex_t[30];
}Q;

Q Commhis[MAX] = {0};

unsigned char pos = 0x80;
static int n = 0;

static pid_t app_pid = 0;

static struct timer_list timer_str;
void init_add_timer(void);
unsigned char sw_data = 0x00;

unsigned char FND_DATA_TBL[]={
        0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,
        0x80,0x90,0x88,0x83,0xC6,0xA1,0x86,0x8E
};

unsigned char dot_data[8];
//unsigned char col[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
unsigned char col[9] = { 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

char CGRAM_TBL[6][8] = {{0x00,0x1F,0x01,0x01,0x01,0x01,0x01,0x00},
                        {0x00,0x04,0x04,0x06,0x04,0x04,0x04,0x00},
                        {0x00,0x00,0x0E,0x11,0x11,0x0E,0x00,0x00},
                        {0x00,0x04,0x04,0x0A,0x11,0x11,0x00,0x00},
                        {0x00,0x04,0x04,0x0C,0x04,0x04,0x04,0x00},
                        {0x00,0x1F,0x0A,0x0A,0x10,0x1F,0x00,0x00}};


void gpio_init(void){
        int i;

        //fpga data gpio init - gpm2_0~2
        for(i=0; i<3; i++){
                gpio_request(DATA_PIN_2(i), "GPM2_0");
                s3c_gpio_cfgpin(DATA_PIN_2(i), S3C_GPIO_SFN(1));
                gpio_direction_output(DATA_PIN_2(i), 1);
        }
        //fpga data gpio init - gpm0_0
        for(i=0; i<1; i++){
                gpio_request(DATA_PIN_0(i), "GPM0_0");
                s3c_gpio_cfgpin(DATA_PIN_0(i), S3C_GPIO_SFN(1));
                gpio_direction_output(DATA_PIN_0(i), 1);
        }
        //fpga data gpio init - gpm2_3~4
        for(i=3; i<5; i++){
                gpio_request(DATA_PIN_2(i), "GPM2_0");
                s3c_gpio_cfgpin(DATA_PIN_2(i), S3C_GPIO_SFN(1));
                gpio_direction_output(DATA_PIN_2(i), 1);
        }
        //fpga data gpio init - gpm3_0~7
        for(i=0; i<2; i++){
                gpio_request(DATA_PIN_3(i), "GPM3_0");
                s3c_gpio_cfgpin(DATA_PIN_3(i), S3C_GPIO_SFN(1));
                gpio_direction_output(DATA_PIN_3(i), 1);
        }
        //fpga address gpio init - gpm0_1~7 <= (0~6+1) //add 1
        for(i=0; i<7; i++){
                gpio_request(ADDR_PIN_L_0(i), "GPM0_0");
                s3c_gpio_cfgpin(ADDR_PIN_L_0(i), S3C_GPIO_SFN(1));
                gpio_direction_output(ADDR_PIN_L_0(i), 0);
                gpio_set_value(ADDR_PIN_L_0(i), 0);
        }
        //fpga address gpio init - gpm1_0~0
        for(i=0; i<1; i++){
                gpio_request(ADDR_PIN_L_1(i), "GPM0_0");
                s3c_gpio_cfgpin(ADDR_PIN_L_1(i), S3C_GPIO_SFN(1));
                gpio_direction_output(ADDR_PIN_L_1(i), 0);
                gpio_set_value(ADDR_PIN_L_1(i), 0);
        }
        //fpga address gpio init - gpm1_1~6
        for(i=0; i<6; i++){
                gpio_request(ADDR_PIN_H_1(i), "GPM1_0");
                s3c_gpio_cfgpin(ADDR_PIN_H_1(i), S3C_GPIO_SFN(1));
                gpio_direction_output(ADDR_PIN_H_1(i), 0);
                gpio_direction_output(ADDR_PIN_L_1(i), 0);
                gpio_set_value(ADDR_PIN_L_1(i), 0);
        }
        //fpga address gpio init - gpm1_1~6
        for(i=0; i<6; i++){
                gpio_request(ADDR_PIN_H_1(i), "GPM1_0");
                s3c_gpio_cfgpin(ADDR_PIN_H_1(i), S3C_GPIO_SFN(1));
                gpio_direction_output(ADDR_PIN_H_1(i), 0);
                gpio_set_value(ADDR_PIN_H_1(i), 0);
        }
}

void _DATA(unsigned char val)
{
        if(val & (1<<0))    gpio_set_value(DATA_PIN_2(0), 1);
        else                gpio_set_value(DATA_PIN_2(0), 0);
        if(val & (1<<1))    gpio_set_value(DATA_PIN_2(1), 1);
        else                gpio_set_value(DATA_PIN_2(1), 0);
        if(val & (1<<2))    gpio_set_value(DATA_PIN_2(2), 1);
        else                gpio_set_value(DATA_PIN_2(2), 0);

        if(val & (1<<3))    gpio_set_value(DATA_PIN_0(0), 1);
        else                gpio_set_value(DATA_PIN_0(0), 0);

        if(val & (1<<4))    gpio_set_value(DATA_PIN_2(3), 1);
        else                gpio_set_value(DATA_PIN_2(3), 0);
        if(val & (1<<5))    gpio_set_value(DATA_PIN_2(4), 1);
        else                gpio_set_value(DATA_PIN_2(4), 0);

        if(val & (1<<6))    gpio_set_value(DATA_PIN_3(0), 1);
        else                gpio_set_value(DATA_PIN_3(0), 0);
        if(val & (1<<7))    gpio_set_value(DATA_PIN_3(1), 1);
        else                gpio_set_value(DATA_PIN_3(1), 0);

}

static unsigned char TACTSW_DATA(void)
{
        int i;

        for(i=0; i<3; i++){
                sw_data |= ((gpio_get_value(DATA_PIN_2(i)))<<i);
        }
                sw_data |= (gpio_get_value(DATA_PIN_0(0)))<<3;
                sw_data |= ((gpio_get_value(DATA_PIN_2(3)))<<4);
                sw_data |= ((gpio_get_value(DATA_PIN_2(4)))<<5);
                sw_data |= ((gpio_get_value(DATA_PIN_3(0)))<<6);
                sw_data |= ((gpio_get_value(DATA_PIN_3(1)))<<7);
        return sw_data;
}

static void write_addr_l(unsigned char val){
        int i;
 
        for(i=0; i<7; i++){
                if(val & (1<<i)){
                        gpio_set_value(ADDR_PIN_L_0(i), 1);
                }
                else{
                        gpio_set_value(ADDR_PIN_L_0(i), 0);
                }
        }
        if(val & (1 << 7))
                gpio_set_value(ADDR_PIN_L_1(0), 1);
        else
                gpio_set_value(ADDR_PIN_L_1(0), 0);
}

void clcd_rs(unsigned char val){
        gpio_set_value(RS, val);
}
void clcd_wr(unsigned char val){
        gpio_set_value(WR, val);
}
void clcd_en(unsigned char val){
        gpio_set_value(EN, val);
}
static void clcd_command(unsigned char val)
{
        clcd_rs(0);
        clcd_wr(0);
        clcd_en(1);
        _DATA(val);
        mdelay(1);
        clcd_en(0);
        mdelay(1);

}
static void clcd_data_write(unsigned char val)
{
        clcd_rs(1);
        clcd_wr(0);
        clcd_en(1);
        _DATA(val);
        mdelay(1);
        clcd_en(0);
        mdelay(1);
}
static void lcd_init(void)
{
        clcd_command(0x30);
        mdelay(40);
        clcd_command(0x30);
        mdelay(1);
        clcd_command(0x30);
        mdelay(1);

        clcd_command(0x38);
        mdelay(100);
        clcd_command(0x0C);
        mdelay(100);
        clcd_command(0x01);
        mdelay(100);
        clcd_command(0x06);
        mdelay(100);
        clcd_command(0x80);
        mdelay(100);
}
void clcd_str(char *str){
        int i;

        for(i=0;i<strlen(str);i++){
                clcd_data_write(str[i]);
                if(i==15)
                        clcd_command(0xC0);
                udelay(100);
        }
}

void clcd_cgram_addr_set(char* cgram_addr){
        clcd_command(*cgram_addr | 0x40);
	udelay(100);
        *cgram_addr += 1;
}


void clcd_cgram_write(char (*arr)[8],char *cgram_addr){
        while(((*cgram_addr)/8)!=6){
                clcd_cgram_addr_set(cgram_addr);
                clcd_data_write(arr[(*cgram_addr)/8][(*cgram_addr)%8-1]);
        }
}

void clcd_print_name(){
        clcd_command(0x80);
        clcd_data_write(0x00);

        clcd_command(0x81);
        clcd_data_write(0x01);

        clcd_command(0x83);
        clcd_data_write(0x03);

        clcd_command(0x84);
        clcd_data_write(0x04);

        clcd_command(0x86);
        clcd_data_write(0x02);

        clcd_command(0xC1);
        clcd_data_write(0x02);

        clcd_command(0xC6);
        clcd_data_write(0x05);
}


void FND_SEL(unsigned char val)
{
        int i;
        for(i=0; i<4; i++){
                if(val & (1<<i)){
                        gpio_set_value(SEL(i), 1);
                }
                else{
                        gpio_set_value(SEL(i), 0);
                }
        }
}

void fnd_disp(int num, unsigned char val)
{
        write_addr_l(FND);
        switch(num)
        {
                case 0:
                        FND_SEL(0x00);
                        _DATA(val);
                        break;
                case 1:
                        FND_SEL(~0x01);
                        _DATA(val);
                        break;
                case 2:
                        FND_SEL(~0x02);
                        _DATA(val);
                        break;
                case 3:
                        FND_SEL(~0x04);
                        _DATA(val);
                        break;
                case 4:
                        FND_SEL(~0x08);
                        _DATA(val);
                        break;
                default :
                        FND_SEL(~0x00);
                        break;
        }
}


void timer_function(unsigned long ptr){
        kill_pid(find_vpid(app_pid),SIGUSR1,1);
        init_add_timer();
}


void init_add_timer(void){
        init_timer(&timer_str); // timer

        timer_str.function = &timer_function;
        timer_str.data = (unsigned long)n;
        timer_str.expires = jiffies + (HZ/10);

        add_timer(&timer_str);
}

void remove_timer(void){
        del_timer(&timer_str);
}


/* drv_ioctl function prototype */
extern int drv_ioctl(struct inode *inode, struct file *pfile, unsigned int command, unsigned long argument);

/* application: open()   <-->   D.D: drv_open()  Matching function */
extern int drv_open(struct inode *inode, struct file *pfile){
	gpio_init();
	printk("DRIVER open() run \n");
	write_addr_l(CLCD);
	lcd_init();
	clcd_command(0x80);
	//clcd_str("CALCULATOR RUN...");

	init_add_timer();
	return 0;
}

/* application: release()  <-->   D.D: drv_release()  Matching function */
extern int drv_release(struct inode *inode, struct file *pfile){
	printk("DRIVER release() run \n");
	remove_timer();
	return 0;
}

/* application: read()  <-->  D.D: drv_read()  Matching function */
ssize_t drv_read(struct file *pfile, char *buffer, size_t count, loff_t *f_pos){
	int i;
	char data = 0x00;

	if(count==sizeof(int)){
		printk("FRONT_SEND\n");
		i = copy_to_user(buffer,&front,sizeof(int));
	}else if(count==sizeof(Q)*MAX){
		printk("QUEUE_SEND\n");
		i = copy_to_user(buffer,&Commhis,sizeof(Q)*MAX);
	}else{
		write_addr_l(TACT);
		data = TACTSW_DATA();

		copy_to_user(buffer,&data,1);

		sw_data = 0x00;

		write_addr_l(LED);
		_DATA(~data);
	}
	return 0;
}

/* application: write()  <-->  D.D: swc_write()  Matching function */
ssize_t drv_write(struct file *pfile, const char *buffer, size_t count, loff_t *f_pos){
	int i;
	char str[32];
	int fnd_arr[4] = {0};
	int f_cnt=0;

	unsigned char data = 0x00;
	Q comm = {0};
	
	if(check==0){
		printk("\n######  write() run ######");
		printk("\n######  pid = %d    ######\n",*buffer);

		get_user(app_pid,(int*)buffer);
		check = 1;	
	}else{
		if(count==sizeof(Q)){
			i = copy_from_user(&comm,buffer,sizeof(Q));
			Commhis[front%MAX].index = comm.index;
			strcpy(Commhis[front%MAX].str,comm.str);
			strcpy(Commhis[front++%MAX].ex_t,comm.ex_t);
			front %=MAX;
		}else if(count==sizeof(int)*4){
			
			i = copy_from_user(fnd_arr,buffer,sizeof(int)*4);
			while(f_cnt!=1000){
	                        write_addr_l(FND);

        	                fnd_disp(1,FND_DATA_TBL[fnd_arr[0]]);
                	        mdelay(1);

                        	fnd_disp(2,FND_DATA_TBL[fnd_arr[1]]);
                        	mdelay(1);

                       		fnd_disp(3,FND_DATA_TBL[fnd_arr[2]]);
                        	mdelay(1);

                        	fnd_disp(4,FND_DATA_TBL[fnd_arr[3]]);
                        	mdelay(1);

                        	f_cnt++;
          	      }

		}else{
			copy_from_user(&app_pid,buffer,count);

		/*	write_addr_l(CLCD);

			if(count==1){
				get_user(data,buffer);
				clcd_command(pos++);
				clcd_data_write(data);	
			}else{
				copy_from_user(str,buffer,sizeof(str));
				clcd_command(pos);
				clcd_str(str);
				pos = 0x80;
	
				mdelay(1000);
				clcd_command(0x01);
			}
		*/
		}
	}
	return 0;
}

/* application: ioctl()  <-->  D.D: drv_ioctl()  Matching function */
extern int drv_ioctl(struct inode  *inode, struct file *pfile, unsigned int command, unsigned long argument){
	int i=0,j=0;
	int pcount = 0,p1=0,p2=0,p3=0,p4=0;
	unsigned char data = 0x01;
//	unsigned char row[8] = {0x00, 0x6C, 0XFE, 0XFE, 0X7C, 0X38, 0x10, 0x00};
	unsigned char row[8] = {0x00, 0x6C, 0x92, 0x82, 0x44, 0x28, 0x10, 0x00};
	gpio_init();

	switch(command){
		case 1:
			
			printk(" LED_BLINK Run...\n");
			write_addr_l(LED);
			
			for(i=0;i<2;i++){
				_DATA(0x00);
				mdelay(500);
				_DATA(0xFF);
				mdelay(500);
			}
			_DATA(0xFF);
			break;
		case 2:
			printk(" LED_SHIFT Run...\n");
			write_addr_l(LED);
			
			for(i=0;i<16;i++){
				data = ~(0x01<<(i%8));
				_DATA(data);
				mdelay(500);
			}
			_DATA(0xFF);
			break;
		case 3:
			printk(" LED_NEON Run...\n");
			write_addr_l(LED);
	
			for(i=0;i<18;i++){
				data = (0x01<<(i%9)) | (0x80>>(i%9));
				_DATA(~data);
				mdelay(500);
			}
			_DATA(0xFF);
			break;
		case 4:
			printk(" FND_Timer Run...\n");
			write_addr_l(FND);

		        while(1){
                		pcount++;
                		fnd_disp(4,FND_DATA_TBL[p1]);
                		mdelay(1);

               			fnd_disp(3,FND_DATA_TBL[p2]);
                		mdelay(1);

                		fnd_disp(2,FND_DATA_TBL[p3]);
                		mdelay(1);

                		fnd_disp(1,FND_DATA_TBL[p4]);
                		mdelay(1);

                		if(pcount%10==0)
                        		p1++;
                		if(pcount%10==0 && p1%10==0)
                        		p2++;
                		if(pcount%100==0 && p2%10==0)
                        		p3++;
                		if(pcount%1000==0 && p3%10==0)
                        		p4++;
				if(p4==10)
					break;
        		}
        		fnd_disp(0,0xFF);

			break;
		case 5:
			printk(" DOT_PRINT Run...\n");
			for(i=0;i<50000;i++){
				for(j=0;j<8;j++){
					write_addr_l(~col[j]);
					_DATA(~row[j]);
				}
			}
			for(j=0;j<8;j++){
				write_addr_l(~col[j]);
				_DATA(0xFF);
			}
			break;
		case 6:
			printk(" CLCD_PRINT Run...\n");
			clcd_command(0x01);
			clcd_command(0x80);
			clcd_str("Hello World");
			clcd_command(0xC0);
			mdelay(1);
			clcd_str("CLCD_Programming");
			break;
		case 7:
			remove_timer();
			printk(" CLCD_NAME Run...\n");
			write_addr_l(CLCD);
			//clcd_str("HELLO WORLD");
			clcd_print_name();
			init_add_timer();
			break;
	}	

	return 0;
}


struct file_operations device_fops = {
	.read = drv_read,
	.write = drv_write,
	.open = drv_open,
	.release = drv_release,
	.unlocked_ioctl = drv_ioctl,    // don't erase ','
};


int init_module(void){          // insmod
        result = register_chrdev(device_major,"mydrv",&device_fops);

        if(result<0){
                printk(" error device major number \n");
                return result;
        }
        printk("Major number : %d\n",result);
        return 0;
}

void cleanup_module(void){      // rmmod
        printk("\n  In case I can't see you again,");
        printk("\n  Good afternoon, Good evening, and Good Night \n\n");

        unregister_chrdev(result,"mydrv");  // Major num and device name
}

