#! /bin/sh


# Try to find which directory this script is in -
# save the result in "CaGe_InstallDir"

parent ()
{
  if parent="` expr \"$1\" : '^\(.*[^/]\)//*[^/][^/]*/*$' 2>&- `"
    then echo "$parent"
  elif expr "$1" : '^/' 2>&- >/dev/null
    then echo "/"
    else echo "."
  fi
}

search_directories ()
{
  dirlist="$1"
  dirsep="$2"
  cmd="$3"
  while dir="` expr \"$dirlist\" : \"^\([^$dirsep]*\)\" 2>&- `"
  do
     test -z "$dir" && dir=.
     if [ -L "$dir/$cmd" ] 2>&-
       then :
     elif [ -x "$dir/$cmd" ]
       then echo "$dir"
            return 0
     fi
     dirlist="` expr \"$dirlist\" : \"^[^$dirsep]*$dirsep\(.*\)$\" 2>&- `"
  done
  return 1
}

cmd="$0"
if expr \"$cmd\" : '.*/' >/dev/null
  then CaGe_InstallDir="` parent \"$cmd\" `"
  else CaGe_InstallDir="` search_directories \"$PATH\" ':' \"$cmd\" `"
fi

if [ -z "$CaGe_InstallDir" ]
  then echo "CaGe: warning - install directory not found, defaulting to current directory" >&2
  else # convert to an absolute path if possible, add trailing slash
       CaGe_InstallDir_NoSlash="` ( cd \"$CaGe_InstallDir\" && pwd ) 2>&- || echo \"$CaGe_InstallDir\" `"
       CaGe_InstallDir="` expr \"$CaGe_InstallDir\" : '\(.*[^/]\)/*$' `/"
fi
# the Java application will take care of an empty CaGe_InstallDir itself
# (it would actually translate to an absolute path as well)


# it has been reported that printf in the external generators may produce
# number formats different to those expected by scanf in the native libraries
# linked to the Java process.  With this value for LANG, we hope to avoid
# inconsistencies.

LANG=POSIX; export LANG


# Construct a class path appropriate for the Java version
# (include standard path for versions before 1.2)

version=` cd "${CaGe_InstallDir}."; sh ./java -cp sysinfo.jar util.SysInfo java.version `
javadir="` cd \"${CaGe_InstallDir}.\"; sh ./javadir `"
if expr $version : '^1-[01]-' 2>&- >&-
  then cp1="$javadir/lib/classes.zip:$javadir/classes:${CaGe_InstallDir}swing.jar:"
  else cp1=""
fi
cp2="${CaGe_InstallDir}collections.jar:${CaGe_InstallDir}CaGe.jar:${CaGe_InstallDir}Jmol.jar:${CaGe_InstallDir}sysinfo.jar:${CaGe_InstallDir}."


# start CaGe

export CaGe_InstallDir
sysname=` cd "${CaGe_InstallDir}."; sh ./java -cp sysinfo.jar util.SysInfo os.name `
if expr $sysname = 'Mac' 2>&- >&-
  then sh "$CaGe_InstallDir"./java \
        -D"CaGe.InstallDir=$CaGe_InstallDir_NoSlash" \
        -classpath "$cp1$cp2" \
        -Xdock:name="CaGe" \
        -Xdock:icon=img/logo.png \
        cage.CaGe
  else sh "$CaGe_InstallDir"./java \
        -D"CaGe.InstallDir=$CaGe_InstallDir_NoSlash" \
        -classpath "$cp1$cp2" \
        cage.CaGe
fi
