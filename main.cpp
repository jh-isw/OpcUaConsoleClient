#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>
#include <open62541.h>
#include <csignal>

void getEndpoints();

void usage();

UA_Boolean running = true;
static void stopHandler(int sig) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
    running = false;
}

int main() {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    UA_Client *client = UA_Client_new(UA_ClientConfig_standard);

    usage();
    while(running){
        char * line = readline("> ");
        if(!line) break;
        if(*line) add_history(line);

        if(strcmp ( line, "g") == 0){
            getEndpoints();
        }

        if(strcmp ( line, "q") == 0){
            break;
        }

        if(strcmp ( line, "u") == 0){
            usage();
        }

        free(line);
    }

    /* Clean up */
    UA_Client_delete(client); /* Disconnects the client internally */
    return UA_STATUSCODE_GOOD;
}

void usage() {
    const char *cmds = R"(
    g:      get Endpoints
    q:      quit
    u:      show usage
)";
    std::cout << cmds << std::endl;
}

void getEndpoints(){
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "getEndpoints called");
}
