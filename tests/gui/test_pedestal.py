# from pathlib import Path
#
# import numpy as np
# from pytestqt.qt_compat import qt_api
#
# from tests.gui import main
# from tests.gui.utils import setup_gui, defaultParams
# from slsdet import runStatus
#
#
# def test_pedestal_substraction(qtbot, tmp_path):
#     """
#     tests Transceiver image acquisition, numpy saving and pedestal substraction
#     """
#
#     # record 10 frames
#     params = defaultParams
#     params['pedestalRecord'] = True
#     params['nFrames'] = 10
#     params['numpy'] = False
#     setup_gui(qtbot, main, tmp_path=tmp_path)
#     qtbot.mouseClick(main.pushButtonStart, qt_api.QtCore.Qt.MouseButton.LeftButton)
#     acqIndex = main.acquisitionTab.view.spinBoxAcquisitionIndex.value() - 1
#     newPath = main.acquisitionTab.outputDir / f'{main.acquisitionTab.outputFileNamePrefix}_{acqIndex}.npy'
#     qtbot.wait_until(lambda: main.det.status == runStatus.IDLE)
#
#     testArray = np.load(newPath)
#     dataArray = np.load(Path(__file__).parent.parent / 'data' / 'matterhorm_image_transceiver.npy')
#
#     assert testArray.shape == (1, 48, 48)
#     assert np.array_equal(dataArray, testArray)
