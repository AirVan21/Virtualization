#!/bin/bash
# Provide paths or use defaults
AUCONT_TEST_DOCKER_CONTAINER_IMAGE_NAME=${1:-eabatalov/aucont16-test-base}
AUCONT_TOOLS_BIN_DIR_PATH=${2:-`pwd`/../tools/}
AUCONT_TEST_SCRIPTS_DIR_PATH=${3:-`pwd`/../scripts/}
AUCONT_TEST_CONT_ROOTFS_DIR_PATH=${4:-`pwd`/../rootfs/}

echo 'Using aucont tools' $AUCONT_TOOLS_BIN_DIR_PATH
echo 'Using test scripts' $AUCONT_TEST_SCRIPTS_DIR_PATH
echo 'Using rootfs' $AUCONT_TEST_CONT_ROOTFS_DIR_PATH

sudo docker run -ti \
    -v ${AUCONT_TOOLS_BIN_DIR_PATH}:'/test/aucont' \
    -v ${AUCONT_TEST_SCRIPTS_DIR_PATH}:'/test/scripts' \
    -v ${AUCONT_TEST_CONT_ROOTFS_DIR_PATH}:'/test/rootfs' \
    --net=host --pid=host --privileged=true \
    ${AUCONT_TEST_DOCKER_CONTAINER_IMAGE_NAME}
