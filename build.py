#!/usr/bin/python3
"""
This script is used to compile projects, or to generate compile scrips.
"""

import sys

import facioparser

class Target(object):
    default_data = {'msg': '', 'msg_color': '0', 'deps': [], 'cmd': '','pre_cmd': '', 'post_cmd': ''}
    options = {}

    def __init__(self, section):
        for key in self.default_data:
            if key in section:
                self.default_data[key] = section[key]
        print(self.default_data)

    def __repr__(self):
        return "\033[{}m{}\033[0m".format(self.default_data['msg_color'],
                                          self.default_data['msg'])


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
                print(target)

if __name__ == "__main__":
    main()
