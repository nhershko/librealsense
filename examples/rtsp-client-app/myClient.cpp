#include "IcamOERtsp.h"
#include "camOERtspClient.h"
#include <iostream>
#include <unistd.h>

int main()
{
    IcamOERtsp* camOErtspInstance = camOERTSPClient::getRtspClient("rtsp://10.12.145.82:8554/unicast", "myClient");
    std::vector<rs2_video_stream> myProfiles;
    ((camOERTSPClient*)camOErtspInstance)->initFunc();
    myProfiles = camOErtspInstance->queryStreams();
    std::cout << "Size = " << myProfiles.size() << "\n";
    for (int i =  0; i < myProfiles.size(); i++)
    {
        std::cout << "Profile " << i << ": " << "width = " << myProfiles[i].width << " height = " << myProfiles[i].height << "\n";
    }

    return 0;
}