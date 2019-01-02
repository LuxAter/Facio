
SHELL=/bin/bash
ifndef .VERBOSE
    .SILENT:
endif
ROOT=$(shell pwd)
root=/home/arden/Programming/bash/facio
CXX=clang++
CXXIGNORE=
CXXFLAGS=-stc=c++17 -fPIC -Wall -Wpedantic static
link=-lz
source=source
include_dir=include
include=-I$(ROOT)/$(EXTERNAL)/estl -I$(ROOT)/$(BUILD)/libpng/include -I$(ROOT)/$(BUILD)/libjpeg/include
build=build
external=external
test=test
COMMON_INCLUDE=-I$(ROOT)/$(INCLUDE_DIR) $(INCLUDE)

SCAN_COLOR=\033[1;35m
BUILD_COLOR=\033[32m
CLEAN_COLOR=\033[1;33m
LINK_COLOR=\033[1;32m
INSTALL_COLOR=\033[1;36m
CMD_COLOR=\033[1;34m
HELP_COLOR=\033[1;34m

define scan_target
printf "%b%s%b\n" "$(SCAN_COLOR)" "Scaning dependencies for target $(1)" "\033[0m"
endef
define complete_target
printf "%s\n" "Built target $(1)"
endef
define clean_target
printf "%b%s%b\n" "$(CLEAN_COLOR)" "Cleaning target $(1)" "\033[0m"
endef
define install_target
printf "%b%s%b\n" "$(INSTALL_COLOR)" "Installing target $(1)" "\033[0m"
endef
define uninstall_target
printf "%b%s%b\n" "$(INSTALL_COLOR)" "Unnstalling target $(1)" "\033[0m"
endef
define print_build_c
str=$$(realpath --relative-to="$(ROOT)" "$(1)");    printf "%b%s%b\n" "$(BUILD_COLOR)" "Building C object $$str" "\033[0m"
endef
define print_build_cpp
str=$$(realpath --relative-to="$(ROOT)" "$(1)");    printf "%b%s%b\n" "$(BUILD_COLOR)" "Building C++ object $$str" "\033[0m"
endef
define print_link_lib
str=$$(realpath --relative-to="$(ROOT)" "$(1)");    printf "%b%s%b\n" "$(LINK_COLOR)" "Linking static library $$str" "\033[0m"
endef
define print_link_exe
str=$$(realpath --relative-to="$(ROOT)" "$(1)");    printf "%b%s%b\n" "$(LINK_COLOR)" "Linking executable $$str" "\033[0m"
endef
define print_run_cmd
printf "%b%s%b\n" "$(CMD_COLOR)" "Running '$(1)'" "\033[0m"
endef
define help
printf "%b%*s%b: %s\n" "$(HELP_COLOR)" 20 "$(1)" "\033[0m" "$(2)"
endef


# SIGNI {{{

SIGNI=/home/arden/Programming/bash/facio/signi
SIGNI_FILES=source/main.cpp
SIGNI_OBJS=$(SIGNI_FILES:%=$(ROOT)/$(BUILD_DIR)/%.o)
-include $(SIGNI_OBJS:.o=.d)

build-signi: build-libsigni.a pre-signi $(SIGNI)
	$(call complete_target,$(shell basename $(SIGNI)))

clean-signi:
	$(call clean_target,$(shell basename $(SIGNI)))
	if [ -e "$(SIGNI)" ]; then rm $(SIGNI); fi

pre-signi:
	$(call scan_target,$(shell basename $(SIGNI)))

$(SIGNI): $(SIGNI_OBJS)
	$(call print_link_exe,$(shell basename $(SIGNI)))
	$(CXX) $(SIGNI_OBJS) $(LIBSIGNI.A) $(LINK) $(COMMON_INCLUDE) -o $(SIGNI)

install-signi: build-signi
	$(call install_target,$(shell basename $(SIGNI)))
	mkdir -p $(INSTALL_PATH)/bin/
	cp $(SIGNI) $(INSTALL_PATH)/bin

uninstall-signi:
	$(call uninstall_target,$(shell basename $(SIGNI)))
	if [ -e "$(INSTALL_PATH)/bin/$(shell basename $(SIGNI))" ]; then rm $(INSTALL_PATH)/bin/$(shell basename $(SIGNI)); fi

# }}}
# LIBSIGNI.A {{{

LIBSIGNI.A=build/libsigni.a
LIBSIGNI.A_FILES=$(filter-out source/main.cpp, $(shell find "source/" -name "*.cpp"))
LIBSIGNI.A_OBJS=$(LIBSIGNI.A_FILES:%=$(ROOT)/$(BUILD_DIR)/%.o)
-include $(LIBSIGNI.A_OBJS:.o=.d)

build-libsigni.a: build-libpng build-libjpeg pre-libsigni.a $(LIBSIGNI.A)
	$(call complete_target,$(shell basename $(LIBSIGNI.A)))

clean-libsigni.a: clean-libpng clean-libjpeg
	$(call clean_target,$(shell basename $(LIBSIGNI.A)))
	if [ -e "$(LIBSIGNI.A)" ]; then rm $(LIBSIGNI.A); fi

pre-libsigni.a:
	$(call scan_target,$(shell basename $(LIBSIGNI.A)))

$(LIBSIGNI.A): $(LIBSIGNI.A_OBJS)
	$(call print_link_lib,$(shell basename $(LIBSIGNI.A)))
	ar rcs $@ $(LIBSIGNI.A_OBJS)
	mkdir -p $(ROOT)/tmp/libpng && cd $(ROOT)/tmp/libpng && ar x build/libpng/lib/build/libpng && ar qc $@ $(ROOT)/tmp/libpng/*.o && rm -rf $(ROOT)/tmp/libpng
	mkdir -p $(ROOT)/tmp/libjpeg && cd $(ROOT)/tmp/libjpeg && ar x build/libjpeg/lib/build/libjpeg && ar qc $@ $(ROOT)/tmp/libjpeg/*.o && rm -rf $(ROOT)/tmp/libjpeg

install-libsigni.a: build-libsigni.a
	$(call install_target,$(shell basename $(LIBSIGNI.A)))
	mkdir -p $(INSTALL_PATH)/lib/
	mkdir -p $(INSTALL_PATH)/include/$(NAME)/
	cp $(LIBSIGNI.A) $(INSTALL_PATH)/lib
	if [ ! -z "$(INCLUDE_DIR)" ]; then cp -R $(INCLUDE_DIR)/ $(INSTALL_PATH)/include/$(NAME)/; fi
	if [ ! -z "$(shell find $(SOURCE_DIR) -name "*.h")" ]; then cd $(SOURCE_DIR) && cp --parents $(sehll cd $(SOURCE_DIR) && find . -name "*.h") $(INSTALL_PATH)/include/$(NAME); fi
	if [ ! -z "$(shell find $(SOURCE_DIR) -name "*.hpp")" ]; then cd $(SOURCE_DIR) && cp --parents $(sehll cd $(SOURCE_DIR) && find . -name "*.hpp") $(INSTALL_PATH)/include/$(NAME); fi

uninstall-libsigni.a:
	$(call uninstall_target,$(shell basename $(LIBSIGNI.A)))
	if [ ! -e "$(INSTALL_PATH)/lib/$(shell basename $(LIBSIGNI.A))" ]; then rm $(INSTALL_PATH)/lib/$(shell basename $(LIBSIGNI.A)); fi
	if [ ! -e "$(INSTALL_PATH)/include/$(NAME)" ]; then rm $(INSTALL_PATH)/include/$(NAME) -r; fi

# }}}
# LIBPNG {{{

build-libpng: pre-libpng
	if [ ! -f "external/libpng/configure" ]; then $(call print_run_cmd,autogen.sh) && cd external/libpng && ./autogen.sh; fi
	if [ ! -f "external/libpng/Makefile" ]; then $(call print_run_cmd,configure) && cd external/libpng && ./configure --prefix=build/libpng; fi
	if [ ! -d "build/libpng" ]; then $(call print_run_cmd,make) && cd external/libpng && make install; fi
	$(call complete_target,libpng)

clean-libpng:
	$(call clean_target,libpng)
	if [ -e "external/libpng/Makefile" ]; then cd external/libpng && make clean && rm Makefile; fi
	if [ -d "build/libpng" ]; then rm build/libpng -r; fi

pre-libpng:
	$(call scan_target,libpng)

# }}}
# LIBJPEG {{{

build-libjpeg: pre-libjpeg
	if [ ! -f "external/jpeg-8d/configure" ]; then $(call print_run_cmd,autogen.sh) && cd external/jpeg-8d && ./autogen.sh; fi
	if [ ! -f "external/jpeg-8d/Makefile" ]; then $(call print_run_cmd,configure) && cd external/jpeg-8d && ./configure --prefix=build/libjpeg; fi
	if [ ! -d "build/libjpeg" ]; then $(call print_run_cmd,make) && cd external/jpeg-8d && make install; fi
	$(call complete_target,libjpeg)

clean-libjpeg:
	$(call clean_target,libjpeg)
	if [ -e "external/jpeg-8d/Makefile" ]; then cd external/jpeg-8d && make clean && rm Makefile; fi
	if [ -d "build/libjpeg" ]; then rm build/libjpeg -r; fi

pre-libjpeg:
	$(call scan_target,libjpeg)

# }}}

$(ROOT)/$(BUILD_DIR)/%.cpp.o: %.cpp
	$(call print_build_cpp,$@)
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MMD -c $(COMMON_INCLUDE) $< -o $@
