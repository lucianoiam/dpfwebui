#!/bin/sh

REMOTE_REPO=https://github.com/stagas/x-waveform
LOCAL_REPO=x-waveform
TAG=v1.0.1
ESBUILD_BIN=node_modules/esbuild/bin/esbuild
OUTPUT_DIR=dist

git clone --depth 1 --branch $TAG $REMOTE_REPO $LOCAL_REPO 2> /dev/null \
    || (cd $LOCAL_REPO ; git pull)

patch $LOCAL_REPO/src/x-waveform.ts x-waveform.ts.diff

cd $LOCAL_REPO
npm install

npm install esbuild
mkdir -p $OUTPUT_DIR

$ESBUILD_BIN --format=esm --minify --bundle \
             --outfile=../$OUTPUT_DIR/x-waveform.js \
             src/x-waveform.ts

#$ESBUILD_BIN --format=esm --minify --bundle \
#             --outfile=../$OUTPUT_DIR/x-waveform-worker.js \
#             src/x-waveform-worker.ts
