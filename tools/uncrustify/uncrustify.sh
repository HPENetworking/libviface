#!/usr/bin/env sh

DIR=$(dirname $0)

# Uncrustify everything if no parameters are given
if [ "$#" -eq 0 ]; then
    find "${DIR}/../../include/" -type f -name \*.hpp -exec uncrustify -c "${DIR}/uncrustify.cfg" --no-backup {} +
    find "${DIR}/../../src/" "${DIR}/../../examples/" "${DIR}/../../test/" -type f -name \*.cpp -exec uncrustify -c "${DIR}/uncrustify.cfg" --no-backup {} +
    exit 0
fi

# Iterate over all parameters and uncrustify them
for file in "$@"; do
    uncrustify --no-backup -c "${DIR}/uncrustify.cfg" "${file}"
done
