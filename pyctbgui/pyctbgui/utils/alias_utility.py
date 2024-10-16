from pathlib import Path


def read_alias_file(alias_file):
    with open(alias_file) as fp:
        lines_alias = fp.readlines()
    return parse_alias_lines(lines_alias)


def parse_alias_lines(lines_alias):
    bit_names = [None] * 64
    bit_plots = [None] * 64
    bit_colors = [None] * 64
    adc_names = [None] * 32
    adc_plots = [None] * 32
    adc_colors = [None] * 32
    dac_names = [None] * 18
    sense_names = [None] * 8
    power_names = [None] * 5
    pat_file_name = None

    for line_nr, line in enumerate(lines_alias):
        ignore_list = ['PATCOMPILER']

        # skip empty lines
        if line == '\n' or len(line) == 0:
            continue
        # skip comments
        if line.startswith('#'):
            continue

        cmd, *args = line.split()

        if not args:
            raise Exception(
                f"Alias file parsing failed: Require atleast one argument in addition to command. ({line_nr}:{line})")

        if cmd.startswith("BIT"):
            process_alias_bit_or_adc(cmd, args, bit_names, bit_plots, bit_colors)

        elif cmd.startswith("ADC"):
            process_alias_bit_or_adc(cmd, args, adc_names, adc_plots, adc_colors)

        elif cmd.startswith("DAC"):
            if len(args) > 1:
                raise Exception(f"Too many arguments {len(args)} (expected max: 1) for this type. ({line_nr}:{line})")
            i = int(cmd[3:])
            dac_names[i] = args[0]

        elif cmd.startswith("SENSE"):
            if len(args) > 1:
                raise Exception(f"Too many arguments {len(args)} (expected max: 1) for this type. ({line_nr}:{line})")
            i = int(cmd[5:])
            sense_names[i] = args[0]

        elif cmd in ["VA", "VB", "VC", "VD", "VIO"]:
            if len(args) > 1:
                raise Exception(f"Too many arguments {len(args)} (expected max: 1) for this type. ({line_nr}:{line})")

            match cmd:
                case "VA":
                    i = 0
                case "VB":
                    i = 1
                case "VC":
                    i = 2
                case "VD":
                    i = 3
                case "VIO":
                    i = 4
            power_names[i] = args[0]

        elif cmd == "PATFILE":
            if len(args) > 1:
                raise Exception(f"Too many arguments {len(args)} (expected max: 1) for this type. ({line_nr}:{line})")

            pat_file_name = args[0]
            path = Path(pat_file_name)
            if not path.is_file():
                raise Exception("Pattern file provided in alias file does not exist.<br><br>Pattern file:" +
                                pat_file_name)
        elif cmd in ignore_list:
            pass

        else:
            raise Exception(f"Command: {cmd} not supported. Line {line_nr}:{line}")

    return bit_names, bit_plots, bit_colors, adc_names, adc_plots, adc_colors, dac_names, sense_names, power_names,\
           pat_file_name


def process_alias_bit_or_adc(cmd, args, names, plots, colors):
    n_args = len(args)
    i = int(cmd[3:])
    names[i] = args[0]
    if n_args > 1:
        plots[i] = bool(int(args[1]))
        if n_args > 2:
            colors[i] = args[2]
        if n_args > 3:
            raise Exception(f"Too many arguments {args} (expected max: 3) for this type in line.")
