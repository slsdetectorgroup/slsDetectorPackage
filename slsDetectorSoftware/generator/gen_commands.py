from pathlib import Path

import yaml

from cpp_codegen.codegen import codegen, if_block, for_block, function, else_block

GEN_PATH = Path(__file__).parent
COMMANDS_PATH = GEN_PATH / 'commands.yaml'
commands_config = yaml.unsafe_load(COMMANDS_PATH.open('r'))

codegen.open(GEN_PATH.parent / 'src' / 'Caller.cpp')
# write call function
codegen.write_opening(GEN_PATH / 'Caller.in.cpp')

# iterate over the commands and generate code for each
for command_name, command in commands_config.items():
    with function('std::string', 'Caller::' + command_name, [('int', 'action')]) as fn:
        codegen.write_line('std::ostringstream os;')

        # infer action based on number of arguments
        codegen.write_line('// infer action based on number of arguments')
        with if_block('action == -1'):
            if command['infer_action']:
                first = True
                for action, action_params in command['actions'].items():
                    for arg in action_params['args']:
                        with if_block(f'args.size() == {arg["argc"]}', elseif=not first):
                            codegen.write_line(f'std::cout << "inferred action: {action}" << std::endl;')
                            codegen.write_line(f'action = {codegen.actions_dict[action]};')
                        first = False
                with else_block():
                    codegen.write_line('throw RuntimeError("Could not infer action: Wrong number of arguments");')
            else:
                codegen.write_line('throw RuntimeError("infer_action is disabled");')

        # check if action and arguments are valid
        codegen.write_line('// check if action and arguments are valid')
        first = True
        for action, action_params in command['actions'].items():
            with if_block(f'action == {codegen.actions_dict[action]}', elseif=not first):

                # check number of arguments
                condition = ""
                for arg in action_params['args']:
                    condition += f'args.size() != {arg["argc"]} && '
                else:
                    condition = condition[:-4]

                with if_block(condition):
                    codegen.write_line(f'throw RuntimeError("Wrong number of arguments for action {action}");')
                for arg in action_params['args']:
                    with if_block(f'args.size() == {arg["argc"]}'):
                        # check argument types

                        if 'separate_time_units' in arg and arg['separate_time_units']:
                            codegen.write_line(f'try {{')
                            # TODO: refactor this repeating code
                            codegen.write_line(f'std::string tmp_time({arg["separate_time_units"]["input"]});')
                            codegen.write_line(f'std::string {arg["separate_time_units"]["output"][1]}'
                                               f' = RemoveUnit(tmp_time);')
                            codegen.write_line(f'auto {arg["separate_time_units"]["output"][0]} = '
                                               f'StringTo < time::ns > (tmp_time,'
                                               f' {arg["separate_time_units"]["output"][1]});')
                            codegen.write_line(
                                f'}} catch (...) {{  throw RuntimeError("Could not convert argument {i} to time::ns");}}')

                        elif 'convert_to_time' in arg and arg['convert_to_time']:
                            codegen.write_line(f'try {{')

                            codegen.write_line(f'StringTo < time::ns > ({", ".join(arg["convert_to_time"]["input"])});')
                            codegen.write_line(
                                f'}} catch (...) {{  throw RuntimeError("Could not convert argument {i} to time::ns");}}')

                        for i in range(len(arg['input'])):
                            if arg["input_types"][i] in ['time::ns']:
                                continue
                            codegen.write_line(f'try {{')
                            codegen.write_line(f'StringTo<{arg["input_types"][i]}>({arg["input"][i]});')
                            codegen.write_line(f'}} catch (...) {{')
                            codegen.write_line(
                                f'  throw RuntimeError("Could not convert argument {i} to {arg["input_types"][i]}");')
                            codegen.write_line(f'}}')
                first = False
        with else_block():
            codegen.write_line('throw RuntimeError("Invalid action");')

        # generate code for each action
        codegen.write_line('// generate code for each action')
        for action, action_params in command['actions'].items():
            with if_block(f'action == {codegen.actions_dict[action]}'):
                # prepare input arguments list
                for arg in action_params['args']:
                    with if_block(f'args.size() == {arg["argc"]}'):
                        if 'separate_time_units' in arg and arg['separate_time_units']:
                            codegen.write_line(f'std::string tmp_time({arg["separate_time_units"]["input"]});')
                            codegen.write_line(f'std::string {arg["separate_time_units"]["output"][1]}'
                                               f' = RemoveUnit(tmp_time);')
                            codegen.write_line(f'auto {arg["separate_time_units"]["output"][0]} = '
                                               f'StringTo < time::ns > (tmp_time,'
                                               f' {arg["separate_time_units"]["output"][1]});')
                        if 'convert_to_time' in arg and arg['convert_to_time']:
                            codegen.write_line(f'auto {arg["convert_to_time"]["output"]} = '
                                               f'StringTo < time::ns > ({", ".join(arg["convert_to_time"]["input"])});')
                        input_arguments = []
                        for i in range(len(arg['input'])):
                            if arg["input_types"][i] not in ['time::ns', 'std::string']:
                                codegen.write_line(
                                    f'auto arg{i} = StringTo<{arg["input_types"][i]}>({arg["input"][i]});')
                                input_arguments.append(f'arg{i}')
                            else:
                                input_arguments.append(arg["input"][i])
                        if 'require_det_id' in arg and arg['require_det_id']:
                            input_arguments.append("std::vector<int>{ det_id }")

                        input_arguments = ", ".join(input_arguments)
                        # call function
                        if action == 'GET':
                            codegen.write_line(f'auto t = det->{arg["function"]}({input_arguments});')
                        else:
                            codegen.write_line(f'det->{arg["function"]}({input_arguments});')

                        output_args = []
                        for output in arg['output']:
                            output_args.append(output)
                        # if len(output_args) > 0:
                        #     codegen.write_line(f"os << OutString(" + ", ".join(output_args) + ") << '\\n';")
                        if len(output_args) > 0:
                            codegen.write_line(f"os << {'<< '.join(output_args)} << '\\n';")
        codegen.write_line('return os.str();')

# close sls namespace
codegen.write_closing()
codegen.close()

codegen.write_header(GEN_PATH / 'Caller.in.h', GEN_PATH.parent / 'src' / 'Caller.h', list(commands_config.keys()))
