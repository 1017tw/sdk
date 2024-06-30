#!/usr/bin/env bash

# https://stackoverflow.com/questions/4774054/reliable-way-for-a-bash-script-to-get-the-full-path-to-itself
SCRIPT_PATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
# echo "${SCRIPT_PATH}"

CONFIG_FILE=$1
SDK_FOLDER=$2
# echo "config file:${CONFIG_FILE}"
# echo "lib folder:${SDK_FOLDER}"

pushd ${SCRIPT_PATH} > /dev/null
CHIP=$(grep "CONFIG_CHIP.*=y" ${CONFIG_FILE} | sed 's/CONFIG_CHIP_//g; s/=y//g; s/_//g' | awk '{print tolower($0)}')
echo "selected chip is:${CHIP}"

if [ ! -d "${SDK_FOLDER}" ]; then
    echo "sdk folder not exist, sdk post process script return"
    exit 0
fi

# change softlink of libai11xxsdk.a
# pushd ${SDK_FOLDER}/lib > /dev/null
# TARGET_SDK_LIB=`ls | grep ${CHIP}_sdk `
# TARGET_SDK_LINK=libai11xxsdk.a
# ln -sf ${TARGET_SDK_LIB} ${TARGET_SDK_LINK}
# popd > /dev/null

# change softlink of link script
# pushd ${SDK_FOLDER}/ldscript > /dev/null
# TARGET_LINK_SCRIPT=`ls | grep ${CHIP}.ld`
# TARGET_LD_LINK=ai11xx.ld
# ln -sf ${TARGET_LINK_SCRIPT} ${TARGET_LD_LINK}
# popd > /dev/null

# change softlink of libflags
# pushd ${SDK_FOLDER}/scripts > /dev/null
# TARGET_LIB_FLAGS=`ls | grep ${CHIP}_Makefile | grep libflags`
# echo "TARGET_LIB_FLAGS: " ${TARGET_LIB_FLAGS}
# TARGET_LIB_FLAGS=`ls | grep ${CHIP}_ | grep libflags`
# TARGET_LIB_FLAGS_LINK=Makefile.libflags
# ln -sf ${TARGET_LIB_FLAGS} ${TARGET_LIB_FLAGS_LINK}
# popd > /dev/null

# change softlink of libincs
# pushd ${SDK_FOLDER}/scripts > /dev/null
# TARGET_LIB_INCS=`ls | grep ${CHIP}_Makefile | grep libincs`
# TARGET_LIB_INCS=`ls | grep ${CHIP}_ | grep libincs`
# TARGET_LIB_INCS_LINK=Makefile.libincs
# ln -sf ${TARGET_LIB_INCS} ${TARGET_LIB_INCS_LINK}
# popd > /dev/null

# change softlink of spl
pushd ${SDK_FOLDER}/tools/bin2rom/spl > /dev/null
# ai1101a ==> ai110xa
TARGET_SPL=`ls | grep ${CHIP:0:5}x${CHIP:6:6}_nor_spl`

TARGET_SPL_LINK=nor_spl.img
ln -sf ${TARGET_SPL} ${TARGET_SPL_LINK}
popd > /dev/null


popd > /dev/null
