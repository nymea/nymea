CoAP client helpers
===================

``libnymea/coap`` offers a lightweight CoAP client implementation geared
towards plugin developers who need to communicate with constrained devices.

CoapRequest and options
-----------------------

``CoapRequest`` stores the target host, resource path, URI query parameters and
optional confirmable/non-confirmable flags.  ``CoapOption`` represents a single
CoAP option and is primarily used when constructing or parsing PDUs.

CoapPdu and blocks
------------------

``CoapPdu`` models the raw protocol data unit, including message id, token,
code, payload and options.  ``CoapPduBlock`` provides helpers for blockwise
transfers (RFC 7959) so large payloads can be segmented and reassembled.

Core link format
----------------

``CoreLink`` is a convenience wrapper for RFC 6690 link-format entries.  Use
``CoreLinkParser`` to parse ``/.well-known/core`` payloads into structured
objects.

Coap and replies
----------------

``Coap`` is the high level client that sends requests over UDP.  It exposes the
usual REST verbs plus ``customRequest()`` for vendor specific codes.  Use
``enableResourceNotifications()``/``disableNotifications()`` for observe
support and ``joinMulticastGroup()``/``leaveMulticastGroup()`` for group
communication.  ``Coap`` emits ``notificationReceived`` whenever an observe
resource pushes an update.

``CoapReply`` wraps an in-flight request, exposing ``statusCode()``,
``payload()``, ``options()`` and progress signals for blockwise transfers.  It
also emits ``finished``/``error`` when a request completes or times out.

``CoapObserveResource`` tracks observe relationships by token and resource path
so the client can correlate notifications to their originating subscription.

