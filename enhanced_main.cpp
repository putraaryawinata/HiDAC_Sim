#include "Agent.h"
#include "ORCAAgent.h"
#include "CrowdObject.h"
#include "Wall.h"
#include "EnhancedCrowdWorld.h"
#include "DatasetLoader.h"
#include "Render.h"
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string>

using namespace std;

Json::Value readJsonFromFile(const char* file);
void printUsage(const char* programName);
void runOriginalSimulation(const Json::Value& data);
void runORCASimulation(const Json::Value& data);
void runDatasetPlayback(const Json::Value& data);

int mysleep(unsigned long millis) {
    struct timespec req = {0};
    time_t sec = (int)(millis / 1000);
    millis = millis - (sec * 1000);
    req.tv_sec = sec;
    req.tv_nsec = millis * 1000000L;
    return nanosleep(&req, NULL);
}

Json::Value readJsonFromFile(const char* file) {
    Json::Reader reader;
    Json::Value root;
    std::ifstream data_file;
    data_file.open(file);
    
    if (!data_file.is_open()) {
        std::cout << "Could not open file: " << file << std::endl;
        exit(1);
    }
    
    std::string the_str(std::istreambuf_iterator<char>(data_file),
                       (std::istreambuf_iterator<char>()));
    
    data_file.close();
    
    bool success = reader.parse(the_str, root);
    
    if (!success) {
        std::cout << "bad json data!\n";
        std::cout << reader.getFormatedErrorMessages();
        exit(1);
    }
    
    return root;
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options] <config_file>" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --mode <mode>     Simulation mode: original, orca, dataset" << std::endl;
    std::cout << "  --dataset <file>  ETH/UCY dataset file for dataset mode" << std::endl;
    std::cout << "  --format <fmt>    Dataset format: eth, ucy, trajnet (default: eth)" << std::endl;
    std::cout << "  --help            Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << " data/test.json                    # Original simulation" << std::endl;
    std::cout << "  " << programName << " --mode orca data/orca_demo.json  # ORCA simulation" << std::endl;
    std::cout << "  " << programName << " --mode dataset --dataset data/sample_eth.txt data/dataset_config.json" << std::endl;
}

int main(int argc, char** argv) {
    // Parse command line arguments
    std::string mode = "original";
    std::string configFile;
    std::string datasetFile;
    std::string datasetFormat = "eth";
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--mode") {
            if (i + 1 < argc) {
                mode = argv[++i];
            } else {
                std::cerr << "Error: --mode requires an argument" << std::endl;
                return 1;
            }
        } else if (arg == "--dataset") {
            if (i + 1 < argc) {
                datasetFile = argv[++i];
            } else {
                std::cerr << "Error: --dataset requires an argument" << std::endl;
                return 1;
            }
        } else if (arg == "--format") {
            if (i + 1 < argc) {
                datasetFormat = argv[++i];
            } else {
                std::cerr << "Error: --format requires an argument" << std::endl;
                return 1;
            }
        } else if (arg[0] != '-') {
            configFile = arg;
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    // Default config file if none specified
    if (configFile.empty()) {
        configFile = "data/test.json";
    }
    
    // Load configuration
    Json::Value data = readJsonFromFile(configFile.c_str());
    std::srand(0);
    
    // Run simulation based on mode
    std::cout << "Running simulation in " << mode << " mode..." << std::endl;
    
    if (mode == "original") {
        runOriginalSimulation(data);
    } else if (mode == "orca") {
        runORCASimulation(data);
    } else if (mode == "dataset") {
        if (datasetFile.empty()) {
            std::cerr << "Error: dataset mode requires --dataset argument" << std::endl;
            return 1;
        }
        // Override dataset configuration
        if (!data.isMember("simulation")) {
            data["simulation"] = Json::Value();
        }
        data["simulation"]["dataset"]["filename"] = datasetFile;
        data["simulation"]["dataset"]["format"] = datasetFormat;
        runDatasetPlayback(data);
    } else {
        std::cerr << "Unknown mode: " << mode << std::endl;
        std::cerr << "Valid modes: original, orca, dataset" << std::endl;
        return 1;
    }
    
    return 0;
}

void runOriginalSimulation(const Json::Value& data) {
    std::cout << "Starting original social force simulation..." << std::endl;
    
    int steps = data["steps"].asInt();
    float deltat = data["timeslice"].asDouble();
    
    Agent* a = new Agent(data["agents"][0u]);
    Render* r = Render::getInstance();
    CrowdObject* cos = twoWalls(data["walls"][0u]);
    CrowdWorld c(data);
    
    while (!r->isInitialized()) {
        mysleep(10);
    }
    
    for (int i = 0; i < steps; i++) {
        c.updateAgents();
        c.calcForces();
        c.stepWorld(deltat);
        c.print();
        r->update(deltat);
        mysleep(10);
    }
    
    delete a;
    delete cos;
}

void runORCASimulation(const Json::Value& data) {
    std::cout << "Starting ORCA simulation..." << std::endl;
    
    // Create enhanced world with ORCA mode
    EnhancedCrowdWorld world(data);
    world.setMode(ORCA_SIMULATION);
    
    // Set ORCA parameters if specified
    if (data.isMember("simulation")) {
        const Json::Value& sim = data["simulation"];
        float timeHorizon = sim.get("timeHorizon", 2.0f).asFloat();
        float neighborDist = sim.get("neighborDist", 10.0f).asFloat();
        int maxNeighbors = sim.get("maxNeighbors", 10).asInt();
        world.setORCAParameters(timeHorizon, neighborDist, maxNeighbors);
    }
    
    Render* r = Render::getInstance();
    while (!r->isInitialized()) {
        mysleep(10);
    }
    
    int steps = data.get("steps", 1000).asInt();
    float deltaT = data.get("timeslice", 0.4f).asFloat();
    
    world.play();
    
    for (int i = 0; i < steps && world.getIsPlaying(); ++i) {
        world.step(deltaT);
        r->update(deltaT);
        mysleep(10);
        
        if (i % 100 == 0) {
            std::cout << "Step " << i << "/" << steps << " (time: " 
                      << world.getCurrentTime() << "s)" << std::endl;
        }
    }
    
    world.printSimulationStats();
}

void runDatasetPlayback(const Json::Value& data) {
    std::cout << "Starting dataset playback..." << std::endl;
    
    // Extract dataset configuration
    std::string filename = data["simulation"]["dataset"]["filename"].asString();
    std::string format = data["simulation"]["dataset"].get("format", "eth").asString();
    float frameRate = data["simulation"]["dataset"].get("frameRate", 2.5f).asFloat();
    float pixelToMeter = data["simulation"]["dataset"].get("pixelToMeter", 0.05f).asFloat();
    
    // Create enhanced world with dataset mode
    EnhancedCrowdWorld world;
    world.setMode(DATASET_PLAYBACK);
    world.setDatasetParameters(frameRate, pixelToMeter);
    
    if (!world.loadDataset(filename, format)) {
        std::cerr << "Failed to load dataset: " << filename << std::endl;
        return;
    }
    
    Render* r = Render::getInstance();
    while (!r->isInitialized()) {
        mysleep(10);
    }
    
    float deltaT = 1.0f / frameRate;
    bool realtime = data["simulation"]["playback"].get("realtime", true).asBool();
    int sleepTime = realtime ? (int)(deltaT * 1000) : 10;
    
    world.play();
    
    while (world.getIsPlaying()) {
        world.step(deltaT);
        r->update(deltaT);
        mysleep(sleepTime);
        
        if (world.getCurrentFrame() % 50 == 0) {
            std::cout << "Frame " << world.getCurrentFrame() 
                      << " (time: " << world.getCurrentTime() << "s)" << std::endl;
        }
    }
    
    // Export results if configured
    if (data["analysis"].get("exportTrajectories", false).asBool()) {
        std::string outputFile = data["analysis"].get("outputFile", "output.json").asString();
        world.exportTrajectories(outputFile);
        std::cout << "Trajectories exported to: " << outputFile << std::endl;
    }
    
    world.printSimulationStats();
}
