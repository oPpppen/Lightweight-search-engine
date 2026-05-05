#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ONLINE_DIR=$(cd "${SCRIPT_DIR}/.." && pwd)
PROJECT_ROOT=$(cd "${ONLINE_DIR}/.." && pwd)

OFFLINE_DATA_DIR=${1:-"${PROJECT_ROOT}/build/data"}
ONLINE_DATA_DIR="${ONLINE_DIR}/data"
ONLINE_BUILD_DATA_DIR="${ONLINE_DIR}/build/data"
ROOT_BUILD_DATA_DIR="${PROJECT_ROOT}/build/data"
STOPWORDS_FILE="${PROJECT_ROOT}/keyword/stopwords/stopwords_cn.txt"

copy_if_needed() {
  local src=$1
  local dst=$2

  if [[ "${src}" == "${dst}" ]]; then
    return 0
  fi

  cp "${src}" "${dst}"
}

required_files=(
  "${OFFLINE_DATA_DIR}/ch_dict.dat"
  "${OFFLINE_DATA_DIR}/ch_index.dat"
  "${OFFLINE_DATA_DIR}/en_dict.dat"
  "${OFFLINE_DATA_DIR}/en_index.dat"
  "${OFFLINE_DATA_DIR}/pages.dat"
  "${OFFLINE_DATA_DIR}/offsets.dat"
  "${OFFLINE_DATA_DIR}/inverted_index.dat"
  "${STOPWORDS_FILE}"
)

for file in "${required_files[@]}"; do
  if [[ ! -f "${file}" ]]; then
    echo "missing required file: ${file}" >&2
    exit 1
  fi
done

mkdir -p "${ONLINE_DATA_DIR}"

copy_if_needed "${OFFLINE_DATA_DIR}/ch_dict.dat" "${ONLINE_DATA_DIR}/ch_dict.dat"
copy_if_needed "${OFFLINE_DATA_DIR}/ch_index.dat" "${ONLINE_DATA_DIR}/ch_index.dat"
copy_if_needed "${OFFLINE_DATA_DIR}/en_dict.dat" "${ONLINE_DATA_DIR}/en_dict.dat"
copy_if_needed "${OFFLINE_DATA_DIR}/en_index.dat" "${ONLINE_DATA_DIR}/en_index.dat"
copy_if_needed "${OFFLINE_DATA_DIR}/pages.dat" "${ONLINE_DATA_DIR}/pages.dat"
copy_if_needed "${OFFLINE_DATA_DIR}/offsets.dat" "${ONLINE_DATA_DIR}/offsets.dat"
copy_if_needed "${OFFLINE_DATA_DIR}/inverted_index.dat" "${ONLINE_DATA_DIR}/inverted_index.dat"
copy_if_needed "${STOPWORDS_FILE}" "${ONLINE_DATA_DIR}/stopwords_cn.txt"

rm -f "${ONLINE_DATA_DIR}/dict.dat" \
      "${ONLINE_DATA_DIR}/index.dat" \
      "${ONLINE_DATA_DIR}/pagelib.dat" \
      "${ONLINE_DATA_DIR}/offset.dat" \
      "${ONLINE_DATA_DIR}/stop_words.txt"

if [[ -d "${ONLINE_DIR}/build" ]]; then
  mkdir -p "${ONLINE_BUILD_DATA_DIR}"
  copy_if_needed "${ONLINE_DATA_DIR}/ch_dict.dat" "${ONLINE_BUILD_DATA_DIR}/ch_dict.dat"
  copy_if_needed "${ONLINE_DATA_DIR}/ch_index.dat" "${ONLINE_BUILD_DATA_DIR}/ch_index.dat"
  copy_if_needed "${ONLINE_DATA_DIR}/en_dict.dat" "${ONLINE_BUILD_DATA_DIR}/en_dict.dat"
  copy_if_needed "${ONLINE_DATA_DIR}/en_index.dat" "${ONLINE_BUILD_DATA_DIR}/en_index.dat"
  copy_if_needed "${ONLINE_DATA_DIR}/pages.dat" "${ONLINE_BUILD_DATA_DIR}/pages.dat"
  copy_if_needed "${ONLINE_DATA_DIR}/offsets.dat" "${ONLINE_BUILD_DATA_DIR}/offsets.dat"
  copy_if_needed "${ONLINE_DATA_DIR}/inverted_index.dat" "${ONLINE_BUILD_DATA_DIR}/inverted_index.dat"
  copy_if_needed "${ONLINE_DATA_DIR}/stopwords_cn.txt" "${ONLINE_BUILD_DATA_DIR}/stopwords_cn.txt"

  rm -f "${ONLINE_BUILD_DATA_DIR}/dict.dat" \
        "${ONLINE_BUILD_DATA_DIR}/index.dat" \
        "${ONLINE_BUILD_DATA_DIR}/pagelib.dat" \
        "${ONLINE_BUILD_DATA_DIR}/offset.dat" \
        "${ONLINE_BUILD_DATA_DIR}/stop_words.txt"
fi

if [[ -d "${PROJECT_ROOT}/build" ]]; then
  mkdir -p "${ROOT_BUILD_DATA_DIR}"
  copy_if_needed "${OFFLINE_DATA_DIR}/ch_dict.dat" "${ROOT_BUILD_DATA_DIR}/ch_dict.dat"
  copy_if_needed "${OFFLINE_DATA_DIR}/ch_index.dat" "${ROOT_BUILD_DATA_DIR}/ch_index.dat"
  copy_if_needed "${OFFLINE_DATA_DIR}/en_dict.dat" "${ROOT_BUILD_DATA_DIR}/en_dict.dat"
  copy_if_needed "${OFFLINE_DATA_DIR}/en_index.dat" "${ROOT_BUILD_DATA_DIR}/en_index.dat"
  copy_if_needed "${OFFLINE_DATA_DIR}/pages.dat" "${ROOT_BUILD_DATA_DIR}/pages.dat"
  copy_if_needed "${OFFLINE_DATA_DIR}/offsets.dat" "${ROOT_BUILD_DATA_DIR}/offsets.dat"
  copy_if_needed "${OFFLINE_DATA_DIR}/inverted_index.dat" "${ROOT_BUILD_DATA_DIR}/inverted_index.dat"
  copy_if_needed "${STOPWORDS_FILE}" "${ROOT_BUILD_DATA_DIR}/stopwords_cn.txt"
fi

echo "prepared online data in: ${ONLINE_DATA_DIR}"
if [[ -d "${ONLINE_DIR}/build" ]]; then
  echo "synced build data in: ${ONLINE_BUILD_DATA_DIR}"
fi
if [[ -d "${PROJECT_ROOT}/build" ]]; then
  echo "synced root build data in: ${ROOT_BUILD_DATA_DIR}"
fi
