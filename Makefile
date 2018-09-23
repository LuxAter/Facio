SHELL=/bin/bash

ifndef .VERBOSE
  .SILENT:
endif

ROOT=$(shell pwd)

C=clang
CFLAGS=
CLINK=-lm -lxcb
INSTALL_PATH=/usr/local
BUILD_DIR=build
SOURCE_DIR=src
INCLUDE_DIR=include

SCAN_COLOR=\033[1;35m
BUILD_COLOR=\033[32m
CLEAN_COLOR=\033[1;33m
LINK_COLOR=\033[1;32m
INSTALL_COLOR=\033[01;36m
HELP_COLOR=\033[1;34m

define scan_target
printf "%b%s%b\n" "$(SCAN_COLOR)" "Scaning dependencies for target $(1)" "\033[0m"
endef
define complete_target
printf "%s\n" "Built target $(1)" endef define clean_target
printf "%b%s%b\n" "$(CLEAN_COLOR)" "Cleaning target $(1)" "\033[0m"
endef
define install_target
printf "%b%s%b\n" "$(INSTALL_COLOR)" "Installing target $(1)" "\033[0m"
endef
define uninstall_target
printf "%b%s%b\n" "$(INSTALL_COLOR)" "Installing target $(1)" "\033[0m"
endef
define print_build_c
str=$$(realpath --relative-to="$(ROOT)" "$(1)");\
printf "%b%s%b\n" "$(BUILD_COLOR)" "Building C object $$str" "\033[0m"
endef
define print_build_cpp
str=$$(realpath --relative-to="$(ROOT)" "$(1)");\
printf "%b%s%b\n" "$(BUILD_COLOR)" "Building Cpp object $$str" "\033[0m"
endef
define print_link_lib_c
str=$$(realpath --relative-to="$(ROOT)" "$(1)");\
printf "%b%s%b\n" "$(LINK_COLOR)" "Linking C static library $$str" "\033[0m"
endef
define print_link_lib_cpp
str=$$(realpath --relative-to="$(ROOT)" "$(1)");\
printf "%b%s%b\n" "$(LINK_COLOR)" "Linking Cpp static library $$str" "\033[0m"
endef
define print_link_exe_c
str=$$(realpath --relative-to="$(ROOT)" "$(1)");\
printf "%b%s%b\n" "$(LINK_COLOR)" "Linking C executable $$str" "\033[0m"
endef
define print_link_exe_cpp
str=$$(realpath --relative-to="$(ROOT)" "$(1)");\
printf "%b%s%b\n" "$(LINK_COLOR)" "Linking Cpp executable $$str" "\033[0m"
endef
define help
printf "%b%*s%b: %s\n" "$(HELP_COLOR)" 20 "$(1)" "\033[0m" "$(2)"
endef

all: build-entis 

clean: clean-libentis.a clean-entis

install: install-libentis.a

uninstall: uninstall-libentis.a

# ENTIS {{{
ENTIS_FILES=src/main.c
ENTIS_OBJS=$(ENTIS_FILES:%=$(ROOT)/build/entis/%.o)
-include $(ENTIS_OBJS:%.o=%.d)
build-entis: build-libentis.a pre_entis ./entis
	$(call complete_target,entis)

./entis: $(ENTIS_OBJS)
	$(call print_link_exe_c,$@)
	$(C) $^ $(ROOT)/build/libentis.a/libentis.a $(CLINK) -o $@

pre_entis:
	$(call scan_target,entis)
	mkdir -p $(ROOT)/build/entis

clean-entis:
	$(call clean_target,entis)
	if [ -e "entis" ]; then rm entis; fi
	if [ -e "$(ROOT)/build/entis" ]; then rm $(ROOT)/build/entis -r; fi

install-entis: build-entis 
	$(call install_target,entis)
	mkdir -p $(INSTALL_PATH)/bin/
	cp entis $(INSTALL_PATH)/bin/

uninstall-entis:
	$(call uninstall_target,entis)
	if [ -e "$(INSTALL_PATH)/bin/entis" ]; then rm $(INSTALL_PATH)/bin/entis; fi

$(ROOT)/build/entis/%.c.o: %.c
	mkdir -p $(@D)
	$(call print_build_c,$@)
	$(C) $(CFLAGS) -MMD -c $(INCLUDE) $^ -o $@

$(ROOT)/build/entis/%.cpp.o: %.cpp
	mkdir -p $(@D)
	$(call print_build_cpp,$@)
	$(CXX) $(CXXFLAGS) -MMD -c $(INCLUDE) $^ -o $@
# }}}
# LIBENTIS.A {{{
LIBENTIS.A_FILES=$(filter-out src/main.c, $(shell find "src/" -name "*.c"))
LIBENTIS.A_OBJS=$(LIBENTIS.A_FILES:%=$(ROOT)/build/libentis.a/%.o)
LIBENTIS.A_HEADER_FILES=$(shell find "src/" -name "*.h")
LIBENTIS.A_HEADER_BASE=$(LIBENTIS.A_HEADER_FILES:src/%=%)
LIBENTIS.A_HEADERS=$(subst $(SPACE),$(COMMA),$(strip $(LIBENTIS.A_HEADER_BASE)))
-include $(LIBENTIS.A_OBJS:%.o=%.d)
build-libentis.a: pre_libentis.a $(ROOT)/build/libentis.a/libentis.a
	$(call complete_target,libentis.a)

$(ROOT)/build/libentis.a/libentis.a: $(LIBENTIS.A_OBJS)
        $(call print_link_lib_c,$@)
	ar rsc $@ $^

pre_libentis.a:
	$(call scan_target,libentis.a)
	mkdir -p $(ROOT)/build/libentis.a

clean-libentis.a:
	$(call clean_target,libentis.a)
	if [ -e "$(ROOT)/build/libentis.a/libentis.a" ]; then rm $(ROOT)/build/libentis.a/libentis.a; fi
	if [ -e "$(ROOT)/build/libentis.a" ]; then rm $(ROOT)/build/libentis.a -r; fi

install-libentis.a: build-libentis.a
	$(call install_target,libentis.a)
	mkdir -p $(INSTALL_PATH)/lib/
	mkdir -p $(INSTALL_PATH)/include/entis/
	cp $(ROOT)/build/libentis.a/libentis.a $(INSTALL_PATH)/lib/
	cp $(ROOT)/src/{$(LIBENTIS.A_HEADERS)} $(INSTALL_PATH)/include/entis

uninstall-libentis.a:
	$(call uninstall_target,libentis.a)
	if [ -e "$(INSTALL_PATH)/lib/libentis.a" ]; then rm $(INSTALL_PATH)/lib/libentis.a; fi
	if [ -e "$(INSTALL_PATH)/include/entis" ]; then rm $(INSTALL_PATH)/include/entis -r; fi

$(ROOT)/build/libentis.a/%.c.o: %.c
	mkdir -p $(@D)
	$(call print_build_c,$@)
	$(C) $(CFLAGS) -MMD -c $(INCLUDE) $^ -o $@

$(ROOT)/build/libentis.a/%.cpp.o: %.cpp
	mkdir -p $(@D)
	$(call print_build_cpp,$@)
	$(CXX) $(CXXFLAGS) -MMD -c $(INCLUDE) $^ -o $@
# }}}
