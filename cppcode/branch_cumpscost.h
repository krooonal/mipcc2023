#ifndef __SCIP_BRANCH_CUMPSCOST_H__
#define __SCIP_BRANCH_CUMPSCOST_H__

#include "scip/scip.h"
#include "var_history.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /** creates the Cumpscost branching rule and includes it in SCIP
     *
     *  @ingroup BranchingRuleIncludes
     */
    SCIP_EXPORT
    SCIP_RETCODE SCIPincludeBranchruleCumpscost(
        SCIP *scip,
        VarHistories *var_histories,
        double cost_update_factor);

    /**@addtogroup BRANCHINGRULES
     *
     * @{
     */

    /* TODO place other public methods in this group to facilitate navigation through the documentation */

    /** @} */

#ifdef __cplusplus
}
#endif

#endif