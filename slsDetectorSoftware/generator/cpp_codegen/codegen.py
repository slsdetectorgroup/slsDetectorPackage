class CodeGenerator:
    def __init__(self):
        self.file = None
        self.actions_dict = {
            'GET': 'slsDetectorDefs::GET_ACTION',
            'PUT': 'slsDetectorDefs::PUT_ACTION',
            'READOUT': 'slsDetectorDefs::READOUT_ACTION',
            'HELP': 'slsDetectorDefs::HELP_ACTION',
        }
        self.template_file = None

    def open(self, path):
        self.file = path.open('w')

    def close(self):
        self.file.close()
        self.file = None

    def write_line(self, line):
        self.file.write(line + '\n')

    def write(self, text):
        self.file.write(text)

    def write_opening(self, path):
        """Write the opening file for the caller.cpp file"""
        self.template_file = path.open('r')
        for line in self.template_file:
            if "THIS COMMENT IS GOING TO BE REPLACED BY THE ACTUAL CODE" in line:
                return
            self.file.write(line)

    def write_closing(self):
        """Write the closing file for the caller.cpp file"""
        for line in self.template_file.readlines():
            self.file.write(line)
        self.template_file.close()

    def write_header(self, in_path, out_path, commands):
        """Write the header file for the caller.h file"""
        with out_path.open('w') as fp:
            with in_path.open('r') as fp2:
                for line in fp2:
                    if "THIS COMMENT IS GOING TO BE REPLACED BY THE ACTUAL CODE" in line:
                        map_string = ""
                        for command in commands:
                            map_string += f'{{"{command}", &Caller::{command}}},'
                        statement = f'FunctionMap functions{{{map_string[:-1]}}};'
                        fp.write(statement)
                        continue

                    fp.write(line)


class if_block:
    def __init__(self, condition, elseif=False):
        self.condition = condition
        self.elseif = elseif

    def __enter__(self):
        if self.elseif:
            codegen.write_line('else if (' + self.condition + ') {')
        else:
            codegen.write_line('if (' + self.condition + ') {')
        codegen.write_line('')

    def __exit__(self, *args):
        codegen.write_line('}')
        codegen.write_line('')


class else_block:
    def __init__(self):
        pass

    def __enter__(self):
        codegen.write_line('else {')
        codegen.write_line('')

    def __exit__(self, *args):
        codegen.write_line('}')
        codegen.write_line('')


class for_block:
    def __init__(self, condition):
        self.condition = condition

    def __enter__(self):
        codegen.write_line('for (' + self.condition + ') {')
        codegen.write_line('')

    def __exit__(self, *args):
        codegen.write_line('}')
        codegen.write_line('')


class function:
    def __init__(self, return_type, name, args):
        self.name = name
        self.args = args
        self.return_type = return_type

    def __enter__(self):
        s = ""
        for arg in self.args:
            arg_type, arg_name = arg
            s += arg_type + ' ' + arg_name + ', '
        s = s[:-2]
        codegen.write_line(self.return_type + ' ' + self.name +
                           f'({s}) {{')
        codegen.write_line('')
        return self

    def __exit__(self, *args):
        codegen.write_line('}')
        codegen.write_line('')


codegen = CodeGenerator()
