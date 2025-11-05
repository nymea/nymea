CoAP API
========

nymea offers a lightweight CoAP interface for constrained devices. The API is
exposed on UDP port 5678 and follows the observe pattern for streaming updates.

Resource layout
---------------

``/coap/devices``
   Returns a list of registered devices.

``/coap/devices/<id>``
   Provides details and allows observing state changes.

Security
--------

DTLS is used for securing CoAP communication. Certificates can be generated via
the nymea command line utilities or provisioned by plugins.
