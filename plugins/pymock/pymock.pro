TEMPLATE = aux

OTHER_FILES = integrationpluginpymock.json \
              integrationpluginpymock.py


# Copy files to build dir as we've set plugin import paths to that
copydata.commands = $(COPY_DIR) $$PWD/integrationpluginpymock.json $$PWD/*.py $$OUT_PWD || true
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata
