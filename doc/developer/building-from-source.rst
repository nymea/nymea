Building from source
====================

The nymea project uses Qt and CMake for building the server core and libraries.
On Debian-based systems you can install the build dependencies with:

.. code-block:: bash

   sudo apt build-dep nymea

Clone the repository and configure the build:

.. code-block:: bash

   git clone https://github.com/guh/nymea.git
   cd nymea
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build

The resulting binaries are located in ``build/server`` and ``build/libnymea``.
