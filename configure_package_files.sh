#! /bin/bash


# - - - - - - - - - - - - - - - - - - - - - - - - - -  - - - - - - - - - - - - # 
function get_maintainer_name_and_email(){
  # Extract the full user name from the passwd file
  FULL_USER_NAME=`getent passwd ${USER} | cut -d ':' -f 5 | cut -d ',' -f 1`

  #set the variables name and email to FULL_USER_NAME and USER@HOST if they
  #are not set
  : ${NAME:=${FULL_USER_NAME}}
  : ${EMAIL:=$USER"@"$HOST}
}
# - - - - - - - - - - - - - - - - - - - - - - - - - -  - - - - - - - - - - - - #


#drop out if an error occurs
set -e

#check that the input parameter is set
#arg1 = TAG_VERSION
if [ $# -ne 1 ] ; then
    echo Wrong number of parameters. Run \'make debian_package\' instead of using this scipt directly.
    exit -1
fi

TAG_VERSION=$1

# - create debian_from_template directory
test -d ./debian_from_template || mkdir debian_from_template

#
# Fill in the url to the tag in the svn repo , remove comments and copy the
# template version into debian_from_template/copyright
cat packaging_related_templates/copyright_template | sed -e "{/#/d}" -e \
"{s|@FW_PROGRAMMER_VERSION@|${TAG_VERSION}|}" > debian_from_template/copyright

#
# find the maintainer name and email, remove comments, fill in control_template
# and copy the filled version to debian_from_template/control
get_maintainer_name_and_email
cat packaging_related_templates/control_template | sed -e "{/#/d}" -e \
"{s|__USER_NAME__|${NAME}|}" -e "s|__EMAIL__|${EMAIL}|" > \
debian_from_template/control


#
# Copy rest of the files needed by th epackaging tools
cp packaging_related_templates/compat \
packaging_related_templates/install \
packaging_related_templates/rules \
debian_from_template