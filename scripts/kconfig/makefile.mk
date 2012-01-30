KCONFIG_SOURCE_PATH := $(SCRIPT_PATH)/kconfig


KCONFIG_APPS        := $(OUTPUT_PATH)/conf $(OUTPUT_PATH)/nconf


KCONFIG_AUTOHEADER  := $(OUTPUT_PATH)/config.h
KCONFIG_AUTOCONFIG  := $(OUTPUT_PATH)/config.mk
KCONFIG_CONFIG      := .config
KCONFIG_OK          := $(OUTPUT_PATH)/config.ok

export KCONFIG_AUTOHEADER
export KCONFIG_AUTOCONFIG
export KCONFIG_CONFIG


KCONFIG_DATA  := $(SCRIPT_PATH)/kconfig.data


CONF_SRCS     := conf.c zconf.tab.c
NCONF_SRCS    := nconf.c zconf.tab.c nconf.gui.c
KCONFIG_SRCS  := $(sort $(CONF_SRCS) $(NCONF_SRCS))

KCONFIG_DEPS  := $(subst .c,.d,$(foreach SRC, $(KCONFIG_SRCS), $(KCONFIG_SOURCE_PATH)/$(SRC)))

CONF_OBJS     := $(foreach OBJ, $(CONF_SRCS:.c=.o), $(KCONFIG_SOURCE_PATH)/$(OBJ))
NCONF_OBJS    := $(foreach OBJ, $(NCONF_SRCS:.c=.o), $(KCONFIG_SOURCE_PATH)/$(OBJ))
KCONFIG_OBJS  := $(foreach OBJ, $(KCONFIG_SRCS:.c=.o), $(KCONFIG_SOURCE_PATH)/$(OBJ))


NCONF_LDFLAGS := -lmenu -lpanel


CONFIG_GOALS  += $(KCONFIG_APPS) $(KCONFIG_AUTOHEADER) $(KCONFIG_AUTOCONFIG)


ifeq ($(filter $(CLEAN_GOALS) $(HELP_GOALS),$(MAKECMDGOALS)),)

  # Include automatically generated dependency files.
  -include $(KCONFIG_DEPS)

endif # ifeq ($(filter $(NO_BUILD_GOALS),$(MAKECMDGOALS)),)


.PHONY: defconfig silentoldconfig

defconfig: $(OUTPUT_PATH)/conf
	$< $(KCONFIG_DATA) --$@

$(KCONFIG_OK): $(KCONFIG_CONFIG)
	@if [ ! -f $(KCONFIG_AUTOHEADER) -o $(KCONFIG_CONFIG) -nt $(KCONFIG_AUTOHEADER) ]; then \
	  echo 'MAKE     silentoldconfig'; \
	  $(MAKE) silentoldconfig; \
	  touch $@; \
	fi

silentoldconfig $(KCONFIG_AUTOHEADER) $(KCONFIG_AUTOCONFIG): $(OUTPUT_PATH)/conf $(KCONFIG_DATA) $(KCONFIG_CONFIG)
ifneq ($(VERBOSE),)
	@echo 'GEN      $(KCONFIG_AUTOHEADER) $(KCONFIG_AUTOCONFIG)'
endif
	@$< $(KCONFIG_DATA) --silentoldconfig

$(OUTPUT_PATH)/conf: $(CONF_OBJS)
	@echo 'HOSTLD   $@'
	@mkdir -p $(OUTPUT_PATH)
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTCPPFLAGS) -o $@ $^


.PHONY: config menuconfig nconfig clean_kconfig

config: nconfig

menuconfig: nconfig

nconfig: $(OUTPUT_PATH)/nconf
	@$< $(KCONFIG_DATA)

$(OUTPUT_PATH)/nconf: $(NCONF_OBJS)
	@echo 'HOSTLD   $@'
	@mkdir -p $(OUTPUT_PATH)
	@$(HOSTCC) $(HOSTCFLAGS) $(HOSTCPPFLAGS) -o $@ $^ $(NCONF_LDFLAGS)


$(KCONFIG_SOURCE_PATH)/%.d: $(KCONFIG_SOURCE_PATH)/%.c
ifneq ($(VERBOSE),)
	@echo 'HOSTDEP  $@'
endif
	@$(HOSTCC) -M $(HOSTCPPFLAGS) -MP -MT $(patsubst %.c,%.o,$<) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm $@.$$$$

$(KCONFIG_SOURCE_PATH)/%.o: $(KCONFIG_SOURCE_PATH)/%.c
	@echo 'HOSTCC   $@'
	@$(HOSTCC) $(HOSTCFLAGS) $(HOSTCPPFLAGS) -c -o $@ $<


clean_kconfig:
	@find $(KCONFIG_SOURCE_PATH) -name '*.[do]' -exec rm {} +
	@rm $(KCONFIG_APPS) $(KCONFIG_AUTOCONFIG) $(KCONFIG_AUTOHEADER) $(KCONFIG_OK) 2> /dev/null || :

