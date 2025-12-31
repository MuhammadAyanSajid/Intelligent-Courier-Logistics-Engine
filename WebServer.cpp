#include "WebServer.h"
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;
using namespace JsonUtils;

WebServer::WebServer(ParcelManager &pm, CourierOperations &ops, Graph &cityMap, string warehouse)
    : pm(pm), ops(ops), cityMap(cityMap), warehouseCity(warehouse)
{
    setupRoutes();
}

void WebServer::start(int port)
{
    cout << "\n+==================================================+\n";
    cout << "|     COURIER LOGISTICS WEB SERVER STARTED         |\n";
    cout << "+==================================================+\n";
    cout << "| Open your browser and go to:                     |\n";
    cout << "|   http://localhost:" << port << "                          |\n";
    cout << "+==================================================+\n";
    cout << "| Press Ctrl+C to stop the server                  |\n";
    cout << "+==================================================+\n\n";

    server.listen("0.0.0.0", port);
}

void WebServer::stop()
{
    server.stop();
}

void WebServer::setupRoutes()
{
    setupStaticFiles();
    setupParcelEndpoints();
    setupQueueEndpoints();
    setupRiderEndpoints();
    setupRouteEndpoints();
    setupSystemEndpoints();
}

void WebServer::setupStaticFiles()
{
    // Serve the main HTML page
    server.Get("/", [](const httplib::Request &, httplib::Response &res)
               {
        ifstream file("public/index.html");
        if (file.is_open()) {
            stringstream buffer;
            buffer << file.rdbuf();
            res.set_content(buffer.str(), "text/html");
        } else {
            res.set_content("<html><body><h1>Error: index.html not found</h1><p>Make sure public/index.html exists.</p></body></html>", "text/html");
        } });

    // Serve static files from public directory
    server.set_mount_point("/static", "./public");
}

void WebServer::setupParcelEndpoints()
{
    // GET /api/parcels - Get all parcels
    server.Get("/api/parcels", [this](const httplib::Request &, httplib::Response &res)
               {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        CustomVector<Parcel> parcels = pm.getAllParcels();
        res.set_content(successResponse("Parcels retrieved", parcelsToJson(parcels)), "application/json"); });

    // GET /api/parcels/:id - Get parcel by ID
    server.Get(R"(/api/parcels/(\d+))", [this](const httplib::Request &req, httplib::Response &res)
               {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        int id = stoi(req.matches[1]);
        Parcel p = pm.getParcel(id);
        if (p.getParcelID() == 0) {
            res.status = 404;
            res.set_content(errorResponse("Parcel not found"), "application/json");
        } else {
            res.set_content(successResponse("Parcel found", parcelToJson(p)), "application/json");
        } });

    // GET /api/parcels/priority - Get highest priority parcel
    server.Get("/api/parcels/priority", [this](const httplib::Request &, httplib::Response &res)
               {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        if (!pm.hasParcels()) {
            res.set_content(errorResponse("No parcels in queue"), "application/json");
        } else {
            Parcel p = pm.peekHighPriority();
            res.set_content(successResponse("High priority parcel", parcelToJson(p)), "application/json");
        } });

    // POST /api/parcels - Add new parcel
    server.Post("/api/parcels", [this](const httplib::Request &req, httplib::Response &res)
                {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        string body = req.body;
        
        int id = getJsonInt(body, "parcelID");
        string dest = getJsonValue(body, "destination");
        int weight = getJsonInt(body, "weight");
        int priority = getJsonInt(body, "priority", 3);
        string sender = getJsonValue(body, "senderAddress");
        if (sender.empty()) sender = "Warehouse";
        
        if (id <= 0) {
            res.status = 400;
            res.set_content(errorResponse("Invalid parcel ID"), "application/json");
            return;
        }
        if (dest.empty()) {
            res.status = 400;
            res.set_content(errorResponse("Destination is required"), "application/json");
            return;
        }
        if (weight <= 0) {
            res.status = 400;
            res.set_content(errorResponse("Invalid weight"), "application/json");
            return;
        }
        
        if (pm.parcelExists(id)) {
            res.status = 400;
            res.set_content(errorResponse("Parcel ID already exists"), "application/json");
            return;
        }
        
        if (!cityMap.cityExists(dest)) {
            res.status = 400;
            res.set_content(errorResponse("Destination city does not exist"), "application/json");
            return;
        }
        
        Parcel newP(id, dest, weight, priority, sender);
        newP.setZone(cityMap.getCityZone(dest));
        pm.addParcel(newP);
        
        Action act(ACTION_ADD_PARCEL, id);
        act.storeParcelState(newP);
        ops.logAction(act);
        
        pm.saveParcels("parcels.txt");
        res.set_content(successResponse("Parcel added successfully", parcelToJson(newP)), "application/json"); });

    // POST /api/parcels/dispatch - Dispatch highest priority parcel
    server.Post("/api/parcels/dispatch", [this](const httplib::Request &, httplib::Response &res)
                {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        if (!pm.hasParcels()) {
            res.set_content(errorResponse("No parcels to dispatch"), "application/json");
            return;
        }
        
        Parcel p = pm.peekHighPriority();
        string prevStatus = p.getStatus();
        
        Parcel dispatched = pm.dispatchNext();
        dispatched.updateStatus(STATUS_IN_TRANSIT);
        pm.updateParcel(dispatched);
        ops.addToTransit(dispatched);
        
        Action act(ACTION_DISPATCH, dispatched.getParcelID());
        act.setDestination(dispatched.getDestination());
        act.setWeight(dispatched.getWeight());
        act.setPriority(dispatched.getPriority());
        act.setPrevStatus(prevStatus);
        ops.logAction(act);
        
        // Don't remove parcel from map - keep it so rider assignment works
        // pm.removeParcel(dispatched.getParcelID());
        pm.saveParcels("parcels.txt");
        
        // Get routes
        CustomVector<RouteResult> routes = cityMap.findKShortestPaths(warehouseCity, dispatched.getDestination(), 3);
        
        stringstream data;
        data << "{\"parcel\":" << parcelToJson(dispatched) << ",\"routes\":" << routesToJson(routes) << "}";
        res.set_content(successResponse("Parcel dispatched", data.str()), "application/json"); });

    // POST /api/parcels/:id/attempt - Record delivery attempt
    server.Post(R"(/api/parcels/(\d+)/attempt)", [this](const httplib::Request &req, httplib::Response &res)
                {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        int id = stoi(req.matches[1]);
        string reason = getJsonValue(req.body, "reason");
        if (reason.empty()) reason = "Recipient not available";
        
        Parcel p = pm.getParcel(id);
        if (p.getParcelID() == 0) {
            res.status = 404;
            res.set_content(errorResponse("Parcel not found"), "application/json");
            return;
        }
        
        if (!p.canAttemptDelivery()) {
            res.set_content(errorResponse("Cannot attempt delivery for this parcel"), "application/json");
            return;
        }
        
        Action act(ACTION_DELIVERY_ATTEMPT, p.getParcelID());
        act.storeParcelState(p);
        
        bool canRetry = p.recordDeliveryAttempt(reason);
        pm.updateParcel(p);
        ops.logAction(act);
        pm.saveParcels("parcels.txt");
        
        stringstream msg;
        if (!canRetry) {
            msg << "Max delivery attempts reached. Parcel will be returned to sender.";
        } else {
            msg << "Attempt recorded. " << (p.getMaxAttempts() - p.getDeliveryAttempts()) << " attempts remaining.";
        }
        res.set_content(successResponse(msg.str(), parcelToJson(p)), "application/json"); });

    // POST /api/parcels/:id/deliver - Mark as delivered
    server.Post(R"(/api/parcels/(\d+)/deliver)", [this](const httplib::Request &req, httplib::Response &res)
                {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        int id = stoi(req.matches[1]);
        Parcel p = pm.getParcel(id);
        
        if (p.getParcelID() == 0) {
            res.status = 404;
            res.set_content(errorResponse("Parcel not found"), "application/json");
            return;
        }
        
        if (p.getStatus() == STATUS_DELIVERED) {
            res.set_content(errorResponse("Parcel is already delivered"), "application/json");
            return;
        }
        
        Action act(ACTION_MARK_DELIVERED, p.getParcelID());
        act.storeParcelState(p);
        
        p.markDelivered();
        
        if (p.getAssignedRiderID() > 0) {
            ops.releaseParcelFromRider(p.getParcelID(), p.getWeight(), p.getAssignedRiderID());
            p.setAssignedRiderID(-1);
        }
        
        pm.updateParcel(p);
        ops.logAction(act);
        pm.saveParcels("parcels.txt");
        
        res.set_content(successResponse("Parcel marked as delivered", parcelToJson(p)), "application/json"); });

    // POST /api/parcels/:id/return - Return to sender
    server.Post(R"(/api/parcels/(\d+)/return)", [this](const httplib::Request &req, httplib::Response &res)
                {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        int id = stoi(req.matches[1]);
        string reason = getJsonValue(req.body, "reason");
        if (reason.empty()) reason = "Returned by request";
        
        Parcel p = pm.getParcel(id);
        
        if (p.getParcelID() == 0) {
            res.status = 404;
            res.set_content(errorResponse("Parcel not found"), "application/json");
            return;
        }
        
        if (p.getStatus() == STATUS_DELIVERED) {
            res.set_content(errorResponse("Cannot return a delivered parcel"), "application/json");
            return;
        }
        
        if (p.getStatus() == STATUS_RETURNED) {
            res.set_content(errorResponse("Parcel is already marked for return"), "application/json");
            return;
        }
        
        Action act(ACTION_RETURN_TO_SENDER, p.getParcelID());
        act.storeParcelState(p);
        
        p.returnToSender(reason);
        
        if (p.getAssignedRiderID() > 0) {
            ops.releaseParcelFromRider(p.getParcelID(), p.getWeight(), p.getAssignedRiderID());
            p.setAssignedRiderID(-1);
        }
        
        pm.updateParcel(p);
        ops.logAction(act);
        pm.saveParcels("parcels.txt");
        
        res.set_content(successResponse("Parcel marked for return to sender", parcelToJson(p)), "application/json"); });
}

void WebServer::setupQueueEndpoints()
{
    // ============== PICKUP QUEUE ==============

    // GET /api/queues/pickup - Get pickup queue status
    server.Get("/api/queues/pickup", [this](const httplib::Request &, httplib::Response &res)
               {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        stringstream data;
        data << "{\"size\":" << ops.getPickupQueueSize();
        if (ops.hasPickupPending()) {
            data << ",\"next\":" << parcelToJson(ops.peekPickupQueue());
        }
        data << "}";
        res.set_content(successResponse("Pickup queue status", data.str()), "application/json"); });

    // POST /api/queues/pickup/:id - Add parcel to pickup queue
    server.Post(R"(/api/queues/pickup/(\d+))", [this](const httplib::Request &req, httplib::Response &res)
                {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        int id = stoi(req.matches[1]);
        Parcel p = pm.getParcel(id);
        
        if (p.getParcelID() == 0) {
            res.status = 404;
            res.set_content(errorResponse("Parcel not found"), "application/json");
            return;
        }
        
        if (p.getStatus() == STATUS_DELIVERED || p.getStatus() == STATUS_RETURNED) {
            res.set_content(errorResponse("Cannot add this parcel to pickup queue"), "application/json");
            return;
        }
        
        p.updateStatus(STATUS_PENDING_PICKUP);
        ops.addToPickupQueue(p);
        pm.updateParcel(p);
        
        Action act(ACTION_PICKUP, id);
        act.storeParcelState(p);
        ops.logAction(act);
        pm.saveParcels("parcels.txt");
        
        res.set_content(successResponse("Parcel added to pickup queue", parcelToJson(p)), "application/json"); });

    // POST /api/queues/pickup/process - Process next pickup
    server.Post("/api/queues/pickup/process", [this](const httplib::Request &, httplib::Response &res)
                {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        if (!ops.hasPickupPending()) {
            res.set_content(errorResponse("Pickup queue is empty"), "application/json");
            return;
        }
        
        Parcel p = ops.processPickup();
        p.updateStatus(STATUS_PICKED_UP);
        pm.updateParcel(p);
        pm.saveParcels("parcels.txt");
        
        res.set_content(successResponse("Pickup processed", parcelToJson(p)), "application/json"); });

    // ============== WAREHOUSE QUEUE ==============

    // GET /api/queues/warehouse - Get warehouse queue status
    server.Get("/api/queues/warehouse", [this](const httplib::Request &, httplib::Response &res)
               {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        stringstream data;
        data << "{\"size\":" << ops.getWarehouseSize();
        if (ops.hasPendingParcels()) {
            data << ",\"next\":" << parcelToJson(ops.peekWarehouse());
        }
        data << "}";
        res.set_content(successResponse("Warehouse queue status", data.str()), "application/json"); });

    // POST /api/queues/warehouse/:id - Add parcel to warehouse
    server.Post(R"(/api/queues/warehouse/(\d+))", [this](const httplib::Request &req, httplib::Response &res)
                {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        int id = stoi(req.matches[1]);
        Parcel p = pm.getParcel(id);
        
        if (p.getParcelID() == 0) {
            res.status = 404;
            res.set_content(errorResponse("Parcel not found"), "application/json");
            return;
        }
        
        if (p.getStatus() == STATUS_DELIVERED || p.getStatus() == STATUS_RETURNED) {
            res.set_content(errorResponse("Cannot add this parcel to warehouse"), "application/json");
            return;
        }
        
        p.updateStatus(STATUS_IN_WAREHOUSE);
        ops.addToWarehouse(p);
        pm.updateParcel(p);
        
        Action act(ACTION_ADD_TO_WAREHOUSE, id);
        ops.logAction(act);
        pm.saveParcels("parcels.txt");
        
        res.set_content(successResponse("Parcel added to warehouse", parcelToJson(p)), "application/json"); });

    // POST /api/queues/warehouse/load - Load from warehouse
    server.Post("/api/queues/warehouse/load", [this](const httplib::Request &, httplib::Response &res)
                {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        if (!ops.hasPendingParcels()) {
            res.set_content(errorResponse("Warehouse queue is empty"), "application/json");
            return;
        }
        
        Parcel p = ops.loadFromWarehouse();
        p.updateStatus(STATUS_QUEUED_LOADING);
        pm.updateParcel(p);
        pm.saveParcels("parcels.txt");
        
        res.set_content(successResponse("Parcel loaded from warehouse", parcelToJson(p)), "application/json"); });

    // ============== TRANSIT QUEUE ==============

    // GET /api/queues/transit - Get transit queue status
    server.Get("/api/queues/transit", [this](const httplib::Request &, httplib::Response &res)
               {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        stringstream data;
        data << "{\"size\":" << ops.getTransitQueueSize();
        if (ops.hasTransitParcels()) {
            data << ",\"next\":" << parcelToJson(ops.peekTransitQueue());
        }
        data << "}";
        res.set_content(successResponse("Transit queue status", data.str()), "application/json"); });

    // POST /api/queues/transit/complete - Complete transit
    server.Post("/api/queues/transit/complete", [this](const httplib::Request &, httplib::Response &res)
                {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        if (!ops.hasTransitParcels()) {
            res.set_content(errorResponse("Transit queue is empty"), "application/json");
            return;
        }
        
        Parcel p = ops.completeTransit();
        p.updateStatus(STATUS_OUT_FOR_DELIVERY);
        pm.updateParcel(p);
        pm.saveParcels("parcels.txt");
        
        res.set_content(successResponse("Transit completed", parcelToJson(p)), "application/json"); });
}

void WebServer::setupRiderEndpoints()
{
    // GET /api/riders - Get all riders
    server.Get("/api/riders", [this](const httplib::Request &, httplib::Response &res)
               {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        CustomVector<Rider> riders = ops.getAllRiders();
        res.set_content(successResponse("Riders retrieved", ridersToJson(riders)), "application/json"); });

    // GET /api/riders/:id - Get rider by ID
    server.Get(R"(/api/riders/(\d+))", [this](const httplib::Request &req, httplib::Response &res)
               {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        int id = stoi(req.matches[1]);
        if (!ops.riderExists(id)) {
            res.status = 404;
            res.set_content(errorResponse("Rider not found"), "application/json");
            return;
        }
        
        Rider r = ops.getRider(id);
        res.set_content(successResponse("Rider found", riderToJson(r)), "application/json"); });

    // POST /api/riders - Add new rider
    server.Post("/api/riders", [this](const httplib::Request &req, httplib::Response &res)
                {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        string name = getJsonValue(req.body, "name");
        int capacity = getJsonInt(req.body, "capacity", 50);
        int maxParcels = getJsonInt(req.body, "maxParcels", 10);
        
        if (name.empty()) {
            res.status = 400;
            res.set_content(errorResponse("Rider name is required"), "application/json");
            return;
        }
        
        int id = ops.addRider(name, capacity, maxParcels);
        Rider r = ops.getRider(id);
        res.set_content(successResponse("Rider added", riderToJson(r)), "application/json"); });

    // POST /api/riders/:riderId/assign/:parcelId - Assign parcel to rider
    server.Post(R"(/api/riders/(\d+)/assign/(\d+))", [this](const httplib::Request &req, httplib::Response &res)
                {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        int riderId = stoi(req.matches[1]);
        int parcelId = stoi(req.matches[2]);
        
        if (!ops.riderExists(riderId)) {
            res.status = 404;
            res.set_content(errorResponse("Rider not found"), "application/json");
            return;
        }
        
        Parcel p = pm.getParcel(parcelId);
        if (p.getParcelID() == 0) {
            res.status = 404;
            res.set_content(errorResponse("Parcel not found"), "application/json");
            return;
        }
        
        if (p.getAssignedRiderID() > 0) {
            res.set_content(errorResponse("Parcel already assigned to a rider"), "application/json");
            return;
        }
        
        if (!ops.assignParcelToRider(parcelId, p.getWeight(), riderId)) {
            res.set_content(errorResponse("Rider cannot take this parcel (capacity exceeded)"), "application/json");
            return;
        }
        
        p.setAssignedRiderID(riderId);
        p.updateStatus(STATUS_OUT_FOR_DELIVERY);
        pm.updateParcel(p);
        
        Action act(ACTION_ASSIGN_RIDER, parcelId);
        act.storeParcelState(p);
        act.setRiderID(riderId);
        ops.logAction(act);
        pm.saveParcels("parcels.txt");
        
        Rider r = ops.getRider(riderId);
        stringstream data;
        data << "{\"parcel\":" << parcelToJson(p) << ",\"rider\":" << riderToJson(r) << "}";
        res.set_content(successResponse("Parcel assigned to rider", data.str()), "application/json"); });

    // POST /api/riders/auto-assign/:parcelId - Auto-assign best rider
    server.Post(R"(/api/riders/auto-assign/(\d+))", [this](const httplib::Request &req, httplib::Response &res)
                {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        int parcelId = stoi(req.matches[1]);
        Parcel p = pm.getParcel(parcelId);
        
        if (p.getParcelID() == 0) {
            res.status = 404;
            res.set_content(errorResponse("Parcel not found"), "application/json");
            return;
        }
        
        if (p.getAssignedRiderID() > 0) {
            res.set_content(errorResponse("Parcel already assigned to a rider"), "application/json");
            return;
        }
        
        Rider* best = ops.findBestRider(p.getWeight(), p.getZone());
        if (best == nullptr) {
            res.set_content(errorResponse("No suitable rider available"), "application/json");
            return;
        }
        
        int riderId = best->getRiderID();
        ops.assignParcelToRider(parcelId, p.getWeight(), riderId);
        
        p.setAssignedRiderID(riderId);
        p.updateStatus(STATUS_OUT_FOR_DELIVERY);
        pm.updateParcel(p);
        
        Action act(ACTION_ASSIGN_RIDER, parcelId);
        act.storeParcelState(p);
        act.setRiderID(riderId);
        ops.logAction(act);
        pm.saveParcels("parcels.txt");
        
        Rider r = ops.getRider(riderId);
        stringstream data;
        data << "{\"parcel\":" << parcelToJson(p) << ",\"rider\":" << riderToJson(r) << "}";
        res.set_content(successResponse("Parcel auto-assigned to best rider", data.str()), "application/json"); });
}

void WebServer::setupRouteEndpoints()
{
    // GET /api/routes - Find routes between cities
    server.Get("/api/routes", [this](const httplib::Request &req, httplib::Response &res)
               {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        string from = req.get_param_value("from");
        string to = req.get_param_value("to");
        int k = 3;
        if (req.has_param("k")) {
            k = stoi(req.get_param_value("k"));
            if (k < 1) k = 1;
            if (k > 5) k = 5;
        }
        
        if (from.empty() || to.empty()) {
            res.status = 400;
            res.set_content(errorResponse("'from' and 'to' parameters are required"), "application/json");
            return;
        }
        
        if (!cityMap.cityExists(from)) {
            res.status = 400;
            res.set_content(errorResponse("Source city does not exist"), "application/json");
            return;
        }
        
        if (!cityMap.cityExists(to)) {
            res.status = 400;
            res.set_content(errorResponse("Destination city does not exist"), "application/json");
            return;
        }
        
        CustomVector<RouteResult> routes = cityMap.findKShortestPaths(from, to, k);
        res.set_content(successResponse("Routes found", routesToJson(routes)), "application/json"); });

    // GET /api/cities - Get all cities
    server.Get("/api/cities", [this](const httplib::Request &, httplib::Response &res)
               {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        CustomVector<string> cities = cityMap.getAllCities();
        res.set_content(successResponse("Cities retrieved", stringsToJson(cities)), "application/json"); });

    // GET /api/zones - Get all zones
    server.Get("/api/zones", [this](const httplib::Request &, httplib::Response &res)
               {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        CustomVector<string> zones = cityMap.getAllZones();
        res.set_content(successResponse("Zones retrieved", stringsToJson(zones)), "application/json"); });

    // GET /api/graph - Get graph data for visualization
    server.Get("/api/graph", [this](const httplib::Request &, httplib::Response &res)
               {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        CustomVector<string> cities = cityMap.getAllCities();
        CustomVector<string> zones = cityMap.getAllZones();
        
        stringstream data;
        data << "{\"cities\":[";
        for (int i = 0; i < cities.getSize(); i++) {
            if (i > 0) data << ",";
            string city = cities[i];
            string zone = cityMap.getCityZone(city);
            data << "{\"name\":\"" << escapeJson(city) << "\",\"zone\":\"" << escapeJson(zone) << "\"}";
        }
        data << "],\"edges\":[";
        
        bool firstEdge = true;
        for (int i = 0; i < cities.getSize(); i++) {
            for (int j = i + 1; j < cities.getSize(); j++) {
                int weight = cityMap.getRoadWeight(cities[i], cities[j]);
                if (weight > 0) {
                    if (!firstEdge) data << ",";
                    bool blocked = cityMap.isRoadBlocked(cities[i], cities[j]);
                    data << edgeToJson(cities[i], cities[j], weight, blocked);
                    firstEdge = false;
                }
            }
        }
        data << "],\"zones\":" << stringsToJson(zones) << "}";
        
        res.set_content(successResponse("Graph data", data.str()), "application/json"); });

    // POST /api/roads/block - Block a road
    server.Post("/api/roads/block", [this](const httplib::Request &req, httplib::Response &res)
                {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        string from = getJsonValue(req.body, "from");
        string to = getJsonValue(req.body, "to");
        
        if (from.empty() || to.empty()) {
            res.status = 400;
            res.set_content(errorResponse("'from' and 'to' are required"), "application/json");
            return;
        }
        
        int originalWeight = cityMap.getRoadWeight(from, to);
        if (originalWeight <= 0) {
            res.set_content(errorResponse("Road does not exist"), "application/json");
            return;
        }
        
        cityMap.blockRoad(from, to);
        
        Action act(ACTION_BLOCK_ROAD, -1, from, to);
        act.setRoadWeight(originalWeight);
        ops.logAction(act);
        
        cityMap.saveGraph("map_data.txt");
        res.set_content(successResponse("Road blocked"), "application/json"); });

    // POST /api/roads/unblock - Unblock a road
    server.Post("/api/roads/unblock", [this](const httplib::Request &req, httplib::Response &res)
                {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        string from = getJsonValue(req.body, "from");
        string to = getJsonValue(req.body, "to");
        int weight = getJsonInt(req.body, "weight", 0);
        
        if (from.empty() || to.empty()) {
            res.status = 400;
            res.set_content(errorResponse("'from' and 'to' are required"), "application/json");
            return;
        }
        
        if (weight <= 0) {
            res.status = 400;
            res.set_content(errorResponse("'weight' is required to unblock"), "application/json");
            return;
        }
        
        if (!cityMap.isRoadBlocked(from, to)) {
            res.set_content(errorResponse("Road is not blocked"), "application/json");
            return;
        }
        
        cityMap.unblockRoad(from, to, weight);
        cityMap.saveGraph("map_data.txt");
        res.set_content(successResponse("Road unblocked"), "application/json"); });
}

void WebServer::setupSystemEndpoints()
{
    // GET /api/status - System status dashboard
    server.Get("/api/status", [this](const httplib::Request &, httplib::Response &res)
               {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        stringstream data;
        data << "{";
        data << "\"cityCount\":" << cityMap.getCityCount() << ",";
        data << "\"zoneCount\":" << cityMap.getAllZones().getSize() << ",";
        data << "\"hasParcels\":" << (pm.hasParcels() ? "true" : "false") << ",";
        data << "\"parcelCount\":" << pm.getAllParcels().getSize() << ",";
        data << "\"pickupQueueSize\":" << ops.getPickupQueueSize() << ",";
        data << "\"warehouseQueueSize\":" << ops.getWarehouseSize() << ",";
        data << "\"transitQueueSize\":" << ops.getTransitQueueSize() << ",";
        data << "\"riderCount\":" << ops.getRiderCount() << ",";
        data << "\"missingParcelsCount\":" << ops.getMissingCount() << ",";
        data << "\"auditLogSize\":" << ops.getAuditLogSize() << ",";
        data << "\"warehouseLocation\":\"" << escapeJson(warehouseCity) << "\"";
        data << "}";
        
        res.set_content(successResponse("System status", data.str()), "application/json"); });

    // GET /api/missing - Get missing parcels
    server.Get("/api/missing", [this](const httplib::Request &, httplib::Response &res)
               {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        CustomVector<Parcel> missing = ops.getMissingParcels();
        res.set_content(successResponse("Missing parcels", parcelsToJson(missing)), "application/json"); });

    // POST /api/missing/:id/report - Report parcel as missing
    server.Post(R"(/api/missing/(\d+)/report)", [this](const httplib::Request &req, httplib::Response &res)
                {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        int id = stoi(req.matches[1]);
        string reason = getJsonValue(req.body, "reason");
        if (reason.empty()) reason = "Unknown reason";
        
        Parcel p = pm.getParcel(id);
        if (p.getParcelID() == 0) {
            res.status = 404;
            res.set_content(errorResponse("Parcel not found"), "application/json");
            return;
        }
        
        ops.reportMissing(p, reason);
        pm.updateParcel(p);
        pm.saveParcels("parcels.txt");
        
        res.set_content(successResponse("Parcel reported as missing", parcelToJson(p)), "application/json"); });

    // POST /api/missing/:id/resolve - Resolve missing parcel
    server.Post(R"(/api/missing/(\d+)/resolve)", [this](const httplib::Request &req, httplib::Response &res)
                {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        int id = stoi(req.matches[1]);
        ops.resolveMissing(id);
        res.set_content(successResponse("Missing parcel resolved"), "application/json"); });

    // POST /api/undo - Undo last action
    server.Post("/api/undo", [this](const httplib::Request &, httplib::Response &res)
                {
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        if (!ops.hasUndoableActions()) {
            res.set_content(errorResponse("Nothing to undo"), "application/json");
            return;
        }
        
        Action act = ops.undoLastAction();
        
        switch (act.getType()) {
            case ACTION_ADD_PARCEL: {
                pm.removeParcel(act.getParcelID());
                break;
            }
            case ACTION_DISPATCH: {
                Parcel restored = act.reconstructParcel();
                pm.reinsertParcel(restored);
                break;
            }
            case ACTION_BLOCK_ROAD: {
                cityMap.unblockRoad(act.getData1(), act.getData2(), act.getRoadWeight());
                cityMap.saveGraph("map_data.txt");
                break;
            }
            case ACTION_ASSIGN_RIDER: {
                Parcel p = pm.getParcel(act.getParcelID());
                if (p.getParcelID() != 0) {
                    ops.releaseParcelFromRider(act.getParcelID(), p.getWeight(), act.getRiderID());
                    p.setAssignedRiderID(-1);
                    p.setStatus(act.getPrevStatus());
                    pm.updateParcel(p);
                }
                break;
            }
            case ACTION_DELIVERY_ATTEMPT: {
                Parcel p = pm.getParcel(act.getParcelID());
                if (p.getParcelID() != 0) {
                    p.setDeliveryAttempts(act.getAttemptCount());
                    p.setStatus(act.getPrevStatus());
                    pm.updateParcel(p);
                }
                break;
            }
            case ACTION_MARK_DELIVERED:
            case ACTION_RETURN_TO_SENDER: {
                Parcel restored = act.reconstructParcel();
                pm.updateParcel(restored);
                break;
            }
            default:
                break;
        }
        
        pm.saveParcels("parcels.txt");
        res.set_content(successResponse("Action undone"), "application/json"); });
}
