#! /bin/bash

# above is tellong os  that who runs this script is bush
umount /dev/nbd0 

./a.out

mkfs.ext4 /dev/nbd0
mount  /dev/nbd0 ./test_mounted_files
