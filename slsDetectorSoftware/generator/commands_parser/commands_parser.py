import copy
import logging
import yaml
from pathlib import Path


class CommandParser:
    def __init__(
            self,
            commands_file: Path = Path(__file__).parent.parent / 'commands.yaml',
            output_file: Path = Path(__file__).parent.parent / 'extended_commands.yaml'
    ):

        self.output_file = output_file
        self.commands_file = commands_file
        self.fp = self.commands_file.open('r')
        self.simple_commands = yaml.unsafe_load(self.fp)
        self.extended_commands = {}
        self.argc_set = set()
        self.logger = logging.getLogger('command_parser')
        self.__current_action: str = ''
        FORMAT = '[%(levelname)s] %(message)s'
        logging.basicConfig(format=FORMAT, level=logging.INFO)

        self.propagate_config = {
            'require_det_id': False,
            'convert_det_id': True,
            'input': [],
            'input_types': [],
            'function': '',
            'output': [],
            'cast_input': [],
            'check_det_id': False,
            'arg_types': [],
            # 'store_result_in_t': False,  # always true in GET action
        }
        self.default_config = {
            'infer_action': True,
            'help': '',
            'actions': {}
        }

    def _verify_argument(self, arg, infer_action, command_name, action):
        if arg['function'] == '' and 'ctb_output_list' not in arg:
            special_exception_message_list = ["Cannot put", "Cannot get"]
            if 'exceptions' in arg and arg['exceptions'][0]['condition'] == 'true' and any(ele in arg['exceptions'][0]['message'] for ele in special_exception_message_list):
                self.logger.warning(f"{command_name} has a special exception message for {action}.")
            else:
                self.logger.warning(f"{command_name} [{action}] does not have a function")
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
        # if infer_action:
        #     if arg['argc'] in self.argc_set:
        #         raise ValueError(f'Argument {arg} has a duplicate argc')
        #     self.argc_set.add(arg['argc'])

    def verify_format(self):
        # todo verify detectors
        # todo verify circular inheritance
        # todo verify child commands (those that inherit)
        # todo verify that there is no wrongly typed parameters
        # todo verify that the same number of input_types and input are given
        # todo verify that each argument has argc (error can happen when inheriting)
        for command_name, command in self.simple_commands.items():
            if 'inherit_actions' in command or 'template' in command and command[
                'template'] or 'is_description' in command and command['is_description']:
                continue
            self.argc_set = set()
            if 'infer_action' not in command:
                command['infer_action'] = True
            if 'actions' not in command:
                raise ValueError(f'Command {command_name} does not have any actions')
            for action, action_params in command['actions'].items():
                if 'argc' in action_params:
                    if 'args' in action_params:
                        raise ValueError(f'Action {action} has both argc and args')
                    arg = {**self.propagate_config, **action_params}
                    self._verify_argument(arg, command['infer_action'], command_name, action)
                elif 'args' in action_params:
                    if type(action_params['args']) is not list:
                        raise ValueError(f'Action {action} args is not a list')
                    if len(action_params['args']) == 0:
                        raise ValueError(f'Action {action} args is empty')
                    action_args = {**self.propagate_config, **action_params}
                    del action_args['args']
                    for arg in action_params['args']:
                        arg = {**action_args, **arg}
                        self._verify_argument(arg, command['infer_action'], command_name, action)
        self.logger.info('Commands file is valid ✅️')
        return True

    def _parse_inherited_command(self, parent, command, simple_parent):
        """
        parse a command that inherits from parent command
        :param parent: parsed parent command
        :param command: the current command
        :param simple_parent: unparsed parent command
        :return: parsed command
        """
        # deepcopy parent and command to avoid modifying the originals
        command = copy.deepcopy(command)
        config = copy.deepcopy(parent)
        # add help
        if 'help' in command:
            config['help'] = command['help']
        if 'actions' not in command:
            return config
        for action, command_params in command['actions'].items():
            self.__current_action = action
            if action not in config['actions']:
                # todo: handle this case
                pass
            parent_params = config['actions'][action]
            if 'args' in command_params:
                # child has args => inherit action level params from parent + override with child args + use child's
                # action level params
                context = {**self.propagate_config, **simple_parent['actions'][action], **command_params}
                config['actions'][action]['args'] = self.parse_action(context, command_params['args'])
            elif 'argc' in command_params:
                # child has action level args (argc)
                context = {**self.propagate_config, **simple_parent['actions'][action], **command_params}
                config['actions'][action]['args'] = self.parse_action(context, [])
            else:
                # child does not have args => use parent's action level params + override with child's action level
                if 'args' in parent_params:
                    config['actions'][action]['args'] = self.parse_action({}, parent_params['args'], command_params)

            if 'detectors' in command_params:
                if command_params['detectors'] is None:
                    # if child has an empty detector section, then delete the parent's detector section
                    del config['actions'][action]['detectors']
                    continue

                for detector_name, detector_params in command_params['detectors'].items():
                    if 'detectors' not in config['actions'][action]:
                        config['actions'][action]['detectors'] = {}
                    config_detector = config['actions'][action]['detectors']
                    if 'detectors' not in parent_params or detector_name not in parent_params['detectors']:
                        if 'args' in detector_params:
                            # if child has detector args and parent does not have detectors
                            # => use child's detector args
                            context = {**self.propagate_config, **simple_parent['actions'][action], **detector_params}
                            config_detector[detector_name] = self.parse_action(context, detector_params['args'])
                        elif 'args' in parent_params:
                            # if child does not have detector args and parent does not have detectors
                            # => use the child's action args
                            context = {**self.propagate_config, **simple_parent['actions'][action]}
                            config_detector[detector_name] = self.parse_action(context,
                                                                               config['actions'][action]['args'],
                                                                               detector_params)
                    elif detector_name in parent_params['detectors']:

                        if 'args' in detector_params:
                            # child and parent have the same detector and child has detector args
                            # => use child's detector args
                            context = {
                                **self.propagate_config,
                                **simple_parent['actions'][action],
                                **simple_parent['actions'][action]['detectors'][detector_name],

                            }
                            config_detector[detector_name] = self.parse_action(context, detector_params['args'])
                        else:
                            # child and parent have the same detector and child does not have detector args
                            # => use parent's detector args
                            priority_context = {**command_params, **detector_params}
                            config_detector[detector_name] = self.parse_action(
                                {},
                                parent_params['detectors'][detector_name],
                                priority_context
                            )

                    else:
                        pass

        return config

    def _parse_command(self, command):
        """
        logic function for parse_command.
        This function is recursive
        :return: parsed command
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
            config = self._parse_inherited_command(parent, command, self.simple_commands[command['inherit_actions']])
            return config

        if 'actions' not in command:
            return config

        for action, action_params in command['actions'].items():
            self.__current_action = action

            config['actions'][action] = {}
            config_action = config['actions'][action]
            # the context in the current command and the current action
            action_context = {**self.propagate_config, **action_params}
            if 'args' not in action_params:
                # parse the action with the action context
                if action_params.keys() != {'detectors'}:
                    config_action['args'] = self.parse_action(action_context, [])
            else:
                config_action['args'] = self.parse_action(action_context, action_params['args'])

            # check if the action has detectors
            if 'detectors' in action_params:
                config_action['detectors'] = {}
                for detector_name, detector_params in action_params['detectors'].items():
                    # get the context for the detector and merge it with the action context
                    detector_context = {**action_context, **detector_params}

                    # if the detector does not have args, then use the action args
                    # otherwise, use the detector args and override the action args
                    tmp_args = []
                    if 'args' not in detector_params:
                        if 'args' in config_action:
                            tmp_args = config_action['args']
                    else:
                        tmp_args = detector_params['args']
                    detector_params['args'] = tmp_args

                    # parse the action with the detector context
                    config_action['detectors'][detector_name] = self.parse_action(detector_context,
                                                                                  tmp_args,
                                                                                  detector_params)
        return config

    def sanitize_argument(func):
        def f(self, action_context, args_old, priority_context={}):
            args = func(self, action_context, args_old, priority_context)
            for i, arg in enumerate(args):
                if 'args' in arg:
                    del arg['args']
                if 'detectors' in arg:
                    del arg['detectors']
                if not arg['cast_input']:
                    # if the cast_input is empty, then set it to False
                    arg['cast_input'] = [False] * len(arg['input'])

                elif len(arg['cast_input']) != len(arg['input']):
                    # if the cast_input is not the same length as the input, then set it to False
                    arg['cast_input'] = [False] * len(arg['input'])
                    self.logger.warning(f'cast_input for {arg["function"]} '
                                        f'with argc: {arg["argc"]} has different length than input')
                if 'store_result_in_t' not in arg:
                    if self.__current_action == 'GET':
                        arg['store_result_in_t'] = True
                    else:
                        arg['store_result_in_t'] = False
            return args

        return f

    @sanitize_argument
    def parse_action(self, action_context, args, priority_context={}):
        """
        parse an action
        :param action_context: context of the action
        :param args: arguments to be used in the action
        :param priority_context: context that should override the arguments params
        :return: parsed action
        """

        def add_cast_input(argument):
            return argument

        # deepcopy action_context to avoid modifying the original
        action_context = {**self.propagate_config, **copy.deepcopy(action_context)}
        priority_context = copy.deepcopy(priority_context)

        if 'detectors' in action_context:
            del action_context['detectors']
        if 'detectors' in priority_context:
            del priority_context['detectors']

        if args == []:
            # if there are no arguments, then the action has only one argument
            context = {**action_context, **priority_context}
            return [add_cast_input(context)]

        ret_args = []
        if 'args' in action_context:
            del action_context['args']
        if 'args' in priority_context:
            del priority_context['args']

        # if there are arguments, then merge them with the action context and priority context
        for arg in args:
            arg = {**action_context, **arg, **priority_context}
            ret_args.append(add_cast_input(arg))
        return ret_args

    def parse_command(self, command_name):
        """
        parse a single command
        This function is recursive

        :param command_name: name of the command to parse
        :return: the parsed command
        """
        command = self.simple_commands[command_name]
        parsed_command = self._parse_command(command)
        if 'function_alias' not in command:
            if 'command_name' in command:
                parsed_command['function_alias'] = command['command_name']
            else:
                parsed_command['function_alias'] = command_name

        if 'command_name' not in command:
            parsed_command['command_name'] = command_name

        if 'template' in command and command['template']:
            return parsed_command
        self.extended_commands[command_name] = parsed_command
        return self.extended_commands[command_name]

    def parse_all_commands(self):
        """
        iterate over all commands in yaml file and parse them
        :return: None
        """

        for command_name in self.simple_commands:
            # todo remove this (added for debugging)
            if command_name != 'xtiming':
                self.parse_command(command_name)

        # post-process the parsed commands
        self.post_process_all_commands()

        yaml.Dumper.ignore_aliases = lambda *args: True
        self.logger.info(f'parsed {len(self.extended_commands)} commands')
        yaml.dump(self.extended_commands, self.output_file.open('w'), default_flow_style=False)

    def post_process_all_commands(self):
        for command_name, command in self.extended_commands.items():
            if 'is_description' in command and command['is_description']:
                continue
            for action_name, action, in command['actions'].items():
                for arg in action['args']:
                    if arg['argc'] == 0:
                        arg['arg_types'] = []
                        continue
                    if arg['argc'] == -1:
                        pass
                    if arg['arg_types'] == []:
                        arg['arg_types'] = arg['input_types']


# command_parser = CommandParser(Path(
#     '/afs/psi.ch/user/b/braham_b/github/slsDetectorPackage/slsDetectorSoftware/generator/tests/command_parser/data/detectors.yaml'))
command_parser = CommandParser()

if __name__ == '__main__':
    command_parser.verify_format()
    command_parser.parse_all_commands()
