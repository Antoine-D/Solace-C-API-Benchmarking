/*
 *  Producer. See Readme.md
 */

#include "os.h"
#include <sys/time.h>
#include "solclient/solClient.h"
#include "solclient/solClientMsg.h"
#include "config.h"

/*****************************************************************************
 * messageReceiveCallback
 *
 * The message receive callback function is mandatory for session creation.
 *****************************************************************************/
solClient_rxMsgCallback_returnCode_t
messageReceiveCallback ( solClient_opaqueSession_pt opaqueSession_p, solClient_opaqueMsg_pt msg_p, void *user_p )
{
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
        printf ( "\nUsage: SolacePublisher <config.json>\n" );
        return -1;
    }

    stress_config config;
    parse_config(&config, argv[1]);

    /* Context */
    solClient_opaqueContext_pt context_p;
    solClient_context_createFuncInfo_t contextFuncInfo = SOLCLIENT_CONTEXT_CREATEFUNC_INITIALIZER;

    /* Session */
    solClient_opaqueSession_pt session_p;
    solClient_session_createFuncInfo_t sessionFuncInfo = SOLCLIENT_SESSION_CREATEFUNC_INITIALIZER;

    /* Session Properties */
    const char *sessionProps[20];
    int propIndex = 0;

    /* solClient needs to be initialized before any other API calls. */
    solClient_initialize ( SOLCLIENT_LOG_DEFAULT_FILTER, NULL );
    solClient_context_create ( SOLCLIENT_CONTEXT_PROPS_DEFAULT_WITH_CREATE_THREAD,
                               &context_p, &contextFuncInfo, sizeof ( contextFuncInfo ) );

    /*************************************************************************
     * Create and connect a Session
     *************************************************************************/

    /*
     * Message receive callback function and the Session event function
     * are both mandatory. In this sample, default functions are used.
     */
    sessionFuncInfo.rxMsgInfo.callback_p = messageReceiveCallback;
    sessionFuncInfo.rxMsgInfo.user_p = NULL;
    sessionFuncInfo.eventInfo.callback_p = eventCallback;
    sessionFuncInfo.eventInfo.user_p = NULL;

    
    propIndex = 0;

    printf("\nSession properties: \n");
    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_HOST;
    sessionProps[propIndex++] = config.hostname;
    printf("Setting %s to: %s\n", SOLCLIENT_SESSION_PROP_HOST, config.hostname);

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_VPN_NAME;
    sessionProps[propIndex++] = config.vpn_name;
    printf("Setting %s to: %s\n", SOLCLIENT_SESSION_PROP_VPN_NAME, config.vpn_name);

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_USERNAME;
    sessionProps[propIndex++] = config.username;
    printf("Setting %s to: %s\n", SOLCLIENT_SESSION_PROP_USERNAME, config.username);

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_PASSWORD;
    sessionProps[propIndex++] = config.password;
    printf("Setting %s to: %s\n", SOLCLIENT_SESSION_PROP_PASSWORD, config.password);

    // sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_CONNECT_BLOCKING;
    // sessionProps[propIndex++] = (config.is_blocking == 1) ? SOLCLIENT_PROP_ENABLE_VAL : SOLCLIENT_PROP_DISABLE_VAL;
    // printf("Setting %s to: %d\n", SOLCLIENT_SESSION_PROP_CONNECT_BLOCKING, config.is_blocking);

    sessionProps[propIndex] = NULL;

    solClient_session_create ( ( char ** ) sessionProps,
                               context_p,
                               &session_p, &sessionFuncInfo, sizeof ( sessionFuncInfo ) );

    /* Connect the Session. */
    if(solClient_session_connect ( session_p ) != SOLCLIENT_FAIL) {
        printf ( "Connected.\n" );
    }
    else {
        printf("not succceded connecting to solclient...");
        solClient_errorInfo_pt errorInfo2 = solClient_getLastErrorInfo();
        printf(errorInfo2->errorStr);
        solClient_resetLastErrorInfo();
    }



    /*************************************************************************
     * Publish
     *************************************************************************/

    solClient_opaqueMsg_pt msg_p = NULL;
    solClient_destination_t destination;
    solClient_msg_alloc ( &msg_p );

    if(config.is_send_direct) {
        solClient_msg_setDeliveryMode ( msg_p, SOLCLIENT_DELIVERY_MODE_DIRECT );
    }
    else {
        solClient_msg_setDeliveryMode ( msg_p, SOLCLIENT_DELIVERY_MODE_PERSISTENT );
    }

    destination.destType = SOLCLIENT_TOPIC_DESTINATION;
    destination.dest = config.topic;
    solClient_msg_setDestination ( msg_p, &destination, sizeof ( destination ) );

    void* payload = malloc(config.payload_size_bytes);
    if(payload == NULL) { 
        printf("ERROR during allocation of payload.");
        return 1;
    }

    /* Add some content to the message. */
    solClient_msg_setBinaryAttachment ( msg_p, payload, ( solClient_uint32_t ) config.payload_size_bytes);

    printf ( "About to send %d message(s) of %d bytes to topic '%s'...\n", config.run_count, config.payload_size_bytes, config.topic );
    
    struct timeval* send_times =  malloc(sizeof(struct timeval) * (config.run_count));
    if(send_times == NULL) { 
        printf("ERROR during allocation of send times.");
        return 1;
    }
    int i;
    for (i = 0; i < config.warmup_count; i++) {
        solClient_session_sendMsg ( session_p, msg_p );
    }
    for (i = 0; i < config.run_count ; i++) {
        solClient_session_sendMsg ( session_p, msg_p );
        gettimeofday(&send_times[i], 0);
    }
    
    solClient_msg_free ( &msg_p );
    solClient_cleanup (  );

    print_analysis("Producer", "sends", send_times, config.run_count, config.threshold);

    // cleanup
    free(send_times);

    return 0;
}
