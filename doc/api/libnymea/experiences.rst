Experiences
===========

``ExperiencePlugin`` provides the base class for immersive nymea interfaces
such as the tablet UI.  The runtime injects ``ThingManager`` and
``JsonRPCServer`` pointers via the private ``initPlugin`` helper before calling
``init()``.  Subclasses use these entry points to expose custom visualisations
or orchestrate higher level automation behaviour.

Register experience plugins with ``Q_PLUGIN_METADATA`` and export the
``io.nymea.ExperiencePlugin`` interface as declared in the header.

