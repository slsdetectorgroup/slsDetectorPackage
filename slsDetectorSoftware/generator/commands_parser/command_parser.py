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
            'output': [],
            'cast_input': []
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
        # todo verify circular inheritance
        for command_name, command in self.simple_commands.items():
            if 'inherit_actions' in command:
                continue
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

    def parse_inherited(self, parent, command, simple_parent):
        """
        parse a command that inherits from another command
        :param parent: parsed parent command
        :param command: the current command
        :param simple_parent: unparsed parent command
        :return:
        """
        # deepcopy parent and command to avoid modifying the originals
        command = copy.deepcopy(command)
        config = copy.deepcopy(parent)
        if 'actions' not in command:
            return config
        for action, command_params in command['actions'].items():
            parent_params = config['actions'][action]
            if 'args' not in command_params:
                config['actions'][action]['args'] = self.parse_action({}, parent_params['args'], command_params)
            else:
                config['actions'][action]['args'] = self.parse_action(simple_parent['actions'][action],
                                                                      command_params['args'], command_params)
            if 'detectors' in command_params:

                if command_params['detectors'] is None:
                    del config['actions'][action]['detectors']
                else:
                    for detector_name, detector_params in command_params['detectors'].items():
                        action_context = {**command_params, **detector_params}

                        if 'args' not in detector_params:
                            detector_params['args'] = []
                        config['actions'][action]['detectors'] = {}
                        config['actions'][action]['detectors'][detector_name] = self.parse_action(
                            action_context, config['actions'][action]['args'], detector_params)

        return config

    def _parse_command(self, command):
        """
        logic function for parse_command.
        This function is recursive
        :return:
        """
        config = self.default_config.copy()
        config.update(command)
        config['actions'] = {}

        # check if command inherits from another command
        if 'inherit_actions' in command:
            if command['inherit_actions'] in self.extended_commands:
                # if parent command has already been parsed, use that
                parent = self.extended_commands[command['inherit_actions']]
            else:
                # if parent command has not been parsed, parse it
                parent = self.parse_command(command['inherit_actions'])
            # parse the current command and merge it with the parent command
            config = self.parse_inherited(parent, command, self.simple_commands[command['inherit_actions']])
            return config
        for action, action_params in command['actions'].items():
            config['actions'][action] = {}
            config_action = config['actions'][action]
            # the context in the current command and the current action
            action_context = {**self.propagate_config, **action_params}
            if 'args' not in action_params:
                action_params['args'] = []
            # parse the action with the action context
            config_action['args'] = self.parse_action(action_context, action_params['args'])
            # check if the action has detectors
            if 'detectors' in action_params:
                for detector_name, detector_params in action_params['detectors'].items():
                    # get the context for the detector and merge it with the action context
                    detector_context = {**action_context, **detector_params}
                    if 'args' not in detector_params:
                        detector_params['args'] = []
                    config_action['detectors'] = {}
                    # parse the action with the detector context
                    config_action['detectors'][detector_name] = self.parse_action(detector_context,
                                                                                  config_action['args'],
                                                                                  detector_params)
        return config

    def parse_command(self, command_name):
        """
        parse a single command
        This function is recursive

        :param command_name: name of the command to parse
        :return: the parsed command
        """
        command = self.simple_commands[command_name]
        self.extended_commands[command_name] = self._parse_command(command)
        return self.extended_commands[command_name]

    def parse_all_commands(self):
        """
        iterate over all commands in yaml file and parse them
        :return: None
        """
        for command_name in self.simple_commands:
            self.parse_command(command_name)
        yaml.Dumper.ignore_aliases = lambda *args: True
        yaml.dump(self.extended_commands, (self.commands_file.parent / 'extended_commands.yaml').open('w'),
                  default_flow_style=False)

    def parse_action(self, action_context, args, priority_context={}):
        """
        parse an action
        :param action_context: context of the action
        :param args: arguments to be used in the action
        :param priority_context: context that should override the arguments params
        :return: parsed action
        """
        # deepcopy action_context to avoid modifying the original
        action_context = {**self.propagate_config, **copy.deepcopy(action_context)}
        priority_context = copy.deepcopy(priority_context)

        if 'detectors' in action_context:
            del action_context['detectors']
        if 'detectors' in priority_context:
            del priority_context['detectors']

        if not args:
            # if there are no arguments, then the action has only one argument
            context = {**action_context, **priority_context}
            if not context['cast_input']:
                # if the cast_input is empty, then set it to False
                context['cast_input'] = [False] * len(context['input'])
            return [{**action_context, **priority_context}]

        ret_args = []
        if 'args' in action_context:
            del action_context['args']
        if 'args' in priority_context:
            del priority_context['args']

        # if there are arguments, then merge them with the action context and priority context
        for arg in args:
            arg = {**action_context, **arg, **priority_context}
            if not arg['cast_input']:
                arg['cast_input'] = [False] * len(arg['input'])
            ret_args.append(arg)
        return ret_args


command_parser = CommandParser(Path(__file__).parent.parent / 'commands.yaml')
command_parser.verify_format()
command_parser.parse_all_commands()
