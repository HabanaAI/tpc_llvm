##
# Usage:
# python tpc_builtins_gen.py <path_to_intr_header> <path_to_buitins_def>
#
# Header requirements:
# - polarity argument must be named as polarity or Polarity
# - constant argument must be named with "I_" prefix
# - target-dependent intrinsics must be defined inside #if - #endif directives:
#   #if defined(__greco__) || defined(__goya__) || ....
#    ...
#   #endif
#   Nested directives are not supported
##

from __future__ import print_function
import sys
import re
import argparse


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


# TODO Turn into enum
class Architecture:
    STRING_VALUES = ["goya", "gaudi", "gaudib", "greco", "gaudi2", "doron1"]

    ALTERNATIVE_NAMES = {"dali": "goya", "goya2": "greco"}

    def __init__(self, value):
        if value >= len(Architecture.STRING_VALUES):
            raise Exception("Unknown architecture")
        self.__value = value

    def __hash__(self):
        return hash(self.__value)

    def __eq__(self, other):
        if isinstance(other, Architecture):
            return self.__value == other.__value
        else:
            return False

    @staticmethod
    def from_string(string_value):
        string_value_lower = string_value.lower()
        arch_name = Architecture.ALTERNATIVE_NAMES.get(string_value_lower, string_value_lower)
        index = -1
        for i, name in enumerate(Architecture.STRING_VALUES):
            if arch_name == name:
                index = i
                break

        if index != -1:
            return Architecture(Architecture.STRING_VALUES.index(arch_name))
        else:
            return None

    @staticmethod
    def from_feature_string(feature_string):
        # remobe bracers
        match_result = re.match(r"\(\s*(\w+)(\+?)\s*\)", feature_string)
        result = []
        if match_result:
            arch = Architecture.from_string(match_result.group(1).lower())
            if arch:
                if match_result.group(2):
                    for value in range(arch.__value, len(Architecture.STRING_VALUES)):
                        result.append(Architecture(value))

        return result

    def to_string(self):
        return Architecture.STRING_VALUES[self.__value]

    @staticmethod
    def getAllArchitecture():
        values = []
        for index in range(len(Architecture.STRING_VALUES)):
            values.append(Architecture(index))

        return values


all_targets = Architecture.getAllArchitecture()

clang_type_codes = {
    "bool": "b",
    "char": "s",
    "short": "s",
    "int": "i",
    "float": "f",
    "bf16": "y",
    "half": "h",
    "minifloat": "g",
    "minihalf": "Q",

    "int8_t": "c",
    "uint8_t": "Uc",
    "int16_t": "s",
    "uint16_t": "Us",
    "int32_t": "i",
    "uint32_t": "Ui",
    "int4_t": "c",
    "uint4_t": "Uc",

    "squeeze_cntr" : "i",

    "__global void *": "v*3",
    "__global void **": "v*3*",

    "__global float **": "f*3*",
    "__global bf16 **": "y*3*",
    "__global half **": "h*3*",
    "__global minifloat **": "q*3*",
    "__global minihalf **": "Q*3*",
    "__global int32_t **": "i*3*",
    "__global uint32_t **": "Ui*3*",
    "__global int16_t **": "s*3*",
    "__global uint16_t **": "Us*3*",
    "__global int8_t **": "c*3*",
    "__global uint8_t **": "Uc*3*",
    "__global bool **": "b*3*",

    "int5": "E5i",
    "void": "v",

    "bool64": "V8Uc",
    "bool128": "V16Uc",
    "bool256": "V32Uc",
    "char256": "E256c",
    "uchar256": "E256Uc",
    "short128": "E128s",
    "ushort128": "E128Us",
    "int64": "E64i",
    "uint64": "E64Ui",
    "float64": "E64f",
    "bfloat128": "E128y",
    "half128": "E128h",
    "nibble512": "E256c",
    "unibble512": "E256Uc",
    "minifloat256": "E256g",
    "minihalf256": "E256Q",

    "float128": 'r0',
    "float64_int64": 'r1',
    "float64_uint64": 'r2',

    "int64_float64": 'r3',
    "int128": "r4",
    "int64_uint64": 'r5',

    "uint64_float64": 'r6',
    "uint64_int64": 'r7',
    "uint128": "r8",

    "bfloat256": 'r9',
    "bfloat128_half128": 'r10',
    "bfloat128_short128": 'r11',
    "bfloat128_ushort128": "r12",

    "half128_bfloat128": 'r13',
    "half256": 'r14',
    "half128_short128": 'r15',
    "half128_ushort128": "r16",

    "short128_bfloat128": 'r17',
    "short128_half128": 'r18',
    "short256": 'r19',
    "short128_ushort128": 'r20',

    "ushort128_bfloat128": 'r21',
    "ushort128_half128": 'r22',
    "ushort128_short128": 'r23',
    "ushort256": 'r24',

    "char512": 'r25',
    "char256_uchar256": 'r26',
    "char256_minifloat256": 'r27',
    "char256_minihalf256": 'r28',

    "uchar256_char256": 'r29',
    "uchar512": 'r30',
    "uchar256_minifloat256": 'r31',
    "uchar256_minihalf256": 'r32',

    "uint32_t_pair_t": 'r33',
    "uint16_t_pair_t": 'r34',
    "uint8_t_pair_t": 'r35',
    "int256": 'r36',
    "uint256": 'r37',
    "float256": 'r38',

    "minifloat512": 'r39',
    "minifloat256_minihalf256": 'r40',
    "minifloat256_char256": 'r41',
    "minifloat256_uchar256": 'r42',

    "minihalf512": 'r43',
    "minihalf256_minifloat256": 'r44',
    "minihalf256_char256": 'r45',
    "minihalf256_uchar256": 'r46',
    "int32_t_pair_t": 'r47'
}

predefined_constants = [
    "polarity",
    "switches"
]

has_default = [
    "polarity",
    "predicate",
    "income",
    "switches"
]


def is_structure_type(type):
    return clang_type_codes[type][0] == 'r'


class Intrinsic:
    def __init__(self, name, return_type, operands, targets):
        self.name = name
        self.return_type = return_type
        self.operands = operands
        self.targets = [elem for elem in targets]
        self.targets.sort(key=lambda x: {"goya": 0, "gaudi": 1, "gaudib": 2, "greco": 3, "gaudi2": 4, "doron1": 5}[x])

    def create_clang_definition(self):
        intr_types_string = clang_type_codes[self.return_type]
        first_with_default = None

        for i, op in enumerate(self.operands):
            if op.name == "polarity":
                intr_types_string += 'I'
            #            elif op.is_constant:
            #                intr_types_string += 'I'
            if op.is_unsigned:
                intr_types_string += 'U'
            intr_types_string += clang_type_codes[op.type_name]
            if first_with_default is None and op.def_value is not None:
                first_with_default = i

        return_attr = 'n' if 'void' in self.return_type else 'nc'
        targets_str = "|".join([elem for elem in self.targets])
        if first_with_default is None:
            first_with_default = -1
        if first_with_default < 0:
            return 'TARGET_BUILTIN( %s, "%s", "%s", "%s" )\n' % (self.name, intr_types_string, return_attr, targets_str)
        else:
            return 'TPC_BUILTIN( %s, "%s", "%s", "%s", %i )\n' %\
                   (self.name, intr_types_string, return_attr, targets_str, first_with_default)


class Parameter:
    def __init__(self, name, type_name, is_constant, is_unsigned, def_value=None):
        self.name = name
        self.type_name = type_name
        self.is_constant = is_constant
        self.is_unsigned = is_unsigned
        self.def_value = def_value


class DoxygenDoc:
    class Param:
        def __init__(self, name, description):
            self.name = name
            self.description = description

    class SwitchValue(object):
        def __init__(self, name, description, architectures):
            self.name = name
            self.description = description
            self.architectures = architectures

    class Switche(SwitchValue):
        def __init__(self, name, description, architectures):
            super(DoxygenDoc.Switche, self).__init__(name, description, architectures)

            self.values = []

        def add_value(self, value):
            self.values.append(value)

    def __init__(self, briefs_notation):
        self.briefs_notation = briefs_notation
        self.architectures_notation = []
        self.params_notation = []
        self.switches_notation = []
        self.returns_notation = ""
        self.notes_notation = ""


current_doxygen = None


class DoxygenValidator:
    # TODO turn into enum
    STATE_START = 0
    STATE_BRIEF = 1
    STATE_PARAMS = 2
    STATE_RETURNS = 3
    STATE_SWITCHES = 4
    STATE_SWITCHE_VALUES = 5
    STATE_INTRINSICS_START = 6

    def __init__(self):
        self.has_new_line = False
        self.state = self.STATE_START
        self.next_line_as_appendix = False
        self.previous_switch_space_length = None

    def process_doxygen(self, line, line_num):
        has_end_of_line = False
        global current_doxygen
        if line.endswith("\\n\n"):
            has_end_of_line = True

        if self.next_line_as_appendix:
            match_result = re.match(r"///\s*(.+)$", line)
            if self.state == DoxygenValidator.STATE_BRIEF:
                current_doxygen.briefs_notation += match_result.group(1)
            elif self.state == DoxygenValidator.STATE_PARAMS:
                current_doxygen.params_notation[-1].description += match_result.group(1)
            elif self.state == DoxygenValidator.STATE_SWITCHES:
                current_doxygen.switches_notation[-1].description += match_result.group(1)
            elif self.state == DoxygenValidator.STATE_SWITCHE_VALUES:
                current_doxygen.switches_notation[-1].values[-1].description += match_result(1)
            self.next_line_as_appendix = False

        match_result = re.match(r"///\s*@brief\s+(.+)$", line)
        if match_result:
            if self.state in [DoxygenValidator.STATE_START, DoxygenValidator.STATE_BRIEF]:
                self.state = DoxygenValidator.STATE_BRIEF
                current_doxygen = DoxygenDoc(match_result.group(1))
                self.previous_switch_space_length = None

                self.next_line_as_appendix = has_end_of_line
            else:
                raise Exception("Unexpected @brief at line {}".format(line_num))

        match_result = re.match(r"///\s*@param\s+(\w+)\s+(.+)$", line)
        if match_result:
            if self.state in [DoxygenValidator.STATE_BRIEF, DoxygenValidator.STATE_PARAMS,
                              DoxygenValidator.STATE_RETURNS]:
                self.state = DoxygenValidator.STATE_PARAMS
                current_doxygen.params_notation.append(
                    DoxygenDoc.Param(match_result.group(1), match_result.group(2)))

                self.next_line_as_appendix = has_end_of_line
            else:
                raise Exception("Unexpected @param at line {}".format(line_num))

        match_result = re.match(r"///\s*@return\s+(.+)$", line)
        if match_result:
            if self.state in [DoxygenValidator.STATE_BRIEF, DoxygenValidator.STATE_PARAMS]:
                self.state = DoxygenValidator.STATE_RETURNS

                self.next_line_as_appendix = has_end_of_line
            else:
                raise Exception("Unexpected @return at line {}".format(line_num))

        if self.state in [DoxygenValidator.STATE_SWITCHES, DoxygenValidator.STATE_SWITCHE_VALUES]:
            match_result = re.match(r"///(\s+)-\ \[?(\w+)\]?(?:.*\s*-\s*(.*))?$", line)
            if match_result:
                is_switch = True if self.previous_switch_space_length == None else self.previous_switch_space_length >= len(
                    match_result.group(1))
                switch_name = match_result.group(2)
                switch_description = "" if match_result.group(3) == None else match_result.group(3)
                architectures = None
                for feature in re.findall(r"\(\s*[a-zA-Z0-9+]+\s*\)", line):
                    architectures = Architecture.from_feature_string(feature)
                    if len(architectures) > 0:
                        break
                if is_switch:
                    self.previous_switch_space_length = len(match_result.group(1))
                    self.state = DoxygenValidator.STATE_SWITCHES

                    current_doxygen.switches_notation.append(
                        DoxygenDoc.Switche(switch_name, switch_description, architectures))
                else:
                    self.state = DoxygenValidator.STATE_SWITCHE_VALUES
                    current_doxygen.switches_notation[-1].values.append(
                        DoxygenDoc.SwitchValue(switch_name, switch_description, architectures))

        match_result = re.match(r"///\s*Allowed switches are:\s*$", line)
        if match_result:
            if self.state in [DoxygenValidator.STATE_PARAMS, DoxygenValidator.STATE_RETURNS]:
                self.state = DoxygenValidator.STATE_SWITCHES

                self.next_line_as_appendix = has_end_of_line
            else:
                raise Exception("Unexpected allowed switches at line {}".format(line_num))

        match_result = re.match(r"///\s*@\{\s*$", line)
        if match_result:
            if self.state in [DoxygenValidator.STATE_PARAMS, DoxygenValidator.STATE_RETURNS,
                              DoxygenValidator.STATE_SWITCHES, DoxygenValidator.STATE_SWITCHE_VALUES]:
                self.state = DoxygenValidator.STATE_INTRINSICS_START

                self.next_line_as_appendix = has_end_of_line
            else:
                raise Exception("Unexpected @{{ at line {}".format(line_num))

        match_result = re.match(r"///\s*@\}\s*$", line)
        if match_result:
            if self.state == self.STATE_INTRINSICS_START:
                self.state = self.STATE_START

                self.next_line_as_appendix = has_end_of_line
            else:
                raise Exception("Unexpected @}} at line {}".format(line_num))


def process_comments(line, line_num, output):
    # Possible, doxygen comments
    if line.startswith("///"):
        doxygen_validator.process_doxygen(line, line_num)
    # File description comments
    elif line.startswith("//-"):
        pass
    # General comments
    elif line.startswith("//"):
        if line[2:].isspace():
            output.append("\n")
        else:
            output.append(line)


doxygen_validator = DoxygenValidator()


class ParseError(Exception):
    def __init__(self, message, line=None, previous=None):
        super(ParseError, self).__init__()
        self.line = line
        self.message = message
        self.previous = previous


def parse_intrinsic_declaration(line, targets):
    if line.startswith('__global void *'):
        return_type = '__global void *'
    else:
        return_type = line[:line.find(' ')]
    if return_type not in clang_type_codes.keys():
        raise ParseError("Invalid return type '{}'".format(return_type))

    line = line[len(return_type):line.find(')')].strip()

    name, param_str = line.split('(')
    name = name.strip()
    param_str = re.sub(r'[\s]+', ' ', param_str)
    param_str = re.sub(', ', ',', param_str)
    param_str = param_str.lstrip(");\n")
    params = param_str.split(',') if param_str else ''
    ops = []
    first_param_with_default = None

    for i, param_def in enumerate(params):
        if param_def == "":
            raise ParseError("Invalid parameter declaration")

        # Default value
        def_value = None
        equ_pos = param_def.find('=')
        if equ_pos > 0:
            def_value = param_def[equ_pos+1:].replace(' ', '')
            param_def = param_def[:equ_pos].strip()
            if not first_param_with_default:
                first_param_with_default = i

        # Parameter name
        name_pos = re.search(r'\w+$', param_def)
        if not name_pos:
            raise ParseError("missing parameter name")
        parameter_name = name_pos.group(0)
        if parameter_name in clang_type_codes.keys():
            raise ParseError("missing parameter name")
        param_def = param_def[:name_pos.start(0)].strip()

        # Parameter type
        if not param_def:
            raise ParseError("missing parameter type")
        is_constant = False
        is_unsigned = False
        if param_def.startswith("const"):
            is_constant = True
            param_def = param_def[len("const"):].strip()
        if param_def.startswith("unsigned"):
            is_unsigned = True
            param_def = param_def[len("unsigned"):].strip()
        if param_def.startswith("__global"):
            if not param_def.endswith("*"):
                raise ParseError("Invalid use of __global: it must apply to pointers only")
        param_type = param_def
        if not is_constant and parameter_name in predefined_constants:
            is_constant = True

        if def_value:
            if param_type == 'bool':
                if def_value != '0' and def_value != '1':
                    if parameter_name != 'income' or (def_value != '{}' and def_value != '{0}'):
                        raise ParseError("Default argument of boolean parameter '{}' must be 1 or 0".format(parameter_name))
            elif param_type == 'int':
                if def_value != '0':
                    raise ParseError("Default argument of integer parameter '{}' must be 0".format(parameter_name))
            elif is_structure_type(param_type):
                if def_value != '{}' and def_value != '{0}':
                    raise ParseError("Invalid default argument of parameter '{}'".format(parameter_name))
            if parameter_name == 'polarity':
                if def_value != '0':
                    raise ParseError("Default argument of 'polarity' must be 0".format(parameter_name))
            if parameter_name == 'predicate':
                if def_value != '1':
                    raise ParseError("Default argument of 'predicate' must be 1".format(parameter_name))
            if parameter_name == 'switches':
                if def_value != '0':
                    raise ParseError("Default argument of 'switches' must be 0".format(parameter_name))
        elif first_param_with_default is not None:
            raise ParseError("Missed default argument in parameter '{}'".format(parameter_name))

        ops.append(Parameter(parameter_name, param_type, is_constant, is_unsigned, def_value))
    return Intrinsic(name, return_type, ops, targets)


def parse_preprocessor_directive(expr):
    # expr := group ( '||' group )*
    # group := term ( '&&' term)*
    # term := ( '!' )? item
    # item := atom
    #      |  '(' expr ')'
    # atom := 'defined' '(' identifier ')'

    all_targets = set(["goya", "gaudi", "gaudib", "greco", "gaudi2", "doron1"])
    valid_defines = {
        "__dali__": set(["goya"]),
        "__goya__": set(["goya"]),
        "__gaudi__": set(["gaudi"]),
        "__gaudib__": set(["gaudib"]),
        "__goya2__": set(["greco"]),
        "__greco__": set(["greco"]),
        "__gaudi2__": set(["gaudi2"]),
        "__doron1__": set(["doron1"]),
        "__gaudi_plus__": set(["gaudi", "gaudib", "greco", "gaudi2", "doron1"]),
        "__goya2_plus__": set(["greco", "gaudi2", "doron1"]),
        "__greco_plus__": set(["greco", "gaudi2", "doron1"]),
        "__gaudi2_plus__": set(["gaudi2", "doron1"]),
    }

    def parse_atom(line):
        line = line.lstrip()
        if not line.startswith('defined'):
            raise ParseError("Expected 'defined'", line)
        line = line[len('defined'):].lstrip()
        if not line.startswith('('):
            raise ParseError("Expected '('", line)
        line = line[1:].lstrip()
        m = re.match("[_A-Za-z][_A-Za-z0-9]*", line)
        if not m:
            raise ParseError("Expected identifier", line)
        id = m.string[m.start():m.end()]
        if id not in valid_defines:
            raise ParseError("Unknown define: {}".format(id), line)
        line = line[len(id):].lstrip()
        if not line.startswith(')'):
            raise ParseError("Expected ')'", line)
        return line[1:], valid_defines[id]

    def parse_item(line):
        line = line.lstrip()
        if line.startswith('('):
            (line, targets) = parse_expr(line[1:])
            if not line.lstrip().startswith(')'):
                raise ParseError("Expected ')'", line)
            return line[1:], targets
        else:
            return parse_atom(line)

    def parse_term(line):
        line = line.lstrip()
        inverted = False
        if line.startswith('!'):
            inverted = True
            line = line[1:].lstrip()
        (line, targets) = parse_item(line)
        if inverted:
            targets = all_targets - targets
        return line, targets

    def parse_group(line):
        (line, targets_left) = parse_term(line)
        line = line.lstrip()
        while line.startswith('&&'):
            line = line[2:].lstrip()
            (line, targets_right) = parse_term(line)
            targets_left = targets_left & targets_right
            line = line.lstrip()
        return line, targets_left

    def parse_expr(line):
        (line, targets_left) = parse_group(line)
        line = line.lstrip()
        while line.startswith('||'):
            line = line[2:].lstrip()
            (line, targets_right) = parse_group(line)
            targets_left = targets_left | targets_right
            line = line.lstrip()
        return line, targets_left

    expr = expr.strip()
    if len(expr) == 0:
        raise ParseError("Expected expression", expr)
    (expr, targets) = parse_expr(expr)
    if len(expr.strip()) != 0:
        raise ParseError("Unexpected symbol", expr)
    return targets


def read_intrinsics_header(filename, collect_defaults):
    all_targets = set(["goya", "gaudi", "gaudib", "greco", "gaudi2", "doron1"])

    with open(filename, 'r') as header:
        data = header.readlines()
    output = []
    targets = all_targets
    in_cond_block = False
    start_of_cond = None
    try:
        for i in range(0, len(data)):
            line = data[i]
            if line.isspace():
                continue
            elif line.startswith("//"):
                process_comments(line, i + 1, output)
            elif line.startswith('#'):
                pp_directive = line[1:].strip()
                if re.match("if\s", pp_directive):
                    if in_cond_block:
                        raise ParseError("nested 'if' directives are not supported", pp_directive, start_of_cond)
                    pp_directive = pp_directive[2:].lstrip()
                    targets = parse_preprocessor_directive(pp_directive)
                    in_cond_block = True
                    start_of_cond = i
                elif re.match("endif", pp_directive):
                    if not in_cond_block:
                        raise ParseError("unbalanced 'endif'", pp_directive)
                    targets = all_targets
                    in_cond_block = False
                    start_of_cond = None
                else:
                    raise ParseError("unsupported preprocessor directive", pp_directive)
            elif ';' not in line or "(" not in line or line.lstrip().startswith("//"):
                continue
            else:
                intrinsic = parse_intrinsic_declaration(line, targets)
                if intrinsic is None:
                    continue
                output.append(intrinsic.create_clang_definition())
    except ParseError as err:
        line = data[i]
        if err.line:
            column = len(line) - len(err.line)
            eprint('\n')
            eprint("line", i + 1, "column", column + 1, "error:", err.message)
            eprint("> ", line, end='')
            eprint(column * ' ' + '  ^')
        else:
            eprint("line", i + 1, "error:", err.message)
            eprint("> ", line, end='')
        if err.previous:
            eprint("previous construct is at line", start_of_cond)
            eprint("> ", data[start_of_cond])
        sys.exit(1)
    return output


if len(sys.argv) < 2:
    print("Invalid arguments.\nUsage: python tpc_builtins_gen.py <input_header_file> [<output_builtins_file>]")
    sys.exit(1)

parser = argparse.ArgumentParser()
parser.add_argument("header")
parser.add_argument("definitions", nargs='?')
parser.add_argument("--def", action="store_const", const=True, dest="defargs", help="collect default arguments info")
args = parser.parse_args()

output = read_intrinsics_header(args.header, args.defargs)
if args.definitions:
    outfile = open(args.definitions, 'w')
else:
    outfile = sys.stdout

outfile.write("""// Autogenerated file, do not change it.
#ifndef TPC_BUILTIN
  #define TPC_BUILTIN(n,a,f,t,d) TARGET_BUILTIN(n,a,f,t)
  #define TPC_BUILTIN_DEFINED
#endif

""")
outfile.writelines(output)
outfile.write("""
#ifdef TPC_BUILTIN_DEFINED
  #undef TPC_BUILTIN
  #undef TPC_BUILTIN_DEFINED
#endif
""")

