import copy
from pathlib import Path

import yaml


class CommandParser:
    def __init__(self, commands_file: Path):
        self.commands_file = commands_file
        self.fp = self.commands_file.open('r')
        self.simple_commands = yaml.unsafe_load(self.fp)
        self.extended_commands = {}
        self.argc_set = set()

        self.propagate_config = {
            'require_det_id': False,
            'input': [],
            'input_types': [],
            'function': '',
            'output': []
        }
        self.default_config = {
            'infer_action': True,
            'help': '',
            'actions': {}
        }

    def _verify_argument(self, arg, infer_action):

        if arg['function'] == '':
            raise ValueError(f'Argument {arg} does not have a function')
        if len(arg['input_types']) != len(arg['input']):
            raise ValueError(f'Argument {arg} does not have the correct number of inputs')
        if 'separate_time_units' in arg:

            if arg['separate_time_units']['input'] == "":
                raise ValueError(f'Argument {arg} does not have the correct number of inputs for separate_time_units')
            if len(arg['separate_time_units']['output']) != 2:
                raise ValueError(f'Argument {arg} does not have the correct number of outputs for separate_time_units')
        if 'convert_to_time' in arg:
            if len(arg['convert_to_time']['input']) != 2:
                raise ValueError(f'Argument {arg} does not have the correct number of inputs for convert_to_time')
            if len(arg['convert_to_time']['output']) == "":
                raise ValueError(f'Argument {arg} does not have the correct number of outputs for convert_to_time')
        if infer_action:
            if arg['argc'] in self.argc_set:
                raise ValueError(f'Argument {arg} has a duplicate argc')
            self.argc_set.add(arg['argc'])

    def verify_format(self):
        # todo verify detectors
        for command_name, command in self.simple_commands.items():
            if 'inherit_actions' in command:
                return
            self.argc_set = set()
            if 'infer_action' not in command:
                command['infer_action'] = False
            if 'actions' not in command:
                raise ValueError(f'Command {command_name} does not have any actions')
            for action, action_params in command['actions'].items():
                if 'argc' in action_params:
                    if 'args' in action_params:
                        raise ValueError(f'Action {action} has both argc and args')
                    arg = {**self.propagate_config, **action_params}
                    self._verify_argument(arg, command['infer_action'])
                elif 'args' in action_params:
                    if type(action_params['args']) is not list:
                        raise ValueError(f'Action {action} args is not a list')
                    if len(action_params['args']) == 0:
                        raise ValueError(f'Action {action} args is empty')
                    action_args = {**self.propagate_config, **action_params}
                    del action_args['args']
                    for arg in action_params['args']:
                        arg = {**action_args, **arg}
                        self._verify_argument(arg, command['infer_action'])
        print('Commands file is valid ✅️')

    def parse_inherited(self, parent, command):
        command = copy.deepcopy(command)
        config = copy.deepcopy(parent)
        for action, command_params in command['actions'].items():
            parent_params = config['actions'][action]
            if 'args' not in command_params:
                config['actions'][action]['args'] = self.parse_action({}, parent_params['args'], command_params)
            else:
                print(parent_params)
                config['actions'][action]['args'] = self.parse_action(parent_params,command_params['args'],command_params)

            # if 'detectors' in command_params:
            #     for detector_name, detector_params in command_params['detectors'].items():
            #         action_context = {**command_params, **detector_params}
            #
            #         if 'args' not in detector_params:
            #             detector_params['args'] = []
            #         config['actions'][action]['detectors'] = {}
            #         config['actions'][action]['detectors'][detector_name] = self.parse_action(
            #             action_context, config['actions'][action]['args'], detector_params)
        return config

    def _parse_command(self, command):
        config = self.default_config.copy()
        config.update(command)
        config['actions'] = {}
        if 'inherit_actions' in command:
            if command['inherit_actions'] in self.extended_commands:
                parent = self.extended_commands[command['inherit_actions']]
            else:
                parent = self.parse_command(command['inherit_actions'])
            config = self.parse_inherited(parent, command)
            return config
        for action, action_params in command['actions'].items():
            config['actions'][action] = {}
            action_config = {**self.propagate_config, **action_params}
            print(action, action_params)
            if 'args' not in action_params:
                action_params['args'] = []
            config['actions'][action]['args'] = self.parse_action(action_config, action_params['args'])
            #
            # if 'detectors' in action_params:
            #     for detector_name, detector_params in action_params['detectors'].items():
            #         action_context = {**action_config, **detector_params}
            #
            #         if 'args' not in detector_params:
            #             detector_params['args'] = []
            #         config['actions'][action]['detectors'] = {}
            #         config['actions'][action]['detectors'][detector_name] = self.parse_action(
            #             action_context, config['actions'][action]['args'], detector_params)
        return config

    def parse_command(self, command_name):
        print(command_name)
        command = self.simple_commands[command_name]
        self.extended_commands[command_name] = self._parse_command(command)
        return self.extended_commands[command_name]

    def parse_all_commands(self):

        for command_name in self.simple_commands:
            self.parse_command(command_name)
        yaml.dump(self.extended_commands, (self.commands_file.parent / 'extended_commands.yaml').open('w'),
                  default_flow_style=False)

    def parse_action(self, action_context, args, priority_context={}):
        action_context = {**self.propagate_config, **copy.deepcopy(action_context)}
        if 'detectors' in action_context:
            del action_context['detectors']
        if not args:
            return [{**action_context, **priority_context}]
        ret_args = []
        if 'args' in action_context:
            del action_context['args']
        if 'args' in priority_context:
            del priority_context['args']

        for arg in args:
            arg = {**action_context, **arg, **priority_context}
            ret_args.append(arg)
        return ret_args


command_parser = CommandParser(Path(__file__).parent.parent / 'commands.yaml')
command_parser.verify_format()
command_parser.parse_all_commands()
