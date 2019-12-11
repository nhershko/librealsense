
enum stream_type_id
{
    STREAM_DEPTH,
    STREAM_COLOR,
};

struct profile_resolution
{ 
     int width; 
     int hight; 
};

class camOE_stream_profile
{
    public:
        camOE_stream_profile(stream_type_id id, profile_resolution res, int fps)
        :m_stream_sensor(id),
        m_profile_res(res),
        m_stream_fps(fps){}

        //in case we want to have self-contained sdp parser
        camOE_stream_profile(char* sdp_line);

        int width(){    return m_profile_res.width;  }
        int hight(){    return m_profile_res.hight;  }
        int fps(){  return m_stream_fps;  }
        stream_type_id stream_sensor(){ return m_stream_sensor; }

    private:
        stream_type_id m_stream_sensor;
        profile_resolution m_profile_res;
        int m_stream_fps;

};