import pytest
from pathlib import Path
from pyctbgui import alias_utility as at


def test_read_non_existing_file_throws():
    with pytest.raises(FileNotFoundError):
        at.read_alias_file('saijvcaiewjrvijaerijvaeoirvjveiojroiajgv')


def test_parse_sample_file():
    fpath = Path(__file__).parent / 'data/moench04_start_trigger.alias'
    bit_names, bit_plots, bit_colors, adc_names, adc_plots, adc_colors, dac_names, sense_names, power_names, \
    pat_file_name = at.read_alias_file(fpath)

    assert len(bit_names) == 64

    true_names = [
        'gHG', 'DGS6', 'pulse', 'DGS7', 'clearSSR', 'DGS8', 'inSSR', 'DGS9', 'clkSSR', 'clk_fb', 'prechargeConnect',
        'clk', 'clear', 'shr_out_15', 'bypassCDS', 'Sto2', 'ENprechPre', 'Sto0_bot', 'pulseOFF', 'Sto1', 'BYPASS',
        'connCDS', 'Dsg3', 'OutSSRbot', 'resCDS', 'res', 'shr_out_31', 'Dsg1', 'READ', 'Sto0_top', 'resDGS', 'shr_in',
        None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, 'DGS23',
        'DGS31', 'DGS22', 'DGS30', 'DGS21', 'DGS29', 'DGS20', 'DGS28', 'DGS19', 'DGS27', 'DGS18', 'DGS26', 'DGS17',
        'DGS25', 'DGS16', 'DGS24'
    ]

    true_plot = [
        None, True, None, True, None, True, None, True, None, None, None, None, None, None, None, None, None, None,
        None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None,
        None, None, None, None, None, None, None, None, None, None, None, None, True, True, True, True, True, True,
        True, True, True, True, True, True, True, True, True, True
    ]

    # Make sure we didn't mess up anything in this file
    assert len(true_names) == len(bit_names)
    for value, reference in zip(bit_names, true_names):
        assert value == reference

    assert len(true_plot) == 64
    for value, reference in zip(bit_plots, true_plot):
        assert value == reference

    # sense names
    true_sese = ['Vcc_iochip', 'Va+', 'Vcc_int', 'T_boa.(C)', 'Vsh', 'Vcc1.8D', 'Vdd_ps', 'Vcc1.8A']
    assert len(true_sese) == 8
    assert true_sese == sense_names

    # ADC

    true_adc = [
        'SCOL9', 'SCOL8', 'SCOL11', 'SCOL10', 'SCOL13', 'SCOL12', 'SCOL15', 'SCOL14', 'SCOL1', 'SCOL0', 'SCOL3',
        'SCOL2', 'SCOL5', 'SCOL4', 'SCOL7', 'SCOL6', 'SCOL23', 'SCOL22', 'SCOL21', 'SCOL20', 'SCOL19', 'SCOL18',
        'SCOL17', 'SCOL16', 'SCOL31', 'SCOL30', 'SCOL29', 'SCOL28', 'SCOL27', 'SCOL26', 'SCOL25', 'SCOL24'
    ]

    assert len(true_adc) == 32
    assert true_adc == adc_names


def test_single_value_parse_gives_exception():
    # Check that we get the correct exception
    with pytest.raises(Exception, match=r'Alias file parsing failed'):
        at.parse_alias_lines(['hej'])
