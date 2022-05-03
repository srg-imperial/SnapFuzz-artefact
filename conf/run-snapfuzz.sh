#!/bin/bash

set -eu
trap 'exit 130' INT

DEBUG=0
LONG_RUN=0

while getopts "dl" opt; do
  case $opt in
  d)
    echo "Debug mode ON"
    DEBUG=1
    ;;
  l)
    echo "24h long run ON"
    LONG_RUN=1
    ;;
  *)
    echo "Uknown flag!"
    exit 1
    ;;
  esac
done
shift $((OPTIND - 1))

if [ "${LONG_RUN}" == 1 ]; then
  AFL_BINS=("./afl-fuzz-noaffin-long")
  AFL_WRAPPER="timeout --preserve-status --signal=SIGINT --kill-after=10 24h"
else
  AFL_BINS=("./afl-fuzz-noaffin")
  # AFL_BINS=("./afl-fuzz-noaffin" "./afl-fuzz-noaffin-print")
  AFL_WRAPPER=""
fi

SBR_BINS=("sabre")
SBR_PLUGINS=("libsbr-afl.so")

case "$1" in
dicom)
  PROJECT_NAME="dicom"
  AFL_ARGS="-m 512 -i ./conf/in-dicom -P DICOM -E -K -R"
  TARGET_CONF=""
  TARGET_BINS=("./dcmqrscp")
  export DCMDICTPATH="./conf/dicom.dic"
  ;;
dicom-asan)
  PROJECT_NAME="dicom-asan"
  AFL_ARGS="-t 1000 -m none -i ./conf/in-dicom -P DICOM -E -K -R"
  TARGET_CONF=""
  TARGET_BINS=("./dcmqrscp-asan")
  export DCMDICTPATH="./conf/dicom.dic"
  ;;

dns)
  PROJECT_NAME="dns"
  AFL_ARGS="-m 512 -i ./conf/in-dns -P DNS -K -R"
  TARGET_CONF="-C ./conf/dnsmasq.conf"
  TARGET_BINS=("./dnsmasq")
  ;;
dns-asan)
  PROJECT_NAME="dns-asan"
  AFL_ARGS="-t 1000 -m none -i ./conf/in-dns -P DNS -K -R"
  TARGET_CONF="-C ./conf/dnsmasq.conf"
  TARGET_BINS=("./dnsmasq-asan")
  ;;

dtls)
  PROJECT_NAME="dtls"
  AFL_ARGS="-m 512 -i ./conf/in-dtls -P DTLS12 -q 3 -s 3 -E -K -R"
  TARGET_CONF=""
  TARGET_BINS=("./dtls-server")
  ;;
dtls-asan)
  PROJECT_NAME="dtls-asan"
  AFL_ARGS="-t 1000 -m none -i ./conf/in-dtls -P DTLS12 -q 3 -s 3 -E -K -R"
  TARGET_CONF=""
  TARGET_BINS=("./dtls-server-asan")
  ;;

ftp)
  PROJECT_NAME="ftp"
  AFL_ARGS="-m 512 -i ./conf/in-ftp -x ./conf/ftp.dict -P FTP -q 3 -s 3 -E -R"
  TARGET_CONF="./conf/fftp.conf 2200"
  # TARGET_BINS=("./fftp" "./fftp-pthreadjoin")
  TARGET_BINS=("./fftp")
  ;;
ftp-asan)
  PROJECT_NAME="ftp-asan"
  AFL_ARGS="-t 1000 -m none -i ./conf/in-ftp -x ./conf/ftp.dict -P FTP -q 3 -s 3 -E -R"
  TARGET_CONF="./conf/fftp.conf 2200"
  TARGET_BINS=("./fftp-asan")
  ;;
ftp-tsan)
  PROJECT_NAME="ftp-tsan"
  AFL_ARGS="-t 1000 -m none -i ./conf/in-ftp -x ./conf/ftp.dict -P FTP -q 3 -s 3 -E -R"
  TARGET_CONF="./conf/fftp.conf 2200"
  TARGET_BINS=("./fftp-tsan")
  ;;
ftp-pth)
  PROJECT_NAME="ftp-pth"
  AFL_ARGS="-m 512 -i ./conf/in-ftp -x ./conf/ftp.dict -P FTP -q 3 -s 3 -E -R"
  TARGET_CONF="./conf/fftp.conf 2200"
  TARGET_BINS=("./fftp")
  export AFL_PRELOAD="./lib2pthread.so"
  ;;

rtsp)
  PROJECT_NAME="rtsp"
  AFL_ARGS="-m 512 -i ./conf/in-rtsp -x ./conf/rtsp.dict -P RTSP -q 3 -s 3 -E -K -R"
  TARGET_CONF="8554"
  TARGET_BINS=("./testOnDemandRTSPServer")
  ;;
rtsp-asan)
  PROJECT_NAME="rtsp-asan"
  AFL_ARGS="-t 1000 -m none -i ./conf/in-rtsp -x ./conf/rtsp.dict -P RTSP -q 3 -s 3 -E -K -R"
  TARGET_CONF="8554"
  TARGET_BINS=("./testOnDemandRTSPServer-asan")
  ;;

*)
  echo "Unknown command. Try one of {dicom,dns,dtls,ftp,rtsp}"
  exit 1
  ;;
esac

ALL_BINS=("${AFL_BINS[@]}" "${TARGET_BINS[@]}" "${SBR_BINS[@]}" "${SBR_PLUGINS[@]}")
for BIN in "${ALL_BINS[@]}"; do
  if [ ! -f "${BIN}" ]; then
    echo "${BIN} is missing."
    exit 1
  fi
done

if [ "$(find . -maxdepth 1 -name "aflout-${PROJECT_NAME}-*" | wc -l)" -ne 0 ]; then
  echo "Aflout directories alredy exists. (Re)move them to continue."
  exit 1
fi
if [ "$(find . -maxdepth 1 -name "output-${PROJECT_NAME}-*" | wc -l)" -ne 0 ]; then
  echo "Output files alredy exists. (Re)move them to continue."
  exit 1
fi

echo "++++++++++++++++++++++++++++ STATS ++++++++++++++++++++++++++++"
echo "PWD: $(pwd)"
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

IT=1
for AFL_BIN in "${AFL_BINS[@]}"; do
  for TARGET_BIN in "${TARGET_BINS[@]}"; do
    for TRY in {0..19}; do
      ALF_RESULT_DIR="aflout-${PROJECT_NAME}-${IT}.${TRY}"
      OUTPUT_FILE="output-${PROJECT_NAME}-${IT}.${TRY}.txt"

      echo "Running ${IT}.${TRY}: ${AFL_BIN} + ${TARGET_BIN}"
      echo "Output: ${OUTPUT_FILE}, ${ALF_RESULT_DIR}"
      CMD="${AFL_WRAPPER} ${AFL_BIN} -A ./libsbr-afl.so ${AFL_ARGS} -o ./${ALF_RESULT_DIR} ${TARGET_BIN} ${TARGET_CONF} &>./${OUTPUT_FILE}"

      if [ "${DEBUG}" == 1 ]; then
        echo "${CMD}"
        exit 1
      else
        # eval "AFL_BENCH_JUST_ONE=1 ${CMD}"
        eval "${CMD}" || true
      fi
      echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
    done
    ((IT = IT + 1))
  done
done

TIMENOW=$(date +%Y-%m-%d-%H-%M)
RESULTS_DIR="results-snapfuzz-${PROJECT_NAME}-${TIMENOW}"
mkdir "${RESULTS_DIR}"
mv aflout-${PROJECT_NAME}-* output-${PROJECT_NAME}-* "${RESULTS_DIR}"
