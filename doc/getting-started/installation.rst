Installation
============

nymea can be deployed on Linux distributions, embedded platforms, and container
runtimes. The following table summarises the recommended installation options.

+-------------------+--------------------------+-----------------------------------+
| Platform          | Package                  | Notes                             |
+===================+==========================+===================================+
| Ubuntu/Debian     | ``apt install nymea``    | Official packages via nymea PPA.  |
+-------------------+--------------------------+-----------------------------------+
| Raspberry Pi OS   | ``apt install nymea``    | Hardware specific optimisations.  |
+-------------------+--------------------------+-----------------------------------+
| Docker            | ``nymea/nymead`` image   | Provides nymead with default      |
|                   |                          | configuration and persistent      |
|                   |                          | volumes.                          |
+-------------------+--------------------------+-----------------------------------+

.. note::
   Refer to the distribution specific packaging documentation if you need to
   enable third-party repositories.

After installation you should have the :command:`nymead` service available on
your system. Start the service and confirm it is reachable before proceeding to
:doc:`first-run`.
