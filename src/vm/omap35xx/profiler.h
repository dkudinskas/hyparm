#define PROFILER_CONTROL_REG         0x0
#define PROFILER_SAMPLE_SIZE_REG     0x4
#define PROFILER_DATA_REG            0x8

#define PROFILER_CONTROL_REG_RUNNING 0x1
#define PROFILER_CONTROL_REG_RESET   0x2

void initProfilerInt(void);
u32int loadProfilerInt(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeProfilerInt(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);
