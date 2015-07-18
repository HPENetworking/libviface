#!/usr/bin/env sh
find include/ -type f -name \*.hpp -exec uncrustify -c uncrustify.cfg --no-backup {} +
find src/ examples/ test/ -type f -name \*.cpp -exec uncrustify -c uncrustify.cfg --no-backup {} +
