#!/usr/bin/env python3
"""
Created on Wed May 24 09:44:53 2017

Plot the pattern for New Chip Test Box (.pat)

Changes:
    - 2017-11-21 Adapt it to python-3
    - 2017-09-25 All can be plotted
    - 2017-09-22 Can be plotted but the loop and wait not work yet

@author: Jiaguo Zhang and Julian Heymes
"""

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.patches import Rectangle


class PlotPattern:

    def __init__(self, pattern, signalNames, colors_plot, colors_wait, linestyles_wait, alpha_wait, alpha_wait_rect,
                 colors_loop, linestyles_loop, alpha_loop, alpha_loop_rect, clock_vertical_lines_spacing,
                 show_clocks_number, line_width):
        self.pattern = pattern
        self.signalNames = signalNames
        self.verbose = False
        # TODO: send alias

        self.colors_plot = colors_plot.copy()
        self.colors_wait = colors_wait.copy()
        self.linestyles_wait = linestyles_wait.copy()
        self.alpha_wait = alpha_wait.copy()
        self.alpha_wait_rect = alpha_wait_rect.copy()
        self.colors_loop = colors_loop.copy()
        self.linestyles_loop = linestyles_loop.copy()
        self.alpha_loop = alpha_loop.copy()
        self.alpha_loop_rect = alpha_loop_rect.copy()
        self.clock_vertical_lines_spacing = clock_vertical_lines_spacing
        self.show_clocks_number = show_clocks_number
        self.line_width = line_width

        self.colors_plot[0] = f'xkcd:{colors_plot[0].lower()}'
        self.colors_plot[1] = f'xkcd:{colors_plot[1].lower()}'

        for i in range(6):
            self.colors_wait[i] = f'xkcd:{colors_wait[i].lower()}'
            self.colors_loop[i] = f'xkcd:{colors_loop[i].lower()}'

        if self.verbose:
            self.printPatViewerParameters()

    def printPatViewerParameters(self):
        print('Pattern Viewer Parameters:')
        print(f'\tcolor1: {self.colors_plot[0]}, color2: {self.colors_plot[1]}')
        print(f"\twait color: {self.colors_wait}")
        print(f"\twait linestyles: {self.linestyles_wait}")
        print(f"\twait alpha: {self.alpha_wait}")
        print(f"\twait alpha rect: {self.alpha_wait_rect}")
        print(f"\tloop color: {self.colors_loop}")
        print(f"\tloop linestyles: {self.linestyles_loop}")
        print(f"\tloop alpha: {self.alpha_loop}")
        print(f"\tloop alpha rect: {self.alpha_loop_rect}")
        print(f'\tclock vertical lines spacing: {self.clock_vertical_lines_spacing}')
        print(f'\tshow clocks number: {self.show_clocks_number}')
        print(f'\tline width: {self.line_width}')
        print('\n')

    def dec2binary(self, dec_num, width=None):
        return np.binary_repr(int(dec_num), width=width)

    def hex2dec(self, string_num):
        return str(int(string_num.upper(), 16))

    def hex2binary(self, string_num, width=None):
        return self.dec2binary(self.hex2dec(string_num.upper()), width=width)

    def patternPlot(self):
        # Define a hex to binary function
        # global definition
        # base = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F]
        self.base = [str(x) for x in range(10)] + [chr(x) for x in range(ord('A'), ord('A') + 6)]

        # Load the pattern and get all lines
        # Loop all lines
        # with open(Folder + "/" + File_pat + ".pat") as f_pat:
        with open(self.pattern) as f_pat:
            lines_pat = f_pat.readlines()

        # number of lines for pattern file
        nlines_pat = len(lines_pat)
        # a counter
        cnt = 0
        if self.verbose:
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
            if len(words_line) < 2:
                continue
            if words_line[0] == "patword":
                # print words_line from b0 to b63
                bits = self.hex2binary(words_line[-1], 64)[::-1]
                if self.verbose:
                    print("The bits for line-", k + 1, "is:", bits)
                # convert string bits to decimal array
                num_bits = np.array(list(map(str, bits)), dtype="uint16")
                if cnt == 0:
                    mat_pat = num_bits
                else:
                    # add bits to matrix
                    mat_pat = np.concatenate((mat_pat, num_bits), axis=0)
                cnt = cnt + 1
                # print("The matrix of pattern:", mat_pat.reshape(int(cnt), int(len(num_bits))))

            # Look at the io: 0 for sending to ASIC, 1 for reading from ASIC
            if words_line[0] == "patioctrl":
                # print words_line
                if self.verbose:
                    print(words_line[-1])
                bits = self.hex2binary(words_line[-1], 64)[::-1]
                if self.verbose:
                    print(bits)
                # convert string bits to decimal array
                self.out_bits = np.array(list(map(str, bits)), dtype="uint16")

            if self.verbose:
                print(words_line)
            # Deal with waiting point

            # ====== WAIT ======
            if words_line[0] == "patwait" and words_line[1] == "0":
                wait0 = int(self.hex2dec(words_line[2]))
                if self.verbose:
                    print("wait 0 at:", wait0)
            if words_line[0] == "patwaittime" and words_line[1] == "0":
                waittime0 = int(words_line[2])
                if self.verbose:
                    print("wait 0 for:", waittime0)

            if words_line[0] == "patwait" and words_line[1] == "1":
                wait1 = int(self.hex2dec(words_line[2]))
                if self.verbose:
                    print("wait 1 at:", wait1)
            if words_line[0] == "patwaittime" and words_line[1] == "1":
                waittime1 = int(words_line[2])
                if self.verbose:
                    print("wait 1 for:", waittime1)

            if words_line[0] == "patwait" and words_line[1] == "2":
                wait2 = int(self.hex2dec(words_line[2]))
                if self.verbose:
                    print("wait 2 at:", wait2)
            if words_line[0] == "patwaittime" and words_line[1] == "2":
                waittime2 = int(words_line[2])
                if self.verbose:
                    print("wait 2 for:", waittime2)

            if words_line[0] == "patwait" and words_line[1] == "3":
                wait3 = int(self.hex2dec(words_line[2]))
                if self.verbose:
                    print("wait 0 at:", wait3)
            if words_line[0] == "patwaittime" and words_line[1] == "3":
                waittime3 = int(words_line[2])
                if self.verbose:
                    print("wait 0 for:", waittime3)

            if words_line[0] == "patwait" and words_line[1] == "4":
                wait4 = int(self.hex2dec(words_line[2]))
                if self.verbose:
                    print("wait 1 at:", wait4)
            if words_line[0] == "patwaittime" and words_line[1] == "4":
                waittime4 = int(words_line[2])
                if self.verbose:
                    print("wait 1 for:", waittime4)

            if words_line[0] == "patwait" and words_line[1] == "5":
                wait5 = int(self.hex2dec(words_line[2]))
                if self.verbose:
                    print("wait 2 at:", wait5)
            if words_line[0] == "patwaittime" and words_line[1] == "5":
                waittime5 = int(words_line[2])
                if self.verbose:
                    print("wait 2 for:", waittime5)

            # ====== LOOPS ======
            if words_line[0] == "patloop" and words_line[1] == "0":
                loop0_start = int(self.hex2dec(words_line[2]))
                loop0_end = int(self.hex2dec(words_line[3]))
                if self.verbose:
                    print("loop 0 start:", loop0_start, ", end:", loop0_end)
            if words_line[0] == "patnloop" and words_line[1] == "0":
                nloop0 = int(words_line[2])
                if self.verbose:
                    print("loop 0 times:", nloop0)

            if words_line[0] == "patloop" and words_line[1] == "1":
                loop1_start = int(self.hex2dec(words_line[2]))
                loop1_end = int(self.hex2dec(words_line[3]))
                if self.verbose:
                    print("loop 1 start:", loop1_start, ", end:", loop1_end)
            if words_line[0] == "patnloop" and words_line[1] == "1":
                nloop1 = int(words_line[2])
                if self.verbose:
                    print("loop 1 times:", nloop1)

            if words_line[0] == "patloop" and words_line[1] == "2":
                loop2_start = int(self.hex2dec(words_line[2]))
                loop2_end = int(self.hex2dec(words_line[3]))
                if self.verbose:
                    print("loop 2 start:", loop2_start, ", end:", loop2_end)
            if words_line[0] == "patnloop" and words_line[1] == "2":
                nloop2 = int(words_line[2])
                if self.verbose:
                    print("loop 2 times:", nloop2)

            if words_line[0] == "patloop" and words_line[1] == "3":
                loop3_start = int(self.hex2dec(words_line[2]))
                loop3_end = int(self.hex2dec(words_line[3]))
                if self.verbose:
                    print("loop 3 start:", loop3_start, ", end:", loop3_end)
            if words_line[0] == "patnloop" and words_line[1] == "3":
                nloop3 = int(words_line[2])
                if self.verbose:
                    print("loop 3 times:", nloop3)

            if words_line[0] == "patloop" and words_line[1] == "4":
                loop4_start = int(self.hex2dec(words_line[2]))
                loop4_end = int(self.hex2dec(words_line[3]))
                if self.verbose:
                    print("loop 4 start:", loop4_start, ", end:", loop4_end)
            if words_line[0] == "patnloop" and words_line[1] == "4":
                nloop4 = int(words_line[2])
                if self.verbose:
                    print("loop 4 times:", nloop4)

            if words_line[0] == "patloop" and words_line[1] == "5":
                loop5_start = int(self.hex2dec(words_line[2]))
                loop5_end = int(self.hex2dec(words_line[3]))
                if self.verbose:
                    print("loop 5 start:", loop5_start, ", end:", loop5_end)
            if words_line[0] == "patnloop" and words_line[1] == "5":
                nloop5 = int(words_line[2])
                if self.verbose:
                    print("loop 5 times:", nloop5)

        # no patioctrl commands read
        if not hasattr(self, 'out_bits'):
            raise Exception("No patioctrl command found in pattern file")
        # print(self.out_bits)

        # internal counter
        avail_index = []
        avail_name = []
        # Remove non-used bits
        for i in range(64):
            # if self.out_bits[0][i] == 1:
            if self.out_bits[i] == 1:
                avail_index.append(i)
                avail_name.append(self.signalNames[i])
        if self.verbose:
            print(avail_index)
            print(avail_name)

        # number of effective used bits
        nbiteff = len(avail_name)

        # subMat = mat_ext[:,index]
        # print(mat_pat.shape)
        subMat = mat_pat.reshape(int(cnt), int(len(num_bits)))[0:, avail_index]
        # subMat = mat_pat[avail_index]
        # timing = np.linspace(0, subMat.shape[0] - 1, subMat.shape[0])
        plt.rcParams['figure.figsize'] = 15, 5

        # ============= PLOTTING =============

        plt.rcParams["font.weight"] = "bold"
        plt.rcParams["axes.labelweight"] = "bold"
        fig, axs = plt.subplots(nbiteff, sharex='all')
        plt.subplots_adjust(wspace=0, hspace=0)
        # axs[nbiteff - 1].set(xlabel='Timing [clk]')
        for idx, i in enumerate(range(nbiteff)):
            axs[idx].tick_params(axis='x', labelsize=6)

            axs[idx].plot(subMat.T[i],
                          "-",
                          drawstyle="steps-post",
                          linewidth=self.line_width,
                          color=self.colors_plot[idx % 2])
            x_additional = range(len(subMat.T[i]) - 1, len(subMat.T[i]) + 2)
            additional_stuff = [subMat.T[i][-1]] * 3

            axs[idx].plot(x_additional,
                          additional_stuff,
                          "--",
                          drawstyle="steps-post",
                          linewidth=self.line_width,
                          color=self.colors_plot[idx % 2],
                          alpha=0.5)
            axs[idx].yaxis.set_ticks([0.5], minor=False)
            axs[idx].xaxis.set_ticks(np.arange(0, len(subMat.T[i]) + 10, self.clock_vertical_lines_spacing))

            axs[idx].yaxis.set_ticklabels([avail_name[i]])
            axs[idx].get_yticklabels()[0].set_color(self.colors_plot[idx % 2])

            axs[idx].grid(1, 'both', 'both', alpha=0.5)
            axs[idx].yaxis.grid(which="both", color=self.colors_plot[idx % 2], alpha=0.2)
            if idx != nbiteff - 1:
                if not self.show_clocks_number:
                    axs[idx].xaxis.set_ticklabels([])
                axs[idx].set(xlabel=' ', ylim=(-0.2, 1.2))
            else:
                axs[idx].set(xlabel='Timing [clk]', ylim=(-0.2, 1.2))
            # axs[idx].set_xlim(left=0)
            axs[idx].set_xlim(left=0, right=len(subMat.T[i]) + 1)
            axs[idx].spines['top'].set_visible(False)
            axs[idx].spines['right'].set_alpha(0.2)
            axs[idx].spines['right'].set_visible(True)
            axs[idx].spines['bottom'].set_visible(False)
            axs[idx].spines['left'].set_visible(False)

            # =====================================================================================================
            # Plot the wait lines
            # Wait 0
            if waittime0 is not None:
                if waittime0 == 0:
                    axs[idx].plot([wait0, wait0], [-10, 10],
                                  linestyle=self.linestyles_wait[0],
                                  color=self.colors_wait[0],
                                  alpha=self.alpha_wait[0],
                                  linewidth=self.line_width)
                    axs[idx].plot([wait0 + 1, wait0 + 1], [-10, 10],
                                  linestyle=self.linestyles_wait[0],
                                  color=self.colors_wait[0],
                                  linewidth=self.line_width,
                                  alpha=self.alpha_wait[0])
                    axs[idx].add_patch(
                        Rectangle((wait0, -10),
                                  1,
                                  20,
                                  label="wait 0: skipped" if idx == 0 else "",
                                  facecolor=self.colors_wait[0],
                                  alpha=self.alpha_wait_rect[0],
                                  hatch='\\\\'))
                else:
                    axs[idx].plot([wait0, wait0], [-10, 10],
                                  linestyle=self.linestyles_wait[0],
                                  color=self.colors_wait[0],
                                  label="wait 0: " + str(waittime0) + " clk" if idx == 0 else "",
                                  linewidth=self.line_width,
                                  alpha=self.alpha_wait[0])

            # Wait 1
            if waittime1 is not None:
                if waittime1 == 0:
                    axs[idx].plot([wait1, wait1], [-10, 10],
                                  linestyle=self.linestyles_wait[1],
                                  color=self.colors_wait[1],
                                  alpha=self.alpha_wait[1],
                                  linewidth=self.line_width)
                    axs[idx].plot([wait1 + 1, wait1 + 1], [-10, 10],
                                  linestyle=self.linestyles_wait[1],
                                  color=self.colors_wait[1],
                                  linewidth=self.line_width,
                                  alpha=self.alpha_wait[1])
                    axs[idx].add_patch(
                        Rectangle((wait1, -10),
                                  1,
                                  20,
                                  label="wait 1: skipped" if idx == 0 else "",
                                  facecolor=self.colors_wait[1],
                                  alpha=self.alpha_wait_rect[1],
                                  hatch='\\\\'))
                else:
                    axs[idx].plot([wait1, wait1], [-10, 10],
                                  linestyle=self.linestyles_wait[1],
                                  color=self.colors_wait[1],
                                  label="wait 1: " + str(waittime1) + " clk" if idx == 0 else "",
                                  linewidth=self.line_width,
                                  alpha=self.alpha_wait[1])

            # Wait 2
            if waittime2 is not None:
                if waittime2 == 0:
                    axs[idx].plot([wait2, wait2], [-10, 10],
                                  linestyle=self.linestyles_wait[2],
                                  color=self.colors_wait[2],
                                  alpha=self.alpha_wait[2],
                                  linewidth=self.line_width)
                    axs[idx].plot([wait2 + 1, wait2 + 1], [-10, 10],
                                  linestyle=self.linestyles_wait[2],
                                  color=self.colors_wait[2],
                                  linewidth=self.line_width,
                                  alpha=self.alpha_wait[2])
                    axs[idx].add_patch(
                        Rectangle((wait2, -10),
                                  1,
                                  20,
                                  label="wait 2: skipped" if idx == 0 else "",
                                  facecolor=self.colors_wait[2],
                                  alpha=self.alpha_wait_rect[2],
                                  hatch='\\\\'))
                else:
                    axs[idx].plot([wait2, wait2], [-10, 10],
                                  linestyle=self.linestyles_wait[2],
                                  color=self.colors_wait[2],
                                  label="wait 2: " + str(waittime2) + " clk" if idx == 0 else "",
                                  linewidth=self.line_width,
                                  alpha=self.alpha_wait[2])

            # Wait 3
            if waittime3 is not None:
                if waittime3 == 0:
                    axs[idx].plot([wait3, wait3], [-10, 10],
                                  linestyle=self.linestyles_wait[3],
                                  color=self.colors_wait[3],
                                  alpha=self.alpha_wait[3],
                                  linewidth=self.line_width)
                    axs[idx].plot([wait3 + 1, wait3 + 1], [-10, 10],
                                  linestyle=self.linestyles_wait[3],
                                  color=self.colors_wait[3],
                                  linewidth=self.line_width,
                                  alpha=self.alpha_wait[3])
                    axs[idx].add_patch(
                        Rectangle((wait3, -10),
                                  1,
                                  20,
                                  label="wait 3: skipped" if idx == 0 else "",
                                  facecolor=self.colors_wait[3],
                                  alpha=self.alpha_wait_rect[3],
                                  hatch='\\\\'))
                else:
                    axs[idx].plot([wait3, wait3], [-10, 10],
                                  linestyle=self.linestyles_wait[3],
                                  color=self.colors_wait[3],
                                  label="wait 3: " + str(waittime3) + " clk" if idx == 0 else "",
                                  linewidth=self.line_width,
                                  alpha=self.alpha_wait[3])

            # Wait 4
            if waittime4 is not None:
                if waittime4 == 0:
                    axs[idx].plot([wait4, wait4], [-10, 10],
                                  linestyle=self.linestyles_wait[4],
                                  color=self.colors_wait[4],
                                  alpha=self.alpha_wait[4],
                                  linewidth=self.line_width)
                    axs[idx].plot([wait4 + 1, wait4 + 1], [-10, 10],
                                  linestyle=self.linestyles_wait[4],
                                  color=self.colors_wait[4],
                                  linewidth=self.line_width,
                                  alpha=self.alpha_wait[4])
                    axs[idx].add_patch(
                        Rectangle((wait4, -10),
                                  1,
                                  20,
                                  label="wait 4: skipped" if idx == 0 else "",
                                  facecolor=self.colors_wait[4],
                                  alpha=self.alpha_wait_rect[4],
                                  hatch='\\\\'))
                else:
                    axs[idx].plot([wait4, wait4], [-10, 10],
                                  linestyle=self.linestyles_wait[4],
                                  color=self.colors_wait[4],
                                  label="wait 4: " + str(waittime4) + " clk" if idx == 0 else "",
                                  linewidth=self.line_width,
                                  alpha=self.alpha_wait[4])

            # Wait 5
            if waittime5 is not None:
                if waittime5 == 0:
                    axs[idx].plot([wait5, wait5], [-10, 10],
                                  linestyle=self.linestyles_wait[5],
                                  color=self.colors_wait[5],
                                  alpha=self.alpha_wait[5],
                                  linewidth=self.line_width)
                    axs[idx].plot([wait5 + 1, wait5 + 1], [-10, 10],
                                  linestyle=self.linestyles_wait[5],
                                  color=self.colors_wait[5],
                                  linewidth=self.line_width,
                                  alpha=self.alpha_wait[5])
                    axs[idx].add_patch(
                        Rectangle((wait5, -10),
                                  1,
                                  20,
                                  label="wait 5: skipped" if idx == 0 else "",
                                  facecolor=self.colors_wait[5],
                                  alpha=self.alpha_wait_rect[5],
                                  hatch='\\\\'))
                else:
                    axs[idx].plot([wait5, wait5], [-10, 10],
                                  linestyle=self.linestyles_wait[5],
                                  color=self.colors_wait[5],
                                  label="wait 5: " + str(waittime5) + " clk" if idx == 0 else "",
                                  linewidth=self.line_width,
                                  alpha=self.alpha_wait[5])

            # =====================================================================================================
            # Plot the loop lines
            # Loop 0
            if nloop0 is not None:
                if nloop0 == 0:
                    axs[idx].plot([loop0_start, loop0_start], [-10, 10],
                                  linestyle=self.linestyles_loop[0],
                                  color=self.colors_loop[0],
                                  alpha=self.alpha_loop[0],
                                  linewidth=self.line_width)
                    axs[idx].plot([loop0_end + 1, loop0_end + 1], [-10, 10],
                                  linestyle=self.linestyles_loop[0],
                                  color=self.colors_loop[0],
                                  alpha=self.alpha_loop[0],
                                  linewidth=self.line_width)
                    axs[idx].add_patch(
                        Rectangle((loop0_start, -10),
                                  loop0_end + 1 - loop0_start,
                                  20,
                                  label="loop 0: skipped" if idx == 0 else "",
                                  facecolor=self.colors_loop[0],
                                  alpha=self.alpha_loop_rect[0],
                                  hatch='//'))
                else:
                    axs[idx].plot([loop0_start, loop0_start], [-10, 10],
                                  linestyle=self.linestyles_loop[0],
                                  color=self.colors_loop[0],
                                  alpha=self.alpha_loop[0],
                                  label="loop 0: " + str(nloop0) + " times" if idx == 0 else "",
                                  linewidth=self.line_width)
                    axs[idx].plot([loop0_end, loop0_end], [-10, 10],
                                  linestyle=self.linestyles_loop[0],
                                  color=self.colors_loop[0],
                                  alpha=self.alpha_loop[0],
                                  linewidth=self.line_width)

            # Loop 1
            if nloop1 is not None:
                if nloop1 == 0:
                    axs[idx].plot([loop1_start, loop1_start], [-10, 10],
                                  linestyle=self.linestyles_loop[1],
                                  color=self.colors_loop[1],
                                  alpha=self.alpha_loop[1],
                                  linewidth=self.line_width)
                    axs[idx].plot([loop1_end + 1, loop1_end + 1], [-10, 10],
                                  linestyle=self.linestyles_loop[1],
                                  color=self.colors_loop[1],
                                  alpha=self.alpha_loop[1],
                                  linewidth=self.line_width)
                    axs[idx].add_patch(
                        Rectangle((loop1_start, -10),
                                  loop1_end + 1 - loop1_start,
                                  20,
                                  label="loop 1: skipped" if idx == 0 else "",
                                  facecolor=self.colors_loop[1],
                                  alpha=self.alpha_loop_rect[1],
                                  hatch='//'))
                else:
                    axs[idx].plot([loop1_start, loop1_start], [-10, 10],
                                  linestyle=self.linestyles_loop[1],
                                  color=self.colors_loop[1],
                                  alpha=self.alpha_loop[1],
                                  label="loop 1: " + str(nloop1) + " times" if idx == 0 else "",
                                  linewidth=self.line_width)
                    axs[idx].plot([loop1_end, loop1_end], [-10, 10],
                                  linestyle=self.linestyles_loop[1],
                                  color=self.colors_loop[1],
                                  alpha=self.alpha_loop[1],
                                  linewidth=self.line_width)

            # Loop 2
            if nloop2 is not None:
                if nloop2 == 0:
                    axs[idx].plot([loop2_start, loop2_start], [-10, 10],
                                  linestyle=self.linestyles_loop[2],
                                  color=self.colors_loop[2],
                                  alpha=self.alpha_loop[2],
                                  linewidth=self.line_width)
                    axs[idx].plot([loop2_end + 1, loop2_end + 1], [-10, 10],
                                  linestyle=self.linestyles_loop[2],
                                  color=self.colors_loop[2],
                                  alpha=self.alpha_loop[2],
                                  linewidth=self.line_width)
                    axs[idx].add_patch(
                        Rectangle((loop2_start, -10),
                                  loop2_end + 1 - loop2_start,
                                  20,
                                  label="loop 2: skipped" if idx == 0 else "",
                                  facecolor=self.colors_loop[2],
                                  alpha=self.alpha_loop_rect[2],
                                  hatch='//'))
                else:
                    axs[idx].plot([loop2_start, loop2_start], [-10, 10],
                                  linestyle=self.linestyles_loop[2],
                                  color=self.colors_loop[2],
                                  alpha=self.alpha_loop[2],
                                  label="loop 2: " + str(nloop2) + " times" if idx == 0 else "",
                                  linewidth=self.line_width)
                    axs[idx].plot([loop2_end, loop2_end], [-10, 10],
                                  linestyle=self.linestyles_loop[2],
                                  color=self.colors_loop[2],
                                  alpha=self.alpha_loop[2],
                                  linewidth=self.line_width)

            # Loop 3
            if nloop3 is not None:
                if nloop3 == 0:
                    axs[idx].plot([loop3_start, loop3_start], [-10, 10],
                                  linestyle=self.linestyles_loop[3],
                                  color=self.colors_loop[3],
                                  alpha=self.alpha_loop[3],
                                  linewidth=self.line_width)
                    axs[idx].plot([loop3_end + 1, loop3_end + 1], [-10, 10],
                                  linestyle=self.linestyles_loop[3],
                                  color=self.colors_loop[3],
                                  alpha=self.alpha_loop[3],
                                  linewidth=self.line_width)
                    axs[idx].add_patch(
                        Rectangle((loop3_start, -10),
                                  loop3_end + 1 - loop3_start,
                                  20,
                                  label="loop 3: skipped" if idx == 0 else "",
                                  facecolor=self.colors_loop[3],
                                  alpha=self.alpha_loop_rect[3],
                                  hatch='//'))
                else:
                    axs[idx].plot([loop3_start, loop3_start], [-10, 10],
                                  linestyle=self.linestyles_loop[3],
                                  color=self.colors_loop[3],
                                  alpha=self.alpha_loop[3],
                                  label="loop 3: " + str(nloop3) + " times" if idx == 0 else "",
                                  linewidth=self.line_width)
                    axs[idx].plot([loop3_end, loop3_end], [-10, 10],
                                  linestyle=self.linestyles_loop[3],
                                  color=self.colors_loop[3],
                                  alpha=self.alpha_loop[3],
                                  linewidth=self.line_width)

            # Loop 4
            if nloop4 is not None:
                if nloop4 == 0:
                    axs[idx].plot([loop4_start, loop4_start], [-10, 10],
                                  linestyle=self.linestyles_loop[4],
                                  color=self.colors_loop[4],
                                  alpha=self.alpha_loop[4],
                                  linewidth=self.line_width)
                    axs[idx].plot([loop4_end + 1, loop4_end + 1], [-10, 10],
                                  linestyle=self.linestyles_loop[4],
                                  color=self.colors_loop[4],
                                  alpha=self.alpha_loop[4],
                                  linewidth=self.line_width)
                    axs[idx].add_patch(
                        Rectangle((loop4_start, -10),
                                  loop4_end + 1 - loop4_start,
                                  20,
                                  label="loop 4: skipped" if idx == 0 else "",
                                  facecolor=self.colors_loop[4],
                                  alpha=self.alpha_loop_rect[4],
                                  hatch='//'))
                else:
                    axs[idx].plot([loop4_start, loop4_start], [-10, 10],
                                  linestyle=self.linestyles_loop[4],
                                  color=self.colors_loop[4],
                                  alpha=self.alpha_loop[4],
                                  label="loop 4: " + str(nloop4) + " times" if idx == 0 else "",
                                  linewidth=self.line_width)
                    axs[idx].plot([loop4_end, loop4_end], [-10, 10],
                                  linestyle=self.linestyles_loop[4],
                                  color=self.colors_loop[4],
                                  alpha=self.alpha_loop[4],
                                  linewidth=self.line_width)

            # Loop 5
            if nloop5 is not None:
                if nloop5 == 0:
                    axs[idx].plot([loop5_start, loop5_start], [-10, 10],
                                  linestyle=self.linestyles_loop[5],
                                  color=self.colors_loop[5],
                                  alpha=self.alpha_loop[5],
                                  linewidth=self.line_width)
                    axs[idx].plot([loop5_end + 1, loop5_end + 1], [-10, 10],
                                  linestyle=self.linestyles_loop[5],
                                  color=self.colors_loop[5],
                                  alpha=self.alpha_loop[5],
                                  linewidth=self.line_width)
                    axs[idx].add_patch(
                        Rectangle((loop5_start, -10),
                                  loop5_end + 1 - loop5_start,
                                  20,
                                  label="loop 5: skipped" if idx == 0 else "",
                                  facecolor=self.colors_loop[5],
                                  alpha=self.alpha_loop_rect[5],
                                  hatch='//'))
                else:
                    axs[idx].plot([loop5_start, loop5_start], [-10, 10],
                                  linestyle=self.linestyles_loop[5],
                                  color=self.colors_loop[5],
                                  alpha=self.alpha_loop[5],
                                  label="loop 5: " + str(nloop5) + " times" if idx == 0 else "",
                                  linewidth=self.line_width)
                    axs[idx].plot([loop5_end, loop5_end], [-10, 10],
                                  linestyle=self.linestyles_loop[5],
                                  color=self.colors_loop[5],
                                  alpha=self.alpha_loop[5],
                                  linewidth=self.line_width)

        n_cols = np.count_nonzero([
            waittime0 != 0, waittime1 != 0, waittime2 != 0, waittime3 != 0, waittime4 != 0, waittime5 != 0, nloop0
            != 0, nloop1 != 0, nloop2 != 0, nloop3 != 0, nloop4 != 0, nloop5 != 0
        ])
        if n_cols > 0:
            fig.legend(loc="upper center", ncol=n_cols)
        return fig
