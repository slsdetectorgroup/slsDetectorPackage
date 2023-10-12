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


def generate(
        COMMANDS_PATH=GEN_PATH / 'extended_commands.yaml',
        CPP_INPUT_PATH=GEN_PATH / 'Caller.in.cpp',
        HEADER_INPUT_PATH=GEN_PATH / 'Caller.in.h',
        CPP_OUTPUT_PATH=GEN_PATH.parent / 'src' / 'Caller.cpp',
        HEADER_OUTPUT_PATH=GEN_PATH.parent / 'src' / 'Caller.h',
):
    commands_config = yaml.unsafe_load(COMMANDS_PATH.open('r'))

    codegen.open(CPP_OUTPUT_PATH)
    # write call function
    codegen.write_opening(CPP_INPUT_PATH)

    # iterate over the commands and generate code for each
    print(f"[X] found {len(commands_config)} commands")
    print('[*] generating code for commands')
    for command_name, command in commands_config.items():
        with function('std::string', 'Caller::' + command['function_alias'], [('int', 'action')]) as fn:
            codegen.write_line('std::ostringstream os;')

            # print help
            codegen.write_line('// print help')
            with if_block('action == slsDetectorDefs::HELP_ACTION'):
                codegen.write_line(f'os << "Command: {command_name}" << std::endl;')
                codegen.write_line(f'os << R"V0G0N({command["help"]} )V0G0N" << std::endl;')
                codegen.write_line('return os.str();')

            # infer action based on number of arguments and types
            type_dist, non_dist = check_infer(commands=commands_config)
            codegen.write_line('// infer action based on number of arguments')

            with if_block('action == -1'):
                if (command_name, -1) in non_dist:
                    codegen.write_line(f'throw RuntimeError("Cannot infer action for command: {command_name}");')
                elif not command['infer_action']:
                    codegen.write_line('throw RuntimeError("infer_action is disabled");')
                else:
                    first = True
                    checked_argcs = set()

                    for action, action_params in command['actions'].items():
                        for arg in action_params['args']:
                            if arg['argc'] in checked_argcs:
                                continue
                            checked_argcs.add(arg['argc'])

                            with if_block(f'args.size() == {arg["argc"]}', elseif=not first):
                                # check if this argc is not distinguishable
                                if (command_name, arg["argc"]) in non_dist:
                                    codegen.write_line(
                                        f'throw RuntimeError("Cannot infer action for command: {command_name}.");')

                                # check if this argc is only distinguishable by type
                                elif (command_name, arg["argc"]) in type_dist:
                                    get_arg_types, put_arg_types = type_dist[(command_name, arg["argc"])]
                                    for index, (get_type, put_type) in enumerate(zip(get_arg_types, put_arg_types)):
                                        if get_type != put_type:
                                            break
                                    # check if we are able to convert to get_type => action is get
                                    # else check if we are able to convert to put_type => action is put
                                    # else throw error

                                    codegen.write_line(f'bool can_convert_to_get = true;')
                                    if type_info(get_type) == 'special':
                                        codegen.write_line(f'can_convert_to_get = false;')

                                        values = type_values[get_type]
                                        conditions = [f"args[{index}] == \"{value}\"" for value in values]
                                        condition = " || ".join(conditions)
                                        with if_block(condition):
                                            codegen.write_line(f'action = {codegen.actions_dict["GET"]};')
                                    elif get_type == 'std::string':
                                        # if type is std::string we can always convert to it
                                        pass
                                    elif type_info(get_type) == 'enum' or type_info(get_type) == 'base':
                                        codegen.write_line(f'try {{')
                                        codegen.write_line(f'StringTo<{get_type}>(args[{index}]);')
                                        codegen.write_line(f'}} catch (...) {{')
                                        codegen.write_line(f'can_convert_to_get = false;')
                                        codegen.write_line(f'}}')

                                    codegen.write_line(f'bool can_convert_to_put = true;')
                                    if type_info(put_type) == 'special':
                                        codegen.write_line(f'can_convert_to_put = false;')

                                        values = type_values[put_type]
                                        conditions = [f"args[{index}] == \"{value}\"" for value in values]
                                        condition = " || ".join(conditions)
                                        with if_block(condition):
                                            codegen.write_line(f'action = {codegen.actions_dict["PUT"]};')


                                    elif put_type == 'std::string':
                                        # if type is std::string we can always convert to it
                                        pass
                                    elif type_info(get_type) == 'enum' or type_info(get_type) == 'base':
                                        codegen.write_line(f'try {{')
                                        codegen.write_line(f'StringTo<{put_type}>(args[{index}]);')
                                        codegen.write_line(f'}} catch (...) {{')
                                        codegen.write_line(f'can_convert_to_put = false;')
                                        codegen.write_line(f'}}')

                                    # Todo: fix duplicated code
                                    if get_type == 'std::string':
                                        with if_block('can_convert_to_put'):
                                            codegen.write_line(f'action = {codegen.actions_dict["PUT"]};')
                                        with if_block('can_convert_to_get', elseif=True):
                                            codegen.write_line(f'action = {codegen.actions_dict["GET"]};')
                                    else:
                                        with if_block('can_convert_to_get'):
                                            codegen.write_line(f'action = {codegen.actions_dict["GET"]};')
                                        with if_block('can_convert_to_put', elseif=True):
                                            codegen.write_line(f'action = {codegen.actions_dict["PUT"]};')
                                    with else_block():
                                        codegen.write_line(f'throw RuntimeError("Could not infer action");')
                                else:
                                    codegen.write_line(f'action = {codegen.actions_dict[action]};')


                            first = False

                    with else_block():
                        codegen.write_line('throw RuntimeError("Could not infer action: Wrong number of arguments");')

            with if_block(f'action == {codegen.actions_dict["PUT"]}'):
                codegen.write_line(f'std::cout << "inferred action: PUT" << std::endl;')
            with if_block(f'action == {codegen.actions_dict["GET"]}', elseif=True):
                codegen.write_line(f'std::cout << "inferred action: GET" << std::endl;')
            with else_block():
                codegen.write_line(f'throw RuntimeError("Could not infer action");')

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
    codegen.write_header(HEADER_INPUT_PATH, HEADER_OUTPUT_PATH,
                         [(command['command_name'], command['function_alias']) for command_name, command in
                          commands_config.items()])
    print('[X] header code generated')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='generate cpp code for commands from the extended_commands.yaml file',
    )
    parser.add_argument('-f', '--format', action='store_true', default=False, dest='format',
                        help='format header and cpp file using clang-format')
    parser.add_argument('-p', '--parse', action='store_true', default=False, dest='parse',
                        help='parse the commands.yaml file into extended_commands.yaml')
    parser.add_argument('-c', '--check', action='store_true', default=False, dest='check',
                        help='check missing commands')
    cli_args = parser.parse_args()

    if cli_args.check:
        from commands_parser.commands_parser import command_parser

        command_parser.verify_format()
        command_parser.parse_all_commands()
        # generate list of commands found in sls_detector_get
        ret = subprocess.run(["sls_detector_get list | tail -n +2 | sort > glist"], shell=True, capture_output=True,
                             check=True)
        glist_path = GEN_PATH / 'glist'
        if ret.stderr != b'':
            print('[!] glist generation failed and glist not found')
            exit(1)

        commands_path = GEN_PATH / 'extended_commands.yaml'
        if not commands_path.exists():
            print('[!] extended_commands.yaml not found')
            exit(1)
        commands_config = yaml.unsafe_load(commands_path.open('r'))
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
        print(f'[X] found {len(not_found)} missing commands from detglist')
        print(not_found)

        for command in detglist:
            if command not in glist:
                print(f'[!] command {command} found in detglist but not found in glist')

        # check for very special functions
        very_special_functions_path = GEN_PATH / 'very_special_functions.txt'
        if not very_special_functions_path.exists():
            print('[!] very_special_functions.txt not found')
            exit(1)
        very_special_functions = very_special_functions_path.read_text().split('\n')
        very_special_commands = set()
        for command in very_special_functions:
            command = command.split(' ')[0]
            if command.startswith('#'):
                continue
            if command != '' and command not in glist:
                print(f'[!] very special command {command} not found in glist')
            else:
                very_special_commands.add(command)
        if "" in very_special_commands:
            very_special_commands.remove("")

        if not_found - very_special_commands:
            print('[!] some commands are missing from very_special_functions.txt')
            print(not_found - very_special_commands)
        else:
            print('[X] all missing commands are present in very_special_functions.txt')

        if set(detglist).intersection(very_special_commands):
            print('[!] some commands are present in both detglist and very_special_functions.txt')
            print(set(detglist).intersection(very_special_commands))

        if glist != detglist.union(very_special_commands):
            print('[!] not all commands from glist are present in detglist and very_special_functions.txt')
        print()
        print(f'Total g commands: {len(glist)}')
        print(f'Generated commands: {len(detglist)}, Manually implemented commands: {len(very_special_commands)}')

        exit(0)

    if cli_args.parse:
        from commands_parser.commands_parser import command_parser

        command_parser.verify_format()
        command_parser.parse_all_commands()

    generate()

    if cli_args.format:
        os.system(f'clang-format -i  --style="{{Standard: C++11}}" {GEN_PATH.parent.absolute() / "src" / "Caller.cpp"}')
        os.system(f'clang-format -i  --style="{{Standard: C++11}}" {GEN_PATH.parent.absolute() / "src" / "Caller.h"}')
        print('[X] code formatted')
