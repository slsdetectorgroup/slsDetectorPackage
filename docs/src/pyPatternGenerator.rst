PatternGenerator
=====================================================

Python class to generate patterns for the Chip Test Board.

.. code-block:: python

    from slsdet import PatternGenerator

    p = PatternGenerator()
    p.SB(5)
    p.PW()
    p.SB(8,9)
    p.PW()
    p.CB(5)

Created a pattern like this:

.. code-block:: bash

    patword 0x0000 0x0000000000000020
    patword 0x0001 0x0000000000000320
    patword 0x0002 0x0000000000000300
    patioctrl 0x0000000000000000
    patlimits 0x0000 0x0002
    ...

.. py:currentmodule:: slsdet

.. autoclass:: PatternGenerator
    :members:  
    :undoc-members:
    :show-inheritance:
    :inherited-members: