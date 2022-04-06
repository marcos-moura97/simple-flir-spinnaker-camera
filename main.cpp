#include "FLIRCamera.h"

#include <unistd.h>

void delay_s(unsigned s) {
    sleep(s);
}

int main() {
    FLIRCamera camera;
    if(!camera.Initialize()) return 1; 
    if(!camera.BeginAcquisition()) return 1;

    while(camera.RetrieveImage("../Images/")) 
        delay_s(2);
    
    camera.EndAcquisition();
    return 0;
}
