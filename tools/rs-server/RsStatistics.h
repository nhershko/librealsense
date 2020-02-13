#ifndef _RS_STATISTICS_HH
#define _RS_STATISTICS_HH

#include <chrono>
 
class RsStatistics
{
public:
 static std::chrono::high_resolution_clock::time_point& get_TPresetPacketStart()
        {
            static std::chrono::high_resolution_clock::time_point TPresetPacketStart = std::chrono::high_resolution_clock::now();
            return TPresetPacketStart;
        }
        static std::chrono::high_resolution_clock::time_point& get_TPfirstPacket()
        {
            static std::chrono::high_resolution_clock::time_point TPfirstPacket = std::chrono::high_resolution_clock::now();
            return TPfirstPacket;
        }
        static std::chrono::high_resolution_clock::time_point& get_TPsendPacket()
        {
            static std::chrono::high_resolution_clock::time_point TPsendPacket = std::chrono::high_resolution_clock::now();
            return TPsendPacket;
        }
        static std::chrono::high_resolution_clock::time_point& get_TPschedule()
        {
            static std::chrono::high_resolution_clock::time_point TPschedule = std::chrono::high_resolution_clock::now();
            return TPschedule;
        }
        static double& get_prev_diff()
        {
            static double prevDiff = 0;
            return prevDiff;
        }
        static double isJump()
        {
            double* prevD = &get_prev_diff();
            //std::chrono::duration<double> diff = std::chrono::high_resolution_clock::now() - RsStatistics::get_TPsendPacket();
            double diff = 1000*std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - RsStatistics::get_TPsendPacket()).count();
            //double ddiff= diff.count()*1000; 
            
            double diffOfDiff = diff - *prevD;
           // printf("isJump from sendtime %f, diffOfDiff is %f\n", diff,diffOfDiff);
            if (diffOfDiff > 5)
            {
              *prevD = diff;
              return diffOfDiff;
            }
            else
            {
                *prevD = diff;
                return 0;
            }
        }
 


private:
//static std::chrono::high_resolution_clock::time_point TPresetPacketStart;
};

#endif
