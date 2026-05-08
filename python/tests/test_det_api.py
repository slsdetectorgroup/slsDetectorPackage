import pytest, sys, traceback

from pathlib import Path
current_dir = Path(__file__).resolve().parents[2]
scripts_dir = current_dir / "tests" / "scripts"
sys.path.append(str(scripts_dir))
print(sys.path)

from utils_for_test import (
    Log,
    LogLevel,
)
from slsdet import Detector

from slsdet._slsdet import slsDetectorDefs

detectorType = slsDetectorDefs.detectorType

@pytest.fixture(
    scope="session", 
    params=['ctb', 'xilinx_ctb', 'mythen3', 'jungfrau']
)
def simulator(request):
    """Fixture to start the detector server once and clean up at the end."""
    det_name = request.param
    num_mods = 1
    fp = sys.stdout

@pytest.mark.detectorintegration
def test_define_reg(session_simulator, request):
    """ Test setting define_reg for ctb and xilinx_ctb."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    from slsdet import RegisterAddress

    if det_type in ['ctb', 'xilinx_ctb']:
        prev_reg_defs = d.getRegisterDefinitions()
        prev_bit_defs = d.getBitDefinitions()
        d.clearRegisterDefinitions()
        d.clearBitDefinitions()

        addr1 = RegisterAddress(0x201)
        addr2 = RegisterAddress(0x202)
        d.define_reg(name="test_reg", addr=RegisterAddress(0x200)) # valid
        d.define_reg(name="test_reg", addr=addr1) # takes a register address
        d.define_reg(name="test_reg2", addr=0x202) # takes an int

        # not using keyword arguments
        with pytest.raises(TypeError) as exc_info:
            d.define_reg("randomreg", 0x203)

        # invalid value type
        with pytest.raises(Exception) as exc_info:
            d.define_reg(name="test_reg3", addr='0x203')
        assert "addr must int or RegisterAddress" in str(exc_info.value)

        # defining with duplicate value
        with pytest.raises(Exception) as exc_info:
            d.define_reg(name="test_reg3", addr=addr1)
        assert "Value already assigned" in str(exc_info.value)

        assert(d.getRegisterAddress("test_reg") == addr1)
        assert(d.getRegisterName(addr1) == "test_reg")

        # accessing non existent reg name
        with pytest.raises(Exception) as exc_info:
            d.reg['random_reg']
        assert "No entry found for key" in str(exc_info.value)

        # get non existing reg address
        with pytest.raises(Exception) as exc_info:
            d.getRegisterName(RegisterAddress(0x300))
        assert "No entry found for value" in str(exc_info.value)

        d.clearRegisterDefinitions()
        
        d.setRegisterDefinitions(prev_reg_defs)
        d.setBitDefinitions(prev_bit_defs)  

    else:
        with pytest.raises(Exception) as exc_info:
            d.define_reg(name="test_reg", addr=0x201)
        assert "Register Definitions only for CTB" in str(exc_info.value)

    Log(LogLevel.INFOGREEN, f"✅ {request.node.name} passed")


@pytest.mark.detectorintegration
def test_define_bit(session_simulator, request):
    """ Test setting define_bit for ctb and xilinx_ctb."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    from slsdet import RegisterAddress, BitAddress

    if det_type in ['ctb', 'xilinx_ctb']:
        prev_reg_defs = d.getRegisterDefinitions()
        prev_bit_defs = d.getBitDefinitions()
        d.clearRegisterDefinitions()
        d.clearBitDefinitions()

        addr1 = RegisterAddress(0x201)
        addr2 = RegisterAddress(0x202)
        d.define_reg(name="test_reg1", addr=addr1) 
        d.define_reg(name="test_reg2", addr=addr2)

        # not using keyword arguments
        with pytest.raises(TypeError) as exc_info:
            d.define_bit("randombit", 0x203, 1)

        # invalid value type (bit=string)
        with pytest.raises(ValueError) as exc_info:
            d.define_bit(name="test_bit1", addr='test_reg1', bit_position='1')
        
        # invalid bit_position
        with pytest.raises(Exception) as exc_info:
            d.define_bit(name="test_bit1", addr='test_reg1', bit_position=32)
        assert "Bit position must be between 0 and 31" in str(exc_info.value)

        # defining with random reg value
        with pytest.raises(Exception) as exc_info:
            d.define_bit(name='test_bit1', addr='random_reg', bit_position=1)
        assert "No entry found for key" in str(exc_info.value)

        bit1 = BitAddress(addr1, 2)
        bit2 = BitAddress(addr1, 4)
        bit3 = BitAddress(addr2, 3)

        # defining bit address with bit_position as well
        with pytest.raises(ValueError) as exc_info:
            d.define_bit(name='test_bit1', addr=bit1, bit_position=1)
        assert "bit_position must be None" in str(exc_info.value)


        d.define_bit(name="test_bit1", addr='test_reg2', bit_position=1)
        d.define_bit(name="test_bit1", addr='test_reg1', bit_position=1) # modify reg
        d.define_bit(name='test_bit1', addr=bit1) # modify pos
        d.define_bit(name="test_bit2", addr=0x201, bit_position=4) # int addr
        d.define_bit(name="test_bit3", addr=addr2, bit_position=3) # RegisterAddress addr


        assert(d.getBitAddress('test_bit1') == bit1)
        assert(d.getBitAddress('test_bit2') == bit2)
        assert(d.getBitAddress('test_bit3') == bit3)
        assert(d.getBitAddress('test_bit1').address() == addr1)
        assert(d.getBitAddress('test_bit1').bitPosition() == 2)
        assert(d.getBitAddress('test_bit2') == BitAddress(addr1, 4))

        assert(d.getBitName(bit1) == 'test_bit1')
        assert(d.getBitName(bit2) == 'test_bit2')
        assert(d.getBitName(bit3) == 'test_bit3')
        assert(d.getBitName(BitAddress(addr2,3)) == 'test_bit3')

        # bit doesnt exist for that reg
        with pytest.raises(Exception) as exc_info:
            d.getBitName(BitAddress(addr1, 5))
        assert "No entry found for value" in str(exc_info.value)

        # addr doesnt exist for that reg
        with pytest.raises(Exception) as exc_info:
            d.getBitName(BitAddress(RegisterAddress(0x300), 5))
        assert "No entry found for value" in str(exc_info.value)

        d.clearRegisterDefinitions()
        d.clearBitDefinitions()
        
        d.setRegisterDefinitions(prev_reg_defs)
        d.setBitDefinitions(prev_bit_defs)  
    else:
        with pytest.raises(Exception) as exc_info:
           d.define_bit(name="test_bit", addr=0x300, bit_position=1)
        assert "Bit Definitions only for CTB" in str(exc_info.value)
    
        
    Log(LogLevel.INFOGREEN, f"✅ {request.node.name} passed")


@pytest.mark.detectorintegration
def test_using_defined_reg_and_bit(session_simulator, request):
    """ Test using defined reg and bit define_bit for ctb and xilinx_ctb."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    from slsdet import RegisterAddress, BitAddress, RegisterValue

    if det_type in ['ctb', 'xilinx_ctb']:
        prev_reg_defs = d.getRegisterDefinitions()
        prev_bit_defs = d.getBitDefinitions()
        d.clearRegisterDefinitions()
        d.clearBitDefinitions()

        addr1 = RegisterAddress(0x201)
        addr2 = RegisterAddress(0x202)
        d.setRegisterDefinition('test_reg1', addr1)
        d.setRegisterDefinition('test_reg2', addr2)
        bit1 = BitAddress(addr1, 2)
        bit2 = BitAddress(addr1, 4)
        bit3 = BitAddress(addr2, 3)
        d.setBitDefinition('test_bit1', bit1)
        d.setBitDefinition('test_bit2', bit2)
        d.setBitDefinition('test_bit3', bit3)

        prev_val_addr1 = d.reg[addr1]
        prev_val_addr2 = d.reg[addr2]

        # reg name doesnt exist
        with pytest.raises(Exception) as exc_info:
            d.reg['random_reg']
        assert "No entry found for key" in str(exc_info.value)
        with pytest.raises(Exception) as exc_info:
            d.setBit('random_reg')
        assert "No entry found for key" in str(exc_info.value)
        with pytest.raises(Exception) as exc_info:
            d.clearBit('random_reg')
        assert "No entry found for key" in str(exc_info.value)
        with pytest.raises(Exception) as exc_info:
            d.getBit('random_reg')
        assert "No entry found for key" in str(exc_info.value)

        # bit name doesnt exist
        with pytest.raises(Exception) as exc_info:
            d.setBit('test_bit1', bit_position=5)
        assert "bit_position must be None" in str(exc_info.value)
        with pytest.raises(Exception) as exc_info:
            d.clearBit('test_bit1', bit_position=5)
        assert "bit_position must be None" in str(exc_info.value)
        with pytest.raises(Exception) as exc_info:
            d.getBit('test_bit1', bit_position=5)
        assert "bit_position must be None" in str(exc_info.value)

        d.reg['test_reg1'] = RegisterValue(0x0)
        assert(d.reg['test_reg1'].value() == 0x0)

        d.reg['test_reg1'] = RegisterValue(0x10)
        assert(d.reg['test_reg1'].value() == 0x10)

        d.setBit('test_bit1')
        assert(d.reg['test_reg1'].value() == 0x14) # 0x10 | (1 << 2)

        d.clearBit('test_bit1')
        assert(d.reg['test_reg1'].value() == 0x10)

        assert(d.getBit('test_bit1') == 0)

        # restore previous values
        d.reg[addr1] = prev_val_addr1
        d.reg[addr2] = prev_val_addr2

        d.clearRegisterDefinitions()
        d.clearBitDefinitions()
        
        d.setRegisterDefinitions(prev_reg_defs)
        d.setBitDefinitions(prev_bit_defs)  
    else:
        with pytest.raises(Exception) as exc_info:
            d.define_bit(name="test_bit", addr=0x300, bit_position=1)
        assert "Bit Definitions only for CTB" in str(exc_info.value)

    Log(LogLevel.INFOGREEN, f"✅ {request.node.name} passed")


@pytest.mark.detectorintegration
def test_definelist_reg(session_simulator, request):
    """ Test using definelist_reg for ctb and xilinx_ctb."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    from slsdet import RegisterAddress, BitAddress, RegisterValue

    if det_type in ['ctb', 'xilinx_ctb']:
        prev_reg_defs = d.getRegisterDefinitions()
        prev_bit_defs = d.getBitDefinitions()
        d.clearRegisterDefinitions()
        d.clearBitDefinitions()

        addr1 = RegisterAddress(0x201)
        addr2 = RegisterAddress(0x202)
        bit1 = BitAddress(addr1, 2)
        bit2 = BitAddress(addr1, 4)
        bit3 = BitAddress(addr2, 3)

        d.setRegisterDefinitions({
            'test_reg1': RegisterAddress(0x201),
            'test_reg2': RegisterAddress(0x202)
        })

        res = d.getRegisterDefinitions()
        assert(res['test_reg1'] == addr1)
        assert(res['test_reg2'] == addr2)
        assert(len(res) == 2)

        d.clearRegisterDefinitions()
        d.clearBitDefinitions()
        
        d.setRegisterDefinitions(prev_reg_defs)
        d.setBitDefinitions(prev_bit_defs)  
    else:
        with pytest.raises(Exception) as exc_info:
            d.define_bit(name="test_bit", addr=0x300, bit_position=1)
        assert "Bit Definitions only for CTB" in str(exc_info.value)

    Log(LogLevel.INFOGREEN, f"✅ {request.node.name} passed")


@pytest.mark.detectorintegration
def test_definelist_bit(session_simulator, request):
    """ Test using definelist_bit for ctb and xilinx_ctb."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    from slsdet import RegisterAddress, BitAddress, RegisterValue

    if det_type in ['ctb', 'xilinx_ctb']:
        prev_reg_defs = d.getRegisterDefinitions()
        prev_bit_defs = d.getBitDefinitions()
        d.clearRegisterDefinitions()
        d.clearBitDefinitions()

        addr1 = RegisterAddress(0x201)
        addr2 = RegisterAddress(0x202)
        bit1 = BitAddress(addr1, 2)
        bit2 = BitAddress(addr1, 4)
        bit3 = BitAddress(addr2, 3)

        d.setRegisterDefinitions({
            'test_reg1': RegisterAddress(0x201),
            'test_reg2': RegisterAddress(0x202)
        })
        d.setBitDefinitions({
            'test_bit1': BitAddress(addr1, 2),
            'test_bit2': BitAddress(addr1, 4),
            'test_bit3': BitAddress(addr2, 3)
        })

        res = d.getBitDefinitions()
        assert(len(res) == 3)
        assert(res['test_bit1'] == bit1)
        assert(res['test_bit2'] == bit2)
        assert(res['test_bit3'] == bit3)
        assert(res['test_bit2'].address() == addr1)
        assert(res['test_bit2'].bitPosition() == 4) 

        d.clearRegisterDefinitions()
        d.clearBitDefinitions()
        
        d.setRegisterDefinitions(prev_reg_defs)
        d.setBitDefinitions(prev_bit_defs)  
    else:
        with pytest.raises(Exception) as exc_info:
            d.define_bit(name="test_bit", addr=0x300, bit_position=1)
        assert "Bit Definitions only for CTB" in str(exc_info.value)

    Log(LogLevel.INFOGREEN, f"✅ {request.node.name} passed")


@pytest.mark.detectorintegration
def test_parameters_file(session_simulator, request):
    """ Test using test_parameters_file."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    with open("/tmp/params.det", "w") as f:
        f.write("frames 2\n")
        f.write("fwrite 1\n")

    # this should not throw
    d.parameters = "/tmp/params.det"

    assert d.frames == 2    
    assert d.fwrite == 1

    Log(LogLevel.INFOGREEN, f"✅ Test passed. Command: parameters")


@pytest.mark.detectorintegration
def test_include_file(session_simulator, request):
    """ Test using test_include_file."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None
    
    with open("/tmp/params.det", "w") as f:
        f.write("frames 3\n")
        f.write("fwrite 0\n")

    # this should not throw
    d.include = "/tmp/params.det"

    assert d.frames == 3    
    assert d.fwrite == 0

    Log(LogLevel.INFOGREEN, f"✅ Test passed. Command: include")


@pytest.mark.detectorintegration
def test_patternstart(session_simulator, request):
    """ Test using patternstart for ctb, xilinx_ctb and mythen3."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    if det_type in ['ctb', 'xilinx_ctb', 'mythen3']:
        d.patternstart()
    else:
        with pytest.raises(Exception) as exc_info:
            d.patternstart()
        assert "not implemented" in str(exc_info.value)

    Log(LogLevel.INFOGREEN, f"✅ {request.node.name} passed")


@pytest.mark.detectorintegration
def test_adcclk(session_simulator, request):
    """ Test using adcclk for ctb and xilinx_ctb."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    from slsdet import Hz, MHz, kHz

    if det_type in ['ctb', 'xilinx_ctb']:
        prev_adcclk = d.getADCClock()

        d.adcclk

        # invalid value type
        with pytest.raises(Exception) as exc_info:
            d.adcclk = 5e6

        with pytest.raises(Exception) as exc_info:
            d.adcclk = 5 * 1000 * 1000

        with pytest.raises(Exception) as exc_info:
            d.adcclk = Hz(5e6)

        d.adcclk = MHz(15)
        assert d.adcclk.value == 15_000_000

        d.adcclk = MHz(14.5)
        assert d.adcclk.value == 14_500_000

        d.adcclk = kHz(15000.5)
        assert d.adcclk.value == 15_000_500

        # invalid values from server
        # max is 300MHz for xilinx and 54 MHz for ctb
        if det_type == 'ctb':
            with pytest.raises(Exception) as exc_info:
                d.adcclk = MHz(66)
        else:
            with pytest.raises(Exception) as exc_info:
                d.adcclk = MHz(301)

        # min is 2MHz for ctb and 10MHz for xilinx_ctb
        if det_type == 'ctb':
            with pytest.raises(Exception) as exc_info:
                d.adcclk = MHz(1)
        else:
            with pytest.raises(Exception) as exc_info:
                d.adcclk = MHz(9)

        c = MHz(2)
        for rc in [5, 10, 15, 20]:
            d.adcclk = rc * c
        assert d.adcclk.value == 40_000_000

        for i in range(len(d)):
            d.setADCClock(prev_adcclk[i], [i])
         
    Log(LogLevel.INFOGREEN, f"✅ {request.node.name} passed")


@pytest.mark.detectorintegration
def test_dbitclk(session_simulator, request):
    """ Test using dbitclk for ctb and xilinx_ctb."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    from slsdet import Hz, MHz, kHz

    if det_type in ['ctb', 'xilinx_ctb']:
        prev_dbitclk = d.getDBITClock()

        d.dbitclk

        # invalid value type
        with pytest.raises(Exception) as exc_info:
            d.dbitclk = 5e6

        with pytest.raises(Exception) as exc_info:
            d.dbitclk = 5 * 1000 * 1000

        with pytest.raises(Exception) as exc_info:
            d.dbitclk = Hz(5e6)

        d.dbitclk = MHz(15)
        assert d.dbitclk.value == 15_000_000

        d.dbitclk = MHz(14.5)
        assert d.dbitclk.value == 14_500_000

        d.dbitclk = kHz(15000.5)
        assert d.dbitclk.value == 15_000_500

        # invalid values from server
        # max is 300MHz
        with pytest.raises(Exception) as exc_info:
            d.dbitclk = MHz(301)

        # min is 2MHz for ctb and 10MHz for xilinx_ctb
        if det_type == 'ctb':
            with pytest.raises(Exception) as exc_info:
                d.dbitclk = MHz(1)
        else:
            with pytest.raises(Exception) as exc_info:
                d.dbitclk = MHz(9)

        c = MHz(2)
        for rc in [5, 10, 15, 20]:
            d.dbitclk = rc * c
        assert d.dbitclk.value == 40_000_000

        for i in range(len(d)):
            d.setDBITClock(prev_dbitclk[i], [i])

    Log(LogLevel.INFOGREEN, f"✅ {request.node.name} passed")



@pytest.mark.detectorintegration
def test_syncclk(session_simulator, request):
    """ Test using syncclk for ctb."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    if det_type in ['ctb']:
        d.syncclk

    Log(LogLevel.INFOGREEN, f"✅ {request.node.name} passed")



@pytest.mark.detectorintegration
def test_v_limit(session_simulator, request):
    """Test v_limit."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    if det_type in ['ctb', 'xilinx_ctb']:

        # save previous value
        prev_val = d.getVoltageLimit()
        from slsdet import dacIndex, powerIndex
        prev_dac_val = d.getDAC(dacIndex.DAC_0, False)
        prev_power_dac_val = d.getPowerDAC(powerIndex.V_POWER_A)

        with pytest.raises(Exception):
            d.v_limit = (1200, 'mV') #mV unit not supported, should be 'no unit'

        with pytest.raises(Exception):
            d.v_limit = -100 # previously worked but not allowing now

        # setting dac and power dac with no vlimit should work
        d.v_limit = 0
        assert d.v_limit == 0
        d.setDAC(dacIndex.DAC_0, 1200, True, [0])
        d.setPowerDAC(powerIndex.V_POWER_A, 1200)

        # setting vlimit should throw setting values above vlimit
        d.v_limit = 1500
        assert d.v_limit == 1500

        with pytest.raises(Exception):
            d.setDAC(dacIndex.DAC_0, 1501, True, [0])

        with pytest.raises(Exception):
            d.setPowerDAC(powerIndex.V_POWER_A, 1501)
        
        # setting dac and power dac below vlimit should still work
        d.setDAC(dacIndex.DAC_0, 1210, True, [0])
        d.setPowerDAC(powerIndex.V_POWER_A, 1210)

        # restore previous value
        d.setVoltageLimit(prev_val)
        d.setPowerDAC(powerIndex.V_POWER_A, prev_power_dac_val)
        for i in range(len(d)):
            d.setDAC(dacIndex.DAC_0, prev_dac_val[i], False, [i])

    else:
        with pytest.raises(Exception) as exc_info:
            d.v_limit
        assert "not implemented" in str(exc_info.value)

    Log(LogLevel.INFOGREEN, f"✅ {request.node.name} passed")


@pytest.mark.detectorintegration
def test_v_abcd(session_simulator, request):
    """Test v_a, v_b, v_c, v_d, v_io are deprecated comands."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None


    with pytest.raises(Exception):
        d.v_a

    with pytest.raises(Exception):
        d.v_b

    with pytest.raises(Exception):
        d.v_c

    with pytest.raises(Exception):
        d.v_d

    with pytest.raises(Exception):
        d.v_io

    Log(LogLevel.INFOGREEN, f"✅ {request.node.name} passed")



@pytest.mark.detectorintegration
def test_powers(session_simulator, request):
    """Test powers and powerlist."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    from slsdet import Ctb
    c = Ctb()
    
    if det_type in ['ctb', 'xilinx_ctb']:

        c.powerlist

        # save previous value
        from slsdet import powerIndex
        prev_val_dac = {power: c.getPowerDAC(power) for power in c.getPowerList()}
        prev_val = {power: c.isPowerEnabled(power) for power in c.getPowerList()}

        # invalid
        invalid_assignments = [
            (c.powers, "random", True), # set random power
            (c.powers, "random", True), # set random attribute of power
            (c.powers.VA, "dac", "1200"),
            (c.powers.VA, "enabled", "True"),
            (c.powers, "VA", "-100"),
            (c.powers, "VA", "-1"),
            (c.powers, "VA", "4096")
        ]
        for obj, attr, value in invalid_assignments:
            with pytest.raises(Exception):
                setattr(obj, attr, value)
        # vchip power can only be accessed via pybindings because it cannot be enabled/disabled
        with pytest.raises(Exception):
            c.powers.VCHIP

        # valid
        c.powers
        c.powers.VA = 1200
        assert c.powers.VA == 1200
        assert c.powers.VA.dac == 1200

        c.powers.VA.enable()
        assert c.powers.VA.enabled == True

        c.setPowerEnabled([powerIndex.V_POWER_B, powerIndex.V_POWER_C], True)
        assert c.powers.VB.enabled == True
        assert c.powers.VC.enabled == True

        c.powers.VA = 1500
        assert c.powers.VA == 1500
        assert c.powers.VA.dac == 1500

        # change power name and test same value 
        temp = c.powers.VB
        c.powerlist = ["VA", "m_VB", "VC", "VD", "VIO"]
        assert c.powers.m_VB.enabled == True
        assert c.powers.m_VB == temp

        # restore previous value
        for power in c.getPowerList():
            c.setPowerDAC(power, prev_val_dac[power])
            c.setPowerEnabled([power], prev_val[power])
    else:
        with pytest.raises(Exception) as exc_info:
            c.powerlist
        assert "only for CTB" in str(exc_info.value)

    Log(LogLevel.INFOGREEN, f"✅ {request.node.name} passed")


@pytest.mark.detectorintegration
def test_adclist(session_simulator, request):
    """Test ADC list."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    from slsdet import Ctb
    c = Ctb()
    
    if det_type in ['ctb', 'xilinx_ctb']:
        c.adclist
        c.adclist = ["1", "2", "3", "test", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31", "32"]
        c.adclist

    else:
        with pytest.raises(Exception) as exc_info:
            c.adclist
        assert "only for CTB" in str(exc_info.value)

    Log(LogLevel.INFOGREEN, f"✅ {request.node.name} passed")


@pytest.mark.detectorintegration
def test_signallist(session_simulator, request):
    """Test signal list."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    from slsdet import Ctb
    c = Ctb()
    
    if det_type in ['ctb', 'xilinx_ctb']:
        c.signallist
        c.signallist = ["1", "2", "3", "test", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "60", "61", "62", "63", "64"]
        c.signallist

    else:
        with pytest.raises(Exception) as exc_info:
            c.signallist
        assert "only for CTB" in str(exc_info.value)

    Log(LogLevel.INFOGREEN, f"✅ {request.node.name} passed")


@pytest.mark.detectorintegration
def test_slowadc(session_simulator, request):
    """Test slow ADC and slow adc list."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    from slsdet import Ctb
    c = Ctb()
    
    if det_type in ['ctb', 'xilinx_ctb']:
        c.slowadc
        c.slowadc.SLOWADC5
        c.slowadclist = ["1", "2", "3", "test", "5", "6", "7", "8"]
        c.slowadc.test

    else:
        with pytest.raises(Exception) as exc_info:
            c.signallist
        assert "only for CTB" in str(exc_info.value)

    Log(LogLevel.INFOGREEN, f"✅ {request.node.name} passed")




@pytest.mark.detectorintegration
def test_dac(session_simulator, request):
    """Test dac."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None
    from slsdet import dacIndex

    if det_type in ['ctb', 'xilinx_ctb']:
        
        from slsdet import Ctb
        c = Ctb()

        # valid
        c.daclist
        c.dacvalues
       
        # save previous value
        prev_val = {dac: c.getDAC(dac, False) for dac in c.getDacList()}
        prev_dac_list = c.daclist

        # invalid
        invalid_assignments = [
            (c.dacs, "vb_comp", "1200"), # set random dac
            (c.dacs, "DAC18", "1200"), # set dac 18
            (c.dacs, "DAC0", "-1"),
            (c.dacs, "DAC0", "4096")
        ]
        for obj, attr, value in invalid_assignments:
            with pytest.raises(Exception):
                setattr(obj, attr, value)

        # valid
        c.dacs.DAC0 = 1200
        assert c.getDAC(dacIndex.DAC_0, False)[0] == 1200

        c.dacs.DAC0 = 0
        assert c.dacs.DAC0[0] == 0

        # restore previous value
        for dac in c.getDacList():
            c.setDAC(dac, prev_val[dac][0], False)
        c.daclist = prev_dac_list

    else:
        with pytest.raises(Exception):
            d.dacs.DAC0

        # valid
        d.daclist
        d.dacvalues

        # remember first dac name and index to test later
        dacname = d.daclist[0]
        assert dacname
        dacIndex = d.getDacList()[0]

        # save previous value
        prev_val = d.getDAC(dacIndex, False)

        if det_type == 'eiger':
            from slsdet import Eiger
            c = Eiger()
        elif det_type == 'jungfrau':
            from slsdet import Jungfrau
            c = Jungfrau()
        elif det_type == 'gotthard2':
            from slsdet import Gotthard2
            c = Gotthard2()
        elif det_type == 'mythen3':
            from slsdet import Mythen3
            c = Mythen3()
        elif det_type == 'moench':
            from slsdet import Moench
            c = Moench()    
        else:
            raise RuntimeError("Unknown detector type to test dac: " + det_type)
        # invalid checks
        invalid_assignments = [
            (c.dacs, "random", "1200"), # set random dac
            (c.dacs, "DAC0", "1200"), # set random dac
            (c.dacs, dacname, "-1"),
            (c.dacs, dacname, "4096")
        ]
        for obj, attr, value in invalid_assignments:
            with pytest.raises(Exception):
                setattr(obj, attr, value)

        # valid, have to use setattr because c is different for each detector 
        # and we cannot hardcode the dac name
        setattr(c.dacs, dacname, 1200)
        assert c.getDAC(dacIndex, False)[0] == 1200
        setattr(c.dacs, dacname, 0)
        assert getattr(c.dacs, dacname)[0] == 0

        # restore previous value
        for i in range(len(d)):
            d.setDAC(dacIndex, prev_val[i], False, [i])
   

    Log(LogLevel.INFOGREEN, f"✅ {request.node.name} passed")

@pytest.mark.detectorintegration
@pytest.mark.parametrize("session_simulator",[("moench", 1, 2)],indirect=True)
def test_type(session_simulator):

    d = Detector()
    assert d.type == detectorType.MOENCH


@pytest.mark.detectorintegration
@pytest.mark.parametrize("session_simulator",[("moench", 1, 2), ("jungfrau", 1, 2)],indirect=True)
def test_numinterfaces(session_simulator):

    d = Detector()
    assert d.numinterfaces == 1

