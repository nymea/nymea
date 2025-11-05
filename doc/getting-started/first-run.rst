First run
=========

When nymead starts for the first time it creates a default configuration and a
self-signed TLS certificate. Use the nymea app or the command-line tools to
connect to the server.

1. Ensure :command:`nymead` is running.
2. Launch the nymea app on your preferred platform and connect to the host.
3. Follow the wizard to create an administrator user and pair initial devices.

You can also verify the JSON-RPC API manually:

.. code-block:: bash

   curl -X POST https://localhost:4443/jsonrpc \
        -H 'Content-Type: application/json' \
        -d '{"id": 1, "jsonrpc": "2.0", "method": "JSONRPC.Hello"}'

A successful response confirms that nymead is reachable and ready for further
configuration.
