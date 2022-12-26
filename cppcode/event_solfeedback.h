#include <iostream>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <scip/struct_var.h>
#include <scip/struct_history.h>
#include "scip/history.h"
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <iomanip>
#include <stdio.h>
#include <time.h>

#include "solutions.h"

using namespace std;

#ifndef __SCIP_EVENT_SOLFEEDBACK_H__
#define __SCIP_EVENT_SOLFEEDBACK_H__

#ifdef __cplusplus
extern "C"
{
#endif

   /** creates event handler for solfeedback event */
   SCIP_EXPORT
   SCIP_RETCODE SCIPincludeEventHdlrSolFeedback(
       SCIP *scip /**< SCIP data structure */,
       SolutionPool *solution_pool);

#ifdef __cplusplus
}
#endif

#endif