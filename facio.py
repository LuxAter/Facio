#!/usr/bin/python3

import fnmatch
import glob
import re
import os
import subprocess
import argparse
import json
from typing.re import Pattern

from pprint import pprint

OPTIONS = {}
TARGETS = {}
VARIABLES = {}
CMD_VARS = {}
PERC = 0.0
STEP = 0.0
ROOT = os.getcwd()
COMPILE_COMMANDS = []


def add_cmd(cmd, directory, file):
    global COMPILE_COMMANDS
    for i, entry in enumerate(COMPILE_COMMANDS):
        print(entry)
        if entry['directory'] == directory and entry['file'] == file:
            entry['arguments'] = [x.strip() for x in cmd.split()]
            return None
    print(COMPILE_COMMANDS)
    COMPILE_COMMANDS.append({
        'arguments': [x.strip() for x in cmd.split()],
        'directory': directory,
        'file': file
    })
    print(COMPILE_COMMANDS)


def glist(src):
    if isinstance(src, list):
        return src
    return [src]


def disp_path(path):
    return os.path.relpath(path, ROOT)


def get_cascade(key, target=None, default=None):
    if key in CMD_VARS:
        return CMD_VARS[key]
    elif target is not None and key in TARGETS[target]:
        return TARGETS[target][key]
    elif key in VARIABLES:
        return VARIABLES[key]
    return default


def get_comp(target, lang):
    if lang == 'C':
        return get_cascade('C', target, 'cc')
    elif lang == 'C++':
        return get_cascade('CXX', target, 'c++')
    else:
        return "c++"


def get_color(comp):
    if comp in ('gcc', 'g++', 'cc', 'c++'):
        return '-fdiagnostics-color=always'
    elif comp in ('clang', 'clang++'):
        return '-fcolor-diagnostics'


def parse_output(cmd, ret):
    if ret is None:
        return 0
    if cmd in ('gcc', 'g++', 'cc', 'c++'):
        if 'error: ' in ret:
            return 1
        elif 'warning: ' in ret:
            return 2
        elif 'note: ' in ret:
            return 3
    elif cmd in ('clang', 'clang++'):
        if 'error: ' in ret:
            return 1
        elif 'warning: ' in ret:
            return 2
        elif 'note: ' in ret:
            return 3
    elif cmd in ('ar'):
        pass
    return 0


def get_lang(obj):
    ext = '.' + obj.split('.')[-1]
    if ext in ('.cpp', '.cxx', '.c++'):
        return "C++"
    elif ext in ('.c'):
        return "C"


def color(code):
    if OPTIONS['COLOR']:
        return "\033[{}m".format(code)
    return ''


def execute_cmd(cmd):
    if OPTIONS['DRY']:
        print("       > {}".format(cmd))
        return 0, ''
    else:
        ret = subprocess.run(
            cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        return ret.returncode, ret.stdout.decode('utf-8')


def gdir(dirname):
    if not os.path.exists(dirname):
        if OPTIONS['DRY']:
            print("       > mkdir -p {}".format(dirname))
        else:
            os.makedirs(dirname)


def replace_regex(regex):
    return glob.glob(regex, recursive=True)


def replace_extension(base, ext):
    return '.'.join(base.split('.')[:-1]) + ext


def filter_out(pat, src):
    out = []
    if not isinstance(pat, list):
        pat = [pat]
    for string in src:
        good = True
        for reg in pat:
            if isinstance(reg, Pattern):
                if reg.match(string):
                    good = False
            elif isinstance(reg, str):
                if string == reg:
                    good = False
        if good is True:
            out.append(string)
    return out


def append(src, ext):
    if isinstance(src, str):
        return src + ext
    elif isinstance(src, list):
        return [x + ext for x in src]


def prepend(pre, src):
    if isinstance(src, str):
        return pre + src
    elif isinstance(src, list):
        return [pre + x for x in src]


def mod(pre, src, post):
    if isinstance(src, str):
        return pre + src + post
    elif isinstance(src, list):
        return [pre + x + post for x in src]


def parse_function(func, args):
    params = []
    count = 0
    pos = 0
    for i, ch in enumerate(args):
        if ch == '(':
            count += 1
        elif ch == ')' and count > 0:
            count -= 1
        if ch == ',' and count == 0:
            params.append(parse_variable(args[pos:i].strip()))
            pos = i + 1
    params.append(parse_variable(args[pos:].strip()))
    func = func.lower()
    if func == 'filter-out':
        return filter_out(params[0], params[1])
    elif func == 'append':
        return append(params[0], params[1])
    elif func == 'prepend':
        return prepend(params[0], params[1])
    elif func == 'mod':
        return mod(params[0], params[1], params[2])


def parse_variable(variable):
    func_re = re.compile('([^\s\(]+)\((.*)\)')
    if func_re.match(variable):
        match = func_re.match(variable)
        return parse_function(match.group(1), match.group(2))
    elif ' ' in variable:
        return [parse_variable(x) for x in variable.split()]
    elif variable.startswith('r') and ('*' in variable or '?' in variable):
        return re.compile(fnmatch.translate(variable[1:]))
    elif '*' in variable or '?' in variable:
        return replace_regex(variable)
    else:
        return variable


def parse_config(file_path):
    lines = ''
    with open(file_path, 'r') as file:
        lines = file.readlines()
    variables = {}
    targets = {}
    current = None
    for line in lines:
        if line == '\n' or line.startswith(';'):
            continue
        line = line.rstrip('\n')
        if line.startswith('['):
            targets[line[1:line.find(']')]] = {}
            current = line[1:line.find(']')]
            if current.startswith('lib') and current.endswith('.a'):
                targets[current]['type'] = 'lib'
            elif current == 'all':
                targets[current]['type'] = 'default'
            elif current.startswith('clean'):
                targets[current]['type'] = 'clean'
            else:
                targets[current]['type'] = 'exe'
        elif current is None:
            variables[line.split('=')[0]] = parse_variable(
                '='.join(line.split('=')[1:]))
        else:
            targets[current][line.split('=')[0]] = parse_variable(
                '='.join(line.split('=')[1:]))
    return variables, targets


def get_target_dest(target):
    if 'output_dir' in TARGETS[target]:
        return os.path.join(ROOT, TARGETS[target]['output_dir'], target)
    return os.path.join(ROOT, get_cascade('build_dir', target, 'build'),
                        target, target)


def get_object_path(target, obj):
    return os.path.join(ROOT, get_cascade('build_dir', target, 'build'),
                        target, obj) + '.o'


def get_dep_file(target, obj=None):
    if obj is None:
        dep_dir = get_target_dest(target) + '.d'
    else:
        dep_dir = replace_extension(get_object_path(target, obj), '.d')
    if os.path.isfile(dep_dir):
        # pass
        print("YAY!", dep_dir)
    return []


def scan_deps(target, deps):
    dest = get_target_dest(target)
    if not os.path.isfile(dest):
        return True
    target_mtime = os.path.getmtime(dest)
    for dep in glist(deps):
        if dep.endswith('.c') or dep.endswith('.cpp'):
            dep = get_object_path(target, dep)
        # print(dep)
        if os.path.isfile(dep) and os.path.getmtime(dep) > target_mtime:
            return True


def print_scan(target):
    print("{}Scaning dependencies for target {}{}".format(
        color("1;35"), target, color("0")))


def print_target_complete(target):
    print("[{:3.0f}%] Built target {}".format(PERC, target))


def print_target_cleaned(target):
    print("[{:3.0f}%] Cleaned target {}".format(PERC, target))


def print_build(target, obj, state=0):
    if state == 0:
        state = '32'
    elif state == 1:
        print("\033[A", end='')
        state = '1;31'
    elif state == 2:
        print("\033[A", end='')
        state = '1;33'
    elif state == 3:
        print("\033[A", end='')
        state = '1;34'
    print("[{:3.0f}%] {}Building {} object {}{}".format(
        PERC, color(state), get_lang(obj),
        disp_path(get_object_path(target, obj)), color('0')))


def print_clean(target, obj):
    print("[{:3.0f}%] {}Removing {} object {}{}".format(
        PERC, color("33"), get_lang(obj),
        disp_path(get_object_path(target, obj)), color('0')))


def print_output(state, output):
    if state == 0:
        if OPTIONS['LEVEL'] > 1:
            return
        state = '32'
    elif state == 1:
        if OPTIONS['LEVEL'] > 4:
            return
        state = '1;31'
    elif state == 2:
        if OPTIONS['LEVEL'] > 3:
            return
        state = '1;33'
    elif state == 3:
        if OPTIONS['LEVEL'] > 2:
            return
        state = '1;34'
    print("       {}>{} {}".format(
        color(state), color('0'), '\n       {}>{} '.format(
            color(state), color('0')).join(output.split('\n'))))


def print_link_lib(target, lang, state=0):
    if state == 0:
        state = '1;36'
    elif state == 1:
        print("\033[A", end='')
        state = '1;31'
    elif state == 2:
        print("\033[A", end='')
        state = '1;33'
    elif state == 3:
        print("\033[A", end='')
        state = '1;34'
    print("[{:3.0f}%] {}Linking {} static library {}{}".format(
        PERC, color(state), lang, target, color('0')))


def print_clean_lib(target, lang):
    print("[{:3.0f}%] {}Removing {} static library {}{}".format(
        PERC, color("1;36"), lang, target, color('0')))


def print_link_exe(target, lang, state=0):
    if state == 0:
        state = '1;36'
    elif state == 1:
        print("\033[A", end='')
        state = '1;31'
    elif state == 2:
        print("\033[A", end='')
        state = '1;33'
    elif state == 3:
        print("\033[A", end='')
        state = '1;34'
    print("[{:3.0f}%] {}Linking {} executable {}{}".format(
        PERC, color(state), lang, target, color('0')))


def print_clean_exe(target, lang):
    print("[{:3.0f}%] {}Removing {} executable {}{}".format(
        PERC, color("1;36"), lang, target, color('0')))


def print_run_cmd(cmd, state=0):
    if state == 0:
        state = '32'
    elif state == 1:
        print("\033[A", end='')
        state = '1;31'
    elif state == 2:
        print("\033[A", end='')
        state = '1;33'
    elif state == 3:
        print("\033[A", end='')
        state = '1;34'
    print("[{:3.0f}%] {}Running command \'{}\'{}".format(
        PERC, color(state), cmd, color('0')))


def build_object(target, obj):
    print_build(target, obj)
    lang = get_lang(obj)
    comp = get_comp(target, get_lang(obj))
    include = []
    if 'include' in TARGETS[target]:
        include = ['-I' + x for x in glist(TARGETS[target]['include'])]
    flags = ['-MMD -c']
    if lang == 'C':
        flags += glist(get_cascade('CFLAGS', target, []))
    elif lang == 'C++':
        flags += glist(get_cascade('CXXFLAGS', target, []))
    gdir(os.path.dirname(get_object_path(target, obj)))
    exit, res = execute_cmd("{} {} {} {} {} -o {}".format(
        comp, get_color(comp), " ".join(flags), " ".join(include), obj,
        get_object_path(target, obj)))
    if OPTIONS['gen-cmds']:
        add_cmd("{} {} {} {} -o {}".format(
            "cc"
            if lang == "C" else "cpp", " ".join(flags), " ".join(include), obj,
            get_object_path(target, obj)), os.path.abspath(
                os.path.dirname(obj)), os.path.basename(obj))
    ret = parse_output(comp, res)
    if ret != 0:
        print_build(target, obj, ret)
        print_output(ret, res)
    elif OPTIONS['VERBOSE']:
        print_output(ret, res)


def clean_object(target, obj):
    if os.path.isfile(get_object_path(target, obj)) or os.path.isfile(
            replace_extension(get_object_path(target, obj), '.d')):
        print_clean(target, obj)
        execute_cmd("rm {} {}".format(
            get_object_path(target, obj),
            replace_extension(get_object_path(target, obj), '.d')))


def link_lib(target):
    lang = get_lang(TARGETS[target]['source'][-1])
    print_link_lib(target, lang)
    deps = [
        get_object_path(target, x) for x in glist(TARGETS[target]['source'])
    ]
    gdir(os.path.dirname(get_target_dest(target)))
    exit, res = execute_cmd("ar rvsc {} {}".format(
        get_target_dest(target), " ".join(deps)))
    if exit != 0:
        print_link_lib(target, lang, 1)
        print_output(1, res)
    elif OPTIONS['VERBOSE']:
        print_output(0, res)


def clean_lib(target):
    if os.path.isfile(get_target_dest(target)):
        lang = get_lang(TARGETS[target]['source'][-1])
        print_clean_lib(target, lang)
        execute_cmd("rm {}".format(get_target_dest(target)))


def link_exe(target):
    lang = get_lang(TARGETS[target]['source'][-1])
    print_link_exe(target, lang)
    deps = [
        get_object_path(target, x) for x in glist(TARGETS[target]['source'])
    ]
    comp = get_comp(target, lang)
    flag = glist(TARGETS[target]['flag']) if 'flag' in TARGETS[target] else []
    link = []
    if 'link' in TARGETS[target]:
        link = ['-l' + x for x in glist(TARGETS[target]['link'])]
    lib = []
    if 'lib' in TARGETS[target]:
        lib = [get_target_dest(x) for x in glist(TARGETS[target]['lib'])]
    gdir(os.path.dirname(get_target_dest(target)))
    exit, res = execute_cmd("{} {} {} {} {} {} -o {}".format(
        comp, get_color(comp), " ".join(flag), " ".join(deps), " ".join(link),
        " ".join(lib), get_target_dest(target)))
    ret = parse_output(comp, res)
    if ret != 0:
        print_link_exe(target, lang, ret)
        print_output(ret, res)
    elif OPTIONS['VERBOSE']:
        print_output(ret, res)


def clean_exe(target):
    if os.path.isfile(get_target_dest(target)):
        lang = get_lang(TARGETS[target]['source'][-1])
        print_clean_exe(target, lang)
        execute_cmd("rm {}".format(get_target_dest(target)))


def run_cmd(target):
    def ex_cmd(cmd):
        if isinstance(cmd, list):
            cmd = ' '.join(cmd)
        print_run_cmd(cmd)
        exit, res = execute_cmd(cmd)
        if exit != 0:
            print_run_cmd(cmd, 1)
            print_output(1, res)
        elif OPTIONS['VERBOSE']:
            print_output(0, res)

    if 'pre_cmd' in TARGETS[target]:
        ex_cmd(TARGETS[target]['pre_cmd'])
    if 'cmd' in TARGETS[target]:
        ex_cmd(TARGETS[target]['cmd'])
    for i in range(1, 10):
        if '{}_cmd'.format(i) in TARGETS[target]:
            ex_cmd(TARGETS[target]['{}_cmd'.format(i)])
    if 'post_cmd' in TARGETS[target]:
        ex_cmd(TARGETS[target]['post_cmd'])


def clean_target(target):
    global PERC
    if TARGETS[target]['type'] == 'clean':
        for key in ('lib', 'dep', 'run'):
            if key not in TARGETS[target]:
                continue
            for src in glist(TARGETS[target][key]):
                if src in TARGETS.keys():
                    clean_target(src)
    if TARGETS[target]['type'] not in ('default', 'clean'):
        print_scan(target)
    if 'source' in TARGETS[target]:
        step = STEP / len(glist(TARGETS[target]['source']))
        for dep in glist(TARGETS[target]['source']):
            clean_object(target, dep)
            PERC += step
        if TARGETS[target]['type'] == 'lib':
            clean_lib(target)
        elif TARGETS[target]['type'] == 'exe':
            clean_exe(target)
    else:
        PERC += STEP
    if TARGETS[target]['type'] not in ('default', 'clean'):
        print_target_cleaned(target)


def execute_target(target):
    global PERC
    if target.startswith('clean'):
        clean_target(target)
        return
    for key in ('lib', 'dep', 'run'):
        if key not in TARGETS[target]:
            continue
        for src in glist(TARGETS[target][key]):
            if src in TARGETS.keys():
                execute_target(src)
    update = False
    if TARGETS[target]['type'] != 'default':
        print_scan(target)
        if TARGETS[target]['type'] != 'cmd':
            update = scan_deps(target, TARGETS[target]['source'])
        else:
            update = True
    if update is True:
        if 'source' in TARGETS[target]:
            step = STEP / len(glist(TARGETS[target]['source']))
            for dep in glist(TARGETS[target]['source']):
                build_object(target, dep)
                PERC += step
        if TARGETS[target]['type'] == 'lib':
            link_lib(target)
        elif TARGETS[target]['type'] == 'exe':
            link_exe(target)
        elif TARGETS[target]['type'] == 'cmd':
            run_cmd(target)
    else:
        PERC += STEP
    if TARGETS[target]['type'] != 'default':
        print_target_complete(target)


def count_targets(target):
    tars = []
    if target not in ('all') and not target.startswith('clean'):
        tars = [target]
    for key in ('lib', 'dep', 'run'):
        if key not in TARGETS[target]:
            continue
        for src in glist(TARGETS[target][key]):
            if src in TARGETS.keys():
                tars += count_targets(src)
    return list(set(tars))

def generate_makefile():
    pass

def main():
    global VARIABLES, TARGETS, STEP, CMD_VARS, COMPILE_COMMANDS
    parser = argparse.ArgumentParser(description="Facio build system")
    parser.add_argument(
        '-v', '--verbose', action='store_true', help='enables verbose output')
    parser.add_argument(
        '-d',
        '--dry',
        action='store_true',
        help='runs the build system without executing the commands')
    parser.add_argument(
        'args', default=['all'], nargs='*', help='additional arguments')
    group = parser.add_argument_group('generators')
    group.add_argument(
        '-g',
        '--gen-cmds',
        action="store_true",
        help="generates a compile_commands.json")
    group.add_argument(
        '-m', '--gen-make', action="store_true", help='generates GNU makefile')
    group.add_argument(
        '-b', '--gen-bash', action="store_true", help='generates bash script')
    group.add_argument(
        '-s',
        '--gen-script',
        action="store_true",
        help='generates simple bash script')
    group = parser.add_argument_group('diagnostics')
    group.add_argument(
        '-C',
        '--no-color',
        action="store_true",
        help="disables colored output")
    group.add_argument(
        '-l',
        '--level',
        type=int,
        default=1,
        choices=[1, 2, 3, 4, 5],
        help='sets what level of messages to display')
    args = parser.parse_args()
    OPTIONS['VERBOSE'] = args.verbose
    OPTIONS['DRY'] = args.dry
    OPTIONS['COLOR'] = not args.no_color
    OPTIONS['LEVEL'] = args.level
    OPTIONS['gen-cmds'] = args.gen_cmds
    if args.gen_make:
        print("Generating Makefile")
        generate_makefile()
        return
    VARIABLES, TARGETS = parse_config('config.ini')
    targets = [x for x in args.args if '=' not in x]
    if OPTIONS['gen-cmds'] and os.path.isfile('compile_commands.json'):
        with open('compile_commands.json', 'r') as file:
            COMPILE_COMMANDS = json.load(file)
    if not targets:
        targets = ['all']
    variables = [x for x in args.args if '=' in x]
    for var in variables:
        CMD_VARS[var.split('=')[0]] = '='.join(var.split('=')[1:])
    target_count = 0
    for tar in targets:
        target_count += len(count_targets(tar))
    STEP = 100.0 / target_count
    for tar in targets:
        execute_target(tar)

    if OPTIONS['gen-cmds']:
        with open('compile_commands.json', 'w') as file:
            json.dump(COMPILE_COMMANDS, file)


if __name__ == "__main__":
    main()
