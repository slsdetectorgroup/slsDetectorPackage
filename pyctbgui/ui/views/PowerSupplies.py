from pathlib import Path

from PyQt5 import QtWidgets, uic

from pyctbgui import utils


class PowerSuppliesView(QtWidgets.QWidget):
    def __init__(self, parent):
        super(PowerSuppliesView, self).__init__()
        uic.loadUi(Path(__file__).parent.parent / "power_supplies.ui", parent)
        utils.powerSuppliesView = parent
