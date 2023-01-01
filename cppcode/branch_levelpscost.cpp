#include <assert.h>
#include <iostream>

#include "branch_levelpscost.h"
#include "scip/struct_var.h"
#include "scip/struct_tree.h"

#include "scip/branch_relpscost.h"

#define BRANCHRULE_NAME "Levelpscost"
#define BRANCHRULE_DESC "branching rule for cumulative pseudocost updates"
#define BRANCHRULE_PRIORITY 20000 /*relpscost = 10000*/
#define BRANCHRULE_MAXDEPTH -1
#define BRANCHRULE_MAXBOUNDDIST 1.0

using namespace std;

/*
 * Data structures
 */

/* TODO: fill in the necessary branching rule data */

/** branching rule data */
struct SCIP_BranchruleData
{
   VarHistories *var_histories = NULL;
   // std::map<long long, double> node_lp_values;
   double cost_update_factor = 0.0;
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
SCIP_DECL_BRANCHCOPY(branchCopyLevelpscost)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of Levelpscost branching rule not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define branchCopyLevelpscost NULL
#endif

/** destructor of branching rule to free user data (called when SCIP is exiting) */
#if 1
static SCIP_DECL_BRANCHFREE(branchFreeLevelpscost)
{ /*lint --e{715}*/
   SCIP_BRANCHRULEDATA *branchruledata;
   branchruledata = SCIPbranchruleGetData(branchrule);

   /* free branching rule data */
   SCIPfreeBlockMemory(scip, &branchruledata);
   SCIPbranchruleSetData(branchrule, NULL);

   return SCIP_OKAY;
}
#else
#define branchFreeLevelpscost NULL
#endif

/** initialization method of branching rule (called after problem was transformed) */
#if 0
static
SCIP_DECL_BRANCHINIT(branchInitLevelpscost)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of Levelpscost branching rule not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define branchInitLevelpscost NULL
#endif

/** deinitialization method of branching rule (called before transformed problem is freed) */
#if 0
static
SCIP_DECL_BRANCHEXIT(branchExitLevelpscost)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of Levelpscost branching rule not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define branchExitLevelpscost NULL
#endif

/** solving process initialization method of branching rule (called when branch and bound process is about to begin) */
#if 0
static
SCIP_DECL_BRANCHINITSOL(branchInitsolLevelpscost)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of Levelpscost branching rule not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define branchInitsolLevelpscost NULL
#endif

/** solving process deinitialization method of branching rule (called before branch and bound process data is freed) */
#if 0
static
SCIP_DECL_BRANCHEXITSOL(branchExitsolLevelpscost)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of Levelpscost branching rule not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define branchExitsolLevelpscost NULL
#endif

/** branching execution method for fractional LP solutions */
#if 1
static SCIP_DECL_BRANCHEXECLP(branchExeclpLevelpscost)
{ /*lint --e{715}*/

   assert(branchrule != NULL);
   SCIP_BRANCHRULEDATA *branchruledata;
   branchruledata = SCIPbranchruleGetData(branchrule);
   assert(branchruledata != NULL);
   assert(strcmp(SCIPbranchruleGetName(branchrule), BRANCHRULE_NAME) == 0);
   assert(scip != NULL);
   assert(result != NULL);

   SCIPdebugMsg(scip, "Execlp method of Levelpscost branching in node %llu\n",
                SCIPnodeGetNumber(SCIPgetCurrentNode(scip)));

   if (SCIPgetLPSolstat(scip) != SCIP_LPSOLSTAT_OPTIMAL)
   {
      *result = SCIP_DIDNOTRUN;
      SCIPdebugMsg(scip, "Could not apply Levelpscost branching, as the current LP was not solved to optimality.\n");

      return SCIP_OKAY;
   }

   SCIP_NODE *current_node = SCIPgetCurrentNode(scip);
   int node_depth = SCIPnodeGetDepth(current_node);
   {
      SCIP_VAR **branchvars;
      SCIP_Real *branchbounds;
      SCIP_BOUNDTYPE *boundtypes;
      int nbranchvars;
      int branchvarssize;
      int nnodes;

      // Store current node variable LP values. This can slow us down?
      // Store current node LP objective value.
      long long current_node_num = current_node->number;
      double current_lp_obj = SCIPgetLPObjval(scip);
      // branchruledata->node_lp_values[current_node_num] = current_lp_obj;

      if (node_depth > 0)
      {
         long long parent_node_num = current_node->parent->number;
         // double parent_lp_obj = branchruledata->node_lp_values[parent_node_num];
         double parent_lp_obj = SCIPnodeGetLowerbound(current_node->parent);
         double lp_gain = current_lp_obj - parent_lp_obj;
         // cout << "LP gain: " << lp_gain << endl;
         lp_gain = max(lp_gain, 0.0);

         branchvarssize = node_depth;
         SCIP_NODE *node = current_node;
         /* memory allocation */
         SCIP_CALL(SCIPallocBufferArray(scip, &branchvars, branchvarssize));
         SCIP_CALL(SCIPallocBufferArray(scip, &branchbounds, branchvarssize));
         SCIP_CALL(SCIPallocBufferArray(scip, &boundtypes, branchvarssize));
         int level = 0;
         double current_update_fac = 1;
         bool update_count = true;

         SCIPnodeGetParentBranchings(node, branchvars,
                                     branchbounds, boundtypes,
                                     &nbranchvars, branchvarssize);

         for (int i = 0; i < nbranchvars; ++i)
         {
            // SCIPvarGetLPSol(var);
            SCIP_VAR *branched_var = branchvars[i];
            double lp_val = SCIPvarGetLPSol(branched_var);
            SCIP_VAR **parent_vars = branched_var->parentvars;
            int n_parents = branched_var->nparentvars;
            string bnd = " <= ";
            string var_name = branched_var->name;
            if (boundtypes[i] == SCIP_BOUNDTYPE_LOWER)
            {
               bnd = " >= ";
            }
            if (n_parents > 0)
            {
               var_name = parent_vars[0]->name;
            }

            // cout << "Previous branch: " << var_name
            //      << bnd << branchbounds[i]
            //      << endl;
            int level = node_depth / 10;
            double cost_update = lp_gain * current_update_fac;
            if (boundtypes[i] == SCIP_BOUNDTYPE_LOWER)
            {
               branchruledata->var_histories->UpdateLevelpscost(var_name, level, cost_update, update_count);
            }
            else
            {
               branchruledata->var_histories->UpdateLevelpscostDown(var_name, level, cost_update, update_count);
            }
         }
         level++;
         update_count = false; // Only update the count of last level var.
         current_update_fac *= branchruledata->cost_update_factor;

         /* free all local memory */
         SCIPfreeBufferArray(scip, &boundtypes);
         SCIPfreeBufferArray(scip, &branchbounds);
         SCIPfreeBufferArray(scip, &branchvars);
      }
   }
   bool use_relpscost = false;
   if (rand() % 10 >= 2)
   {
      use_relpscost = true;
      *result = SCIP_DIDNOTRUN;
      return SCIP_OKAY;
   }
   SCIP_VAR **tmplpcands;
   SCIP_VAR **lpcands;
   SCIP_Real *tmplpcandssol;
   SCIP_Real *lpcandssol;
   SCIP_Real *tmplpcandsfrac;
   SCIP_Real *lpcandsfrac;
   int nlpcands;

   /* get branching candidates */
   SCIP_CALL(SCIPgetLPBranchCands(scip, &tmplpcands, &tmplpcandssol,
                                  &tmplpcandsfrac, NULL, &nlpcands, NULL));
   assert(nlpcands > 0);

   /* copy LP banching candidates and solution values, because they
    * will be updated w.r.t. the strong branching LP
    * solution
    */
   SCIP_CALL(SCIPduplicateBufferArray(scip, &lpcands, tmplpcands, nlpcands));
   SCIP_CALL(SCIPduplicateBufferArray(scip, &lpcandssol, tmplpcandssol, nlpcands));
   SCIP_CALL(SCIPduplicateBufferArray(scip, &lpcandsfrac, tmplpcandsfrac, nlpcands));

   /* execute branching rule */
   int level = node_depth / 10;
   int best_lp_candidate_index = -1;
   double best_cost = 0.0;
   for (int i = 0; i < nlpcands; ++i)
   {
      SCIP_VAR *var = lpcands[i];
      SCIP_VAR **parent_vars = var->parentvars;
      int n_parents = var->nparentvars;
      string var_name = var->name;
      if (n_parents > 0)
      {
         var_name = parent_vars[0]->name;
      }
      long long cumpscount = branchruledata->var_histories->GetLevelpscostCount(var_name, level);
      if (cumpscount < 40ll)
         continue;
      double Levelpscost = branchruledata->var_histories->GetLevelpscost(var_name, level);
      if (Levelpscost > best_cost)
      {
         best_lp_candidate_index = i;
         best_cost = Levelpscost;
      }
   }

   if (best_lp_candidate_index == -1)
      use_relpscost = true;

   if (use_relpscost)
   {
      *result = SCIP_DIDNOTRUN;
      /*
      SCIP_BRANCHRULE *relbranchrule;
      relbranchrule = SCIPfindBranchrule(scip, "relpscost");
      // SCIPexecRelpscostBranching(SCIP *scip, SCIP_VAR **branchcands, double *branchcandssol,
      // double *branchcandsfrac, int nbranchcands, unsigned int executebranching, SCIP_RESULT *result)
      SCIPexecRelpscostBranching(scip, lpcands, lpcandssol, lpcandsfrac,
                                 nlpcands, TRUE, result);
                                 */
   }
   else
   {
      // cout << "Branching on Levelpscost rule. Best cost: " << best_cost << endl;
      SCIP_NODE *downchild;
      SCIP_NODE *upchild;
      SCIP_VAR *var = lpcands[best_lp_candidate_index];
      double val = lpcandssol[best_lp_candidate_index];
      SCIPbranchVarVal(scip, var, val, &downchild, NULL, &upchild);
      *result = SCIP_BRANCHED;
   }

   // if (*result == SCIP_BRANCHED)
   // {
   //    // cout << "Branched on current node\n";
   // }

   SCIPfreeBufferArray(scip, &lpcandsfrac);
   SCIPfreeBufferArray(scip, &lpcandssol);
   SCIPfreeBufferArray(scip, &lpcands);
   return SCIP_OKAY;
}
#else
#define branchExeclpLevelpscost NULL
#endif

/** branching execution method for external candidates */
#if 0
static
SCIP_DECL_BRANCHEXECEXT(branchExecextLevelpscost)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of Levelpscost branching rule not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define branchExecextLevelpscost NULL
#endif

/** branching execution method for not completely fixed pseudo solutions */
#if 0
static
SCIP_DECL_BRANCHEXECPS(branchExecpsLevelpscost)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of Levelpscost branching rule not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define branchExecpsLevelpscost NULL
#endif

/*
 * branching rule specific interface methods
 */

/** creates the Levelpscost branching rule and includes it in SCIP */
SCIP_RETCODE SCIPincludeBranchruleLevelpscost(
    SCIP *scip,
    VarHistories *var_histories,
    double cost_update_factor)
{
   SCIP_BRANCHRULEDATA *branchruledata;
   SCIP_BRANCHRULE *branchrule;
   srand(42);

   /* create Levelpscost branching rule data */
   branchruledata = NULL;
   /* TODO: (optional) create branching rule specific data here */
   SCIP_CALL(SCIPallocBlockMemory(scip, &branchruledata));
   branchruledata->var_histories = var_histories;
   branchruledata->cost_update_factor = cost_update_factor;

   branchrule = NULL;

   /* include branching rule */

   /* use SCIPincludeBranchruleBasic() plus setter functions if you want to set
    * callbacks one-by-one and your code should
    * compile independent of new callbacks being added in future SCIP versions
    */
   SCIP_CALL(SCIPincludeBranchruleBasic(scip, &branchrule, BRANCHRULE_NAME,
                                        BRANCHRULE_DESC, BRANCHRULE_PRIORITY,
                                        BRANCHRULE_MAXDEPTH,
                                        BRANCHRULE_MAXBOUNDDIST, branchruledata));

   assert(branchrule != NULL);

   /* set non fundamental callbacks via setter functions */
   SCIP_CALL(SCIPsetBranchruleCopy(scip, branchrule, branchCopyLevelpscost));
   SCIP_CALL(SCIPsetBranchruleFree(scip, branchrule, branchFreeLevelpscost));
   SCIP_CALL(SCIPsetBranchruleInit(scip, branchrule, branchInitLevelpscost));
   SCIP_CALL(SCIPsetBranchruleExit(scip, branchrule, branchExitLevelpscost));
   SCIP_CALL(SCIPsetBranchruleInitsol(scip, branchrule, branchInitsolLevelpscost));
   SCIP_CALL(SCIPsetBranchruleExitsol(scip, branchrule, branchExitsolLevelpscost));
   SCIP_CALL(SCIPsetBranchruleExecLp(scip, branchrule, branchExeclpLevelpscost));
   SCIP_CALL(SCIPsetBranchruleExecExt(scip, branchrule, branchExecextLevelpscost));
   SCIP_CALL(SCIPsetBranchruleExecPs(scip, branchrule, branchExecpsLevelpscost));

   /* add Levelpscost branching rule parameters */
   /* TODO: (optional) add branching rule specific parameters with SCIPaddTypeParam() here */

   return SCIP_OKAY;
}