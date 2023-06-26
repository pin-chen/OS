#include <allocator.h>

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <utils.h>
#include <uart.h>
#include <devicetree.h>

#define FRAME_SIZE 0x1000
#define LIST_SIZE 17
#define F -1
#define X -2

typedef struct NODE_{
    NODE_ *parent;
    NODE_ *left;
    NODE_ *right;
    NODE_ *next;
    NODE_ *prev;
    int index;
    int val;

} NODE;

NODE* free_lists[LIST_SIZE + 1];

NOSE* binary_tree[4];

extern uint32_t _heap_begin;
extern uint32_t _heap_end;

extern uint32_t _image_memory_begin;
extern uint32_t _image_memory_end;

void *mem_chunk_begin;
void *mem_chunk_end;

int check;

void *simple_malloc(size_t size){
    if(check != 1){
        check = 1;
        mem_chunk_begin = (void *) &_heap_begin;
        mem_chunk_end = (void *) &_heap_end;
    }
    size = (((size - 1) >> 4) + 1) << 4;
    if((uint64_t) mem_chunk_begin + size >= (uint64_t) mem_chunk_end){
        return NULL;
    }
    void *chunk = mem_chunk_begin;
    mem_chunk_begin = (void *)((uint64_t) mem_chunk_begin + size);
    return chunk;
}

///
typedef struct{
    int found;
    char *initrd_start;
    char *initrd_end;
} ramdisk_data;

int ramdisk_fdt_callback(char* node, const char *name, void *data){
    if(strcmp(name, "chosen")) return 0;
    ramdisk_data* _data = (ramdisk_data*) data;
    fdt_prop *prop;
    while(prop = fdt_nextprop(node + 1, &node)){
        if(!strcmp(prop->name,"linux,initrd-start")){
            _data->initrd_start = (char *)(uint64_t)ntohl(*(uint32_t*)prop->value);
        }
        if(!strcmp(prop->name,"linux,initrd-end")){
            _data->initrd_end = (char *)(uint64_t)ntohl((uint32_t*)prop->value);
        }
    }
    _data->found = 1;
    return 1;
}

int ramdisk_reserve(){
    ramdisk_data data;
    data.initrd_start = 0;
    data.initrd_end = 0;
    data.found = 0;
    fdt_traverse(ramdisk_fdt_callback, (void *)&data);
    if(data.found){
        memory_reserve(data.initrd_start, data.initrd_end);
        return 0;
    }
    return -1;
}
///

int log_2(uint64_t num){
    int count = -1;
    while(num > 0){
        num >>= 1;
        count++;
    }
    return count;
}

///

void malloc_init(){
    for(int i = 0; i < LIST_SIZE + 1; i++){
        free_lists[i] = NULL;
    }
    buddy_system_init(0, 0x00, 0x20000000);
    buddy_system_init(1, 0x20000000, 0x30000000);
    buddy_system_init(2, 0x30000000, 0x38000000);
    buddy_system_init(3, 0x38000000, 0x3c000000);

    memory_reserve(0x0000, 0x1000); // 1
    memory_reserve(_image_memory_begin, _image_memory_end);// 2, 5
    ramdisk_reserve(); // 3
    if((uint64_t) _devicetree_begin != 0xffffffff){ // 4
        fdt_header *header = (fdt_header *) _devicetree_begin;
        uint64_t _devicetree_end = (uint64_t) _devicetree_begin + ntohl(header->totalsize);
        memory_reserve((uint64_t)_devicetree_begin, (uint64_t) _devicetree_end);
    }
}

void list_insert(int i, NODE* tmp){
    if(free_lists[i] == NULL){
        free_lists[i] = tmp;
    }else{
        tmp->next = free_lists[i];
        free_lists[i]->prev = tmp;
        free_lists[i] = tmp;
    }
}

NODE *list_pop(int i){
    NODE *tmp = free_lists[i];
    free_lists[i] = tmp->next;
    free_lists[i]->prev = NULL;
    return tmp;
}

NODE *node_split(NODE* tmp){
    tmp->left = simple_malloc(size(NODE));
    tmp->right = simple_malloc(size(NODE));

    tmp->left->parent = tmp;
    tmp->left->left = NULL;
    tmp->left->right = NULL;
    tmp->left->next = NULL;
    tmp->left->prev = NULL;
    tmp->left->index = tmp->index;
    tmp->left->val = tmp->val - 1;

    tmp->right->parent = tmp;
    tmp->right->left = NULL;
    tmp->right->right = NULL;
    tmp->right->next = NULL;
    tmp->right->prev = NULL;
    tmp->right->index = tmp->index + (0x1 << (tmp->val - 1));
    tmp->right->val = tmp->val - 1;

    list_insert(tmp->right->val, tmp->right);
    return tmp->left;
}

void *phy_addr(NODE* tmp){
    return (void *) (uint64_t) (tmp->index * FRAME_SIZE);
}

void buddy_system_init(int i, uint64_t begin, uint64_t end) {
    binary_tree[i] = simple_malloc(size(NODE));
    binary_tree[i]->parent = NULL;
    binary_tree[i]->left = NULL;
    binary_tree[i]->right = NULL;
    binary_tree[i]->next = NULL;
    binary_tree[i]->prev = NULL;
    binary_tree[i]->index = begin / FRAME_SIZE;
    binary_tree[i]->val = log_2(end - begin);
    list_insert(binary_tree[i]->val, binary_tree[i]);
}

void *buddy_system_malloc(size_t size){
    const int o_power_size = log_2(size);
    int power_size = o_power_size;
    while(1){
        if(free_lists[power_size] == NULL){
            power_size++;
            continue;
        }
        NODE *tmp = list_pop(power_size);
        while(o_power_size < power_size){
            tmp = node_split(tmp);
            power_size--;
        }
        return phy_addr(tmp);
    }
}

void buddy_system_free(){

}

void memory_reserve(uint64_t begin, uint64_t end) {
    //…
}
