#!/bin/bash

set -eu
trap 'exit 130' INT

DEBUG=0

while getopts "dl" opt; do
  case $opt in
  d)
    echo "Debug mode ON"
    DEBUG=1
    ;;
  *)
    echo "Uknown flag!"
    exit 1
    ;;
  esac
done
shift $((OPTIND - 1))

AFL_BINS=("./afl-fuzz-noaffin")
SBR_BINS=("sabre")
SBR_PLUGINS=("libsbr-afl-and-fs.so")

case "$1" in
dicom)
  PROJECT_NAME="dicom"
  AFL_ARGS="-m 512 -i ./conf/in-dicom -P DICOM -E -K -R"
  TARGET_CONF=""
  TARGET_BINS=("./dcmqrscp")
  export DCMDICTPATH="./conf/dicom.dic"
  ;;
dns)
  PROJECT_NAME="dns"
  AFL_ARGS="-m 512 -i ./conf/in-dns -P DNS -K -R"
  TARGET_CONF="-C ./conf/dnsmasq.conf"
  TARGET_BINS=("./dnsmasq")
  ;;
dtls)
  PROJECT_NAME="dtls"
  AFL_ARGS="-m 512 -i ./conf/in-dtls -P DTLS12 -q 3 -s 3 -E -K -R"
  TARGET_CONF=""
  TARGET_BINS=("./dtls-server")
  ;;
ftp)
  PROJECT_NAME="ftp"
  AFL_ARGS="-m 512 -i ./conf/in-ftp -x ./conf/ftp.dict -P FTP -q 3 -s 3 -E -R"
  TARGET_CONF="./conf/fftp.conf 2200"
  # TARGET_BINS=("./fftp" "./fftp-pthreadjoin")
  TARGET_BINS=("./fftp")
  ;;
rtsp)
  PROJECT_NAME="rtsp"
  AFL_ARGS="-m 512 -i ./conf/in-rtsp -x ./conf/rtsp.dict -P RTSP -q 3 -s 3 -E -K -R"
  TARGET_CONF="8554"
  TARGET_BINS=("./testOnDemandRTSPServer")
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
for SBR_PLUGIN in "${SBR_PLUGINS[@]}"; do
  SABRE_MODE=$(echo "${SBR_PLUGIN}" | cut -d"-" -f3- | cut -d'.' -f1)

  for AFL_BIN in "${AFL_BINS[@]}"; do
    for TARGET_BIN in "${TARGET_BINS[@]}"; do
      for TRY in {0..2}; do
        ALF_RESULT_DIR="aflout-${PROJECT_NAME}-${SABRE_MODE}-${IT}.${TRY}"
        OUTPUT_FILE="output-${PROJECT_NAME}-${SABRE_MODE}-${IT}.${TRY}.txt"

        echo "Running ${IT}.${TRY}: ${AFL_BIN} + ${TARGET_BIN}"
        echo "Output: ${OUTPUT_FILE}, ${ALF_RESULT_DIR}"
        CMD="${AFL_BIN} -A ./${SBR_PLUGIN} ${AFL_ARGS} -o ./${ALF_RESULT_DIR} ${TARGET_BIN} ${TARGET_CONF} &>./${OUTPUT_FILE}"

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
  RESULTS_DIR="results-snapfuzz-${PROJECT_NAME}-${SABRE_MODE}-${TIMENOW}"
  mkdir "${RESULTS_DIR}"
  mv aflout-${PROJECT_NAME}-* output-${PROJECT_NAME}-* "${RESULTS_DIR}"
done
