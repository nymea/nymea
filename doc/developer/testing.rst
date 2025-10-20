Testing
=======

nymea provides several layers of automated testing:

Unit tests
   Located in the ``tests`` directory and run with :command:`ctest`.

Integration tests
   Exercise the JSON-RPC and CoAP APIs using scripted scenarios.

Plugin tests
   Each plugin can ship its own Qt Test suite to validate hardware specific
   behaviour.

Running the full test suite:

.. code-block:: bash

   cmake -B build -DNYMEA_ENABLE_TESTS=ON
   cmake --build build
   ctest --test-dir build

Tests should be executed before every release to ensure compatibility across all
supported platforms.
