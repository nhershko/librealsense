#include "IcamOERtsp.h"
#include "camOERtspClient.h"
#include "rtp_callback.hh"
#include <iostream>
#include <unistd.h>

FILE* myFile;

class my_callback : public rtp_callback
{
private:

    std::string app_name;
    /* data */
public:
    my_callback(std::string app_name)
    {
        this->app_name = app_name;
    };
    
    ~my_callback();

    void on_frame(unsigned char*buffer,ssize_t size, struct timeval presentationTime)
    {
        printf("[%s] %ld.%06ld got frame. data size: %zu \n",this->app_name.c_str(), presentationTime.tv_sec, presentationTime.tv_usec, size);
    }
};



int main()
{

    myFile = fopen("myFile.bin", "ab");

    int res = 0;
    IcamOERtsp* camOErtspInstance = camOERTSPClient::getRtspClient("rtsp://10.12.144.74:8554/depth", "myClient");
    //IcamOERtsp* camOErtspInstance = camOERTSPClient::getRtspClient("rtsp://10.12.144.35:8554/unicast", "myClient");
    std::vector<rs2_video_stream> myProfiles;
    ((camOERTSPClient*)camOErtspInstance)->initFunc();
    myProfiles = camOErtspInstance->queryStreams();
    std::cout << "Size = " << myProfiles.size() << "\n";
    for (int i =  0; i < myProfiles.size(); i++)
    {
        std::cout << "Profile " << i << ": " << "width = " << myProfiles[i].width << " height = " << myProfiles[i].height << " sensor id = " << myProfiles[i].type <<" UID = " << myProfiles[i].uid << "\n";
    }


    IcamOERtsp* camOErtspInstance2 = camOERTSPClient::getRtspClient("rtsp://10.12.144.74:8554/color", "myClient");
    //IcamOERtsp* camOErtspInstance2 = camOERTSPClient::getRtspClient("rtsp://10.12.144.35:8554/unicast", "myClient");
    std::vector<rs2_video_stream> myProfiles2;
    //((camOERTSPClient*)camOErtspInstance2)->initFunc();
   // myProfiles2 = camOErtspInstance2->queryStreams();
    //std::cout << "Size = " << myProfiles2.size() << "\n";
    //for (int i =  0; i < myProfiles2.size(); i++)
    //{
    //    std::cout << "Profile " << i << ": " << "width = " << myProfiles2[i].width << " height = " << myProfiles2[i].height << " sensor id = " << myProfiles2[i].type << " UID = " << myProfiles2[i].uid << "\n";
    //}

    camOErtspInstance2->addStream(myProfiles2[0],new my_callback("mycolor"));
    camOErtspInstance2->start();
    sleep(3);
    camOErtspInstance2->stop();
    camOErtspInstance2->close();

    std::cout << "\nwait 3 secs\n";
    sleep(3);

    camOErtspInstance2 = camOERTSPClient::getRtspClient("rtsp://10.12.144.74:8554/color", "myClient");
    ((camOERTSPClient*)camOErtspInstance2)->initFunc();
    myProfiles2 = camOErtspInstance2->queryStreams();
    camOErtspInstance2->addStream(myProfiles2[0],new my_callback("mycolor"));
    camOErtspInstance2->start();
    sleep(3);

    camOErtspInstance2->stop();
    camOErtspInstance2->close();
    sleep(3);

    //res = camOErtspInstance->addStream(myProfiles[0], new my_callback("myclient"));
    std::cout << "After setup. res = " << res << "\n";
    //res = camOErtspInstance->start();
    std::cout << "After start. res = " << res << "\n";
    
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
