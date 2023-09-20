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
                    if "THIS COMMENT IS GOING TO BE REPLACED BY THE ACTUAL CODE (1)" in line:
                        for command in commands:
                            fp.write(f'std::string {command}(int action);\n')
                        continue
                    if "THIS COMMENT IS GOING TO BE REPLACED BY THE ACTUAL CODE (2)" in line:
                        map_string = ""
                        for command in commands:
                            map_string += f'{{"{command}", &Caller::{command}}},'
                        statement = f'FunctionMap functions{{{map_string[:-1]}}};'
                        fp.write(statement)
                        continue

                    fp.write(line)

    def write_arg(self, args, action):
        # prepare input arguments list
        for arg in args:
            with if_block(f'args.size() == {arg["argc"]}'):
                if 'separate_time_units' in arg and arg['separate_time_units']:
                    self.write_line(f'std::string tmp_time({arg["separate_time_units"]["input"]});')
                    self.write_line(f'std::string {arg["separate_time_units"]["output"][1]}'
                                    f' = RemoveUnit(tmp_time);')
                    self.write_line(f'auto {arg["separate_time_units"]["output"][0]} = '
                                    f'StringTo < time::ns > (tmp_time,'
                                    f' {arg["separate_time_units"]["output"][1]});')
                if 'convert_to_time' in arg and arg['convert_to_time']:
                    self.write_line(f'auto {arg["convert_to_time"]["output"]} = '
                                    f'StringTo < time::ns > ({", ".join(arg["convert_to_time"]["input"])});')
                input_arguments = []
                for i in range(len(arg['input'])):
                    if arg["input_types"][i] not in ['time::ns', 'std::string']:
                        self.write_line(
                            f'auto arg{i} = StringTo<{arg["input_types"][i]}>({arg["input"][i]});')
                        input_arguments.append(f'arg{i}')
                    else:
                        input_arguments.append(arg["input"][i])
                if 'require_det_id' in arg and arg['require_det_id']:
                    input_arguments.append("std::vector<int>{ det_id }")

                input_arguments = ", ".join(input_arguments)
                # call function
                if action == 'GET':
                    self.write_line(f'auto t = det->{arg["function"]}({input_arguments});')
                else:
                    self.write_line(f'det->{arg["function"]}({input_arguments});')

                output_args = []
                for output in arg['output']:
                    output_args.append(output)
                # if len(output_args) > 0:
                #     self.write_line(f"os << OutString(" + ", ".join(output_args) + ") << '\\n';")
                if len(output_args) > 0:
                    self.write_line(f"os << {'<< '.join(output_args)} << '\\n';")


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
    def __init__(self, return_type, name, args: list[tuple[str, str]]):
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
