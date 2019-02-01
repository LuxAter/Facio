"""
Facio V 3.0
Makefile geneorator for C/C++ projects.
"""
import json
import os
import argparse


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
        if 'gtest' in target['dep']:
            target['link_deps'] += ' {}/libgtest.a -lpthread'.format(targets['gtest']['path'])
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
        # TODO: Fix bug here as it will only work with lib stuffs
        target['join'] = '\n\t'.join(["mkdir -p $(ROOT)/tmp/{0} && cd $(ROOT)/tmp/{0} && ar x {1}/lib/{0} && ar qc $(ROOT)/$@ $(ROOT)/tmp/{0}/*.o && rm -rf $(ROOT)/tmp/{0}".format(x, targets[x]['path']) for x in target['link_deps']])
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

def gen_gtest(target, targets):
    if 'dep' in target:
        target['dep'] = "build-" + " build-".join(target['dep'])
    else:
        target['dep'] = ''
    return """
build-{name}: {dep} pre-gtest {path}/lib{name}.a
	$(call complete_target,googletest)

clean-{name}:
	$(call clean_target,googletest)
	if [ -d "{path}/googletest" ]; then rm {path}/googletest -r; fi
	if [ -e "{path}/lib{name}.a" ]; then rm {path}/lib{name}.a; fi

pre-{name}:
	$(call scan_target,googletest)

{path}/libgtest.a: {source}/src/gtest-all.cc {source}/src/gtest_main.cc
	mkdir -p {path}/googletest
	$(call print_build_cpp,{path}/googletest/gtests_all.o)
	$(CXX) -isystem {source}/include -I{source} -pthread -c {source}/src/gtest-all.cc -o {path}/googletest/gtests_all.o
	$(call print_build_cpp,{path}/googletest/gtests_main.o)
	$(CXX) -isystem {source}/include -I{source} -pthread -c {source}/src/gtest_main.cc -o {path}/googletest/gtests_main.o
	$(call print_link_lib,libgtest.a)
	ar -rc {path}/lib{name}.a {path}/googletest/gtests_main.o {path}/googletest/gtests_all.o
""".format(**target)

def gen_cmd(target, targets):
    if 'dep' in target:
        target['dep'] = "build-" + " build-".join(target['dep'])
    else:
        target['dep'] = ''
    return """
{name}: {dep}
	$(call execute,{name})
	{cmd}
	$(call complete_target,{name})

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
    elif target['type'] == 'gtest':
        return src + gen_gtest(target, targets) + "\n# }}}\n"
    elif target['type'] == 'cmd':
        return src + gen_cmd(target, targets) + "\n# }}}\n"


def gen_pre(settings):
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
	mkdir -p $(@D)
	$(call print_build_cpp,$@)
	$(CXX) $(CXXFLAGS) -MMD -c $(COMMON_INCLUDE) $< -o $@

$(ROOT)/$(BUILD)/%.c.o: %.c
	mkdir -p $(@D)
	$(call print_build_c,$@)
	$(CC) $(CCFLAGS) -MMD -c $(COMMON_INCLUDE) $< -o $@

FORCE:
"""

def gen_group(targets):
    def gen_cmd(name, pre, targets):
            return "{}: {}\n".format(name, ' '.join("{}-{}".format(pre, x) for x in targets))
    groups = {}
    for key, val in targets.items():
        if 'group' in val:
            if val['group'] in groups:
                groups[val['group']].append(key)
            else:
                groups[val['group']] = [key]
    string = ""
    if 'main' in groups:
        string += gen_cmd("all", "build", groups['main'])
        string += gen_cmd("clean", "clean", groups['main'])
        string += gen_cmd("install", "install", groups['main'])
        string += gen_cmd("uninstall", "uninstall", groups['main'])
        string += '\n'
    for key, val in groups.items():
        if key == "main":
            continue
        string += gen_cmd("{}".format(key), "build", val)
        string += gen_cmd("clean-{}".format(key), "clean", val)
        string += gen_cmd("install-{}".format(key), "install", val)
        string += gen_cmd("uninstall-{}".format(key), "uninstall", val)
        string += '\n'
    return string



def parse_config(raw):
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

def load_config():
    """Loads config file"""
    config = {}
    if os.path.isfile("config.json"):
        with open("config.json", 'r') as file:
            config = json.load(file)
    else:
        print("ERROR: No config.json or facio.cfg file found")
    return config

def gen_config():
    """Generates a default config.json file"""
    config_base = {"CXX": "clang++", "CC": "clang", "CCIGNORE": "", "CXXIGNORE": "", "CCFLAGS": ["-fPIC -Wall -Wpedantic --static"], "CXXFLAGS": ["-std=c++17", "-fPIC", "-Wall", "-Wpedantic", "--static"], "LINK": "", "source": "src", "include_dir": "include", "include": [], "build":"build"}
    with open("config.json", 'w') as file:
        json.dump(config_base, file)

def modify_config(config, args):
    """Modifies both config.json if it exist"""
    obj = {'path': args.path.replace('[[', '$(').replace(']]', ')')}
    if 'exclude' in args and args.exclude and args.exclude not in ('none', 'None', ''):
        obj['files_exclude'] = args.exclude.replace('[[', '$(').replace(']]', ')')
    if 'files' in args and args.files:
        obj['files'] = args.files.replace('[[', '$(').replace(']]', ')')
    if 'dep' in args and args.dep:
        obj['dep'] = args.dep
    if 'join' in args and args.join:
        obj['join'] = True
    if 'source' in args and args.source:
        obj['source'] = args.source.replace('[[', '$(').replace(']]', ')')
    if 'group' in args and args.group and args.group not in ('none', 'None', ''):
        obj['group'] = args.group
    if args.mtype == 'config':
        config[args.name] = args.value.replace('[[', '$(').replace(']]', ')')
    elif args.mtype == 'exe':
        obj['type'] = 'exe'
        config[args.name] = obj
    elif args.mtype == 'lib':
        obj['type'] = 'lib'
        config[args.name] = obj
    elif args.mtype == 'aumake':
        obj['type'] = 'aumake'
        config[args.name] = obj
    elif args.mtype == 'faumake':
        obj['type'] = 'faumake'
        config[args.name] = obj
    elif args.mtype == 'make':
        obj['type'] = 'make'
        config[args.name] = obj
    with open("config.json", 'w') as file:
        json.dump(config, file)

def main():
    """Core component of the facio genorator"""
    parser = argparse.ArgumentParser("Facio")
    subparsers = parser.add_subparsers(dest='command')
    modify = subparsers.add_parser('add', help="modifies current config file")
    parser.add_argument("--gen-config", action='store_true', help="Generates config from either base config or from facio.conf")
    parser.add_argument('config', nargs='?', type=str, default="", help="Config files to generate make file from")
    msubparsers = modify.add_subparsers(dest='mtype')
    config = msubparsers.add_parser('config', help="Add config variable")
    config.add_argument('name', help="Name of the target")
    config.add_argument('value', help="Value of the config variable")
    exe = msubparsers.add_parser('exe', help="Add exe target")
    exe.add_argument('name', help="Name of the target")
    exe.add_argument('path', help="Destination path")
    exe.add_argument('files', help="Files to use for target")
    exe.add_argument('exclude', nargs="?", help="Files to exclude from target")
    exe.add_argument('group', nargs='?', help="Adds target to group target")
    exe.add_argument('dep', nargs='*', help="Target dependencies")
    lib = msubparsers.add_parser('lib', help="Add lib target")
    lib.add_argument('name', help="Name of the target")
    lib.add_argument('path', help="Destination path")
    lib.add_argument('files', help="Files to use for target")
    lib.add_argument('exclude', nargs="?", help="Files to exclude from target")
    lib.add_argument('group', nargs='?', help="Adds target to group target")
    lib.add_argument('dep', nargs='*', help="Target dependencies")
    lib.add_argument('--join', action="store_true", help="Joins target dependencies into one library")
    aumake = msubparsers.add_parser('aumake', help="Add aumake target")
    aumake.add_argument('name', help="Name of the target")
    aumake.add_argument('path', help="Destination path")
    aumake.add_argument('source', help="Source path")
    aumake.add_argument('group', nargs='?', help="Adds target to group target")
    faumake = msubparsers.add_parser('faumake', help="Add faumake target")
    faumake.add_argument('name', help="Name of the target")
    faumake.add_argument('path', help="Destination path")
    faumake.add_argument('source', help="Source path")
    faumake.add_argument('group', nargs='?', help="Adds target to group target")
    make = msubparsers.add_parser('make', help="Add make target")
    make.add_argument('name', help="Name of the target")
    make.add_argument('path', help="Destination path")
    make.add_argument('source', help="Source path")
    make.add_argument('group', nargs='?', help="Adds target to group target")
    args = parser.parse_args()
    print(args)
    config = load_config()
    if args.command == 'add':
        modify_config(config, args)
    elif args.gen_config:
        gen_config()
    else:
        settings, targets = parse_config(config)
        makefile = gen_pre(settings)
        makefile += gen_group(targets)
        for key in targets:
            makefile += gen_target(targets[key], targets)
        makefile += gen_post(settings)
        save_makefile(makefile)


if __name__ == "__main__":
    main()
