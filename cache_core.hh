#ifndef _COCHE_CORE_H_

#include "cache.hh"
#include "cache_util.hh"

class CacheCore{

private:
  CacheL1 *_cache;
  CacheCore(){};

public:
  static CacheCore& getInstance(){
    static char buf[sizeof(CacheCore)];
    static CacheCore *singleObject = new (buf) CacheCore();
    return *singleObject;
  }

  void initialize(int cores ){
    _cache = new CacheL1[cores];
  }

  void read( unsigned int address, int core ){
    access(address, core);
    
  }
  void write( unsigned int address, int core){
    access(address, core);
  }
  void access( unsigned int address, int core){
    unsigned int setIndex = CacheUtil::getInstance().getSetIndex(address);
    unsigned int tag = CacheUtil::getInstance().getTag(address);
    bool hit = 0;
    fprintf(stderr, "setIndex %d\n", setIndex);
    CacheSet *set = _cache[core].getSet(setIndex);
    CacheLine *line = set->access(tag);

    fprintf(stderr, "tag %u foundtag %u\n", tag, line->getTag());
    
  }

  



};


#endif
