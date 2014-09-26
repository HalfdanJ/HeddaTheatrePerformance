#include "Visca.h"


void Visca::setup(){
//    serialOK = serial.setup("/dev/tty.usbserial-FTDHHELD", 9600);
    serialOK = serial.setup("/dev/tty.usbserial-A7XGKVKA", 9600);
    
    serialInOK = false;//serialIn.setup("/dev/tty.usbserial-A7XGKVKA", 9600);
}

void Visca::update(){
    while(serialOK && serial.available()){
        int r = serial.readByte();
        incommingBytes.push_back(r);
        
        if(r==0xFF){
            messageReceived();
            incommingBytes.clear();
        }
    }
    
    while(serialInOK && serialIn.available()){
        int r = serialIn.readByte();
        incommingRelayBytes.push_back(r);
        
        if(r==0xFF){
            relayMessageReceived();
            incommingRelayBytes.clear();
        }

        
    }
    
    if(waitingForResponse){
        timeout --;
        if(timeout < 0){
            cout<<"Timeout"<<endl;
            waitingForResponse = false;
        }
    }
    
    if(!waitingForResponse ){
        if(messageQueue.size() > 0){
            cout<<"Send command to cam ";

            for(int i=0;i<messageQueue.front().bytes.size();i++){
                serial.writeByte(messageQueue.front().bytes[i]);
                cout<<messageQueue.front().bytes[i]<<"  ";

            }
            cout<<endl;

            sendingMessage = messageQueue.front();
            messageQueue.pop();
            waitingForResponse = true;
            timeout = 30;
        } else {
            if(flip && relayMessages.size() > 0){
                cout<<"Send relay to cam ";
                for(int i=0;i<relayMessages.front().bytes.size();i++){
                    serial.writeByte(relayMessages.front().bytes[i]);
                    cout<<relayMessages.front().bytes[i]<<"  ";
                }
                cout<<endl;
                sendingMessage = relayMessages.front();
                relayMessages.pop();

                waitingForResponse = true;
                timeout = 30;
                
            } else if(statusQueue.size()>0){
                for(int i=0;i<statusQueue.front().bytes.size();i++){
                    serial.writeByte(statusQueue.front().bytes[i]);
                }
                sendingMessage = statusQueue.front();
                statusQueue.push(statusQueue.front());
                statusQueue.pop();
                waitingForResponse = true;
                timeout = 30;
            }
            flip = !flip;
            
        }
    }
}

void Visca::setZoom(int cam, int zoomLevel, int speed){
    zoomLevel = MIN(zoomLevel, 0x4000);
    int s = zoomLevel & 0x0F;
    int r = (zoomLevel >> 4) & 0x0F;
    int q = (zoomLevel >> 8) & 0x0F;
    int p = (zoomLevel >> 12) & 0x0F;
    addMessage(11, 0x81+cam, 0x01, 0x7E, 0x01, 0x4A, speed, p, q, r, s, 0xFF);
}

void Visca::setAE(int cam, int ae){
    addMessage(7, 0x81+cam, 0x01, 0x04, 0x39, ae?0x00:0x03, 0xFF);
}
void Visca::setIris(int cam, int val){
    
    int s = val & 0x0F;
    int r = (val >> 4) & 0x0F;
    int q = (val >> 8) & 0x0F;
    int p = (val >> 12) & 0x0F;
    
    addMessage(11, 0x81+cam, 0x01, 0x04, 0x4B, 0x00, 0x00,r,s, 0xFF);
    
}

void Visca::setGain(int cam, int val){
    
    int s = val & 0x0F;
    int r = (val >> 4) & 0x0F;
    int q = (val >> 8) & 0x0F;
    int p = (val >> 12) & 0x0F;
    
    addMessage(11, 0x81+cam, 0x01, 0x04, 0x4C, 0x00, 0x00,r,s, 0xFF);
    
}


void Visca::recallMemory(int cam, int memory){
    addMessage(7, 0x81+cam, 0x01, 0x04, 0x3F, 0x02, memory, 0xFF);
}


void Visca::messageReceived(){
    __block message m;
    cout<<"MESSAGE RECEIVED: ";

    for(int i=0;i<incommingBytes.size();i++){
        cout<<hex<<uppercase<<incommingBytes[i]<<"  ";
        m.bytes.push_back(incommingBytes[i]);
    }
    cout<<endl;
    waitingForResponse = false;
    
    //cout<<sendingMessage->callbacks.size()<<endl;
/*    for(int i=0;i<sendingMessage->callbacks.size();i++){
        sendingMessage->callbacks[i](m);
    }*/
    
    if(incommingBytes[1] == 0x62 && incommingBytes[2] == 0x41){
        m.error = true;
    }
    sendingMessage.callback(m, &serialIn);
}


void Visca::relayMessageReceived(){
    __block message m;
    
    for(int i=0;i<incommingRelayBytes.size();i++){
        cout<<hex<<uppercase<<incommingRelayBytes[i]<<"  ";
        m.bytes.push_back(incommingRelayBytes[i]);
    }
    cout<<"  relayMessageReceived"<<endl;
    m.callback =(^(message m, ofSerial * s){
        cout<<"##############################"<<endl<<"Send response to relay: ";
        
        for(int i=0;i<m.bytes.size();i++){
            cout<<hex<<uppercase<<m.bytes[i]<<"  ";
            s->writeByte(m.bytes[i]);
        }
    });

    relayMessages.push(m);
    //sendingMessage.callback(m);
}



message* Visca::addMessage(int Count, ... ){
    va_list Numbers;
    va_start(Numbers, Count);
    
    message m;
    for(int i = 0; i < Count; ++i )
        m.bytes.push_back(va_arg(Numbers, int));
    va_end(Numbers);
    
    messageQueue.push(m);
    return &messageQueue.back();
}



message* Visca::addStatusMessage(int Count, ... ){
    va_list Numbers;
    va_start(Numbers, Count);
    
    message m;
    for(int i = 0; i < Count; ++i )
        m.bytes.push_back(va_arg(Numbers, int));
    va_end(Numbers);
    
    statusQueue.push(m);
 
    return &statusQueue.back();
}