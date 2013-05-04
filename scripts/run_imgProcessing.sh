##
# run_imgProcessing.sh 
# EC535 TeamLux
# Date: April 26, 2013
# 
# Starts running imgProcessing program repeatedly after seting up the environmental variables
# which imgProcessing uses using the stdlib.h function getenv()
##
export THRESHHOLD=110
export PANMIN=292
export PANMAX=370
export TILTMIN=302
export TILTMAX=364

while true; do
  ~/imgProcessing
done;
