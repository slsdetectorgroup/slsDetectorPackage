from enum import Enum


class Defines:
    Time_Wait_For_Packets_ms = 0.5
    Time_Status_Refresh_ms = 100
    Time_Plot_Refresh_ms = 20

    Zmq_hwm_high_speed = 2
    Zmq_hwm_low_speed = -1

    Acquisition_Tab_Index = 7
    Max_Tabs = 9

    class adc:
        tabIndex = 5
        count = 32
        half = 16
        BIT0_15_MASK = 0x0000FFFF
        BIT16_31_MASK = 0xFFFF0000

    class dac:
        tabIndex = 0
        count = 18

    class signals:
        tabIndex = 3
        count = 64
        half = 32
        BIT0_31_MASK = 0x00000000FFFFFFFF
        BIT32_63_MASK = 0xFFFFFFFF00000000

    class pattern:
        tabIndex = 6
        loops_count = 6

    class transceiver:
        count = 4
        tabIndex = 4

    class slowAdc:
        tabIndex = 2
        count = 8

    colCount = 4

    powerSupplies = ('A', 'B', 'C', 'D', 'IO')

    class ImageIndex(Enum):
        Matterhorn = 0
        Moench04 = 1

    class Matterhorn:
        nRows = 48
        nHalfCols = 24
        nCols = 48
        nTransceivers = 2
        tranceiverEnable = 0x3
        nPixelsPerTransceiver = 4

    class Moench04:
        nRows = 400
        nCols = 400
        adcNumbers = [
            9, 8, 11, 10, 13, 12, 15, 14, 1, 0, 3, 2, 5, 4, 7, 6, 23, 22, 21, 20, 19, 18, 17, 16, 31, 30, 29, 28, 27,
            26, 25, 24
        ]
        nPixelsPerSuperColumn = 5000
        superColumnWidth = 25

    Color_map = [
        'viridis', 'plasma', 'inferno', 'magma', 'cividis', 'binary', 'gist_yarg', 'gist_gray', 'gray', 'bone', 'pink',
        'spring', 'summer', 'autumn', 'winter', 'cool', 'Wistia', 'hot', 'afmhot', 'gist_heat', 'copper',
        'gist_rainbow', 'rainbow', 'jet', 'turbo'
    ]
    Default_Color_Map = 'viridis'

    # pattern viewer defines

    # pattern plot
    Colors_plot = ['Blue', 'Orange']

    # Wait colors and line styles (6 needed from 0 to 5)
    # Colors_wait = ['b', 'g', 'r', 'c', 'm', 'y']
    Colors_wait = ['Blue', 'Green', 'Red', 'Cyan', 'Magenta', 'Yellow']
    Linestyles_wait = ['--', '--', '--', '--', '--', '--']
    Alpha_wait = [0.5, 0.5, 0.5, 0.5, 0.5, 0.5]
    Alpha_wait_rect = [0.2, 0.2, 0.2, 0.2, 0.2, 0.2]

    # Loop colors and line styles (6 needed from 0 to 5)
    Colors_loop = ['Green', 'Red', 'Purple', 'Brown', 'Pink', 'Grey']
    Linestyles_loop = ['-.', '-.', '-.', '-.', '-.', '-.']
    Alpha_loop = [0.5, 0.5, 0.5, 0.5, 0.5, 0.5]
    Alpha_loop_rect = [0.2, 0.2, 0.2, 0.2, 0.2, 0.2]

    # Display the count of clocks
    Clock_vertical_lines_spacing = 50
    Show_clocks_number = True
    Line_width = 2.0

    Colors = [
        'Blue', 'Orange', 'Green', 'Red', 'Purple', 'Brown', 'Pink', 'Gray', 'Olive', 'Cyan', 'Magenta', 'Yellow',
        'Black', 'White'
    ]

    LineStyles = ['-', '--', '-.', ':']

    class colorRange(Enum):
        all = 0
        center = 1
        fixed = 2
