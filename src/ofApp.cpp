#include "ofApp.h"

float ease(float t, float b, float c, float d) {
	/*t /= d/2.;
	if (t < 1) return c/2.*t*t + b;
	t--;
	return -c/2. * (t*(t-2.) - 1.) + b;*/
    
    return -c/2. * (cosf(PI*t/d) - 1) + b;
};

ptzSettings settings[2];

//--------------------------------------------------------------
void ofApp::setup(){
	midiOut.listPorts(); // via instance
	midiOut.openPort(0); // by number
    ofSetFrameRate(30);
    
    for(int i=0;i<7;i++){
        camera newCam;
        newCam.input = i+1;
        if(i==0){
            newCam.pos = ofVec2f(0.9,0.9);
            newCam.color = ofColor(255,0,0);
            newCam.name = "DSF";
            cameras.push_back(newCam);
        }
        if(i==1){
            newCam.pos = ofVec2f(0.1,0.5);
            newCam.color = ofColor(255,0,0);
            newCam.name = "KS";
            cameras.push_back(newCam);
        }
        if(i==2){
            newCam.pos = ofVec2f(0.9,0.1);
            newCam.color = ofColor(255,0,0);
            newCam.name = "DSB";
            cameras.push_back(newCam);
        }
        if(i==3){
            newCam.pos = ofVec2f(0.3,0.3);
            newCam.color = ofColor(0,255,255);
            newCam.name = "";
            cameras.push_back(newCam);
        }
        if(i==4){
            newCam.pos = ofVec2f(0.5,0.5);
            newCam.color = ofColor(0,255,0);
            newCam.name = "PTZ1";
            cameras.push_back(newCam);
        }
        if(i==5){
            newCam.pos = ofVec2f(0.3,0.8);
            newCam.color = ofColor(0,255,0);
            newCam.name = "PTZ2";
            cameras.push_back(newCam);
        }
        if(i==6){
            newCam.pos = ofVec2f(0.0,0.0);
            newCam.color = ofColor(0,255,255);
            newCam.name = "KLAVER";
            cameras.push_back(newCam);
        }
    }
    
    ofBackground(0,0,0);
    
    font.loadFont("verdana.ttf", 20);
    fontS.loadFont("verdana.ttf", 12);
    
    oscReceiver.setup(3333);
    
    ptz.setup();
    
    ptz.addStatusMessage(5, 0x81, 0x09, 0x04, 0x00, 0xFF);
    
    //Zoom Inq
    for(int c=0;c<2;c++){
        ptz.addStatusMessage(5, 0x81+c, 0x09, 0x04, 0x47, 0xFF)->callback =(^(message m){
            if(!m.error){
                int cam = (m.bytes[0] - 0x90) >> 4;
                //cout<<"CALLBACK"<<m.bytes.size()<<"  "<<cam<<endl;
                
                settings[cam].zoomLevel = (int)m.bytes[2];
                settings[cam].zoomLevel = settings[cam].zoomLevel<<4;
                
                settings[cam].zoomLevel += (int)m.bytes[3];
                settings[cam].zoomLevel = settings[cam].zoomLevel<<4;
                
                settings[cam].zoomLevel += (int)m.bytes[4];
                settings[cam].zoomLevel = settings[cam].zoomLevel<<4;
                
                settings[cam].zoomLevel += (int)m.bytes[5];
                //            cout<<dec<<cam<<"  "<<settings[cam].zoomLevel<<endl;
            }
        });
        
        ptz.addStatusMessage(5, 0x81+c, 0x09, 0x04, 0x39, 0xFF)->callback =(^(message m){
            if(!m.error){
                
                int cam = (m.bytes[0] - 0x90) >> 4;
                
                if(m.bytes[2] == 0x00){
                    settings[cam].ae = true;
                } else {
                    settings[cam].ae = false;
                }
            }
        });
        
        ptz.addStatusMessage(5, 0x81+c, 0x09, 0x04, 0x4A, 0xFF)->callback =(^(message m){
            if(!m.error){
                
                int cam = (m.bytes[0] - 0x90) >> 4;
                settings[cam].shutter = (int)m.bytes[4];
                settings[cam].shutter = settings[cam].shutter<<4;
                
                settings[cam].shutter += (int)m.bytes[5];
            }
        });
        
        ptz.addStatusMessage(5, 0x81+c, 0x09, 0x04, 0x4B, 0xFF)->callback =(^(message m){
            if(!m.error){
                int cam = (m.bytes[0] - 0x90) >> 4;
                settings[cam].iris = (int)m.bytes[4];
                settings[cam].iris = settings[cam].iris<<4;
                
                settings[cam].iris += (int)m.bytes[5];
            }
        });
        
        ptz.addStatusMessage(5, 0x81+c, 0x09, 0x04, 0x4C, 0xFF)->callback =(^(message m){
            if(!m.error){
                int cam = (m.bytes[0] - 0x90) >> 4;
                settings[cam].gain = (int)m.bytes[4];
                settings[cam].gain = settings[cam].gain<<4;
                
                settings[cam].gain += (int)m.bytes[5];
            }
        });

    }
   ;
}

//--------------------------------------------------------------
void ofApp::update(){
    
    while(oscReceiver.hasWaitingMessages()){
        ofxOscMessage m;
        oscReceiver.getNextMessage(&m);
        if(m.getAddress() == "/ptz/memory"){
            ptz.recallMemory(m.getArgAsInt32(0)-1, m.getArgAsInt32(1)-1);
        }
        if(m.getAddress() == "/cut"){
            int time = 0;
            if(m.getNumArgs() >= 2){
                time = m.getArgAsFloat(1);
            }
            
            if(m.getArgTypeName(0) == "string"){
                fadeTo(inputFromName(m.getArgAsString(0)), time);

            } else {
                fadeTo(m.getArgAsInt32(0), time);
            }
        }
        if(m.getAddress() == "/preview"){
            if(m.getArgTypeName(0) == "string"){
                preview( inputFromName(m.getArgAsString(0)));
            } else {
                preview(m.getArgAsInt32(0)-1);
            }
        }
    }
           
    if(currentFade < goalFade){
        currentFade += 1.0/fadeDuration * 1.0/ofGetFrameRate();
        if(currentFade > goalFade){
            currentFade = goalFade;
        }
        setFade(ease(currentFade, 0, 1, 1));
    }
    if(currentFade > goalFade){
        currentFade -=  1.0/fadeDuration *  1.0/ofGetFrameRate();
        if(currentFade < goalFade){
            currentFade = goalFade;
        }
        setFade(ease(currentFade, 0, 1, 1));
    }
    
    ptz.update();
    
    

}

void ofApp::cutTo(int input){
    setDeck(currentDeck, input);
    setFade(currentFade);
}

void ofApp::preview(int input){
    midiOut.sendControlChange(1, 4, input);
}

void ofApp::setDeck(int deck, int input){
    midiOut.sendControlChange(1, 1+deck,input-1);
    currentInput[deck] = input;
}

void ofApp::fadeTo(int input, float duration){
    if(duration == 0){
        cutTo(input);
        return;
    }
    
    setDeck(!currentDeck, input);
        


    if(currentDeck){
        goalFade = 0.0;
        currentFade = 1.0;
    } else {
        goalFade = 1.0;
        currentFade = 0.0;
    }
    setFade(currentFade);
    fadeDuration = duration;
    currentDeck = !currentDeck;
}

void ofApp::setFade(float fade){
    midiOut.sendControlChange(1, 3,fade*127);
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofPushMatrix();
    ofVec2f scale = ofVec2f(ofGetWidth(), ofGetHeight());
    
    for(int i=0;i<cameras.size();i++){
        if(currentInput[currentDeck] == cameras[i].input){
            ofSetColor(255, 255, 0);
            ofCircle(scale.x*cameras[i].pos.x, scale.y*cameras[i].pos.y, 25*fabs(sin(ofGetElapsedTimeMillis()/200.)));
        }
        ofSetColor(cameras[i].color);
        ofCircle(scale.x*cameras[i].pos.x, scale.y*cameras[i].pos.y, 20);
        ofSetColor(255);
        font.drawString(ofToString(cameras[i].input), scale.x*cameras[i].pos.x+24, scale.y*cameras[i].pos.y+5);
    }
    ofPopMatrix();
    
    
    for(int i=0;i<2;i++){
        ofPushMatrix();
        ofTranslate(300*i, 20);
        fontS.drawString("Zoom "+ofToString(settings[i].zoomLevel), 0, 0);
        ofTranslate(0, 15);
        fontS.drawString("AE  "+ofToString(settings[i].ae), 0, 0);
        ofTranslate(0, 15);
        fontS.drawString("Shutter "+ofToString(settings[i].shutter), 0, 0);
        ofTranslate(0, 15);
        fontS.drawString("Iris "+ofToString(settings[i].iris), 0, 0);
        ofTranslate(0, 15);
        fontS.drawString("Gain "+ofToString(settings[i].gain), 0, 0);
        ofTranslate(0, 15);
        ofPopMatrix();
    }
}

int ofApp::inputFromName(string name){
    for(int i=0;i<cameras.size();i++){
        std::transform(name.begin(), name.end(),name.begin(), ::toupper);

        
        if(cameras[i].name == name){
            return cameras[i].input;
        }
    }
}

//--------------------------------------------------------------
void ofApp::exit() {
	midiOut.closePort();
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    switch(key){
        case '1':
            cutTo(1);
            break;
        case '2':
            cutTo(2);
            break;
        case '3':
            cutTo(3);
            break;
        case '4':
            cutTo(4);
            break;
        case '5':
            cutTo(5);
            break;
        case '6':
            cutTo(6);
            break;
        case '7':
            cutTo(7);
            break;
            
        case 'q':
            fadeTo(1);
            break;
        case 'w':
            fadeTo(2);
            break;
        case 'e':
            fadeTo(3);
            break;
        case 'r':
            fadeTo(4);
            break;
        case 't':
            fadeTo(5);
            break;
        case 'y':
            fadeTo(6);
            break;
        case 'u':
            fadeTo(7);
            break;
            
        case 'a':
            ptz.recallMemory(1, 0);
            break;
        case 's':
            ptz.recallMemory(1, 1);
            break;
        case 'd':
            ptz.recallMemory(1, 2);
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    ptz.setZoom(0, x*10, 7);

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
