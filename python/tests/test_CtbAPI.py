'''
cd python/tests
Specific test: pytest -s -x test_CtbAPI.py::test_define_bit  #-x=abort on first failure
Specific test with specific server: pytest -s -x test_CtbAPI.py::test_define_reg[ctb]

'''

import pytest, sys, traceback

from pathlib import Path
current_dir = Path(__file__).resolve().parents[2]
scripts_dir = current_dir / "tests" / "scripts"
sys.path.append(str(scripts_dir))
print(sys.path)

from utils_for_test import (
    Log,
    LogLevel,
    cleanup,
    startDetectorVirtualServer,
    connectToVirtualServers,
    SERVER_START_PORTNO, 
)

from slsdet import Detector, detectorType


@pytest.fixture(
    scope="session", 
    params=['ctb', 'xilinx_ctb', 'mythen3']
)
def simulator(request):
    """Fixture to start the detector server once and clean up at the end."""
    det_name = request.param
    num_mods = 1
    fp = sys.stdout

    # set up: once per server
    Log(LogLevel.INFOBLUE, f'---- {det_name} ----')
    cleanup(fp)
    startDetectorVirtualServer(det_name, num_mods, fp)

    Log(LogLevel.INFOBLUE, f'Waiting for server to start up and connect')
    connectToVirtualServers(det_name, num_mods)

    yield det_name # tests run here

    cleanup(fp)


@pytest.mark.detectorintegration
def test_define_reg(simulator, request):
    """ Test setting define_reg for ctb and xilinx_ctb."""
    det_name = simulator
    from slsdet import RegisterAddress

    d = Detector()
    d.hostname = f"localhost:{SERVER_START_PORTNO}" 

    if det_name in ['ctb', 'xilinx_ctb']:
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
def test_define_bit(simulator, request):
    """ Test setting define_bit for ctb and xilinx_ctb."""
    det_name = simulator
    from slsdet import RegisterAddress, BitAddress

    # setup
    d = Detector()
    d.hostname = f"localhost:{SERVER_START_PORTNO}" 

    if det_name in ['ctb', 'xilinx_ctb']:
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
def test_using_defined_reg_and_bit(simulator, request):
    """ Test using defined reg and bit define_bit for ctb and xilinx_ctb."""
    det_name = simulator
    from slsdet import RegisterAddress, BitAddress, RegisterValue

    # setup
    d = Detector()
    d.hostname = f"localhost:{SERVER_START_PORTNO}" 

    if det_name in ['ctb', 'xilinx_ctb']:
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
def test_definelist_reg(simulator, request):
    """ Test using definelist_reg for ctb and xilinx_ctb."""
    det_name = simulator
    from slsdet import RegisterAddress, BitAddress, RegisterValue

    # setup
    d = Detector()
    d.hostname = f"localhost:{SERVER_START_PORTNO}" 

    if det_name in ['ctb', 'xilinx_ctb']:
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
def test_definelist_bit(simulator, request):
    """ Test using definelist_bit for ctb and xilinx_ctb."""
    det_name = simulator
    from slsdet import RegisterAddress, BitAddress, RegisterValue

    # setup
    d = Detector()
    d.hostname = f"localhost:{SERVER_START_PORTNO}" 

    if det_name in ['ctb', 'xilinx_ctb']:
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