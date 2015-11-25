#include "pin.H"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>
#include <assert.h>
#include "cache_util.hh"
#include "cache_core.hh"

#define BUF_SIZE 512
//#define COUNTER 1

enum {BRANCH_TAKEN=4, BRANCH_NOT_TAKEN};
/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */
volatile INT16 active_threads = 1;
volatile INT16 start_profile = 0;

//fstream out_file;
fstream **out_file; //file for each thread
THREADID total_threads;
volatile unsigned int inst_counter = 0;
int sample_counter;
const CHAR *excluded_lib_names[] = { "lib64","libpthread","libstdc++",
                                      "libm", "libgcc_s", "libc", "ld-linux", "ld_linux", "librt"};
const UINT16 n_excluded_lib_names = 9;
const UINT16 MAX_MODULES = 32;
bool SeqProgram = false;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool","o", "perf.log", "specify the output file name" );
KNOB<string> KnobThreadCount(KNOB_MODE_WRITEONCE, "pintool","threads", "1", "specify number of threads in the program" );
KNOB<string> KnobStdinFile(KNOB_MODE_WRITEONCE,  "pintool", "stdin", "", "STDIN file for the application");
KNOB<string> KnobSeqProgram(KNOB_MODE_WRITEONCE,  "pintool", "seq", "", "a seqential program?");
KNOB<string> KnobSampleRate(KNOB_MODE_WRITEONCE,  "pintool", "counter", "", "a seqential program?");
/* ===================================================================== */
/* Analysis routines                                                     */
/* ===================================================================== */
VOID ThreadBegin(THREADID thread_id, CONTEXT *ctxt, INT32 flags, VOID *v)
{
  INT16 val = 1;
  __asm__ __volatile__("lock xaddw %3, %0"
                         : "=m"(active_threads), "=r"(val)
                         :  "m"(active_threads), "1"(val)
                         );
  ASSERTX(thread_id < total_threads);
  //ASSERTX(thread_id < MAX_THREADS);
  if (start_profile == 0 && active_threads > 1) start_profile = 1;
  std::cout << "Thread start " << thread_id << endl;
}


VOID ThreadEnd(THREADID thread_id, const CONTEXT *ctxt, INT32 code, VOID *v)
{
  INT16 val = -1;
   __asm__ __volatile__("lock xaddw %3, %0"
                    : "=m"(active_threads), "=r"(val)
                    :  "m"(active_threads), "1"(val)
                     );
  cout << "Thread end " << thread_id << endl;
}

VOID ProcessBranch( BOOL isTaken, ADDRINT _inst ){
  if(isTaken){
    *(out_file[0]) << "0x"<< hex << _inst << "," << dec << BRANCH_TAKEN << endl;
  }
  else {
    *(out_file[0]) << "0x" << hex << _inst << "," << dec << BRANCH_NOT_TAKEN << endl;
  }
  
}

VOID ProcessInst( THREADID thread_id , ADDRINT _waddr, ADDRINT _raddr, ADDRINT _raddr2, ADDRINT _inst ){
  ASSERTX(thread_id < total_threads);
	ASSERTX(_waddr || _raddr || _raddr2);
  if(start_profile){
#if 0
    unsigned int val = 1;
    __asm__ __volatile__("lock xaddl %3, %0"
                         : "=m"(inst_counter), "=r"(val)
                         :  "m"(inst_counter), "1"(val)
                        ); 
#endif
    if( _waddr ){
      CacheCore::getInstance().write(_waddr, thread_id );
    }
    else {
      if(_raddr){
        CacheCore::getInstance().read(_raddr, thread_id );
      
        if( _raddr2 ) {
          CacheCore::getInstance().read(_raddr2, thread_id ); 
        }
      }
    }
    //int rand_sample = rand()%100;
    //if( rand_sample <  10 ) { //sample rate of  10 in 100perf event
    //  inst_counter = 0;
      *(out_file[thread_id]) <<"0x"<<hex<<_inst<<dec<< "," << CacheCore::getInstance().getCacheEvent(thread_id) << endl;
    //}
  }


}

/* ===================================================================== */
/* Instrumentation routines                                              */
/* ===================================================================== */
VOID Image(IMG img, VOID *v){

  for (UINT16 i = 0; i < n_excluded_lib_names; i++)
  {
    if (IMG_Name(img).find(excluded_lib_names[i]) != string::npos){
      cout << "Excluded module: " << IMG_Name(img) << endl;
      return;
    }
  }

	for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)){
    for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {
      RTN_Open(rtn);
      for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins)) {
        // Avoid instrumenting the instrumentation
        if (!INS_IsOriginal(ins))
          continue;
				
        if(!SeqProgram) {
				  if ((INS_IsMemoryWrite(ins) || INS_IsMemoryRead(ins)) && INS_HasFallThrough(ins)) {		
				  	if (INS_IsMemoryWrite(ins) && INS_IsMemoryRead(ins) &&
                INS_HasMemoryRead2(ins)) {
              INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ProcessInst, IARG_THREAD_ID,
                            IARG_MEMORYWRITE_EA,
                            IARG_MEMORYREAD_EA,
                            IARG_MEMORYREAD2_EA,
                            IARG_INST_PTR,
                            IARG_END); 
            }					
				  	else if (INS_IsMemoryWrite(ins) && INS_IsMemoryRead(ins) &&
          	          !INS_HasMemoryRead2(ins)) {
				  		INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ProcessInst, IARG_THREAD_ID,
                            IARG_MEMORYWRITE_EA,
                            IARG_MEMORYREAD_EA,
                            IARG_ADDRINT, 0,
                            IARG_INST_PTR,
                            IARG_END);		
				  		
				  	}
				  	else if (INS_IsMemoryWrite(ins) && !INS_IsMemoryRead(ins)) {
				  		INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ProcessInst, IARG_THREAD_ID,
                            IARG_MEMORYWRITE_EA,
                            IARG_ADDRINT, 0,
                            IARG_ADDRINT, 0,
                            IARG_INST_PTR,
                            IARG_END);		
				  	}
				  	else if (!INS_IsMemoryWrite(ins) && INS_IsMemoryRead(ins) &&
                    INS_HasMemoryRead2(ins)) {
				  		INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ProcessInst, IARG_THREAD_ID,
                 						IARG_ADDRINT, 0,
                            IARG_MEMORYREAD_EA,
                            IARG_MEMORYREAD2_EA,
                            IARG_INST_PTR,
                            IARG_END);
				  	}
				  	else if (!INS_IsMemoryWrite(ins) && INS_IsMemoryRead(ins) &&
                    !INS_HasMemoryRead2(ins)) {
				  		INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ProcessInst, IARG_THREAD_ID,
                 						IARG_ADDRINT, 0,
                            IARG_MEMORYREAD_EA,
                            IARG_ADDRINT,0,
                            IARG_INST_PTR,
                            IARG_END);	
				  	}
				  	else { //not a memory opeartion     
              ASSERTX(0);
				  	}
				  }
        }
        else {
          if(INS_IsBranch(ins)){
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ProcessBranch, 
              IARG_BRANCH_TAKEN, IARG_INST_PTR, IARG_END);
          }
        }
			}
			RTN_Close(rtn);
		}
	}

  
}


VOID Fini(INT32 code, VOID *v){

	//out_file.close();
  for (INT32 i = 0; i < total_threads; i++) {
    out_file[i]->close();
    delete out_file[i];
  }
  delete[] out_file;
}

INT32 Usage(){
  return 0;
}



int main( int argc, char **argv ){
  
  PIN_InitSymbols();

  if( PIN_Init(argc,argv) ) {
    return Usage();
  }

  //out_file.open(KnobOutputFile.Value().c_str(), fstream::out);
	
	
	stringstream ss(KnobThreadCount.Value());
	ss >> total_threads;

  char *out_file_name = new char[BUF_SIZE];
  out_file = new fstream*[total_threads];
  for (INT32 i = 0; i < total_threads; i++) {
    sprintf(out_file_name, "%s.%d", KnobOutputFile.Value().c_str(), i);
    //cout<< out_file_name << endl;
    out_file[i] = new fstream(out_file_name, fstream::out);
  }


  stringstream ss_count(KnobSampleRate.Value());
	ss_count >> sample_counter;

  cout << "::PINTOOL:: sample per " << sample_counter << " inst" <<  endl;

  string stdinFile = KnobStdinFile.Value();
  if(stdinFile.size() > 0) {
    assert(freopen(stdinFile.c_str(), "rt", stdin));
  }

  string isSeqProgram = KnobSeqProgram.Value();
  if(isSeqProgram.size() > 0) {
    SeqProgram = true;
  }


  CacheUtil::getInstance().initialize();
  CacheCore::getInstance().initialize(total_threads);
  

  if(!SeqProgram)	{
    PIN_AddThreadStartFunction(ThreadBegin,0);
    PIN_AddThreadFiniFunction(ThreadEnd,0);
  }


  /* initialize random seed: */
  srand (time(NULL));

  IMG_AddInstrumentFunction(Image, 0);

  PIN_AddFiniFunction(Fini, 0);
  
  PIN_StartProgram();
}

