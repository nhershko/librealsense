#include "IcamOERtsp.h"
#include "camOERtspClient.h"
#include <iostream>
#include <unistd.h>

FILE* myFile;

void myFrameCallBack(u_int8_t* buf, unsigned int size, struct timeval presentationTime)
{
    std::cout << "myFrameCallBack. size = " << size << " time (sec) = " << presentationTime.tv_sec << "\n";
    fwrite(buf, size, 1, myFile);
}

int main()
{
    const char* fileName = "myFile.bin";
    if (remove(fileName) ==  0)
    {
        printf("File removed\n");
    }
    else
    {
        printf("Cannot remove file\n");
    }
    myFile = fopen(fileName, "ab");

    int res = 0;
    IcamOERtsp* camOErtspInstance = camOERTSPClient::getRtspClient("rtsp://10.12.145.82:8554/depth", "myClient");
    //IcamOERtsp* camOErtspInstance = camOERTSPClient::getRtspClient("rtsp://10.12.144.35:8554/unicast", "myClient");
    //IcamOERtsp* camOErtspInstance = camOERTSPClient::getRtspClient("rtsp://10.12.144.74:8554/depth", "myClient");

    std::vector<rs2_video_stream> myProfiles;
    ((camOERTSPClient*)camOErtspInstance)->initFunc();
    myProfiles = camOErtspInstance->queryStreams();
    std::cout << "Size = " << myProfiles.size() << "\n";
    for (int i =  0; i < myProfiles.size(); i++)
    {
        std::cout << "Profile " << i << ": " << "width = " << myProfiles[i].width << " height = " << myProfiles[i].height << " sensor id = " << myProfiles[i].type <<" UID = " << myProfiles[i].uid << "\n";
    }


    //IcamOERtsp* camOErtspInstance2 = camOERTSPClient::getRtspClient("rtsp://10.12.145.82:8554/color", "myClient");
    //IcamOERtsp* camOErtspInstance2 = camOERTSPClient::getRtspClient("rtsp://10.12.144.35:8554/unicast", "myClient");
    std::vector<rs2_video_stream> myProfiles2;
    //((camOERTSPClient*)camOErtspInstance2)->initFunc();
   // myProfiles2 = camOErtspInstance2->queryStreams();
    //std::cout << "Size = " << myProfiles2.size() << "\n";
    //for (int i =  0; i < myProfiles2.size(); i++)
    //{
    //    std::cout << "Profile " << i << ": " << "width = " << myProfiles2[i].width << " height = " << myProfiles2[i].height << " sensor id = " << myProfiles2[i].type << " UID = " << myProfiles2[i].uid << "\n";
    //}

    res = camOErtspInstance->addStream(myProfiles[0], &myFrameCallBack);
    std::cout << "After setup. res = " << res << "\n";
    //res = camOErtspInstance->start();
    //std::cout << "After start. res = " << res << "\n";
    
    //res = camOErtspInstance2->addStream(myProfiles2[0], &myFrameCallBack);
    //std::cout << "After setup. res = " << res << "\n";
    //res = camOErtspInstance->stop(myProfiles[0]);
    //std::cout << "After stop. res = " << res << "\n";
    //res = camOErtspInstance2->start();
    //std::cout << "After start. res = " << res << "\n";
    sleep(5);
    
   // res = camOErtspInstance->stop();
   // std::cout << "After stop. res = " << res << "\n";
    //res = camOErtspInstance->stop(myProfiles[0]);
    //std::cout << "After stop. res = " << res << "\n";  
   // res = camOErtspInstance->close();
   // std::cout << "After close. res = " << res << "\n";
    fclose(myFile);

//  camOErtspInstance = camOERTSPClient::getRtspClient("rtsp://10.12.145.82:8554/depth", "myClient");
 //((camOERTSPClient*)camOErtspInstance)->initFunc();
 //    myProfiles = camOErtspInstance->queryStreams();
//     res = camOErtspInstance->addStream(myProfiles[0]);
//     std::cout << "After setup. res = " << res << "\n";
//     res = camOErtspInstance->start();
//     std::cout << "After start. res = " << res << "\n";

    return 0;
}
