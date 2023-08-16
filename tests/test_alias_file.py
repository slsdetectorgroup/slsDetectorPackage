import pytest
from pathlib import Path
from pyctbgui import alias_utility as at

def test_read_non_existing_file_throws():
    with pytest.raises(Exception):
        at.read_alias_file('saijvcaiewjrvijaerijvaeoirvjveiojroiajgv')

def test_parse_sample_file():
    fpath = Path(__file__).parent/'data/moench04_start_trigger.alias'
    bit_names, bit_plots, bit_colors, adc_names, adc_plots, adc_colors, dac_names, sense_names, power_names, pat_file_name = at.read_alias_file(fpath)

    assert len(bit_names) == 64


    true_names = [
        'gHG', 'DGS6', 'pulse','DGS7','clearSSR','DGS8','inSSR','DGS9','clkSSR','clk_fb','prechargeConnect',
        'clk','clear','shr_out_15','bypassCDS','Sto2','ENprechPre','Sto0_bot','pulseOFF','Sto1','BYPASS','connCDS',
        'Dsg3','OutSSRbot','resCDS','res','shr_out_31','Dsg1','READ','Sto0_top','resDGS','shr_in',
        None, None, None, None, None, None, None, None, None, None, None,None, None, None, None,None,
        'DGS23','DGS31',
        'DGS22','DGS30','DGS21','DGS29','DGS20','DGS28','DGS19','DGS27','DGS18','DGS26','DGS17','DGS25','DGS16',
        'DGS24'
        ]
    
    #Make sure we didn't mess up anything in this file
    assert len(true_names) == len(bit_names)
    for value, reference in zip(bit_names, true_names):
        assert value == reference
