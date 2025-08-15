.. _setup commands:

Setup Commands
================================


Introduction
-------------

To connect to any device, one needs a unique combination of IP address and port number. The IP identifies which device one is talking to, and the port tells it which service one wants to use.

This package usually deals with two network interfaces:

* 1 GbE public interface - Accessible from anywhere on the network. One can simply ping this interface from any PC to check connectivity. 

* 10 GbE private interface - Usually dedicated to a specific PC for high-speed data transfer. Along with the 1 GbE public interface (1500 MTU), it also has 1 or more private 10GbE interfaces (9000 MTU) and they are not reachable from other machines. 

Client to Module
-----------------

.. figure:: images/Client_module_commands.png
   :target: _images/Client_module_commands.png
   :width: 700px
   :align: center
   :alt: Client Module Components

   Client Module Commands

Here, one uses the 1 GbE public TCP interface. This means one should be able to ping the module's hostname from any PC on the network.

If it pings successfully, one should be able to connect to the module's on-board servers ie. the `hostname` command should run successfully. If it does not ping, the servers have probably not been started yet.

For physical modules, each one has its own unique IP address.
Because the IPs are already different, one can use the same default ports for all modules:

* 1952 - Control port
* 1953 - Stop port

.. code-block:: bash  

    # Therefore, one can use 
    hostname bchip100+bchip101+
    #instead of 
    hostname bchip100:1952+bchip101:1954+

**Simulators**, on the other hand, usually run on the same PC, so they must use different ports for each instance as shown below. With increasing port numbers, you can also use the `virtual command for simulators <commandline.html#term-virtual-n_servers-starting_port_number>`_ with the same effect.

.. code-block:: bash  

    hostname localhost:1952+localhost:1954+
    virtual 2 1952



