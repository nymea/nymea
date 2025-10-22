Set up the build environment
===========================

This tutorial explains how to prepare a Debian or Ubuntu development system for
building nymea and authoring plugins. The recommended IDE is Qt Creator; other
Linux distributions can be used but may require building libnymea from source.

Install Qt
----------

The Qt libraries and Qt Creator are required for building libnymea and running
the examples:

.. code-block:: console

   sudo apt-get install qtcreator qt5-default qtbase5-dev python3 dpkg-dev debhelper

Install nymea dependencies
--------------------------

Import the nymea repository signing key:

.. code-block:: console

   sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-key A1A19ED6

Add the nymea repository to the system:

.. code-block:: console

   sudo apt-add-repository "deb http://repository.nymea.io $(lsb_release -cs) main"

More detailed instructions are available in the
`nymea installation wiki <https://nymea.io/en/wiki/nymea/master/install>`_.

Update the package lists:

.. code-block:: console

   sudo apt-get update

Install the nymea packages:

.. code-block:: console

   sudo apt-get install nymea nymea-doc libnymea1-dev nymea-dev-tools nymea-qtcreator-wizards

Once the dependencies are installed you can continue with :doc:`getting-started`.
