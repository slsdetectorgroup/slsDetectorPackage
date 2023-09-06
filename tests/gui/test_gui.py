from pathlib import Path

import numpy as np
import pytest
from pytestqt.qt_compat import qt_api

from pyctbgui.ui import MainWindow

defaultParams = {
    'image': True,
    "chip": "Matterhorn",
    "numpy": True,
    "mode": "Transceiver",
    "outputDir": "/tmp",
    "filename": "run",
    "nFrames": 1,
    "enabled": [],
    'plotted': []
}


def __setup_gui(qtbot, widget, params=defaultParams, tmp_path=Path('/tmp')):
    if params['image']:
        widget.plotTab.view.radioButtonImage.setChecked(True)
        widget.plotTab.view.radioButtonWaveform.setChecked(False)
    else:
        widget.plotTab.view.radioButtonWaveform.setChecked(True)
        widget.plotTab.view.radioButtonImage.setChecked(False)
        if params['mode'] == "Transceiver":

            for i in params['enabled']:
                enable = getattr(widget.transceiverTab.view, f"checkBoxTransceiver{i}")
                enable.setChecked(True)
            for i in params['plotted']:
                enable = getattr(widget.transceiverTab.view, f"checkBoxTransceiver{i}Plot")
                enable.setChecked(True)

    qtbot.keyClicks(widget.plotTab.view.comboBoxPlot, params['chip'])
    widget.acquisitionTab.view.checkBoxFileWriteNumpy.setChecked(params['numpy'])
    qtbot.keyClicks(widget.acquisitionTab.view.comboBoxROMode, params['mode'])

    widget.acquisitionTab.view.lineEditFilePath.setText(str(tmp_path))
    widget.acquisitionTab.setFilePath()

    widget.acquisitionTab.view.lineEditFileName.setText(params['filename'])
    widget.acquisitionTab.setFileName()

    widget.acquisitionTab.view.spinBoxFrames.setValue(params['nFrames'])
    widget.acquisitionTab.setFrames()

    qtbot.wait_until(lambda: widget.acquisitionTab.view.spinBoxFrames.value() == 1)

    assert widget.acquisitionTab.view.comboBoxROMode.currentText() == params['mode']
    assert widget.plotTab.view.comboBoxPlot.currentText() == params['chip']
    assert widget.acquisitionTab.writeNumpy == params['numpy']
    assert widget.acquisitionTab.view.spinBoxFrames.value() == params['nFrames']
    assert widget.acquisitionTab.outputDir == Path(str(tmp_path))


@pytest.fixture(scope="module")
def main():
    widget = MainWindow()
    widget.show()
    return widget


def test_image_acq(main, tmp_path, qtbot):
    """
    tests Transceiver image acquisition and numpy saving
    """

    qtbot.addWidget(main)
    __setup_gui(qtbot, main, tmp_path=tmp_path)

    qtbot.mouseClick(main.pushButtonStart, qt_api.QtCore.Qt.MouseButton.LeftButton)

    acqIndex = main.acquisitionTab.view.spinBoxAcquisitionIndex.value() - 1
    newPath = main.acquisitionTab.outputDir / f'{main.acquisitionTab.outputFileNamePrefix}_{acqIndex}.npy'
    qtbot.wait_until(lambda: newPath.is_file())

    testArray = np.load(newPath)
    dataArray = np.load(Path(__file__).parent.parent / 'data' / 'matterhorm_image_transceiver.npy')
    assert testArray.shape == (1, 48, 48)
    assert np.array_equal(dataArray, testArray)


# def test_waveform_acq(main, qtbot, tmp_path):
#     qtbot.addWidget(main)
#
#     params = defaultParams
#     params['image'] = False
#     __setup_gui(qtbot, main, params=params, tmp_path=tmp_path)
#
#     qtbot.mouseClick(main.pushButtonStart, qt_api.QtCore.Qt.MouseButton.LeftButton)
#
#     acqIndex = main.acquisitionTab.view.spinBoxAcquisitionIndex.value() - 1
#     newPath = main.acquisitionTab.outputDir / f'{main.acquisitionTab.outputFileNamePrefix}_{acqIndex}.npy'
#     qtbot.wait_until(lambda: newPath.is_file())
#
#     testArray = np.load(newPath)
#     dataArray = np.load(Path(__file__).parent.parent / 'data' / 'matterhorm_image_transceiver.npy')
#     assert testArray.shape == (1, 48, 48)
#     assert np.array_equal(dataArray, testArray)
