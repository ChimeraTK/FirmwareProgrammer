#!/bin/bash

# This script has been borrowed from the matlabtools project:
# https://svnsrv.desy.de/desy/mtca4u_applications/matlab_tools/trunk/

#
# This script is only intended for internal use at DESY. It is created because
# automated generation with CMake is more convenient and easier to maintain
# than reverse-engeneering the package names using 'ls', 'grep' etc.
# In addition it can serve as a template for other debian installations.
#
# Trying to execute this, even at DESY, will probably fail due to a lack of
# priviliges. This script is intendet to be used by MTCA4U maintainers at DESY
# only.
#
# Run 'make debian_package" before you execute this script.

DEBIAN_CODENAME=`lsb_release -c | sed "{s/Codename:\s*//}"`
PACKAGE_FILES_WILDCARDS="mtca4u-fw-programmer*.deb mtca4u-fw-programmer*.changes"

cd debian_package

# Step 1: Remove an older version of the package
# -- from the nfs archive
(cd /home/debian/${DEBIAN_CODENAME}/stable; mv ${PACKAGE_FILES_WILDCARDS} ../old)
# -- from the actual repository
ssh doocspkgs sudo -H reprepro --waitforlock 2 -Vb \
    /export/reprepro/intern/doocs remove ${DEBIAN_CODENAME} mtca4u-fw-programmer

# Step 2: Set the priviledges and the priviledge mask for new files to rw-rw-r--
chmod 664 ${PACKAGE_FILES_WILDCARDS}
umask 002
# -- and copy the files to the nfs archive, using sg to copy with the flash group as target group
sg flash -c "cp ${PACKAGE_FILES_WILDCARDS} /home/debian/${DEBIAN_CODENAME}/stable"

# Step 3: Install to the repository (only desy intern)
for FILE in *.deb; do
    ssh doocspkgs sudo -H reprepro --waitforlock 2 -Vb \
	/export/reprepro/intern/doocs includedeb ${DEBIAN_CODENAME} \
	/home/debian/${DEBIAN_CODENAME}/stable/${FILE}
done

