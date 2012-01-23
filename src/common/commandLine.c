#include "common/commandLine.h"
#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"


static struct commandLine *createCommandLine(void);
static struct commandLineOption *createCommandLineOption(void);
static struct commandLineOption *findCommandLineOption(struct commandLineOption *options,
    const char *name);


struct commandLineOption *addCommandLineOption(struct commandLineOption *options, const char *name,
    const char *description, bool hasArgument, bool isRequired, u16int id)
{
  struct commandLineOption *p = options;
  if (p)
  {
    while (p->next)
    {
      p = p->next;
    }
    p = p->next = createCommandLineOption();
  }
  else
  {
    options = p = createCommandLineOption();
  }
  p->name = name;
  p->description = description;
  p->options.hasArgument = !!(hasArgument);
  p->options.isRequired = !!(isRequired);
  p->options.id = id;
  return options;
}


static struct commandLine *createCommandLine()
{
  struct commandLine *p = malloc(sizeof(struct commandLine));
  if (p == NULL)
  {
    DIE_NOW(NULL, "malloc failed");
  }
  memset(p, 0, sizeof(struct commandLine));
  return p;
}

static struct commandLineOption *createCommandLineOption()
{
  struct commandLineOption *p = malloc(sizeof(struct commandLineOption));
  if (p == NULL)
  {
    DIE_NOW(NULL, "malloc failed");
  }
  memset(p, 0, sizeof(struct commandLineOption));
  return p;
}

static struct commandLineOption *findCommandLineOption(struct commandLineOption *options,
    const char *name)
{
  while (options && strcmp(name, options->name))
  {
    options = options->next;
  }
  return options;
}

void freeCommandLine(struct commandLine *commandLine)
{
  struct commandLine *next;
  while (commandLine)
  {
    next = commandLine->next;
    free(commandLine);
    commandLine = next;
  }
}

void freeCommandLineOptions(struct commandLineOption *options)
{
  struct commandLineOption *next;
  while (options)
  {
    next = options->next;
    free(options);
    options = next;
  }
}

struct commandLine *parseCommandLine(struct commandLineOption *options, s32int argc, char *argv[])
{
  struct commandLine *commandLine = NULL, *p = NULL;
  s32int i;
  bool readOptArg = FALSE;
  for (i = 0; i < argc; ++i)
  {
    const char *arg = argv[i];
    if (readOptArg)
    {
      /* assign value */
      p->value = arg;
      readOptArg = FALSE;
    }
    else
    {
      struct commandLineOption *thisDef;
      if (p)
      {
        p = p->next = createCommandLine();
      }
      else
      {
        commandLine = p = createCommandLine();
      }
      if (*arg == '-' && (thisDef = findCommandLineOption(options, arg + 1)))
      {
        p->argumentId = thisDef->options.id;
        readOptArg = thisDef->options.hasArgument;
      }
      else
      {
        p->value = arg;
      }
    }
  }
  return commandLine;
}

void printCommandLineHelp(struct commandLineOption *options)
{
  printf("Options:" EOL);
  while (options)
  {
    const char *argument = options->options.hasArgument ? " arg" : "";
    const char *required = options->options.isRequired ? " [required]" : "";
    printf("  -%s%s%s" EOL, options->name, argument, required);
    printf("    %s" EOL, options->description);
    options = options->next;
  }
}
