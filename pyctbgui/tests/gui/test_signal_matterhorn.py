# from pathlib import Path
#
# import numpy as np
# from pytestqt.qt_compat import qt_api
#
# from .utils import setup_gui, defaultParams
#
#
# def test_waveform_signals(main, qtbot, tmp_path):
#     """
#     tests signals waveform acquisition and numpy saving
#     """
#     params = defaultParams()
#     params.image = False
#     params.mode = "Signal"
#     params.enabled = list(range(128))
#     params.plotted = params.enabled
#
#     setup_gui(qtbot, main, params, tmp_path=tmp_path)
#
#     qtbot.mouseClick(main.pushButtonStart, qt_api.QtCore.Qt.MouseButton.LeftButton)
#
#     acqIndex = main.acquisitionTab.view.spinBoxAcquisitionIndex.value() - 1
#     newPath = main.acquisitionTab.outputDir / f'{main.acquisitionTab.outputFileNamePrefix}_{acqIndex}.npz'
#
#     qtbot.wait_until(lambda: newPath.is_file())
#     testArray = np.load(newPath)
#
