#!/usr/bin/env sh
find include/viface/ src/ -type f -name \*viface* -exec uncrustify -c uncrustify.cfg --no-backup {} +
find examples -type f -name \*.cpp -exec uncrustify -c uncrustify.cfg --no-backup {} +
