/*!*****************************************************************************
\file pl_mpeg.cpp
\brief Implementation file for pl_mpeg single-header MPEG-1 decoder.
       This file defines PL_MPEG_IMPLEMENTATION exactly once to instantiate
       all function definitions from the header-only library.
*******************************************************************************/

// pl_mpeg requires standard C library headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define implementation BEFORE including the header
#define PL_MPEG_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable: 4244 4267 4305 4996)
#include "pl_mpeg.h"
#pragma warning(pop)

