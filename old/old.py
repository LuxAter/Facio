#!/usr/bin/python3
"""
This script is used to compile projects or to generate makefiles that will generate the projects.
"""
import configparser
import argparse
import os
import glob
import fnmatch
import subprocess
import json

CONFIG = configparser.ConfigParser()
CMDS = []
COLOR = True
GEN_CMDS = False
DRY = False
VERBOSE = 1
PERC = 0.0
STEP = 0.0


def color(code):
    """Returns the color code if COLOR is set to true, otherwise it returns an empty string."""
    if COLOR:
        return code
    return ""


def error(msg):
    """Prints an error message"""
    print("{}Error: {}{}".format(color("\033[1;31m"), msg, color("\033[0m")))


def warning(msg):
    """Prints a warning message"""
    print("{}Warning: {}{}".format(color("\033[1;33m"), msg, color("\033[0m")))


def note(msg):
    """Prints a note message"""
    print("{}Note: {}{}".format(color("\033[1;34m"), msg, color("\033[0m")))


def in_cmds(dirs, file):
    """Checks if cmd is in the command list"""
    for i, cmd in enumerate(CMDS):
        if cmd['directory'] == dirs and cmd['file'] == file:
            return i
    return None


def scan_deps(target):
    """Scans a target for relevant dependancies, either found from config file of source/include directories, or from a .d files generated from previouse runs."""
    deps = []
    if target in CONFIG.sections(
    ) and "type" in CONFIG[target] and CONFIG[target]['type'] == "lib":
        files = glob.glob(
            "{}/**/*.cpp".format(CONFIG[target]['source_dir']), recursive=True)
        for f in files:
            add = True
            for d in CONFIG[target]['exclude_dir'].split():
                if fnmatch.fnmatch(f, "*/" + d + "/*"):
                    add = False
                    break
            if add:
                deps += [f + ".o"]

    elif target in CONFIG.sections(
    ) and "type" in CONFIG[target] and CONFIG[target]['type'] == "exe":
        dep_file = target + ".d"
        if os.path.isfile(dep_file):
            src = ""
            with open(dep_file, "r") as file:
                src = (' '.join(file.readlines()).replace(
                    '\\', '')).split(':')[1].split()
            deps += src
    elif (target in CONFIG.sections() and "type" in CONFIG[target] and
          CONFIG[target]['type'] == "obj") or target not in CONFIG.sections():
        dep_file = target.replace('.o', '.d')
        if os.path.isfile(dep_file):
            src = ""
            with open(dep_file, "r") as file:
                src = (' '.join(file.readlines()).replace(
                    '\\', '')).split(':')[1].split()
            deps += src
    if target in CONFIG.sections():
        if 'source_file' in CONFIG[target]:
            deps = CONFIG[target]['source_file'].split()
        if 'exclude_file' in CONFIG[target]:
            for f in CONFIG[target]['exclude_file'].split():
                deps = [
                    x for x in deps if not fnmatch.fnmatch(x, "*/" + f)
                    and not fnmatch.fnmatch(x, "*/" + f + ".o")
                ]
    return deps


def handle_response(msg, cmd):
    """Prints the response with color coding"""
    col = ''
    if "fatal error:" in msg:
        print("\033[A[{:3.0f}%] {}{}{}".format(PERC, color("\033[31;1m"), cmd,
                                               color("\033[0m")))
        col = '\033[31;1m'
    elif "error:" in msg:
        print("\033[A[{:3.0f}%] {}{}{}".format(PERC, color("\033[31;1m"), cmd,
                                               color("\033[0m")))
        col = '\033[31;1m'
    elif "warning:" in msg:
        print("\033[A[{:3.0f}%] {}{}{}".format(PERC, color("\033[33;1m"), cmd,
                                               color("\033[0m")))
        col = '\033[33;1m'
    elif "note:" in msg:
        print("\033[A[{:3.0f}%] {}{}{}".format(PERC, color("\033[34;1m"), cmd,
                                               color("\033[0m")))
        col = '\033[34;1m'
    else:
        return
    print(
        color(col) + ">>  " + color("\033[0m") +
        msg.replace('\n', '\n' + color(col) + '>>  ' + color('\033[0m')))


def check_deps(target, deps):
    """Checks the update time on dependencies and determins if target must be compiled"""
    if os.path.isfile(target):
        dest = os.path.getmtime(target)
    else:
        return True
    for dep in deps:
        if os.path.isfile(dep):
            if os.path.getmtime(dep) > dest:
                return True
        else:
            return True
    return False


def Exec(cmd):
    """Runs command if not dry-run, else prints command"""
    if DRY is True:
        print(">>  " + cmd)
        return None
    else:
        return subprocess.run(
            cmd.split(), stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT).stdout.decode('utf-8')


def Clean(target, file):
    if os.path.isfile(file):
        print("[{:3.0f}%] {}{}{}".format(PERC, color("\033[33m"),
                                         "Removing {} object {}".format(
                                             "C++", file), color("\033[0m")))
        Exec("rm {}".format(file))
        if os.path.isfile(file.replace('.o', '.d')):
            Exec("rm {}".format(file.replace('.o', '.d')))


def Build(target, file, flags):
    global COLOR
    deps = scan_deps(file)
    if check_deps(file, deps):
        print("[{:3.0f}%] {}{}{}".format(PERC, color("\033[32m"),
                                         "Building {} object {}".format(
                                             "C++", file), color("\033[0m")))
        ret = Exec("clang++ {} {} -MMD -c {} -o {}".format(
            "-fcolor-diagnostics" if COLOR else '', flags,
            file.replace('.o', ''), file))
        if GEN_CMDS:
            if in_cmds(
                    os.path.dirname(os.path.abspath(file.replace('.o', ''))),
                    os.path.basename(file.replace('.o', ''))) is not None:
                del CMDS[in_cmds(
                    os.path.dirname(os.path.abspath(file.replace('.o', ''))),
                    os.path.basename(file.replace('.o', '')))]
            CMDS.append({
                "arguments": ['c++', '-c', '-MMD'] + flags.split(' ') + [
                    '-o',
                    os.path.abspath(file),
                    os.path.abspath(file.replace('.o', ''))
                ],
                "directory":
                os.path.dirname(
                    os.path.abspath(os.path.join('.', file.replace('.o',
                                                                   '')))),
                "file":
                os.path.basename(file.replace('.o', ''))
            })
        if ret:
            handle_response(ret, "Building {} object {}".format("C++", file))


def LinkLib(target, deps, links):
    print("[{:3.0f}%] {}{}{}".format(PERC, color('\033[36;1m'),
                                     "Linking {} static library {}".format(
                                         "C++", target), color("\033[0m")))
    ret = Exec("ar rvs {} {}".format(target, " ".join(deps)))
    if ret:
        handle_response(ret, "Linking {} static library {}".format(
            "C++", target))


def LinkExe(target, deps, flags, links):
    global COLOR
    print("[{:3.0f}%] {}{}{}".format(PERC, color('\033[36;1m'),
                                     "Linking {} executable {}".format(
                                         "C++", target), color("\033[0m")))
    ret = Exec("clang++ {} {} {} {} -o {}".format(
        "-fcolor-diagnostics"
        if COLOR else '', flags, " ".join(deps), links, target))
    if ret:
        handle_response(ret, "Linking {} executable {}".format("C++", target))


def Target(target):
    global PERC
    global STEP
    print("{}{}{}".format(
        color("\033[1;35m"),
        "Scaning dependencies for target {}".format(target), color("\033[0m")))
    deps = scan_deps(target)
    flags = []
    links = []
    libs = []
    if "include_dir" in CONFIG[target]:
        flags = [
            '-I' + os.path.abspath(os.path.join('.', x))
            for x in CONFIG[target]['include_dir'].split(' ')
        ]
    if "link" in CONFIG[target]:
        links = ('-l' + CONFIG[target]['link'].replace(' ', '-l')).split(' ')
    if "dep" in CONFIG[target]:
        libs = [x for x in CONFIG[target]['dep'].split(' ') if '.a' in x]
        links += libs
    step = STEP / (len(deps) + 2)
    for dep in deps:
        Build(target, dep, " ".join(flags))
        PERC += step
    PERC += step
    if check_deps(target, deps + libs):
        if "type" in CONFIG[target] and CONFIG[target]['type'] == 'lib':
            LinkLib(target, deps, " ".join(links))
        elif "type" in CONFIG[target] and CONFIG[target]['type'] == 'exe':
            LinkExe(target, deps, " ".join(flags), " ".join(links))
        else:
            LinkExe(target, deps, " ".join(flags), " ".join(links))
    PERC += step
    print("[{:3.0f}%] {}".format(PERC, "Built target {}".format(target)))


def CleanTarget(target):
    global PERC
    print("{}{}{}".format(
        color("\033[1;35m"),
        "Scaning dependencies for target {}".format(target), color("\033[0m")))
    deps = scan_deps(target)
    step = STEP / (len(deps) + 2)
    for dep in deps:
        Clean(target, dep)
        PERC += step
    PERC += step
    if os.path.isfile(target):
        if "type" in CONFIG[target] and CONFIG[target]['type'] == 'lib':
            print("[{:3.0f}%] {}{}{}".format(
                PERC, color('\033[36;1m'),
                "Removing {} static library {}".format("C++", target),
                color("\033[0m")))
        else:
            print("[{:3.0f}%] {}{}{}".format(
                PERC, color('\033[36;1m'), "Removing {} executable {}".format(
                    "C++", target), color("\033[0m")))
        Exec("rm {}".format(target))
    PERC += step
    print("[{:3.0f}%] {}".format(PERC, "Cleaned target {}".format(target)))


def main():
    def add_deps(target):
        if 'dep' in CONFIG[target]:
            for tar in CONFIG[target]['dep'].split():
                if tar not in targets:
                    add_deps(tar)
        targets.append(target)

    global CONFIG
    global DRY
    global VERBOSE
    global COLOR
    global GEN_CMDS

    CONFIG.read("config.ini")
    parser = argparse.ArgumentParser(description="Facio build system")
    parser.add_argument(
        "-d", "--dry", action="store_true", help="Dry runs the build system")
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="Enables verbose output")
    parser.add_argument(
        "-g",
        "--gen-cmds",
        action="store_true",
        help="Generates a compile_commands.json")
    parser.add_argument("-m", "--gen-make", action="store_true", help="Generates a makefile for GNU make")
    parser.add_argument(
        "-C",
        "--no-color",
        action="store_true",
        help="Disables colored output")
    parser.add_argument(
        "-c", "--clean", action="store_true", help="Removes build files")
    parser.add_argument(
        "-l",
        "--level",
        type=int,
        default=1,
        help="Sets what level of messages to display")
    parser.add_argument("args", nargs="*", help="Additional arguments")
    args = parser.parse_args()
    COLOR = not args.no_color
    DRY = args.dry
    VERBOSE = args.level
    GEN_CMDS = args.gen_cmds
    targets = []
    options = []

    for arg in args.args:
        if "=" in arg:
            options.append(arg)
        elif arg in CONFIG.sections():
            if arg not in targets:
                add_deps(arg)
        elif arg == "all":
            for tar in CONFIG.sections():
                if ('run' in CONFIG[tar] and CONFIG[tar]['run'] == 'true'
                    ) or 'run' not in CONFIG[tar]:
                    if tar not in targets:
                        add_deps(tar)
        else:
            print("ERR")
    if not targets:
        for tar in CONFIG.sections():
            if ('run' in CONFIG[tar] and
                    CONFIG[tar]['run'] == 'true') or 'run' not in CONFIG[tar]:
                if tar not in targets and tar != 'settings':
                    add_deps(tar)
    global STEP
    global PERC
    global CMDS
    STEP = 100 / len(targets)
    if GEN_CMDS and os.path.isfile('compile_commands.json'):
        with open('compile_commands.json', 'r') as file:
            CMDS = json.load(file)
    for tar in targets:
        if args.clean:
            CleanTarget(tar)
        else:
            Target(tar)
    if GEN_CMDS:
        with open('compile_commands.json', 'w') as file:
            json.dump(CMDS, file)


if __name__ == "__main__":
    main()
