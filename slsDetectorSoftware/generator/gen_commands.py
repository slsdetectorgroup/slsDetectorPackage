from pathlib import Path

import yaml

from cpp_codegen.codegen import codegen, if_block, for_block, function, else_block

GEN_PATH = Path(__file__).parent
COMMANDS_PATH = GEN_PATH / 'commands.yaml'
commands_config = yaml.unsafe_load(COMMANDS_PATH.open('r'))

print(commands_config)
codegen.open(GEN_PATH / 'Caller.cpp')
# write call function
codegen.write_opening(GEN_PATH / 'Caller.in.cpp')

# iterate over the commands and generate code for each
for command_name, command in commands_config.items():
    with function('std::string', 'Caller::' + command_name, [('int', 'action')]) as fn:
        codegen.write_line(f'std::cout << "{fn.name} called\\n";')
        codegen.write_line('std::ostringstream os;')
        codegen.write_line('os << cmd << \' \';')

        # infer action based on number of arguments
        codegen.write_line('// infer action based on number of arguments')
        if command['infer_action']:
            with if_block('action == -1'):
                first = True
                for action, action_params in command['actions'].items():
                    with if_block(f'args.size() == {action_params["argc"]}', elseif=not first):
                        codegen.write_line(f'action = {codegen.actions_dict[action]};')
                    first = False
                with else_block():
                    codegen.write_line('throw RuntimeError("Wrong number of arguments");')

        # check if action and arguments are valid
        codegen.write_line('// check if action and arguments are valid')

        # generate code for each action
        codegen.write_line('// generate code for each action')
        for action, action_params in command['actions'].items():
            with if_block(f'action == {codegen.actions_dict[action]}'):
                # prepare arguments list
                arguments = []
                for i in range(action_params['argc']):
                    codegen.write_line(f'auto arg{i} = StringTo<{action_params["arg_types"][i]}>(args[{i}]);')
                    arguments.append(f'arg{i}')
                if 'require_det_id' in action_params and action_params['require_det_id']:
                    arguments.append("std::vector<int>{ det_id }")

                arguments = ", ".join(arguments)
                # call function
                if action == 'GET':
                    codegen.write_line(f'auto t = det->{action_params["function"]}({arguments});')
                    codegen.write_line('os << OutString(t) << \'\\n\';')
                elif action == 'PUT':
                    codegen.write_line(f'det->{action_params["function"]}({arguments});')
                    codegen.write_line('os << args.front() << \'\\n\';')
        codegen.write_line('return os.str();')

# close sls namespace
codegen.write_closing()
codegen.close()

codegen.write_header(GEN_PATH / 'Caller.in.h',GEN_PATH / 'Caller.h', list(commands_config.keys()))
