from pathlib import Path

from pytestqt.qt_compat import qt_api

from pyctbgui.utils.defines import Defines
from tests.gui.utils import defaultParams


def test_view_pattern(main, qtbot, tmp_path):
    """
    Tests pattern file viewing
    """
    params = defaultParams()
    params.image = False
    main.tabWidget.setCurrentIndex(Defines.pattern.tabIndex)
    qtbot.keyClicks(main.patternTab.view.lineEditPatternFile, "tests/gui/data/pattern.pat")
    qtbot.mouseClick(main.patternTab.view.pushButtonViewPattern, qt_api.QtCore.Qt.MouseButton.LeftButton)
    qtbot.wait_until(lambda: main.patternTab.figure is not None)
    assert main.patternTab.figure is not None

    # save pattern
    main.patternTab.figure.savefig(tmp_path / "pattern.png")
    assert Path(tmp_path / "pattern.png").exists()

    # pattern files generated from python3.10 libraries differ from python3.11. this would make this
    # test flaky so we skip it for now
    # assert filecmp.cmp("tests/gui/data/pattern.png", tmp_path / "pattern.png")
