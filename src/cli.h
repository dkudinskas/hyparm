#ifndef __CLI_H__
#define __CLI_H__ 1


#ifdef TEST_CLI
void enterCliLoop(void);
#else
void enterCliLoop(void) __attribute__((noreturn));
#endif

#endif
