# Initialization
cd `dirname $0`
SCRIPTDIR=`pwd`
cd -

LD_LIBRARY_PATH=$SCRIPTDIR:$SCRIPTDIR/../bin/:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH

# Run application
cd $SCRIPTDIR/../bin/
./bc-gui
