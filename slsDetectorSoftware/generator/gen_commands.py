import argparse
import os
import subprocess
from pathlib import Path

import yaml

from autocomplete.autocomplete import type_info
from cpp_codegen.codegen import codegen, if_block, for_block, function, else_block
from infer_action.check_infer import check_infer
from autocomplete.autocomplete import type_values

GEN_PATH = Path(__file__).parent

COMMANDS_PATH = GEN_PATH / 'extended_commands.yaml'
DEPRECATED_COMMANDS_PATH = GEN_PATH / 'deprecated_commands.yaml'
CPP_INPUT_PATH = GEN_PATH / 'Caller.in.cpp'
HEADER_INPUT_PATH = GEN_PATH / 'Caller.in.h'
CPP_OUTPUT_PATH = GEN_PATH.parent / 'src' / 'Caller.cpp'
HEADER_OUTPUT_PATH = GEN_PATH.parent / 'src' / 'Caller.h'

INFER_HEADER_INPUT_PATH = GEN_PATH / 'inferAction.in.h'
INFER_CPP_INPUT_PATH = GEN_PATH / 'inferAction.in.cpp'
INFER_HEADER_OUTPUT_PATH = GEN_PATH.parent / 'src' / 'inferAction.h'
INFER_CPP_OUTPUT_PATH = GEN_PATH.parent / 'src' / 'inferAction.cpp'


def generate(
        commands_path=COMMANDS_PATH,
        cpp_input_path=CPP_INPUT_PATH,
        header_input_path=HEADER_INPUT_PATH,
        cpp_output_path=CPP_OUTPUT_PATH,
        header_output_path=HEADER_OUTPUT_PATH,
        infer_header_input_path=INFER_HEADER_INPUT_PATH,
        infer_cpp_input_path=INFER_CPP_INPUT_PATH,
        infer_header_output_path=INFER_HEADER_OUTPUT_PATH,
        infer_cpp_output_path=INFER_CPP_OUTPUT_PATH,

):
    commands_config = yaml.unsafe_load(commands_path.open('r'))
    deprecated_commands_config = yaml.unsafe_load(DEPRECATED_COMMANDS_PATH.open('r'))
    type_dist, non_dist = check_infer(commands=commands_config)

    codegen.open(cpp_output_path)
    # write call function
    codegen.write_opening(cpp_input_path)

    # iterate over the commands and generate code for each
    print(f"[X] found {len(commands_config)} commands")
    print('[*] generating code for commands')
    for command_name, command in commands_config.items():
        if 'is_description' in command and command['is_description']:
            continue
        with function('std::string', 'Caller::' + command['function_alias'], [('int', 'action')]) as fn:
            codegen.write_line('std::ostringstream os;')

            # print help
            codegen.write_line('// print help')
            with if_block('action == slsDetectorDefs::HELP_ACTION'):
                if command["help"].startswith('code:'):
                    codegen.write_line(command["help"].strip('code:'))
                else:
                    codegen.write_line(f'os << R"V0G0N({command["help"]} )V0G0N" << std::endl;')
                    codegen.write_line('return os.str();')

            # check if action and arguments are valid

            codegen.write_line('// check if action and arguments are valid')
            first = True
            for action, action_params in command['actions'].items():

                with if_block(f'action == {codegen.actions_dict[action]}', elseif=not first):

                    check_argc = True
                    for arg in action_params['args']:
                        if arg['argc'] == -1:
                            check_argc = False
                            break
                    # check number of arguments
                    condition = "1" if check_argc else "0"

                    if check_argc:
                        for arg in action_params['args']:
                            condition += f' && args.size() != {arg["argc"]}'

                    with if_block(condition):
                        codegen.write_line(f'throw RuntimeError("Wrong number of arguments for action {action}");')

                    for arg in action_params['args']:
                        if not check_argc:
                            continue
                        with if_block(f'args.size() == {arg["argc"]}'):
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

                                codegen.write_line(
                                    f'StringTo < time::ns > ({", ".join(arg["convert_to_time"]["input"])});')
                                codegen.write_line(
                                    f'}} catch (...) {{  throw RuntimeError("Could not convert arguments to time::ns");}}')

                            for i in range(len(arg['input'])):
                                if not arg['cast_input'][i]:
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
                    f'throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions are {list(command["actions"].keys())}");')

            # generate code for each action
            codegen.write_line('// generate code for each action')
            for action, action_params in command['actions'].items():
                if 'detectors' in action_params:
                    codegen.write_line('auto detector_type = det->getDetectorType().squash();')

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
    deprecated_commands = []
    codegen.write_header(header_input_path, header_output_path, commands_config, deprecated_commands_config)
    print('[X] header code generated')

    
    codegen.write_infer_header(infer_header_input_path, infer_header_output_path, commands_config) #TODO: add deprecated commands
    print('[X] infer header code generated')
    codegen.open(infer_cpp_output_path)

    codegen.write_infer_cpp(infer_cpp_input_path, infer_cpp_output_path, commands_config, non_dist, type_dist)
    codegen.close()
    print('[X] infer cpp code generated')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='generate c++ code for cli commands from the commands.yaml file',
    )
    parser.add_argument('-f', '--format', action='store_true', default=False, dest='format',
                        help='format header and cpp file using clang-format')
    parser.add_argument('-p', '--parse', action='store_true', default=False, dest='parse',
                        help='parse the commands.yaml file into extended_commands.yaml')
    parser.add_argument('-c', '--check', action='store_true', default=False, dest='check',
                        help='check missing commands')
    parser.add_argument('-g', '--generate', action='store_true', default=False, dest='generate', help='generate code (C++ or bash if -a is used)')
    parser.add_argument('-a', '--autocomplete', action='store_true', default=False, dest='autocomplete',
                        help='print bash autocomplete values')
    cli_args = parser.parse_args()

    if cli_args.autocomplete:
        from autocomplete.autocomplete import generate_type_values, generate_bash_autocomplete
        if cli_args.generate:
            generate_bash_autocomplete()
            print('[X] bash autocomplete generated')
            generate_bash_autocomplete(
                output_path=Path(__file__).parent / 'autocomplete' / 'zsh_autocomplete.sh',
                input_path=Path(__file__).parent / 'autocomplete' / 'zsh_autocomplete.in.sh'
            )
            print('[X] zsh autocomplete generated')
            exit(0)
        else:
            ret = generate_type_values()
            print(ret)
            exit(0)

    if cli_args.check:
        from commands_parser.commands_parser import command_parser

        commands_config = yaml.unsafe_load(COMMANDS_PATH.open('r'))

        # infer action based on number of arguments and types
        type_dist, non_dist = check_infer(commands=commands_config)

        command_parser.verify_format()
        command_parser.parse_all_commands()
        # generate list of commands found in sls_detector_get
        glist_path = GEN_PATH / 'glist'
        ret = subprocess.run([f"sls_detector_get list | tail -n +2 | sort > {glist_path.absolute()}"], shell=True,
                             capture_output=True, check=True)
        if ret.stderr != b'':
            print('[!] glist generation failed and glist not found')
            exit(1)

        if not COMMANDS_PATH.exists():
            print('[!] extended_commands.yaml not found')
            exit(1)
        detglist = set(command['command_name'] for __, command in commands_config.items())
        detglist.add('free')
        detglist.add('list')

        g_path = GEN_PATH / 'glist'
        if not g_path.exists():
            print('[!] glist not found')
            exit(1)
        glist = set(g_path.read_text().split('\n'))
        if "" in glist:
            glist.remove("")
        if "" in detglist:
            detglist.remove("")

        not_found = set()
        for command in glist:
            if command not in detglist:
                not_found.add(command)
        print()
        if len(not_found) > 0:
            print(f'[!] found {len(not_found)} missing')
            print(f"not_found: {not_found}")
        else:
            print(f'[X] found no missing commands')

        for command in detglist:
            if command not in glist:
                print(f'[!] command {command} found in commands.yaml but not found in g list')

        exit(0)

    if cli_args.parse:
        from commands_parser.commands_parser import command_parser

        command_parser.verify_format()
        command_parser.parse_all_commands()

    if cli_args.generate:
        generate()

    if cli_args.format:
        files = [CPP_OUTPUT_PATH, HEADER_OUTPUT_PATH, INFER_HEADER_OUTPUT_PATH, INFER_CPP_OUTPUT_PATH]
        for file in files:
            os.system(f'clang-format -i  {file.absolute()}')
            #os.system(f'clang-format -i  --style="{{Standard: C++11}}" {file.absolute()}')
        print('[X] code formatted')
