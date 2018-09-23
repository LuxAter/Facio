#!/bin/bash

MAKEFILE=''

MAKE_FILE_HEADER='''
SHELL=/bin/bash
ifndef .VERBOSE
  .SILENT:
endif
ROOT=$(shell pwd)
SPACE=$(null) #
COMMA=,
SCAN_COLOR=\033[1;35m
BUILD_COLOR=\033[32m
CLEAN_COLOR=\033[1;33m
LINK_COLOR=\033[1;32m
INSTALL_COLOR=\033[01;36m
HELP_COLOR=\033[1;34m

C=clang
CXX=clang++
CFLAG=-Wall
CXXFLAG=-Wall
CLINK=
'''

MAKE_FILE_FUNCTIONS='''
# FUNCTIONS {{{
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
printf "%b%s%b\n" "$(INSTALL_COLOR)" "Installing target $(1)" "\033[0m"
endef
define print_build_c
str=$$(realpath --relative-to="$(ROOT)" "$(1)");\
printf "%b%s%b\n" "$(BUILD_COLOR)" "Building C object $$str" "\033[0m"
endef
define print_build_cxx
str=$$(realpath --relative-to="$(ROOT)" "$(1)");\
printf "%b%s%b\n" "$(BUILD_COLOR)" "Building Cpp object $$str" "\033[0m"
endef
define print_link_lib_c
str=$$(realpath --relative-to="$(ROOT)" "$(1)");\
printf "%b%s%b\n" "$(LINK_COLOR)" "Linking C static library $$str" "\033[0m"
endef
define print_link_lib_cxx
str=$$(realpath --relative-to="$(ROOT)" "$(1)");\
printf "%b%s%b\n" "$(LINK_COLOR)" "Linking Cpp static library $$str" "\033[0m"
endef
define print_link_exe_c
str=$$(realpath --relative-to="$(ROOT)" "$(1)");\
printf "%b%s%b\n" "$(LINK_COLOR)" "Linking C executable $$str" "\033[0m"
endef
define print_link_exe_cxx
str=$$(realpath --relative-to="$(ROOT)" "$(1)");\
printf "%b%s%b\n" "$(LINK_COLOR)" "Linking Cpp executable $$str" "\033[0m"
endef
define help
printf "%b%*s%b: %s\n" "$(HELP_COLOR)" 20 "$(1)" "\033[0m" "$(2)"
endef
# }}}

all:

clean:

install:

uninstall:
'''

MAKE_FILE_EXE='''
# {TARGET} {{{
{TARGET}_FILES={SOURCE}
{TARGET}_OBJS=$({TARGET}_FILES:%=$(ROOT)/{object}/{target}/%.o)
-include $({TARGET}_OBJS:%.o=%.d)
build-{target}: pre_{target} {build}/{target}
	$(call complete_target,{target})

{build}/{target}: $({TARGET}_OBJS)
	$(call print_link_exe_{lang},$@)
	$({LANG}) $^ $({LANG}LINK) -o $@

pre_{target}:
	$(call scan_target,{target})
	mkdir -p $(ROOT)/{object}/{target}

clean-{target}:
	$(call clean_target,{target})
	if [ -e "{build}/{target}" ]; then rm {build}/{target}; fi
	if [ -e "$(ROOT)/{object}/{target}" ]; then rm $(ROOT)/{object}/{target} -r; fi

install-{target}: build-{target}
	$(call install_target,{target})
	mkdir -p $(INSTALL_PATH)/bin/
	cp {build}/{target} $(INSTALL_PATH)/bin/

uninstall-{target}:
	$(call uninstall_target,{target})
	if [ -e "$(INSTALL_PATH)/bin/{target}" ]; then rm $(INSTALL_PATH)/bin/{target}; fi

$(ROOT)/{object}/{target}/%.c.o: %.c
	mkdir -p $(@D)
	$(call print_build_c,$@)
	$(C) $(CFLAGS) -MMD -c $(INCLUDE) $^ -o $@

$(ROOT)/{object}/{target}/%.cpp.o: %.cpp
	mkdir -p $(@D)
	$(call print_build_cpp,$@)
	$(CXX) $(CXXFLAGS) -MMD -c $(INCLUDE) $^ -o $@
# }}}
'''

MAKE_FILE_LIB='''
# {TARGET} {{{
{TARGET}_FILES={SOURCE}
{TARGET}_OBJS=$({TARGET}_FILES:%=$(ROOT)/{object}/{target}/%.o)
{TARGET}_HEADER_FILES=$(shell find "$(INCLUDE)/" -name "*.h*")
{TARGET}_HEADER_BASE=$({TARGET}_HEADER_FILES:$(INCLUDE)/%=%)
{TARGET}_HEADERS=$(subst $(SPACE),$(COMMA),$(strip $({TARGET}_HEADER_BASE)))
-include $({TARGET}_OBJS:%.o=%.d)
build-{target}: pre_{target} $(ROOT)/{build}/{target}/{target}
	$(call complete_target,{target})

$(ROOT)/{build}/{target}/{target}: $({TARGET}_OBJS)
	$(call print_link_lib_c,$@)
	ar rsc $@ $^

pre_{target}:
	$(call scan_target,{target})
	mkdir -p $(ROOT)/{object}

clean-{target}:
	$(call clean_target,{target})
	if [ -e "$(ROOT)/{build}/{target}/{target}" ]; then rm $(ROOT)/{build}/{target}/{target}; fi
	if [ -e "$(ROOT)/{object}/{target}" ]; then rm $(ROOT)/{object}/{target} -r; fi

install-{target}: build-{target}
	$(call install_target,{target})
	mkdir -p $(INSTALL_PATH)/lib/
	mkdir -p $(INSTALL_PATH)/include/{install_dir}/
	cp $(ROOT)/{build}/{target}/{target} $(INSTALL_PATH)/lib/
	cp $(ROOT)/src/{$({TARGET}_HEADERS)} $(INSTALL_PATH)/include/{install_dir}

uninstall-{target}:
	$(call uninstall_target,{target})
	if [ -e "$(INSTALL_PATH)/lib/{target}" ]; then rm $(INSTALL_PATH)/lib/{target}; fi
	if [ -e "$(INSTALL_PATH)/include/{install_dir}" ]; then rm $(INSTALL_PATH)/include/{install_dir}-r; fi

$(ROOT)/{build}/{target}/%.c.o: %.c
	mkdir -p $(@D)
	$(call print_build_c,$@)
	$(C) $(CFLAGS) -MMD -c $(INCLUDE) $^ -o $@

$(ROOT)/{build}/{target}/%.cpp.o: %.cpp
	mkdir -p $(@D)
	$(call print_build_cpp,$@)
	$(CXX) $(CXXFLAGS) -MMD -c $(INCLUDE) $^ -o $@
# }}}
'''

function GenerateExe(){
  target="$1"
  TARGET=${target^^}
  lang="$2"
  llang=${target,,}
  SOURCE="$3"
  build="$4"
  if [[ ! -z "$5" ]]; then
    object="$5"
  else
    object="$4"
  fi
  make_target="${MAKE_FILE_EXE//"{TARGET}"/"$TARGET"}"
  make_target="${make_target//"{target}"/"$target"}"
  make_target="${make_target//"{build}"/"$build"}"
  make_target="${make_target//"{object}"/"$object"}"
  make_target="${make_target//"{SOURCE}"/"$SOURCE"}"
  make_target="${make_target//"{LANG}"/"$lang"}"
  make_target="${make_target//"{lang}"/"$llang"}"
  echo "$make_target"
  MAKEFILE="$MAKEFILE$make_target"
}
function GenerateLib(){
  target="$1"
  TARGET=${target^^}
  lang="$2"
  SOURCE="$3"
  include_dir="$4"
  build="$5"
  if [[ ! -z "$6" ]]; then
    object="$6"
  else
    object="$5"
  fi
  make_target="${MAKE_FILE_LIB//"{TARGET}"/"$TARGET"}"
  make_target="${make_target//"{target}"/"$target"}"
  make_target="${make_target//"{build}"/"$build"}"
  make_target="${make_target//"{object}"/"$object"}"
  make_target="${make_target//"{install_dir}"/"$include_dir"}"
  make_target="${make_target//"{SOURCE}"/"$SOURCE"}"
  echo "$make_target"
  MAKEFILE="$MAKEFILE$make_target"
}

function GenerateHelp(){
  make_target="help:\n"
  MAKEFILE="$MAKEFILE$make_target"
}

function GenerateBase(){
  if [[ -e "Makefile" ]]; then
    MAKEFILE=$(<Makefile)
  else
    MAKEFILE="$MAKE_FILE_HEADER$MAKE_FILE_FUNCTIONS"
  fi
}

function AddExe(){
  printf "%b%s%b: " "\033[1;36m" "target" "\033[0m"
  read target
  printf "%b%s%b: " "\033[1;36m" "language" "\033[0m"
  read lang 
  printf "%b%s%b: " "\033[1;36m" "source" "\033[0m"
  read src 
  printf "%b%s%b: " "\033[1;36m" "build" "\033[0m"
  read build 
  printf "%b%s%b: " "\033[1;36m" "object" "\033[0m"
  read object 
  if [[ -z "$build" ]]; then
    build="build"
  fi
  GenerateExe "$target" "$lang" "$src" "$build" "$object"
}

function AddLib(){
  printf "%b%s%b: " "\033[1;36m" "target" "\033[0m"
  read target
  printf "%b%s%b: " "\033[1;36m" "language" "\033[0m"
  read lang 
  printf "%b%s%b: " "\033[1;36m" "source" "\033[0m"
  read src 
  printf "%b%s%b: " "\033[1;36m" "install dir" "\033[0m"
  read include_dir
  printf "%b%s%b: " "\033[1;36m" "build" "\033[0m"
  read build 
  printf "%b%s%b: " "\033[1;36m" "object" "\033[0m"
  read object 
  if [[ -z "$build" ]]; then
    build="build"
  fi
  GenerateLib "$target" "$lang" "$src" "$install_dir" "$build" "$object"
}

function SaveMakefile(){
  echo "$MAKEFILE" >> "Makefile"
}

if [[ "$1" == "addlib" ]] || [[ "$1" == "add-lib" ]] || [[ "$1" == "lib" ]]; then
  GenerateBase
  AddLib
elif [[ "$1" == "addexe" ]] || [[ "$1" == "add-exe" ]] || [[ "$1" == "exe" ]]; then
  GenerateBase
  AddExe
  SaveMakefile
elif [[ -z "$1" ]] || [[ "$1" == "new" ]]; then
  MAKEFILE="$MAKE_FILE_HEADER$MAKE_FILE_FUNCTIONS"
  running=true
  while [[ $running == true ]]; do
    printf ">> "
    read action
    if [[ "$action" == "addlib" ]] || [[ "$action" == "add-lib" ]] || [[ "$action" == "lib" ]]; then
      AddLib
    elif [[ "$action" == "addexe" ]] || [[ "$action" == "add-exe" ]] || [[ "$action" == "exe" ]]; then
      AddExe
    elif [[ -z "$action" ]] || [[ "$action" == "done" ]] || [[ "$action" == "quit" ]] || [[ "$action" == "finish" ]]; then
      running=false
    fi
  done
  SaveMakefile
fi
