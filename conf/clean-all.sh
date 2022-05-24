#!/usr/bin/env bash

set -eu
trap 'exit 130' INT

# Delete all AFL related binaries
rm -f afl-fuzz
rm -f afl-fuzz-*
rm -f aflnet-replay

# Delete all AFL run related files and directories (only unfinished runs)
rm -rf aflout-*
rm -f output-*.txt

# Delete builds
rm -rf builds/

# Delete generated configs
rm -f conf/dcmqrscp.cfg
rm -f conf/fftp.conf
rm -rf conf/ACME_STORE/

# Delete generated binaries for each target
rm -f dcmqrscp
rm -f dcmqrscp-asan
rm -f dnsmasq
rm -f dnsmasq-asan
rm -f dtls-server
rm -f dtls-server-asan
rm -f fftp
rm -f fftp-asan
rm -f fftp-tsan
rm -f testOnDemandRTSPServer
rm -f testOnDemandRTSPServer-asan

# Delete generated SnapFuzz binaries
rm -f libsbr-*.so
rm -f sabre

# Delete input files of targets
rm -f test.aac
rm -f test.ac3
rm -f test.mkv
rm -f test.mp3
rm -f test.mpg
rm -f test.wav
rm -f test.webm
rm -rf ftpshare/
