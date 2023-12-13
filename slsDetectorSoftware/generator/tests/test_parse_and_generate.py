from pathlib import Path
from commands_parser.commands_parser import CommandParser
import gen_commands

data_path = Path(__file__).parent.parent


def test_parse_and_generate(tmp_path):
    """
    tests that the parse and generate functions work without errors
    :param tmp_path:
    :return:
    """
    output_file = tmp_path / "detectors.yaml"
    command_parser = CommandParser(commands_file=data_path / "commands.yaml", output_file=output_file)
    command_parser.verify_format()
    command_parser.parse_all_commands()
    assert output_file.exists()

    GEN_PATH = Path(__file__).parent.parent
    gen_commands.generate(
        output_file,
        GEN_PATH / "Caller.in.cpp",
        GEN_PATH / "Caller.in.h",
        tmp_path / "Caller.cpp",
        tmp_path / "Caller.h",
    )
    assert (tmp_path / "Caller.cpp").exists()
    assert (tmp_path / "Caller.h").exists()
