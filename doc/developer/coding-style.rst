Coding style
============

nymea follows the Qt coding conventions with a few project-specific rules:

* Use CamelCase for Qt derived classes and snake_case for free functions.
* Prefer ``std::unique_ptr`` and ``QSharedPointer`` over raw pointers.
* Keep includes ordered alphabetically and group Qt headers separately.
* Document public APIs with Doxygen-style comments to enable automatic
  extraction into the Sphinx reference.

Before submitting a change, run the formatting tools and the unit tests outlined
in :doc:`testing`.
