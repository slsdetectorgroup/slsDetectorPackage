import json
from pathlib import Path

import pytest as pytest
import yaml

from commands_parser.commands_parser import CommandParser

data_path = Path(__file__).parent / "data"


@pytest.fixture()
def detector_file_commands(tmp_path):
    output_file = tmp_path / "detectors.yaml"
    command_parser = CommandParser(commands_file=data_path / "detectors.yaml", output_file=output_file)
    command_parser.verify_format()

    def func(command):
        return command_parser.parse_command(command)

    return func


def test_basic_propagation(tmp_path, detector_file_commands):
    command = detector_file_commands('basic')

    assert command['help'] == "xx11"

    # GET
    assert command['actions']['GET'].keys() == {'detectors', 'args'}
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


def test_inheritance_0100(tmp_path, detector_file_commands):
    command = detector_file_commands('case_0100')
    assert command['help'] == "0100"
    assert 'actions' in command
    assert command['actions'].keys() == {'GET'}
    assert command['actions']['GET'].keys() == {'detectors'}
    assert command['actions']['GET']['detectors'].keys() == {'MYTHEN3', 'CHIPTESTBOARD'}
    mythen = command['actions']['GET']['detectors']['MYTHEN3']
    assert len(mythen) == 1
    assert mythen[0]['argc'] == 99
    assert mythen[0]['function'] == 'do_mythen3'


def test_inheritance_0101(tmp_path, detector_file_commands):
    command = detector_file_commands('case_0101')
    assert command['help'] == "0101"
    assert 'actions' in command
    assert command['actions'].keys() == {'GET'}
    assert command['actions']['GET'].keys() == {'detectors'}
    assert command['actions']['GET']['detectors'].keys() == {'MYTHEN3', 'CHIPTESTBOARD'}
    mythen = command['actions']['GET']['detectors']['MYTHEN3']
    assert len(mythen) == 1
    assert mythen[0]['argc'] == 420
    assert mythen[0]['function'] == 'do_mythen23'
    ctb = command['actions']['GET']['detectors']['CHIPTESTBOARD']
    assert len(ctb) == 1
    assert ctb[0]['argc'] == 98
    assert ctb[0]['function'] == 'do_ctb'


def test_inheritance_0110(tmp_path, detector_file_commands):
    command = detector_file_commands('case_0110')
    assert command['help'] == "0110"
    assert 'actions' in command
    assert command['actions'].keys() == {'GET'}
    assert command['actions']['GET'].keys() == {'args', 'detectors'}
    assert command['actions']['GET']['args'][0]['argc'] == 111
    mythen = command['actions']['GET']['detectors']['MYTHEN3']
    assert len(mythen) == 1
    assert mythen[0]['argc'] == 99
    assert mythen[0]['function'] == 'do_mythen3'
    ctb = command['actions']['GET']['detectors']['CHIPTESTBOARD']
    assert len(ctb) == 1
    assert ctb[0]['argc'] == 98
    assert ctb[0]['function'] == 'do_ctb'


def test_inheritance_0110v2(tmp_path, detector_file_commands):
    command = detector_file_commands('case_0110v2')
    assert command['help'] == "0110v2"
    assert 'actions' in command
    assert command['actions'].keys() == {'GET'}
    assert command['actions']['GET'].keys() == {'args', 'detectors'}
    assert command['actions']['GET']['args'][0]['argc'] == 111
    mythen = command['actions']['GET']['detectors']['MYTHEN3']
    assert len(mythen) == 1
    assert mythen[0]['argc'] == 99
    assert mythen[0]['function'] == 'do_mythen3'
    ctb = command['actions']['GET']['detectors']['CHIPTESTBOARD']
    assert len(ctb) == 1
    assert ctb[0]['argc'] == 98
    assert ctb[0]['function'] == 'do_ctb'


def test_inheritacnce_0111(tmp_path, detector_file_commands):
    command = detector_file_commands('case_0111')
    assert command['help'] == "0111"
    assert 'actions' in command
    assert command['actions'].keys() == {'GET'}
    assert command['actions']['GET'].keys() == {'args', 'detectors'}
    assert command['actions']['GET']['detectors'].keys() == {'MYTHEN3', 'CHIPTESTBOARD'}
    mythen = command['actions']['GET']['detectors']['MYTHEN3']
    assert len(mythen) == 1
    assert command['actions']['GET']['args'][0]['argc'] == 111
    assert len(mythen) == 1
    assert mythen[0]['argc'] == 420
    assert mythen[0]['function'] == 'do_mythen23'
    ctb = command['actions']['GET']['detectors']['CHIPTESTBOARD']
    assert len(ctb) == 1
    assert ctb[0]['argc'] == 98
    assert ctb[0]['function'] == 'do_ctb'


# cases 1000, 1001, 1010, 1011
def test_inheritance_1000(tmp_path, detector_file_commands):
    command = detector_file_commands('case_1000')
    assert command['help'] == "1000"
    assert 'actions' in command
    assert command['actions'].keys() == {'GET'}
    assert command['actions']['GET'].keys() == {'args'}
    assert len(command['actions']['GET']['args']) == 2
    assert command['actions']['GET']['args'][0]['argc'] == 0
    assert command['actions']['GET']['args'][0]['output'] == []

    assert command['actions']['GET']['args'][1]['argc'] == 1
    assert command['actions']['GET']['args'][1]['output'] == ['testytest']

def test_inheritance_1001(tmp_path, detector_file_commands):
    command = detector_file_commands('case_1001')
    assert command['help'] == "1001"
    assert 'actions' in command
    assert command['actions'].keys() == {'GET'}
    assert command['actions']['GET'].keys() == {'args', 'detectors'}
    assert len(command['actions']['GET']['args']) == 2
    assert command['actions']['GET']['args'][0]['argc'] == 0
    assert command['actions']['GET']['args'][0]['output'] == []
    assert command['actions']['GET']['args'][1]['argc'] == 1
    assert command['actions']['GET']['args'][1]['output'] == ['testytest']

    assert command['actions']['GET']['detectors'].keys() == {'MYTHEN3'}
    assert len(command['actions']['GET']['detectors']['MYTHEN3']) == 2
    assert command['actions']['GET']['detectors']['MYTHEN3'][0]['argc'] == 420
    assert command['actions']['GET']['detectors']['MYTHEN3'][0]['function'] == 'do_mythen23'
    assert command['actions']['GET']['detectors']['MYTHEN3'][1]['argc'] == 99
    assert command['actions']['GET']['detectors']['MYTHEN3'][1]['function'] == 'do_mythen3'







