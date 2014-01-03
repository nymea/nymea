#!/bin/sh
CODE_DIR=Hive
BUILD_DIR=builddir
USER=root
USER_ID=0
PASSWORD=hive
BINARY=hive
TARGET_IP=${TARGET_IP-10.10.10.125}
TARGET_SSH_PORT=22
TARGET_DEBUG_PORT=3768
RUN_OPTIONS=-qmljsdebugger=port:$TARGET_DEBUG_PORT
SETUP=false
GDB=false
SUDO="echo $PASSWORD | sudo -S"
NUM_JOBS='$(( `grep -c ^processor /proc/cpuinfo` + 1 ))'
FLIPPED=false

usage() {
    echo "usage: run_on_device [OPTIONS]\n"
    echo "Script to setup a build environment and sync build and run $BINARY on the device\n"
    echo "OPTIONS:"
    echo "  -s, --setup   Setup the build environment"
    echo ""
    echo "IMPORTANT:"
    echo " * Make sure to have networking setup on the device beforehand."
    echo " * Execute that script from a directory containing $PACKAGE code."
    exit 1
}

exec_with_ssh() {
    ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -t $USER@$TARGET_IP -p $TARGET_SSH_PORT sudo -u $USER -i bash -ic \"$@\"
}

install_ssh_key() {
    ssh-keygen -R $TARGET_IP
    ssh-copy-id $USER@$TARGET_IP
}

install_dependencies() {
    exec_with_ssh $SUDO apt-get -y install build-essential gcc-4.7 g++-4.7 ccache gdb
    echo "** Switching system to gcc 4.7 ***"
    exec_with_ssh update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.7 40 --slave /usr/bin/g++ g++ /usr/bin/g++-4.7
}

sync_code() {
    WORK_DIR=`pwd`
    [ -e .bzr ] && bzr export --uncommitted --format=dir /tmp/$CODE_DIR
    [ -e .git ] && cd /tmp && cp -r $WORK_DIR $CODE_DIR && cd $CODE_DIR && git clean -f -x && rm .git -rf && cd -
    rsync -crlOzv --delete --exclude builddir -e "ssh -p $TARGET_SSH_PORT -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no" /tmp/$CODE_DIR/ $USER@$TARGET_IP:$CODE_DIR/
    rm -rf /tmp/$CODE_DIR
}

build() {
    exec_with_ssh mkdir -p $CODE_DIR/$BUILD_DIR
    exec_with_ssh QT_SELECT=qt5 PATH=/usr/local/qt5/bin:/usr/lib/ccache:$PATH "cd $CODE_DIR/$BUILD_DIR && PATH=/usr/local/qt5/bin:/usr/lib/ccache:$PATH qmake .."
    exec_with_ssh PATH=/usr/lib/ccache:$PATH "cd $CODE_DIR/$BUILD_DIR && PATH=/usr/lib/ccache:$PATH make -j2"
}

run() {
    exec_with_ssh "LD_LIBRARY_PATH=$CODE_DIR/$BUILD_DIR/libhive $CODE_DIR/$BUILD_DIR/server/$BINARY"
}

set -- `getopt -n$0 -u -a --longoptions="setup,gdbhelp" "sgh" "$@"`

# FIXME: giving incorrect arguments does not call usage and exit
while [ $# -gt 0 ]
do
    case "$1" in
       -s|--setup)   SETUP=true;;
       -g|--gdb)     GDB=true;;
       -h|--help)    usage;;
       --)           shift;break;;
    esac
    shift
done


if $SETUP; then
    echo "Setting up environment for building $PACKAGE..."
    install_ssh_key
    install_dependencies
    sync_code
else
    echo "Transferring code.."
    sync_code

    export PATH="/usr/local/qt5/:$PATH"

    echo "Building.."
    build
    echo "Running.."
    run
fi
