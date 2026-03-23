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
            d.v_limit = -100

        d.v_limit = 0
        assert d.v_limit == 0
        d.setDAC(dacIndex.DAC_0, 1200, True, [0])
        d.setPowerDAC(powerIndex.V_POWER_A, 1200)

        d.v_limit = 1500
        assert d.v_limit == 1500

        with pytest.raises(Exception):
            d.setDAC(dacIndex.DAC_0, 1501, True, [0])

        with pytest.raises(Exception):
            d.setPowerDAC(powerIndex.V_POWER_A, 1501)

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
    """Test powers."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    if det_type in ['ctb', 'xilinx_ctb']:

        # save previous value
        from slsdet import powerIndex
        prev_val_dac = d.getPowerDAC(powerIndex.V_POWER_A)
        prev_val = d.isPowerEnabled(powerIndex.V_POWER_A)

        invalid_assignments = [
            (d.powers, "random", True), # set random power
            (d.powers.VA, "random", True), # set random attribute of power
            (d.powers.VA, "dac", "-100"),
            (d.powers.VA, "dac", "-1"),
            (d.powers.VA, "dac", "4096")
        ]

        for obj, attr, value in invalid_assignments:
            with pytest.raises(Exception):
                setattr(obj, attr, value)

        with pytest.raises(Exception):
            d.powers.VCHIP.dac


        d.powers
        d.powers.VA.dac = 1200
        assert d.powers.VA.dac == 1200

        d.powers.VA.enable = True
        assert d.powers.VA.enable == True

        d.setPowerEnabled([powerIndex.V_POWER_B, powerIndex.V_POWER_C], True)
        assert d.powers.VB.enable == True
        assert d.powers.VC.enable == True

        d.powers.VA.dac = 1500
        assert d.powers.VA.dac == 1500

        d.powers._powernames[1] = "v_named_b"
        assert d.powers.v_named_b.enable == True

        # restore previous value
        d.setPowerDAC(powerIndex.V_POWER_A, prev_val_dac)
        d.setPowerEnabled(powerIndex.V_POWER_A, prev_val)
    else:
        with pytest.raises(Exception) as exc_info:
            d.v_limit
        assert "not implemented" in str(exc_info.value)

    Log(LogLevel.INFOGREEN, f"✅ {request.node.name} passed")




'''
@pytest.mark.detectorintegration
def test_dac(session_simulator, request):
    """Test dac."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    if det_type in ['ctb', 'xilinx_ctb']:

        # save previous value
        from slsdet import dacIndex
        prev_val = d.getDAC(dacIndex.V_LIMIT, True)
        prev_dac_val = d.getDAC(dacIndex.DAC_0, True)

        with pytest.raises(Exception):
            d.v_limit = (1200, 'mV') #mV unit not supported, should be 'no unit'

        with pytest.raises(Exception):
            d.v_limit = -100

        d.v_limit = 0
        assert d.v_limit == 0
        d.setDAC(dacIndex.DAC_0, 1200, True, {0})

        d.v_limit = 1500
        assert d.v_limit == 1500

        with pytest.raises(Exception):
            d.setDAC(dacIndex.DAC_0, 1501, True, {0})

        # restore previous value
        for i in range(len(d)):
            d.setDAC(dacIndex.V_LIMIT, prev_val[i], True, {i})
            d.setDAC(dacIndex.DAC_0, prev_dac_val[i], True, {i})


    Log(LogLevel.INFOGREEN, f"✅ {request.node.name} passed")
'''