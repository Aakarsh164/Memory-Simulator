#!/bin/sh
set -e
./bin/memsim <<EOF
init memory 1024
set allocator first_fit
malloc 100
malloc 200
free 1
dump memory
stats
exit
EOF
