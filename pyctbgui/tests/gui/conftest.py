import pytest

from pyctbgui.ui import MainWindow


@pytest.fixture(scope='package')
def main():
    main = MainWindow()
    main.show()
    yield main
    main.close()
