/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 *  This file contains an ISA-portable PIN tool for tracing memory accesses.
 */

#include <stdio.h>
#include<iostream>
#include "pin.H"

// FILE* trace;
FILE* new_trace;
// PIN_MUTEX lock;
PIN_LOCK lock;

long long total_mem_access=0;


void break_the_access(VOID* address,THREADID tid,USIZE length){

    long long addr=(long long)address;
    long long len = (long long)length;
    if(addr/64 == (addr+len-1)/64){
        while(len!=0){
            if(len>=8){
                // fprintf(new_trace,"Tid: %d Addr: %lld Len:%d\n",tid,addr,8);
                fprintf(new_trace,"%d %lld %d\n",tid,addr,8);
                len=len-8;
                addr+=8;
            }
            else if (len>=4){
                // fprintf(new_trace,"Tid: %d Addr: %lld Len:%d\n",tid,addr,4);
                fprintf(new_trace,"%d %lld %d\n",tid,addr,4);

                len=len-4;
                addr+=4;

            }
            else if(len>=2){
                // fprintf(new_trace,"Tid: %d Addr: %lld Len:%d\n",tid,addr,2);
                fprintf(new_trace,"%d %lld %d\n",tid,addr,2);
                len=len-2;
                addr+=2;

            }
            else if(len>=1){
                fprintf(new_trace,"%d %lld %d\n",tid,addr,1);
                len=len-1;
                addr+=1;

            }
        }
    }
    else{
        // std::cout<<"Crossing the cache block: "<<addr<<" "<<len<<std::endl;
        break_the_access((VOID*)addr,tid,(addr/64+1)*64-addr);
        break_the_access((VOID*)((addr/64+1)*64),tid,len+addr-(addr/64+1)*64);

    }
}
// Print a memory read recordmake
VOID RecordMemRead(VOID* ip, VOID* addr,THREADID tid,USIZE len) {

    PIN_GetLock(&lock, tid+1);
    // fprintf(trace, "%p: R %lld TID:%d LEN: %lu\n", ip, (long long)addr,tid,len); 
    break_the_access(addr,tid,len);
    total_mem_access++;
    PIN_ReleaseLock(&lock);

}

// Print a memory write record
VOID RecordMemWrite(VOID* ip, VOID* addr,THREADID tid,USIZE len) { 
    PIN_GetLock(&lock, tid+1);
    // fprintf(trace, "%p: W %lld TID:%d LEN: %lu\n", ip, (long long)addr,tid,len); 
    break_the_access(addr,tid,len);
    total_mem_access++;
    PIN_ReleaseLock(&lock);

}

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID* v)
{
    
    UINT32 memOperands = INS_MemoryOperandCount(ins);

    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            UINT32 len1 = INS_MemoryOperandSize(ins,memOp);
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead, IARG_INST_PTR, IARG_MEMORYOP_EA, memOp,IARG_THREAD_ID,IARG_UINT32,len1,
                                     IARG_END);
        }
        // Note that in some architectures a single memory operand can be
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))

        {
            UINT32 len2 =  INS_MemoryOperandSize(ins,memOp);
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite, IARG_INST_PTR, IARG_MEMORYOP_EA, memOp,IARG_THREAD_ID,IARG_UINT32,len2,
                                     IARG_END);
        }
    }
}

VOID Fini(INT32 code, VOID* v)
{
    // fprintf(trace, "#eof\n");
    // std::cout<<"TOTAL_MEM_ACCESS: "<<total_mem_access<<std::endl;
    // fclose(trace);
    // fprintf(new_trace, "#eof\n");
    fclose(new_trace);
}


INT32 Usage()
{
    PIN_ERROR("This Pintool prints a trace of memory addresses\n" + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}


int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv)) return Usage();
    PIN_InitLock(&lock);


    // string filename;
    
    // trace = fopen("raw_trace.out", "w");
    new_trace = fopen("mem_trace.out","w");

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
