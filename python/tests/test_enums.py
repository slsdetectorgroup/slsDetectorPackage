from slsdet.enums import streamingInterface
import pytest
def test_streamingInterface_bitwise():
    """Bitwise operations on streaming interfaces are allowed"""

    sif_none = streamingInterface.NONE
    sif_low = streamingInterface.LOW_LATENCY_LINK
    sif_10g = streamingInterface.ETHERNET_10GB
    sif_all = streamingInterface.ALL

    assert sif_low | sif_none == sif_low
    assert sif_10g | sif_none == sif_10g
    assert sif_low | sif_10g == sif_all
    assert sif_10g | sif_all == sif_all

    assert sif_10g & sif_low == sif_none
    assert sif_low & sif_low == sif_low
    assert sif_all & sif_all == sif_all

def test_streamingInterface_bitwise_only_allowed_on_same_type():
    with pytest.raises(Exception):
        assert streamingInterface.LOW_LATENCY_LINK & 5 == 7

