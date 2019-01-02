"""
Facio V 3.0
Makefile geneorator for C/C++ projects.
"""
import json
import os
import sys


def end_replace(string, terms):
    """Replaces {{KEY}} in string with values from dictionary"""
    for key, value in terms.items():
        string = string.replace('{{' + key + '}}', value)
    return string


def recurse_replace(obj, terms):
    """
    Replaces all {{KEY}} values in strings in a dictionary with values from
    terms dictionary
    """
    for key in obj:
        if isinstance(obj[key], dict):
            obj[key] = recurse_replace(obj[key], terms)
        elif isinstance(obj[key], list):
            obj[key] = [end_replace(x, terms) for x in obj[key]]
        elif isinstance(obj[key], str):
            obj[key] = end_replace(obj[key], terms)
    return obj

def load_conf():
    """Reads facio.conf file if no config.json file is found"""

def load_config():
    """Reads config.json file"""
    raw = {}
    with open("config.json", 'r') as file:
        raw = json.load(file)
    settings = {'root': os.getcwd()}
    targets = {}
    for key, value in raw.items():
        if isinstance(value, str):
            settings[key] = value
        elif isinstance(value, list):
            settings[key] = ' '.join(value)
        else:
            targets[key] = value
    for key in settings:
        for key_sub, value_sub in settings.items():
            settings[key] = settings[key].replace("{{" + key_sub + "}}",
                                                  value_sub)
    for key in targets:
        targets[key] = recurse_replace(targets[key], settings)
        if 'name' not in targets[key]:
            targets[key]['name'] = key
        targets[key]['title'] = targets[key]['name'].upper()
    return settings, targets


def save_makefile(makefile):
    """Saves makefile"""
    with open("Makefile", "w") as file:
        file.write(makefile)


def gen_exe(target, targets):
    """Genetates make target for executables"""
    if 'dep' in target:
        target['link_deps'] = ' '.join([
            '$({})'.format(x.upper())
            for x in target['dep']
            if targets[x]['type'] == 'lib'
        ])
        target['dep'] = "build-" + " build-".join(target['dep'])
    else:
        target['link_deps'] = ''
        target['dep'] = ''
    if '*' in target['files']:
        target['file_find'] = "$(shell find \"{}\" -name \"{}\")".format(
            target['files'].split('*')[0], '*' + target['files'].split('*')[1])
    else:
        if isinstance(target['files'], list):
            target['files'] = ' '.join(target['files'])
        target['file_find'] = target['files']
    if 'files_exclude' in target:
        if isinstance(target['files_exclude'], list):
            target['files_exclude'] = ' '.join(target['files_exclude'])
        target['file_find'] = "$(filter-out {}, {})".format(
            target['files_exclude'], target['file_find'])
    return """
{title}={path}
{title}_FILES={file_find}
{title}_OBJS=$({title}_FILES:%=$(ROOT)/$(BUILD)/%.o)
-include $({title}_OBJS:.o=.d)

build-{name}: {dep} pre-{name} $({title})
	$(call complete_target,$(shell basename $({title})))

clean-{name}:
	$(call clean_target,$(shell basename $({title})))
	if [ -e "$({title})" ]; then rm $({title}); fi

pre-{name}:
	$(call scan_target,$(shell basename $({title})))

$({title}): $({title}_OBJS) FORCE
	$(call print_link_exe,$(shell basename $({title})))
	$(CXX) $({title}_OBJS) {link_deps} $(LINK) $(COMMON_INCLUDE) -o $({title})

install-{name}: build-{name}
	$(call install_target,$(shell basename $({title})))
	mkdir -p $(INSTALL_PATH)/bin/
	cp $({title}) $(INSTALL_PATH)/bin

uninstall-{name}:
	$(call uninstall_target,$(shell basename $({title})))
	if [ -e "$(INSTALL_PATH)/bin/$(shell basename $({title}))" ]; then rm $(INSTALL_PATH)/bin/$(shell basename $({title})); fi
""".format(**target)


def gen_lib(target, targets):
    """Generates targets for libraries"""
    if 'dep' in target:
        target['clean_deps'] = "clean-" + " clean-".join(target['dep'])
        target['link_deps'] = [x for x in target['dep'] if targets[x]['type'] in ('lib', 'aumake', 'make', 'faumake')]
        target['dep'] = "build-" + " build-".join(target['dep'])
        print(target['link_deps'])
    else:
        target['dep'] = ''
        target['clean_deps'] = ''
    if '*' in target['files']:
        target['file_find'] = "$(shell find \"{}\" -name \"{}\")".format(
            target['files'].split('*')[0], '*' + target['files'].split('*')[1])
    else:
        if isinstance(target['files'], list):
            target['files'] = ' '.join(target['files'])
        target['file_find'] = target['files']
    if 'files_exclude' in target:
        if isinstance(target['files_exclude'], list):
            target['files_exclude'] = ' '.join(target['files_exclude'])
        target['file_find'] = "$(filter-out {}, {})".format(
            target['files_exclude'], target['file_find'])
    if 'join' in target and target['join'] is True:
        target['join'] = '\n\t'.join(["mkdir -p $(ROOT)/tmp/{0} && cd $(ROOT)/tmp/{0} && ar x {1}/lib/{0} && ar qc $(ROOT)/$@ $(ROOT)/tmp/{0}/*.o && rm -rf $(ROOT)/tmp/{0}".format(x, targets[x]['path']) for x in target['link_deps']])
        print("JOINING!")
        print(target['join'])
    else:
        target['join'] = ''
    return """
{title}={path}
{title}_FILES={file_find}
{title}_OBJS=$({title}_FILES:%=$(ROOT)/$(BUILD)/%.o)
-include $({title}_OBJS:.o=.d)

build-{name}: {dep} pre-{name} $({title})
	$(call complete_target,$(shell basename $({title})))

clean-{name}: {clean_deps}
	$(call clean_target,$(shell basename $({title})))
	if [ -e "$({title})" ]; then rm $({title}); fi

pre-{name}:
	$(call scan_target,$(shell basename $({title})))

$({title}): $({title}_OBJS) FORCE
	$(call print_link_lib,$(shell basename $({title})))
	ar rcs $@ $({title}_OBJS)
	{join}

install-{name}: build-{name}
	$(call install_target,$(shell basename $({title})))
	mkdir -p $(INSTALL_PATH)/lib/
	mkdir -p $(INSTALL_PATH)/include/$(NAME)/
	cp $({title}) $(INSTALL_PATH)/lib
	if [ ! -z "$(INCLUDE_DIR)" ]; then cp -R $(INCLUDE_DIR)/ $(INSTALL_PATH)/include/$(NAME)/; fi
	if [ ! -z "$(shell find $(SOURCE_DIR) -name "*.h")" ]; then cd $(SOURCE_DIR) && cp --parents $(sehll cd $(SOURCE_DIR) && find . -name "*.h") $(INSTALL_PATH)/include/$(NAME); fi
	if [ ! -z "$(shell find $(SOURCE_DIR) -name "*.hpp")" ]; then cd $(SOURCE_DIR) && cp --parents $(sehll cd $(SOURCE_DIR) && find . -name "*.hpp") $(INSTALL_PATH)/include/$(NAME); fi

uninstall-{name}:
	$(call uninstall_target,$(shell basename $({title})))
	if [ ! -e "$(INSTALL_PATH)/lib/$(shell basename $({title}))" ]; then rm $(INSTALL_PATH)/lib/$(shell basename $({title})); fi
	if [ ! -e "$(INSTALL_PATH)/include/$(NAME)" ]; then rm $(INSTALL_PATH)/include/$(NAME) -r; fi
""".format(**target)


def gen_aumake(target, targets):
    if 'dep' in target:
        target['dep'] = "build-" + " build-".join(target['dep'])
    else:
        target['dep'] = ''
    return """
build-{name}: pre-{name}
	if [ ! -f "{source}/configure" ]; then $(call print_run_cmd,autogen.sh) && cd {source} && ./autogen.sh; fi
	if [ ! -f "{source}/Makefile" ]; then $(call print_run_cmd,configure) && cd {source} && ./configure --prefix={path}; fi
	if [ ! -d "{path}" ]; then $(call print_run_cmd,make) && cd {source} && make install; fi
	$(call complete_target,{name})

clean-{name}:
	$(call clean_target,{name})
	if [ -e "{source}/Makefile" ]; then cd {source} && make clean && rm Makefile; fi
	if [ -d "{path}" ]; then rm {path} -r; fi

pre-{name}:
	$(call scan_target,{name})
""".format(**target)

def gen_faumake(target, targets):
    if 'dep' in target:
        target['dep'] = "build-" + " build-".join(target['dep'])
    else:
        target['dep'] = ''
    return """
build-{name}: pre-{name}
	$(call print_run_cmd,autogen.sh) && cd {source} && ./autogen.sh
	$(call print_run_cmd,configure) && cd {source} && ./configure --prefix={path}
	if [ ! -d "{path}" ]; then $(call print_run_cmd,make) && cd {source} && make install; fi
	$(call complete_target,{name})

clean-{name}:
	$(call clean_target,{name})
	if [ -e "{source}/Makefile" ]; then cd {source} && make clean && rm Makefile; fi
	if [ -d "{path}" ]; then rm {path} -r; fi

pre-{name}:
	$(call scan_target,{name})
""".format(**target)


def gen_make(target, targets):
    if 'dep' in target:
        target['dep'] = "build-" + " build-".join(target['dep'])
    else:
        target['dep'] = ''
    return """
build-{name}: pre-{name}
	if [ ! -d "{path}" ]; then $(call print_run_cmd,make) && cd {source} && make install PREFIX={path}; fi
	$(call complete_target,{name})

clean-{name}:
	$(call clean_target,{name})
	if [ -e "{source}/Makefile" ]; then cd {source} && make clean && rm Makefile; fi
	if [ -d "{path}" ]; then rm {path} -r; fi

pre-{name}:
	$(call scan_target,{name})
""".format(**target)


def gen_target(target, targets):
    src = "# {} ".format(target['title']) + "{{{\n"
    if 'type' not in target or target['type'] == 'exe':
        return src + gen_exe(target, targets) + "\n# }}}\n"
    elif target['type'] == 'lib':
        return src + gen_lib(target, targets) + "\n# }}}\n"
    elif target['type'] == 'aumake':
        return src + gen_aumake(target, targets) + "\n# }}}\n"
    elif target['type'] == 'faumake':
        return src + gen_faumake(target, targets) + "\n# }}}\n"
    elif target['type'] == 'make':
        return src + gen_make(target, targets) + "\n# }}}\n"


def gen_pre(settings):
    print(settings)
    return """
SHELL=/bin/bash
ifndef .VERBOSE
    .SILENT:
endif
ROOT=$(shell pwd)
{}
COMMON_INCLUDE=-I$(ROOT)/$(INCLUDE_DIR) $(INCLUDE)

SCAN_COLOR=\\033[1;35m
BUILD_COLOR=\\033[32m
CLEAN_COLOR=\\033[1;33m
LINK_COLOR=\\033[1;32m
INSTALL_COLOR=\\033[1;36m
CMD_COLOR=\\033[1;34m
HELP_COLOR=\\033[1;34m

define scan_target
printf "%b%s%b\\n" "$(SCAN_COLOR)" "Scaning dependencies for target $(1)" "\\033[0m"
endef
define complete_target
printf "%s\\n" "Built target $(1)"
endef
define clean_target
printf "%b%s%b\\n" "$(CLEAN_COLOR)" "Cleaning target $(1)" "\\033[0m"
endef
define install_target
printf "%b%s%b\\n" "$(INSTALL_COLOR)" "Installing target $(1)" "\\033[0m"
endef
define uninstall_target
printf "%b%s%b\\n" "$(INSTALL_COLOR)" "Unnstalling target $(1)" "\\033[0m"
endef
define print_build_c
str=$$(realpath --relative-to="$(ROOT)" "$(1)");\
    printf "%b%s%b\\n" "$(BUILD_COLOR)" "Building C object $$str" "\\033[0m"
endef
define print_build_cpp
str=$$(realpath --relative-to="$(ROOT)" "$(1)");\
    printf "%b%s%b\\n" "$(BUILD_COLOR)" "Building C++ object $$str" "\\033[0m"
endef
define print_link_lib
str=$$(realpath --relative-to="$(ROOT)" "$(1)");\
    printf "%b%s%b\\n" "$(LINK_COLOR)" "Linking static library $$str" "\\033[0m"
endef
define print_link_exe
str=$$(realpath --relative-to="$(ROOT)" "$(1)");\
    printf "%b%s%b\\n" "$(LINK_COLOR)" "Linking executable $$str" "\\033[0m"
endef
define print_run_cmd
printf "%b%s%b\\n" "$(CMD_COLOR)" "Running '$(1)'" "\\033[0m"
endef
define help
printf "%b%*s%b: %s\\n" "$(HELP_COLOR)" 20 "$(1)" "\\033[0m" "$(2)"
endef


""".format("\n".join(
        ["{}={}".format(key.upper(), value) for key, value in settings.items()]))


def gen_post(settings):
    return """
$(ROOT)/$(BUILD)/%.cpp.o: %.cpp
	$(call print_build_cpp,$@)
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MMD -c $(COMMON_INCLUDE) $< -o $@

$(ROOT)/$(BUILD)/%.c.o: %.c
	$(call print_build_c,$@)
	mkdir -p $(@D)
	$(CC) $(CCFLAGS) -MMD -c $(COMMON_INCLUDE) $< -o $@

FORCE:
"""


def main():
    """Core component of the facio genorator"""
    settings, targets = load_config()
    makefile = gen_pre(settings)
    for key in targets:
        makefile += gen_target(targets[key], targets)
    makefile += gen_post(settings)
    save_makefile(makefile)


if __name__ == "__main__":
    main()
