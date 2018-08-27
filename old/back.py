#!/usr/bin/python3
"""
This script is used to compile projects, or to generate compile scrips.
"""

import sys
import os
from fnmatch import fnmatch
import re
import glob

import facioparser

PARSER = facioparser.Parser()

def rep_ext(text, ext):
    if '.' in text:
        return '.'.join(text.split('.')[:-1]) + ext
    else:
        return text + ext

class Target(object):
    data = {
        'target': '',
        'msg': '',
        'msg_color': '0',
        'source': '',
        'cmd': '',
        'pre_cmd': '',
        'post_cmd': '',
        'build_dir': '.'
    }
    deps = []
    options = {}

    def __init__(self, section):
        self.deps = []
        self.options = {}
        for key in self.data:
            if key in section:
                self.data[key] = section[key]
        if 'dep' in section:
            self.deps += section['dep'].split()
        if 'deps' in section:
            self.deps += section['deps'].split()
        if 'source' in section:
            self.deps += section['source'].split()
        if 'source_dir' in section:
            for sdir in section['source_dir'].split():
                files = glob.glob(
                    "{}/**/*.cpp".format(sdir), recursive=True)
                self.deps += [x + '.o' for x in files]
        if 'exclude_dir' in section:
            for edir in section['exclude_dir'].split():
                self.deps = [
                    x for x in self.deps
                    if not fnmatch(x, '*/{}/*'.format(edir))
                ]
        if 'exclude_file' in section:
            for ef in section['exclude_file'].split():
                self.deps = [x for x in self.deps if not fnmatch(x, '*/{}'.format(ef)) and not fnmatch(x, '*/{}'.format(ef + '.o'))]
        print(self.data)
        print(self.deps)

    def __repr__(self):
        return "\033[{}m{}\033[0m".format(self.data['msg_color'],
                                          self.data['msg'])

    def gen_dep_tree(self):
        tree = {}
        dep_file = []
        if os.path.isfile(os.path.join(self.data['build_dir'], self.data['target'] + '.d')):
            print("HI")
        for dep in self.deps + dep_file:
            if PARSER.is_target(dep):
                tree[dep] = Target(PARSER.get_target(dep)).gen_dep_tree()
        return tree

    def check_deps(self, target):
        regex = re.compile(self.data['target'].replace('*', '(.*)'))
        if '*' in self.data['target']:
            base = regex.match(target).group(1)
            deps = [x.replace('*', base) for x in self.deps]
        else:
            base = None
            deps = self.deps
        deps = [os.path.join(self.data['build_dir'], x) for x in deps]
        print(deps)

def object_target(target):
    pass


def main():
    PARSER.read("config.ini")
    if len(sys.argv) > 1:
        if sys.argv[1].startswith('c:'):
            if PARSER.is_target(sys.argv[1], 'clean'):
                target = Target(PARSER.get_target(sys.argv[1], 'clean'))
                print(target)
        else:
            if PARSER.is_target(sys.argv[1]):
                target = Target(PARSER.get_target(sys.argv[1]))
                # target.check_deps(sys.argv[1])
                print(target.gen_dep_tree())


if __name__ == "__main__":
    main()
