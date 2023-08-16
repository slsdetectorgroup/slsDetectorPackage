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
        
        #skip empty lines
        if line == '\n' or len(line) == 0:
            continue
        #skip comments
        if line.startswith('#'):
            continue

        words = line.split()
        nwords = len(words)
        if nwords == 1:
            raise Exception(f"Alias file parsing failed: Require atleast 2 arguments in line: {line_nr}:{line}")
              
        cmd = words[0]
        
        if cmd.startswith("BIT"):
            process_alias_bit_or_adc(words, bit_names, bit_plots, bit_colors)
        
        elif cmd.startswith("ADC"):
            process_alias_bit_or_adc(words, adc_names, adc_plots, adc_colors)

        elif cmd.startswith("DAC"):
            i = int(words[0][3:])
            dac_names[i] = words[1]
            if nwords > 2:
                raise Exception("Too many arguments " + str(nwords) + " (expected max: 4) for this type in line <br>" + str(words))
        
        elif cmd.startswith("SENSE"):
            i = int(words[0][5:])
            sense_names[i] = words[1]
            if nwords > 2:
                raise Exception("Too many arguments " + str(nwords) + " (expected max: 4) for this type in line <br>" + str(words))

        elif cmd in ["VA", "VB", "VC", "VD", "VIO"]:
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
            power_names[i] = words[1]                    
            if nwords > 2:
                raise Exception("Too many arguments " + str(nwords) + " (expected max: 4) for this type in line <br>" + str(words))
        
        elif cmd == "PATFILE":
            if nwords > 2:
                raise Exception("Too many arguments " + str(nwords) + " (expected max: 4) for this type in line <br>" + str(words))
            pat_file_name = words[1]
            path = Path(pat_file_name)
            if not path.is_file():
                raise Exception("Pattern file provided in alias file does not exist.<br><br>Pattern file:" + pat_file_name)
        elif cmd in ignore_list:
            pass
        
        else:
            raise Exception(f"Command: {cmd} not supported. Line {line_nr}:{line}")
    
    return bit_names, bit_plots, bit_colors, adc_names, adc_plots, adc_colors, dac_names, sense_names, power_names, pat_file_name


def process_alias_bit_or_adc(words, names, plots, colors):
    nwords = len(words)
    i = int(words[0][3:])
    names[i] = words[1]
    if nwords > 2:
        plots[i] = bool(int(words[2]))
        if nwords > 3:
            colors[i] = words[3]
        if nwords > 4:
            raise Exception(f"Too many arguments {nwords} (expected max: 4) for this type in line. Called with: {words}")
    
