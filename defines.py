
class Defines():

    BIT0_15_MASK = 0x0000FFFF
    BIT16_31_MASK = 0xFFFF0000
    BIT0_31_MASK = 0x00000000FFFFFFFF
    BIT32_63_MASK = 0xFFFFFFFF00000000

    Time_Wait_For_Packets_ms = 0.5
    Time_Status_Refresh_ms  = 100
    Time_Plot_Refresh_ms = 20

    Acquisition_Tab_Index = 7

    class Matterhorn():
        nRows = 48
        nHalfCols = 24
        nCols = 48
        nTransceivers = 2
        nSamplesPerRowPerTransceiver = 6
        tranceiverEnable = 0x3


    # pattern viewer defines

    # pattern plot
    Colors_plot  = ['Blue', 'Orange']

    # Wait colors and line styles (6 needed from 0 to 5)
    #Colors_wait = ['b', 'g', 'r', 'c', 'm', 'y']
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
        'Blue',
        'Orange',
        'Green',
        'Red',
        'Purple',
        'Brown',
        'Pink',
        'Gray',
        'Olive',
        'Cyan',
        'Magenta',
        'Yellow',
        'Black',
        'White'
    ]

    LineStyles = [
        '-',
        '--',
        '-.',
        ':'
    ]
