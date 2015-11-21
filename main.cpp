#include<stdio.h>

#include "cache_util.hh"
#include "cache_core.hh"

int main(){

  CacheUtil::getInstance().initialize();
  unsigned int val = 0xf000f001;
  //printf("addr %u set %u tag %u\n", val, CacheUtil::getInstance().getSetIndex(val), CacheUtil::getInstance().getTag(val));
  CacheCore::getInstance().initialize(4);

  printf("\n\n");
  CacheCore::getInstance().read(val, 0);
  printf("\n\n");

  CacheCore::getInstance().read(val, 1);
  printf("\n\n");

  CacheCore::getInstance().read(val, 0);
  printf("\n\n");

  CacheCore::getInstance().write(val, 3);
  printf("\n\n");

  CacheCore::getInstance().read(val, 2);
  printf("\n\n");

  CacheCore::getInstance().read(val, 2);
  printf("\n\n");

  CacheCore::getInstance().read(val, 2);
  printf("\n\n");

  CacheCore::getInstance().read(val, 0);
  printf("\n\n");

  CacheCore::getInstance().read(val, 2);
  printf("\n\n");

#if 0
  val = 0xf000f101;

  CacheCore::getInstance().read(val, 0);

  val = 0xf010f101;

  CacheCore::getInstance().read(val, 0);

  //val = 0xf010f001;

  CacheCore::getInstance().read(val, 0);

  val = 0xf011f101;

  CacheCore::getInstance().read(val, 0);

  //val = 0xf010f001;

  CacheCore::getInstance().read(val, 0);

  val = 0xf111f101;

  CacheCore::getInstance().read(val, 0);

  //val = 0xf010f001;

  CacheCore::getInstance().read(val, 0);

  val = 0xf211f101;

  CacheCore::getInstance().read(val, 0);

  //val = 0xf010f001;

  CacheCore::getInstance().read(val, 0);


  val = 0xf011f101;

  CacheCore::getInstance().read(val, 0);


#endif


}
