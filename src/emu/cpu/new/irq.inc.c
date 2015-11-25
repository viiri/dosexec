/* TODO: BRK flag */
int cpu_has_irq(void)
{
  /* Dispatch IRQ */
  if(_cpu.irq && !_cpu.irq_disable) {
    /* We don't need to bother with NMI's because WS doesn't use them */
    if(_cpu.psw.ie)
      return 1;
    else
      return 2;
  }
  return 0;
}

void cpu_interrupt(uint16 vect)
{
  uint32 dest_seg, dest_off;

  _cpu.halted = FALSE;

  if(_cpu_irq_handlers[vect] != NULL) {
    if(_cpu_irq_handlers[vect](vect)) { // Handled!
      return;
    }
  }

  vect = vect << 2;
  dest_off = cpu_memr16(vect+0);
  dest_seg = cpu_memr16(vect+2);

  cpu_push_psw();
  _cpu.psw.ie = FALSE;
  _cpu.psw.brk = FALSE;
  _cpu.psw.md = TRUE;

  cpu_push(_cpu.reg.cs);
  _cpu.reg.cs = dest_seg;
  cpu_push(_cpu.reg.ip);
  _cpu.reg.ip = dest_off;
}

void cpu_handle_irq(void)
{
  _cpu.irq = 0;

  _cpu.exec.cycles += 32;
  cpu_interrupt(0/*soc_get_next_irq()*/);
}

void cpu_check_irq(void)
{
  /* We don't need to bother with NMI's because WS doesn't use them */
  /* Dispatch IRQ */
  switch(cpu_has_irq()) {
    case 1:
      cpu_handle_irq();
      /* fall-thru */
    case 2:
      _cpu.halted = FALSE;
      break;
  }

  /* No interrupt allowed between last instruction and this one */
  if(_cpu.irq_disable)
    _cpu.irq_disable--;
}
