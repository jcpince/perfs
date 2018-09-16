#include <perfs/timestamper.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

#define max(a, b) ((a) > (b) ? (a) : (b))

#ifdef __i386
static inline uint64_t read_tsc()
{
    uint64_t tsc;
    __asm__ volatile ("rdtsc" : "=A" (tsc));
    return tsc;
}
#elif defined __amd64
static inline uint64_t read_tsc()
{
    uint64_t tsc_lo, tsc_hi;
    __asm__ volatile ("rdtsc" : "=a" (tsc_lo), "=d" (tsc_hi));
    return ( tsc_hi << 32 ) | tsc_lo;
}
#endif

static inline uint64_t read_time_ms()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint64_t)now.tv_sec * 1000 + (uint64_t)now.tv_nsec / 1000000;
}

int perfs_ts_init(perfs_ts_t *ts)
{
    ts->events = NULL;
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now))
        return errno;
    return 0;
}

int perfs_ts_deinit(perfs_ts_t *ts)
{
    for (perfs_ts_event_t *event = ts->events ; event ; event = event->next)
    {
        if (event->name) free((char *)event->name);
        if (event->timestamps) free(event->timestamps);
        free(event);
    }
}

perfs_ts_event_t *perfs_add_ts_event(perfs_ts_t *ts, const char *name,
    perfs_ts_event_type_t type, perfs_ts_buffer_type_t buffer_type, uint32_t depth)
{
    int rc;
    perfs_ts_event_t *new_event = calloc(1, sizeof(perfs_ts_event_t));
    rc = errno;
    if (!new_event) goto cleanup;

    new_event->name = strdup(name);
    rc = errno;
    if (!new_event->name) goto cleanup;
    new_event->timestamps = calloc(depth, sizeof(uint64_t));
    rc = errno;
    if (!new_event->timestamps) goto cleanup;

    new_event->buffer_type = buffer_type;
    new_event->type = type;
    new_event->depth = depth;
    new_event->next = ts->events;
    new_event->current_idx = 0;
    new_event->full = false;
    new_event->filled_once = false;
    ts->events = new_event;
    return new_event;
cleanup:
    if (new_event)
    {
        if (new_event->name) free((char *)new_event->name);
        if (new_event->timestamps) free(new_event->timestamps);
        free(new_event);
    }
    errno = rc;
    return NULL;
}

void perfs_record_ts(perfs_ts_event_t *tse)
{
    if (tse->full) return;
    uint64_t now;
    if (tse->type == PERFS_TS_EVENT_QUICK)
        now = read_tsc();
    else
        now = read_time_ms();
    if (!now)
    {
        printf("%s just got now == 0!!!\n", tse->name);
    }
    tse->timestamps[tse->current_idx++] = now;
    if (tse->current_idx >= tse->depth)
    {
        if (tse->buffer_type == PERFS_TS_BUFFER_CIRCULAR)
        {
            tse->current_idx = 0;
            tse->filled_once = true;
        }
        else
            tse->full = true;
    }
}

int perfs_save_ts(perfs_ts_t *ts, const char *filename)
{
    /* First, print the header */
    FILE *f = fopen(filename, "w");
    if (!f)
        return errno;

    uint32_t max_depth = 0;
    /* print the header, and get the max depth */
    for (perfs_ts_event_t *event = ts->events ; event ; event = event->next)
    {
        max_depth = max(max_depth, event->depth);
        fprintf(f, "%s_%s_%s, ",
            (event->buffer_type == PERFS_TS_BUFFER_REGULAR ? "first" : "last"),
            event->name,
            (event->type == PERFS_TS_EVENT_QUICK ? "core_ticks" : "ms"));
    }
    fprintf(f, "\n");

    /* Write the events in the event's chronological order (not global!) */
    for (int idx = 0 ; idx < max_depth ; idx++)
    {
        for (perfs_ts_event_t *event = ts->events ; event ; event = event->next)
        {
            uint32_t evt_idx = idx;
            if (event->filled_once)
                evt_idx = (event->current_idx + idx) % event->depth;
            if (idx < event->depth)
                fprintf(f, "%ld, ", event->timestamps[evt_idx]);
            else
                fprintf(f, "nan, ");
        }
        fprintf(f, "\n");
    }

    fclose(f);
    return 0;
}
