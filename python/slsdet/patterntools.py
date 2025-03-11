from . import Detector, Pattern
from .bits import setbit, clearbit
from .format import hexFormat, hexFormat_nox, binFormat, binFormat_nob, decFormat



class pat:
    """
    Class to generate a pattern for the SLS detector. Intents to as closely as possible 
    mimic the old pattern generation in the C code.
    """
    def __init__(self):
        self.pattern = Pattern()
        self.iaddr = 0

    def SB(self, *args):
        for i in args:
            self.pattern.word[self.iaddr] = setbit(i, self.pattern.word[self.iaddr])
        return self.pattern.word[self.iaddr]
 
    def CB(self, *args):
        for i in args:
            self.pattern.word[self.iaddr] = clearbit(i, self.pattern.word[self.iaddr])
        return self.pattern.word[self.iaddr]
    

    def _pw(self, verbose = False):
        if verbose:
            print(f'{self.iaddr:#06x} {self.pattern.word[self.iaddr]:#018x}') 
        
        self.iaddr += 1
        self.pattern.limits[1] = self.iaddr
        self.pattern.word[self.iaddr] = self.pattern.word[self.iaddr-1]

    def PW(self, x = 1, verbose = False):
        for i in range(x):
            self._pw(verbose)
            
    def REPEAT(self, x, verbose = False):
        for i in range(x):
            self._pw(verbose)

    def PW2(self, verbose = 0):
        self.REPEAT(2, verbose)


    def CLOCKS(self, bit, times = 1, length = 1, verbose = False):
        """
        clocks "bit" n "times", every half clock is long "length"
        length is optional, default value is 1
        """
        for i in range(0, times):
            self.SB(bit); self.PW(length, verbose)
            self.CB(bit); self.PW(length, verbose)

    def CLOCK(self, bit, length = 1, verbose = 0):
        self.CLOCKS(bit, 1, length ,verbose)

    def serializer(self, value, serInBit, clkBit, nbits, msbfirst = True, length = 1):
        """serializer(value,serInBit,clkBit,nbits,msbfirst=1,length=1)
        Produces the .pat file needed to serialize a word into a shift register.
        value: value to be serialized
        serInBit: control bit corresponding to serial in 
        clkBit: control bit corresponding to the clock 
        nbits: number of bits of the target register to load
        msbfirst: if 1 pushes in the MSB first (default), 
        if 0 pushes in the LSB first
        length: length of all the PWs in the pattern
        It produces no output because it modifies directly the members of the class pat via SB and CB"""
        
        c = value
        self.CB(serInBit, clkBit)
        self.PW(length) #generate initial line with clk and serIn to 0
        
        start = 0
        stop = nbits
        step = 1

        if msbfirst:
            start = nbits - 1
            stop = -1
            step =- 1 #reverts loop if msb has to be pushed in first

        for i in range(start, stop, step):
            if c & (1<<i): 
                self.SB(serInBit)
                self.PW(length)
            else:
                self.CB(serInBit)
                self.PW(length)
            self.SB(clkBit)
            self.PW(length)
            self.CB(clkBit)
            self.PW(length)

        self.CB(serInBit, clkBit)
        self.PW(length) #generate final line with clk and serIn to 0   


    #NOT IMPLEMENTED YET
    #TODO! What should setstop do? Or can we remove it? 
    #def setstop():    
    #

    def setoutput(self, bit):
       self.pattern.ioctrl = setbit(bit, self.pattern.ioctrl)
    
    def setinput(self, bit):
       self.pattern.ioctrl= clearbit(bit, self.pattern.ioctrl)
    
    #TODO! What should setclk do? Or can we remove it?
    # def setclk(bit):
    #    self.clkctrl=self.setbit(bit,self.clkctrl)

    def setinputs(self, *args):
       for i in args:
          self.setinput(i)

    def setoutputs(self, *args):
      for i in args:
         self.setoutput(i)
        
    #def setclks(self, *args):
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
        self.pattern.limits[0]=self.iaddr


    def setstop(self,l):
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

    
    def patInfo(self):
        print("### SUMMARY OF PATTERN PARAMETERS ###")
        print("Pattern limits (patlimits):",self.pattern.limits) 
        print("startloop:",self.pattern.startloop)
        print("stoploop:",self.pattern.stoploop)
        print("Nloop:",self.pattern.nloop)
        print("Wait:",self.pattern.wait)
        print("Waittime",self.pattern.waittime)
        print("Words", self.pattern.word[self.pattern.word>0])
        print("########################################")

    def to_lines(self):
        """
        Convert pattern to text representation.
        """
        lines = []

        # write pattern words
        for i in range(self.pattern.limits[0], self.pattern.limits[1], 1):
            lines.append(f'patword {hexFormat(i,4)} {hexFormat(self.pattern.word[i],16)}')

        # write ioctrl and limits (TODO! Should this always be here?)
        lines.append(f'patioctrl {hexFormat(self.pattern.ioctrl,16)}')
        lines.append(f'patlimits {hexFormat(self.pattern.limits[0],4)} {hexFormat(self.pattern.limits[1],4)}')

        # write loop limits
        for i in range(6):
            lines.append(f'patloop {i} {hexFormat(self.pattern.startloop[i],4)} {hexFormat(self.pattern.stoploop[i],4)}')                    
            lines.append(f'patnloop {i} {self.pattern.nloop[i]}')


        # write wait times
        for i in range(6):
            lines.append(f'patwait {i} {hexFormat(self.pattern.wait[i],4)}')
            lines.append(f'patwaittime {i} {self.pattern.waittime[i]}')
        
        

        return lines
    
    def print(self):
        for l in self.to_lines():
            print(l)

    def saveToFile(self, fname):
        with open(fname,'w') as f:
            f.writelines(l + '\n' for l in self.to_lines())
        

    def load(self, det):
        """
        Load the pattern into the detector.
        """
        det.setPattern(self.pattern)