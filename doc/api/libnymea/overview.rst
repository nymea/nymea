Overview
========

``libnymea`` follows Qt idioms: most public types are `QObject` based,
expose change notifications through signals, and rely on ids implemented as
``QUuid`` wrappers from :ref:`Types <libnymea-types>`.

Header files live under ``libnymea/`` and are grouped by domain.  Unless
explicitly noted the classes are exported through ``LIBNYMEA_EXPORT`` and can
be used by third party plugins without linking against private server code.

The documentation mirrors the source tree and uses the following
conventions:

* Containers declared as ``QList`` or ``QVector`` subclasses behave like the
  underlying Qt container while adding ``get``/``put`` helpers for QML.
* ``ThingError`` enumerations follow ``Thing::ThingError`` from
  :ref:`Thing <libnymea-integrations-thing>`.
* All asynchronous jobs emit ``finished``/``error`` signals rather than
  blocking.
* Hardware resources inherit from :ref:`HardwareResource
  <libnymea-core-hardware-resource>` and are retrieved through
  :ref:`HardwareManager <libnymea-core-hardware-manager>`.

Each page in this section lists the available classes, their construction
patterns, and the most relevant signals and slots.

