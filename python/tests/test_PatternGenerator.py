import pytest 
from slsdet import PatternGenerator


def apply_detconf(p):
    """
    Hacky workaround to apply detConf_mh02 to a pattern
    """
    DACMON = 0
    cnt_en_3 = 1
    pulse_counter_en = 2
    cnt_en_1 = 3
    par_load = 4
    pulse_mux_ctr = 5
    reset_cnt = 6
    reset_periphery = 7
    config_load = 8
    cnt_en_0 = 9
    tbl = 10
    clk_ext = 11
    trimbit_load_reg = 12
    store = 13
    data_in = 14
    en_pll_clk = 15
    cnt_en_2 = 16
    DACINT = 17
    data_out_slow = 18 #IN
    COMP2_MON = 19 #IN
    start_read = 20
    dac_store = 21
    CNT3_MON = 22 #IN
    EN_PIX_DIG_MON = 23
    clk_sel = 24
    BUSY = 25 #IN
    COMP3_MON = 26 #IN
    CNT2_MON = 27 #IN

    dbit_ena=62 #FIFO LATCH
    adc_ena=63  #ADC ENABLE

    #FPGA input/ouutputs
    p.setoutput(DACMON)
    p.setoutput(cnt_en_3)
    p.setoutput(pulse_counter_en)
    p.setoutput(cnt_en_1)
    p.setoutput(par_load)
    p.setoutput(pulse_mux_ctr)
    p.setoutput(reset_cnt)
    p.setoutput(reset_periphery)
    p.setoutput(cnt_en_0)
    p.setoutput(tbl)
    p.setoutput(clk_ext)
    p.setoutput(config_load)
    p.setoutput(trimbit_load_reg)
    p.setoutput(store)
    p.setoutput(data_in)
    p.setoutput(en_pll_clk)
    p.setoutput(cnt_en_2)
    p.setoutput(DACINT)
    p.setinput(data_out_slow)
    p.setinput(COMP2_MON)
    p.setoutput(start_read)
    p.setoutput(dac_store)
    p.setinput(CNT3_MON)
    p.setoutput(EN_PIX_DIG_MON)
    p.setoutput(clk_sel)
    p.setinput(BUSY)
    p.setinput(COMP3_MON)
    p.setinput(CNT2_MON)

    #system signals
    p.setoutput(adc_ena)
    # FIFO LATCH
    p.setoutput(dbit_ena)
    return p    





def test_first_two_PW():
    p = PatternGenerator()

    #The pattern is created with a single empty word
    assert p.pattern.limits[0] == 0
    assert p.pattern.limits[1] == 0

    p.SB(8)
    p.PW()

    #When doing the first PW the empty word is overwritten
    assert p.pattern.limits[0] == 0
    assert p.pattern.limits[1] == 0
    assert p.pattern.word[0] == 256

    p.SB(9)
    p.PW()

    #When doing the second PW we add a new word
    assert p.pattern.limits[0] == 0
    assert p.pattern.limits[1] == 1
    assert p.pattern.word[0] == 256
    assert p.pattern.word[1] == 768

def test_simple_pattern():
    """
    Using enable pll pattern for MH02
    """
    en_pll_clk = 15
    p = PatternGenerator()
    p = apply_detconf(p)
    p.SB(en_pll_clk)
    p.PW()
    p.PW() 

    lines = str(p).split("\n")

    enable_pll_pattern = [
        "patword 0x0000 0x0000000000008000",
        "patword 0x0001 0x0000000000008000",
        "patioctrl 0xc000000001b3ffff",
        "patlimits 0x0000 0x0001",
        "patloop 0 0x1fff 0x1fff",
        "patnloop 0 0",
        "patloop 1 0x1fff 0x1fff",
        "patnloop 1 0",
        "patloop 2 0x1fff 0x1fff",
        "patnloop 2 0",
        "patloop 3 0x1fff 0x1fff",
        "patnloop 3 0",
        "patloop 4 0x1fff 0x1fff",
        "patnloop 4 0",
        "patloop 5 0x1fff 0x1fff",
        "patnloop 5 0",
        "patwait 0 0x1fff",
        "patwaittime 0 0",
        "patwait 1 0x1fff",
        "patwaittime 1 0",
        "patwait 2 0x1fff",
        "patwaittime 2 0",
        "patwait 3 0x1fff",
        "patwaittime 3 0",
        "patwait 4 0x1fff",
        "patwaittime 4 0",
        "patwait 5 0x1fff",
        "patwaittime 5 0",
    ]

    assert len(lines) == len(enable_pll_pattern)

    for i, line in enumerate(lines):
        assert line == enable_pll_pattern[i]