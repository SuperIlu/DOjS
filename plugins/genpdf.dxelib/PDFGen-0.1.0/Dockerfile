FROM ubuntu:20.04

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y dirmngr
RUN apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 86B72ED9 \
 && echo "deb http://mirror.mxe.cc/repos/apt xenial main" > /etc/apt/sources.list.d/mxeapt.list

RUN dpkg --add-architecture i386
RUN apt-get update && apt-get install --no-install-recommends -y \
	ca-certificates \
	clang-10 \
	clang-format-10 \
	clang-tools-10 \
	cloc \
	colordiff \
	cppcheck \
	curl \
	doxygen \
	gcc \
	g++ \
	ghostscript \
	git \
	graphviz \
	libtinfo5 \
	llvm-10 \
	make \
	mxe-i686-w64-mingw32.static-gcc \
	pdftk-java \
	poppler-utils \
	python2.7 \
	python3-pip \
	python3-setuptools \
	software-properties-common \
	tzdata \
	valgrind \
	vim \
	xz-utils

RUN python3 -m pip install cpp-coveralls

# Install Infer
RUN mkdir -p /opt && curl -L https://github.com/facebook/infer/releases/download/v1.0.0/infer-linux64-v1.0.0.tar.xz | tar -C /opt -x -J
ENV PATH $PATH:/opt/infer-linux64-v1.0.0/bin/

# Install acroread
RUN apt-get install -y --no-install-recommends \
	libgtk2.0-0:i386 \
	libxml2:i386
RUN curl -L -O http://ardownload.adobe.com/pub/adobe/reader/unix/9.x/9.5.5/enu/AdbeRdr9.5.5-1_i486linux_enu.bin && chmod +x AdbeRdr9.5.5-1_i486linux_enu.bin && ./AdbeRdr9.5.5-1_i486linux_enu.bin --install_path=/opt

RUN apt-get clean
