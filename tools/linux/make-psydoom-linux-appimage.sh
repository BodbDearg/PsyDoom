#/bin/sh

# This script packages up a Linux appimage for PsyDoom.
# It will output the app image in the current directory along with some temporaries.
#
# It takes 4 arguments:
# (1) The path to 'appimagetool'.
#     This can be downloaded from: https://github.com/AppImage/AppImageKit/releases
# (2) The path to 'linuxdeploy'.
#     This can be downloaded from: https://github.com/linuxdeploy/linuxdeploy/releases
# (3) The path to the PsyDoom source code (root folder).
# (4) The path to the compiled binary for PsyDoom for the target architecture.
#
# Note: 'appimagetool' and 'linuxdeploy' might require various packages to be installed.
# Run them and examine any errors to see what is missing.
#

# Exit on any error
set -e

# Extract the input arguments
if [ $# -ne 4 ]; then
    echo "Expected args: <'appimagetool' path> <'linuxdeploy' path> <PsyDoom source root> <PsyDoom binary path>"
    exit 1
fi

export APP_IMAGE_TOOL_PATH="$1"
export LINUX_DEPLOY_PATH="$2"
export PSYDOOM_ROOT_PATH="$3"
export PSYDOOM_BINARY_PATH="$4"

# Various paths
export APP_DIR_PATH="AppDir"
export DESKTOP_FILE="PsyDoom.desktop"
export SRC_ICON_DIR="$PSYDOOM_ROOT_PATH/game/Resources/Generic"
export SRC_MAIN_ICON_FILE_PATH="$SRC_ICON_DIR/icon_32.png"
export DST_MAIN_ICON_FILE_PATH="PsyDoom.png"

# Ensure the PsyDoom binary is executable
chmod +x "$PSYDOOM_BINARY_PATH"

# Cleanup previous output and some temporaries
rm -rf "$APP_DIR_PATH"
rm -rf "$DESKTOP_FILE"
rm -rf "$DST_MAIN_ICON_FILE_PATH"

export BINARY_FILES=`find -type f -name "PsyDoom*.AppImage"`

for f in $BINARY_FILES; do
  rm "$f"
done

# Make the main icon accessible
cp "$SRC_MAIN_ICON_FILE_PATH" "$DST_MAIN_ICON_FILE_PATH"

# Generate the desktop file
echo "[Desktop Entry]" > "$DESKTOP_FILE"
echo "Name=PsyDoom" >> "$DESKTOP_FILE"
echo "Exec=PsyDoom" >> "$DESKTOP_FILE"
echo "Icon=PsyDoom" >> "$DESKTOP_FILE"
echo "Type=Application" >> "$DESKTOP_FILE"
echo "Categories=Game" >> "$DESKTOP_FILE"

# Use the 'linuxdeploy' tool to build the AppDir
"$LINUX_DEPLOY_PATH"\
  --appdir="$APP_DIR_PATH"\
  --executable="$PSYDOOM_BINARY_PATH"\
  --desktop-file="$DESKTOP_FILE"\
  --icon-filename="PsyDoom"\
  --icon-file="$DST_MAIN_ICON_FILE_PATH"\
  --icon-file="$SRC_ICON_DIR/icon_8.png"\
  --icon-file="$SRC_ICON_DIR/icon_16.png"\
  --icon-file="$SRC_ICON_DIR/icon_20.png"\
  --icon-file="$SRC_ICON_DIR/icon_22.png"\
  --icon-file="$SRC_ICON_DIR/icon_24.png"\
  --icon-file="$SRC_ICON_DIR/icon_28.png"\
  --icon-file="$SRC_ICON_DIR/icon_32.png"\
  --icon-file="$SRC_ICON_DIR/icon_36.png"\
  --icon-file="$SRC_ICON_DIR/icon_42.png"\
  --icon-file="$SRC_ICON_DIR/icon_48.png"\
  --icon-file="$SRC_ICON_DIR/icon_64.png"\
  --icon-file="$SRC_ICON_DIR/icon_72.png"\
  --icon-file="$SRC_ICON_DIR/icon_96.png"\
  --icon-file="$SRC_ICON_DIR/icon_128.png"\
  --icon-file="$SRC_ICON_DIR/icon_160.png"\
  --icon-file="$SRC_ICON_DIR/icon_192.png"\
  --icon-file="$SRC_ICON_DIR/icon_256.png"\
  --icon-file="$SRC_ICON_DIR/icon_384.png"\
  --icon-file="$SRC_ICON_DIR/icon_480.png"\
  --icon-file="$SRC_ICON_DIR/icon_512.png"

# Generate the appimage from the AppDir, using 'appimagetool'
"$APP_IMAGE_TOOL_PATH" "$APP_DIR_PATH"

# Cleanup some temporaries (leave the AppDir for manual inspection however)
rm -rf "$DESKTOP_FILE"
rm -rf "$DST_MAIN_ICON_FILE_PATH"
