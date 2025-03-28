Pattern
========================

This is a test and development feature implemented only for Ctb, Xilinx_Ctb and Mythen3.

A pattern is a sequence of 64-bit words which is executed using a clock on the FPGA. Each of the 64 bits is connected to a pin or internal signal of the FPGA. The purpose of a pattern is to provide a way to change these 64 signals with precise timing. Commands run by the detector server could manipulate the same signals as the pattern, but they cannot enforce a change in a specific clock cycle.

**Usage**

A pattern is written to memory on the FPGA using the patword command.

.. code-block::

   patword 0x0000 0x000000000000000A
   patword 0x0001 0x000000000000000B
   patword 0x0002 0x000000000000000C
   patword 0x0003 0x000000000000000D
   patword 0x0004 0x000000000000000E

The example above writes a five-word pattern into FPGA memory. The first argument is the memory address, the second argument is the content to be written into this address. Before executing a pattern one has to set the address limits of the pattern:

.. code-block::

   patlimits 0x0000 0x0004

This instructs the firmware to execute the commands from address 0 to 4 (including 0 and 4). The execution can be started from the GUI or with the commands

.. code-block::

   start [Ctb, Xilinx_Ctb]
   patternstart [Mythen3]

The maximal number of patword addresses is 8192. However, it is possible to extend the length of the pattern sequence using loops and wait commands. Loops can be configured with the following commands:

.. code-block::

   patloop 0 0x0001 0x0003
   patnloop 0 7

The first argument of both commands is the ID of the loop. Ctb and Xilinx_Ctb can have 6 loops (ID 0-5), Mythen3 can have 4 loop definitions. The commands above configure the loop with ID 0 to run 7 times and jump from the patword with address 3 to the patword with address 1. Important: If patnloop is set to 1, the addresses 0x1-0x3 will execute exactly once; if it is set to 0, the pattern addresses will be skipped.

The same idea is used to introduce wait times. The example below causes the patword at address 0x0002 to be active for 9 clock cycles before the execution continues.

.. code-block::

   patwait 0 0x0002
   patwaittime 0 9

Waits can be placed inside a loop and loops can be nested.

**patioctrl**

The function of each bit in the sequence of 64-bit words depends on the connected detector and firmware version. Some of the 64 bits might connect directly to pads of a chip. The patioctrl command is used to configure the direction of some of these signals (not all of them !! See tables below). Signals where the corresponding bit in the argument of patioctrl is set to 1 will be driven from the FPGA.

**patsetbit and patmask**

The functions patsetbit and patmask can be used to ignore a specific bit of the pattern.
Example:

.. code-block::

   patmask 0x0101
   patsetbit 0x0001

Patmask configures bit 0 and 8 of the pattern to be set to their value in patsetbit. These bits will be ignored during pattern execution and will always be 0 (bit 8) and 1 (bit 0).

The mappings of bit positions in the pattern word to signals/pads of the FPGA are listed below for the three detector types where patterns are used. In the case of the two CTB's, connections of the signals to actual pads of a chip depend on the layout of the used detector adapter board. Therefore, each type of detector adapter board adds an additional mapping layer.

**CTB Pattern Bit Mapping**

.. table:: 

   +----+---+------+----+----------+-------------------+----------------+
   | 63 | 62| 61-57| 56 |  55-48   |  47-32            |  31-0          |
   +----+---+------+----+----------+-------------------+----------------+
   |  A |  D|  --- |  T | EXTIO    | DO, stream source | DIO            |
   +----+---+------+----+----------+-------------------+----------------+

DIO: Driving the 32 FPGA pins corresponding to the lowest 32 bits of the patioctrl command. If bits in patioctrl are 0, the same bit positions in DIO will switch to input pins and connect to dbit sampling. Additionally, some of these 32 bits have an automatic override by detector-specific statemachines which is active whenever these sm's are running (currently bits 7,8,11,14 and 20).

DO: Directly connected to 16 FPGA pins. Output only. Not influenced by patioctrl. Also connected to bit 47-32 in all Ctb dbit samples. All of them can be used as dbit sample trigger. In addition, every bit of DO can be selected as trigger for sending out a udp packet with samples to the receiver.

EXTIO: Similar to DIO, but not used as input to the fpga. With the corresponding patioctrl bits set to 0 these pins will switch to a high impedance mode and be ignored by the firmware.

T: trigger output

D: enable signal for digital sampling

A: adc enable

**Xilinx_CTB Pattern Bit Mapping**

.. table:: 

   +-------+----------------+
   | 63-32 |  31-0          |
   +-------+----------------+
   |  ---  | DIO            |
   +-------+----------------+

DIO: Driving the 32 FPGA pins corresponding to the lowest 32 bits of the patioctrl command. If bits in patioctrl are 0, the same bit positions in DIO will switch to input pins and connect to dbit sampling. Additionally, some of these 32 bits have an automatic override by detector-specific statemachines which is active whenever these sm's are running (currently bits 7,8,11,14 and 20).


**Mythen3 Pattern Bit Mapping**

.. table:: 

   +-------+--------+-------+--------+------------+----------+----------+-----+-----+
   | 63-33 |  32    | 31-25 |  24    | 23         |  22      | 21       | 20  | 19  |
   +-------+--------+-------+--------+------------+----------+----------+-----+-----+
   |  ---  | signARD|  ---  | CHSclk |  cnt_rst   |  sto_rst | STATLOAD | STO | SIN |
   +-------+--------+-------+--------+------------+----------+----------+-----+-----+

.. table:: 

   +---------+-----+-------+-------+----+-------+---------+--------+
   | 18      | 17  | 16-14 | 13    | 12 | 11    | 10      | 9-0    |
   +---------+-----+-------+-------+----+-------+---------+--------+
   | SR_MODE | clk | EN    | PULSE | RD | CHSIN | ANAMode | TBLOAD |
   +---------+-----+-------+-------+----+-------+---------+--------+

For Mythen3 the pattern word only connects to output pins of the FPGA when the pattern is running. Afterwards the signals will switch back to other logic in the FPGA. Both CTB's hold the last executed pattern word until a new pattern is started.