#ifndef DATAMANAGER_H_
#define DATAMANAGER_H_

#include"DataMan.h"


class DataManager: public DataMan{
    public:
        DataManager();
        ~DataManager();

        virtual int put(const void *p_data, json p_jmsg);

        int get(void *p_data,
                string p_doid,
                string p_var,
                string p_dtype,
                vector<size_t> p_getshape,
                vector<size_t> p_varshape,
                vector<size_t> p_offset,
                size_t p_timestep
                );

        int get(void *p_data,
                string p_doid,
                string p_var,
                string &p_dtype,
                vector<size_t> &p_varshape,
                size_t &p_timestep
                );
        void flush();

        void add_stream(
                string p_local_ip,
                string p_remote_ip,
                int p_local_port,
                int p_remote_port,
                int p_num_channels = 1,
                string p_method = "zmq",
                vector<int> p_tolerance = vector<int>(),
                vector<int> p_priority = vector<int>()
                );

        int put(const void *p_data,
                string p_doid,
                string p_var,
                string p_dtype,
                vector<size_t> p_putshape,
                vector<size_t> p_varshape,
                vector<size_t> p_offset,
                size_t p_timestep,
                int p_tolerance=0,
                int p_priority=100
                );

        void add_file(string p_method);

        string name(){return "DataManager";}

    private:

        string m_local_ip="";
        string m_remote_ip="";
        int m_local_port=0;
        int m_remote_port=0;
        int m_num_channels=0;
        vector<int> m_tolerance;
        vector<int> m_priority;


};

#endif
