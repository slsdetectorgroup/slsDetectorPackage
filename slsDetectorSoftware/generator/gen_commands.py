import argparse
import os
from pathlib import Path

import yaml

from cpp_codegen.codegen import codegen, if_block, for_block, function, else_block

parser = argparse.ArgumentParser(
    prog='cpp command code generator',
    description='generate cpp code for commands using the commands.yaml file',
)
parser.add_argument('-f', '--format', action='store_true', default=False, dest='format', )

GEN_PATH = Path(__file__).parent
COMMANDS_PATH = GEN_PATH / 'extended_commands.yaml'
commands_config = yaml.unsafe_load(COMMANDS_PATH.open('r'))

codegen.open(GEN_PATH.parent / 'src' / 'Caller.cpp')
# write call function
codegen.write_opening(GEN_PATH / 'Caller.in.cpp')

# iterate over the commands and generate code for each
print(f"[X] found {len(commands_config)} commands")
print('[*] generating code for commands')
for command_name, command in commands_config.items():
    with function('std::string', 'Caller::' + command_name, [('int', 'action')]) as fn:
        codegen.write_line('std::ostringstream os;')

        # print help
        codegen.write_line('// print help')
        with if_block('action == slsDetectorDefs::HELP_ACTION'):
            codegen.write_line(f'os << "Command: {command_name}" << std::endl;')
            codegen.write_line(f'os << R"V0G0N({command["help"]} )V0G0N" << std::endl;')
            codegen.write_line('return os.str();')
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
                    with if_block(f'args.size() == {arg["argc"]}', block=True):
                        # check argument types
                        if 'extra_variables' in arg:
                            for var in arg['extra_variables']:
                                codegen.write_line(f'{var["type"]} {var["name"]} = {var["value"]};')

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
                                f'}} catch (...) {{  throw RuntimeError("Could not convert argument to time::ns");}}')

                        elif 'convert_to_time' in arg and arg['convert_to_time']:
                            codegen.write_line(f'try {{')

                            codegen.write_line(f'StringTo < time::ns > ({", ".join(arg["convert_to_time"]["input"])});')
                            codegen.write_line(
                                f'}} catch (...) {{  throw RuntimeError("Could not convert arguments to time::ns");}}')

                        for i in range(len(arg['input'])):
                            if arg["input_types"][i] in ['std::string', 'time::ns'] or not arg['cast_input'][i]:
                                continue
                            codegen.write_line(f'try {{')
                            codegen.write_line(f'StringTo<{arg["input_types"][i]}>({arg["input"][i]});')
                            codegen.write_line(f'}} catch (...) {{')
                            codegen.write_line(
                                f'  throw RuntimeError("Could not convert argument {i} to {arg["input_types"][i]}");')
                            codegen.write_line(f'}}')
                first = False
        with else_block():
            codegen.write_line(
                f'throw RuntimeError("Invalid action: supported actions are {list(command["actions"].keys())}");')

        # generate code for each action
        codegen.write_line('// generate code for each action')
        codegen.write_line('auto detector_type = det->getDetectorType().squash();')
        for action, action_params in command['actions'].items():

            with if_block(f'action == {codegen.actions_dict[action]}'):
                if 'detectors' in action_params:
                    first = True
                    for detector, detector_params in action_params['detectors'].items():
                        with if_block(f'detector_type == defs::{detector}', elseif=not first):
                            codegen.write_arg(detector_params, action, command_name)

                    else_block().__enter__()

                if not action_params:
                    codegen.write_line(f'throw RuntimeError("detector not supported for action: {action}");')
                else:
                    tmp_args = []
                    if 'args' in action_params:
                        tmp_args = action_params['args']
                    codegen.write_arg(tmp_args, action, command_name)

                if 'detectors' in action_params:
                    else_block().__exit__()

        codegen.write_line('return os.str();')

# close sls namespace
codegen.write_closing()
codegen.close()
print('[X] .cpp code generated')

codegen.write_header(GEN_PATH / 'Caller.in.h', GEN_PATH.parent / 'src' / 'Caller.h', list(commands_config.keys()))
print('[X] header code generated')

if parser.parse_args().format:
    os.system(f'clang-format -i  --style="{{Standard: C++11}}" {GEN_PATH.parent.absolute() / "src" / "Caller.cpp"}')
    os.system(f'clang-format -i  --style="{{Standard: C++11}}" {GEN_PATH.parent.absolute() / "src" / "Caller.h"}')
    print('[X] code formatted')
