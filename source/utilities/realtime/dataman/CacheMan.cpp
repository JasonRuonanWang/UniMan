#include "utilities/realtime/dataman/CacheMan.h"

namespace adios
{
namespace realtime
{

int CacheMan::init(json p_jmsg){
    return 0;
}

int CacheItem::init(json p_jmsg){
    m_doid = p_jmsg["doid"];
    m_var = p_jmsg["var"];
    m_dtype = p_jmsg["dtype"];
    m_varshape = p_jmsg["varshape"].get<vector<size_t>>();
    m_bytes = dsize(m_dtype);
    m_varsize = product(m_varshape);
    m_varbytes = m_varsize * m_bytes;

    if(m_buffer[m_timestep].size() != m_varbytes){
        m_buffer[m_timestep].resize(m_varbytes);
    }
    return 0;
}

int CacheItem::put(const void *p_data, json p_jmsg){
    if(!check_json(p_jmsg, {"doid", "var", "dtype", "varshape", "putshape", "offset"}, "CacheItem")){
        return -1;
    }
    init(p_jmsg);
    vector<size_t> p_putshape = p_jmsg["putshape"].get<vector<size_t>>();
    vector<size_t> p_varshape = p_jmsg["varshape"].get<vector<size_t>>();
    vector<size_t> p_offset = p_jmsg["offset"].get<vector<size_t>>();

    size_t putsize = product(p_putshape);
    size_t chunksize = p_putshape.back();
    for(size_t i=0; i<putsize; i+=chunksize){
        vector<size_t> p = one2multi(p_putshape, i);
        p = apply_offset(p, p_offset);
        size_t ig = multi2one(p_varshape, p);
        copy((char*)p_data + i * m_bytes, (char*)p_data + i * m_bytes + chunksize * m_bytes,  m_buffer[m_timestep].data() + ig * m_bytes);
    }

    return 0;
}

int CacheItem::get(void *p_data, json &p_jmsg){
    return 0;
}

vector<size_t> CacheItem::get_shape(){
    return m_varshape;
}

string CacheItem::get_dtype(){
    return m_dtype;
}

void CacheItem::flush(){
    m_timestep++;
}

void CacheMan::remove(string doid, string var, size_t timestep){
    m_cache[doid][var].remove(timestep);
}

void CacheMan::remove_all(size_t timestep){
    for(auto i : m_cache){
        for(auto j : m_cache[i.first]){
            j.second.remove(timestep);
        }
    }
}

void CacheItem::remove(size_t timestep){
    m_buffer.erase(timestep);
}

void CacheItem::clean(const string mode){
    if(mode == "zero"){
        memset(m_buffer[m_timestep].data(), 0, m_varbytes);
        return;
    }
    if(mode == "nan"){
        for(size_t i=0; i<m_varsize; i++){
            if(m_dtype == "float")
                ((float*)m_buffer[m_timestep].data())[i] = numeric_limits<float>::quiet_NaN();
        }
        return;
    }
}

const void *CacheItem::get_buffer(){
    return m_buffer[m_timestep].data();
}

int CacheMan::put(const void *p_data, json p_jmsg){
    if(check_json(p_jmsg, {"doid", "var"}, "CacheMan")){
        string doid = p_jmsg["doid"];
        string var = p_jmsg["var"];
        return m_cache[doid][var].put(p_data, p_jmsg);
    }
    return -1;
}

int CacheMan::get(void *p_data, json &p_jmsg){
    return 0;
}

void CacheMan::flush(){
    for(auto i : m_cache){
        for(auto j : m_cache[i.first]){
            j.second.flush();
        }
    }
}

const void *CacheMan::get_buffer(string doid, string var){
    return m_cache[doid][var].get_buffer();
}

void CacheMan::clean(string doid, string var, string mode){
    m_cache[doid][var].clean(mode);
}

void CacheMan::clean_all(string mode){
    for(auto i = m_cache.begin(); i != m_cache.end(); ++i){
        for(auto j = m_cache[i->first].begin(); j != m_cache[i->first].end(); ++j){
            j->second.clean(mode);
        }
    }
}

vector<string> CacheMan::get_do_list(){
    vector<string> do_list;
    for(auto it = m_cache.begin(); it != m_cache.end(); ++it)
        do_list.push_back(it->first);
    return do_list;
}

vector<string> CacheMan::get_var_list(string doid){
    vector<string> var_list;
    for(auto it = m_cache[doid].begin(); it != m_cache[doid].end(); ++it)
        var_list.push_back(it->first);
    return var_list;
}

vector<size_t> CacheMan::get_shape(string doid, string var){
    return m_cache[doid][var].get_shape();
}

string CacheMan::get_dtype(string doid, string var){
    return m_cache[doid][var].get_dtype();
}

} // end namespace realtime
} // end namespace adios
