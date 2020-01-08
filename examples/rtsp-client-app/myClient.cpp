#include "IcamOERtsp.h"
#include "camOERtspClient.h"
#include <iostream>
#include <unistd.h>

int main()
{
    IcamOERtsp* camOErtspInstance = camOERTSPClient::getRtspClient("rtsp://10.12.144.74:8554/depth", "myClient");
    std::vector<rs2_video_stream> myProfiles;
    ((camOERTSPClient*)camOErtspInstance)->initFunc();
    myProfiles = camOErtspInstance->queryStreams();
    std::cout << "Size = " << myProfiles.size() << "\n";
    for (int i =  0; i < myProfiles.size(); i++)
    {
        std::cout << "Profile " << i << ": " << "width = " << myProfiles[i].width << " height = " << myProfiles[i].height << " sensor id = " << myProfiles[i].type << "\n";
    }


    IcamOERtsp* camOErtspInstance2 = camOERTSPClient::getRtspClient("rtsp://10.12.145.82:8554/color", "myClient");
    //IcamOERtsp* camOErtspInstance2 = camOERTSPClient::getRtspClient("rtsp://10.12.144.35:8554/unicast", "myClient");
    std::vector<rs2_video_stream> myProfiles2;
    ((camOERTSPClient*)camOErtspInstance2)->initFunc();
    myProfiles2 = camOErtspInstance2->queryStreams();
    std::cout << "Size = " << myProfiles2.size() << "\n";
    for (int i =  0; i < myProfiles2.size(); i++)
    {
        std::cout << "Profile " << i << ": " << "width = " << myProfiles2[i].width << " height = " << myProfiles2[i].height << " sensor id = " << myProfiles2[i].type <<"\n";
    }


    int res = camOErtspInstance->addStream(myProfiles[0]);
    std::cout << "After setup. res = " << res << "\n";
    camOErtspInstance->start();
     std::cout << "After play.\n";

    return 0;
}