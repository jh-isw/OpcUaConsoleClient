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
            ("url", po::value<string>()->default_value("opc.tcp://localhost:4840"), "set URL of the server to connect to")
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
    while(running){
        char * line = readline("> ");
        if(!line) break;
        if(*line) add_history(line);

        if(strcmp ( line, "g") == 0){
//            getEndpoints(url);
            UA_EndpointDescription* endpointArray = NULL;
            size_t endpointArraySize = 0;
            UA_StatusCode retval = UA_Client_getEndpoints(client, url.c_str(),
                                                          &endpointArraySize, &endpointArray);
            if(retval != UA_STATUSCODE_GOOD) {
                UA_Array_delete(endpointArray, endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
                UA_Client_delete(client);
                return (int)retval;
            }
            printf("%i endpoints found\n", (int)endpointArraySize);
            for(size_t i=0;i<endpointArraySize;i++){
                printf("URL of endpoint %i is %.*s\n", (int)i,
                       (int)endpointArray[i].endpointUrl.length,
                       endpointArray[i].endpointUrl.data);
            }
            UA_Array_delete(endpointArray,endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
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

//void getEndpoints(string url){
//    //char * msg = strcat("getEndpoints called on ", url.c_str()); TODO: produces segfault
//    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "getEndpoints called");
//}
