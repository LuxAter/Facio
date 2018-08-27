#!/usr/bin/python3

import fnmatch
import glob
import re
import os
import subprocess
from typing.re import Pattern

OPTIONS = {'COLOR': True, 'DRY': False}
TARGETS = {}
VARIABLES = {}
PERC = 0.0
STEP = 0.0
ROOT = os.getcwd()


def glist(src):
    if isinstance(src, list):
        return src
    return [src]


def disp_path(path):
    return os.path.relpath(path, ROOT)


def get_comp(target, lang):
    if lang == 'C':
        if 'C' in TARGETS[target]:
            return TARGETS[target]['C']
        elif 'C' in VARIABLES:
            return VARIABLES['C']
        else:
            return 'cc'
    elif lang == 'C++':
        if 'CXX' in TARGETS[target]:
            return TARGETS[target]['CXX']
        elif 'CXX' in VARIABLES:
            return VARIABLES['CXX']
        else:
            return 'c++'
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
        return None, None
    else:
        ret = subprocess.run(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
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
    return base.split('.')[:-1] + ext


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
    elif 'build_dir' in TARGETS[target]:
        return os.path.join(ROOT, TARGETS['build_dir'], target)
    elif 'build_dir' in VARIABLES:
        return os.path.join(ROOT, VARIABLES['build_dir'], target, target)
    return os.path.join(ROOT, 'build', target, target)


def get_object_path(target, obj):
    if 'build_dir' in TARGETS[target]:
        return os.path.join(ROOT, TARGETS[target]['build_dir'], obj) + '.o'
    elif 'build_dir' in VARIABLES:
        return os.path.join(ROOT, VARIABLES['build_dir'], target, obj) + '.o'
    return os.path.join(ROOT, 'build', target, obj) + '.o'


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

def print_output(state, output):
    if state == 0:
        state = '32'
    elif state == 1:
        state = '1;31'
    elif state == 2:
        state = '1;33'
    elif state == 3:
        state = '1;34'
    print("       {}>{} {}".format(color(state), color('0'), '\n       {}>{} '.format(color(state), color('0')).join(output.split('\n'))))


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
        if 'CFLAGS' in TARGETS[target]:
            flags += glist(TARGETS[target]['CFLAGS'])
        elif 'CFLAGS' in VARIABLES:
            flags += glist(VARIABLES['CFLAGS'])
    elif lang == 'C++':
        if 'CXXFLAGS' in TARGETS[target]:
            flags += glist(TARGETS[target]['CXXFLAGS'])
        elif 'CXXFLAGS' in VARIABLES:
            flags += glist(VARIABLES['CXXFLAGS'])
    gdir(os.path.dirname(get_object_path(target, obj)))
    exit, res = execute_cmd("{} {} {} {} {} -o {}".format(
        comp, get_color(comp), " ".join(flags), " ".join(include), obj,
        get_object_path(target, obj)))
    ret = parse_output(comp, res)
    if ret != 0:
        print_build(target, obj, ret)
        print_output(ret, res)


def link_lib(target):
    lang = 'C'
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


def link_exe(target):
    lang = 'C'
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
        comp, get_color(comp), " ".join(flag), " ".join(deps), " ".join(link), " ".join(lib),
        get_target_dest(target)))
    ret = parse_output(comp, res)
    if ret != 0:
        print_link_exe(target, lang, ret)
        print_output(ret, res)

def run_cmd(target):
    def ex_cmd(cmd):
        if isinstance(cmd, list):
            cmd = ' '.join(cmd)
        print_run_cmd(cmd)
        exit, res = execute_cmd(cmd)
        if exit != 0:
            print_run_cmd(cmd, 1)
            print_output(1, res)
    if 'pre_cmd' in TARGETS[target]:
        ex_cmd(TARGETS[target]['pre_cmd'])
    if 'cmd' in TARGETS[target]:
        ex_cmd(TARGETS[target]['cmd'])
    for i in range(1, 10):
        if '{}_cmd'.format(i) in TARGETS[target]:
            ex_cmd(TARGETS[target]['{}_cmd'.format(i)])
    if 'post_cmd' in TARGETS[target]:
        ex_cmd(TARGETS[target]['post_cmd'])

def execute_target(target):
    global PERC
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
    if target != 'all':
        tars = [target]
    for key in('lib', 'dep', 'run'):
        if key not in TARGETS[target]:
            continue
        for src in glist(TARGETS[target][key]):
            if src in TARGETS.keys():
                tars += count_targets(src)
    return list(set(tars))


def main():
    global VARIABLES, TARGETS, STEP
    VARIABLES, TARGETS = parse_config('config.ini')
    target_count = len(count_targets('all'))
    STEP = 100.0 / target_count
    execute_target('all')


if __name__ == "__main__":
    main()
