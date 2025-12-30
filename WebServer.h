#ifndef WEB_SERVER_H
#define WEB_SERVER_H
// Fix std::byte conflict on Windows
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _HAS_STD_BYTE 0
#include <winsock2.h>
#include <windows.h>
#endif
#include "httplib.h"
#include "ParcelManager.h"
#include "CourierOperations.h"
#include "Graph.h"
#include "JsonUtils.h"
#include <string>

using namespace std;

class WebServer
{
private:
    httplib::Server server;
    ParcelManager &pm;
    CourierOperations &ops;
    Graph &cityMap;
    string warehouseCity;

    void setupRoutes();
    void setupStaticFiles();
    void setupParcelEndpoints();
    void setupQueueEndpoints();
    void setupRiderEndpoints();
    void setupRouteEndpoints();
    void setupSystemEndpoints();

public:
    WebServer(ParcelManager &pm, CourierOperations &ops, Graph &cityMap, string warehouse = "Karachi");
    void start(int port = 8080);
    void stop();
};

#endif
