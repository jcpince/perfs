#include <perfs/timestamper.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define FILE_OPERATIONS     5000000
#define MAX_FILE_SIZE       5000
#define WRITE_CHUNKS_SIZE   100

#define debug_printf printf
//#define debug_printf(...)   do {} while (0)

int main(int argc, const char **argv)
{
    perfs_ts_t timestamper;
    int rc = perfs_ts_init(&timestamper);
    if (rc)
    {
        fprintf(stderr, "Cannot intialize the timestamper library -- %s\n",
            strerror(rc));
        return -1;
    }
    perfs_ts_event_t *evt0_0 = perfs_add_ts_event(&timestamper, "memset_start",
        PERFS_TS_EVENT_QUICK, PERFS_TS_BUFFER_CIRCULAR, 1000);
    perfs_ts_event_t *evt0_1 = perfs_add_ts_event(&timestamper, "memset_end",
        PERFS_TS_EVENT_QUICK, PERFS_TS_BUFFER_CIRCULAR, 1000);
    perfs_ts_event_t *evt1 = perfs_add_ts_event(&timestamper, "file_op",
        PERFS_TS_EVENT_REGULAR, PERFS_TS_BUFFER_CIRCULAR, 5000);
    perfs_ts_event_t *evt2_0 = perfs_add_ts_event(&timestamper, "fwrite_start",
        PERFS_TS_EVENT_QUICK, PERFS_TS_BUFFER_REGULAR, 2000);
    perfs_ts_event_t *evt2_1 = perfs_add_ts_event(&timestamper, "fwrite_end",
        PERFS_TS_EVENT_QUICK, PERFS_TS_BUFFER_REGULAR, 2000);

    /* Write some stuff in the filesystem */
    FILE *f = fopen("/tmp/perfs_timestamper.bin", "w");
    if (!f)
    {
        fprintf(stderr, "Cannot open the test file -- %s\n", strerror(errno));
        goto deinit;
    }

    /* Main working loop */
    uint32_t current_idx = 0;
    uint8_t buffer[WRITE_CHUNKS_SIZE];
    for ( int idx = 0 ; idx < FILE_OPERATIONS ; idx++ )
    {
        perfs_record_ts(evt1);
        perfs_record_ts(evt0_0);
        memset(buffer, (uint8_t)idx, WRITE_CHUNKS_SIZE);
        perfs_record_ts(evt0_1);
        perfs_record_ts(evt2_0);
        fwrite(buffer, 1, WRITE_CHUNKS_SIZE, f);
        perfs_record_ts(evt2_1);
        current_idx += WRITE_CHUNKS_SIZE;
        if (current_idx >= MAX_FILE_SIZE)
        {
            current_idx = 0;
            fseek(f, 0, SEEK_SET);
        }
    }

    rc = perfs_save_ts(&timestamper, "perfs.csv");
    if (rc)
        fprintf(stderr, "Cannot save the timestamps -- %s\n", strerror(errno));

deinit:
    rc = perfs_ts_deinit(&timestamper);
    if (rc)
    {
        fprintf(stderr, "Cannot deintialize the timestamper library -- %s\n",
            strerror(rc));
        return rc;
    }
    fprintf(stdout, "That's all folks!!!\n");
    return rc;
}
