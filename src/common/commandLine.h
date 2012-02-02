#ifndef __COMMON__COMMAND_LINE_H__
#define __COMMON__COMMAND_LINE_H__

#include "common/types.h"


struct commandLineOption
{
  const char *name;
  const char *description;
  struct
  {
    unsigned hasArgument : 1;
    unsigned isRequired : 1;
    unsigned : 14;
    unsigned id : 16;
  } options;
  struct commandLineOption *next;
};

struct commandLine
{
  u16int argumentId;
  const char *value;
  struct commandLine *next;
};


struct commandLineOption *addCommandLineOption(struct commandLineOption *options, const char *name,
    const char *description, bool hasArgument, bool isRequired, u16int id);

void freeCommandLine(struct commandLine *commandLine);
void freeCommandLineOptions(struct commandLineOption *options);

struct commandLine *parseCommandLine(struct commandLineOption *options, s32int argc, char *argv[]);

void printCommandLineHelp(struct commandLineOption *options);

#endif /* __COMMON__COMMAND_LINE_H__ */
