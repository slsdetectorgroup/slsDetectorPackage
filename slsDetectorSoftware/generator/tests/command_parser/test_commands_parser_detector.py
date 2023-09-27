import json
from pathlib import Path

import yaml

from commands_parser.commands_parser import CommandParser

data_path = Path(__file__).parent / "data"


def test_basic_propagation(tmp_path):
    output_file = tmp_path / "detectors.yaml"
    command_parser = CommandParser(commands_file=data_path / "detectors.yaml", output_file=output_file)
    command_parser.verify_format()
    command_parser.parse_all_commands()

    assert output_file.exists()
    command = yaml.unsafe_load(output_file.open('r'))['basic']
    assert command['help'] == "xx11"

    # GET
    assert command['actions']['GET'].keys() == {'detectors','args'}
    assert command['actions']['GET']['detectors'].keys() == {'MYTHEN3', 'CHIPTESTBOARD'}
    mythen = command['actions']['GET']['detectors']['MYTHEN3']
    assert len(mythen) == 2
    assert mythen[0]['argc'] == 0
    assert mythen[1]['argc'] == 1
    assert mythen[0]['function'] == mythen[1]['function'] == 'do_mythen3'
    assert mythen[0]['output'] == ['OutString(t)']
    assert mythen[1]['output'] == ['testytest']

    ctb = command['actions']['GET']['detectors']['CHIPTESTBOARD']
    assert len(ctb) == 1
    assert ctb[0]['argc'] == 55
    assert ctb[0]['function'] == 'func1'

    # PUT
    assert command['actions']['PUT'].keys() == {'detectors'}
    assert command['actions']['PUT']['detectors'].keys() == {'EIGER'}
    eiger = command['actions']['PUT']['detectors']['EIGER']
    assert len(eiger) == 1
    assert eiger[0]['argc'] == 99
    assert eiger[0]['function'] == 'do_eiger'
    assert eiger[0]['output'] == ['eigerOutput']


def test_inheritance_01(tmp_path):
    output_file = tmp_path / "detectors.yaml"
    command_parser = CommandParser(commands_file=data_path / "detectors.yaml", output_file=output_file)
    command_parser.verify_format()
    command_parser.parse_all_commands()
    assert output_file.exists()
    commands = yaml.unsafe_load(output_file.open('r'))

    # case 0100
    command = commands['case_0100']
    assert command['help'] == "0100"
    assert 'actions' in command
    assert command['actions'].keys() == {'GET'}
    assert command['actions']['GET'].keys() == {'detectors'}
    assert command['actions']['GET']['detectors'].keys() == {'MYTHEN3','CHIPTESTBOARD'}
    mythen = command['actions']['GET']['detectors']['MYTHEN3']
    assert len(mythen) == 1
    assert mythen[0]['argc'] == 99
    assert mythen[0]['function'] == 'do_mythen3'

