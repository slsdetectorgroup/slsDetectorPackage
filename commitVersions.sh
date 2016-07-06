sh updateSvnVersion.sh

cd slsDetectorGui
git commit -a -m "updating versions"
git push origin developer:developer

cd ../slsDetectorSoftware
git commit -a -m "updating versions"
git push origin developer:developer

cd ../slsReceiverSoftware
git commit -a -m "updating versions"
git push origin developer:developer

cd ..
