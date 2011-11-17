#!/bin/bash

repository=svn+ssh://caagt.ugent.be/public/svn-cage/cage

if [ $# -eq 1 ]
then

    release=$1
	
	svn ls ${repository}/tags/${release} > /dev/null 2> /dev/null
	if [ $? -eq 0 ]
	then
	    echo "That release already exists."
		exit
	fi
	
    svn copy ${repository}/trunk ${repository}/tags/${release} -m "Tagging version ${release}"
	
elif [ $# -eq 2 ]
then

    revision=$1
    release=$2
	
	svn ls ${repository}/tags/${release} > /dev/null 2> /dev/null
	if [ $? -eq 0 ]
	then
	    echo "That release already exists."
		exit
	fi

    svn copy -r $revision ${repository}/trunk ${repository}/tags/${release} -m "Tagging version ${release}"
	
fi