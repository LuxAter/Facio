#!/usr/bin/python3

import configparser
import argparse
import os
import glob
from fnmatch import fnmatch
import subprocess
import sys

from pprint import pprint
import json

OPTIONS = {'DRY': False, 'COLOR': True}
PERC = 0.0

VARIABLES = {'CXX': 'clang++', 'FLAGS': [], 'LINKS': []}
TARGETS = {}


class Target(object):
    def __init__(self):
        self.msg = ''
        self.color = ''
        self.cmd = ''


def replace_dict(text, items):
    """Replaces all values in a string based on dictionary"""
    for i, j in items.items():
        if isinstance(j, list):
            text = text.replace('$' + i, ' '.join(j))
        elif isinstance(j, str):
            text = text.replace('$' + i, j)
    return text


def run_cmd(cmd, local_var):
    cmd = replace_dict(cmd, local_var)
    if OPTIONS['DRY']:
        print(">>  {}".format(cmd))
        return None
    return subprocess.run(
        cmd.split(), stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT).stdout.decode('utf-8')


def gen_folder(path):
    if not os.path.exists(path):
        os.makedirs(path)


def color(code):
    if OPTIONS['COLOR']:
        return "\033[{}m".format(code)
    return ""


def get_files(base):
    files = []
    if 'source_dir' in TARGETS[base]:
        for sdir in TARGETS[base]['source_dir']:
            files = glob.glob(
                "{}/**/*.cpp".format(sdir.rstrip('/')), recursive=True)
    if 'source_file' in TARGETS[base]:
        for sfile in TARGETS[base]['source_file']:
            files += [sfile]
    if 'source' in TARGETS[base]:
        for src in TARGETS[base]['source']:
            if src.endswith('/'):
                files = glob.glob(
                    "{}/**/*.cpp".format(src.rstrip('/')), recursive=True)
            else:
                files += [src]
    files = list(set(files))
    if 'exclude_dir' in TARGETS[base]:
        for edir in TARGETS[base]['exclude_dir']:
            files = [
                x for x in files
                if not fnmatch(x, '*/{}/*'.format(edir.rstrip('/')))
            ]
    if 'exclude_file' in TARGETS[base]:
        for efile in TARGETS[base]['exclude_file']:
            files = [x for x in files if not fnmatch(x, '*/{}'.format(efile))]
    if 'exclude' in TARGETS[base]:
        for exc in TARGETS[base]['exclude']:
            if exc.endswith('/'):
                files = [
                    x for x in files
                    if not fnmatch(x, '*/{}/*'.format(exc.rstrip('/')))
                ]
            else:
                files = [
                    x for x in files if not fnmatch(x, '*/{}'.format(exc))
                ]
    return list(set(files))


def get_deps(file_path):
    lines = []
    with open(file_path, 'r') as file:
        lines = " ".join(" ".join(file.readlines()).split(':')[1:])[1:]
    lines = lines.replace(' \\\n  ', '')
    lines = lines.replace('\n', '')
    return lines.split()


def gen_target_tree(base):
    if base is None:
        base = 'default'
    tree = []
    for key in ('run', 'dep', 'deps', 'lib'):
        if key in TARGETS[base]:
            for tar in TARGETS[base][key]:
                if tar in TARGETS:
                    build_dir = TARGETS[base]['object_dir'][
                        0] if 'object_dir' in TARGETS[base] else None
                    if build_dir is None:
                        build_dir = TARGETS[base]['build_dir'][
                            0] if 'build_dir' in TARGETS[base] else './'
                    tree.append({
                        'target': tar,
                        'dest': os.path.join(build_dir, tar),
                        'deps': gen_target_tree(tar),
                        'type': TARGETS[tar]['type'][0],
                        'update': False
                    })
                else:
                    print("ERR {}".format(tar))
    for file in get_files(base):
        build_dir = TARGETS[base]['object_dir'][0] if 'object_dir' in TARGETS[
            base] else None
        if build_dir is None:
            build_dir = TARGETS[base]['build_dir'][
                0] if 'build_dir' in TARGETS[base] else './'
        obj = os.path.join(build_dir, file) + '.o'
        tree.append({
            'target': obj,
            'dest': os.path.join(build_dir, obj),
            'deps': [file],
            'type': 'object',
            'update': False
        })
        if os.path.isfile(os.path.join(build_dir, file) + '.d'):
            tree[-1]['deps'] += get_deps(os.path.join(build_dir, file) + '.d')
        tree[-1]['deps'] = list(set(tree[-1]['deps']))
    return tree


def check_dep_tree(deps):
    res = False
    if deps['type'] == 'object':
        if not os.path.isfile(deps['target']):
            res = True
        else:
            dest = os.path.getmtime(deps['target'])
            for dep in deps['deps']:
                if os.path.isfile(dep) and os.path.getmtime(dep) > dest:
                    res = True
                    break
    else:
        if not os.path.isfile(
                deps['target']
        ) and deps['target'] != 'default' and deps['type'] != 'cmd':
            res = True
        for dep in deps['deps']:
            if check_dep_tree(dep):
                res = True
    deps['update'] = res
    if deps['type'] != 'object':
        if res is True:
            for dep in deps['deps']:
                if dep['type'] == 'cmd':
                    dep['update'] = True
    return res


def read_targets():
    global TARGETS
    config = configparser.ConfigParser()
    config.read('config.ini')
    for sec in config:
        TARGETS[sec] = {}
        for var in config[sec]:
            TARGETS[sec][var] = config[sec][var].split()


def execute_object(tree, local_var):
    print("[{:3.0f}%] {}{}{}".format(PERC, color("32"),
                                     "Building C++ object {}".format(
                                         tree['target']), color("0")))
    gen_folder(os.path.dirname(tree['target']))
    ret = run_cmd("{} -fcolor-diagnostics {} -MMD -c {} -o {}".format(
        local_var['CXX'], ' '.join(local_var['FLAGS']), ' '.join(
            [x for x in tree['deps'] if x.endswith('.cpp')]), tree['target']),
                  local_var)


def execute_lib(tree, local_var):
    print("[{:3.0f}%] {}{}{}".format(PERC, color("1;36"),
                                     "Linking C++ static library {}".format(
                                         tree['dest']), color("0")))
    deps = [x['dest'] for x in tree['deps'] if x['target'].endswith('.o')]
    libs = [x['dest'] for x in tree['deps'] if x['target'].endswith('.a')]
    ret = run_cmd("ar rvs {} {}".format(tree['target'], " ".join(deps)),
                  local_var)
    print(ret)


def execute_exe(tree, local_var):
    print("[{:3.0f}%] {}{}{}".format(PERC, color("1;36"),
                                     "Linking C++ executable {}".format(
                                         tree['target']), color("0")))
    deps = [x['dest'] for x in tree['deps'] if x['target'].endswith('.o')]
    libs = [x['dest'] for x in tree['deps'] if x['target'].endswith('.a')]
    ret = run_cmd("{} -fcolor-diagnostics {} {} {} -o {}".format(
        local_var['CXX'], ' '.join(local_var['FLAGS']), ' '.join(deps),
        ' '.join(local_var['LINKS'] + libs), tree['dest']), local_var)
    print(ret)


def execute_tree(tree, local_var=VARIABLES):
    if tree['update'] is False:
        return
    if tree['target'] in TARGETS:
        target = TARGETS[tree['target']]
        if 'include' in target:
            local_var['FLAGS'] += (
                '-I' + ' -I'.join(target['include'])).split()
            local_var['FLAGS'] = list(set(local_var['FLAGS']))

    if tree['type'] != 'object':
        for dep in tree['deps']:
            if dep['type'] != 'object':
                execute_tree(dep, local_var)
    if tree['target'] != 'default' and tree['type'] != 'object':
        print("{}{}{}".format(
            color("1;35"), "Scaning dependencies for target {}".format(
                tree['target']), color("0")))
    if tree['type'] != 'object':
        for dep in tree['deps']:
            if dep['type'] == 'object':
                execute_tree(dep, local_var)
    if tree['type'] == 'object':
        execute_object(tree, local_var)
    elif tree['type'] == 'lib':
        execute_lib(tree, local_var)
    elif tree['type'] == 'exe':
        execute_exe(tree, local_var)


def main():
    global OPTIONS
    read_targets()
    parser = argparse.ArgumentParser(description="Facio build system")
    parser.add_argument(
        '-d', '--dry', action='store_true', help="Dry runs the build system")
    parser.add_argument(
        'target',
        nargs='*',
        default=['all'],
        help="List of targets to run")
    args = parser.parse_args()
    OPTIONS['DRY'] = args.dry
    for tar in list(args.target):
        if tar is None or tar == 'all' or tar == 'DEFAULT' or tar == 'default':
            tree = {
                'target': 'default',
                'deps': gen_target_tree(None),
                'type': 'None',
                'update': False
            }
        else:
            tree = {
                'target': tar,
                'deps': gen_target_tree(tar),
                'type': TARGETS[tar]['type'],
                'update': False
            }
        check_dep_tree(tree)
        execute_tree(tree)

if __name__ == "__main__":
    main()
