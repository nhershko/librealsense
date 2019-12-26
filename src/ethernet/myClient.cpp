#include "IcamOERtsp.h"
#include "camOERtspClient.h"
#include <iostream>
#include <unistd.h>

int main()
{
    // camOErtspWrapper rtspWrapper("rtsp://10.12.145.82:8554/testStream");
    // std::vector<Profile> myProfiles;
    // myProfiles = rtspWrapper.queryProfiles();
    // //std::cout << "Size: " << myProfiles.size();
    IcamOERtsp* camOErtspInstance = camOERTSPClient::getRtspClient("rtsp://10.12.145.82:8554/testStream", "myClient");
    std::vector<rs2_video_stream> myProfiles;
    ((camOERTSPClient*)camOErtspInstance)->initFunc();
    myProfiles = camOErtspInstance->queryProfiles();
    std::cout << "Size = " << myProfiles.size() << "\n";
    for (int i =  0; i < myProfiles.size(); i++)
    {
    std::cout << "Profile " << i << ": " << "width = " << myProfiles[i].width << " height = " << myProfiles[i].height << "\n";
    }
    std::cout << "After!!!\n";

    /*IcamOERtsp* camOErtspInstance2 = camOERTSPClient::getRtspClient("rtsp://10.12.145.82:8554/testStream2", "myClient");
    std::vector<Profile> myProfiles2;
    myProfiles2 = camOErtspInstance2->queryProfiles();
    std::cout << "After2222!!!\n";*/

    return 0;
}