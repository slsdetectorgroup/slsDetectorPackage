#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Tests for network related functions of the detector
"""
import pytest
import config_test
from fixtures import eiger, eigertest, detector


# def test_last_client(detector):
#     import socket
#     # We probably should check for multiple ip's
#     myip = socket.gethostbyname_ex(socket.gethostname())[-1][0]
#     assert detector.last_client_ip == myip

def test_get_hostname(detector):
    for detector_host, config_host in zip(detector.hostname, config_test.known_hostnames):
        assert detector_host == config_host

def test_hostname_has_same_length_as_n_modules(detector):
    assert len(detector.hostname) == detector.n_modules


# # def test_get_receiver_hostname(detector):
# #     """Assume that the receiver are on the local computer"""
# #     import socket
# #     host = socket.gethostname().split('.')[0]
# #     assert detector.rx_hostname == host

# def test_set_receiver_hostname(detector):
#     import socket
#     host = socket.gethostname().split('.')[0]
#     phony_host = 'madeup'
#     detector.rx_hostname = phony_host
#     assert detector.rx_hostname == phony_host
#     detector.rx_hostname = host
#     assert detector.rx_hostname == host

@eigertest
def test_set_rx_zmqport_single_value(eiger):
    eiger.rx_zmqport = 35000
    assert eiger.rx_zmqport == [35000, 35001, 35002, 35003]

@eigertest
def test_set_rx_zmqport_list(eiger):
    eiger.rx_zmqport = [37000, 38000]
    assert eiger.rx_zmqport == [37000, 37001, 38000, 38001]

@eigertest
def test_set_rx_updport(eiger):
    ports = [60010,60011,60012,60013]
    eiger.rx_udpport = ports
    assert eiger.rx_udpport == ports
    eiger.acq()
    assert eiger.frames_caught == 1

@eigertest
def test_rx_tcpport(eiger):
    ports = eiger.rx_tcpport
    eiger.rx_tcpport = [2000,2001]
    assert eiger.rx_tcpport == [2000,2001]
    eiger.rx_tcpport = ports
    assert eiger.rx_tcpport == ports
    eiger.acq()
    assert eiger.frames_caught == 1

@eigertest
@pytest.mark.new
def test_enable_disable_tengiga(eiger):
    """
    This test does not check for dat on the 10Gbit link, only the set and get functions
    """
    eiger.tengiga = True
    assert eiger.tengiga == True
    eiger.tengiga = False
    assert eiger.tengiga == False



#TODO! Add test for Jungfrau