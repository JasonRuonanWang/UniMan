#include<stdio.h>
#include<stdlib.h>
#include<dlfcn.h>
#include"DataManager.h"

DataManager::DataManager()
    :DataMan()
{
}
DataManager::~DataManager()
{
}

int DataManager::init(json p_jmsg){
    return 0;
}

int DataManager::put(const void *p_data,
        string p_doid,
        string p_var,
        string p_dtype,
        vector<size_t> p_putshape,
        vector<size_t> p_varshape,
        vector<size_t> p_offset,
        size_t p_timestep,
        int p_tolerance,
        int p_priority
        ){
    return DataMan::put(
            p_data,
            p_doid,
            p_var,
            p_dtype,
            p_putshape,
            p_varshape,
            p_offset,
            p_timestep,
            p_tolerance,
            p_priority
            );
}

int DataManager::put(const void *p_data, json p_jmsg){

    vector<size_t> p_putshape = p_jmsg["putshape"].get<vector<size_t>>();
    vector<size_t> p_varshape = p_jmsg["varshape"].get<vector<size_t>>();
    vector<size_t> p_offset = p_jmsg["offset"].get<vector<size_t>>();

    if(p_putshape.size()>0){
        if(p_putshape.size()>p_varshape.size()){
            p_varshape.resize(p_putshape.size());
            for (size_t i=0; i<p_putshape.size(); i++){
                p_varshape[i] = p_putshape[i];
            }
        }
        if(p_putshape.size()>p_offset.size()){
            p_offset.resize(p_putshape.size());
            for (size_t i=0; i<p_putshape.size(); i++){
                p_offset[i] = 0;
            }
        }
        put_next(p_data, p_jmsg);
    }
    return 0;
}

void DataManager::add_file(string p_method){
}

void DataManager::add_stream(json p_jmsg){

    string method = "zmq";
    if(p_jmsg["method"] != nullptr) method = p_jmsg["method"];


    if (m_tolerance.size() < m_num_channels){
        for (int i=0; i<m_num_channels; i++){
            m_tolerance.push_back(0);
        }
    }
    if (m_priority.size() < m_num_channels){
        for (int i=0; i<m_num_channels; i++){
            m_priority.push_back(100/(i+1));
        }
    }

    string soname = "lib" + method + "man.so";
    void *so = NULL;
    so = dlopen(soname.c_str(),RTLD_NOW);
    if(so){
        shared_ptr<DataMan> (*get_man)() = NULL;
        get_man = (shared_ptr<DataMan>(*)()) dlsym(so,"getMan");
        if(get_man){
            add_next(get_man());
        }
        else{
            cout << "getMan() not found in " << soname << endl;
        }
    }
    else{
        cout << soname << " not found!" << endl;
    }
}

void DataManager::flush(){
}

int DataManager::get(void *p_data, json &p_jmsg){
    return 0;
}
