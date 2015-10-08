#!/bin/bash
# copy reverse engineering filter
sudo cp -f bin/rastertokpsl-re /Library/Printers/Kyocera/kpsl/rastertokpsl.app/Contents/MacOS
# install PPD
sudo cp -f Kyocera_FS-1020MFPGDI_RE.ppd /Library/Printers/PPDs/Contents/Resources
