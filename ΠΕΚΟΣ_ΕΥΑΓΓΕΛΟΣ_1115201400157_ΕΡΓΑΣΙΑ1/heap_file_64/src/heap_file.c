#include "heap_file.h"
#include "bf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }
  //Gia thn ektupwsh twn la8wn kai gia dieukolunsh xrhsimopoihsa thn CALL_OR_DIE pou dinetai sthn hp_main.c
  // anaferomai gia tis klhseis ton sunarthsewn pou sxetizontai me to epipedo block. Anti se kathe klhsh tetoias sunarthseis na elegxw
  //ton kwdiko la8ous pou epistrefei kai na kalw sthn BF_PrintError xrhsimopoiw thn CALL_OR_DIE h opoia kanei to idio dhladh elegxei
  //ti epistrefei h sunarthsh kai ektupwnei to ti la8os exoume mesw thn BF_PrintError. Epishs kanei ton kwdika pio euanagnwsto

HP_ErrorCode HP_Init() {
  // write your code here

  return HP_OK;
}

HP_ErrorCode HP_CreateIndex(const char *filename) {
  // write your code here
  BF_Block *fblock;
  int fd;
  char* data;

  CALL_OR_DIE(BF_CreateFile(filename));
  CALL_OR_DIE(BF_OpenFile(filename ,&fd));
  BF_Block_Init(&fblock);
  CALL_OR_DIE(BF_AllocateBlock(fd , fblock));
  data = BF_Block_GetData(fblock);
  memcpy(data , "Heap",sizeof("Heap"));
  BF_Block_SetDirty(fblock);
  CALL_OR_DIE(BF_UnpinBlock(fblock));
  BF_Block_Destroy(&fblock);
  CALL_OR_DIE(BF_CloseFile(fd));

  return HP_OK;
}

HP_ErrorCode HP_OpenFile(const char *fileName, int *fileDesc){
  // write your code here
  char* data;
  BF_Block *block;
  CALL_OR_DIE(BF_OpenFile(fileName, fileDesc));
  BF_Block_Init(&block);
  CALL_OR_DIE(BF_GetBlock(*fileDesc , 0 ,block));
  data = BF_Block_GetData(block);
  if (memcmp(data , "Heap" , sizeof("Heap"))) //metadata: Sto prwto block uparxei to string "HEAP" ws anagnwristiko heap file , an den prokeitai gia heap file return
  {
    BF_UnpinBlock(block);
    BF_Block_Destroy(&block);
    return HP_ERROR;
  }
  BF_UnpinBlock(block);                   //unpin efoson stamatame na to xrhsimopoioume
  BF_Block_Destroy(&block);               //kai apodesmeush


  return HP_OK;
}

HP_ErrorCode HP_CloseFile(int fileDesc) {
  // write your code here
  CALL_OR_DIE(BF_CloseFile(fileDesc));   //apla kaloume thn BF_CloseFile gia na kleisoume
  return HP_OK;
}

HP_ErrorCode HP_InsertEntry(int fileDesc, Record record) {
  // write your code here
  int counter = 0;
  int blocknum;
  BF_Block *ablock;
  char* data;
  int max_records;

  CALL_OR_DIE(BF_GetBlockCounter(fileDesc , &blocknum));
  BF_Block_Init(&ablock);                           //initialize to block
  if (blocknum == 1)
  {
    CALL_OR_DIE(BF_AllocateBlock(fileDesc , ablock));          //an uparxei mono ena block sto arxeio dhladh auto me ta metadata prepei na ftiaxoume kainourio
    data = BF_Block_GetData(ablock);              //opote kaloume thn BF_AllocateBlock gia na dhmiourghsei ena kainourio block
    counter = 1;
    memcpy(data , &counter ,sizeof(int));
    data += sizeof(int);                          //afou perasw ton counter pou deixnei poses eggrafes exei to block pernaw kai thn nea eggrafh kai epistrefw
    memcpy(data , &record , sizeof(Record));
    BF_Block_SetDirty(ablock);    //to kanw dirty epeidh ekana allages sto block kai prepei na enhmerw8ei prin to unpin
    CALL_OR_DIE(BF_UnpinBlock(ablock));      //unpin
    BF_Block_Destroy(&ablock);
    return HP_OK;
  }
  CALL_OR_DIE(BF_GetBlock(fileDesc , blocknum-1 , ablock));
  data = BF_Block_GetData(ablock);                        //sthn periptwsh pou uparxoun toulaxiston 2 blocks:
  memcpy(&counter , data , sizeof(int));
  max_records = (BF_BLOCK_SIZE - sizeof(int))/sizeof(Record);
  if (counter == max_records)                     //an to block einai gemato prepei na dhmiourghsoume neo block
  {
    counter =1;
    CALL_OR_DIE(BF_UnpinBlock(ablock));                      //unpin auto pou xrhsimopoioume
    CALL_OR_DIE(BF_AllocateBlock(fileDesc , ablock));        //dhmiourgia kainouriou block
    data = BF_Block_GetData(ablock);
    memcpy(data , &counter ,sizeof(int));       //enhmerwnoume ton counter gia thn prwth eggrafh
    data += sizeof(int);                        //metakinoume twn deikth na deixnei sthn prwth dia8esimh 8esh gia na valoume thn eggrafh
    memcpy(data , &record , sizeof(Record));    //insert thn eggrafh

  }
  else                                          //periptwsh pou uparxei xwros sto block
  {
    //data = BF_Block_GetData(ablock);
    memcpy(&counter , data , sizeof(int));              //enhmerwnoume ton counter gia thn nea eggrafh
    counter++;
    memcpy(data , &counter , sizeof(int));
    data += sizeof(Record)*(counter-1);   //metakinoyme ton deikth sthn prwth dia8esimh 8esh dhladh 4 bytes(logw counter)  + oso xwro pianoun oi eggrafes mas
    data += sizeof(int);                  //o counter einai meion ena epeidh ton aukshsa pio panw
    memcpy(data , &record , sizeof(Record));  //insert thn eggrafhs

  }
  BF_Block_SetDirty(ablock);    //to kanw dirty epeidh ekana allages sto block kai prepei na enhmw8ei prin to unpin
  CALL_OR_DIE(BF_UnpinBlock(ablock));      //unpin
  BF_Block_Destroy(&ablock);
  //CALL_OR_DIE(BF_CloseFile(fileDesc));

  return HP_OK;
}

HP_ErrorCode HP_PrintAllEntries(int fileDesc) {
  // write your code here
  int blocknum;
  int i ,j;
  BF_Block *block;
  int numbofentries;
  char *data;
  Record rec;

  CALL_OR_DIE(BF_GetBlockCounter(fileDesc , &blocknum));
  if (blocknum == 1)                                                    //an uparxei mono to prwto block den exoume kati na ektupwsoume!
  {
    printf("The are not any blocks besides the first one!\n");
    return HP_ERROR;
  }
  BF_Block_Init(&block);
  for (i=1; i<blocknum; i++) //gia kathe block sto arxeio....(ksekinaw apo to i=1 gia na mhn metrhsei to prwto block)
  {
    CALL_OR_DIE(BF_GetBlock(fileDesc , i , block));
    data = BF_Block_GetData(block);
    memcpy(&numbofentries , data ,sizeof(int));     //pernoume poses eggrafes exei
    data += sizeof(int);                            //metakinish tou deikth sthn prwth dia8esimh eggrafh
    for(j=0; j<numbofentries; j++)                  //gia ka8e eggrafh...
    {
      memcpy(&rec  , data , sizeof(Record));        //pername to record sthn metablhth mas
      printf("%d,\"%s\",\"%s\",\"%s\"\n" , rec.id , rec.name , rec.surname ,rec.city);  //ektupwnoume ka8e stoixeio
      data += sizeof(Record); //metakinoume ton deikth sthn epomenh eggrafh

    }
    CALL_OR_DIE(BF_UnpinBlock(block)); //unpin

  }
    BF_Block_Destroy(&block);

  return HP_OK;
}

HP_ErrorCode HP_GetEntry(int fileDesc, int rowId, Record *record) {
  // write your code here
  BF_Block *block;
  char* data;
  int block_to_go , modblock , blocknum;
  int max_records = (BF_BLOCK_SIZE - sizeof(int))/sizeof(Record);
  block_to_go = rowId/max_records;  //*akeraia diairesh gia na vrw to katallhlo block
  modblock = rowId%max_records;     //an exw kai upoloipo shmainei oti h eggrafh mou vrisketai sto epomeno block apo auto pou deixnei to block_to_go
                                    //dld estw oti exoume 20 eggrafes ana block kai theloume thn 87: to block_to_go tha einai 4 kai to upoloipo diaforo tou 0
                                    // ara h eggrafh mas vrisketai sto block_to_go++ = 5 kai einai h 87mod20 = 7 eggrafh (8ewrwntas ws block 0 to prwto block)
  BF_Block_Init(&block);
  CALL_OR_DIE(BF_GetBlockCounter(fileDesc , &blocknum));
  if (block_to_go +1 > blocknum)    //to if to kanw apla gia na elegxw an zhtaw mia eggrafh pou vrisketai se block to opoio den uparxei
  {                                // kanw + gia na suberilavw kai to prwto block to opoio den exei eggrafes mesa epeidh ksekinaei apo to 0
    return HP_ERROR;
  }
  if (modblock != 0)                              //edw h periptwsh tou na mhn theloyme thn teleutaia eggrafh
  {
    CALL_OR_DIE(BF_GetBlock(fileDesc ,block_to_go + 1  ,block));
    data = BF_Block_GetData(block);
    data += sizeof(int) + (modblock-1)*sizeof(Record);  //metakinw ton deikth sthn eggrafh pou 8eloume
    memcpy(record , data , sizeof(Record)); //copy
    CALL_OR_DIE(BF_UnpinBlock(block)); //unpin
    BF_Block_Destroy(&block);
    return HP_OK;
  }
  else if(modblock == 0)                          //teleutaia eggrafh
  {
    CALL_OR_DIE(BF_GetBlock(fileDesc ,block_to_go ,block));
    data = BF_Block_GetData(block);
    data += sizeof(int) + (max_records-1)*sizeof(Record);
    memcpy(record , data , sizeof(Record));
    CALL_OR_DIE(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    return HP_OK;
  }
  else
    return HP_ERROR;



  return HP_OK;
}
