
/*
 * cache.h
 */


#define TRUE 1
#define FALSE 0

//default cache parameters--can be changed
#define WORD_SIZE 4
#define WORD_SIZE_OFFSET 2
#define DEFAULT_CACHE_SIZE (8 * 1024)
#define DEFAULT_CACHE_BLOCK_SIZE 16
#define DEFAULT_CACHE_ASSOC 1
#define DEFAULT_CACHE_WRITEBACK TRUE
#define DEFAULT_CACHE_WRITEALLOC TRUE

//constants for settting cache parameters 
#define CACHE_PARAM_BLOCK_SIZE 0
#define CACHE_PARAM_USIZE 1
#define CACHE_PARAM_ISIZE 2
#define CACHE_PARAM_DSIZE 3
#define CACHE_PARAM_ASSOC 4
#define CACHE_PARAM_WRITEBACK 5
#define CACHE_PARAM_WRITETHROUGH 6
#define CACHE_PARAM_WRITEALLOC 7
#define CACHE_PARAM_NOWRITEALLOC 8


/* structure definitions */
typedef struct cache_line_ {
  unsigned tag;
  int dirty;

  struct cache_line_ *line_next;
  struct cache_line_ *line_prev;
} cache_line, *Pcache_line;

//modified cache set structure
typedef struct cache_set_{
  unsigned index;
  int lines_count;

  struct cache_set_ *set_next;
  struct cache_set_ *set_prev;
  Pcache_line LINE_head;
  Pcache_line LINE_tail;
} cache_set, *Pcache_set;

typedef struct cache_ {
  int size;			/* cache size */
  int associativity;		/* cache associativity */
  int sets;	
  Pcache_set SET_head;	/* head of LRU list for each set */
  Pcache_set SET_tail;	/* tail of LRU list for each set */
  int *set_contents;
  int sets_count;	/* number of valid entries in set */
  int contents;			/* number of valid entries in cache */
} cache, *Pcache;

typedef struct cache_stat_ {
  int accesses;			/* number of memory references */
  int misses;			/* number of cache misses */
  int replacements;		/* number of misses that cause replacments */
  int demand_fetches;		/* number of fetches */
  int copies_back;		/* number of write backs */
} cache_stat, *Pcache_stat;


/* function prototypes */
void set_cache_param();
void init_cache();
void perform_access();
void flush();
void delete();
void insert();
void dump_settings();
void print_stats();


/* macros */
#define LOG2(x) ((int) rint((log((double) (x))) / (log(2.0))))
