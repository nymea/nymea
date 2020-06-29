TEMPLATE = aux

OTHER_FILES = integrationpluginpymock.json \
              integrationpluginpymock.py


# Copy files to build dir as we've set plugin import paths to that
#mytarget.target = integrationpluginpymock.py
#mytarget.commands = cp $$PWD/$$mytarget.target $$mytarget.target
#mytarget.depends = mytarget2

#mytarget2.commands = cp $$PWD/integrationpluginpymock.json integrationpluginpymock.json

#QMAKE_EXTRA_TARGETS += mytarget mytarget2



copydata.commands = $(COPY_DIR) $$PWD/integrationpluginpymock.json $$PWD/*.py $$OUT_PWD
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata
