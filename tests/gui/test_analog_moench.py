from pathlib import Path

import numpy as np
from pytestqt.qt_compat import qt_api

from .utils import setup_gui, defaultParams


def test_image_acq(main, qtbot, tmp_path):
    """
    tests Analog image acquisition and numpy saving
    """
    params = defaultParams()
    params.detector = "Moench04"
    params.mode = "Analog"
    params.enabled = list(range(32))

    setup_gui(qtbot, main, params, tmp_path=tmp_path)

    qtbot.mouseClick(main.pushButtonStart, qt_api.QtCore.Qt.MouseButton.LeftButton)

    acqIndex = main.acquisitionTab.view.spinBoxAcquisitionIndex.value() - 1
    newPath = main.acquisitionTab.outputDir / f'{main.acquisitionTab.outputFileNamePrefix}_{acqIndex}.npy'
    qtbot.wait_until(lambda: newPath.is_file())

    testArray = np.load(newPath)
    dataArray = np.load(Path(__file__).parent / 'data' / 'moench04_image_analog.npy')

    assert testArray.shape == (1, 400, 400)
    assert np.array_equal(dataArray, testArray)


def test_waveform_acq(main, qtbot, tmp_path):
    """
    tests Analog waveform acquisition and numpy saving
    """
    params = defaultParams()
    params.image = False
    params.detector = "Moench04"
    params.mode = "Analog"
    params.enabled = list(range(32))
    params.plotted = params.enabled

    setup_gui(qtbot, main, params, tmp_path=tmp_path)

    qtbot.mouseClick(main.pushButtonStart, qt_api.QtCore.Qt.MouseButton.LeftButton)

    acqIndex = main.acquisitionTab.view.spinBoxAcquisitionIndex.value() - 1
    newPath = main.acquisitionTab.outputDir / f'{main.acquisitionTab.outputFileNamePrefix}_{acqIndex}.npz'

    qtbot.wait_until(lambda: newPath.is_file())
    testArray = np.load(newPath)
    dataArray = np.load(Path(__file__).parent / 'data' / 'moench04_waveform_adc.npz')
    files = [f"ADC{i}" for i in params.enabled]
    assert testArray.files == files
    for i in files:
        assert np.array_equal(dataArray[i], testArray[i])
