#!/usr/bin/env bash

set -eu
trap 'exit 130' INT

if [ ! -x "./conf/config.sh" ]; then
  echo "You are not in the correct directory. You should be in .. from where this script lives."
  exit 1
fi

echo "Configuring AFL. This requires sudo access!"

echo core | sudo tee /proc/sys/kernel/core_pattern >/dev/null

echo "Configuring FTP benchmark"

mkdir -p ./ftpshare
chmod 777 ./ftpshare

BENCHMARK_HOME=$(pwd)
sed "s#<absolute_path_to_here>#${BENCHMARK_HOME}#g" ./conf/fftp-template.conf >./conf/fftp.conf

echo "Configuring DICOM benchmark"

mkdir -p ./conf/ACME_STORE
sed "s#<absolute_path_to_here>#${BENCHMARK_HOME}#g" ./conf/dcmqrscp-template.cfg >./conf/dcmqrscp.cfg

echo "Done!"
