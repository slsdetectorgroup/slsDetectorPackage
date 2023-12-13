from pathlib import Path

import yaml

from commands_parser.commands_parser import CommandParser

data_path = Path(__file__).parent.parent / "data"


def test_basic_propagation(tmp_path):
    output_file = tmp_path / "basic.yaml"
    command_parser = CommandParser(commands_file=data_path / "basic.yaml", output_file=output_file)
    command_parser.verify_format()
    command_parser.parse_all_commands()

    assert output_file.exists()
    command = yaml.unsafe_load(output_file.open('r'))['basic']
    assert command['help'] == "xx11"
    assert len(command['actions']) == 2
    # test 'GET' action
    assert 'args' in command['actions']['GET']
    assert len(command['actions']['GET'].keys()) == 1  # only 'args' key
    assert len(command['actions']['GET']['args']) == 1  # only one argument
    assert command['actions']['GET']['args'][0]['argc'] == 0
    assert command['actions']['GET']['args'][0]['function'] == 'func1'
    assert command['actions']['GET']['args'][0]['output'] == ['OutString(t)']
    assert command['actions']['GET']['args'][0]['input'] == []
    assert command['actions']['GET']['args'][0]['cast_input'] == []
    assert command['actions']['GET']['args'][0]['require_det_id'] is False
    # test PUT action
    assert 'args' in command['actions']['PUT']
    assert len(command['actions']['PUT'].keys()) == 1  # only 'args' key
    assert len(command['actions']['PUT']['args']) == 1  # only one argument
    assert command['actions']['PUT']['args'][0]['argc'] == 1
    assert command['actions']['PUT']['args'][0]['function'] == 'func2'
    assert command['actions']['PUT']['args'][0]['cast_input'] == [True]
    assert command['actions']['PUT']['args'][0]['output'] == ['args.front()']
    assert command['actions']['PUT']['args'][0]['input_types'] == ['int']
    assert command['actions']['PUT']['args'][0]['require_det_id'] is False


def test_basic_inheritance(tmp_path):
    output_file = tmp_path / "basic_inheritance.yaml"
    command_parser = CommandParser(commands_file=data_path / "basic_inheritance.yaml", output_file=output_file)
    command_parser.verify_format()
    command_parser.parse_all_commands()
    assert output_file.exists()
    command = yaml.unsafe_load(output_file.open('r'))['basic']
    assert command['help'] == "xx11"
    assert command['actions'].keys() == {'GET', 'PUT'}
    # test 'GET' action
    assert 'args' in command['actions']['GET']
    assert command['actions']['GET'].keys() == {'args'}  # only 'args' key
    assert len(command['actions']['GET']['args']) == 1  # only one argument
    assert command['actions']['GET']['args'][0]['argc'] == 2
    assert command['actions']['GET']['args'][0]['function'] == 'x'
    assert command['actions']['GET']['args'][0]['output'] == []  # test overwriting args when they are present in child
    assert command['actions']['GET']['args'][0]['input'] == []
    assert command['actions']['GET']['args'][0]['cast_input'] == []
    assert command['actions']['GET']['args'][0]['require_det_id'] is False
    # test PUT action
    assert 'args' in command['actions']['PUT']
    assert command['actions']['PUT'].keys() == {'args'}  # only 'args' key
    assert len(command['actions']['PUT']['args']) == 1  # only one argument
    assert command['actions']['PUT']['args'][0]['argc'] == 1
    assert command['actions']['PUT']['args'][0]['function'] == 'func2'
    assert command['actions']['PUT']['args'][0]['cast_input'] == [True]
    assert command['actions']['PUT']['args'][0]['output'] == ['args.front()']
    assert command['actions']['PUT']['args'][0]['input_types'] == ['int']
    assert command['actions']['PUT']['args'][0]['require_det_id'] is False


def test_basic_inheritance2(tmp_path):
    output_file = tmp_path / "basic_inheritance.yaml"
    command_parser = CommandParser(commands_file=data_path / "basic_inheritance.yaml", output_file=output_file)
    command_parser.verify_format()
    command_parser.parse_all_commands()
    assert output_file.exists()
    command = yaml.unsafe_load(output_file.open('r'))['basic2']
    # check GET
    assert len(command['actions']['GET']['args']) == 1
    assert command['actions']['GET'].keys() == {'args'}
    arg = command['actions']['GET']['args'][0]
    assert arg['argc'] == 2
    assert arg['output'] == ['OutString(t)']
    # check that length of cast input is equal to length of input_types and input
    assert len(arg['input']) == len(arg['input_types']) == len(arg['cast_input']) == 4
    assert arg['function'] == 'x'
    assert 'convert_to_time' in arg
    assert arg['convert_to_time'].keys() == {'input', 'output'}
    assert 'separate_time_units' in arg
    assert arg['separate_time_units'].keys() == {'input', 'output'}

    # check PUT
    assert command['actions']['PUT'].keys() == {'args'}
    assert len(command['actions']['PUT']['args']) == 2
    assert command['actions']['PUT']['args'][0]['argc'] == 19
    assert command['actions']['PUT']['args'][0]['function'] == 'y'
    assert command['actions']['PUT']['args'][1]['argc'] == 91
    assert command['actions']['PUT']['args'][1]['function'] == 'y'

