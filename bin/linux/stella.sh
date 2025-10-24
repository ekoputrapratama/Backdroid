#!/bin/bash
appname=`basename $0 | sed s/.sh$//g`
dirname=`dirname $0`
tmp="${dirname#?}"

if [ "${dirname%$tmp}" != "/" ]; then
    dirname=$PWD/$dirname
fi

export LD_LIBRARY_PATH=$dirname:$dirname/lib
export PATH=$dirname/bin:$PATH
#export QT_DEBUG_PLUGINS=1

chmod +x $appname
$dirname/$appname "$@"
