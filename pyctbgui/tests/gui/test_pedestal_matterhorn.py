import numpy as np
from pytestqt.qt_compat import qt_api

from tests.gui.utils import defaultParams, setup_gui


def test_pedestal_substraction(main, qtbot, tmp_path):
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
