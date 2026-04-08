#!/bin/bash
set -e

APP_NAME="convertus"
REPO="Alfiestuff/Convertus"
VERSION="v1.0.0"
URL="https://github.com/$REPO/archive/refs/tags/$VERSION.tar.gz"

TMP_DIR=$(mktemp -d)

cleanup() {
    rm -rf "$TMP_DIR"
}
trap cleanup EXIT

echo "Downloading $APP_NAME $VERSION..."

cd "$TMP_DIR"
curl -L "$URL" -o source.tar.gz

echo "Extracting..."
tar -xzf source.tar.gz

# Get extracted folder dynamically (FIXED)
EXTRACTED_DIR=$(find . -maxdepth 1 -type d -name "*$VERSION*" | head -n 1)

if [ -z "$EXTRACTED_DIR" ]; then
    echo "Failed to find extracted directory"
    exit 1
fi

cd "$EXTRACTED_DIR"

echo "Building..."
g++ Linux/main.cpp -o "$APP_NAME" -std=c++20 -O3 -march=native

echo "Installing..."

install -Dm755 "$APP_NAME" "$HOME/.local/bin/$APP_NAME"

echo "Done."
echo "Installed to $HOME/.local/bin/$APP_NAME"
