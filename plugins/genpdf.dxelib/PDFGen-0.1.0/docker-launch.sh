#!/bin/sh
# Utility script to build the docker development environment,
# and then launch it with the current directory mounted inside.
set -e
docker build -t pdfgen_image - < Dockerfile
docker run --privileged -ti -v `pwd`:`pwd` -w `pwd` -u `id -u`:`id -g` pdfgen_image
