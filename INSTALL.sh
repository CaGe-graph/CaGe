#! /bin/sh

# CaGe install script, version 1.1
# Sebastian Lisken
# requires standard shell with 'expr' command.


### helper functions #################################################

error_exit ()
{
  echo ""
  echo "$1"
  echo ""
  exit 1
}

find_cmd ()
{
  found=""
  for cmd
  do
     t="` type $cmd 2>&- `"
     if found="` expr \"$t\" : \"^$cmd is \(.*\)$\" 2>&- `"
       then break
     fi
  done
  test -n "$found" && echo "$found"
}

find_cmds ()
{
  value=0
  for cmd
  do
     find_cmd "$cmd" >/dev/null || { echo "$cmd"; value=1; }
  done
  return $value
}

parent ()
{
  if parent="` expr \"$1\" : '^\(.*[^/]\)//*[^/][^/]*/*$' 2>&- `"
    then echo "$parent"
  elif expr "$1" : '^/' 2>&- >/dev/null
    then echo "/"
    else echo "."
  fi
}

is_javadir ()
{
  javadir="` parent \"$1\" `"
  [ -x "$javadir/bin/$java_cmd" ] && [ ! -d "$javadir/bin/$java_cmd" ] && [ -r "$javadir/include/jni.h" ]
}

exec 0</dev/tty 1>/dev/tty 2>>/dev/tty


### Welcome message ##################################################

eval clear 2>&- || echo ""
echo ""
echo "C a G e  --  Chemical & abstract Graph environment"
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo "                                           version J1.0  *  install v1.1"
echo ""


### find out how to invoke echo with backslash escapes ###############

if [ -z "` echo '\c' `" ]
  then esc=""
elif [ -z "` echo -e '\c' `" ]
  then esc="-e"
  else error_exit "Can't get 'echo' escapes to work - aborted."
fi

### see if 'read' has an '-e' option (read with completion) ##########

if echo . | eval "read -e test" 2>/dev/null
  then with_completion="-e"
  else with_completion=""
fi


### find a C compiler ################################################

echo ""
echo "*  Looking for a C compiler ..."
if CC="` find_cmd gcc cc `"
  then echo "   Ok, using '$CC'."
  else echo "   None found. Make sure a C compiler ('cc' or 'gcc') is in your path."
       error_exit "-  Installation aborted."
fi
echo ""
export CC


### find other commands that we will need ############################

echo ""
echo "*  Looking for commands: 'unzip', 'make', 'mkdir', 'chmod', 'find' ..."
if missing=` find_cmds unzip make mkdir chmod find `
  then echo "   All found."
  else echo "   Command(s) not found in your path: " $missing
       error_exit "-  Installation aborted."
fi
echo ""


### find the "bin" directory of a Java installation ##################

echo ""
echo "*  Looking for a Java installation ..."

java_cmd="java"
javadirs=0
nl="
"
space="	 "
javadirlist=""

javadirlist_get ()
{
  num="$1"
  expr "$nl$javadirlist" : ".*$nl[ ]*$num:  \([^$nl]*\)"
}

add_to_javadirlist ()
{
  dirlist="$1"
  dirsep="$2"
  get_parents="$3"
  while dir="` expr \"$dirlist\" : \"^\([^$dirsep]*\)\" 2>&- `"
  do
     test "$get_parents" && dir="` parent \"$dir\" `"
     test -z "$dir" && dir=.
     if is_javadir "$dir"
       then javadirs=` expr $javadirs + 1 `
	   javadirlist="$javadirlist $javadirs:  $dir$nl"
	   # javadirlist="$javadirlist` expr substr '   ' '(' length $javadirs ')' 3 `$javadirs:  $dir$nl"
     fi
     dirlist="` expr \"$dirlist\" : \"^[^$dirsep]*$dirsep\(.*\)$\" 2>&- `"
  done
}

prepare_javadirlist_prompt ()
{
  echo ""
  if [ -z "$javadirlist" ]
    then echo "   None found. Enter a directory that contains the '$java_cmd' command."
	 prompt="directory:  "
    else if [ $javadirs -eq 1 ]
	   then directories_seem="directory seems"
		choose_one="Choose it (enter 1)"
	   else directories_seem="directories seem"
		choose_one="Choose one (enter its number)"
	 fi
	 echo "   The following $directories_seem to be part of a Java installation."
	 echo "   $choose_one or enter the path of another such directory."
	 prompt="number or directory:  "
  fi
  echo "   Enter '?' for a full search, '-' to exit."
}


add_to_javadirlist "$PATH" ":"
prepare_javadirlist_prompt

choice=""
while [ -z "$choice" ]
do
   echo ""
   test -n "$javadirlist" && echo "$nl$javadirlist"
   echo ""
   echo $esc "$prompt\c"
   read $with_completion choice
   numeric="` expr \"$choice\" : '^\([1-9][0-9]*\)$' 2>&- `"
   if [ "$choice" = "-" ]
     then error_exit "-  Installation cancelled."
   elif [ "$choice" = "?" ]
     then choice=""
	  echo ""
          echo "   Enter a list of directories to start searching from (space-separated)."
	  echo "   An empty response cancels the search. You may use '/' to start a full"
	  echo "   search. Some good start points are:"
	  echo ""
	  echo "        /usr/lib/jvm        (Linux)"
	  echo "        /System /Libraries  (Mac OS X)"
	  echo ""
	  echo $esc "   start points:  \c"
	  read $with_completion points
	  points="` expr \"$points\" : \"[$space]*\(.*[^$space]\)[$space]*$\" `"
	  test -z "$points" && continue
	  echo ""
	  echo ""
	  additional=""
	  test $javadirs -gt 0 && additional=" additional"
	  echo "   Searching for$additional java directories ..."
	  add_to_javadirlist "` find $points \( -type f -o -type l \) -name "$java_cmd" -perm -ugo=x -print 2>/dev/null `" "$nl" true
	  prepare_javadirlist_prompt
   elif [ "$numeric" ] && [ $choice -le $javadirs ]
     then javadir="` javadirlist_get \"$choice\" `"
	  javadir="` parent \"$javadir\" `"
          nonumber=""
   elif is_javadir "$choice"
     then nonumber=true
     else echo ""
          echo $esc "   Sorry, your answer was incorrect. \c"
	  if [ $javadirs -eq 0 ] || [ -z "$numeric" ]
            then echo "The directory must contain a '$java_cmd'"
		 echo "   command and a \"sibling\" directory called 'include' containing 'jni.h'."
	  fi
          echo "   Try again or enter '?' for a full search, '-' to exit."
          choice=""
   fi
done

javadir="` cd \"$javadir\"; pwd `"
echo ""
echo "   Ok."
echo ""

### found java installation in $javadir (bin/java and include/) ######


### find directory for native libraries ##############################

echo "echo '$javadir'" > javadir
sysname=` sh ./java -cp sysinfo.jar util.SysInfo os.name `
test -z "$sysname" && error_exit "-  Can't determine system name ('java -cp sysinfo.jar util.SysInfo os.name') - aborted."


### find include files for native compilation ########################

include_other_dir=""
other_include="jni_md.h"
if [ ! -r "$javadir/include/$other_include" ]
  then other_found=` echo "$javadir/include"/*/"$other_include" `
       if [ "$other_found" != "$javadir/include/*/$other_include" ]
         then include_other_dir=" -I` expr \"$other_found\" : \"^\([^ ]*\)/$other_include\" 2>&- `"
	 else echo "   Warning: can't find include file '$other_include'."
	      echo "   Making of native libraries might fail. Press Return."
	      read
       fi
fi

echo ""
echo "*  extracting C sources ..."
unzip -q CaGe-C.zip || error_exit "-  'unzip' failure, aborting."
#echo "   Ok."
#echo ""

echo ""
echo "*  Precomputing data in the background ..."
echo ""
(
cd PreCompute &&
make
make compute &
) || error_exit "-  'make' failure, aborting."
echo ""
echo "   Ok."
echo ""

echo ""
echo "*  Making generators and embedders ..."
echo ""
(
cd Generators &&
make
) || error_exit "-  'make' failure, aborting."
echo ""
echo "   Ok."
echo ""

echo ""
echo "*  Making native libraries ..."
echo ""
(
cd Native/src &&
mkdir -p ../$sysname &&
CPPFLAGS="$CPPFLAGS -I$javadir/include$include_other_dir -w" make "$sysname"
) || error_exit "-  'make' failure, aborting."
echo ""
echo "   Ok."
echo ""

chmod u+x cage.sh
echo ""
echo "+  Installation successful - congratulations!"
echo "   CaGe is started by the 'cage.sh' command."
echo "   The file 'CaGe.ini' contains some options and comments."
echo ""

