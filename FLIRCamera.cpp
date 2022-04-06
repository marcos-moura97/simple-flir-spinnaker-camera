#include "FLIRCamera.h"
#include <iostream>

#include <chrono>
#include <iostream>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <bits/stdc++.h>

bool AcessFolder(const char* imageFolderPath) {
    // check if the folder exists
    struct stat info;

    if (stat(imageFolderPath, &info) != 0) {
        std::cout << imageFolderPath << " does not exists. Creatind folder." << std::endl;
        if (mkdir(imageFolderPath, 0777) == 1) {
            std::cout << "Error: " << strerror(errno) << std::endl;
            return false;
        }
        else {
            return true;
        }
    }

    else if (info.st_mode & S_IFDIR) {
        return true;
    }

    else {
        std::cout << "Error on acess " << imageFolderPath << std::endl;
        return false;
    }

}

FLIRCamera::FLIRCamera(){
    mSystem = Spinnaker::System::GetInstance();
    mInitialized = false;
}

FLIRCamera::~FLIRCamera(){
    if(mInitialized){
        mCamera->DeInit();
        mCamera = nullptr;
        mInitialized = false;
    }
    mSystem->ReleaseInstance();
    std::cout << "FLIR camera deinitialized" << std::endl;
}

bool FLIRCamera::Initialize(){
    std::cout << "FLIR camera initialization" << std::endl;
    const Spinnaker::LibraryVersion spinnakerVersion = mSystem->GetLibraryVersion();
    std::cout << "Spinnaker SDK version: " << spinnakerVersion.major << "." << spinnakerVersion.minor << "." << spinnakerVersion.type << "." << spinnakerVersion.build << std::endl;
    Spinnaker::CameraList cameraList = mSystem->GetCameras();
    if(cameraList.GetSize() == 0){
        std::cout << "FLIR camera not found" << std::endl;
        cameraList.Clear();
        return false;
    }
    std::cout << "Discovered " << cameraList.GetSize() << " FLIR cameras, using #0" << std::endl;
    mCamera = cameraList.GetByIndex(0);
    cameraList.Clear();
    mTLDevice = &(mCamera->GetTLDeviceNodeMap());
    mCamera->Init();
    mNodeMap = &(mCamera->GetNodeMap());
    mInitialized = true;
    std::cout << "Camera #0 initialized" << std::endl;
    std::cout << "Resolution: " << mCamera->Width.GetValue() << "x" << mCamera->Height.GetValue() << std::endl;
    std::cout << "Exposure time: " << mCamera->ExposureTime.GetValue() / 1000.0 << " ms" << std::endl;
    std::cout << "Gain: " << mCamera->Gain.GetValue() << std::endl;
    std::cout << "Pixel format: " << mCamera->PixelFormat.GetCurrentEntry()->GetSymbolic() << std::endl;
    return true;
}

bool FLIRCamera::BeginAcquisition(){
    if(!mInitialized){
        std::cout << "Attempt to begin acquisition before initialization" << std::endl;
        return false;
    }
    Spinnaker::GenApi::CEnumerationPtr pAcquisitionMode = mNodeMap->GetNode("AcquisitionMode");
    if(!Spinnaker::GenApi::IsAvailable(pAcquisitionMode) || !Spinnaker::GenApi::IsWritable(pAcquisitionMode)){
        std::cout << "Acquisition mode unavailable or not writable" << std::endl;
        return false;
    }
    Spinnaker::GenApi::CEnumEntryPtr pAcquisitionModeContinuous = pAcquisitionMode->GetEntryByName("Continuous");
    if(!Spinnaker::GenApi::IsAvailable(pAcquisitionModeContinuous) || !Spinnaker::GenApi::IsReadable(pAcquisitionModeContinuous)){
        std::cout << "Continuous acqusition mode entry unavailable or not readable" << std::endl;
        return false;
    }
    const uint64_t acquisitionModeContinuous = pAcquisitionModeContinuous->GetValue();
    pAcquisitionMode->SetIntValue(acquisitionModeContinuous);
    std::cout << "Acquisition mode set to continuous" << std::endl;
    mCamera->BeginAcquisition();
    std::cout << "Acquisition begin" << std::endl;
    return true;
}

bool FLIRCamera::EndAcquisition() {
    if(!mInitialized){
        std::cout << "Attempt to end acquisition before initialization" << std::endl;
        return false;
    }
    mCamera->EndAcquisition();
    return true;
}

bool FLIRCamera::RetrieveImage(const char* imageFolderPath) {
    if (!AcessFolder(imageFolderPath)) {
        return false;
    }
    Spinnaker::ImagePtr pResultImage = mCamera->GetNextImage();
    if(pResultImage->IsIncomplete()){
        std::cout << "Image incomplete: " << Spinnaker::Image::GetImageStatusDescription(pResultImage->GetImageStatus()) << std::endl;
        return false;
    }
    const size_t width = pResultImage->GetWidth();
    const size_t height = pResultImage->GetHeight();
    Spinnaker::ImagePtr pConvertedImage = pResultImage->Convert(Spinnaker::PixelFormatEnums::PixelFormat_BGR8, Spinnaker::ColorProcessingAlgorithm::HQ_LINEAR);

    std::ostringstream filename;
    filename << imageFolderPath << "Acquisition-" << mCamera->ExposureTime.GetValue() << "-" << mCamera->Gain.GetValue() << "-";
    // get timestamp
    auto timeNow = std::chrono::system_clock::now();
    filename << std::chrono::duration_cast<std::chrono::seconds>(
        timeNow.time_since_epoch()).count() << ".jpg";

    pConvertedImage->Save(filename.str().c_str());
    std::cout << "Image saved at " << filename.str() << std::endl;
    pResultImage->Release();
    return true;
}
