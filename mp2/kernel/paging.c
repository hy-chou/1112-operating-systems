// clang-format off
#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"
#include "proc.h"

// clang-format on

/* for handle_page_fault_va() */
void handle_page_fault_va_alloc(uint64 va) {
  char *pa = kalloc();
  int flags = PTE_R | PTE_W | PTE_X | PTE_U;

  memset(pa, 0, PGSIZE);
  mappages(myproc()->pagetable, PGROUNDDOWN(va), PGSIZE, (uint64)pa, flags);
}

/* for handle_page_fault_va() */
void handle_page_fault_va_swapin(pte_t *pte) {
  uint flags = PTE_FLAGS(*pte);
  char *pa = kalloc();

  begin_op();
  read_page_from_disk(ROOTDEV, pa, PTE2BLOCKNO(*pte));
  bfree_page(ROOTDEV, PTE2BLOCKNO(*pte));
  end_op();

  *pte = (PA2PTE(pa) | flags | PTE_V) & ~PTE_S;
}

/* for handle_page_fault() */
void handle_page_fault_va(uint64 va) {
  pte_t *pte = walk(myproc()->pagetable, va, 0);

  if (PTE_FLAGS(*pte) == 0) {
    handle_page_fault_va_alloc(va);
  } else if (*pte & PTE_S) {
    handle_page_fault_va_swapin(pte);
  }
}

/* NTU OS 2022 */
/* Page fault handler */
void handle_page_fault(void) { handle_page_fault_va(r_stval()); }

// clang-format off
