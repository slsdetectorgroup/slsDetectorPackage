from pathlib import Path

import argparse
import yaml


def check_infer(EXTENDED_COMMANDS_PATH=Path(__file__).parent.parent / "extended_commands.yaml", commands=None):
    if commands is None:
        # load yaml file
        with EXTENDED_COMMANDS_PATH.open('r') as f:
            commands = yaml.safe_load(f)

    type_distinguishable = {}
    non_distinguishable = {}

    for command_name, command in commands.items():
        # todo: remove this (added for debug)
        # if command_name != 'badchannels':
        #     continue
        if len(command["actions"]) == 1:
            action = list(command["actions"].items())[0][1]
            for arg in action['args']:
                if arg['argc'] == -1:
                    non_distinguishable[(command_name, arg['argc'])] = ([], arg['arg_types'])
            continue


        get_argcs = {}
        get_args = command['actions']['GET']['args']
        for arg in get_args:
            if arg['argc'] != -1:
                get_argcs[arg["argc"]] = arg['arg_types']
            else:
                non_distinguishable[(command_name, arg['argc'])] = ([], arg['arg_types'])
        put_args = command['actions']['PUT']['args']
        for arg in put_args:
            if arg['argc'] == -1:
                non_distinguishable[(command_name, arg['argc'])] = ([], arg['arg_types'])
            elif arg['argc'] in get_argcs:
                if arg['arg_types'] != get_argcs[arg['argc']]:
                    type_distinguishable[(command_name, arg['argc'])] = (get_argcs[arg['argc']], arg['arg_types'])
                else:
                    non_distinguishable[(command_name, arg['argc'])] = (get_argcs[arg['argc']], arg['arg_types'])

    return type_distinguishable, non_distinguishable


if __name__ == "__main__":
    argparse = argparse.ArgumentParser()
    argparse.add_argument("--print", choices=['all', 'type', 'impossible'], default="all", help="command name")
    args = argparse.parse_args()

    type_distinguishable, non_distinguishable = check_infer()
    if args.print == 'type' or args.print == 'all':
        print("type distinguishable:")
        print("command_name: argc get_arg_type put_arg_type\n")
        for (command_name, argc), (get_arg_types, put_arg_types) in type_distinguishable.items():
            print(f"{command_name}: {argc} {get_arg_types} {put_arg_types}")

    if args.print == 'impossible' or args.print == 'all':
        print("\n\nimpossible to distinguish:")
        print("command_name: argc get_arg_type put_arg_type")
        for (command_name, argc), (get_arg_types, put_arg_types) in non_distinguishable.items():
            print(f"{command_name}: {argc} {get_arg_types} {put_arg_types}")
