/*
* Copyright (C) 2015 University of Nizhni Novgorod
* This file is part of the Globalizer Light library, v 1.0
* 
* common.h - common definitions
* 
* This file may be used under the terms of the GNU Lesser General Public 
* License version 2.1 as published by the Free Software Foundation 
* and appearing in the file LICENSE.LGPL included in the packaging of this 
* file. Please review the following information to ensure the GNU Lesser 
* General Public License version 2.1 requirements will be met: 
* http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
*/ 

#ifndef __COMMON_H__
#define __COMMON_H__

/* ========================================================================= *\
**  Constants                                                                **
\* ========================================================================= */

const int MaxNumOfTaskLevels = 5;
/*
 * в настоящий момент решаются только безусловные
 * задачи, поэтому функционал один
 */
const int MaxNumOfFunc = 1;

const int MaxDim = 5;
const int MaxM = 15;
const int MaxL = 10;

const double MaxDouble = 1.7e308;

const int DefaultQueueSize = 1048575; // should be 2^k - 1
const int MaxQueueSize = 67108863; // should be 2^26 - 1
const int DefaultSearchDataSize = 100000;

const int TagChildStartSolve = 101;
const int TagChildSolved  = 102;
const int ChildStopMsg = -101;

/* ========================================================================= *\
**  Types                                                                    **
\* ========================================================================= */

class TProcess;
#ifdef WIN32
typedef void (__cdecl *tIterationHandler)(TProcess *pProcess);
typedef double (__cdecl *tFunction)(const double*);
#else
typedef void (*tIterationHandler)(TProcess *pProcess);
typedef double (*tFunction)(const double*);
#endif

#endif
// - end of file ----------------------------------------------------------------------------------
