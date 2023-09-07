import sys
from PyQt5.QtWidgets import QApplication
from pyctbgui.ui import MainWindow

app = QApplication(sys.argv)
main = MainWindow()
main.show()
