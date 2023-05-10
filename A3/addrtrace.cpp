/*********************** Author: Mainak Chaudhuri ****************/

#include <stdio.h>
// #include<bits/stdc++.h>
#include "pin.H"

#define LOG_BLOCK_SIZE 6
#define BLOCK_SIZE (1 << LOG_BLOCK_SIZE)
#define BLOCK_MASK 0x3f

FILE * trace[8];



FILE *trace_l;
UINT64 trace_length;
UINT64 trace_per_thread_len[8]={0};
PIN_LOCK pinLock;

UINT64 global_ctr=0;

struct mem_access{
    UINT64 ctr;
    UINT64 addr;
    UINT64 is_write;
};

// Print a memory read record
VOID RecordMemAccess(VOID * addr, USIZE size, THREADID tid,USIZE iswrite)
{
    UINT64 start = (UINT64)addr;
    USIZE size1;
    UINT64 start1, end1, elem;
    struct mem_access temp;
    UINT32 num8, num4, num2, num1, i;

    start1 = start;
    end1 = start1 | BLOCK_MASK;
    if (start1 + size - 1 <= end1) {
       num8 = size >> 3;
       size1 = size & 0x7;
       num4 = size1 >> 2;
       size1 = size1 & 0x3;
       num2 = size1 >> 1;
       num1 = size1 & 0x1;
       for (i=0; i<num8; i++) {
          elem = ((start1 + (i << 3))>>3);
          
            
          PIN_GetLock(&pinLock, tid+1);
          temp.addr = elem;
          temp.is_write = iswrite;
          temp.ctr=global_ctr;
          global_ctr++;
        temp.addr >>=3; fwrite(&temp, sizeof(temp), 1, trace[tid]); fflush(trace[tid]);
          // fprintf(trace[tid],"%lu %lu %lu\n",temp.ctr,temp.addr>>3,temp.is_write); fflush(trace[tid]);
          trace_length++;
          trace_per_thread_len[tid]++;          
          PIN_ReleaseLock(&pinLock);
       }
       if (num4) {
          elem = ((start1 + (num8 << 3)) << 3);
          PIN_GetLock(&pinLock, tid+1);
          temp.addr = elem;
          temp.is_write = iswrite;
          temp.ctr=global_ctr;
          global_ctr++;
        temp.addr >>=3; fwrite(&temp, sizeof(temp), 1, trace[tid]); fflush(trace[tid]);
        // fprintf(trace[tid],"%lu %lu %lu\n",temp.ctr,temp.addr>>3,temp.is_write); fflush(trace[tid]);
          trace_length++;
            trace_per_thread_len[tid]++;
          PIN_ReleaseLock(&pinLock);
       }
       if (num2) {
          elem = ((start1 + (num8 << 3) + (num4 << 2)) << 3);
          PIN_GetLock(&pinLock, tid+1);
          temp.addr = elem;
          temp.is_write = iswrite;
          temp.ctr=global_ctr;
          global_ctr++;
        temp.addr >>=3; fwrite(&temp, sizeof(temp), 1, trace[tid]); fflush(trace[tid]);
        // fprintf(trace[tid],"%lu %lu %lu\n",temp.ctr,temp.addr>>3,temp.is_write); fflush(trace[tid]);
          trace_length++;
            trace_per_thread_len[tid]++;
          PIN_ReleaseLock(&pinLock);
       }
       if (num1) {
          elem = ((start1 + (num8 << 3) + (num4 << 2) + (num2 << 1)) << 3);
          PIN_GetLock(&pinLock, tid+1);
          temp.addr = elem;
          temp.is_write = iswrite;
          temp.ctr=global_ctr;
          global_ctr++;
        temp.addr >>=3; fwrite(&temp, sizeof(temp), 1, trace[tid]); fflush(trace[tid]);
        // fprintf(trace[tid],"%lu %lu %lu\n",temp.ctr,temp.addr>>3,temp.is_write); fflush(trace[tid]);
          trace_length++;
            trace_per_thread_len[tid]++;
          PIN_ReleaseLock(&pinLock);
       }
    }
    else {
       size1 = end1 - start1 + 1;
       num8 = size1 >> 3;
       size1 = size1 & 0x7;
       num4 = size1 >> 2;
       size1 = size1 & 0x3;
       num2 = size1 >> 1;
       num1 = size1 & 0x1;
       for (i=0; i<num8; i++) {
          elem = ((start1 + (i << 3)) << 3);
          PIN_GetLock(&pinLock, tid+1);
          temp.addr = elem;
          temp.is_write = iswrite;
          temp.ctr=global_ctr;
          global_ctr++;
        temp.addr >>=3; fwrite(&temp, sizeof(temp), 1, trace[tid]); fflush(trace[tid]);
        // fprintf(trace[tid],"%lu %lu %lu\n",temp.ctr,temp.addr>>3,temp.is_write); fflush(trace[tid]);
          trace_length++;
            trace_per_thread_len[tid]++;
          PIN_ReleaseLock(&pinLock);
       }
       if (num4) {
          elem = ((start1 + (num8 << 3)) << 3);
          PIN_GetLock(&pinLock, tid+1);
          temp.addr = elem;
          temp.is_write = iswrite;
          temp.ctr=global_ctr;
          global_ctr++;
        temp.addr >>=3; fwrite(&temp, sizeof(temp), 1, trace[tid]); fflush(trace[tid]);
        // fprintf(trace[tid],"%lu %lu %lu\n",temp.ctr,temp.addr>>3,temp.is_write); fflush(trace[tid]);
          trace_length++;
            trace_per_thread_len[tid]++;
          PIN_ReleaseLock(&pinLock);
       }
       if (num2) {
          elem = ((start1 + (num8 << 3) + (num4 << 2)) << 3) ;
          PIN_GetLock(&pinLock, tid+1);
          temp.addr = elem;
          temp.is_write = iswrite;
          temp.ctr=global_ctr;
          global_ctr++;
        temp.addr >>=3; fwrite(&temp, sizeof(temp), 1, trace[tid]); fflush(trace[tid]);
        // fprintf(trace[tid],"%lu %lu %lu\n",temp.ctr,temp.addr>>3,temp.is_write); fflush(trace[tid]);
          trace_length++;
            trace_per_thread_len[tid]++;
          PIN_ReleaseLock(&pinLock);
       }
       if (num1) {
          elem = ((start1 + (num8 << 3) + (num4 << 2) + (num2 << 1)) << 3);
          PIN_GetLock(&pinLock, tid+1);
          temp.addr = elem;
          temp.is_write = iswrite;
          temp.ctr=global_ctr;
          global_ctr++;
        temp.addr >>=3; fwrite(&temp, sizeof(temp), 1, trace[tid]); fflush(trace[tid]);
        // fprintf(trace[tid],"%lu %lu %lu\n",temp.ctr,temp.addr>>3,temp.is_write); fflush(trace[tid]);
          trace_length++;
            trace_per_thread_len[tid]++;
          PIN_ReleaseLock(&pinLock);
       }

       size1 = size - (end1 - start1 + 1);
       start1 = end1 + 1;
       while (size1 > BLOCK_SIZE) {
          for (i=0; i<BLOCK_SIZE >> 3; i++) {
             elem = ((start1 + (i << 3)) << 3);
             PIN_GetLock(&pinLock, tid+1);
             temp.addr = elem;
             temp.is_write = iswrite;
             temp.ctr=global_ctr;
             global_ctr++;
             temp.addr >>= 3;
             fwrite(&temp, sizeof(temp), 1, trace[tid]); fflush(trace[tid]);
             // fprintf(trace[tid],"%lu %lu %lu\n",temp.ctr,temp.addr>>3,temp.is_write); fflush(trace[tid]);
             trace_length++;
                trace_per_thread_len[tid]++;
             PIN_ReleaseLock(&pinLock);
          }
          size1 = size1 - BLOCK_SIZE;
          start1 = start1 + BLOCK_SIZE;
       }

       num8 = size1 >> 3;
       size1 = size1 & 0x7;
       num4 = size1 >> 2;
       size1 = size1 & 0x3;
       num2 = size1 >> 1;
       num1 = size1 & 0x1;
       for (i=0; i<num8; i++) {
          elem = ((start1 + (i << 3)) << 3);
          PIN_GetLock(&pinLock, tid+1);
          temp.addr = elem;
          temp.is_write = iswrite;
          temp.ctr=global_ctr;
          global_ctr++;
        temp.addr >>=3; fwrite(&temp, sizeof(temp), 1, trace[tid]); fflush(trace[tid]);
        // fprintf(trace[tid],"%lu %lu %lu\n",temp.ctr,temp.addr>>3,temp.is_write); fflush(trace[tid]);
          trace_length++;
            trace_per_thread_len[tid]++;
          PIN_ReleaseLock(&pinLock);
       }
       if (num4) {
          elem = ((start1 + (num8 << 3)) << 3);
          PIN_GetLock(&pinLock, tid+1);
          temp.addr = elem;
          temp.is_write = iswrite;
          temp.ctr=global_ctr;
          global_ctr++;
        temp.addr >>=3; fwrite(&temp, sizeof(temp), 1, trace[tid]); fflush(trace[tid]);
        // fprintf(trace[tid],"%lu %lu %lu\n",temp.ctr,temp.addr>>3,temp.is_write); fflush(trace[tid]);
          trace_length++;
            trace_per_thread_len[tid]++;
          PIN_ReleaseLock(&pinLock);
       }
       if (num2) {
          elem = ((start1 + (num8 << 3) + (num4 << 2)) << 3);
          PIN_GetLock(&pinLock, tid+1);
          temp.addr = elem;
          temp.is_write = iswrite;
          temp.ctr=global_ctr;
          global_ctr++;
        temp.addr >>=3; fwrite(&temp, sizeof(temp), 1, trace[tid]); fflush(trace[tid]);
        // fprintf(trace[tid],"%lu %lu %lu\n",temp.ctr,temp.addr>>3,temp.is_write); fflush(trace[tid]);
          trace_length++;
            trace_per_thread_len[tid]++;
          PIN_ReleaseLock(&pinLock);
       }
       if (num1) {
          elem = ((start1 + (num8 << 3) + (num4 << 2) + (num2 << 1)) << 3);
          PIN_GetLock(&pinLock, tid+1);
          temp.addr = elem;
          temp.is_write = iswrite;
          temp.ctr=global_ctr;
          global_ctr++;
        temp.addr >>=3; fwrite(&temp, sizeof(temp), 1, trace[tid]); fflush(trace[tid]);
        // fprintf(trace[tid],"%lu %lu %lu\n",temp.ctr,temp.addr>>3,temp.is_write); fflush(trace[tid]);
          trace_length++;
            trace_per_thread_len[tid]++;
          PIN_ReleaseLock(&pinLock);
       }
    }
}

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v)
{
    // Instruments memory accesses using a predicated call, i.e.
    // the instrumentation is called iff the instruction will actually be executed.
    //
    // On the IA-32 and Intel(R) 64 architectures conditional moves and REP 
    // prefixed instructions appear as predicated instructions in Pin.
    UINT32 memOperands = INS_MemoryOperandCount(ins);

    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemAccess,
                IARG_MEMORYOP_EA, memOp, IARG_UINT32, INS_MemoryOperandSize(ins, memOp), IARG_THREAD_ID,IARG_UINT32,0,
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemAccess,
                IARG_MEMORYOP_EA, memOp,  IARG_UINT32, INS_MemoryOperandSize(ins, memOp), IARG_THREAD_ID,IARG_UINT32,1,
                IARG_END);
        }
    }
}

VOID Fini(INT32 code, VOID *v)
{
    fprintf(trace_l, "%lu\n", trace_length);
    fprintf(trace_l, "%lu \n%lu \n%lu \n%lu \n%lu \n%lu \n%lu \n%lu\n",trace_per_thread_len[0],trace_per_thread_len[1],trace_per_thread_len[2],trace_per_thread_len[3],trace_per_thread_len[4],trace_per_thread_len[5],trace_per_thread_len[6],trace_per_thread_len[7]);
    fclose(trace[0]);
    fclose(trace[1]);
    fclose(trace[2]);
    fclose(trace[3]);
    fclose(trace[4]);
    fclose(trace[5]);
    fclose(trace[6]);
    fclose(trace[7]);

    fclose(trace_l);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
   
INT32 Usage()
{
    PIN_ERROR( "This Pintool prints a trace of memory addresses\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    if (PIN_Init(argc, argv)) return Usage();

    trace[0] = fopen("trace0.out", "wb");
    trace[1] = fopen("trace1.out", "wb");
    trace[2] = fopen("trace2.out", "wb");
    trace[3] = fopen("trace3.out", "wb");
    trace[4] = fopen("trace4.out", "wb");
    trace[5] = fopen("trace5.out", "wb");
    trace[6] = fopen("trace6.out", "wb");
    trace[7] = fopen("trace7.out", "wb");
    

    trace_l = fopen("addrtrace_l.out", "w");
    trace_length = 0;

    PIN_InitLock(&pinLock);
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();
    
    return 0;
}