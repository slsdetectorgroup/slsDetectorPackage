mv bin/eigerDetectorServer bin/$2
cp bin/$2 /tftpboot
git rm -f bin/$1
git add bin/$2
