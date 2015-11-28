#include "sys.h"
/*--- Core includes ---*/
#include "core/util.h"
/*--- Emu includes ---*/
#include "emu/bios/bios.h"
#include "emu/bios/bios_internal.h"
#include "emu/bios/mem.h"
#include "emu/mem.h"

typedef struct MemNode MemNode;
struct MemNode {
  MemNode *next;
  MemNode *prev;
  size_t size;
  uint16 para;
  BOOL in_use;
};

static MemNode *s_nodes = NULL;

static void prv_debug(const char *hdr) {
  return;
  MemNode *node = s_nodes;
  printf("[%s]\n", hdr);
  for(node = s_nodes; node != NULL; node = node->next) {
    printf("Node @ 0x%04X, %sin use, 0x%04zX paragraphs\n", node->para, node->in_use ? "" : "not ", node->size);
  }
  printf("[/%s]\n", hdr);
}

static BOOL prv_alloc(MemNode *entry, size_t size) {
  MemNode *newent = NULL;
  if(size < entry->size) {
    newent = calloc(1, sizeof(MemNode));
    newent->next = entry->next;
    newent->prev = entry;
    newent->size = entry->size - size;
    newent->para = entry->para + size;
    newent->in_use = FALSE;
    entry->size = size;
    if(entry->next)
      entry->next->prev = newent;
    entry->next = newent;
  }else if(size != entry->size) {
    return FALSE;
  }
  entry->in_use = TRUE;
  return TRUE;
}

static void prv_condense_list(void) {
  prv_debug("pre condense");
  MemNode *list, *next;
  for(list = s_nodes; list->next != NULL; list = next) {
    next = list->next;
    if(list->in_use || next->in_use)
      continue;
    if(list->para + list->size == next->para) {
      list->size += next->size;
      if(next->next)
        next->next->prev = list;
      list->next = next->next;
      free(next);
      next = list;
    }
  }
  prv_debug("post condense");
}

void bios_mem_init(void) {
  MemNode *entry = calloc(1, sizeof(MemNode));
  entry->size = _mem_size >> 4;
  entry->para = 0;
  entry->prev = NULL;
  entry->next = s_nodes;
  entry->in_use = FALSE;
  s_nodes = entry;
}

void bios_mem_finish(void) {
  MemNode *ptr;
  while(s_nodes) {
    ptr = s_nodes;
    s_nodes = s_nodes->next;
    free(ptr);
  }
}

uint8 bios_mem_alloc(uint16 size, uint16 *ptr) {
  MemNode *list = NULL;
  prv_condense_list();
  for(list = s_nodes; list != NULL; list = list->next) {
    if(list->in_use)
      continue;
    if(size <= list->size) {
      if(prv_alloc(list, size)) {
        break;
      }
    }
  }
  prv_debug("post alloc");
  if(list == NULL) {
    return DOSERR_OUT_OF_MEMORY;
  }
  *ptr = list->para;
  return DOSERR_NONE;
}

static MemNode *prv_find_node_by_para(uint16 para) {
  MemNode *list = NULL;
  for(list = s_nodes; list != NULL; list = list->next) {
    if(!list->in_use)
      continue;
    if(list->para == para)
      break;
  }
  return list;
}

uint8 bios_mem_free(uint16 para) {
  MemNode *list = prv_find_node_by_para(para);
  if(list == NULL)
    return DOSERR_MBA_INVALID;

  list->in_use = FALSE;

  prv_debug("post free");
  return DOSERR_NONE;
}

uint8 bios_mem_resize(uint16 size, uint16 para) {
  prv_condense_list();

  MemNode *list = prv_find_node_by_para(para);
  if(list == NULL)
    return DOSERR_MBA_INVALID;

  int32 size_diff = size - list->size;
  if(size_diff > 0) {
    if(list->next->in_use) {
      return DOSERR_OUT_OF_MEMORY;
    }
    if(list->next->size < (uint32)size_diff) {
      return DOSERR_OUT_OF_MEMORY;
    }
    list->next->size -= size_diff;
    list->next->para += size_diff;
  }else if(size_diff < 0) {
    logerr_pc("DOSEXEC: Resize shrink currently unsupported!!!\n");
  }
  list->size = size;
  prv_debug("post resize");
  return DOSERR_NONE;
}
