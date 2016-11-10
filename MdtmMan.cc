#include <sys/stat.h>
#include <unistd.h>
#include"MdtmMan.h"
#include"zmq.h"
#include <fcntl.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

MdtmMan::MdtmMan(string local_address,
        string remote_address,
        string mode,
        string prefix,
        int num_pipes,
        vector<int> tolerance,
        vector<int> priority)
    :StreamMan(local_address, remote_address)
{
    pipe_desc["operation"] = "init";
    pipe_desc["mode"] = mode;
    pipe_desc["pipe_prefix"] = prefix;

    string pipename_prefix = "MdtmManPipe";
    for(int i=0; i<num_pipes; i++){
        stringstream pipename;
        pipename << pipename_prefix << i;
        if(i==0){
            pipe_desc["pipe_names"] = {pipename.str()};
            pipe_desc["loss_tolerance"] = {tolerance[i]};
            pipe_desc["priority"] = {priority[i]};
        }
        else{
            pipe_desc["pipe_names"].insert(pipe_desc["pipe_names"].end(), pipename.str());
            pipe_desc["loss_tolerance"].insert(pipe_desc["loss_tolerance"].end(), tolerance[i]);
            pipe_desc["priority"].insert(pipe_desc["priority"].end(), priority[i]);
        }
    }

    // Pipes
    for (int i=0; i<pipe_desc["pipe_names"].size(); i++){
        string filename = prefix + rmquote(pipe_desc["pipe_names"][i].dump());
        mkfifo(filename.c_str(), 0666);
    }


    for(int i=0; i<num_pipes; i++){
        stringstream pipename;
        pipename << pipename_prefix << i;
        string fullpipename = prefix + pipename.str();
        cout << fullpipename << endl;
        if (mode == "sender"){
            cout << "sender pipe open" << endl;
            int fp = open(fullpipename.c_str(), O_WRONLY | O_NONBLOCK);
            pipes.push_back(fp);
            cout << "init pipe " << fullpipename << endl;
        }
        if (mode == "receiver"){
            cout << "receiver pipe open" << endl;
            int fp = open(fullpipename.c_str(), O_RDONLY | O_NONBLOCK);
            pipes.push_back(fp);
            cout << "init pipe " << fullpipename << endl;
        }
        printf("pipe pointer %d ------------------- \n", pipes[i]);
        pipenames.push_back(pipename.str());
    }


    // ZMQ_DataMan_MDTM

    zmq_ipc_req = zmq_socket (zmq_context, ZMQ_REQ);
    zmq_ipc_rep = zmq_socket (zmq_context, ZMQ_REP);
    zmq_connect (zmq_ipc_req, "ipc:///tmp/ADIOS_MDTM_pipe");
    zmq_bind (zmq_ipc_rep, "ipc:///tmp/MDTM_ADIOS_pipe");

    char buffer_return[10];
    zmq_send (zmq_ipc_req, pipe_desc.dump().c_str(), pipe_desc.dump().length(), 0);
    zmq_recv (zmq_ipc_req, buffer_return, 10, 0);

    zmq_ipc_rep_thread_active = true;
    zmq_ipc_rep_thread = new thread(&MdtmMan::zmq_ipc_rep_thread_func, this);

}

MdtmMan::~MdtmMan(){
    zmq_close (zmq_ipc_rep);
    zmq_close (zmq_ipc_req);
    zmq_ipc_rep_thread_active = false;
    zmq_ipc_rep_thread->join();
    delete zmq_ipc_rep_thread;
}

int MdtmMan::put(const void *data,
        string doid,
        string var,
        string dtype,
        vector<uint64_t> putshape,
        vector<uint64_t> varshape,
        vector<uint64_t> offset,
        int tolerance,
        int priority)
{
    json msg;
    msg["operation"] = "put";
    msg["doid"] = doid;
    msg["var"] = var;
    msg["dtype"] = dtype;
    msg["putshape"] = putshape;
    msg["varshape"] = varshape;
    msg["offset"] = offset;

    int index = closest(priority, pipe_desc["priority"], true);
    msg["pipe"] = pipe_desc["pipe_names"][index];

    uint64_t putsize = std::accumulate(putshape.begin(), putshape.end(), dsize(dtype), std::multiplies<uint64_t>());
    msg["putsize"] = putsize;
    uint64_t varsize = std::accumulate(varshape.begin(), varshape.end(), dsize(dtype), std::multiplies<uint64_t>());
    msg["varsize"] = varsize;

    cout << msg << endl;

    char ret[10];
    zmq_send(zmq_tcp_req, msg.dump().c_str(), msg.dump().length(), 0);
    zmq_recv(zmq_tcp_req, ret, 10, 0);

    index=0;
    for(int i=0; i<pipenames.size(); i++){
        if(rmquote(msg["pipe"].dump()) == pipenames[i]){
            index=i;
        }
    }
    string pipename = rmquote(pipe_desc["pipe_prefix"].dump()) + rmquote(msg["pipe"].dump());
    write(pipes[index], data, putsize);
    return 0;
}

int MdtmMan::get(void *data, json j){
    int putsize = j["putsize"].get<int>();
    int index=0;
    for(int i=0; i<pipenames.size(); i++){
        cout << j["pipe"].dump() << "    " << pipenames[i] << endl;
        if(rmquote(j["pipe"].dump()) == pipenames[i]){
            index=i;
            cout << "found pipe " << j["pipe"].dump() << "    " << i << endl;
        }
    }
    read(pipes[index], data, putsize);
    return 0;
}

void MdtmMan::zmq_ipc_rep_thread_func(){
    while (zmq_ipc_rep_thread_active){
        char msg[1024];
        zmq_recv (zmq_ipc_rep, msg, 1024, ZMQ_NOBLOCK);
        zmq_send (zmq_ipc_rep, "OK", 10, 0);
        usleep(10000);
    }
}


