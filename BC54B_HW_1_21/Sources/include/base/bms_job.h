/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_task.h
* @brief
* @note
* @author
* @date 2012-5-9
*
*/

#ifndef BMS_JOB_H_
#define BMS_JOB_H_
#include "bms_defs.h"

typedef INT8U JobGroupId;
typedef INT16U JobId;

#ifndef BMS_MAX_JOB_NUM
#define BMS_MAX_JOB_NUM         23
#endif

#ifndef BMS_MAX_JOB_GROUP_NUM
#define BMS_MAX_JOB_GROUP_NUM   4
#endif

#define MAIN_JOB_GROUP          0


typedef void (*JobHandler)(void* data);

typedef struct
{
    INT16U     interval;
    JobHandler handler;
    void*   data;
}Job;

void job_init(void);
void job_uninit(void);

JobId job_schedule(JobGroupId group, INT16U interval, JobHandler handler, void* data);
JobId job_schedule_ex(JobGroupId group, Job* job);
void job_cancel(JobGroupId group, JobId job);

void job_heartbeat(JobGroupId group);


#endif /* BMS_JOB_H_ */

