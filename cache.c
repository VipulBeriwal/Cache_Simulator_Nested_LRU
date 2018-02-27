/*
 * cache.c
 */


#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "cache.h"
#include "main.h"

/* cache configuration parameters */
static int cache_split = 0;
static int cache_usize = DEFAULT_CACHE_SIZE;
static int cache_isize = DEFAULT_CACHE_SIZE; 
static int cache_dsize = DEFAULT_CACHE_SIZE;
static int cache_block_size = DEFAULT_CACHE_BLOCK_SIZE;
static int words_per_block = DEFAULT_CACHE_BLOCK_SIZE / WORD_SIZE;
static int cache_assoc = DEFAULT_CACHE_ASSOC;
static int cache_writeback = DEFAULT_CACHE_WRITEBACK;
static int cache_writealloc = DEFAULT_CACHE_WRITEALLOC;
static int cache_sets = 0;
static int cache_lines = 0;


static int hits = 0;
static int accesses = 0;
static int misses = 0;


Pcache CACHE;
//cache model data structures 
/*static Pcache icache;
static Pcache dcache;
static cache c1;
static cache c2;
static cache_stat cache_stat_inst;
static cache_stat cache_stat_data;
*/

void Set_Zero_Node(unsigned index){
  Pcache_set node = malloc(sizeof(struct cache_set_));
  node->index = index;
  node->set_next = NULL;
  node->set_prev = NULL;
  //for lines
  node->LINE_head = NULL;
  node->LINE_tail = NULL;
  node->lines_count = 0;
  CACHE->SET_head = node;
  CACHE->SET_tail = node;
}

void Set_Node(unsigned index){
  Pcache_set node = malloc(sizeof(struct cache_set_));
  node->set_next = CACHE->SET_head;
  node->set_prev = NULL;
  node->index = index;
  node->LINE_head = NULL;
  node->LINE_tail = NULL;
  node->lines_count = 0;
  (CACHE->SET_head)->set_prev = node;
  CACHE->SET_head = node;
}

void Arrange_Set(Pcache_set PTR){
  if ((PTR == CACHE->SET_tail) && (PTR != CACHE->SET_head)){
    printf("The index matches on the last tail node\n");
     (PTR->set_prev)->set_next = NULL;
     CACHE->SET_tail = PTR->set_prev;
     PTR->set_next = CACHE->SET_head;
     PTR->set_prev = NULL;
     (CACHE->SET_head)->set_prev = PTR;
     CACHE->SET_head = PTR;
  }
  else if (PTR == CACHE->SET_head){
    printf("The index matches on the first node itself\n");
  }
  else{
    printf("The index matched on any middle node\n");
    (PTR->set_next)->set_prev = PTR->set_prev;
    (PTR->set_prev)->set_next = PTR->set_next;
    PTR->set_next = CACHE->SET_head;
    PTR->set_prev = NULL;
    CACHE->SET_head->set_prev = PTR;
    CACHE->SET_head = PTR;
  }
}

Pcache_set SET(unsigned index){
 Pcache_set PTR = CACHE->SET_head;

 if(PTR == NULL){
  printf("\n\n->SET_Zero_Node function is called to make new node.\nThe value of PTR is null, not even a single set node.\n");
  Set_Zero_Node(index);
  CACHE->sets_count = CACHE->sets_count + 1;
 }
 else{
  printf("Traversing for the index match......\n");
  while(PTR != NULL){
    if (PTR->index == index){
      printf("The index matches. No need to create new node\n");
      Arrange_Set(PTR);
      break;
    }
    PTR = PTR->set_next;
    //printf("PTR while searching for the set index \n", PTR);
  }
  
  //we do not find the value of that index therefore create one set
  if ((PTR == NULL) && (CACHE->sets_count != cache_sets) ){
    printf("\n\n->Set_Node function is called to make a new node.\nAdding new set node to the set list\n");
    Set_Node(index);
    CACHE->sets_count = CACHE->sets_count + 1;
  }

 }
 // now we get the PTR of set_mtached node
return CACHE->SET_head;
}

void Line_Zero_Node(Pcache_line *Head_of_line, Pcache_line *Tail_of_line, unsigned tag){
  Pcache_line node = malloc(sizeof(struct cache_line_));
  node->line_next = NULL;
  node->line_prev = NULL;
  node->tag = tag;
  *Head_of_line = node;
  *Tail_of_line = node;
  printf("Line Zero Node\n");
  printf("The Address of new Line node is = %d\n", node);
  misses ++;
}

void Line_Node(Pcache_line *Head_of_line, Pcache_line *Tail_of_line, unsigned tag){
  Pcache_line node = malloc(sizeof(struct cache_line_));
  node->line_prev = NULL;
  node->line_next = *Head_of_line;
  node->tag = tag;
  (*Head_of_line)->line_prev = node;
  *Head_of_line = node; 
  printf("The Address of new Line node is = %d\n", node);
  misses ++;
}

void Arrange_Line(Pcache_line *KTR, Pcache_line *Head_of_line, Pcache_line *Tail_of_line){
  // if it the last node not the first node
  if ((*KTR == *Tail_of_line) && ((*KTR) != *Head_of_line)){
    printf("The tag matches on the last tail node\n");
    ((*KTR)->line_prev)->line_next = NULL;
    *Tail_of_line = (*KTR)->line_prev;
    (*KTR)->line_next = *Head_of_line;
    (*KTR)->line_prev = NULL;
    (*Head_of_line)->line_prev = *KTR;
    *Head_of_line = *KTR;
  }
  else if (*KTR == *Head_of_line){
    printf("The tag matches on the first node itself\n");
  }
  else{
    printf("The tag matched on any middle node\n");
    ((*KTR)->line_prev)->line_next = (*KTR)->line_next;
    ((*KTR)->line_next)->line_prev = (*KTR)->line_prev;
    (*KTR)->line_next = *Head_of_line;
    (*KTR)->line_prev = NULL;
    (*Head_of_line)->line_prev = *KTR;
    *Head_of_line = *KTR;
  }
}

void Delete_node(Pcache_line *Head_of_line, Pcache_line *Tail_of_line, unsigned tag){
  Pcache_line node = malloc(sizeof(struct cache_line_));
  node->tag = tag;
  node->line_prev = NULL;
  node->line_next = *Head_of_line;
  (*Head_of_line)->line_prev = node;
  *Head_of_line = node;
  ((*Tail_of_line)->line_prev)->line_next = NULL;
  Pcache_line temp = (*Tail_of_line);
  printf("The Address of last node which will be deleted is = %d\n", *Tail_of_line);
  *Tail_of_line = (*Tail_of_line)->line_prev;
  free(temp);
  printf("The new head has been inserted and last node has been deleted\n");
  misses++;
}


Pcache_line LINE(Pcache_set *address_of_set, unsigned tag){
  //set will store the addres of pointer PTR
  Pcache_set *SET = address_of_set;
  Pcache_line *address_of_LINE_head = &((*SET)->LINE_head);
  Pcache_line *address_of_LINE_tail = &((*SET)->LINE_tail);
  //pointer for traversing just line PTR so just copying the value
  Pcache_line KTR = *address_of_LINE_head;
  if (KTR == NULL){
    printf("Line_Zero_Node is called. As there are no lines in the set.\n");
    Line_Zero_Node(address_of_LINE_head, address_of_LINE_tail, tag);
    printf("LINE head address after update %d\n", *address_of_LINE_head);
    (*SET)->lines_count ++;
  }
  else{
    while(KTR != NULL){
      printf("\nTraversing for the tag match..........\n");
      if(KTR->tag == tag){
        printf("Tag matches. Arranging the line linked list, No need to create new node\n");
        hits ++;
        Arrange_Line(&KTR, address_of_LINE_head, address_of_LINE_tail);
        break;
      }
      KTR = KTR->line_next;
    }
    if ((KTR == NULL) && ((*SET)->lines_count < cache_lines)){
      printf("New cache node will be attached as the size of set line is available\n");
      Line_Node(address_of_LINE_head, address_of_LINE_tail, tag);
      (*SET)->lines_count ++;
    }
    else if (((KTR == NULL) && ((*SET)->lines_count == cache_lines))) {
      printf("Cache line overflow. The last node will be deleted\n");
      Delete_node(address_of_LINE_head, address_of_LINE_tail, tag);
    }
  }
  return *address_of_LINE_head;
}



void set_cache_param(param, value)
  int param;
  int value;
{

  switch (param) {
  case CACHE_PARAM_BLOCK_SIZE:
    cache_block_size = value;
    words_per_block = value / WORD_SIZE;
    break;
  case CACHE_PARAM_USIZE:
    cache_split = FALSE;
    cache_usize = value;
    break;
  case CACHE_PARAM_ISIZE:
    cache_split = FALSE;
    cache_isize = value;
    break;
  case CACHE_PARAM_DSIZE:
    cache_split = FALSE;
    cache_dsize = value;
    break;
  case CACHE_PARAM_ASSOC:
    cache_assoc = value;
    break;
  case CACHE_PARAM_WRITEBACK:
    cache_writeback = TRUE;
    break;
  /*case CACHE_PARAM_WRITETHROUGH:
    cache_writeback = FALSE;
    break;*/
  case CACHE_PARAM_WRITEALLOC:
    cache_writealloc = TRUE;
    break;
  /*case CACHE_PARAM_NOWRITEALLOC:
    cache_writealloc = FALSE;
    break;*/
  default:
    printf("error set_cache_param: bad parameter value\n");
    exit(-1);
  }

}
/************************************************************/
/************************************************************/


void init_cache()
{
  // pointer to cache
  Pcache cache_model = malloc(sizeof(struct cache_));
  //unified cache size
  cache_model->size = cache_usize;
  // making counter for sets zerp
  cache_model->sets_count = 0;      
  //no f sets
  cache_model->associativity = cache_assoc; 
  //global variable updated
  cache_model->sets = cache_usize/(cache_block_size*cache_assoc);  
  //global variable updated
  //offset_bits
  cache_model->SET_head = NULL;
  cache_model->SET_tail = NULL;

  //GLOBAL VARIABLES
  cache_lines = cache_model->associativity;
  cache_sets = cache_model->sets;
  CACHE = cache_model;
  printf("\n************************* THE CACHE HAS BEEN INITIALIZED *************************\n");

}

void initialization (unsigned index, unsigned tag);

void perform_access(addr, access_type)
  unsigned addr, access_type;
{
  printf("\n***************************** Address Calculations ********************************\n");
  /* handle an access to the cache */
  unsigned tag;
  unsigned index;
  unsigned index_bits, tag_bits;
  

  index_bits = LOG2(CACHE->sets);
  tag_bits = 32 - index_bits;
  
  //printf("\nThe offset_bits are    %d", offset_bits);
  printf("\nThe index_bits are     %d", index_bits);
  printf("\nThe tag_bits are       %d", tag_bits);
  
  index = addr & (int)(pow(2, index_bits)-1);
  tag = ((addr >> index_bits) & (int)(pow(2, tag_bits)-1));

  printf("\nThe index is           %x", index);
  printf("\nThe tag is             %x\n\n", tag);

  initialization(index, tag);
  accesses ++;

}

//creation of linked lists
void initialization (unsigned index, unsigned tag){
  printf("**********************Initialization function is called**************************** For Nth Trace %d\n", CACHE->sets_count + 1);
  //To find the set
  Pcache_set PTR = SET(index);
  //To find cache line of that set PTR
  Pcache_line KTR = LINE(&PTR, tag);
  printf("\n**********************************Addresses******************************************");
  printf("\nThe Address of SET node  %d\n", PTR);
  printf("The index of SET is %x\n", PTR->index);
  printf("The Address of LINE node %d\n", KTR);
  printf("The lines count in this SET node %d\n", PTR->lines_count);
  printf("The tag of this CACHELINE is %x\n\n", KTR->tag);

}

void dump_settings()
{
  printf("\n**************************** CACHE SETTINGS *************************************\n");
  //we are not providing split cache initially
  if (cache_split == TRUE) {
    printf("  Split I- D-cache\n");
    printf("  I-cache size: \t%d\n", cache_isize);
    printf("  D-cache size: \t%d\n", cache_dsize);
  } else {
    printf("  UNIFIED I-D CACHE\n");
    printf("  Size: \t\t%d\n", cache_usize);
  }
  printf("  Associativity: \t%d\n", cache_assoc);
  printf("  Block size: \t\t%d\n", cache_block_size);
  printf("  Write policy: \t%s\n", 
   cache_writeback ? "WRITE BACK" : "WRITE THROUGH");
  printf("  Allocation policy: \t%s\n\n",
   cache_writealloc ? "WRITE ALLOCATE" : "WRITE NO ALLOCATE");
}


/************************************************************/
void flush()
{

  /* flush the cache */
printf("Now Flusing the cache structure...........\nCAache is flushed!\n\n");
//free(CACHE);

}





/************************************************************/

/************************************************************/
void print_stats()
{
  printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ CACHE STATISTICS @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@2\n");
  printf("\nThe accesses are %d\n", accesses);
  printf("The Hits are %d\n", hits);
  printf("The misses are %d\n\n", misses);
  printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ END of A FETCH @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\n");

  
}
