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

//static UA_StatusCode
//nodeIter(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle) {
//    if(isInverse)
//        return UA_STATUSCODE_GOOD;
//    UA_NodeId *parent = (UA_NodeId *)handle;
//    printf("%d, %d --- %d ---> NodeId %d, %d\n",
//           parent->namespaceIndex, parent->identifier.numeric,
//           referenceTypeId.identifier.numeric, childId.namespaceIndex,
//           childId.identifier.numeric);
//    return UA_STATUSCODE_GOOD;
//}

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

        // browse
        if(strcmp ( line, "b") == 0){
            UA_BrowseRequest bReq;
            UA_BrowseRequest_init(&bReq);
            bReq.requestedMaxReferencesPerNode = 0;
            bReq.nodesToBrowse = UA_BrowseDescription_new();
            bReq.nodesToBrowseSize = 1;
            bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ROOTFOLDER);
            bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; /* return everything */
            UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
            printf("%-9s %-16s %-16s %-16s\n", "NAMESPACE", "NODEID", "BROWSE NAME", "DISPLAY NAME");
            for (size_t i = 0; i < bResp.resultsSize; ++i) {
                for (size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
                    UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
                    if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {
                        printf("%-9d %-16d %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
                               ref->nodeId.nodeId.identifier.numeric, (int)ref->browseName.name.length,
                               ref->browseName.name.data, (int)ref->displayName.text.length,
                               ref->displayName.text.data);
                    } else if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
                        printf("%-9d %-16.*s %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
                               (int)ref->nodeId.nodeId.identifier.string.length,
                               ref->nodeId.nodeId.identifier.string.data,
                               (int)ref->browseName.name.length, ref->browseName.name.data,
                               (int)ref->displayName.text.length, ref->displayName.text.data);
                    }
                    /* TODO: distinguish further types */
                }
            }
            UA_BrowseRequest_deleteMembers(&bReq);
            UA_BrowseResponse_deleteMembers(&bResp);

//            /* Same thing, this time using the node iterator... */
//            UA_NodeId *parent = UA_NodeId_new();
//            *parent = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
//            UA_Client_forEachChildNodeCall(client, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
//                                           nodeIter, (void *) parent);
//            UA_NodeId_delete(parent);
        }

        // connect
        else if(strcmp ( line, "c") == 0){
            if(!isConnected){
                retval = UA_Client_connect(client, url.c_str());
                if(retval != UA_STATUSCODE_GOOD) {
                    UA_Client_delete(client);
                    free(line);
                    cout << "connect failed" << endl;
                    return (int)retval;
                }
                isConnected = true;
                cout << "connected to " << url << endl;
            }
            else{
                cout << "you are already connected to " << url << endl;
            }
        }

        // disconnect
        else if(strcmp ( line, "d") == 0){
            if(isConnected){
                UA_Client_disconnect(client);
                isConnected = false;
                cout << "you are now disconnected" << endl;
            }
            else {
                cout << "no active connection to disconnect" << endl;
            }

        }

        // get Endpoints
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
    b:      browse
    c:      connect
    d:      disconnect
    g:      get Endpoints
    q:      quit
    u:      show usage
)";
    std::cout << cmds << std::endl;
}
