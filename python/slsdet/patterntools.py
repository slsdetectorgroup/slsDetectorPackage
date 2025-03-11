from . import Detector, Pattern
from .bits import setbit, clearbit
from .format import hexFormat, hexFormat_nox, binFormat, binFormat_nob, decFormat



class pat:
    def __init__(self):
        self.pattern=Pattern()
        self.iaddr=0

    def SB(self,*args):
        for i in args:
            self.pattern.word[self.iaddr]=setbit(i,self.pattern.word[self.iaddr])
        return self.pattern.word[self.iaddr]
 
    def CB(self,*args):
        for i in args:
            self.pattern.word[self.iaddr]=clearbit(i,self.pattern.word[self.iaddr])
        return self.pattern.word[self.iaddr]
    

    def pw(self,verbose=0):
        if verbose==1:
            print(f'{self.iaddr:#06x} {self.pattern.word[self.iaddr]:#018x}') 
        self.pattern.limits[1]=self.iaddr
        #print("pw",self.iaddr,self.pattern.word[self.iaddr])
        self.iaddr+=1
        self.pattern.word[self.iaddr]=self.pattern.word[self.iaddr-1]

    def PW(self,x=1,verbose=0):
        for i in range(x):
            self.pw(verbose)
            
    def REPEAT(self,x,verbose=0):
        for i in range(x):
            self.pw(verbose)

    def PW2(self,verbose=0):
        self.REPEAT(2,verbose)


    def CLOCKS(self,bit,times=1,length=1,verbose=0):
        """
        clocks "bit" n "times", every half clock is long "length"
        lenght is optional, default value is 1
        """
        for i in range(0,times):
            self.SB(bit); self.PW(length,verbose)
            self.CB(bit); self.pw(length,verbose)

    def CLOCK(self,bit,length=1,verbose=0):
        self.CLOCKS(bit,1,length,verbose)

    def serializer(self,value,serInBit,clkBit,nbits,msbfirst=1,length=1):
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
        c=value
        self.CB(serInBit,clkBit)
        self.PW(length) #generate intial line with clk and serIn to 0
        start=0;stop=nbits;step=1
        if msbfirst:
            start=nbits-1;stop=-1;step=-1 #reverts loop if msb has to be pushed in first
            for i in range(start,stop,step):
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
            self.CB(serInBit,clkBit)
            self.PW(length) #generate final line with clk and serIn to 0   


    #NOT IMPLEMENTED YET
    #def setstop():    
    #
    #def setoutput(bit):
    #    self.ioctrl=self.setbit(bit,self.ioctrl)
    #
    #def setinput(bit):
    #    self.ioctrl=self.clearbit(bit,self.ioctrl)
    #
    #def setclk(bit):
    #    self.clkctrl=self.setbit(bit,self.clkctrl)

    # def setinputs(self, *args):
    #    for i in args:
    #       self.setinput(i)

    #def setoutputs(self, *args):
    #   for i in args:
    #      self.setoutput(i)
        
    #def setclks(self, *args):
    #    for i in args:
    #        self.setclk(i)

    def setnloop(self,l,reps):
        self.pattern.nloop[l]=reps
        #print("patnloop",l,reps,self.pattern.nloop[l])

    def setstartloop(self,l):
        self.pattern.loop[l*2]=self.iaddr
        #print("patstart",l,self.iaddr,self.pattern.loop[l*2])

    def setstoploop(self,l):
        self.pattern.loop[l*2+1]=self.iaddr
        #print("patstop",l,self.iaddr,self.pattern.loop[l*2+1])
        

    def setstart(self,l):
        self.pattern.limits[0]=self.iaddr
        #print("start",self.iaddr,self.pattern.limits[0])

    def setstop(self,l):
        self.pattern.limits[1]=self.iaddr
        #print("stop",self.iaddr,self.pattern.limits[1])
        
    def setwaitpoint(self,l):
        self.pattern.wait[l]=self.iaddr
        #print("wait",l,self.iaddr,self.pattern.wait[l])

    def setwaittime(self,l,t):
        self.pattern.waittime[l]=t
        #print("waittime",l,t,self.pattern.waittime[l])

    def setwait(self,l,t):
        self.setwait(l)
        self.setwaittime(l,t)

    
    def patInfo(self):
        print("### SUMMARY OF PATTERN PARAMETERS ###")
        print("Pattern limits (patlimits):",self.pattern.limits) 
        print("Loop:",self.pattern.loop)
        print("Nloop:",self.pattern.nloop)
        print("Wait:",self.pattern.wait)
        print("Waittime",self.pattern.waittime)
        print("Words",self.pattern.word[self.pattern.word>0])
        print("########################################")
          
    def saveToFile(self,fname):
        pwords=''
        for i in range(self.pattern.limits[1]):
            l='patword '+hexFormat(i,4)+' '+hexFormat(self.pattern.word[i],16)+'\n'
            pwords+=l
        for i in range(6):
            l='patloop'+str(i)+' '+hexFormat(self.pattern.loop[i*2],4)+' '+hexFormat(self.pattern.loop[i*2+1],4)+'\n'+'patnloop'+str(i)+' '+str(self.pattern.nloop[i])+'\n'
            pwords+=l
        for i in range(6):
            l='patwait'+str(i)+' '+hexFormat(self.pattern.wait[i],4)+'\n'+'patwaittime'+str(i)+' '+str(self.pattern.waittime[i])+'\n'
            pwords+=l
        
        l='patlimits '+hexFormat(self.pattern.limits[0],4)+' '+hexFormat(self.pattern.limits[1],4)+'\n'
        pwords+=l

        f=open(fname,'w')
        f.write(pwords)
        f.close()
        

    def load(self,det):
        #print("Loading pattern onto detector")
        det.setPattern(self.pattern)