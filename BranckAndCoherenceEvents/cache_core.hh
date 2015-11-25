#ifndef _COCHE_CORE_H_

#include "cache.hh"
#include "cache_util.hh"

class CacheCore{

private:
  CacheL1 *_cache;
  int _cores;
  CacheCore(){};

public:
  static CacheCore& getInstance(){
    static char buf[sizeof(CacheCore)];
    static CacheCore *singleObject = new (buf) CacheCore();
    return *singleObject;
  }

  void initialize(int cores ){
    _cache = new CacheL1[cores];
    _cores = cores;
  }

  void read( unsigned int address, int core ){
    //access(address, core);
    unsigned int setIndex = CacheUtil::getInstance().getSetIndex(address);
    unsigned int tag = CacheUtil::getInstance().getTag(address);
   
    fprintf(stderr, "read core %d setIndex %u, tag %u\n", core, setIndex, tag);

    //local read
    bool hit;
    CacheSet *set = _cache[core].getSet(setIndex);
    CacheLine *line = set->access(tag,&hit);

    fprintf(stderr, "Line state before %d\n", line->getState());
    //bus read
    if( hit && (line->getState() == EXCLUSIVE || line->getState() == MODIFIED))
      return;
    int read_hit = false;
    for(int i= 0; i< _cores; i++ ){
      if( i != core ){
        CacheSet *remote_set = _cache[i].getSet(setIndex);
        CacheLine *remote_line = remote_set->remoteAccess(tag);

        if( remote_line != NULL ) {
          fprintf(stderr, "Remote line in %d state before %d\n", i, remote_line->getState()); 
          read_hit = true;
          if(remote_line->getState() != INVALID && remote_line->getState() != SHARED)
            remote_line->setState(SHARED);
        }

      }
    }
    if(!read_hit){
      assert( line->getState() != SHARED );
      if(line->getState() == INVALID)
         line->setState(EXCLUSIVE);
      //if line is modified then remain so
    }
    else {
      line->setState(SHARED);
    }
    
  }
  void write( unsigned int address, int core){
    //access(address, core);
    unsigned int setIndex = CacheUtil::getInstance().getSetIndex(address);
    unsigned int tag = CacheUtil::getInstance().getTag(address);
  
    fprintf(stderr, "write core %d  setIndex %d tag %d\n",core, setIndex,tag);

    //local write
    bool hit;
    CacheSet *set = _cache[core].getSet(setIndex);
    CacheLine *line = set->access(tag, &hit);
    fprintf(stderr, "Line state before %d\n", line->getState());
    if(hit && line->getState() == MODIFIED ) return;
     
    line->setState(MODIFIED);

    //bus write invalidate
    for(int i=0; i<_cores; i++){
      if(i != core){
        CacheSet *remote_set = _cache[i].getSet(setIndex);
        CacheLine *remote_line = remote_set->remoteAccess(tag);
        
        if( remote_line != NULL ){ //remote hit
          fprintf(stderr, "Remote line in %d state before %d\n",i,  remote_line->getState());
          remote_line->setState(INVALID); 
        }
      }
    }
  }


  void access( unsigned int address, int core){
    unsigned int setIndex = CacheUtil::getInstance().getSetIndex(address);
    unsigned int tag = CacheUtil::getInstance().getTag(address);
    bool hit;
    fprintf(stderr, "setIndex %d\n", setIndex);
    CacheSet *set = _cache[core].getSet(setIndex);
    CacheLine *line = set->access(tag, &hit);


    fprintf(stderr, "tag %u foundtag %u\n", tag, line->getTag());
    
  }


};


#endif
