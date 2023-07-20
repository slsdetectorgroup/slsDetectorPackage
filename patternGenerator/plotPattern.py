#!/usr/bin/env python3

"""
Created on Wed May 24 09:44:53 2017

Plot the pattern for New Chip Test Box (.pat)

Changes:
    - 2017-11-21 Adapt it to python-3
    - 2017-09-25 All can be plotted
    - 2017-09-22 Can be plotted but the loop and wait not work yet

@author: zhang_j1
"""
import matplotlib.pyplot as plt
from numpy import *
from matplotlib.pyplot import *
from matplotlib.patches import Rectangle
import os
import argparse

###############################################################################
# COLORS AND LINE STYLES
# alternating colors of the plots (2 needed)
colors_plot = ['tab:blue', 'tab:orange']

# Wait colors and line styles (6 needed from 0 to 5)
colors_wait = ['b', 'g', 'r', 'c', 'm', 'y']
linestyles_wait = ['--', '--', '--', '--', '--', '--']
alpha_wait = [0.5, 0.5, 0.5, 0.5, 0.5, 0.5]
alpha_wait_rect = [0.2, 0.2, 0.2, 0.2, 0.2, 0.2]

# Loop colors and line styles (6 needed from 0 to 5)
colors_loop = ['tab:green', 'tab:red', 'tab:purple', 'tab:brown', 'tab:pink', 'tab:grey']
linestyles_loop = ['-.', '-.', '-.', '-.', '-.', '-.']
alpha_loop = [0.5, 0.5, 0.5, 0.5, 0.5, 0.5]
alpha_loop_rect = [0.2, 0.2, 0.2, 0.2, 0.2, 0.2]

# Display the count of clocks
clock_vertical_lines_spacing = 1
show_clocks_number = True

###############################################################################


# Define a hex to binary function
# global definition
# base = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F]
base = [str(x) for x in range(10)] + [chr(x) for x in range(ord('A'), ord('A')+6)]
# dec2bin


def dec2bin(string_num):
    num = int(string_num)
    mid = []
    while True:
        if num == 0:
            break
        num, rem = divmod(num, 2)
        mid.append(base[rem])
 
    return ''.join([str(x) for x in mid[::-1]])


# dec2binary: better than dec2bin
def dec2binary(dec_num, width=None):
    return binary_repr(int(dec_num), width=width)


# hex2dec
def hex2dec(string_num):
    return str(int(string_num.upper(), 16))


# hex2bin
def hex2bin(string_num):
    return dec2bin(hex2dec(string_num.upper()))


# hex2bin
def hex2binary(string_num, width=None):
    return dec2binary(hex2dec(string_num.upper()), width=width)


parser = argparse.ArgumentParser()

parser.add_argument('-d', '--directory', required = True, help = "Working directory where the pattern is located")
parser.add_argument('-p', '--pattern', required = True, help = "Pattern name")
parser.add_argument('-a', '--alias', help = "Alias name")
parser.add_argument('-v', '--verbose', action='store_true')

args = parser.parse_args()

Folder = args.directory
File_pat = args.pattern
File_alias = args.alias
verbose = args.verbose

# Look at the alias file and generate the lookup table for pin names
# Create a 64 bit look up table
table = []
for i in range(64):
    # for special bit
    if i+1 == 59:
        table.append([str(i+1), "external_trigger"])
    elif i+1 == 63:
        table.append([str(i+1), "adc_enable"])
    elif i+1 == 62:
        table.append([str(i+1), "dbit_enable"])
    else:
        table.append([str(i+1), ""])

# Loop all lines
try:
    with open(Folder + "/" + File_alias + ".alias") as f:
        lines = f.readlines()
        f.close()
        nlines = len(lines)
except:
    nlines = 0

if nlines > 0:
    for i in range(nlines):
        # whether the line is bit definition
        if lines[i][0:3] == "BIT":
            # split words
            words = lines[i].split()
            bit_num = int(words[0][3:])
            table[bit_num][0] = words[0][3:]
            table[bit_num][1] = words[1]
else:
    for i in range(64):
        table[i][0] = i
        table[i][1] = f'BIT#{i}'

if verbose:
    print(table)

# Load the pattern and get all lines
# Loop all lines
if os.path.exists(Folder + "/" + File_pat + ".pat"):
    with open(Folder + "/" + File_pat + ".pat") as f_pat:
        lines_pat = f_pat.readlines()
elif os.path.exists(Folder + "/" + File_pat + ".pyat"):
    with open(Folder + "/" + File_pat + ".pyat") as f_pat:
        lines_pat = f_pat.readlines()
else:
    print("No file found - Check it")
    exit()
f_pat.close()

# number of lines for pattern file
nlines_pat = len(lines_pat)
# a counter
cnt = 0
if verbose:
    print("The total number of lines of pattern:", nlines_pat)

# Loop all lines of pattern
waittime0 = None
waittime1 = None
waittime2 = None
waittime3 = None
waittime4 = None
waittime5 = None

nloop0 = None
nloop1 = None
nloop2 = None
nloop3 = None
nloop4 = None
nloop5 = None

for k in range(nlines_pat):
    # content of line
    words_line = lines_pat[k].split()
    if words_line[0] == "patword":
        # print words_line from b0 to b63
        bits = hex2binary(words_line[-1], 64)[::-1]
        if verbose:
            print("The bits for line-", k+1, "is:", bits)
        # convert string bits to decimal array
        num_bits = array(list(map(str, bits)), dtype="uint16")
        if cnt == 0:
            mat_pat = num_bits
        else:
            # add bits to matrix
            mat_pat = concatenate((mat_pat, num_bits), axis=0)
        cnt = cnt + 1
        # print("The matrix of pattern:", mat_pat.reshape(int(cnt), int(len(num_bits))))
    
    # Look at the io: 0 for sending to ASIC, 1 for reading from ASIC
    if words_line[0] == "patioctrl":
        # print words_line
        if verbose:
            print(words_line[-1])
        bits = hex2binary(words_line[-1], 64)[::-1]
        if verbose:
            print(bits)
        # convert string bits to decimal array
        out_bits = array(list(map(str, bits)), dtype="uint16")
    
    if verbose:
        print(words_line)
    # Deal with waiting point

    # ====== WAIT ======
    if words_line[0] == "patwait" and words_line[1] == "0":
        wait0 = int(hex2dec(words_line[2]))
        if verbose:
            print("wait 0 at:", wait0)
    if words_line[0] == "patwaittime" and words_line[1] == "0":
        waittime0 = int(words_line[2])
        if verbose:
            print("wait 0 for:", waittime0)
        
    if words_line[0] == "patwait" and words_line[1] == "1":
        wait1 = int(hex2dec(words_line[2]))
        if verbose:
            print("wait 1 at:", wait1)
    if words_line[0] == "patwaittime" and words_line[1] == "1":
        waittime1 = int(words_line[2])
        if verbose:
            print("wait 1 for:", waittime1)
        
    if words_line[0] == "patwait" and words_line[1] == "2":
        wait2 = int(hex2dec(words_line[2]))
        if verbose:
            print("wait 2 at:", wait2)
    if words_line[0] == "patwaittime" and words_line[1] == "2":
        waittime2 = int(words_line[2])
        if verbose:
            print("wait 2 for:", waittime2)

    if words_line[0] == "patwait" and words_line[1] == "3":
        wait3 = int(hex2dec(words_line[2]))
        if verbose:
            print("wait 0 at:", wait3)
    if words_line[0] == "patwaittime" and words_line[1] == "3":
        waittime3 = int(words_line[2])
        if verbose:
            print("wait 0 for:", waittime3)
        
    if words_line[0] == "patwait" and words_line[1] == "4":
        wait4 = int(hex2dec(words_line[2]))
        if verbose:
            print("wait 1 at:", wait4)
    if words_line[0] == "patwaittime" and words_line[1] == "4":
        waittime4 = int(words_line[2])
        if verbose:
            print("wait 1 for:", waittime4)
        
    if words_line[0] == "patwait" and words_line[1] == "5":
        wait5 = int(hex2dec(words_line[2]))
        if verbose:
            print("wait 2 at:", wait5)
    if words_line[0] == "patwaittime" and words_line[1] == "5":
        waittime5 = int(words_line[2])
        if verbose:
            print("wait 2 for:", waittime5)

    # ====== LOOPS ======
    if words_line[0] == "patloop" and words_line[1] == "0":
        loop0_start = int(hex2dec(words_line[2]))
        loop0_end = int(hex2dec(words_line[3]))
        if verbose:
            print("loop 0 start:", loop0_start, ", end:", loop0_end)
    if words_line[0] == "patnloop" and words_line[1] == "0":
        nloop0 = int(words_line[2])
        if verbose:
            print("loop 0 times:", nloop0)

    if words_line[0] == "patloop" and words_line[1] == "1":
        loop1_start = int(hex2dec(words_line[2]))
        loop1_end = int(hex2dec(words_line[3]))
        if verbose:
            print("loop 1 start:", loop1_start, ", end:", loop1_end)
    if words_line[0] == "patnloop" and words_line[1] == "1":
        nloop1 = int(words_line[2])
        if verbose:
            print("loop 1 times:", nloop1)

    if words_line[0] == "patloop" and words_line[1] == "2":
        loop2_start = int(hex2dec(words_line[2]))
        loop2_end = int(hex2dec(words_line[3]))
        if verbose:
            print("loop 2 start:", loop2_start, ", end:", loop2_end)
    if words_line[0] == "patnloop" and words_line[1] == "2":
        nloop2 = int(words_line[2])
        if verbose:
            print("loop 2 times:", nloop2)
    
    if words_line[0] == "patloop" and words_line[1] == "3":
        loop3_start = int(hex2dec(words_line[2]))
        loop3_end = int(hex2dec(words_line[3]))
        if verbose:
            print("loop 3 start:", loop3_start, ", end:", loop3_end)
    if words_line[0] == "patnloop" and words_line[1] == "3":
        nloop3 = int(words_line[2])
        if verbose:
            print("loop 3 times:", nloop3)

    if words_line[0] == "patloop" and words_line[1] == "4":
        loop4_start = int(hex2dec(words_line[2]))
        loop4_end = int(hex2dec(words_line[3]))
        if verbose:
            print("loop 4 start:", loop4_start, ", end:", loop4_end)
    if words_line[0] == "patnloop" and words_line[1] == "4":
        nloop4 = int(words_line[2])
        if verbose:
            print("loop 4 times:", nloop4)

    if words_line[0] == "patloop" and words_line[1] == "5":
        loop5_start = int(hex2dec(words_line[2]))
        loop5_end = int(hex2dec(words_line[3]))
        if verbose:
            print("loop 5 start:", loop5_start, ", end:", loop5_end)
    if words_line[0] == "patnloop" and words_line[1] == "5":
        nloop5 = int(words_line[2])
        if verbose:
            print("loop 5 times:", nloop5)
# print(out_bits)

# internal counter
avail_index = []
avail_name = []
# Remove non-used bits
for i in range(64):
    # if out_bits[0][i] == 1:
    if out_bits[i] == 1:
        avail_index.append(i)
        avail_name.append(table[i][1])
if verbose:
    print(avail_index)
    print(avail_name)

# number of effective used bits
nbiteff = len(avail_name)

# subMat = mat_ext[:,index]
# print(mat_pat.shape)
subMat = mat_pat.reshape(int(cnt), int(len(num_bits)))[0:, avail_index]
# subMat = mat_pat[avail_index]
timing = linspace(0, subMat.shape[0]-1, subMat.shape[0])
rcParams['figure.figsize'] = 15, 5


# ============= PLOTTING =============

rcParams["font.weight"] = "bold"
rcParams["axes.labelweight"] = "bold"
fig2, axs2 = subplots(nbiteff, sharex='all')
subplots_adjust(wspace=0, hspace=0)
# axs2[nbiteff - 1].set(xlabel='Timing [clk]')
for idx, i in enumerate(range(nbiteff)):

    axs2[idx].plot(subMat.T[i], "-", drawstyle="steps-post", linewidth=2.0, color=colors_plot[idx % 2])
    x_additional = range(len(subMat.T[i]) - 1, len(subMat.T[i]) + 2)
    additional_stuff = [subMat.T[i][-1]] * 3

    axs2[idx].plot(x_additional, additional_stuff,
                   "--", drawstyle="steps-post", linewidth=2.0, color=colors_plot[idx % 2], alpha=0.5)
    axs2[idx].yaxis.set_ticks([0.5], minor=False)
    axs2[idx].xaxis.set_ticks(arange(0, len(subMat.T[i]) + 10, clock_vertical_lines_spacing))

    axs2[idx].yaxis.set_ticklabels([avail_name[i]])
    axs2[idx].get_yticklabels()[0].set_color(colors_plot[idx % 2])

    axs2[idx].grid(1, 'both', 'both', alpha=0.5)
    axs2[idx].yaxis.grid(which="both", color=colors_plot[idx % 2], alpha=0.2)
    if idx != nbiteff - 1:
        if not show_clocks_number:
            axs2[idx].xaxis.set_ticklabels([])
        axs2[idx].set(xlabel=' ', ylim=(-0.2, 1.2))
    else:
        axs2[idx].set(xlabel='Timing [clk]', ylim=(-0.2, 1.2))
    # axs2[idx].set_xlim(left=0)
    axs2[idx].set_xlim(left=0, right=len(subMat.T[i]) + 1)
    axs2[idx].spines['top'].set_visible(False)
    axs2[idx].spines['right'].set_alpha(0.2)
    axs2[idx].spines['right'].set_visible(True)
    axs2[idx].spines['bottom'].set_visible(False)
    axs2[idx].spines['left'].set_visible(False)

    # =====================================================================================================
    # Plot the wait lines
    # Wait 0
    if waittime0 is not None:
        if waittime0 == 0:
            axs2[idx].plot([wait0, wait0], [-10, 10],
                           linestyle=linestyles_wait[0], color=colors_wait[0], alpha=alpha_wait[0], linewidth=2.0)
            axs2[idx].plot([wait0 + 1, wait0 + 1], [-10, 10],
                           linestyle=linestyles_wait[0], color=colors_wait[0], linewidth=2.0, alpha=alpha_wait[0])
            axs2[idx].add_patch(Rectangle((wait0, -10), 1, 20,
                                          label="wait 0: skipped" if idx == 0 else "",
                                          facecolor=colors_wait[0], alpha=alpha_wait_rect[0], hatch='\\\\'))
        else:
            axs2[idx].plot([wait0, wait0], [-10, 10],
                           linestyle=linestyles_wait[0], color=colors_wait[0],
                           label="wait 0: " + str(waittime0) + " clk" if idx == 0 else "",
                           linewidth=2.0, alpha=alpha_wait[0])

    # Wait 1
    if waittime1 is not None:
        if waittime1 == 0:
            axs2[idx].plot([wait1, wait1], [-10, 10],
                           linestyle=linestyles_wait[1], color=colors_wait[1], alpha=alpha_wait[1], linewidth=2.0)
            axs2[idx].plot([wait1 + 1, wait1 + 1], [-10, 10],
                           linestyle=linestyles_wait[1], color=colors_wait[1], linewidth=2.0, alpha=alpha_wait[1])
            axs2[idx].add_patch(Rectangle((wait1, -10), 1, 20,
                                          label="wait 1: skipped" if idx == 0 else "",
                                          facecolor=colors_wait[1], alpha=alpha_wait_rect[1], hatch='\\\\'))
        else:
            axs2[idx].plot([wait1, wait1], [-10, 10],
                           linestyle=linestyles_wait[1], color=colors_wait[1],
                           label="wait 1: " + str(waittime1) + " clk" if idx == 0 else "",
                           linewidth=2.0, alpha=alpha_wait[1])

    # Wait 2
    if waittime2 is not None:
        if waittime2 == 0:
            axs2[idx].plot([wait2, wait2], [-10, 10],
                           linestyle=linestyles_wait[2], color=colors_wait[2], alpha=alpha_wait[2], linewidth=2.0)
            axs2[idx].plot([wait2 + 1, wait2 + 1], [-10, 10],
                           linestyle=linestyles_wait[2], color=colors_wait[2], linewidth=2.0, alpha=alpha_wait[2])
            axs2[idx].add_patch(Rectangle((wait2, -10), 1, 20,
                                          label="wait 2: skipped" if idx == 0 else "",
                                          facecolor=colors_wait[2], alpha=alpha_wait_rect[2], hatch='\\\\'))
        else:
            axs2[idx].plot([wait2, wait2], [-10, 10],
                           linestyle=linestyles_wait[2], color=colors_wait[2],
                           label="wait 2: " + str(waittime2) + " clk" if idx == 0 else "",
                           linewidth=2.0, alpha=alpha_wait[2])

    # Wait 3
    if waittime3 is not None:
        if waittime3 == 0:
            axs2[idx].plot([wait3, wait3], [-10, 10],
                           linestyle=linestyles_wait[3], color=colors_wait[3], alpha=alpha_wait[3], linewidth=2.0)
            axs2[idx].plot([wait3 + 1, wait3 + 1], [-10, 10],
                           linestyle=linestyles_wait[3], color=colors_wait[3], linewidth=2.0, alpha=alpha_wait[3])
            axs2[idx].add_patch(Rectangle((wait3, -10), 1, 20,
                                          label="wait 3: skipped" if idx == 0 else "",
                                          facecolor=colors_wait[3], alpha=alpha_wait_rect[3], hatch='\\\\'))
        else:
            axs2[idx].plot([wait3, wait3], [-10, 10],
                           linestyle=linestyles_wait[3], color=colors_wait[3],
                           label="wait 3: " + str(waittime3) + " clk" if idx == 0 else "",
                           linewidth=2.0, alpha=alpha_wait[3])

    # Wait 4
    if waittime4 is not None:
        if waittime4 == 0:
            axs2[idx].plot([wait4, wait4], [-10, 10],
                           linestyle=linestyles_wait[4], color=colors_wait[4], alpha=alpha_wait[4], linewidth=2.0)
            axs2[idx].plot([wait4 + 1, wait4 + 1], [-10, 10],
                           linestyle=linestyles_wait[4], color=colors_wait[4], linewidth=2.0, alpha=alpha_wait[4])
            axs2[idx].add_patch(Rectangle((wait4, -10), 1, 20,
                                          label="wait 4: skipped" if idx == 0 else "",
                                          facecolor=colors_wait[4], alpha=alpha_wait_rect[4], hatch='\\\\'))
        else:
            axs2[idx].plot([wait4, wait4], [-10, 10],
                           linestyle=linestyles_wait[4], color=colors_wait[4],
                           label="wait 4: " + str(waittime4) + " clk" if idx == 0 else "",
                           linewidth=2.0, alpha=alpha_wait[4])

    # Wait 5
    if waittime5 is not None:
        if waittime5 == 0:
            axs2[idx].plot([wait5, wait5], [-10, 10],
                           linestyle=linestyles_wait[5], color=colors_wait[5], alpha=alpha_wait[5], linewidth=2.0)
            axs2[idx].plot([wait5 + 1, wait5 + 1], [-10, 10],
                           linestyle=linestyles_wait[5], color=colors_wait[5], linewidth=2.0, alpha=alpha_wait[5])
            axs2[idx].add_patch(Rectangle((wait5, -10), 1, 20,
                                          label="wait 5: skipped" if idx == 0 else "",
                                          facecolor=colors_wait[5], alpha=alpha_wait_rect[5], hatch='\\\\'))
        else:
            axs2[idx].plot([wait5, wait5], [-10, 10],
                           linestyle=linestyles_wait[5], color=colors_wait[5],
                           label="wait 5: " + str(waittime5) + " clk" if idx == 0 else "",
                           linewidth=2.0, alpha=alpha_wait[5])

    # =====================================================================================================
    # Plot the loop lines
    # Loop 0
    if nloop0 is not None:
        if nloop0 == 0:
            axs2[idx].plot([loop0_start, loop0_start], [-10, 10],
                           linestyle=linestyles_loop[0], color=colors_loop[0],
                           alpha=alpha_loop[0], linewidth=2.0)
            axs2[idx].plot([loop0_end + 1, loop0_end + 1], [-10, 10],
                           linestyle=linestyles_loop[0], color=colors_loop[0], alpha=alpha_loop[0], linewidth=2.0)
            axs2[idx].add_patch(Rectangle((loop0_start, -10), loop0_end + 1 - loop0_start, 20,
                                          label="loop 0: skipped" if idx == 0 else "",
                                          facecolor=colors_loop[0], alpha=alpha_loop_rect[0], hatch='//'))
        else:
            axs2[idx].plot([loop0_start, loop0_start], [-10, 10],
                           linestyle=linestyles_loop[0], color=colors_loop[0], alpha=alpha_loop[0],
                           label="loop 0: " + str(nloop0) + " times" if idx == 0 else "", linewidth=2.0)
            axs2[idx].plot([loop0_end, loop0_end], [-10, 10],
                           linestyle=linestyles_loop[0], color=colors_loop[0], alpha=alpha_loop[0], linewidth=2.0)

    # Loop 1
    if nloop1 is not None:
        if nloop1 == 0:
            axs2[idx].plot([loop1_start, loop1_start], [-10, 10],
                           linestyle=linestyles_loop[1], color=colors_loop[1],
                           alpha=alpha_loop[1], linewidth=2.0)
            axs2[idx].plot([loop1_end + 1, loop1_end + 1], [-10, 10],
                           linestyle=linestyles_loop[1], color=colors_loop[1], alpha=alpha_loop[1], linewidth=2.0)
            axs2[idx].add_patch(Rectangle((loop1_start, -10), loop1_end + 1 - loop1_start, 20,
                                          label="loop 1: skipped" if idx == 0 else "",
                                          facecolor=colors_loop[1], alpha=alpha_loop_rect[1], hatch='//'))
        else:
            axs2[idx].plot([loop1_start, loop1_start], [-10, 10],
                           linestyle=linestyles_loop[1], color=colors_loop[1], alpha=alpha_loop[1],
                           label="loop 1: " + str(nloop1) + " times" if idx == 0 else "", linewidth=2.0)
            axs2[idx].plot([loop1_end, loop1_end], [-10, 10],
                           linestyle=linestyles_loop[1], color=colors_loop[1], alpha=alpha_loop[1], linewidth=2.0)

    # Loop 2
    if nloop2 is not None:
        if nloop2 == 0:
            axs2[idx].plot([loop2_start, loop2_start], [-10, 10],
                           linestyle=linestyles_loop[2], color=colors_loop[2],
                           alpha=alpha_loop[2], linewidth=2.0)
            axs2[idx].plot([loop2_end + 1, loop2_end + 1], [-10, 10],
                           linestyle=linestyles_loop[2], color=colors_loop[2], alpha=alpha_loop[2], linewidth=2.0)
            axs2[idx].add_patch(Rectangle((loop2_start, -10), loop2_end + 1 - loop2_start, 20,
                                          label="loop 2: skipped" if idx == 0 else "",
                                          facecolor=colors_loop[2], alpha=alpha_loop_rect[2], hatch='//'))
        else:
            axs2[idx].plot([loop2_start, loop2_start], [-10, 10],
                           linestyle=linestyles_loop[2], color=colors_loop[2], alpha=alpha_loop[2],
                           label="loop 2: " + str(nloop2) + " times" if idx == 0 else "", linewidth=2.0)
            axs2[idx].plot([loop2_end, loop2_end], [-10, 10],
                           linestyle=linestyles_loop[2], color=colors_loop[2], alpha=alpha_loop[2], linewidth=2.0)

    # Loop 3
    if nloop3 is not None:
        if nloop3 == 0:
            axs2[idx].plot([loop3_start, loop3_start], [-10, 10],
                           linestyle=linestyles_loop[3], color=colors_loop[3],
                           alpha=alpha_loop[3], linewidth=2.0)
            axs2[idx].plot([loop3_end + 1, loop3_end + 1], [-10, 10],
                           linestyle=linestyles_loop[3], color=colors_loop[3], alpha=alpha_loop[3], linewidth=2.0)
            axs2[idx].add_patch(Rectangle((loop3_start, -10), loop3_end + 1 - loop3_start, 20,
                                          label="loop 3: skipped" if idx == 0 else "",
                                          facecolor=colors_loop[3], alpha=alpha_loop_rect[3], hatch='//'))
        else:
            axs2[idx].plot([loop3_start, loop3_start], [-10, 10],
                           linestyle=linestyles_loop[3], color=colors_loop[3], alpha=alpha_loop[3],
                           label="loop 3: " + str(nloop3) + " times" if idx == 0 else "", linewidth=2.0)
            axs2[idx].plot([loop3_end, loop3_end], [-10, 10],
                           linestyle=linestyles_loop[3], color=colors_loop[3], alpha=alpha_loop[3], linewidth=2.0)

    # Loop 4
    if nloop4 is not None:
        if nloop4 == 0:
            axs2[idx].plot([loop4_start, loop4_start], [-10, 10],
                           linestyle=linestyles_loop[4], color=colors_loop[4],
                           alpha=alpha_loop[4], linewidth=2.0)
            axs2[idx].plot([loop4_end + 1, loop4_end + 1], [-10, 10],
                           linestyle=linestyles_loop[4], color=colors_loop[4], alpha=alpha_loop[4], linewidth=2.0)
            axs2[idx].add_patch(Rectangle((loop4_start, -10), loop4_end + 1 - loop4_start, 20,
                                          label="loop 4: skipped" if idx == 0 else "",
                                          facecolor=colors_loop[4], alpha=alpha_loop_rect[4], hatch='//'))
        else:
            axs2[idx].plot([loop4_start, loop4_start], [-10, 10],
                           linestyle=linestyles_loop[4], color=colors_loop[4], alpha=alpha_loop[4],
                           label="loop 4: " + str(nloop4) + " times" if idx == 0 else "", linewidth=2.0)
            axs2[idx].plot([loop4_end, loop4_end], [-10, 10],
                           linestyle=linestyles_loop[4], color=colors_loop[4], alpha=alpha_loop[4], linewidth=2.0)

    # Loop 5
    if nloop5 is not None:
        if nloop5 == 0:
            axs2[idx].plot([loop5_start, loop5_start], [-10, 10],
                           linestyle=linestyles_loop[5], color=colors_loop[5],
                           alpha=alpha_loop[5], linewidth=2.0)
            axs2[idx].plot([loop5_end + 1, loop5_end + 1], [-10, 10],
                           linestyle=linestyles_loop[5], color=colors_loop[5], alpha=alpha_loop[5], linewidth=2.0)
            axs2[idx].add_patch(Rectangle((loop5_start, -10), loop5_end + 1 - loop5_start, 20,
                                          label="loop 5: skipped" if idx == 0 else "",
                                          facecolor=colors_loop[5], alpha=alpha_loop_rect[5], hatch='//'))
        else:
            axs2[idx].plot([loop5_start, loop5_start], [-10, 10],
                           linestyle=linestyles_loop[5], color=colors_loop[5], alpha=alpha_loop[5],
                           label="loop 5: " + str(nloop5) + " times" if idx == 0 else "", linewidth=2.0)
            axs2[idx].plot([loop5_end, loop5_end], [-10, 10],
                           linestyle=linestyles_loop[5], color=colors_loop[5], alpha=alpha_loop[5], linewidth=2.0)


n_cols = count_nonzero([waittime0 != 0, waittime1 != 0, waittime2 != 0, waittime3 != 0, waittime4 != 0, waittime5 != 0,
                        nloop0 != 0, nloop1 != 0, nloop2 != 0, nloop3 != 0, nloop4 != 0, nloop5 != 0])
if n_cols > 0:
    fig2.legend(loc="upper center", ncol=n_cols)
# manager = get_current_fig_manager()
# manager.window.showMaximized()

figure = plt.gcf()  # get current figure
figure.set_size_inches(20, 10)
# when saving, specify the DPI
# tight_layout()
plt.savefig(Folder+"/"+File_pat+".png", dpi=300)

# Remove the white space around the plot -- only works on Unix (ImageMagick command)
os.system(f'mogrify -trim {Folder}/{File_pat}.png')

show()
