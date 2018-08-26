#!/usr/bin/python3
"""
This script is used to compile projects, or to generate compile scrips.
"""

import sys
import fnmatch

import facioparser


class Target(object):
    data = {
        'target': '',
        'msg': '',
        'msg_color': '0',
        'dep': [],
        'cmd': '',
        'pre_cmd': '',
        'post_cmd': '',
        'build_dir': '.'
    }
    options = {}

    def __init__(self, section):
        for key in self.data:
            if key == 'dep' and key in section:
                self.data[key] = section[key].split()
            elif key in section:
                self.data[key] = section[key]
        print(self.data)

    def __repr__(self):
        return "\033[{}m{}\033[0m".format(self.data['msg_color'],
                                          self.data['msg'])

    def check_deps(self, target):
        print(fnmatch.translate(self.data['target']))
        # regex = re.compile(fnmatch.translate(self.data['target']))
        # if any(reg in self.data['target'] for reg in ('*', '?'))



def main():
    parser = facioparser.Parser()
    targets = parser.read("config.ini")
    if len(sys.argv) > 1:
        if sys.argv[1].startswith('c:'):
            if parser.is_target(sys.argv[1], 'clean'):
                target = Target(parser.get_target(sys.argv[1], 'clean'))
                print(target)
        else:
            if parser.is_target(sys.argv[1]):
                target = Target(parser.get_target(sys.argv[1]))
                target.check_deps(sys.argv[1])
                print(target)


if __name__ == "__main__":
    main()
