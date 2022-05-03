#!/bin/bash

set -eu
trap 'exit 130' INT

TAG=""
EXPERIMENT=""

while getopts ":t:e:" opt; do
  case $opt in
  t)
    TAG="${OPTARG}"
    ;;
  e)
    EXPERIMENT="${OPTARG}"
    ;;
  *)
    echo "Only use -t and -e"
    exit 1
    ;;
  esac
done
shift $((OPTIND - 1))

if [ -z "${TAG}" ]; then
  echo "Provide a tag with -t for the output directory!"
  exit 1
fi
case ${TAG} in
orig | snap) ;;
*)
  echo "-t tag must be one of: orig | snap"
  exit 1
  ;;
esac

if [ -z "${EXPERIMENT}" ]; then
  echo "Provide which experiment you are running with -e!"
  exit 1
fi
case ${EXPERIMENT} in
dicom | dns | dtls | ftp | rtsp) ;;
*)
  echo "-e experiment must be one of: dicom | dns | dtls | ftp | rtsp"
  exit 1
  ;;
esac

ALL_BINS=("./aflnet-replay" "./dcmqrscp-asan" "./dnsmasq-asan" "./dtls-server-asan" "./fftp-asan" "./testOnDemandRTSPServer-asan")
ALL_CONFS=("./conf/dicomclean.sh" "./conf/dicom.dic" "./conf/dnsmasq.conf" "./conf/fftp.conf" "./conf/ftpclean.sh")
ALL_ARTEFACTS=("${ALL_BINS[@]}" "${ALL_CONFS[@]}")
for BIN in "${ALL_ARTEFACTS[@]}"; do
  if [ ! -f "${BIN}" ]; then
    echo "${BIN} is missing."
    exit 1
  fi
done

# TIMENOW=$(date +%Y-%m-%d-%H-%M)
# RESULTS_DIR="autofind_bug_results_${TAG}_${EXPERIMENT}_${TIMENOW}"
RESULTS_DIR="autofind_bug_results_${TAG}_${EXPERIMENT}"

if [ -d "${RESULTS_DIR}" ]; then
  echo "${RESULTS_DIR} directories alredy exists. (Re)move them to continue."
  exit 1
fi

mkdir -p "${RESULTS_DIR}"

IT=1
for BUG_FILE in $(find "${1}" -maxdepth 1 -type f | sort); do
  (
    case ${EXPERIMENT} in
    dicom)
      ./conf/dicomclean.sh
      export DCMDICTPATH="./conf/dicom.dic"
      timeout 2 ./dcmqrscp-asan &>"./${RESULTS_DIR}/${IT}.log"
      ./conf/dicomclean.sh
      ;;

    dns)
      timeout 2 ./dnsmasq-asan -C ./conf/dnsmasq.conf &>"./${RESULTS_DIR}/${IT}.log"
      ;;

    dtls)
      timeout 2 ./dtls-server-asan &>"./${RESULTS_DIR}/${IT}.log"
      ;;

    ftp)
      ./conf/ftpclean.sh
      timeout 2 ./fftp-asan ./conf/fftp.conf 2200 &>"./${RESULTS_DIR}/${IT}.log"
      ./conf/ftpclean.sh
      ;;

    rtsp)
      timeout 2 ./testOnDemandRTSPServer-asan 8554 &>"./${RESULTS_DIR}/${IT}.log"
      ;;
    esac
  ) &

  case ${EXPERIMENT} in
  dicom)
    ./aflnet-replay "${BUG_FILE}" DICOM 5158 &>/dev/null || echo "aflnet-replay died for ${BUG_FILE}"
    ;;

  dns)
    ./aflnet-replay "${BUG_FILE}" DNS 5353 &>/dev/null || echo "aflnet-replay died for ${BUG_FILE}"
    ;;

  dtls)
    ./aflnet-replay "${BUG_FILE}" DTLS12 20220 &>/dev/null || echo "aflnet-replay died for ${BUG_FILE}"
    ;;

  ftp)
    ./aflnet-replay "${BUG_FILE}" FTP 2200 &>/dev/null || echo "aflnet-replay died for ${BUG_FILE}"
    ;;

  rtsp)
    ./aflnet-replay "${BUG_FILE}" RTSP 8554 &>/dev/null || echo "aflnet-replay died for ${BUG_FILE}"
    ;;
  esac

  echo "waiting!"

  wait

  echo "${IT} done!"

  ((IT = IT + 1))
done

ASAN_REPORTS_DIR="${RESULTS_DIR}/asan_reports"

echo "------------------------------------------------------------------------"
echo "We are now grouping errors."
echo "You can find ASan reports in ${ASAN_REPORTS_DIR}."
echo "All other errors are in ${RESULTS_DIR}"
echo "Empty logs will be deleted."

mkdir -p "${ASAN_REPORTS_DIR}"

for BUG_REPORT_FILE in "${RESULTS_DIR}"/*.log; do
  if grep -Fq "SUMMARY:" "${BUG_REPORT_FILE}"; then
    mv "${BUG_REPORT_FILE}" "${ASAN_REPORTS_DIR}"
  elif [[ ! -s ${BUG_REPORT_FILE} ]]; then
    rm "${BUG_REPORT_FILE}"
  fi
done

echo "------------------------------------------------------------------------"
echo "Total unique ASan errors:"

grep -hR "SUMMARY:" "${ASAN_REPORTS_DIR}" | sort -u

echo "------------------------------------------------------------------------"
echo "Total uncategorized errors:"

find "${RESULTS_DIR}" -maxdepth 1 -type f | wc -l
