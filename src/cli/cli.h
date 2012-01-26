#ifndef __CLI__CLI_H__
#define __CLI__CLI_H__ 1


#define CLI_COMMAND_HANDLER(name)  void name(int argc, const char *const *argv)


void enterCliLoop(void)
#ifndef TEST_CLI
  __attribute__((noreturn))
#endif
  ;

#endif
