#ifndef _RVM_H_
#define _RVM_H_


/*struct _rvm_t
{ 
  char path[128];
  char* redo_log;
};*/ 

typedef int trans_t;


typedef const char* rvm_t;

//typedef struct _rvm_t *rvm_t;

struct map_struct
{
  const char* segname;
  int size;
  void* v_addr;
  bool being_modified;
  int tid;
  rvm_t rvm;
};

struct atm_struct
{
  struct map_struct* seg_map;
  int offset;
  int size;
  void* undo_log;
};

rvm_t rvm_init(const char *dir);
void *rvm_map(rvm_t rvm, const char*segname, int size_to_create);

void rvm_unmap(rvm_t rvm, void *segbase);

void rvm_destroy(rvm_t rvm, const char* segname);


trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases);

void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size);

void rvm_commit_trans(trans_t tid);
void rvm_abort_trans(trans_t tid);
void rvm_truncate_log(rvm_t rvm);
void rvm_verbose(int enable_flag);

#endif
