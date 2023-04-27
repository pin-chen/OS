#include <ramdisk.h>

#include <string.h>

#include <utils.h>
#include <uart.h>
#include <allocator.h>

void *_randisk_start_addr;
void *_randisk_end_addr;
int isinit;

int ramdisk_get_addr(){
    _randisk_start_addr = (void*) 0x8000000;
    //_randisk_end_addr = (void*) ;
    return 0;
}

cpio_file* cpio_parse(){
    if(!isinit){
        int initres = ramdisk_get_addr();
        if(!initres) isinit = 1;
        else return 0;
    }
    char *ptr = (char *) _randisk_start_addr;
    char *name = ptr + sizeof(struct cpio_newc_header);
    struct cpio_newc_header* cur_cpio_header;
    cpio_file *head = NULL;
    cpio_file *prev = NULL;
    cpio_file *cur = NULL;
    while(1){ //(uint64_t)ptr < (uint64_t)INITRD_ADDRESS_END
        cur_cpio_header = (struct cpio_newc_header*) ptr;
        name = ptr + sizeof(struct cpio_newc_header);
        if(strcmp("TRAILER!!!", name) == 0) break;
        cur = (cpio_file*) simple_malloc (sizeof(cpio_file));
        if(head == NULL) head = cur;
        cur->header = ptr;
        cur->namesize = hex2u32_8(cur_cpio_header->c_namesize);
        cur->filesize = hex2u32_8(cur_cpio_header->c_filesize);
        cur->name = name;
        cur->content = (char*) (((((uint64_t) name + cur->namesize - 1) >> 2) + 1) << 2);
        cur->next = 0;
        ptr = (char *) (((((uint64_t) cur->content + cur->filesize - 1) >> 2) + 1) << 2);
        if(prev != NULL) prev->next = cur;
        prev = cur;
    }
    return head;
}