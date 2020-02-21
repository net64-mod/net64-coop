#!/bin/bash

# Creates an AppImage in the directory it's called from
# $1 - appdir

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

wget -nc https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage

chmod +x linuxdeploy*.AppImage

export OUTPUT="Net64.AppImage"

./linuxdeploy-x86_64.AppImage --appdir $1 -d "${DIR}/../dist/Net64.desktop" -i "${DIR}/../dist/net64.png" --output appimage
