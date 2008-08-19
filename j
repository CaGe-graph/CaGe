
if [ $# -eq 0 ]
  then echo "usage: $0 [commandline]" >&2
       echo "  java environment variables will be set." >&2
       exit 2
fi

jdk=/vol/local/Software/java/jdk118_v3
subdir=linux
stdextracp=:/vol/local/Software/java/Swing-1.1/swing.jar:/vol/local/Software/java/collections1.1/lib/collections.jar



CLASSPATH=${CLASSPATH:-.:$jdk/classes:$jdk/lib/classes.zip}${stdextracp}
if extracp=` expr "$1" : '^-cp\(.*\)$' 2>&- `
  then 
       shift
       CLASSPATH=$extracp:$CLASSPATH
fi

export CLASSPATH PATH CPPFLAGS

PATH=$jdk/bin:$PATH \
CPPFLAGS="$CPPFLAGS -I$jdk/include -I$jdk/include/$subdir" \
LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH \
exec sh -c "exec $*"

