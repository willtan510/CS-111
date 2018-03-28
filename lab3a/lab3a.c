#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "ext2_fs.h"

#define EXT2_S_IFLNK 0xA000
#define EXT2_S_IFREG 0x8000
#define EXT2_S_IFDIR 0x4000

int fsFD;
struct ext2_super_block bigBlock;  
struct ext2_group_desc groupInfo;
int fatal;

void supBlock()
{
  
  if (pread(fsFD,&bigBlock,sizeof(struct ext2_super_block),1024) < 0)
  {
      fatal = errno;
      fprintf(stderr, "%s", strerror(fatal));
      exit(2);
  }
  
  fprintf(stdout,"SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n",bigBlock.s_blocks_count , bigBlock.s_inodes_count, 1024<<bigBlock.s_log_block_size ,bigBlock.s_inode_size, bigBlock.s_blocks_per_group,bigBlock.s_inodes_per_group,bigBlock.s_first_ino);
}

void group()
{
  pread(fsFD,&groupInfo,sizeof(struct ext2_group_desc),1024+sizeof(struct ext2_super_block));
  
  fprintf(stdout,"GROUP,0,%d,%d,%d,%d,%d,%d,%d\n",bigBlock.s_blocks_count,bigBlock.s_inodes_count,groupInfo.bg_free_blocks_count,groupInfo.bg_free_inodes_count,groupInfo.bg_block_bitmap,groupInfo.bg_inode_bitmap,groupInfo.bg_inode_table ); //missing 3,4,9
}

void freeBlocks()
{
  for(int i=0;i<1024;i++)  //0 to blocksize
  {
    int byte;
    pread(fsFD,&byte,1,1024*groupInfo.bg_block_bitmap+i);
    int bit=1;
    for(int j=0;j<8;j++)
    {
      if((byte&(1<<j))>>j==0)  //if bit is free
      {
        fprintf(stdout,"BFREE,%d\n",(i*8)+(j+1));  //account for byte # and bit #
      }
    }
  }
}

void freeInodes()
{
    for (int i = 0; i < 1024; i++)
    {
        int byte;
        pread(fsFD, &byte,1,1024*groupInfo.bg_inode_bitmap+i);
        for (int j = 0; j < 8; j++)
        {
            if ((byte&(1<<j))>>j==0)
            {
                fprintf(stdout,"IFREE,%d\n",(i*8)+(j+1));
            }
        }
    }
}

void inodeSummary()
{
  struct ext2_inode node;
  
  for(int i=0;i<bigBlock.s_inodes_count;i++)
  {
    pread(fsFD,&node,sizeof(struct ext2_inode),5*1024+i*sizeof(struct ext2_inode));
    if(node.i_mode==0||node.i_links_count==0)
      continue;
    else 
    {
      char fileType;
      if(node.i_mode&EXT2_S_IFREG)
        fileType='f';
      else if(node.i_mode&EXT2_S_IFDIR)
        fileType='d';
      else if(node.i_mode&EXT2_S_IFLNK)
        fileType='s';
      else
        fileType='?';
      
      int mask=(~(-1<<12));
      int mode=mask&node.i_mode;  
      int ownerID=node.i_uid;
      int groupID=node.i_gid;
      int linkCount=node.i_links_count;
      
      int createTime=node.i_ctime;
      char createString[30];
      int modTime=node.i_mtime;
      char modString[30];
      int accessTime=node.i_atime;
      char accessString[30];
      
      time_t total=createTime;
      struct tm timeStruct=*gmtime(&total);
      strftime(createString,30,"%m/%d/%y %H:%M:%S", &timeStruct);
      total=modTime;
      timeStruct=*gmtime(&total);
      strftime(modString,30,"%m/%d/%y %H:%M:%S", &timeStruct);
      total=accessTime;
      timeStruct=*gmtime(&total);
      strftime(accessString,30,"%m/%d/%y %H:%M:%S", &timeStruct);
      
      int fSize=node.i_size;
      int numBlocks=node.i_blocks;
      
      fprintf(stdout,"INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d",i+1,fileType,mode,ownerID,groupID,linkCount,createString,modString,accessString,fSize,numBlocks);
      for(int j=0;j<15;j++)
        fprintf(stdout,",%d",node.i_block[j]);
      fprintf(stdout,"\n");
    }
  }
}

int main (int argc, char** argv)
{
  if(argc!=2)
  {
    fprintf(stderr,"Error: incorrect number of command-line arguments\n");
    exit(1);
  }
  
  fsFD=open(argv[1],O_RDONLY);
  if(fsFD<0)
  {
    fatal=errno;
    fprintf(stderr,"%s\n",strerror(fatal));
    exit(1);
  }
  //following lines, idk yet
  supBlock();
  group();
  freeBlocks();
  freeInodes();
  inodeSummary();
}