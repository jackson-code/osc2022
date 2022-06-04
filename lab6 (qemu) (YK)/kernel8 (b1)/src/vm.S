.global update_pgd
update_pgd:
    dsb ish
    msr ttbr0_el1, x0
    tlbi vmalle1is
    dsb ish
    isb
    ret
