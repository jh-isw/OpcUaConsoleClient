#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>
#include <open62541.h>
#include <csignal>
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

void getEndpoints(string url);

void usage();

UA_Boolean running = true;
static void stopHandler(int sig) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
    running = false;
}

int main(int argc, const char** argv) {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("url", po::value<string>()->default_value("opc.tcp://localhost:16664"), "set URL of the server to connect to")
            ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    string url = vm["url"].as<string>();

    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }

    cout << "url is " << url << "\n";

    UA_Client *client = UA_Client_new(UA_ClientConfig_standard);

    usage();

    UA_StatusCode retval;
    bool isConnected = false;

    while(running){
        char * line = readline("> ");
        // if(!line) break;
        if(*line) add_history(line);

        if(strcmp ( line, "c") == 0){
            retval = UA_Client_connect(client, url.c_str());
            if(retval != UA_STATUSCODE_GOOD) {
                UA_Client_delete(client);
                free(line);
                return (int)retval;
            }
            isConnected = true;
        }

        else if(strcmp ( line, "d") == 0){
            if(isConnected){
                UA_Client_disconnect(client);
                isConnected = false;
            }
            else {
                cout << "no connection to disconnect" << endl;
            }

        }

        else if(strcmp ( line, "g") == 0){
            UA_EndpointDescription* endpointArray = NULL;
            size_t endpointArraySize = 0;
            retval = UA_Client_getEndpoints(client, url.c_str(),
                                                          &endpointArraySize, &endpointArray);
            if(retval != UA_STATUSCODE_GOOD) {
                UA_Array_delete(endpointArray, endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
                UA_Client_delete(client);
                free(line);
                return (int)retval;
            }
            else {
                isConnected = true;
            }
            printf("%i endpoints found\n", (int)endpointArraySize);
            for(size_t i=0;i<endpointArraySize;i++){
                printf("URL of endpoint %i is %.*s\n", (int)i,
                       (int)endpointArray[i].endpointUrl.length,
                       endpointArray[i].endpointUrl.data);
            }
            UA_Array_delete(endpointArray,endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
        }

        else if(strcmp ( line, "q") == 0){
            free(line);
            break;
        }

        else if(strcmp ( line, "u") == 0){
            usage();
        }

        else {
            cout << "not a valid command" << endl;
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
    c:      connect
    d:      disconnect
    g:      get Endpoints
    q:      quit
    u:      show usage
)";
    std::cout << cmds << std::endl;
}
