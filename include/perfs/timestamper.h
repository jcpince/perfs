#ifndef __TIMESTAMPER__
#define __TIMESTAMPER__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef enum
{
    PERFS_TS_EVENT_REGULAR,
    PERFS_TS_EVENT_QUICK,       /* TSC based: time granularity below 0.1 ms */
} perfs_ts_event_type_t;

typedef enum
{
    PERFS_TS_BUFFER_REGULAR,
    PERFS_TS_BUFFER_CIRCULAR,
} perfs_ts_buffer_type_t;

typedef struct perfs_ts_event_
{
    const char              *name;
    struct perfs_ts_event_  *next;
    uint64_t                *timestamps;
    uint32_t                current_idx;
    uint32_t                depth;
    perfs_ts_buffer_type_t  buffer_type:8;
    perfs_ts_event_type_t   type:8;
    bool                    full;
    bool                    filled_once;
} perfs_ts_event_t;

typedef struct
{
    perfs_ts_event_t        *events;
} perfs_ts_t;

int                 perfs_ts_init(perfs_ts_t *ts);
int                 perfs_ts_deinit(perfs_ts_t *ts);
perfs_ts_event_t    *perfs_add_ts_event(perfs_ts_t *ts, const char *name,
    perfs_ts_event_type_t type, perfs_ts_buffer_type_t buffer_type, uint32_t depth);
void                perfs_record_ts(perfs_ts_event_t *tse);
int                 perfs_save_ts(perfs_ts_t *ts, const char *filename);
int                 perfs_dump_ts(perfs_ts_t *ts, FILE *f, const char *prefix);

#ifdef __cplusplus
}
#endif

#endif /* __TIMESTAMPER__ */
