#!/bin/sh
CODE_DIR=guh
BUILD_DIR=builddir-rpi
USER=root
USER_ID=0
PASSWORD=guh
BINARY=guh
TARGET_IP=${TARGET_IP-10.10.10.108}
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

build() {
    mkdir -p $BUILD_DIR
    cd $BUILD_DIR
    /usr/local/qt5pi/bin/qmake .. CONFIG+=boblight CONFIG+=xcompile
    make -j9
    cd ..
}

sync_build() {
    rsync -rl $BUILD_DIR/ $USER@$TARGET_IP:/root/$BUILD_DIR/
}

run() {
    exec_with_ssh "LD_LIBRARY_PATH=/root/$BUILD_DIR/libguh /root/$BUILD_DIR/server/$BINARY"
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
    echo "Building.."
    build
    echo "Syncing build..."
    sync_build
    echo "Running.."
    run
fi
