from pathlib import Path


def read_alias_file(alias_file):
    pathAlias = Path(alias_file)
    if not pathAlias.is_file():
        raise Exception("Alias file does not exist.")
    lines_alias = None
    with open(alias_file) as fp:
        lines_alias = fp.readlines()
    fp.close()
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

    for line in lines_alias:
        words = line.split()
        nwords = len(words)
        # ignore empty lines
        if nwords == 0:
            continue
        # ignore comments
        if words[0][0:1] == '#':
            continue
        # invalid (only command)
        if nwords == 1:
            QtWidgets.QMessageBox.warning(self, "Alias File Fail", "Require atleast 2 arguments in line:<br>" + line + "<br>File: " + self.alias_file, QtWidgets.QMessageBox.Ok)
            return                 
        cmd = words[0]
        #print(f'line: {line}')

        if cmd[:3] == "BIT":
            process_alias_bit_or_adc(words, bit_names, bit_plots, bit_colors)
        
        elif cmd[:3] == "ADC":
            process_alias_bit_or_adc(words, adc_names, adc_plots, adc_colors)

        elif cmd[:3] == "DAC":
            i = int(words[0][3:])
            dac_names[i] = words[1]
            if nwords > 2:
                raise Exception("Too many arguments " + str(nwords) + " (expected max: 4) for this type in line <br>" + str(words))
        
        elif cmd[:5] == "SENSE":
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
            raise Exception("Too many arguments " + str(nwords) + " (expected max: 4) for this type in line <br>" + str(words))
    
