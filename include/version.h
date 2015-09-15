/* 
 * File:   version.h
 * Author: pperek
 *
 * Created on 15 wrzesień 2015, 12:36
 */

#ifndef VERSION_H
#define	VERSION_H

#define VERSION_MAJ                 3
#define VERSION_MIN                 1

#define TO_STR2(x) #x
#define TO_STR(x) TO_STR2(x)
#define VERSION (TO_STR(VERSION_MAJ) "." TO_STR(VERSION_MIN) "." TO_STR(SVN_REV))

#endif	/* VERSION_H */

