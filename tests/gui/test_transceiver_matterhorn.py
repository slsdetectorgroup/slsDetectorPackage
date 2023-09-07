from pathlib import Path

import numpy as np
from pytestqt.qt_compat import qt_api

from .utils import setup_gui, defaultParams
from . import main

# @pytest.fixture(scope='module')
# def main():
#     app = QApplication(sys.argv)
#     main = MainWindow()
#     main.show()
#     return main


def test_image_acq(qtbot, tmp_path):
    """
    tests Transceiver image acquisition and numpy saving
    """

    setup_gui(qtbot, main, tmp_path=tmp_path)

    qtbot.mouseClick(main.pushButtonStart, qt_api.QtCore.Qt.MouseButton.LeftButton)

    acqIndex = main.acquisitionTab.view.spinBoxAcquisitionIndex.value() - 1
    newPath = main.acquisitionTab.outputDir / f'{main.acquisitionTab.outputFileNamePrefix}_{acqIndex}.npy'
    qtbot.wait_until(lambda: newPath.is_file())

    testArray = np.load(newPath)
    dataArray = np.load(Path(__file__).parent.parent / 'data' / 'matterhorm_image_transceiver.npy')

    assert testArray.shape == (1, 48, 48)
    assert np.array_equal(dataArray, testArray)


def test_waveform_acq(qtbot, tmp_path):
    """
    tests Transceiver waveform acquisition and numpy saving
    """
    params = defaultParams()
    params.image = False
    params.enabled = [0, 1]
    params.plotted = [0, 1]

    setup_gui(qtbot, main, params, tmp_path=tmp_path)

    qtbot.mouseClick(main.pushButtonStart, qt_api.QtCore.Qt.MouseButton.LeftButton)

    acqIndex = main.acquisitionTab.view.spinBoxAcquisitionIndex.value() - 1
    newPath = main.acquisitionTab.outputDir / f'{main.acquisitionTab.outputFileNamePrefix}_{acqIndex}.npz'

    qtbot.wait_until(lambda: newPath.is_file())
    testArray = np.load(newPath)
    dataArray = np.load(Path(__file__).parent.parent / 'data' / 'matterhorn_waveform_transceiver1and2.npz')
    assert testArray.files == ['Transceiver 0', 'Transceiver 1']
    assert np.array_equal(dataArray['Transceiver 0'], testArray['Transceiver 0'])
    assert np.array_equal(dataArray['Transceiver 1'], testArray['Transceiver 1'])


def test_pedestal_substraction(qtbot, tmp_path):
    """
    tests Transceiver image acquisition, numpy saving and pedestal substraction
    """

    # record 10 frames
    params = defaultParams()
    params.pedestalRecord = True
    params.nFrames = 10
    params.numpy = False
    setup_gui(qtbot, main, params, tmp_path=tmp_path)
    qtbot.mouseClick(main.pushButtonStart, qt_api.QtCore.Qt.MouseButton.LeftButton)
    qtbot.wait_until(lambda: main.plotTab.view.labelPedestalFrames.text() == 'recorded frames: 10')

    # apply pedestal and save
    params.pedestalRecord = False
    params.numpy = True
    params.nFrames = 2
    setup_gui(qtbot, main, params, tmp_path=tmp_path)
    qtbot.mouseClick(main.pushButtonStart, qt_api.QtCore.Qt.MouseButton.LeftButton)
    acqIndex = main.acquisitionTab.view.spinBoxAcquisitionIndex.value() - 1
    newPath = main.acquisitionTab.outputDir / f'{main.acquisitionTab.outputFileNamePrefix}_{acqIndex}.npy'
    qtbot.wait_until(lambda: newPath.is_file())

    testArray = np.load(newPath)

    assert testArray.shape == (2, 48, 48)
    assert np.array_equal(testArray, np.zeros((2, 48, 48)))
