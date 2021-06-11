rm -rf /tmp/fs
mkdir /tmp/fs
./fuselab -s -f -o nonempty /tmp/fs
