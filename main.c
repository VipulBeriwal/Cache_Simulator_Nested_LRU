
/*
 * main.c
 */


#include <stdlib.h>
#include <stdio.h>
#include "cache.h"
#include "main.h"

static FILE *traceFile;


int main(int argc, char **argv)
{ 
  //pass by reference
  printf("->Will execute parse_args \n");
  parse_args(argc, argv);
  printf("\n->Will execute init_cache");
  init_cache();
  printf("->Will execute play trace\n");
  play_trace(traceFile);
  print_stats();

}


/************************************************************/
void parse_args(argc, argv)
  int argc; //count the number of arguments passed
  char **argv;  // pointer of the argument array
{
  int arg_index, i, value;

  if (argc < 2) {
    // it takes sim as an argument also so, default number of arguments without any specicfication would be 1
    printf("usage:  sim <options> <trace file>\n");
    // <options> = zeroth argument, <trace file> = first argument
    exit(-1);
  }

   
  /* parse the command line arguments */
  for (i = 0; i < argc; i++)
    if (!strcmp(argv[i], "-h")) {
      printf("\t-h:  \t\tthis message\n\n");
      printf("\t-bs <bs>: \tset cache block size to <bs>\n");
      printf("\t-us <us>: \tset unified cache size to <us>\n");
      printf("\t-is <is>: \tset instruction cache size to <is>\n");
      printf("\t-ds <ds>: \tset data cache size to <ds>\n");
      printf("\t-a <a>: \tset cache associativity to <a>\n");
      printf("\t-wb: \t\tset write policy to write back\n");
      printf("\t-wt: \t\tset write policy to write through\n");
      printf("\t-wa: \t\tset allocation policy to write allocate\n");
      printf("\t-nw: \t\tset allocation policy to no write allocate\n");
      exit(0);
    }
    
  arg_index = 1;

  // If we do not provide trace file then control shifts to while loop
  // it sets the cache parameters so that we define the cache model
  // If we provide trace then it will not go to this while loop to set the cache.
  // If we provide trace then argc = 2 and arg_index will be 1 and this will not run.
  // strcmp provides "0" if it finds the match!
  while (arg_index != 9) {

    /* set the cache simulator parameters */
    //argv breakdown in the form of array
    // ./sim -bs () -us () -is () -ds () -a  ()  write-policy  allocation-policy
    //    0   1  2  3   4   5  6   7  8  9  10      11              12 

    if (!strcmp(argv[arg_index], "-bs")) {
      value = atoi(argv[arg_index+1]);
      set_cache_param(CACHE_PARAM_BLOCK_SIZE, value);
      //printf("\n arg_index");
      arg_index += 2;
      continue;
    }

    if (!strcmp(argv[arg_index], "-us")) {
      value = atoi(argv[arg_index+1]);
      set_cache_param(CACHE_PARAM_USIZE, value);
      //printf("\n arg_index");
      arg_index += 2;
      continue;
    }

      
    if (!strcmp(argv[arg_index], "-a")) {
      value = atoi(argv[arg_index+1]);
      set_cache_param(CACHE_PARAM_ASSOC, value);
      //printf("\n arg_index");
      arg_index += 2;
      continue;
    }

    if (!strcmp(argv[arg_index], "-wb")) {
      set_cache_param(CACHE_PARAM_WRITEBACK, value);
      //printf("\n arg_index");
      arg_index += 1;
      continue;
    }
    

    if (!strcmp(argv[arg_index], "-wa")) {
      set_cache_param(CACHE_PARAM_WRITEALLOC, value);
      //printf("\n arg_index");
      arg_index += 1;
      continue;
    }

    printf("error:  unrecognized flag %s\n", argv[arg_index]);
    exit(-1);

  }
  printf("->Dump Settings will be executed");
  dump_settings();
  // if exactly two arguments specifically (options and trace file) it opens the trace file in read mode.  
  /* open the trace file */
  
  traceFile = fopen(argv[arg_index], "r");
  //printf("The value of trace file variable is %d", traceFile);
}
/************************************************************/

/************************************************************/
void play_trace(inFile)
  FILE *inFile;
{
  unsigned addr, data, access_type;
  int num_inst;
  int v;
  printf("->Will execute read trace \n");
  while(read_trace_element(inFile, &access_type, &addr)){
  printf("->Will execute perform access\n");
  printf("The value of address is %x\n", addr);
  perform_access(addr, access_type);
 }

  flush();
}
/************************************************************/

/************************************************************/
int read_trace_element(inFile, access_type, addr)
  FILE *inFile;
  unsigned *access_type, *addr;
{ 
  int result;
  char c;
  result = fscanf(inFile, "%u %x%c", access_type, addr, &c);
  while (c != '\n') {
    result = fscanf(inFile, "%c", &c);
    if (result == EOF) 
      break;
  }
  if (result != EOF)
    return(1);
  else
    return(0);
}
/************************************************************/
