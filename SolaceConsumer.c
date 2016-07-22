/*
 *  Consumer. See Readme.md
 */

#include "os.h"
#include "solclient/solClient.h"
#include "solclient/solClientMsg.h"
#include "config.h"
#include <time.h>

// messages consumed count
static int msgCount = 0;

// message receive times 
struct timeval *receive_times;

stress_config config;

/*****************************************************************************
 * messageReceiveCallback
 *
 * The message callback is invoked for each Direct message received by
 * the Session. In this sample, the message is printed to the screen.
 *****************************************************************************/
solClient_rxMsgCallback_returnCode_t
messageReceiveCallback ( solClient_opaqueSession_pt opaqueSession_p, solClient_opaqueMsg_pt msg_p, void *user_p )
{
    if(msgCount >= config.warmup_count) {
        gettimeofday(&receive_times[msgCount - config.warmup_count], 0);
    }
    
    msgCount++;

    return SOLCLIENT_CALLBACK_OK;
}

/*****************************************************************************
 * eventCallback
 *
 * The event callback function is mandatory for session creation.
 *****************************************************************************/
void
eventCallback ( solClient_opaqueSession_pt opaqueSession_p,
                solClient_session_eventCallbackInfo_pt eventInfo_p, void *user_p )
{
}

/*****************************************************************************
 * main
 * 
 * The entry point to the application.
 *****************************************************************************/
int
main ( int argc, char *argv[] )
{
    if ( argc < 2 ) {
        printf ( "\nUsage: SolaceConsumer <config.json>\n" );
        return -1;
    }

    parse_config(&config, argv[1]);

    /* Context */
    solClient_opaqueContext_pt context_p;
    solClient_context_createFuncInfo_t contextFuncInfo = SOLCLIENT_CONTEXT_CREATEFUNC_INITIALIZER;

    /* Session */
    solClient_opaqueSession_pt session_p;
    solClient_session_createFuncInfo_t sessionFuncInfo = SOLCLIENT_SESSION_CREATEFUNC_INITIALIZER;

    /* Session Properties */
    const char     *sessionProps[20];
    int             propIndex = 0;

    /* solClient needs to be initialized before any other API calls. */
    solClient_initialize ( SOLCLIENT_LOG_DEFAULT_FILTER, NULL );
    solClient_context_create ( SOLCLIENT_CONTEXT_PROPS_DEFAULT_WITH_CREATE_THREAD,
                               &context_p, &contextFuncInfo, sizeof ( contextFuncInfo ) );

    /*************************************************************************
     * Create and connect a Session
     *************************************************************************/

    /* Configure the Session function information. */
    sessionFuncInfo.rxMsgInfo.callback_p = messageReceiveCallback;
    sessionFuncInfo.rxMsgInfo.user_p = NULL;
    sessionFuncInfo.eventInfo.callback_p = eventCallback;
    sessionFuncInfo.eventInfo.user_p = NULL;

    /* Configure the Session properties. */
    propIndex = 0;

    printf("\nSession properties: \n");
    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_HOST;
    sessionProps[propIndex++] = config.hostname;
    printf("%s: %s\n", SOLCLIENT_SESSION_PROP_HOST, config.hostname);

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_VPN_NAME;
    sessionProps[propIndex++] = config.vpn_name;
    printf("%s: %s\n", SOLCLIENT_SESSION_PROP_VPN_NAME, config.vpn_name);

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_USERNAME;
    sessionProps[propIndex++] = config.username;
    printf("%s: %s\n", SOLCLIENT_SESSION_PROP_USERNAME, config.username);

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_PASSWORD;
    sessionProps[propIndex++] = config.password;
    printf("%s: %s\n", SOLCLIENT_SESSION_PROP_PASSWORD, config.password);

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_CONNECT_BLOCKING;
    sessionProps[propIndex++] = (config.is_blocking == 1) ? SOLCLIENT_PROP_ENABLE_VAL : SOLCLIENT_PROP_DISABLE_VAL;
    printf("%s: %d\n", SOLCLIENT_SESSION_PROP_CONNECT_BLOCKING, config.is_blocking);

    sessionProps[propIndex] = NULL;

    /* Create the Session. */
    solClient_session_create ( ( char ** ) sessionProps,
                               context_p,
                               &session_p, &sessionFuncInfo, sizeof ( sessionFuncInfo ) );

    if(solClient_session_connect ( session_p ) != SOLCLIENT_FAIL) {
        printf ( "Connected.\n" );
    }
    else {
        printf("not succceded connecting to solclient...");
        solClient_errorInfo_pt errorInfo2 = solClient_getLastErrorInfo();
        printf("%s", errorInfo2->errorStr);
        solClient_resetLastErrorInfo();
    }

    receive_times = malloc(sizeof(struct timeval) * (config.run_count));

    // subscribe
    solClient_session_topicSubscribeExt ( session_p,
                                          SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM,
                                          config.topic );

    printf ( "Waiting for %d message(s)......\n", config.run_count );
    fflush ( stdout );

    while ( msgCount < config.run_count + config.warmup_count );

    printf ( "Exiting.\n" );

    // unsubscribe
    solClient_session_topicUnsubscribeExt ( 
        session_p, SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM, config.topic );

    solClient_cleanup (  );

    /* analysis on the data */
    print_analysis("Consumer", "receives", receive_times, config.run_count, config.threshold);

    // cleanup
    free(receive_times);

    return 0;
}
