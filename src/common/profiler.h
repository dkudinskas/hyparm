#define PROFILER_ENTRY_NUM  10000

void profilerReset(void);
void profilerInit(void);
void profilerRecord(u32int address);
void profilerPause(void);
void profilerResume(void);

struct Profiler {
  u32int entries[PROFILER_ENTRY_NUM];
  u32int writeIndex;
  u32int readIndex;
  u32int counter;
  bool   enabled;
  bool   bufferFull;
};

extern struct Profiler profiler;
