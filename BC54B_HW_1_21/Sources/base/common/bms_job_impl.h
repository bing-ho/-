/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_job_impl.h
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-6-29
*
*/

#ifndef BMS_JOB_IMPL_H_
#define BMS_JOB_IMPL_H_
#include "bms_job.h"

typedef struct
{
    JobHandler handler;
    void*      user_data;
    INT16U     interval;
    INT32U     last_tick;
    JobId      next;
}JobContext;

typedef struct
{
    INT16U    count;
    JobId     next_id;
}JobGroupContext;


JobId job_find_tail(JobGroupId group);
JobId job_find_availabled(void);

#endif /* BMS_JOB_IMPL_H_ */
