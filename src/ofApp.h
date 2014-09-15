#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "Visca.h"
#include "ofxOsc.h"

struct camera {
    ofVec2f pos;
    ofColor color;
    int input;
    string name;
};

struct ptzSettings {
    int zoomLevel;
    int iris;
    int gain;
    int shutter;
    bool ae;
};

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    void exit();
    
    ofxMidiOut midiOut;
    ofTrueTypeFont font, fontS;
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    void cutTo(int input);
    void fadeTo(int input, float duration = 1.0);
    void setDeck(int deck, int input);
    void setFade(float fade);
    void preview(int input);
    
    int currentDeck;
    float currentFade, goalFade;
    int currentInput[2];
    float fadeDuration;
    
    int inputFromName(string name);
    
    vector<camera> cameras;
    
    Visca ptz;
    
    ofxOscReceiver oscReceiver;
    
    
    
};
