import argparse
import json
from pathlib import Path

# command to generate the ast
# clang version: 14.0.0-1ubuntu1.1
# clang++ -Xclang -ast-dump=json -Xclang -ast-dump-filter -Xclang StringTo  -c ToString.cpp -I ../include/ -std=gnu++11
#


AUTOCOMPLETE_PATH = Path(__file__).parent
DUMP_PATH = AUTOCOMPLETE_PATH / 'dump.json'
FIXED_PATH = AUTOCOMPLETE_PATH / 'fixed.json'

type_values = {
    'special::mv': ["mv", "mV"],
    "special::deg": ["deg"],
    "special::time_unit": ["s", "ms", "us", "ns"],
    "special::hard": ["hard"],
    "special::force-delete-normal-file": ["--force-delete-normal-file"],
    "special::currentSourceFix": ["fix","nofix"],
    "special::currentSourceLow": ["normal","low"]

}
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


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='use parsed c++ code to generate autocomplete snippets')
    parser.add_argument('-f', '--fix', action='store_true', help='fix the parsed ast to make it loadable')
    # parser.add_argument('-p', '--path', type=str, help='output path to the fixed ast', default='ast.json')
    args = parser.parse_known_args()
    if args[0].fix:
        fix_json()
    ret = generate_type_values()
    print(ret)
