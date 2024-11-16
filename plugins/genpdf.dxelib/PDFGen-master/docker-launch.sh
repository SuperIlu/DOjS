#!/bin/sh
# Utility script to build the docker development environment,
# and then launch it with the current directory mounted inside.
set -e
docker build -t andredesigna/pdfgen - < Dockerfile
docker run --privileged -ti -v `pwd`:`pwd` -w `pwd` -u `id -u`:`id -g` andredesigna/pdfgen

# Build/push to dockerhub, for cloud builds:
# docker push andredesigna/pdfgen