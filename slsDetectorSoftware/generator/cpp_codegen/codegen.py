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
            if "THIS COMMENT TO BE REPLACED BY THE ACTUAL CODE" in line:
                return
            self.file.write(line)

    def write_closing(self):
        """Write the closing file for the caller.cpp file"""
        for line in self.template_file.readlines():
            self.file.write(line)
        self.template_file.close()

    def write_header(self, in_path, out_path, commands, deprecated_commands):
        """Write the header file for the caller.h file"""
        with out_path.open('w') as fp:
            with in_path.open('r') as fp2:
                for line in fp2:
                    if "THIS COMMENT TO BE REPLACED BY THE ACTUAL CODE (1)" in line:
                        for command_name, command in commands.items():
                            if 'duplicate_function' in command and command['duplicate_function']:
                                continue
                            fp.write(f'std::string {command["function_alias"]}(int action);\n')
                        continue
                    if "THIS COMMENT TO BE REPLACED BY THE ACTUAL CODE (2)" in line:
                        map_string = ''
                        for command_name, command in commands.items():
                            map_string += f'{{"{command_name}", &Caller::{command["function_alias"]}}},'
                        fp.write(map_string[:-1] + '\n')
                        continue

                    if "THIS COMMENT TO BE REPLACED BY THE ACTUAL CODE (3)" in line:
                        for key, value in deprecated_commands.items():
                            fp.write(f'{{"{key}", "{value}"}},\n')
                        continue

                    fp.write(line)

    def write_infer_header(self, in_path, out_path, commands):
        """Write the header file for the inferAction.h file"""
        with out_path.open('w+') as fp:
            with in_path.open('r') as fp2:
                for line in fp2:
                    if "THIS COMMENT TO BE REPLACED BY THE ACTUAL CODE (1) - DO NOT REMOVE" in line:
                        for command_name, command in commands.items():
                            if 'duplicate_function' in command and command['duplicate_function']:
                                continue

                            fp.write(f'int {command["function_alias"]}();\n')
                        continue
                    if "THIS COMMENT TO BE REPLACED BY THE ACTUAL CODE (2) - DO NOT REMOVE" in line:
                        map_string = ''
                        for command_name, command in commands.items():
                            map_string += f'{{"{command_name}", &InferAction::{command["function_alias"]}}},'
                        fp.write(map_string[:-1] + '\n')
                        continue
                    fp.write(line)

    def write_infer_cpp(self, in_path, out_path, commands, non_dist, type_dist):
        """Write the source file for the inferAction.cpp file"""
        with in_path.open('r') as fp2:
            for line in fp2:
                if "THIS COMMENT TO BE REPLACED BY THE ACTUAL CODE (1) - DO NOT REMOVE" in line:
                    for command_name, command in commands.items():
                        if 'duplicate_function' in command and command['duplicate_function']:
                            continue

                        with function('int', f"InferAction::{command['function_alias']}", []) as f:
                            if (command_name, -1) in non_dist| type_dist:
                                self.write_line(
                                    f'throw RuntimeError("sls_detector is disabled for command: {command_name}. Use sls_detector_get or sls_detector_put");')
                            elif not command['infer_action']:
                                self.write_line('throw RuntimeError("infer_action is disabled");')
                            else:
                                checked_argcs = set()
                                for action, action_params in command['actions'].items():
                                    for arg in action_params['args']:
                                        if arg['argc'] in checked_argcs:
                                            continue
                                        checked_argcs.add(arg['argc'])
                                        with if_block(f'args.size() == {arg["argc"]}'):
                                            # check if this argc is not distinguishable
                                            if (command_name, arg["argc"]) in non_dist | type_dist:
                                                self.write_line(
                                                    f'throw RuntimeError("sls_detector is disabled for command: {command_name} with number of arguments {arg["argc"]}. Use sls_detector_get or sls_detector_put");')
                                            else:
                                                self.write_line(f'return {self.actions_dict[action]};')
                                with else_block():
                                    self.write_line(
                                        'throw RuntimeError("Could not infer action: Wrong number of arguments");')
                    continue
                self.write_line(line)

    def write_check_arg(self):
        pass

    def write_arg(self, args, action, command_name):
        for arg in args:
            if arg['argc'] != -1:
                if_block(f'args.size() == {arg["argc"]}',).__enter__()
            if 'pattern_command' in arg and arg['pattern_command']:
                self.write_line(f'int level = -1, iArg = 0, '
                                f'nGetArgs = {arg["pattern_command"]["nGetArgs"]},'
                                f' nPutArgs = {arg["pattern_command"]["nPutArgs"]};\nGetLevelAndUpdateArgIndex(action, '
                                f'"{arg["pattern_command"]["command_name"]}", level, iArg, nGetArgs,nPutArgs);'
                                )

            if 'extra_variables' in arg:
                for var in arg['extra_variables']:
                    codegen.write_line(f'{var["type"]} {var["name"]} = {var["value"]};')
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
            if 'exceptions' in arg:
                for exception in arg['exceptions']:
                    self.write_line(f'if ({exception["condition"]}) {{ throw RuntimeError({exception["message"]}); }}')
            if 'check_det_id' in arg and arg['check_det_id']:
                self.write_line(
                    f'if (det_id != -1) {{ throw RuntimeError("Cannot execute {command_name} at module level"); }} '
                )
            # only used for 3 commands :(
            if 'ctb_output_list' in arg:
                self.write_line(f"""
                    std::string suffix = " {arg['ctb_output_list']['suffix']}";                                        
                    auto t = det->{arg['ctb_output_list']['GETFCNLIST']}();""")
                if arg['ctb_output_list']['GETFCNNAME'] != '':
                    self.write_line(f"""
                    auto names = det->{arg['ctb_output_list']['GETFCNNAME']}();                                    
                    auto name_it = names.begin();""")
                self.write_line("os << '[';")
                with if_block(f't.size() > 0'):
                    self.write_line(f"""
                    auto it = t.cbegin();                                              
                    os << ToString({arg['ctb_output_list']['printable_name']}) << ' ';                                 
                    os << OutString(det->{arg['ctb_output_list']['GETFCN']}(*it++, std::vector<int>{{det_id}}))<< suffix;                                                      
                    while (it != t.cend()) {{                                         
                        os << ", " << ToString({arg['ctb_output_list']['printable_name']}) << ' ';                     
                        os << OutString(det->{arg['ctb_output_list']['GETFCN']}(*it++, std::vector<int>{{det_id}}))<< suffix;
                    }}                                                                  
                    """)
                self.write_line('os << "]\\n";')
                if arg['argc'] != -1:
                    if_block().__exit__()

                return

            for i in range(len(arg['input'])):
                if arg['cast_input'][i]:
                    self.write_line(
                        f'auto arg{i} = StringTo<{arg["input_types"][i]}>({arg["input"][i]});')
                    input_arguments.append(f'arg{i}')
                else:
                    input_arguments.append(arg["input"][i])
            if 'require_det_id' in arg and arg['require_det_id']:
                if 'convert_det_id' in arg and arg['convert_det_id']:
                    input_arguments.append("std::vector<int>{ det_id }")
                else:
                    input_arguments.append("det_id")

            input_arguments = ", ".join(input_arguments)
            # call function
            if arg["function"]:
                if arg['store_result_in_t']:
                    self.write_line(f'auto t = det->{arg["function"]}({input_arguments});')
                else:
                    self.write_line(f'det->{arg["function"]}({input_arguments});')
            else:
                pass #We have no function so skip block

            output_args = []
            for output in arg['output']:
                output_args.append(output)
            if len(output_args) > 0:
                self.write_line(f"os << {'<< '.join(output_args)} << '\\n';")

            if arg['argc'] != -1:
                if_block().__exit__()


class if_block:
    def __init__(self, condition="", elseif=False):
        self.condition = condition
        self.elseif = elseif
        self.block = False

    def __enter__(self):
        if self.elseif:
            codegen.write_line('else if (' + self.condition + ') {')
        else:
            codegen.write_line('if (' + self.condition + ') {')
        if self.block:
            codegen.write_line('{\n')

    def __exit__(self, *args):
        codegen.write_line('}')
        if self.block:
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
