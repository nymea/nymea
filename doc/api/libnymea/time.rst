Time descriptors
================

``libnymea/time`` contains helper types for schedule based automations.

TimeDescriptor
--------------

``TimeDescriptor`` combines a collection of :ref:`TimeEventItem
<libnymea-time-event>` entries with :ref:`CalendarItem <libnymea-time-calendar>`
blocks.  ``evaluate()`` returns ``true`` when any of the contained items match
a candidate ``QDateTime`` relative to the last evaluation timestamp.
``isValid()``/``isEmpty()`` allow quick checks before persisting the descriptor.

.. _libnymea-time-event:

TimeEventItem
-------------

``TimeEventItem`` defines a single point in time and optional repeating rule.
It stores the base ``QDateTime``/``QTime`` combination, a
``RepeatingOption`` and exposes ``evaluate(lastEvaluation, now)`` so rule
executors can detect transitions.  ``TimeEventItems`` is the container type
with ``get``/``put`` helpers for QML.

.. _libnymea-time-calendar:

CalendarItem
------------

``CalendarItem`` represents a window with a ``startTime()``, ``duration()`` and
``RepeatingOption``.  ``evaluate()`` returns ``true`` while the current
``QDateTime`` lies inside the window.  ``CalendarItems`` behaves like a
``QList`` and mirrors the QML helpers from ``TimeEventItems``.

RepeatingOption
---------------

``RepeatingOption`` is shared by both event and calendar items.  It supports
hourly, daily, weekly, monthly and yearly repetition.  ``weekDays`` and
``monthDays`` hold optional constraints, while ``evaluateWeekDay()`` and
``evaluateMonthDay()`` help compute matches.  ``isValid()`` allows rejecting
incomplete patterns.

