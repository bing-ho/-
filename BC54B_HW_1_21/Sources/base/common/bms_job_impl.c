/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_job_impl.c
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-6-29
*
*/
#include "bms_job_impl.h"
#include "bms_system.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_JOB
JobContext g_jobs_context[BMS_MAX_JOB_NUM] = {NULL, NULL};
JobGroupContext g_job_groups_context[BMS_MAX_JOB_GROUP_NUM] = {0};
#pragma DATA_SEG DEFAULT

void job_init(void)
{
}

void job_uninit(void)
{
}

JobId job_schedule(JobGroupId group, INT16U interval, JobHandler handler, void* data)
{
  Job job;
  memset(&job, 0, sizeof(job));
  job.data = data;
  job.interval = interval;
  job.handler = handler;
  return job_schedule_ex(group, &job);
}

JobId job_schedule_ex(JobGroupId group, Job* job)
{
    JobId tail, allocated;
    if (job == NULL || job->handler == NULL) return 0;
    if (group >= BMS_MAX_JOB_GROUP_NUM) return 0;

    allocated = job_find_availabled();//find available job context
    if (allocated == 0) return 0;

    g_jobs_context[allocated].handler = job->handler;
    g_jobs_context[allocated].user_data = job->data;
    g_jobs_context[allocated].interval = job->interval;
    g_jobs_context[allocated].last_tick = 0;
    g_jobs_context[allocated].next = 0;

    tail = job_find_tail(group);
    if (tail == 0)//group is empty
    {
        g_job_groups_context[group].next_id = allocated;
    }
    else
    {
        g_jobs_context[tail].next = allocated;
    }

    ++g_job_groups_context[group].count;//increase the job count of the group

    return allocated;

}

void job_cancel(JobGroupId group, JobId job)
{
    JobId prev_id;
    if (job == 0 || job >= BMS_MAX_JOB_NUM) return;
    if (group >= BMS_MAX_JOB_GROUP_NUM) return;

    g_jobs_context[job].handler = NULL;
    g_jobs_context[job].user_data = NULL;

    if (g_job_groups_context[group].next_id == job)//cancel job is the first job of the group
    {
        g_job_groups_context[group].next_id = g_jobs_context[job].next;//relocate the first job of the group
        g_jobs_context[job].next = 0;
        return;
    }

    prev_id = g_job_groups_context[group].next_id;
    while (prev_id > 0)
    {
        if (g_jobs_context[prev_id].next == job)//find the job
        {
            g_jobs_context[prev_id].next = g_jobs_context[job].next;//relocate previous job's next to the cancel job's next
            g_jobs_context[job].next = 0;//remove the job from the group
            return;
        }
        prev_id = g_jobs_context[prev_id].next;
    }

}

void job_heartbeat(JobGroupId group)
{
    JobId index;
    INT32U tick;

    if (group >= BMS_MAX_JOB_GROUP_NUM) return;

    index = g_job_groups_context[group].next_id;
    tick = get_tick_count();

    while (index > 0)
    {
        if (get_interval_by_tick(g_jobs_context[index].last_tick, tick) >= g_jobs_context[index].interval)
        {
            g_jobs_context[index].handler(g_jobs_context[index].user_data);
            g_jobs_context[index].last_tick = tick;
        }

        index = g_jobs_context[index].next;
    }
}

JobId job_find_tail(JobGroupId group)
{
    JobId start_id = g_job_groups_context[group].next_id;
    while (start_id > 0)
    {
        if (g_jobs_context[start_id].next == 0)
            break;
        start_id = g_jobs_context[start_id].next;
    }
    return start_id;
}

JobId job_find_availabled(void)
{
    INT16U index;

    for (index = 1; index < BMS_MAX_JOB_NUM; ++index)
    {
        if (g_jobs_context[index].handler == NULL)
            return index;
    }

    return 0;
}
