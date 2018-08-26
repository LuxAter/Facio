"""
Parser parses the build config files, that are similar to the format of .ini files.
"""
from os import path
from fnmatch import fnmatch

class Parser(object):
    """This class is used to parse the config files"""
    data = {}

    def read(self, file_path):
        """Parses config file and returns json data"""
        if not path.isfile(file_path):
            return {}
        file = []
        current = ''
        with open(file_path) as src:
            file = src.readlines()
        for line in file:
            line = line.strip()
            if line.startswith(';'):
                continue
            elif line.startswith('[') and line.endswith(']'):
                line = line[1:-1]
                current = line
                self.data[current] = {'cmds': []}
                if ';' in line:
                    tmp = line.split(';')
                    self.data[current]['varient'] = tmp[
                        1] if tmp[1] != '' else 'default'
                    self.data[current]['language'] = tmp[
                        2] if len(tmp) >= 3 else 'default'
                else:
                    self.data[current]['varient'] = 'default'
                    self.data[current]['language'] = 'default'
            elif '=' in line:
                self.data[current][line.split('=')[0]] = '='.join(
                    line.split('=')[1:])
            else:
                self.data[current]['cmds'].append(line)
        return self.data

    def is_target(self, target_path, varient=None):
        """Checks if target is viable"""
        for target in self.data:
            if fnmatch(target_path, target.split(';')[0]):
                if varient and self.data[target]['varient'] == varient:
                    return True
                elif varient is None:
                    return True
        return False

    def get_target(self, target_path, varient=None):
        """Gets data for target"""
        for target in self.data:
            if fnmatch(target_path, target.split(';')[0]):
                if varient and self.data[target]['varient'] == varient:
                    return self.data[target]
                elif varient is None and self.data[target]['varient'] == 'default':
                    return self.data[target]
        return {}
