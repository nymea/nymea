.. _nymea-documentation:

nymea developer documentation
=============================

Welcome to the nymea developer documentation. The material is organised into
three major sections covering the server runtime, the C++ ``libnymea`` API and
client integrations via JSON-RPC.

.. rubric:: Overview

* :doc:`guide/index` contains tutorials and background material for authoring
  plugins and working with the nymea build environment.
* :doc:`reference/api-index` exposes the complete C++ API reference generated
  from the ``libnymea`` sources using Doxygen and Breathe.
* :doc:`reference/jsonrpc` documents the JSON-RPC protocol that clients use to
  interact with the nymea daemon.
* :doc:`reference/generated/jsonrpc-api` mirrors the machine introspection data
  that is available via ``JSONRPC.Introspect`` at runtime.
* :doc:`reference/interfaces` lists all generated interfaces that are available
  to plugin authors.

Quick links
-----------

* :ref:`All libnymea classes <reference/api-index>`
* :doc:`reference/interfaces`
* :doc:`reference/generated/jsonrpc-api`

.. toctree::
   :maxdepth: 2
   :caption: Contents

   guide/index
   reference/index
   reference/jsonrpc
   reference/generated/jsonrpc-api
   reference/interfaces
