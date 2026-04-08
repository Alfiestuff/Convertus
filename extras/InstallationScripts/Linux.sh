#!/bin/bash

set -e

APP_NAME="convertus"
REPO="Alfiestuff/Convertus"
VERSION="v1.0.0"
URL="https://github.com/$REPO/archive/refs/tags/$VERSION.tar.gz"

TMP_DIR=$(mktemp -d)

echo "Downloading $APP_NAME $VERSION..."

cd "$TMP_DIR"
curl -L "$URL" -o source.tar.gz

echo "Extracting..."
tar -xzf source.tar.gz

cd "$APP_NAME-$VERSION" || cd convertus-$VERSION

echo "Building..."

g++ Linux/main.cpp -o $APP_NAME -std=c++20 -O3 -march=native

echo "Installing..."

sudo install -Dm755 $APP_NAME /usr/local/bin/$APP_NAME

echo "Done."
echo "Installed to /usr/local/bin/$APP_NAME"

cd ~
rm -rf "$TMP_DIR"
