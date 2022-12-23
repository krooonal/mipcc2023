
#include "event_solfeedback.h"

#define EVENTHDLR_NAME "Solution Feedback"
#define EVENTHDLR_DESC "event handler for solution feedback event"

/*
 * Data structures
 */

/* TODO: fill in the necessary event handler data */

/** event handler data */
struct SCIP_EventhdlrData
{
    bool solution_found = false;
    SolutionPool *solution_pool = NULL;
};

/*
 * Local methods
 */

/* put your local methods here, and declare them static */

/*
 * Callback methods of event handler
 */

/** copy method for event handler plugins (called when SCIP copies plugins) */
#if 0
static
SCIP_DECL_EVENTCOPY(eventCopySolFeedback)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of SolFeedback dialog not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define eventCopySolFeedback NULL
#endif

/** destructor of event handler to free user data (called when SCIP is exiting) */
#if 0
static
SCIP_DECL_EVENTFREE(eventFreeSolFeedback)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of SolFeedback event handler not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define eventFreeSolFeedback NULL
#endif

/** initialization method of event handler (called after problem was transformed) */
#if 1
static SCIP_DECL_EVENTINIT(eventInitSolFeedback)
{ /*lint --e{715}*/
    assert(scip != NULL);
    assert(eventhdlr != NULL);
    assert(strcmp(SCIPeventhdlrGetName(eventhdlr), EVENTHDLR_NAME) == 0);

    /* notify SCIP that your event handler wants to react on the event type presolve round finished. */
    SCIP_CALL(SCIPcatchEvent(scip, SCIP_EVENTTYPE_PRESOLVEROUND, eventhdlr, NULL, NULL));

    int n_solutions = SCIPgetNSols(scip);
    if (n_solutions > 0)
    {
        // cout << "Found " << n_solutions << " solutions by now\n";
        return SCIP_OKAY;
    }
    cout << "Transformed and no solutions found yet!\n";
    SCIP_EVENTHDLRDATA *eventhdlrdata = SCIPeventhdlrGetData(eventhdlr);
    SolutionPool *solution_pool = eventhdlrdata->solution_pool;
    int num_pool_solutions = solution_pool->GetNumSolutions();
    // cout << "Solution pool has " << num_pool_solutions << " solutions\n";

    if (num_pool_solutions > 0)
    {
        for (int i = 0; i < 5; ++i)
        {
            SCIP_CALL(solution_pool->AddNextSolutionToModel(scip));
        }
    }

    return SCIP_OKAY;
}
#else
#define eventInitSolFeedback NULL
#endif

/** deinitialization method of event handler (called before transformed problem is freed) */
#if 1
static SCIP_DECL_EVENTEXIT(eventExitSolFeedback)
{ /*lint --e{715}*/
    assert(scip != NULL);
    assert(eventhdlr != NULL);
    assert(strcmp(SCIPeventhdlrGetName(eventhdlr), EVENTHDLR_NAME) == 0);

    /* notify SCIP that your event handler wants to drop the event type presolve round finished. */
    SCIP_CALL(SCIPdropEvent(scip, SCIP_EVENTTYPE_PRESOLVEROUND, eventhdlr, NULL, -1));

    return SCIP_OKAY;
}
#else
#define eventExitSolFeedback NULL
#endif

/** solving process initialization method of event handler (called when branch and bound process is about to begin) */
#if 0
static
SCIP_DECL_EVENTINITSOL(eventInitsolSolFeedback)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of SolFeedback event handler not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define eventInitsolSolFeedback NULL
#endif

/** solving process deinitialization method of event handler (called before branch and bound process data is freed) */
#if 0
static
SCIP_DECL_EVENTEXITSOL(eventExitsolSolFeedback)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of SolFeedback event handler not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define eventExitsolSolFeedback NULL
#endif

/** frees specific event data */
#if 0
static
SCIP_DECL_EVENTDELETE(eventDeleteSolFeedback)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of SolFeedback event handler not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define eventDeleteSolFeedback NULL
#endif

/** execution method of event handler */
static SCIP_DECL_EVENTEXEC(eventExecSolFeedback)
{ /*lint --e{715}*/
    assert(scip != NULL);
    assert(eventhdlr != NULL);
    // cout << "My event handler was called.\n";

    int n_solutions = SCIPgetNSols(scip);
    if (n_solutions > 0)
    {
        // cout << "Found " << n_solutions << " solutions by now\n";
        return SCIP_OKAY;
    }
    cout << "No solutions found yet!\n";
    SCIP_EVENTHDLRDATA *eventhdlrdata = SCIPeventhdlrGetData(eventhdlr);
    SolutionPool *solution_pool = eventhdlrdata->solution_pool;
    int num_pool_solutions = solution_pool->GetNumSolutions();
    // cout << "Solution pool has " << num_pool_solutions << " solutions\n";

    if (num_pool_solutions > 0)
    {
        for (int i = 0; i < 5; ++i)
        {
            SCIP_CALL(solution_pool->AddNextSolutionToModel(scip));
        }
    }

    return SCIP_OKAY;
}

/** creates event handler for SolFeedback event */
SCIP_RETCODE SCIPincludeEventHdlrSolFeedback(
    SCIP *scip /**< SCIP data structure */,
    SolutionPool *solution_pool)
{
    SCIP_EVENTHDLRDATA *eventhdlrdata;
    SCIP_EVENTHDLR *eventhdlr;

    /* create SolFeedback event handler data */
    eventhdlrdata = NULL;
    /* TODO: (optional) create event handler specific data here */
    SCIP_CALL(SCIPallocBlockMemory(scip, &eventhdlrdata));
    eventhdlrdata->solution_pool = solution_pool;

    eventhdlr = NULL;

    /* include event handler into SCIP */
    /* use SCIPincludeEventhdlrBasic() plus setter functions if you want to set
     * callbacks one-by-one and your code should
     * compile independent of new callbacks being added in future SCIP versions
     */
    SCIP_CALL(SCIPincludeEventhdlrBasic(scip, &eventhdlr, EVENTHDLR_NAME, EVENTHDLR_DESC,
                                        eventExecSolFeedback, eventhdlrdata));
    assert(eventhdlr != NULL);

    /* set non fundamental callbacks via setter functions */
    SCIP_CALL(SCIPsetEventhdlrCopy(scip, eventhdlr, eventCopySolFeedback));
    SCIP_CALL(SCIPsetEventhdlrFree(scip, eventhdlr, eventFreeSolFeedback));
    SCIP_CALL(SCIPsetEventhdlrInit(scip, eventhdlr, eventInitSolFeedback));
    SCIP_CALL(SCIPsetEventhdlrExit(scip, eventhdlr, eventExitSolFeedback));
    SCIP_CALL(SCIPsetEventhdlrInitsol(scip, eventhdlr, eventInitsolSolFeedback));
    SCIP_CALL(SCIPsetEventhdlrExitsol(scip, eventhdlr, eventExitsolSolFeedback));
    SCIP_CALL(SCIPsetEventhdlrDelete(scip, eventhdlr, eventDeleteSolFeedback));

    /* add SolFeedback event handler parameters */
    /* TODO: (optional) add event handler specific parameters with SCIPaddTypeParam() here */

    return SCIP_OKAY;
}