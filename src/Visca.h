#pragma once
#include "ofMain.h"


template <typename T, size_t N>
T* begin(T(&arr)[N]) { return &arr[0]; }
template <typename T, size_t N>
T* end(T(&arr)[N]) { return &arr[0]+N; }


class message {
public:
    vector<int> bytes;
    
   // vector<void (^)(message)> callbacks;
    
    void (^callback)(message, ofSerial*) = ^(message m, ofSerial * serialIn){};
    
    bool error;
};

class Visca {
public:
    void setup();
    void update();
    
    ofSerial serial;
    __block ofSerial serialIn;
    
    queue<message> messageQueue;
    queue<message> statusQueue;
    
    bool waitingForResponse;
    
    message * addMessage(int Count, ... );
    message * addStatusMessage(int Count, ... );
    
    void setZoom(int cam, int zoomLevel, int speed);
    void setAE(int cam, int ae);
    void setIris(int cam, int val);
    void setGain(int cam, int val);
    
    void recallMemory(int cam, int memory);
    
private:
    void messageReceived();
    vector<int> incommingBytes;
    message sendingMessage;

    void relayMessageReceived();
    vector<int> incommingRelayBytes;
    queue<message> relayMessages;
    
    bool serialOK, serialInOK;
    
    int timeout;
    
    bool flip;
    
};