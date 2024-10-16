import argparse
import json
from pathlib import Path

# command to generate the ast
# clang version: 14.0.0-1ubuntu1.1
# clang++ -Xclang -ast-dump=json -Xclang -ast-dump-filter -Xclang StringTo  -c ToString.cpp -I ../include/ -std=gnu++11
#
import yaml

AUTOCOMPLETE_PATH = Path(__file__).parent
DUMP_PATH = AUTOCOMPLETE_PATH / 'dump.json'
FIXED_PATH = AUTOCOMPLETE_PATH / 'fixed.json'

type_values = {
    'special::mv': ["mv", "mV"],
    "special::deg": ["deg"],
    "special::time_unit": ["s", "ms", "us", "ns"],
    "special::hard": ["hard"],
    "special::force-delete-normal-file": ["--force-delete-normal-file"],
    "special::currentSourceFix": ["fix", "nofix"],
    "special::currentSourceLow": ["normal", "low"],
    "special::path": [],
    "special::pedestal_parameters" : ["", "0"],
    "special::validate": ["--validate"]
}


def get_types(arg_types):
    ret = set()
    for arg_type in arg_types:
        if type_info(arg_type) == 'base':
            if arg_type == 'bool':
                ret  = ret.union(["0", "1"])
        else:
            tmp = [not_list for not_list in type_values[arg_type] if not isinstance(not_list, list)]
            ret = ret.union(tmp)

    #Intercept the options and in case detector specific options appear replace the
    #list of options with a command line call that fetches them
    #TODO! Rename sls_detector_get 
    if "defs::dacIndex" in arg_types:
        return "`sls_detector_get daclist | sed -e 's/.*\[\(.*\)\].*/\\1/' | sed 's/,//g'`"
    elif "defs::detectorSettings" in arg_types:
        return "`sls_detector_get settingslist | sed -e 's/.*\[\(.*\)\].*/\\1/' | sed 's/,//g'`"
    elif "defs::timingMode" in arg_types:
        return "`sls_detector_get timinglist | sed -e 's/.*\[\(.*\)\].*/\\1/' | sed 's/,//g'`"

    
    return ret


def type_info(type_name):
    if type_name.startswith('defs::') or type_name.startswith('slsDetectorDefs::'):
        return 'enum'
    if type_name.startswith('special::'):
        return 'special'
    return 'base'


def get_enum(function):
    return function['type']['qualType'].split(' ')[0]


def get_literal(ifstmt):
    stringliteral = []
    expression = ifstmt['inner'][0]
    if expression['kind'] == 'BinaryOperator':
        if expression['opcode'] == '!=':
            return None, None
        for cxxOperatorCall in expression['inner']:
            if cxxOperatorCall['kind'] == 'CXXOperatorCallExpr':
                implicitCastExpr = cxxOperatorCall['inner'][2]
                stringliteral.append(implicitCastExpr['inner'][0]['value'][1:-1])
    else:
        cxxOperatorCall = expression
        implicitCastExpr = cxxOperatorCall['inner'][2]
        stringliteral = implicitCastExpr['inner'][0]['value'][1:-1]

    retstmt = get_object_by_kind(ifstmt['inner'], 'ReturnStmt')
    declrefexpt = get_object_by_kind(retstmt['inner'], 'DeclRefExpr')
    enum_val = declrefexpt["referencedDecl"]["name"]

    return enum_val, stringliteral


def get_object_by_kind(inner, kind, position=1):
    for obj in inner:
        if obj['kind'] == kind:
            position -= 1
            if position == 0:
                return obj
    return None


def generate_type_values():
    functions = json.loads(FIXED_PATH.read_text())
    for function in functions:
        if function['kind'] != 'FunctionDecl' or function['name'] != 'StringTo':
            continue
        enum = get_enum(function)

        if not enum.startswith('defs::'):
            continue
        # if enum != 'defs::dacIndex':
        #     continue
        if not function['loc']['file'].endswith('ToString.cpp'):
            continue

        compound_stmt = get_object_by_kind(function['inner'], 'CompoundStmt')

        for ifstmt in compound_stmt['inner']:
            if ifstmt['kind'] != 'IfStmt':
                continue
            enum_val, stringliteral = get_literal(ifstmt)
            if enum_val is None:
                continue

            if enum not in type_values or type_values[enum] is None:
                type_values[enum] = []
            type_values[enum].append(stringliteral)
    items = list(type_values.items())
    for key, val in items:
        if key.startswith('defs::'):
            new_key = key.split('::')[1]
            new_key = 'slsDetectorDefs::' + new_key
            type_values[new_key] = val
        elif key.startswith('slsDetectorDefs::'):
            new_key = key.split('::')[1]
            new_key = 'defs::' + new_key
            type_values[new_key] = val

    return json.dumps(type_values, indent=2)


def fix_json():
    with DUMP_PATH.open('r') as f:
        tmp = '[\n'
        for line in f.read().split('\n'):
            if line.startswith('}'):
                tmp += line + ',\n'
            else:
                tmp += line + '\n'
        tmp = tmp[:-3] + '\n]'
    with FIXED_PATH.open('w') as f:
        f.write(tmp)


def generate_bash_autocomplete(output_path=Path(__file__).parent / 'bash_autocomplete.sh', input_path=Path(__file__).parent / 'bash_autocomplete.in.sh'):
    generate_type_values()
    output_file = output_path.open('w')
    template_file = input_path.open('r')

    def writeline(line):
        output_file.write(line + '\n')

    class if_block:
        def __init__(self, condition):
            self.condition = condition

        def __enter__(self):
            output_file.write('if [[ ' + self.condition + ' ]]; then\n')

        def __exit__(self, type, value, traceback):
            output_file.write('fi\n')

    class function:
        def __init__(self, name):
            self.name = name

        def __enter__(self):
            output_file.write(self.name + '() {\n')

        def __exit__(self, type, value, traceback):
            output_file.write('}\n')

    command_path = Path(__file__).parent.parent / 'extended_commands.yaml'
    commands = yaml.unsafe_load(command_path.open('r'))

    for line in template_file:
        if '-- THIS LINE WILL BE REPLACED WITH GENERATED CODE --' not in line:
            output_file.write(line)
            continue
        writeline(f'local SLS_COMMANDS=" {" ".join(commands.keys())} "')
        # generate functions
        for command_name, command in commands.items():
            # added for debugging
            if command_name == 'xxxexptime':
                continue
            with function('__' + command_name):
                writeline('FCN_RETURN=""')

                actions = ['GET', 'PUT']
                for action in actions:
                    if action in command['actions'] and 'args' in command['actions'][action]:
                        args = command['actions'][action]['args']
                        possible_argc = {}
                        for arg in args:
                            if arg['argc'] == 0:
                                pass
                            for i in range(arg['argc']):
                                if i + 1 not in possible_argc:
                                    possible_argc[i + 1] = []
                                possible_argc[i + 1].append(arg['arg_types'][i])
                    if possible_argc:
                        with if_block(f'${{IS_GET}} -eq {"1" if action == "GET" else "0"}'):
                            for argc in possible_argc:
                                with if_block(f'"${{cword}}" == "{argc + 1}"'):
                                    if "defs::detectorSettings" in possible_argc[argc]:
                                        print(argc, command_name, possible_argc[argc])
                                    choices = get_types(possible_argc[argc])

                                    #check if we got choices back or a bash command
                                    if isinstance(choices, (list,set)):
                                        writeline(f'FCN_RETURN="{" ".join(sorted(choices))}"')
                                    else:
                                        writeline(f'FCN_RETURN="{choices}"')
                                    if 'special::path' in possible_argc[argc]:
                                        writeline('IS_PATH=1')

                writeline('return 0')



    output_file.close()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='use parsed c++ code to generate autocomplete snippets')
    parser.add_argument('-f', '--fix', action='store_true', help='fix the parsed ast to make it loadable')
    # parser.add_argument('-p', '--path', type=str, help='output path to the fixed ast', default='ast.json')
    args = parser.parse_known_args()
    if args[0].fix:
        fix_json()
    ret = generate_type_values()
    print(ret)
