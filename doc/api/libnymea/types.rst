.. _libnymea-types:

type system
===========

Identifiers and enums
---------------------

``typeutils.h`` defines the ``DECLARE_TYPE_ID`` helper used throughout the
library.  It provides strongly typed wrappers around ``QUuid`` for entities
such as ``ThingId``, ``ThingClassId``, ``ParamTypeId`` and ``PluginId``.  The
``Types`` utility class also exposes shared enums:

* ``InputType`` — hints for user interfaces (text line, password, search,
  IPv4/IPv6, URL, MAC address).
* ``Unit`` — the canonical units used for state and param descriptors.
* ``ValueOperator`` and ``StateOperator`` — comparison and boolean operators.
* ``BrowserType`` — identifies generic browser capabilities.
* ``IOType`` — physical IO channels exposed by :ref:`IOConnection
  <libnymea-integrations-io>`.
* ``StateValueFilter`` — named filter implementations for state smoothing.
* ``PermissionScope`` — OAuth scopes represented as flags.
* ``LoggingType`` and ``SampleRate`` — logging semantics and sampling hints.

Parameters
----------

* ``Param`` stores a ``ParamTypeId``/value pair.  ``ParamList`` is the
  ``QList`` container used in most APIs and offers ``get``/``put`` for QML.
* ``ParamType`` describes a parameter: it contains identifiers, display
  metadata, validation hints (value range, allowed values, ``Types::Unit``) and
  default values.  ``ParamTypes`` is the matching list wrapper.
* ``ParamDescriptor`` extends ``Param`` with descriptor level fields and is
  grouped in ``ParamDescriptors``.
* ``InterfaceParamType``/``InterfaceParamTypes`` specialise ``ParamType`` for
  interface definitions and add the owning interface identifier.

States
------

* ``State`` mirrors ``Param`` for runtime values and keeps track of minimum,
  maximum and possible values.  ``States`` is the container type referenced by
  :ref:`Thing <libnymea-integrations-thing>`.
* ``StateType`` describes a ``State``: it holds identifiers, names, units,
  value type information and logging defaults.  ``StateTypes`` is the
  associated list type.
* ``StateDescriptor`` packages a ``StateTypeId`` with display metadata to
  simplify configuration UIs.
* ``InterfaceStateType``/``InterfaceStateTypes`` extend ``StateType`` for use in
  interface catalogues.

Events
------

* ``Event`` contains an ``EventTypeId`` and the emitted ``ParamList``.  It is
  used by ``Thing::emitEvent()`` and ``IntegrationPlugin::emitEvent``.
* ``EventType`` and ``EventTypes`` describe available events for a thing
  class.  They include severity, logging flags and the relevant params.
* ``EventDescriptor`` / ``EventDescriptors`` mirror ``StateDescriptor`` for
  events.
* ``InterfaceEventType``/``InterfaceEventTypes`` extend ``EventType`` for
  interface level documentation.

Actions
-------

* ``Action`` bundles an ``ActionTypeId`` with its ``ThingId`` target,
  parameters and the ``TriggeredBy`` origin (user, rule, script).
* ``ActionType``/``ActionTypes`` describe executable actions, including return
  behaviour, parameter schema and logging defaults.
* ``InterfaceActionType``/``InterfaceActionTypes`` extend ``ActionType`` for
  shared interface catalogues.

Thing classes and vendors
-------------------------

* ``Vendor``/``Vendors`` describe manufacturers along with logo and homepage
  metadata.
* ``ThingClass`` stores the full metadata for a device class: vendor, name,
  creation methods, default params, supported states/events/actions, and
  interface hints.  ``ThingClasses`` is the ``QList`` façade used throughout
  the integrations API.
* ``Interface``/``Interfaces`` enumerate optional feature sets (for example the
  alarm interface) and reference their param/event/action collections.
* ``MediaBrowserItem`` specialises :ref:`BrowserItem <libnymea-types-browser>`
  with additional media metadata (artist, album, duration, track numbers).

.. _libnymea-types-browser:

Browser model
-------------

* ``BrowserItem`` represents a single entry in a browsable hierarchy.  It
  exposes identifiers, display strings, icon hints, thumbnails, executability
  flags and associated ``ActionTypeId`` values.  ``BrowserItems`` is the list
  wrapper.
* ``BrowserAction`` describes a generic action triggered from a browse
  context.  ``BrowserItemAction`` does the same for a specific
  ``BrowserItem``.  Both are lightweight value types with ids and human
  readable captions and carry an optional ``ParamList`` payload.

