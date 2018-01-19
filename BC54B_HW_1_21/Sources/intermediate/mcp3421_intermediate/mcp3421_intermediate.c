
#include "mcp3421_intermediate.h" 
#include "ucos_ii.h"
#include "derivative.h"
#include "iic_interface.h"
#include "bms_bcu.h"

signed int Mcp3421_CurrentBuffer[8] ={0};
signed int last_current = 0;

static unsigned long Mcp3421_SingleConvert(const Mcp3421_BspInterface *__FAR iface);
static void Mcp3421_ResultToCurrent(const Mcp3421_BspInterface *__FAR iface,unsigned long result,signed int *cur);
static signed int Mcp3421_BubbleFunction(signed int *Buffer_Addr,unsigned char Bubble_Num,unsigned char Omit_Num);

void Mcp3421_Intermediate_Init(const Mcp3421_BspInterface *__FAR iface) 
{
    if (iface == NULL) 
    {
        return;
    }
    iface->init(iface);
}

static unsigned long Mcp3421_SingleConvert(const Mcp3421_BspInterface *__FAR iface) 
{

    unsigned short i = 0;    
    unsigned long temp;
    if (iface == NULL) 
    {
       
        return MCP3421_RESULT_ERROR;
    } 
	
	
	if (!iface->i2c_write(iface))   //写配置  开启转换
	{

	    return  MCP3421_RESULT_ERROR;
	}

       
    do    // 等待转换完成.
    { 
        iface->delay_ms(iface, 6);
        if (++i > 5) 
        {
            return MCP3421_RESULT_ERROR;

        }
        if (!iface->i2c_read(iface,&temp)) 
        {
            return MCP3421_RESULT_ERROR;
        }
    } while (temp&0x00000080);
	
    return ((temp>>8)&0x0000FFFF);    

}

static void Mcp3421_ResultToCurrent(const Mcp3421_BspInterface *__FAR iface,unsigned long result,signed int *cur) 
{
    unsigned long full_range,lsb;
    unsigned int temp,resistance;
    signed int value;   
    signed long  current_10ma = 0,full_voltage;  
 
    unsigned char pga;

  
    resistance = config_get(kCurrentDivideResIndex);
    full_range =  4096 << (iface->mcp3421_presion *2); 
    
    temp = (unsigned int)(result&0x0000FFFF);  
    
    value = (signed int)temp; 
     

                                                                                                
    lsb = ((unsigned long)MCP3421_ONBOARD_REF*2*1000*1000)/full_range;  //pv

    
       
    pga = (1 << iface->mcp3421_pga)*13;
    full_voltage = (signed long)value*lsb;
    
    if(resistance != 0) 
    {
        current_10ma= (signed long)(full_voltage/resistance/pga); 
    }
    
    if((current_10ma >= 0)&&((current_10ma%10) >= 5)) 
    {
        *cur = (signed int)(current_10ma/10 + 1);          
    } 
    else if((current_10ma < 0)&&((current_10ma%10) <= -5))
    {
        *cur = (signed int)(current_10ma/10 - 1);
    } 
    else
    {
        *cur = (signed int)(current_10ma/10);      
    } 
    
    *cur = 0 - *cur;
    
}

         
                
unsigned char Mcp3421_AvrgCurrent_Get(signed int *avrg_current)
{
    unsigned char i;
    unsigned char retval = 0;
    unsigned long ad_res;

    
 
    for(i = 0;i < 8;i++)
    {
        ad_res = Mcp3421_SingleConvert(&g_Mcp3421_BspInterface); 
        if(ad_res == MCP3421_RESULT_ERROR)
        {                  
            retval = 1;
            break;
        }
        Mcp3421_ResultToCurrent(&g_Mcp3421_BspInterface,ad_res,&Mcp3421_CurrentBuffer[i]);   
    }
    
    
    if(retval == 0)
    {
        *avrg_current = Mcp3421_BubbleFunction(Mcp3421_CurrentBuffer,8,1);
    } 

    

    return  retval;    
}
                                       
                                                         
static signed int Mcp3421_BubbleFunction(signed int *Buffer_Addr,unsigned char Bubble_Num, unsigned char Omit_Num)
{
    unsigned char LoopBig,LoopSmall;
    signed int temp;
    signed long sum = 0; 
    signed int  res;                                                                                   
    
    for(LoopBig = 0;LoopBig < Bubble_Num - 1;LoopBig++)
    {
        for(LoopSmall = LoopBig + 1;LoopSmall < Bubble_Num;LoopSmall++)
        {
            if(Buffer_Addr[LoopBig] > Buffer_Addr[LoopSmall])
            {                      
                temp = Buffer_Addr[LoopBig]; 
                Buffer_Addr[LoopBig] = Buffer_Addr[LoopSmall];
                Buffer_Addr[LoopSmall] = temp;
            }
        }
    }  
    for(LoopBig = Omit_Num; LoopBig < Bubble_Num - Omit_Num; LoopBig++)
    {
        sum += Buffer_Addr[LoopBig];    
    }                                                       
   
    res = (signed int)(sum/(Bubble_Num - 2*Omit_Num)); 
    return res; 
}




