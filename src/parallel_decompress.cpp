#include <bits/stdc++.h>
#include <zlib.h>
#include <pthread.h>
using namespace std;

struct CBlock {
    vector<unsigned char> cdata; // datos comprimidos
    size_t idx;
    uint64_t olen;               // tamaño original
};
struct DBlock {
    vector<unsigned char> data;  // datos descomprimidos
    bool ok=false;
};

struct Args {
    const vector<CBlock>* inBlocks;
    vector<DBlock>* outBlocks;
    size_t tid, nth;
};

void* worker(void* p){
    Args* a=(Args*)p;
    for(size_t i=a->tid;i<a->inBlocks->size();i+=a->nth){
        const CBlock& b=(*a->inBlocks)[i];
        DBlock& out=(*a->outBlocks)[b.idx];
        out.data.resize(b.olen);
        uLongf destLen=b.olen;
        int r=uncompress(out.data.data(), &destLen, b.cdata.data(), b.cdata.size());
        if(r==Z_OK && destLen==b.olen){ out.ok=true; }
        else { fprintf(stderr,"Error descomprimiendo bloque %zu\n",b.idx); pthread_exit(nullptr); }
    }
    return nullptr;
}

int main(int argc,char**argv){
    if(argc<4){
        fprintf(stderr,"Uso: %s <in_par.bin> <out.txt> <hilos>\n", argv[0]);
        return 1;
    }
    string inF=argv[1], outF=argv[2]; size_t T=stoul(argv[3]);

    ifstream in(inF, ios::binary);
    if(!in){ perror("in"); return 1; }

    uint32_t nBlocks=0; in.read((char*)&nBlocks,sizeof(nBlocks));
    vector<CBlock> blocks(nBlocks);
    for(uint32_t i=0;i<nBlocks;++i){
        uint64_t osz=0, csz=0;
        in.read((char*)&osz,sizeof(osz));
        in.read((char*)&csz,sizeof(csz));
        blocks[i].cdata.resize((size_t)csz);
        blocks[i].idx=i; blocks[i].olen=osz;
        in.read((char*)blocks[i].cdata.data(), csz);
    }

    vector<DBlock> outs(nBlocks);
    vector<pthread_t> th(T); vector<Args> ar(T);
    for(size_t t=0;t<T;++t){
        ar[t]=Args{&blocks,&outs,t,T};
        pthread_create(&th[t], nullptr, worker, &ar[t]);
    }
    for(auto&h:th) pthread_join(h,nullptr);

    ofstream out(outF, ios::binary);
    for(uint32_t i=0;i<nBlocks;++i){
        if(!outs[i].ok){ fprintf(stderr,"Bloque %u falló\n",i); return 1; }
        out.write((char*)outs[i].data.data(), outs[i].data.size());
    }
    fprintf(stdout,"OK descompresion, bloques=%u\n",nBlocks);
    return 0;
}
