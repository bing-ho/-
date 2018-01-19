


#ifndef __MCP3421_HARDWARE_H__
#define __MCP3421_HARDWARE_H__
#include "bms_bcu.h"
#include "ucos_ii.h"
#include "iic_interface.h"
#include "app_cfg.h"

#ifndef __FAR
#define __FAR
#endif

#define MCP3421_ADDR_ADDRRESS  0x68  

#define MCP3421_ONBOARD_REF    2048   /*  mv */

                                     

typedef enum 
{
    MCP3421_ONESHOT = 0,
    MCP3421_CONTINUOUS,
  
}Mcp3421_ConvertMode;


typedef enum 
{
    MCP3421_12BIT_RESULT = 0,
    MCP3421_14BIT_RESULT,
    MCP3421_16BIT_RESULT,
    MCP3421_18BIT_RESULT, 

}Mcp3421_SamplePresion;

typedef enum 
{
    MCP3421_MULTIPLE_1 = 0,
    MCP3421_MULTIPLE_2,
    MCP3421_MULTIPLE_4,
    MCP3421_MULTIPLE_8,

}MCP3421_Gain_Amplifier;






typedef struct ST_Mcp3421_BspInterface 
{
    const struct i2c_bus *__FAR bus; 
    Mcp3421_ConvertMode    mcp3421_mode;
    Mcp3421_SamplePresion  mcp3421_presion;
    MCP3421_Gain_Amplifier mcp3421_pga;
     
    void (*init)(const struct ST_Mcp3421_BspInterface  *__FAR platform); 
  
    char (*i2c_write)(const struct ST_Mcp3421_BspInterface  *__FAR platform); 

    char (*i2c_read)(const struct ST_Mcp3421_BspInterface  *__FAR platform, unsigned long *pvalue);

    void (*delay_ms)(const struct ST_Mcp3421_BspInterface *__FAR iface, unsigned short howmany);
} Mcp3421_BspInterface;



extern const Mcp3421_BspInterface g_Mcp3421_BspInterface;
extern void MCP3421_Hardware_Init(const Mcp3421_BspInterface *__FAR iface); 

#endif
