from . import Pattern
from .bits import setbit, clearbit, flipbit
import textwrap


class PatternGenerator:
    """
    Class to generate a pattern for the SLS detector. Intents to as closely as possible
    mimic the old pattern generation in the C code.
    """

    def __init__(self, verbose=False):
        self.pattern = Pattern()
        self.iaddr = 0
        self.verbose = verbose

    def clear_pattern(self):
        """
        Clear the pattern and reset the address to 0.
        """
        self.pattern[:] = 0
        self.iaddr = 0

    def SB(self, *bits):
        """
        Set one or multiple bits. Change will take affect with the next PW.
        """
        for bit in bits:
            self.pattern.word[self.iaddr] = setbit(bit, self.pattern.word[self.iaddr])
        return self.pattern.word[self.iaddr]

    def CB(self, *bits):
        """
        Clear one or multiple bits. Change will take affect with the next PW.
        """
        for bit in bits:
            self.pattern.word[self.iaddr] = clearbit(bit, self.pattern.word[self.iaddr])
        return self.pattern.word[self.iaddr]

    def FB(self, *bits):
        """
        Flip one or multiple bits. Change will take affect with the next PW.
        """
        for bit in bits:
            self.pattern.word[self.iaddr] = flipbit(bit, self.pattern.word[self.iaddr])
        return self.pattern.word[self.iaddr]

    def _pw(self):
        if self.verbose:
            print(f"{self.iaddr:#06x} {self.pattern.word[self.iaddr]:#018x}")

        # Increment the address before the next word since limits are inclusive
        self.pattern.limits[1] = self.iaddr
        self.iaddr += 1
        self.pattern.word[self.iaddr] = self.pattern.word[self.iaddr - 1]

    def PW(self, words=1):
        for _ in range(words):
            self._pw()

    def CLOCKS(self, bits, repeats=1, half_period=1):
        """
        Generate clock pulses on the specified bits.

        Parameters
        ----------
        bit_mask : int
            Bitmask selecting which clock line(s) to pulse.
        repeats : int, optional
            Number of full clock cycles to generate (default 1).
        half_period : int, optional
            Hold time for each half of the cycle (default 1).
        verbose : bool, optional
            If True, print timing/debug info during the wait (default False).
        """
        if isinstance(bits, int):
            bits = [bits]
        for _ in range(repeats):
            self.FB(*bits)
            self.PW(half_period)
            self.FB(*bits)
            self.PW(half_period)

    def serializer(self, value, ser_in_bit, clk_bit, nbits, msb_first=True, length=1):
        """
        Serialize `value` into a shift register via ser_in_bit/clk_bit.

        Parameters
        ----------
        value : int
            Value to serialize.
        ser_in_bit : int
            Control bit corresponding to serial in.
        clk_bit : int
            Control bit corresponding to the clock.
        nbits : int
            Number of bits of the target register to load.
        msb_first : bool, optional
            Push in the MSB first if True (default), else LSB first.
        length : int, optional
            Duration of each PW in the pattern (default 1).
        """
        bit_order = range(nbits - 1, -1, -1) if msb_first else range(nbits)

        self.CB(ser_in_bit, clk_bit)
        self.PW(length)  # initial line with clk and serIn low

        for i in bit_order:
            if value & (1 << i):
                self.SB(ser_in_bit)
            else:
                self.CB(ser_in_bit)
            self.PW(length)
            self.SB(clk_bit)
            self.PW(length)
            self.CB(clk_bit)
            self.PW(length)

        self.CB(ser_in_bit, clk_bit)
        self.PW(length)  # final line with clk and serIn low

    def setoutput(self, *bits):
        """
        Define one or multiple bits as output.
        """
        for bit in bits:
            self.pattern.ioctrl = setbit(bit, self.pattern.ioctrl)

    def setinput(self, *bits):
        """
        Define one or multiple bits as input.
        """
        for bit in bits:
            self.pattern.ioctrl = clearbit(bit, self.pattern.ioctrl)

    # TODO! What should setclk do? Or can we remove it?
    # def setclk(bit):
    #    self.clkctrl=self.setbit(bit,self.clkctrl)

    # def setclks(self, *args):
    #    for i in args:
    #        self.setclk(i)

    def setnloop(self, i, reps):
        self.pattern.nloop[i] = reps

    def setstartloop(self, i):
        """
        Set startloop[i] to the current address.
        """
        self.pattern.startloop[i] = self.iaddr

    def setstoploop(self, i):
        """
        Set stoploop[i] to the current address.
        """
        self.pattern.stoploop[i] = self.iaddr

    def setstart(self):
        """
        Set start of pattern to the current address.
        """
        self.pattern.limits[0] = self.iaddr

    def setstop(self):
        """
        Set stop of pattern to the current address.
        """
        self.pattern.limits[1] = self.iaddr

    def setwaitpoint(self, i):
        """
        Set wait[i] to the current address.
        """
        self.pattern.wait[i] = self.iaddr

    def setwaittime(self, i, t):
        """
        Set waittime[i] to t.
        """
        self.pattern.waittime[i] = t

    def setwait(self, i, t):
        """
        Set wait[i] to the current address and waittime[i] to t.
        """
        self.setwait(i)
        self.setwaittime(i, t)

    def __repr__(self):
        return textwrap.dedent(f"""\
                            PatternGenerator:
                                patlimits: {self.pattern.limits}
                                startloop: {self.pattern.startloop}
                                stoploop: {self.pattern.stoploop}
                                nloop: {self.pattern.nloop}
                                wait: {self.pattern.wait}
                                waittime: {self.pattern.waittime}""")

    def __str__(self):
        return self.pattern.str()

    def export_pattern(self):
        """
        Generate the pattern and return it as a Pattern object.
        """
        return self.pattern.copy()

    def load_pattern(self, fname):
        """Load pattern from text file"""
        iaddr = self.pattern.load(fname)

        # Assume an empty pattern if the first and only word is zero
        if iaddr == 1 and self.pattern.word[0] == 0:
            iaddr = 0
        self.iaddr = iaddr

        # Set (last + 1) word to the last word
        if iaddr > 0:
            self.pattern.word[iaddr] = self.pattern.word[iaddr - 1]
