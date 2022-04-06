#pragma once
#ifndef FLIRCAMERA_H_
#define FLIRCAMERA_H_

#include <Spinnaker.h>
#include <SpinGenApi/SpinnakerGenApi.h>

class FLIRCamera {
    private:
        bool mInitialized;
        Spinnaker::SystemPtr mSystem;
        Spinnaker::CameraPtr mCamera;
        Spinnaker::GenApi::INodeMap *mTLDevice;
        Spinnaker::GenApi::INodeMap *mNodeMap;
    public:
        FLIRCamera();
        ~FLIRCamera();
        bool Initialize();
        bool BeginAcquisition();
        bool EndAcquisition();
        bool RetrieveImage(const char* imageFolderPath);
};

#endif // FLIRCAMERA_H_
