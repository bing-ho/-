

#include "mcp3421_hardware.h"


void Mcp3421_Hardware_delay_ms(const Mcp3421_BspInterface *__FAR iface, unsigned short howmany) 
{
    (void)iface;
    OSTimeDly(howmany);
}



char Mcp3421_Hardware_Write(const Mcp3421_BspInterface *__FAR iface) 
{ 
    unsigned char config = 0;
   
    if(iface->mcp3421_mode == MCP3421_ONESHOT)       
    {
        config &= 0xEF;
    } 
    else 
    {
        config |= 0x10;
    }
    
    config += (iface->mcp3421_presion<<2) & 0x0C;
    
    config += iface->mcp3421_pga & 0x03;
    config |= 0x80;    

    if (1 != bus_i2c_transmit(iface->bus, MCP3421_ADDR_ADDRRESS, &config, 1, NULL, 0)) 
    {
        return 0;
    }
    return 1;
}


char Mcp3421_Hardware_Read(const Mcp3421_BspInterface *__FAR iface, unsigned long *pvalue) 
{
    unsigned char r_len = 3;
    unsigned char w_reg;
    unsigned char dat[3] = {0};
    
     

    if (r_len != bus_i2c_transmit(iface->bus, MCP3421_ADDR_ADDRRESS, &w_reg, 0, dat, r_len)) 
    {
        return 0;
    }
    
    *pvalue = ((((unsigned long)dat[0])&0x000000FF)<<16)+((((unsigned long)dat[1])&0x000000FF)<<8)+dat[2];
    return 1;
}



void MCP3421_Hardware_Init(const Mcp3421_BspInterface *__FAR iface) 
{
    (void)bus_i2c_init(iface->bus);
}




const Mcp3421_BspInterface g_Mcp3421_BspInterface = 
{

    &hcs12_i2c_bus_1,
    MCP3421_ONESHOT,
    MCP3421_12BIT_RESULT,
    MCP3421_MULTIPLE_2,
    MCP3421_Hardware_Init,
    Mcp3421_Hardware_Write,
    Mcp3421_Hardware_Read,
    Mcp3421_Hardware_delay_ms,
};





