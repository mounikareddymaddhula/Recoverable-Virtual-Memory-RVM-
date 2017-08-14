#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <map>
#include <iostream>
#include <vector>
#include <dirent.h>
#include "rvm.h"

using namespace std;

#define MAX_PATH_SIZE 128
int tid =-1;

static map<const char*, struct map_struct*> hash_of_segments;
static map<void*, struct map_struct*> hash_of_vaddr;
vector<struct atm_struct> v_atm;
static map<trans_t, vector<struct atm_struct> > hash_of_tid;

bool verbose_flag = true;

rvm_t rvm_init(const char *dir)
{
  if(verbose_flag)
    printf("Line %d : %s : start\n",__LINE__,__func__);
  rvm_t rvm;

  struct stat st;
  if (stat(dir, &st)==-1)
  {
    int r = mkdir( dir,0777);
    if (!r)
    {
      if(verbose_flag) printf("mkdir succeeded\n"); 
    }
    else
    {
      printf("mkdir failed \n");
      return NULL;
    }
  }
  rvm = dir;

  if(verbose_flag)
    printf("Line %d : %s end\n",__LINE__,__func__);
  return rvm;
}



void *rvm_map(rvm_t rvm, const char*segname, int size_to_create)
{
  if(verbose_flag)
    printf("Line %d : %s : start\n",__LINE__,__func__);
  if(segname == NULL)
  {
    printf("ERROR: Segment name is null\n");
    return (void*)-1;
  }

  if(hash_of_segments.find(segname) != hash_of_segments.end())
  {
    printf("ERROR: Segment already mapped\n");
    //return (void*)-1;
    exit(1);
  }
  struct map_struct* seg = (struct map_struct*)malloc(sizeof(map_struct));
  seg->size = size_to_create;
  seg->segname = segname;
  void* seg_base = malloc(size_to_create);
  seg->v_addr = seg_base;
  seg->being_modified = false;
  seg->rvm = rvm;
  char* dir = (char*) rvm;
  char *filepath = (char*) malloc(MAX_PATH_SIZE*2+1);
  strcpy(filepath,dir);
  strcat(filepath,"/");
  strcat(filepath,segname);
  struct stat st;
  if(stat(filepath, &st) != 0)
  {
    //File does not exist

    /*int fd = open(filepath,"w");
    int file_lock = flock(fileno(fd),LOCK_EX);
    if(file_lock == 0) printf("Locked successfully\n");
    close(fd);*/
    if(verbose_flag)
      printf("Creating segment file for map\n");
    FILE* fp = fopen(filepath,"w");
    //int file_lock = flock(fileno(fp),LOCK_EX);
    /*if(file_lock != 0)
    {
      printf("segment already mapped to another thread/process\n");
      return NULL;
    }*/
    fclose(fp);
    truncate(filepath, size_to_create);
    
  }
  else
  {
    //File exists
    int existing_filesize = st.st_size;
    truncate(filepath, size_to_create);

    /*int fd = open(filepath,"r");
    int file_lock = flock(fileno(fd),LOCK_EX);
    if(file_lock == 0) printf("Locked successfully\n");
    close(fd);*/
    if(verbose_flag)
      printf("Mapping existing segment file\n");
    FILE* fp = fopen(filepath,"r");
    /*if(file_lock != 0)
    {
      printf("segment already mapped to another thread/process\n");
      return NULL;
    }*/
    fread(seg->v_addr,size_to_create,1,fp);
    fclose(fp);
    char *filepath2 = (char*) malloc(MAX_PATH_SIZE*2+5);
    strcpy(filepath2,dir);
    strcat(filepath2,"/");
    strcat(filepath2, "_log");
    strcat(filepath2,segname);
    if(stat(filepath2, &st) == 0)
    {
      FILE *fp2 = fopen(filepath2, "r");
      int size_of_segname;
      fread(&size_of_segname, sizeof(int), 1, fp2);
      char *throw_away = (char*)malloc(size_of_segname);
      fread(throw_away,size_of_segname,1,fp2);
      free(throw_away);
      while(!feof(fp2))
      { 
        int offset, size;
        fread(&offset, sizeof(int), 1, fp2);
        fread(&size, sizeof(int), 1, fp2);
        fread((char*)seg->v_addr + offset,1,size,fp2);
      }
      fclose(fp2);
    }
  }
  hash_of_segments.insert ( std::pair<const char*, struct map_struct*>(segname, seg));
  hash_of_vaddr.insert( std::pair<void*, struct map_struct*>(seg->v_addr, seg));
  free(filepath);
  if(verbose_flag)
    printf("Line %d : %s end\n",__LINE__,__func__);
  return seg->v_addr;
}

void rvm_unmap(rvm_t rvm, void *segbase)
{
  if(verbose_flag)
    printf("Line %d : %s : start\n",__LINE__,__func__);
  if(hash_of_vaddr.find(segbase) == hash_of_vaddr.end())
  {
    printf("ERROR: segbase not mapped\n");
    return;
  }

  struct map_struct* m_temp = hash_of_vaddr.find(segbase)->second;

  if (hash_of_segments.find(m_temp->segname) != hash_of_segments.end())
  {
    if(verbose_flag)
      printf("Unmapping segment file\n");
    hash_of_segments.erase(m_temp->segname);
  }
  else
  {
    printf("ERROR: segment not mapped\n");
    return;
  }

  hash_of_vaddr.erase(segbase);

  free(m_temp);
  free(segbase);
  if(verbose_flag)
    printf("Line %d : %s end\n",__LINE__,__func__);
}

void rvm_destroy(rvm_t rvm, const char* segname)
{
  if(verbose_flag)
    printf("Line %d : %s : start\n",__LINE__,__func__);
  if(hash_of_segments.find(segname) != hash_of_segments.end())
  {
    printf("ERROR: Segment cannot be destroyed as it is still mapped\n");
    return;
  }
  char* dir = (char*) rvm;
  char* filepath = (char*) malloc(MAX_PATH_SIZE*2+1);
  strcpy(filepath,dir);
  strcat(filepath,"/");
  strcat(filepath,segname);

  int status = remove(filepath);
  if(!status)
  {
    if(verbose_flag)
      printf("Success!! File destroyed \n");
  }
  else
  {
    printf("Error!! File not destroyed or file does not exist \n");
  }
  free(filepath);
  if(verbose_flag)
    printf("Line %d : %s end\n",__LINE__,__func__);
  return;
}



trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases)
{
  if(verbose_flag)
    printf("Line %d : %s : start\n",__LINE__,__func__);
  for (int i = 0; i < numsegs; i++)
  {
     if (hash_of_vaddr.find(segbases[i]) == hash_of_vaddr.end() || (hash_of_vaddr.find(segbases[i])->second->being_modified == true))
     {
        printf("ERROR: Segment not in memory or the transaction has already begin \n");
        return -1;
     }
   }
   tid++;
   for (int i = 0; i < numsegs; i++)  
   {   
      hash_of_vaddr.find(segbases[i])->second->being_modified = true; 
      hash_of_vaddr.find(segbases[i])->second->tid = tid;
    }

    vector<struct atm_struct> val_tid;
    hash_of_tid.insert (std::pair<trans_t, vector<struct atm_struct> >(tid, val_tid));
   
   if(verbose_flag)
    printf("Line %d : %s end\n",__LINE__,__func__);
   return tid;
}

void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size)
{
  if(verbose_flag)
    printf("Line %d : %s : start\n",__LINE__,__func__);
  if (hash_of_vaddr.find(segbase)->second->v_addr != segbase)
  {
    printf("Segment not mapped, cannot be modified \n");
    return;
  }
  if (hash_of_vaddr.find(segbase) == hash_of_vaddr.end())
  {
    printf("Segment not found in hash-map \n");
    return;
  }

  if (hash_of_vaddr.find(segbase)->second->tid != tid)
  {
    printf("ERROR: Not a valid transaction \n");
    return;
  }
  if (offset + size > hash_of_vaddr.find(segbase)->second->size)
  {
    printf("Trying to access out of segment \n");
    return;
  }

  struct atm_struct atm;
  atm.offset = offset;
  atm.size = size;
  atm.seg_map = hash_of_vaddr.find(segbase)->second;
  if(verbose_flag)
      printf("Creating undo logs\n");
  atm.undo_log = (void*)malloc(size);
  for(int i=0; i < size; i++)
   *((char*)atm.undo_log+i) = *((char*)segbase+offset+i);
  if (hash_of_tid.find(tid) != hash_of_tid.end())
  {
    hash_of_tid.find(tid)->second.push_back(atm);
  }
  else
  {
    vector<struct atm_struct> val_tid;
    val_tid.push_back(atm);
    hash_of_tid.insert (std::pair<trans_t, vector<struct atm_struct> >(tid, val_tid));
  } 
  if(verbose_flag)
    printf("Line %d : %s end\n",__LINE__,__func__);
}



void rvm_commit_trans(trans_t tid)
{
  if(verbose_flag)
    printf("Line %d : %s : start\n",__LINE__,__func__);
  for(int i=0; i < hash_of_tid.find(tid)->second.size(); i++)
  {
    const char* dir = hash_of_tid.find(tid)->second[i].seg_map->rvm;
    char *filepath = (char*) malloc(MAX_PATH_SIZE*2+5);
    strcpy(filepath,dir);
    strcat(filepath,"/");
    strcat(filepath, "_log");
    strcat(filepath,hash_of_tid.find(tid)->second[i].seg_map->segname);
    
    struct stat st;
    FILE* fp;
    if(verbose_flag)
      printf("Create or append to redo log\n");
    if(stat(filepath, &st) != 0)
    {
      //File does not exist
      fp = fopen(filepath,"w");
      int size_of_segname = strlen(hash_of_tid.find(tid)->second[i].seg_map->segname);
      fwrite(&size_of_segname, sizeof(int), 1, fp);
      fwrite(hash_of_tid.find(tid)->second[i].seg_map->segname,size_of_segname,1,fp);
    }
    else
    {
      //File exists
      fp = fopen(filepath,"a");
    }
    //for (int j = 0; j < hash_of_tid.find(tid)->second.size(); j++) 
    //{
      //fwrite(hash_of_tid.find(tid)->second[j]->segmap->v_addr, 8, 1, fp);
    fwrite(&(hash_of_tid.find(tid)->second[i].offset), sizeof(int), 1, fp);
    fwrite(&(hash_of_tid.find(tid)->second[i].size), sizeof(int), 1, fp);
    fwrite((char*)(hash_of_tid.find(tid)->second[i].seg_map->v_addr)+(hash_of_tid.find(tid)->second[i].offset), (hash_of_tid.find(tid)->second[i].size), 1, fp);
    fclose(fp);
    //}
    hash_of_tid.find(tid)->second[i].seg_map->being_modified = false;
    //hash_of_tid.erase(tid);//Delete undo record at commit
    free(hash_of_tid.find(tid)->second[i].undo_log);
    free(filepath);
  }
  hash_of_tid.erase(tid);//Delete undo record at commit
  if(verbose_flag)
    printf("Line %d : %s end\n",__LINE__,__func__);
}

void rvm_abort_trans(trans_t tid)
{
  if(verbose_flag)
    printf("Line %d : %s : start\n",__LINE__,__func__);
  for(int i = hash_of_tid.find(tid)->second.size() - 1; i>=0; i--)
  {
    for(int k=0; k < hash_of_tid.find(tid)->second[i].size; k++)
       *((char*)(hash_of_tid.find(tid)->second[i].seg_map->v_addr)+hash_of_tid.find(tid)->second[i].offset+k) = *((char*)hash_of_tid.find(tid)->second[i].undo_log+k);
    hash_of_tid.find(tid)->second[i].seg_map->being_modified = false;
    free(hash_of_tid.find(tid)->second[i].undo_log);
  }
  hash_of_tid.erase(tid);
  if(verbose_flag)
    printf("Line %d : %s end\n",__LINE__,__func__);
}


void rvm_truncate_log(rvm_t rvm)
{
  if(verbose_flag)
    printf("Line %d : %s : start\n",__LINE__,__func__);
  const char* dir = rvm;
  char *filepath = (char*) malloc(MAX_PATH_SIZE*2+5);
  strcpy(filepath,dir);
  DIR *dirp = opendir(filepath);
  struct dirent* in_file;
  while ((in_file = readdir(dirp)))
  {
    if(!strncmp (in_file->d_name, "_log", 4))
    {
      char *filepath3 = (char*) malloc(MAX_PATH_SIZE*2+5);
      strcpy(filepath3,dir);
      strcat(filepath3,"/");
      strcat(filepath3,in_file->d_name);
      FILE* fp = fopen(filepath3,"r");
      fseek(fp,0,SEEK_END);
      int redolog_size =  ftell(fp);
      fclose(fp);
      if(verbose_flag)
        printf("Read the redo log\n");
      fp = fopen(filepath3,"r");
      char *redoinmemory = (char*) malloc(redolog_size);
      int redo_read_offset = 0;
      fread(redoinmemory,redolog_size,1,fp);
      fclose(fp);

      int segment_name_size = *((int*)redoinmemory);
      redo_read_offset += sizeof(int);
      char *segname = (char*)malloc(segment_name_size);
      for(int i=0; i<segment_name_size; i++)
        segname[i] = *((char*)(redoinmemory + redo_read_offset) + i);
      redo_read_offset += segment_name_size;
      segname[segment_name_size] = '\0';

      char *filepath2 = (char*) malloc(MAX_PATH_SIZE*2+1);
      strcpy(filepath2,dir);
      strcat(filepath2,"/");
      strcat(filepath2,segname);
      FILE *fpseg = fopen(filepath2,"r");
      if(!fpseg) continue;
      fseek(fpseg,0,SEEK_END);
      int seg_size =  ftell(fpseg);
      fclose(fpseg);
      if(verbose_flag)
        printf("Read the segment file\n");
      fpseg = fopen(filepath2,"r");
      char *seginmemory = (char*) malloc(seg_size);
      fread(seginmemory,seg_size,1,fpseg);
      fclose(fpseg);
      //cout << "truncate segname is " << segname << endl;

      if(verbose_flag)
        printf("Apply operations from redo log to segment on memory\n");
      while(redo_read_offset < redolog_size )
      {
        int offset, size;
        offset = *((int*)(redoinmemory + redo_read_offset));
        redo_read_offset += sizeof(int);
        size = *((int*)(redoinmemory + redo_read_offset));
        redo_read_offset += sizeof(int);
        //char *temp = (char*)malloc(size);

        for(int i=0; i<size;i++)
          seginmemory[offset+i] = *((char*)(redoinmemory + redo_read_offset) + i);
        redo_read_offset += size;
      }

      if(verbose_flag)
        printf("Write the segment to disk\n");
      FILE *fpseg2 = fopen(filepath2,"w+");
      fwrite(seginmemory,1,seg_size,fpseg2);
      fclose(fpseg2);

      remove(filepath3);
      free(filepath3);
      free(redoinmemory);
      free(segname);
      free(seginmemory);
    } 
  }
  free(filepath);
  if(verbose_flag)
    printf("Line %d : %s end\n",__LINE__,__func__);
}

void rvm_verbose(int enable_flag)
{
  verbose_flag = (bool)enable_flag;
}


