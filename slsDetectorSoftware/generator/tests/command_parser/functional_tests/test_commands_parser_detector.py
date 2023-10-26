import json
from pathlib import Path

import pytest as pytest
import yaml

from commands_parser.commands_parser import CommandParser

data_path = Path(__file__).parent.parent / "data"


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

# 16 test cases for inheritance
# 1st bit: parent has args
# 2nd bit: parent has detectors
# 3rd bit: child has args
# 4th bit: child has detectors
# each test case is a combination of the above bits
# all the possible combinations are tested 

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


def test_inheritance_1010(tmp_path, detector_file_commands):
    command = detector_file_commands('case_1010')
    assert command['help'] == "1010"
    assert 'actions' in command
    assert command['actions'].keys() == {'GET'}
    assert command['actions']['GET'].keys() == {'args'}
    assert len(command['actions']['GET']['args']) == 1
    assert command['actions']['GET']['args'][0]['argc'] == 111
    assert command['actions']['GET']['args'][0]['function'] == 'get_function'


def test_inheritance_1011(tmp_path, detector_file_commands):
    command = detector_file_commands('case_1011')
    assert command['help'] == "1011"
    assert 'actions' in command
    assert command['actions'].keys() == {'GET'}
    assert command['actions']['GET'].keys() == {'args', 'detectors'}
    assert len(command['actions']['GET']['args']) == 1
    assert command['actions']['GET']['args'][0]['argc'] == 111
    assert command['actions']['GET']['args'][0]['function'] == 'get_function'

    assert command['actions']['GET']['detectors'].keys() == {'MYTHEN3'}
    assert len(command['actions']['GET']['detectors']['MYTHEN3']) == 1
    assert command['actions']['GET']['detectors']['MYTHEN3'][0]['argc'] == 111
    assert command['actions']['GET']['detectors']['MYTHEN3'][0]['function'] == 'do_mythen23'


# cases 1100, 1101, 1110, 1111

def test_inheritance_1100(tmp_path, detector_file_commands):
    command = detector_file_commands('case_1100')
    assert command['help'] == "1100"
    assert 'actions' in command
    assert command['actions'].keys() == {'GET'}
    assert command['actions']['GET'].keys() == {'args', 'detectors'}
    assert len(command['actions']['GET']['args']) == 2
    assert command['actions']['GET']['args'][0]['argc'] == 0
    assert command['actions']['GET']['args'][0]['output'] == []

    assert command['actions']['GET']['args'][1]['argc'] == 1
    assert command['actions']['GET']['args'][1]['output'] == ['testytest']

    assert command['actions']['GET']['detectors'].keys() == {'EIGER', 'POTATO'}
    assert len(command['actions']['GET']['detectors']['EIGER']) == 1
    assert command['actions']['GET']['detectors']['EIGER'][0]['argc'] == 99
    assert command['actions']['GET']['detectors']['EIGER'][0]['function'] == 'do_eiger'
    assert command['actions']['GET']['detectors']['EIGER'][0]['output'] == ['eigerOutput']

    assert len(command['actions']['GET']['detectors']['POTATO']) == 2
    assert command['actions']['GET']['detectors']['POTATO'][0]['argc'] == 0
    assert command['actions']['GET']['detectors']['POTATO'][0]['function'] == 'do_potato'


def test_inheritance_1101(tmp_path, detector_file_commands):
    command = detector_file_commands('case_1101')
    assert command['help'] == "1101"
    assert 'actions' in command
    assert command['actions'].keys() == {'GET'}
    assert command['actions']['GET'].keys() == {'args', 'detectors'}
    assert len(command['actions']['GET']['args']) == 2
    assert command['actions']['GET']['args'][0]['argc'] == 0
    assert command['actions']['GET']['args'][0]['output'] == []
    assert command['actions']['GET']['args'][1]['argc'] == 1
    assert command['actions']['GET']['args'][1]['output'] == ['testytest']

    assert command['actions']['GET']['detectors'].keys() == {'EIGER', 'MYTHEN3', 'POTATO'}
    assert len(command['actions']['GET']['detectors']['MYTHEN3']) == 2
    assert command['actions']['GET']['detectors']['MYTHEN3'][0]['argc'] == 0
    assert command['actions']['GET']['detectors']['MYTHEN3'][0]['function'] == 'do_mythen3'
    assert command['actions']['GET']['detectors']['MYTHEN3'][1]['argc'] == 1
    assert command['actions']['GET']['detectors']['MYTHEN3'][1]['function'] == 'do_mythen3'

    assert len(command['actions']['GET']['detectors']['EIGER']) == 1
    assert command['actions']['GET']['detectors']['EIGER'][0]['argc'] == 99
    assert command['actions']['GET']['detectors']['EIGER'][0]['function'] == 'do_eiger'
    assert command['actions']['GET']['detectors']['EIGER'][0]['output'] == ['eigerOutput']

    assert len(command['actions']['GET']['detectors']['POTATO']) == 2
    assert command['actions']['GET']['detectors']['POTATO'][0]['argc'] == 101
    assert command['actions']['GET']['detectors']['POTATO'][0]['function'] == 'potato_function'
    assert command['actions']['GET']['detectors']['POTATO'][1]['argc'] == 202
    assert command['actions']['GET']['detectors']['POTATO'][1]['function'] == 'do_potato'


def test_inheritance_1110(tmp_path, detector_file_commands):
    command = detector_file_commands('case_1110')
    assert command['help'] == "1110"
    assert 'actions' in command
    assert command['actions'].keys() == {'GET'}
    assert command['actions']['GET'].keys() == {'args', 'detectors'}
    assert len(command['actions']['GET']['args']) == 1
    assert command['actions']['GET']['args'][0]['argc'] == 77
    assert command['actions']['GET']['args'][0]['function'] == 'get_function'

    assert command['actions']['GET']['detectors'].keys() == {'EIGER', 'POTATO'}
    assert len(command['actions']['GET']['detectors']['EIGER']) == 1
    assert command['actions']['GET']['detectors']['EIGER'][0]['argc'] == 99
    assert command['actions']['GET']['detectors']['EIGER'][0]['function'] == 'do_eiger'

    assert len(command['actions']['GET']['detectors']['POTATO']) == 2
    assert command['actions']['GET']['detectors']['POTATO'][0]['argc'] == 0
    assert command['actions']['GET']['detectors']['POTATO'][0]['function'] == 'do_potato'
    assert command['actions']['GET']['detectors']['POTATO'][1]['argc'] == 1
    assert command['actions']['GET']['detectors']['POTATO'][1]['function'] == 'do_potato'


def test_inheritance_1111(tmp_path, detector_file_commands):
    command = detector_file_commands('case_1111')
    assert command['help'] == "1111"
    assert 'actions' in command
    assert command['actions'].keys() == {'GET'}
    assert command['actions']['GET'].keys() == {'args', 'detectors'}
    assert len(command['actions']['GET']['args']) == 1
    assert command['actions']['GET']['args'][0]['argc'] == 77
    assert command['actions']['GET']['args'][0]['function'] == 'get_function'

    assert command['actions']['GET']['detectors'].keys() == {'EIGER', 'MYTHEN3', 'POTATO'}
    assert len(command['actions']['GET']['detectors']['MYTHEN3']) == 1
    assert command['actions']['GET']['detectors']['MYTHEN3'][0]['argc'] == 77
    assert command['actions']['GET']['detectors']['MYTHEN3'][0]['function'] == 'do_mythen3'

    assert len(command['actions']['GET']['detectors']['EIGER']) == 1
    assert command['actions']['GET']['detectors']['EIGER'][0]['argc'] == 99
    assert command['actions']['GET']['detectors']['EIGER'][0]['function'] == 'do_eiger'

    assert len(command['actions']['GET']['detectors']['POTATO']) == 2
    assert command['actions']['GET']['detectors']['POTATO'][0]['argc'] == 101
    assert command['actions']['GET']['detectors']['POTATO'][0]['function'] == 'potato_function'
    assert command['actions']['GET']['detectors']['POTATO'][1]['argc'] == 202
    assert command['actions']['GET']['detectors']['POTATO'][1]['function'] == 'do_potato'
