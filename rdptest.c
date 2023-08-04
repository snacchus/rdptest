#include <libdragon.h>

#define SAMPLE_COUNT 64

void collect_clock_samples(uint32_t *samples, size_t sample_count)
{
    for (size_t i = 0; i < sample_count; i++)
    {
        MEMORY_BARRIER();
        samples[i] = *DP_CLOCK;
        MEMORY_BARRIER();
    }
}

void print_samples(uint32_t *samples, size_t sample_count)
{
    for (size_t i = 0; i < sample_count; i++)
    {
        printf("%#10lx\n", samples[i]);
    }
}

static volatile int dp_interrupt_happened = 0;
void dp_interrupt(void)
{
    dp_interrupt_happened = 1;
}

int main(void)
{
    uint32_t startup_samples[SAMPLE_COUNT] = {0};
    uint32_t reset_clock_samples[SAMPLE_COUNT] = {0};
    uint32_t freeze_samples[SAMPLE_COUNT] = {0};
    uint32_t flush_samples[SAMPLE_COUNT] = {0};
    uint32_t write_clock_samples[SAMPLE_COUNT] = {0};
    uint32_t full_sync_samples[SAMPLE_COUNT] = {0};

    // Test case 1: Is DP_CLOCK running at startup?
    collect_clock_samples(startup_samples, SAMPLE_COUNT);

    // Test case 2: Does DP_CLOCK get reset to 0 after writing 1<<9 to DP_STATUS?
    MEMORY_BARRIER();
    *DP_STATUS = DP_WSTATUS_RESET_CLOCK_COUNTER;
    MEMORY_BARRIER();
    collect_clock_samples(reset_clock_samples, SAMPLE_COUNT);

    // Test case 3: Is DP_CLOCK running after setting FREEZE?
    MEMORY_BARRIER();
    *DP_STATUS = DP_WSTATUS_SET_FREEZE;
    MEMORY_BARRIER();
    collect_clock_samples(freeze_samples, SAMPLE_COUNT);
    MEMORY_BARRIER();
    *DP_STATUS = DP_WSTATUS_RESET_FREEZE;
    MEMORY_BARRIER();

    // Test case 4: Is DP_CLOCK running after setting FLUSH?
    MEMORY_BARRIER();
    *DP_STATUS = DP_WSTATUS_SET_FLUSH;
    MEMORY_BARRIER();
    collect_clock_samples(flush_samples, SAMPLE_COUNT);
    MEMORY_BARRIER();
    *DP_STATUS = DP_WSTATUS_RESET_FLUSH;
    MEMORY_BARRIER();

    // Test case 5: Is DP_CLOCK writable from CPU?
    MEMORY_BARRIER();
    *DP_CLOCK = 0;
    MEMORY_BARRIER();
    collect_clock_samples(write_clock_samples, SAMPLE_COUNT);

    // Test case 6: Is DP_CLOCK running after running SYNC_FULL?
    static uint64_t sync_full_buffer = (uint64_t)RDPQ_CMD_SYNC_FULL << 56;

    set_DP_interrupt(1);
    register_DP_handler(dp_interrupt);

    MEMORY_BARRIER();
    *DP_STATUS = DP_WSTATUS_RESET_XBUS_DMEM_DMA;
    MEMORY_BARRIER();
    *DP_START = PhysicalAddr(&sync_full_buffer);
    MEMORY_BARRIER();
    *DP_END = PhysicalAddr(&sync_full_buffer + 1);
    MEMORY_BARRIER();
    while(dp_interrupt_happened == 0);
    collect_clock_samples(full_sync_samples, SAMPLE_COUNT);

    // Print out results

    debug_init_usblog();
    debug_init_isviewer();

    console_init();
    console_set_debug(true);

    printf("Test case 1: Is DP_CLOCK running at startup?\n");
    print_samples(startup_samples, SAMPLE_COUNT);
    
    printf("Test case 2: Does DP_CLOCK get reset to 0 after writing 1<<9 to DP_STATUS?\n");
    print_samples(reset_clock_samples, SAMPLE_COUNT);

    printf("Test case 3: Is DP_CLOCK running after setting FREEZE?\n");
    print_samples(freeze_samples, SAMPLE_COUNT);

    printf("Test case 4: Is DP_CLOCK running after setting FLUSH?\n");
    print_samples(flush_samples, SAMPLE_COUNT);

    printf("Test case 5: Is DP_CLOCK writable from CPU?\n");
    print_samples(write_clock_samples, SAMPLE_COUNT);

    printf("Test case 6: Is DP_CLOCK running after running SYNC_FULL?\n");
    print_samples(full_sync_samples, SAMPLE_COUNT);
}