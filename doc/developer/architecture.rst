Architecture overview
=====================

nymea consists of multiple layers:

Server core
   Implements device management, automation rules, and the event bus.

Plugin engine
   Dynamically loads device plugins that connect to hardware or cloud services.

Client APIs
   JSON-RPC, CoAP, and libnymea bindings expose the platform to external
   applications.

Data store
   Persists configuration and state in an embedded database so that restarts are
   fast and reliable.

The architecture encourages decoupling through signals and slots (Qt), making it
simple to extend the platform by adding new plugins or protocol adapters without
modifying the server core.
