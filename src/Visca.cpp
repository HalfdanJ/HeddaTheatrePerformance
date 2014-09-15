#include "Visca.h"


void Visca::setup(){
    serialOK = serial.setup("/dev/tty.usbserial-FTDHHELD", 9600);
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
    
    if(!waitingForResponse ){
        if(messageQueue.size() > 0){
            for(int i=0;i<messageQueue.front().bytes.size();i++){
                serial.writeByte(messageQueue.front().bytes[i]);
            }
            sendingMessage = messageQueue.front();
            messageQueue.pop();
            waitingForResponse = true;
        } else {
            for(int i=0;i<statusQueue.front().bytes.size();i++){
                serial.writeByte(statusQueue.front().bytes[i]);
            }
            sendingMessage = statusQueue.front();
            statusQueue.push(statusQueue.front());
            statusQueue.pop();
            waitingForResponse = true;
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

void Visca::recallMemory(int cam, int memory){
    addMessage(7, 0x81+cam, 0x01, 0x04, 0x3F, 0x02, memory, 0xFF);
}


void Visca::messageReceived(){
    __block message m;

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
    sendingMessage.callback(m);
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