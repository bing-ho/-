/**  
*  
* Copyright (c) 2012 Ligoo Inc.  
*  
* @file bms_j1939.h
* @brief 
* @note  
* @author
* @date 2012-5-23  
*  
*/

#ifndef BMS_J1939_H_
#define BMS_J1939_H_
#include "bms_can.h"

/*****************************************************
 *  Definition & Structures
*****************************************************/
typedef struct J1939Context *j1939_t;

typedef struct
{
    INT8U  priority;
    INT32U pgn;
    INT8U  destination_address;
    INT8U  source_address;
    INT8U* data;
    INT16U data_size;
}J1939Message;

typedef Result (*J1939HandlerFunc)(j1939_t context, J1939Message* message, void* user_data);

typedef struct
{
    CanDev can_dev; //< Can Device
    INT8U  id;

    INT16U sending_timeout; // < Timeout for sending message
}J1939InitInfo;


/*****************************************************
 *  Init functions
*****************************************************/
j1939_t j1939_init(J1939InitInfo* init_info);
void j1939_uninit(j1939_t context);

/*****************************************************
 *  application layer interface
*****************************************************/
Result j1939_heart_beat(j1939_t context);

Result j1939_register_handler(j1939_t context, INT32U pgn, J1939HandlerFunc func, void* user_data);
Result j1939_register_default_handler(j1939_t context, J1939HandlerFunc func, void* user_data);

Result j1939_post_message(j1939_t context, J1939Message* message);
Result j1939_send_message(j1939_t context, J1939Message* request, J1939Message* response);



#endif /* BMS_J1939_H_ */
