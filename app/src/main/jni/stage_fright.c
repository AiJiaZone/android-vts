
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <fcntl.h>
#include<signal.h>

#include <jni.h>

typedef int32_t     status_t;


void *(*graphicBufferConstructor)(void *object);


/*
status_t GraphicBuffer::unflatten(
        void const*& buffer, size_t& size, int const*& fds, size_t& count) {
*/
status_t (*graphicBufferUnflatten)(void const*& buffer, size_t& size, int const*& fds, size_t& count);


static void die(const char *msg)
{
        perror(msg);
        exit(errno);
}


static void * resolveSymbol(void * lib, char * symbol){
    void * r;
    if ((r = (void *)dlsym(lib, symbol)) == NULL)
       die("[-] dlsym");
    return r;
}

int process_media_file(const char *media_file) {
  void * libui = dlopen("libui.so",0);
  if(!libui){
    die("[-] dlopen failed");
  }

  stageFrightConstructor  = resolveSymbol(libstagefright, "_ZN7android28StagefrightMetadataRetrieverC1Ev");
  setDataSource           = resolveSymbol(libstagefright, "_ZN7android28StagefrightMetadataRetriever13setDataSourceEixx");
  extractMetaData         = resolveSymbol(libstagefright, "_ZN7android28StagefrightMetadataRetriever15extractMetadataEi");

  void * graphicBufferObject  = malloc(0x100);
  if(!graphicBufferObject){
     die("[-] no memory for object");
  }

  GraphicBufferConstructor(graphicBufferObject);

  int testPOC = open(media_file, 0xa << 12);
  if(testPOC < 0){
   die("[-] failed opening file");
  }
  errno = 0;
  status_t ret = setDataSource(metaDataReceiverObject, testPOC, 0ull,0x7FFFFFFFFFFFFFFull);
  if(ret){
    printf("[-] setDataSource = 0x%x\n", ret);
    die("[-] setDataSource");
  }
  ret = extractMetaData(metaDataReceiverObject, 12);
  printf("ret value %d\n", ret);

  return 0;
}

void sig_handler(int signo)
{
  printf("Boom goes the dynamite\n");
  fflush(stdout);
  exit(-1);
}

int main(int argc, char *argv[]){
   if(argc < 2){
     printf("Usage %s <media_file>", argv[0]);
     return -1;
   }

   struct sigaction action;
   bzero(&action, sizeof(struct sigaction));

   action.sa_handler = sig_handler;
   action.sa_mask = SA_RESTART;

   sigaction(SIGSEGV, &action, NULL);
   sigaction(SIGABRT, &action, NULL);
   sigaction(SIGBUS, &action, NULL);
   sigaction(SIGFPE, &action, NULL);
   sigaction(SIGILL, &action, NULL);
   sigaction(SIGPIPE, &action, NULL);
   sigaction(SIGTRAP, &action, NULL);

   printf("Running stagefright detector!\n");

   char * media_file = argv[1];
   process_media_file(media_file);

   return 0;
}

