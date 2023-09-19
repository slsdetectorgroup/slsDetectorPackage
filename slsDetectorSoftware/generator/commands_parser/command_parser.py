# """
# frames:
#   infer_action: true # infer action based on actions' argc (they must be unique if true)
#   help: "Get or set the number of frames to be collected."
#   actions:
#     GET:
#       args:
#         - argc: 0
#           require_det_id: true
#           input: [ ]
#           input_types: [ ]
#           function: getNumberOfFrames
#           output: [ OutString(t) ]
#
#     PUT:
#       args:
#         - argc: 1
#           input: [ 'args[0]' ]
#           input_types: [ int ]
#           function: setNumberOfFrames
#           output: [ 'args[0]' ]
# """
# from pathlib import Path
#
# import yaml
#
#
# class CommandParser:
#     def __init__(self, commands_file: Path):
#         self.commands_file = commands_file
#         self.fp = self.commands_file.open('r')
#         self.raw_commands = yaml.unsafe_load(self.fp)
#
#     def parse(self):
#         for command in self.raw_commands:
#
#
