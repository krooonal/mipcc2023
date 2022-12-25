#include <assert.h>

#include "branch_cumpscost.h"

#include "scip/branch_relpscost.h"

#define BRANCHRULE_NAME "Cumpscost"
#define BRANCHRULE_DESC "branching rule for cumulative pseudocost updates"
#define BRANCHRULE_PRIORITY 0
#define BRANCHRULE_MAXDEPTH -1
#define BRANCHRULE_MAXBOUNDDIST 1.0

/*
 * Data structures
 */

/* TODO: fill in the necessary branching rule data */

/** branching rule data */
struct SCIP_BranchruleData
{
};

/*
 * Local methods
 */

/* put your local methods here, and declare them static */

/*
 * Callback methods of branching rule
 */

/* TODO: Implement all necessary branching rule methods. The methods with an #if 0 ... #else #define ... are optional */

/** copy method for branchrule plugins (called when SCIP copies plugins) */
#if 0
static
SCIP_DECL_BRANCHCOPY(branchCopyCumpscost)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of Cumpscost branching rule not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define branchCopyCumpscost NULL
#endif

/** destructor of branching rule to free user data (called when SCIP is exiting) */
#if 0
static
SCIP_DECL_BRANCHFREE(branchFreeCumpscost)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of Cumpscost branching rule not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define branchFreeCumpscost NULL
#endif

/** initialization method of branching rule (called after problem was transformed) */
#if 0
static
SCIP_DECL_BRANCHINIT(branchInitCumpscost)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of Cumpscost branching rule not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define branchInitCumpscost NULL
#endif

/** deinitialization method of branching rule (called before transformed problem is freed) */
#if 0
static
SCIP_DECL_BRANCHEXIT(branchExitCumpscost)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of Cumpscost branching rule not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define branchExitCumpscost NULL
#endif

/** solving process initialization method of branching rule (called when branch and bound process is about to begin) */
#if 0
static
SCIP_DECL_BRANCHINITSOL(branchInitsolCumpscost)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of Cumpscost branching rule not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define branchInitsolCumpscost NULL
#endif

/** solving process deinitialization method of branching rule (called before branch and bound process data is freed) */
#if 0
static
SCIP_DECL_BRANCHEXITSOL(branchExitsolCumpscost)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of Cumpscost branching rule not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define branchExitsolCumpscost NULL
#endif

/** branching execution method for fractional LP solutions */
#if 1
static SCIP_DECL_BRANCHEXECLP(branchExeclpCumpscost)
{ /*lint --e{715}*/
    SCIP_VAR **tmplpcands;
    SCIP_VAR **lpcands;
    SCIP_Real *tmplpcandssol;
    SCIP_Real *lpcandssol;
    SCIP_Real *tmplpcandsfrac;
    SCIP_Real *lpcandsfrac;
    int nlpcands;

    assert(branchrule != NULL);
    assert(strcmp(SCIPbranchruleGetName(branchrule), BRANCHRULE_NAME) == 0);
    assert(scip != NULL);
    assert(result != NULL);

    SCIPdebugMsg(scip, "Execlp method of Cumpscost branching in node %llu\n",
                 SCIPnodeGetNumber(SCIPgetCurrentNode(scip)));

    if (SCIPgetLPSolstat(scip) != SCIP_LPSOLSTAT_OPTIMAL)
    {
        *result = SCIP_DIDNOTRUN;
        SCIPdebugMsg(scip, "Could not apply Cumpscost branching, as the current LP was not solved to optimality.\n");

        return SCIP_OKAY;
    }

    /* get branching candidates */
    SCIP_CALL(SCIPgetLPBranchCands(scip, &tmplpcands, &tmplpcandssol, &tmplpcandsfrac, NULL, &nlpcands, NULL));
    assert(nlpcands > 0);

    /* copy LP banching candidates and solution values, because they will be updated w.r.t. the strong branching LP
     * solution
     */
    SCIP_CALL(SCIPduplicateBufferArray(scip, &lpcands, tmplpcands, nlpcands));
    SCIP_CALL(SCIPduplicateBufferArray(scip, &lpcandssol, tmplpcandssol, nlpcands));
    SCIP_CALL(SCIPduplicateBufferArray(scip, &lpcandsfrac, tmplpcandsfrac, nlpcands));

    /* execute branching rule */
    SCIP_BRANCHRULE *relbranchrule;
    relbranchrule = SCIPfindBranchrule(scip, "relpscost");
    // SCIPexecRelpscostBranching(SCIP *scip, SCIP_VAR **branchcands, double *branchcandssol,
    // double *branchcandsfrac, int nbranchcands, unsigned int executebranching, SCIP_RESULT *result)
    SCIPexecRelpscostBranching(scip, lpcands, lpcandssol, lpcandsfrac, nlpcands, TRUE, result);
    // SCIP_CALL(execRelpscost(scip, branchrule, allowaddcons, lpcands,
    // lpcandssol, lpcandsfrac, nlpcands, TRUE, result));

    SCIPfreeBufferArray(scip, &lpcandsfrac);
    SCIPfreeBufferArray(scip, &lpcandssol);
    SCIPfreeBufferArray(scip, &lpcands);

    return SCIP_OKAY;
}
#else
#define branchExeclpCumpscost NULL
#endif

/** branching execution method for external candidates */
#if 0
static
SCIP_DECL_BRANCHEXECEXT(branchExecextCumpscost)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of Cumpscost branching rule not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define branchExecextCumpscost NULL
#endif

/** branching execution method for not completely fixed pseudo solutions */
#if 0
static
SCIP_DECL_BRANCHEXECPS(branchExecpsCumpscost)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of Cumpscost branching rule not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define branchExecpsCumpscost NULL
#endif

/*
 * branching rule specific interface methods
 */

/** creates the Cumpscost branching rule and includes it in SCIP */
SCIP_RETCODE SCIPincludeBranchruleCumpscost(
    SCIP *scip /**< SCIP data structure */
)
{
    SCIP_BRANCHRULEDATA *branchruledata;
    SCIP_BRANCHRULE *branchrule;

    /* create Cumpscost branching rule data */
    branchruledata = NULL;
    /* TODO: (optional) create branching rule specific data here */

    branchrule = NULL;

    /* include branching rule */

    /* use SCIPincludeBranchruleBasic() plus setter functions if you want to set callbacks one-by-one and your code should
     * compile independent of new callbacks being added in future SCIP versions
     */
    SCIP_CALL(SCIPincludeBranchruleBasic(scip, &branchrule, BRANCHRULE_NAME, BRANCHRULE_DESC, BRANCHRULE_PRIORITY,
                                         BRANCHRULE_MAXDEPTH, BRANCHRULE_MAXBOUNDDIST, branchruledata));

    assert(branchrule != NULL);

    /* set non fundamental callbacks via setter functions */
    SCIP_CALL(SCIPsetBranchruleCopy(scip, branchrule, branchCopyCumpscost));
    SCIP_CALL(SCIPsetBranchruleFree(scip, branchrule, branchFreeCumpscost));
    SCIP_CALL(SCIPsetBranchruleInit(scip, branchrule, branchInitCumpscost));
    SCIP_CALL(SCIPsetBranchruleExit(scip, branchrule, branchExitCumpscost));
    SCIP_CALL(SCIPsetBranchruleInitsol(scip, branchrule, branchInitsolCumpscost));
    SCIP_CALL(SCIPsetBranchruleExitsol(scip, branchrule, branchExitsolCumpscost));
    SCIP_CALL(SCIPsetBranchruleExecLp(scip, branchrule, branchExeclpCumpscost));
    SCIP_CALL(SCIPsetBranchruleExecExt(scip, branchrule, branchExecextCumpscost));
    SCIP_CALL(SCIPsetBranchruleExecPs(scip, branchrule, branchExecpsCumpscost));

    /* add Cumpscost branching rule parameters */
    /* TODO: (optional) add branching rule specific parameters with SCIPaddTypeParam() here */

    return SCIP_OKAY;
}