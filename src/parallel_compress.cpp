#include <bits/stdc++.h>
#include <zlib.h>
#include <pthread.h>
using namespace std;

struct Task {
    const unsigned char* src;
    size_t len;
    size_t idx;
};
struct Result {
    vector<unsigned char> data;
    uLongf clen = 0;
    size_t olen = 0;
};

// args por hilo
struct Args {
    const vector<Task>* tasks;
    vector<Result>* results;
    size_t tid, nth;
};

static int zlevel = Z_BEST_SPEED; // nivel de compresión

void* worker(void* p){
    Args* a = (Args*)p;
    for(size_t i=a->tid;i<a->tasks->size();i+=a->nth){
        const Task& t = (*a->tasks)[i];
        uLongf bound = compressBound(t.len);
        Result& r = (*a->results)[t.idx];
        r.data.resize(bound);
        r.olen = t.len;
        int ok = compress2(r.data.data(), &bound, t.src, t.len, zlevel);
        if(ok!=Z_OK){ fprintf(stderr,"Error comprimiendo bloque %zu\n",t.idx); pthread_exit(nullptr); }
        r.clen = bound;
        r.data.resize(bound);
    }
    return nullptr;
}

int main(int argc,char**argv){
    if(argc<5){
        fprintf(stderr,"Uso: %s <in> <out> <hilos> <MB_bloque>\n",argv[0]);
        return 1;
    }
    string inF=argv[1], outF=argv[2];
    size_t T = stoul(argv[3]);
    size_t MB = stoul(argv[4]);
    size_t BLOCK = MB*(1ULL<<20);

    ifstream in(inF, ios::binary|ios::ate);
    if(!in){ perror("in"); return 1; }
    size_t total=(size_t)in.tellg(); in.seekg(0);
    vector<unsigned char> buf(total);
    if(!in.read((char*)buf.data(), total)){ fprintf(stderr,"Error leyendo\n"); return 1; }

    size_t nBlocks=(total+BLOCK-1)/BLOCK;
    vector<Task> tasks; tasks.reserve(nBlocks);
    vector<Result> results(nBlocks);
    for(size_t i=0;i<nBlocks;++i){
        size_t off=i*BLOCK, len=min(BLOCK,total-off);
        tasks.push_back({buf.data()+off, len, i});
    }

    vector<pthread_t> th(T); vector<Args> ar(T);
    for(size_t t=0;t<T;++t){
        ar[t]=Args{&tasks,&results,t,T};
        pthread_create(&th[t], nullptr, worker, &ar[t]);
    }
    for(auto&h:th) pthread_join(h,nullptr);


    ofstream out(outF, ios::binary);
    uint32_t nb=(uint32_t)nBlocks; out.write((char*)&nb,sizeof(nb));
    for(size_t i=0;i<nBlocks;++i){
        uint64_t osz=results[i].olen, csz=results[i].clen;
        out.write((char*)&osz,sizeof(osz));
        out.write((char*)&csz,sizeof(csz));
        out.write((char*)results[i].data.data(), results[i].clen);
    }
    fprintf(stdout,"OK: bloques=%u, bytes=%zu\n",nb,total);
    return 0;
}
