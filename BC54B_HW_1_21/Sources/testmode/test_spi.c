/***************************************************
*
*				新增 spi 通信检测 2016/11/9  
*
***************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


#include "os_cpu.h"
#include "stdint.h"
#include "spi_hardware.h"
//#include "spi_platform.h"
#include "gpio_interface.h"
//#include "test_spi.h"
#include "bms_error.h"
#include "spi_interface.h"
#include "uCOS_II.h"


typedef struct spi_test_platform *spi_test_platform_t;

static const struct hcs12_gpio cs_pin = {&PTIH, &PTH, &DDRH, 1 << 3 };
//static const struct hcs12_gpio clk_pin = {&PTIH, &PTH, &DDRH, 1 << 2 };
//static const struct hcs12_gpio mosi_pin = {&PTIH, &PTH, &DDRH, 1 << 1 };
//static const struct hcs12_gpio miso_pin = {&PTIH, &PTH, &DDRH, 1 << 0 };

static const struct hcs12_spi_platform test_hcs12_spi = {
    (struct hcs12_spi_regs *)0xF0,  // SPI1
    (1 << 2) | (1 << 0), //baud_reg_value
    1,
    &cs_pin,
};

static const struct spi_bus test_spi_bus = {
  &test_hcs12_spi, &hcs12_spi_ops
};


const char shell_help_spi[]	= "\
  dev ops xx\n\
	dev: 设备号, 必须为0\n\
	ops xxxx\n\
		read xx: 从spi接收数据并打印, xx为超时时间(ms);\n\
		send xx: 通过spi发送数据, xx为需要发送的数据;\n\
";

const char shell_summary_spi[]="\
	SPI 操作相关";


void test_spi_init(void)
{
    (void) spi_init(&test_spi_bus, 1); //test_spi初始化 
}

static uint8_t write_to_spi(spi_bus_t test_spi_bus, char *s) //spi_bus
{   
    uint8_t len = (uint8_t)strlen(s);
    uint8_t send_buff[16] = {0}; 
    uint8_t i;
    
    if(len > 15){
      (void)printf("error : char num > 15\n");
      return -1;
    }
    
    for(i=0;i<len; i++){
        send_buff[i] = *(s + i);
    }
    
    send_buff[len] = 0x20;
    
      
    (void)test_spi_bus->ops->select(test_spi_bus->private_data, 0, 1);
	
    (void)test_spi_bus->ops->transfer(test_spi_bus->private_data, 0, &send_buff, sizeof(send_buff));
	  (void)test_spi_bus->ops->select(test_spi_bus->private_data, 0, 0);
	  
	  return 0;
}

static void recv_form_spi(spi_bus_t test_spi_bus, uint16_t timeout){

    uint8_t recv_flag = 0;
    uint16_t i = 0;
    byte count = 0;
							            //id  data---
    byte send_buff[16] = {0x05,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    byte recv_buff[16] = {0};
    byte show_buff[16] = {0};
    
    
	  (void)test_spi_bus->ops->select(test_spi_bus->private_data, 0, 1);
	
	  
    while(!recv_flag){
        OSTimeDly(1);
         i++;
         if(i > timeout){
            (void)printf("timeout :not reaceived data\n");
            return;
         }
		     (void)test_spi_bus->ops->transfer(test_spi_bus->private_data, &recv_buff[count], &send_buff[count], 1);
		     if(0x20 == recv_buff[count]){
		        recv_flag = 1;   
		     }
		     count ++;
		     if(count > 15){
		        count = 0; 
		     }
		         
    }    
    (void)test_spi_bus->ops->select(test_spi_bus->private_data, 0, 0);
    
    for(i=0;i<16;i++) {
        if(i<3)
          show_buff[15-i] = recv_buff[3-i];
        else
          show_buff[15-i] = recv_buff[18-i];
    }
    
	  (void)printf("%s\n", show_buff);
    
}


int shell_func_spi(int argc, char **argv) {

	if (argc != 4) {
		goto __error_params;
	}
	if (strcmp(argv[1], "0") != 0) {
		goto __error_params;
	}

	if (strcmp(argv[2], "read") == 0) { // read recv buffer
	
		recv_form_spi(&test_spi_bus, atoi(argv[3]));
	  
		return 0;
	}

	if (strcmp(argv[2], "send") == 0) { // send argv[3]
	
	  OSTimeDly(100);
		if(write_to_spi(&test_spi_bus, argv[3])){
		    return -1;
		}
		return 0;
	}

__error_params:
	printf("arguments error, \"help %s\" for more information\n", argv[0]);
	return -1;

}

