#define INFLOOP_HALT 1

#define OP(CODE,NAME,CYC) \
case CODE:

#define OP_END() break

#define CPUHELPER static

CPUHELPER void cpue_fault(uint8 vec)
{
  _cpu.reg.ip = _cpu.fault_ip;
  cpu_interrupt(vec);
}

CPUHELPER void cpue_modrm_get_ea(void)
{
  int seg = -1;
  uint16 ea = 0;
  if(_cpu.exec.ea_seg != -1) // We already have EA
    return;
  if((_cpu.exec.modrm & 0xC0) == 0xC0) { // There is no EA for register ModR/Ms
    _cpu.exec.ea_seg = -1;
    return;
  }
  switch(_cpu.exec.modrm & 0x7) {
    case 0: ea += _cpu.reg.bw + _cpu.reg.ix; seg = SREG_DS; break;
    case 1: ea += _cpu.reg.bw + _cpu.reg.iy; seg = SREG_DS; break;
    case 2: ea += _cpu.reg.bp + _cpu.reg.ix; seg = SREG_SS; break;
    case 3: ea += _cpu.reg.bp + _cpu.reg.iy; seg = SREG_SS; break;
    case 4: ea +=               _cpu.reg.ix; seg = SREG_DS; break;
    case 5: ea +=               _cpu.reg.iy; seg = SREG_DS; break;
    case 7: ea +=               _cpu.reg.bw; seg = SREG_DS; break;
    case 6:
      if((_cpu.exec.modrm & 0xC0) == 0) {
        ea += cpu_fetch16();
        seg = SREG_DS;
      }else{
        ea += _cpu.reg.bp;
        seg = SREG_SS;
      }
      break;
  }
  switch(_cpu.exec.modrm & 0xC0) {
    case 0x00: break;
    case 0x40: ea += (int8)cpu_fetch8(); break;
    case 0x80: ea += (int16)cpu_fetch16(); break;
  }
  _cpu.exec.ea_seg = SegDefault(seg);
  _cpu.exec.ea = ea;
}

CPUHELPER uint8 cpue_modrm_get_regb(void)
{
  return _cpu.reg.r8[_cpu_modrm_regb[_cpu.exec.modrm]];
}
CPUHELPER uint16 cpue_modrm_get_regw(void)
{
  return _cpu.reg.r16[_cpu_modrm_regw[_cpu.exec.modrm]];
}
CPUHELPER void cpue_modrm_set_regb(uint8 val)
{
  _cpu.reg.r8[_cpu_modrm_regb[_cpu.exec.modrm]] = val;
}
CPUHELPER void cpue_modrm_set_regw(uint16 val)
{
  _cpu.reg.r16[_cpu_modrm_regw[_cpu.exec.modrm]] = val;
}

CPUHELPER uint8 cpue_modrm_get_rmb(uint16 offset)
{
  if(_cpu.exec.modrm >= 0xC0) {
    if(offset != 0) {
      logwarn_pc("GetRMB w/ offset on register ModRM\n");
    }
    return _cpu.reg.r8[_cpu_modrm_rmb[_cpu.exec.modrm]];
  }else{
    cpue_modrm_get_ea();
    return cpu_memr8(SegAddr(_cpu.exec.ea_seg, _cpu.exec.ea+offset));
  }
}
CPUHELPER uint16 cpue_modrm_get_rmw(uint16 offset)
{
  if(_cpu.exec.modrm >= 0xC0) {
    if(offset != 0) {
      logwarn_pc("GetRMW w/ offset on register ModRM\n");
    }
    return _cpu.reg.r16[_cpu_modrm_rmw[_cpu.exec.modrm]];
  }else{
    cpue_modrm_get_ea();
    return cpu_memr16(SegAddr(_cpu.exec.ea_seg, _cpu.exec.ea+offset));
  }
}
CPUHELPER void cpue_modrm_set_rmb(uint16 offset, uint8 val)
{
  if(_cpu.exec.modrm >= 0xC0) {
    if(offset != 0) {
      logwarn_pc("SetRMB w/ offset on register ModRM\n");
    }
    _cpu.reg.r8[_cpu_modrm_rmb[_cpu.exec.modrm]] = val;
  }else{
    cpue_modrm_get_ea();
    cpu_memw8(SegAddr(_cpu.exec.ea_seg, _cpu.exec.ea+offset), val);
  }
}
CPUHELPER void cpue_modrm_set_rmw(uint16 offset, uint16 val)
{
  if(_cpu.exec.modrm >= 0xC0) {
    if(offset != 0) {
      logwarn_pc("SetRMW w/ offset on register ModRM\n");
    }
    _cpu.reg.r16[_cpu_modrm_rmw[_cpu.exec.modrm]] = val;
  }else{
    cpue_modrm_get_ea();
    cpu_memw16(SegAddr(_cpu.exec.ea_seg, _cpu.exec.ea+offset), val);
  }
}
CPUHELPER void cpue_modrm_get(void)
{
  _cpu.exec.modrm = cpu_fetch8();
  cpue_modrm_get_ea();
}

CPUHELPER void cpue_modrm_r8_reg_mem(void)
{
  cpue_modrm_get();

  _cpu.exec.src0 = cpue_modrm_get_regb();
  _cpu.exec.src1 = cpue_modrm_get_rmb(0);
}
CPUHELPER void cpue_modrm_r16_reg_mem(void)
{
  cpue_modrm_get();

  _cpu.exec.src0 = cpue_modrm_get_regw();
  _cpu.exec.src1 = cpue_modrm_get_rmw(0);
}
CPUHELPER void cpue_modrm_r8_mem_reg(void)
{
  cpue_modrm_get();

  _cpu.exec.src0 = cpue_modrm_get_rmb(0);
  _cpu.exec.src1 = cpue_modrm_get_regb();
}
CPUHELPER void cpue_modrm_r16_mem_reg(void)
{
  cpue_modrm_get();

  _cpu.exec.src0 = cpue_modrm_get_rmw(0);
  _cpu.exec.src1 = cpue_modrm_get_regw();
}
CPUHELPER void cpue_al_imm8(void)
{
  _cpu.exec.src0 = _cpu.reg.al;
  _cpu.exec.src1 = cpu_fetch8();
}
CPUHELPER void cpue_aw_imm16(void)
{
  _cpu.exec.src0 = _cpu.reg.aw;
  _cpu.exec.src1 = cpu_fetch16();
}
CPUHELPER void cpue_imm8(void)
{
  _cpu.exec.src0 = cpu_fetch8();
}
CPUHELPER void cpue_imm8s(void)
{
  cpue_imm8();
  _cpu.exec.src0 = (uint16)(int8)_cpu.exec.src0;
}
CPUHELPER void cpue_imm16(void)
{
  _cpu.exec.src0 = cpu_fetch16();
}
CPUHELPER void cpue_imm16_imm8(void)
{
  _cpu.exec.src0 = cpu_fetch16();
  _cpu.exec.src1 = cpu_fetch8();
}
CPUHELPER void cpue_immfar(void)
{
  _cpu.exec.src0 = cpu_fetch16();
  _cpu.exec.src1 = cpu_fetch16();
}

CPUHELPER void cpue_flag_szp8(uint8 val)
{
  _cpu.psw.s = (val & 0x80) ? TRUE : FALSE;
  _cpu.psw.z = (val == 0) ? TRUE : FALSE;
  _cpu.psw.p = _cpu_parity[val];
}
CPUHELPER void cpue_flag_szp16(uint16 val)
{
  _cpu.psw.s = (val & 0x8000) ? TRUE : FALSE;
  _cpu.psw.z = (val == 0) ? TRUE : FALSE;
  _cpu.psw.p = _cpu_parity[val & 0xFF];
}
CPUHELPER void cpue_flag_auxcarry_add(uint8 a, uint8 b, int c)
{
  a &= 0xF;
  b &= 0xF;
  _cpu.psw.ac = ((a+b+c) >> 4) ? TRUE : FALSE;
}
CPUHELPER void cpue_flag_auxcarry_sub(uint8 a, uint8 b, int c)
{
  a &= 0xF;
  b &= 0xF;
  _cpu.psw.ac = ((a-b-c) >> 4) ? TRUE : FALSE;
}
CPUHELPER void cpue_flag_overflow_add8(uint8 res, uint8 a, uint8 b, int c)
{
  b += c; // todo: ?
  _cpu.psw.v = ((res ^ a) & (res ^ b) & 0x80) ? TRUE : FALSE;
}
CPUHELPER void cpue_flag_overflow_add16(uint32 res, uint a, uint32 b, int c)
{
  cpue_flag_overflow_add8(res>>8,a>>8,b>>8, c);
}
CPUHELPER void cpue_flag_overflow_sub8(uint8 res, uint8 a, uint8 b, int c)
{
  b += c; // todo: ?
  _cpu.psw.v = ((a ^ b) & (a ^ res) & 0x80) ? TRUE : FALSE;
}
CPUHELPER void cpue_flag_overflow_sub16(uint32 res, uint a, uint32 b, int c)
{
  cpue_flag_overflow_sub8(res>>8,a>>8,b>>8, c);
}
CPUHELPER BOOL cpue_divbound8u(void)
{
  uint32_t v = _cpu.exec.dst;
  if(v > 0xFF)
    return FALSE;
  return TRUE;
}
CPUHELPER BOOL cpue_divbound8s(void)
{
  int32_t v = _cpu.exec.dst;
  if((v < -0x7F) || (v >  0x7F))
    return FALSE;
  return TRUE;
}
CPUHELPER BOOL cpue_divbound16u(void)
{
  uint32_t v = _cpu.exec.dst;
  if(v > 0xFFFF)
    return FALSE;
  return TRUE;
}
CPUHELPER BOOL cpue_divbound16s(void)
{
  int32_t v = _cpu.exec.dst;
  if((v < -0x7FFF) || (v >  0x7FFF))
    return FALSE;
  return TRUE;
}
CPUHELPER void cpue_i_halt(void)
{
  _cpu.halted = TRUE;
}
CPUHELPER void cpue_i_add8(int is_adc)
{
  int src2 = 0;
  if(is_adc && _cpu.psw.cy)
    src2 = 1;
  uint32 res = _cpu.exec.src0 + _cpu.exec.src1 + src2;
  _cpu.psw.cy = (res >> 8) ? TRUE : FALSE;
  cpue_flag_szp8(res);
  cpue_flag_overflow_add8(res, _cpu.exec.src0, _cpu.exec.src1, src2);
  cpue_flag_auxcarry_add(_cpu.exec.src0, _cpu.exec.src1, src2);
  _cpu.exec.dst = res & 0xFF;
}
CPUHELPER void cpue_i_add16(int is_adc)
{
  int src2 = 0;
  if(is_adc && _cpu.psw.cy)
    src2 = 1;
  uint32 res = _cpu.exec.src0 + _cpu.exec.src1 + src2;
  _cpu.psw.cy = (res >> 16) ? TRUE : FALSE;
  cpue_flag_szp16(res);
  cpue_flag_overflow_add16(res, _cpu.exec.src0, _cpu.exec.src1, src2);
  cpue_flag_auxcarry_add(_cpu.exec.src0, _cpu.exec.src1, src2);
  _cpu.exec.dst = res & 0xFFFF;
}
CPUHELPER void cpue_i_sub8(int is_sbb)
{
  int src2 = 0;
  if(is_sbb && _cpu.psw.cy)
    src2 = 1;
  uint32 res = _cpu.exec.src0 - _cpu.exec.src1 - src2;
  _cpu.psw.cy = (res >> 8) ? TRUE : FALSE;
  cpue_flag_szp8(res);
  cpue_flag_overflow_sub8(res, _cpu.exec.src0, _cpu.exec.src1, src2);
  cpue_flag_auxcarry_sub(_cpu.exec.src0, _cpu.exec.src1, src2);
  _cpu.exec.dst = res & 0xFF;
}
CPUHELPER void cpue_i_sub16(int is_sbb)
{
  int src2 = 0;
  if(is_sbb && _cpu.psw.cy)
    src2 = 1;
  uint32 res = _cpu.exec.src0 - _cpu.exec.src1 - src2;
  _cpu.psw.cy = (res >> 16) ? TRUE : FALSE;
  cpue_flag_szp16(res);
  cpue_flag_overflow_sub16(res, _cpu.exec.src0, _cpu.exec.src1, src2);
  cpue_flag_auxcarry_sub(_cpu.exec.src0, _cpu.exec.src1, src2);
  _cpu.exec.dst = res & 0xFFFF;
}
CPUHELPER void cpue_i_or8(void)
{
  uint32 res = _cpu.exec.src0 | _cpu.exec.src1;
  _cpu.psw.cy = _cpu.psw.v = _cpu.psw.ac = FALSE;
  cpue_flag_szp8(res);
  _cpu.exec.dst = res & 0xFF;
}
CPUHELPER void cpue_i_or16(void)
{
  uint32 res = _cpu.exec.src0 | _cpu.exec.src1;
  _cpu.psw.cy = _cpu.psw.v = _cpu.psw.ac = FALSE;
  cpue_flag_szp16(res);
  _cpu.exec.dst = res & 0xFFFF;
}
CPUHELPER void cpue_i_and8(void)
{
  uint32 res = _cpu.exec.src0 & _cpu.exec.src1;
  _cpu.psw.cy = _cpu.psw.v = _cpu.psw.ac = FALSE;
  cpue_flag_szp8(res);
  _cpu.exec.dst = res & 0xFF;
}
CPUHELPER void cpue_i_and16(void)
{
  uint32 res = _cpu.exec.src0 & _cpu.exec.src1;
  _cpu.psw.cy = _cpu.psw.v = _cpu.psw.ac = FALSE;
  cpue_flag_szp16(res);
  _cpu.exec.dst = res & 0xFFFF;
}
CPUHELPER void cpue_i_xor8(void)
{
  uint32 res = _cpu.exec.src0 ^ _cpu.exec.src1;
  _cpu.psw.cy = _cpu.psw.v = _cpu.psw.ac = FALSE;
  cpue_flag_szp8(res);
  _cpu.exec.dst = res & 0xFF;
}
CPUHELPER void cpue_i_xor16(void)
{
  uint32 res = _cpu.exec.src0 ^ _cpu.exec.src1;
  _cpu.psw.cy = _cpu.psw.v = _cpu.psw.ac = FALSE;
  cpue_flag_szp16(res);
  _cpu.exec.dst = res & 0xFFFF;
}
CPUHELPER void cpue_i_daa_das(BOOL neg)
{
  uint8 old = _cpu.reg.al;
  int bcd_lo, bcd_hi;
  bcd_lo = 0x06;
  bcd_hi = 0x60;
  if(neg) {
    bcd_lo = -bcd_lo;
    bcd_hi = -bcd_hi;
  }
  if(_cpu.psw.ac || ((old & 0x0F) > 0x09)) {
    _cpu.reg.al += bcd_lo;
    _cpu.psw.ac = TRUE;
  }
  if(_cpu.psw.cy || (old > 0x99)) {
    _cpu.reg.al += bcd_hi;
    _cpu.psw.cy = TRUE;
  }
  /* TODO: Unknown results:
   * This is pretty unverified...
   */
  /* TODO: Undefined flags:
   * V
   */
  cpue_flag_szp8(_cpu.reg.al);
}
CPUHELPER void cpue_i_aaa_aas(BOOL neg)
{
  int lo, hi;
  lo = 0x06;
  hi = 0x01;
  if(neg) {
    lo = -lo;
    hi = -hi;
  }
  if(_cpu.psw.ac || ((_cpu.reg.al & 0x0F) > 0x09)) {
    _cpu.reg.al += lo;
    _cpu.reg.ah += hi;
    _cpu.psw.ac = TRUE;
    _cpu.psw.cy = TRUE;
  }else{
    _cpu.psw.ac = FALSE;
    _cpu.psw.cy = FALSE;
  }
  /* TODO: Undefined flags:
   * V, P, S, Z
   */
  _cpu.reg.al &= 0x0F;
}
CPUHELPER void cpue_i_push(void)
{
  cpu_push(_cpu.exec.src0);
}
CPUHELPER void cpue_i_pop(void)
{
  _cpu.exec.dst = cpu_pop();
}
CPUHELPER void cpue_i_pushf(void)
{
  _cpu.exec.src0 = cpu_get_psw();
  cpue_i_push();
}
CPUHELPER void cpue_i_popf(void)
{
  uint16 psw;
  psw = cpu_pop();
  cpu_set_psw(psw);
}
CPUHELPER void cpue_i_inc8(void)
{
  uint32 res = _cpu.exec.src0 + 1;
  _cpu.psw.v = (res == 0x80) ? TRUE : FALSE;
  cpue_flag_szp8(res);
  _cpu.psw.ac = (_cpu.exec.src0 & 0xF) == 0xF ? TRUE : FALSE;
  _cpu.exec.dst = res;
}
CPUHELPER void cpue_i_dec8(void)
{
  uint32 res = _cpu.exec.src0 - 1;
  _cpu.psw.v = (res == 0x7F) ? TRUE : FALSE;
  cpue_flag_szp8(res);
  _cpu.psw.ac = (_cpu.exec.src0 & 0xF) == 0x0 ? TRUE : FALSE;
  _cpu.exec.dst = res;
}
CPUHELPER void cpue_i_inc16(void)
{
  uint32 res = _cpu.exec.src0 + 1;
  _cpu.psw.v = (res == 0x8000) ? TRUE : FALSE;
  cpue_flag_szp16(res);
  _cpu.psw.ac = (_cpu.exec.src0 & 0xF) == 0xF ? TRUE : FALSE;
  _cpu.exec.dst = res;
}
CPUHELPER void cpue_i_dec16(void)
{
  uint32 res = _cpu.exec.src0 - 1;
  _cpu.psw.v = (res == 0x7FFF) ? TRUE : FALSE;
  cpue_flag_szp16(res);
  _cpu.psw.ac = (_cpu.exec.src0 & 0xF) == 0x0 ? TRUE : FALSE;
  _cpu.exec.dst = res;
}
CPUHELPER void cpue_i_pusha(void)
{
  uint16 sp = _cpu.reg.sp;
  cpu_push(_cpu.reg.aw);
  cpu_push(_cpu.reg.cw);
  cpu_push(_cpu.reg.dw);
  cpu_push(_cpu.reg.bw);
  cpu_push(sp);
  cpu_push(_cpu.reg.bp);
  cpu_push(_cpu.reg.ix);
  cpu_push(_cpu.reg.iy);
}
CPUHELPER void cpue_i_popa(void)
{
  uint16 sp;
  _cpu.reg.iy = cpu_pop();
  _cpu.reg.ix = cpu_pop();
  _cpu.reg.bp = cpu_pop();
  sp = cpu_pop();
  _cpu.reg.bw = cpu_pop();
  _cpu.reg.dw = cpu_pop();
  _cpu.reg.cw = cpu_pop();
  _cpu.reg.aw = cpu_pop();
  _cpu.reg.sp = sp;
}
CPUHELPER void cpue_i_jmp(BOOL cond)
{
  if(!cond) return;
  _cpu.reg.ip += _cpu.exec.src0;
}
CPUHELPER void cpue_i_jmpf(void)
{
  _cpu.reg.ip = _cpu.exec.src0;
  _cpu.reg.cs = _cpu.exec.src1;
}
CPUHELPER void cpue_i_jmpn(void)
{
  _cpu.reg.ip += (int16)_cpu.exec.src0;
}
CPUHELPER void cpue_i_jmps(void)
{
#if INFLOOP_HALT
  if((int8)_cpu.exec.src0 == -2)
    _cpu.halted = TRUE;
#endif
  _cpu.reg.ip += _cpu.exec.src0;
}
CPUHELPER void cpue_i_loop(BOOL cond, BOOL is_cond)
{
  (void)is_cond;
  _cpu.reg.cw--;
  if(cond && _cpu.reg.cw != 0) {
    _cpu.reg.ip += _cpu.exec.src0;
  }
}
CPUHELPER void cpue_i_jcxz(void)
{
  if(_cpu.reg.cw == 0) {
    _cpu.reg.ip += _cpu.exec.src0;
  }
}
CPUHELPER void cpue_i_read8(int seg)
{
  uint32 addr;
  addr = SegAddr(SegDefault(seg), _cpu.exec.src0);
  _cpu.exec.dst = cpu_memr8(addr);
}
CPUHELPER void cpue_i_read16(int seg)
{
  uint32 addr;
  addr = SegAddr(SegDefault(seg), _cpu.exec.src0);
  _cpu.exec.dst = cpu_memr16(addr);
}
CPUHELPER void cpue_i_write8(int seg)
{
  uint32 addr;
  addr = SegAddr(SegDefault(seg), _cpu.exec.src0);
  cpu_memw8(addr, _cpu.exec.dst);
}
CPUHELPER void cpue_i_write16(int seg)
{
  uint32 addr;
  addr = SegAddr(SegDefault(seg), _cpu.exec.src0);
  cpu_memw16(addr, _cpu.exec.dst);
}
CPUHELPER void cpue_i_in8(void)
{
  _cpu.exec.dst = cpu_memr8io(_cpu.exec.src0);
}
CPUHELPER void cpue_i_in16(void)
{
  _cpu.exec.dst = cpu_memr16io(_cpu.exec.src0);
}
CPUHELPER void cpue_i_out8(void)
{
  cpu_memw8io(_cpu.exec.src0, _cpu.exec.src1);
}
CPUHELPER void cpue_i_out16(void)
{
  cpu_memw16io(_cpu.exec.src0, _cpu.exec.src1);
}
CPUHELPER void cpue_i_callf(void)
{
  cpu_push(_cpu.reg.cs);
  cpu_push(_cpu.reg.ip);
  _cpu.reg.ip = _cpu.exec.src0;
  _cpu.reg.cs = _cpu.exec.src1;
}
CPUHELPER void cpue_i_calln(void)
{
  cpu_push(_cpu.reg.ip);
  _cpu.reg.ip += (int16)_cpu.exec.src0;
}
CPUHELPER void cpue_i_call(void)
{
  cpu_push(_cpu.reg.ip);
  _cpu.reg.ip = _cpu.exec.src0;
}
CPUHELPER void cpue_i_sahf(void)
{
  uint16 psw;
  psw  = cpu_get_psw() & 0xFF00;
  psw |= _cpu.reg.ah;
  cpu_set_psw(psw);
}
CPUHELPER void cpue_i_lahf(void)
{
  _cpu.reg.ah = cpu_get_psw() & 0xFF;
}
CPUHELPER void cpue_block_inc(int count, int flag)
{
  if(_cpu.psw.dir) {
    count = -count;
  }
  if(flag & 1)
    _cpu.reg.ix += count;
  if(flag & 2)
    _cpu.reg.iy += count;
}
CPUHELPER void cpue_i_insb(void)
{
  uint8 data = cpu_memr8io(_cpu.reg.dw);
  cpu_memw8(SegAddr(SREG_ES, _cpu.reg.iy), data);
  cpue_block_inc(1, 2);
}
CPUHELPER void cpue_i_insw(void)
{
  uint16 data = cpu_memr16io(_cpu.reg.dw);
  cpu_memw16(SegAddr(SREG_ES, _cpu.reg.iy), data);
  cpue_block_inc(2, 2);
}
CPUHELPER void cpue_i_outsb(void)
{
  uint8 data = cpu_memr8(SegAddr(SegDefault(SREG_DS), _cpu.reg.ix));
  cpu_memw8io(_cpu.reg.dw, data);
  cpue_block_inc(1, 1);
}
CPUHELPER void cpue_i_outsw(void)
{
  uint16 data = cpu_memr16(SegAddr(SegDefault(SREG_DS), _cpu.reg.ix));
  cpu_memw16io(_cpu.reg.dw, data);
  cpue_block_inc(2, 1);
}
CPUHELPER void cpue_i_movsb(void)
{
  uint8 data = cpu_memr8(SegAddr(SegDefault(SREG_DS), _cpu.reg.ix));
  cpu_memw8(SegAddr(SREG_ES, _cpu.reg.iy), data);
  cpue_block_inc(1, 3);
}
CPUHELPER void cpue_i_movsw(void)
{
  uint16 data = cpu_memr16(SegAddr(SegDefault(SREG_DS), _cpu.reg.ix));
  cpu_memw16(SegAddr(SREG_ES, _cpu.reg.iy), data);
  cpue_block_inc(2, 3);
}
CPUHELPER void cpue_i_cmpsb(void)
{
  _cpu.exec.src0 = cpu_memr8(SegAddr(SegDefault(SREG_DS), _cpu.reg.ix));;
  _cpu.exec.src1 = cpu_memr8(SegAddr(SREG_ES, _cpu.reg.iy));
  cpue_i_sub8(0);
  cpue_block_inc(1, 3);

  _cpu.rep_zflag = _cpu.psw.z;
}
CPUHELPER void cpue_i_cmpsw(void)
{
  _cpu.exec.src0 = cpu_memr16(SegAddr(SegDefault(SREG_DS), _cpu.reg.ix));
  _cpu.exec.src1 = cpu_memr16(SegAddr(SREG_ES, _cpu.reg.iy));
  cpue_i_sub16(0);
  cpue_block_inc(2, 3);

  _cpu.rep_zflag = _cpu.psw.z;
}
CPUHELPER void cpue_i_stosb(void)
{
  cpu_memw8(SegAddr(SREG_ES, _cpu.reg.iy), _cpu.reg.al);
  cpue_block_inc(1, 2);
}
CPUHELPER void cpue_i_stosw(void)
{
  cpu_memw16(SegAddr(SREG_ES, _cpu.reg.iy), _cpu.reg.aw);
  cpue_block_inc(2, 2);
}
CPUHELPER void cpue_i_lodsb(void)
{
  _cpu.reg.al = cpu_memr8(SegAddr(SegDefault(SREG_DS), _cpu.reg.ix));
  cpue_block_inc(1, 1);
}
CPUHELPER void cpue_i_lodsw(void)
{
  _cpu.reg.aw = cpu_memr16(SegAddr(SegDefault(SREG_DS), _cpu.reg.ix));
  cpue_block_inc(2, 1);
}
CPUHELPER void cpue_i_scasb(void)
{
  _cpu.exec.src0 = _cpu.reg.al;
  _cpu.exec.src1 = cpu_memr8(SegAddr(SREG_ES, _cpu.reg.iy));
  cpue_i_sub8(0);
  cpue_block_inc(1, 2);

  _cpu.rep_zflag = _cpu.psw.z;
}
CPUHELPER void cpue_i_scasw(void)
{
  _cpu.exec.src0 = _cpu.reg.aw;
  _cpu.exec.src1 = cpu_memr16(SegAddr(SREG_ES, _cpu.reg.iy));
  cpue_i_sub16(0);
  cpue_block_inc(2, 2);

  _cpu.rep_zflag = _cpu.psw.z;
}
CPUHELPER void cpue_i_ret(BOOL do_pop)
{
  _cpu.reg.ip = cpu_pop();
  if(do_pop) {
    _cpu.reg.sp += _cpu.exec.src0;
  }
}
CPUHELPER void cpue_i_enter(void)
{
  uint i;
  uint16 data;
  _cpu.exec.src1 &= 0x1F; /* Only bottom 5 bits matter */
  cpu_push(_cpu.reg.bp);
  _cpu.reg.bp = _cpu.reg.sp;
  _cpu.reg.sp -= _cpu.exec.src0;

  for(i = 1; i < _cpu.exec.src1; i++) {
    data = cpu_memr16(SegAddr(SegDefault(SREG_SS), _cpu.reg.bp - i*2));
    cpu_push(data);
  }
  if(_cpu.exec.src1) {
    cpu_push(_cpu.reg.bp);
  }
}
CPUHELPER void cpue_i_leave(void)
{
  _cpu.reg.sp = _cpu.reg.bp;
  _cpu.reg.bp = cpu_pop();
}
CPUHELPER void cpue_i_retf(BOOL do_pop)
{
  _cpu.reg.ip = cpu_pop();
  _cpu.reg.cs = cpu_pop();
  if(do_pop) {
    _cpu.reg.sp += _cpu.exec.src0;
  }
}
CPUHELPER void cpue_i_int(void)
{
  cpu_interrupt(_cpu.exec.src0);
}
CPUHELPER void cpue_i_into(void)
{
  if(_cpu.psw.v) {
    _cpu.exec.src0 = INTVEC_BRKV;
    cpue_i_int();
  }
}
CPUHELPER void cpue_i_aam(void)
{
  if(_cpu.exec.src0 == 0) {
    cpue_fault(INTVEC_DIVERR);
    return;
  }
  _cpu.reg.ah = _cpu.reg.al / _cpu.exec.src0;
  _cpu.reg.al %= _cpu.exec.src0;
  cpue_flag_szp16(_cpu.reg.aw);
}
CPUHELPER void cpue_i_aad(void)
{
  _cpu.reg.al += _cpu.reg.ah * _cpu.exec.src0;
  _cpu.reg.ah = 0;
  cpue_flag_szp8(_cpu.reg.al);
}
CPUHELPER void cpue_i_xlat(void)
{
  uint16 dst;
  dst = _cpu.reg.bw + _cpu.reg.al;
  _cpu.exec.dst = cpu_memr8(SegAddr(SegDefault(SREG_DS), dst));
}
CPUHELPER void cpue_i_ldes(void)
{
  cpue_modrm_set_regw(cpue_modrm_get_rmw(0));
  _cpu.exec.dst = cpue_modrm_get_rmw(2);
}
CPUHELPER void cpue_i_segpfx(int seg)
{
  _cpu.pfx_reset = FALSE;
  _cpu.was_prefix = TRUE;
  _cpu.segpfx = seg;
}
CPUHELPER uint8 cpue_rep_pfxs(void)
{
  uint8 next;
  int i;

  /* V30MZ manual says only 7 prefixes allowed... */
  for(i = 0; i < 6; i++) {
    next = cpu_fetch8();
    switch(next) {
      case 0x26:/*ES:*/ cpue_i_segpfx(SREG_ES); break;
      case 0x2E:/*CS:*/ cpue_i_segpfx(SREG_CS); break;
      case 0x36:/*SS:*/ cpue_i_segpfx(SREG_SS); break;
      case 0x3E:/*DS:*/ cpue_i_segpfx(SREG_DS); break;
      default: return next;
    }
  }
  return 0; /* not valid, so it'll kick out of rep */
}
/* TODO: This logic should be built into the individual instructions! */
CPUHELPER void cpue_i_rep(BOOL is_z)
{
  uint8 next;
  int cmpc = 9;

  next = cpue_rep_pfxs();
  _cpu.pfx_reset = TRUE;
  _cpu.irq_disable = FALSE;
  switch(next) {
    case 0x6C: case 0x6D: case 0x6E: case 0x6F:
    case 0xA4: case 0xA5: case 0xA6: case 0xA7:
                          case 0xAA: case 0xAB:
    case 0xAC: case 0xAD: case 0xAE: case 0xAF:
      break;
    default:
      logwarn_pc("Invalid REP%s insn %02X\n", is_z?"Z":"NZ", next);
      _cpu.reg.ip--;
      _cpu.pfx_reset = FALSE;
      _cpu.was_prefix = TRUE;
      return;
  }
  if(_cpu.reg.cw == 0) {
    return;
  }
  if(is_z)
    cmpc += 1;
  _cpu.rep_zflag = is_z;
  while(_cpu.reg.cw != 0 && _cpu.rep_zflag == is_z) {
    switch(next) {
      case 0x6C: cpue_i_insb(); break;
      case 0x6D: cpue_i_insw(); break;
      case 0x6E: cpue_i_outsb(); break;
      case 0x6F: cpue_i_outsw(); break;
      case 0xA4: cpue_i_movsb(); break;
      case 0xA5: cpue_i_movsw(); break;
      case 0xA6: cpue_i_cmpsb(); break;
      case 0xA7: cpue_i_cmpsw(); break;
      case 0xAA: cpue_i_stosb(); break;
      case 0xAB: cpue_i_stosw(); break;
      case 0xAC: cpue_i_lodsb(); break;
      case 0xAD: cpue_i_lodsw(); break;
      case 0xAE: cpue_i_scasb(); break;
      case 0xAF: cpue_i_scasw(); break;
    }
    _cpu.reg.cw--;
  }
}
CPUHELPER void cpue_modrmEb_imm8(void)
{
  cpue_modrm_get();
  _cpu.exec.src0 = cpue_modrm_get_rmb(0);

  _cpu.exec.src1 = cpu_fetch8();
}
CPUHELPER void cpue_modrmEb_imm8s(void)
{
  cpue_modrmEb_imm8();
  _cpu.exec.src1 = (uint16)(int8)_cpu.exec.src1;
}
CPUHELPER void cpue_modrmEw_imm16(void)
{
  cpue_modrm_get();
  _cpu.exec.src0 = cpue_modrm_get_rmw(0);

  _cpu.exec.src1 = cpu_fetch16();
}
CPUHELPER void cpue_modrmEb(void)
{
  cpue_modrm_get();
  _cpu.exec.src0 = cpue_modrm_get_rmb(0);
}
CPUHELPER void cpue_modrmEw(void)
{
  cpue_modrm_get();
  _cpu.exec.src0 = cpue_modrm_get_rmw(0);
}
CPUHELPER void cpue_modrmEw_imm8(void)
{
  cpue_modrm_get();
  _cpu.exec.src0 = cpue_modrm_get_rmw(0);

  _cpu.exec.src1 = cpu_fetch8();
}
CPUHELPER void cpue_modrmEw_imm8s(void)
{
  cpue_modrm_get();
  _cpu.exec.src0 = cpue_modrm_get_rmw(0);

  _cpu.exec.src1 = (uint16)(int8)cpu_fetch8();
}
CPUHELPER void cpue_i_grp1_8(void)
{
  switch((_cpu.exec.modrm >> 3) & 7) {
    case 0: /* add */
      cpue_i_add8(0);
      break;
    case 1: /* or */
      cpue_i_or8();
      break;
    case 2: /* adc */
      cpue_i_add8(1);
      break;
    case 3: /* sbb */
      cpue_i_sub8(1);
      break;
    case 4: /* and */
      cpue_i_and8();
      break;
    case 5: /* sub */
      cpue_i_sub8(0);
      break;
    case 6: /* xor */
      cpue_i_xor8();
      break;
    case 7: /* cmp */
      cpue_i_sub8(0);
      break;
  }
  if(((_cpu.exec.modrm >> 3) & 7) != 7) {
    cpue_modrm_set_rmb(0, _cpu.exec.dst);
  }
}
CPUHELPER void cpue_i_grp1_16(void)
{
  switch((_cpu.exec.modrm >> 3) & 7) {
    case 0: /* add */
      cpue_i_add16(0);
      break;
    case 1: /* or */
      cpue_i_or16();
      break;
    case 2: /* adc */
      cpue_i_add16(1);
      break;
    case 3: /* sbb */
      cpue_i_sub16(1);
      break;
    case 4: /* and */
      cpue_i_and16();
      break;
    case 5: /* sub */
      cpue_i_sub16(0);
      break;
    case 6: /* xor */
      cpue_i_xor16();
      break;
    case 7: /* cmp */
      cpue_i_sub16(0);
      break;
  }
  if(((_cpu.exec.modrm >> 3) & 7) != 7) {
    cpue_modrm_set_rmw(0, _cpu.exec.dst);
  }
}
CPUHELPER void cpue_i_grp2_8(void)
{
  if(_cpu.exec.src1 == 0) {
    /* TODO */
    logwarn_pc("Shift8 with 0 count\n");
  }
  uint i;
  BOOL newcy;
  _cpu.exec.src1 &= 31;
  switch((_cpu.exec.modrm >> 3) & 7) {
    case 0: /* rol */
      _cpu.psw.cy = (_cpu.exec.src0 << _cpu.exec.src1) & 0x100 ? TRUE : FALSE;
      _cpu.exec.src1 &= 7;
      _cpu.exec.dst = (_cpu.exec.src0 << _cpu.exec.src1) | (_cpu.exec.src0 >> (8-_cpu.exec.src1));
      /* TODO: Verify this for shifts of non-1 */
      _cpu.psw.v = (_cpu.exec.src0 & 0x80) != (_cpu.exec.dst & 0x80);
      break;
    case 1: /* ror */
      _cpu.psw.cy = (_cpu.exec.src0 >> (_cpu.exec.src1-1)) & 0x1 ? TRUE : FALSE;
      _cpu.exec.src1 &= 7;
      _cpu.exec.dst = (_cpu.exec.src0 >> _cpu.exec.src1) | (_cpu.exec.src0 << (8-_cpu.exec.src1));
      /* TODO: Verify this for shifts of non-1 */
      _cpu.psw.v = (_cpu.exec.src0 & 0x80) != (_cpu.exec.dst & 0x80);
      break;
    case 2: /* rcl */
      _cpu.exec.dst = _cpu.exec.src0;
      for(i = 0; i < _cpu.exec.src1; i++) {
        newcy = (_cpu.exec.dst & 0x80) ? TRUE : FALSE;
        _cpu.exec.dst = (_cpu.exec.dst << 1) | (_cpu.psw.cy ? 0x01 : 0x00);
        _cpu.exec.dst &= 0xFF;
        _cpu.psw.cy = newcy;
      }
      /* TODO: Verify this for shifts of non-1 */
      _cpu.psw.v = (_cpu.exec.src0 & 0x80) != (_cpu.exec.dst & 0x80);
      break;
    case 3: /* rcr */
      _cpu.exec.dst = _cpu.exec.src0;
      for(i = 0; i < _cpu.exec.src1; i++) {
        newcy = (_cpu.exec.dst & 0x01) ? TRUE : FALSE;
        _cpu.exec.dst = (_cpu.exec.dst >> 1) | (_cpu.psw.cy ? 0x80 : 0x00);
        _cpu.exec.dst &= 0xFF;
        _cpu.psw.cy = newcy;
      }
      /* TODO: Verify this for shifts of non-1 */
      _cpu.psw.v = (_cpu.exec.src0 & 0x80) != (_cpu.exec.dst & 0x80);
      break;
    case 4: /* shl */
      _cpu.psw.cy = (_cpu.exec.src0 << _cpu.exec.src1) & 0x100 ? TRUE : FALSE;
      _cpu.exec.dst = _cpu.exec.src0 << _cpu.exec.src1;
      cpue_flag_szp8(_cpu.exec.dst);
      /* TODO: Verify this for shifts of non-1 */
      _cpu.psw.v = (_cpu.exec.src0 & 0x80) != (_cpu.exec.dst & 0x80);
      break;
    case 5: /* shr */
      _cpu.psw.cy = (_cpu.exec.src0 >> (_cpu.exec.src1-1)) & 0x1 ? TRUE : FALSE;
      _cpu.exec.dst = _cpu.exec.src0 >> _cpu.exec.src1;
      cpue_flag_szp8(_cpu.exec.dst);
      /* TODO: Verify this for shifts of non-1 */
      _cpu.psw.v = (_cpu.exec.src0 & 0x80) != (_cpu.exec.dst & 0x80);
      break;
    case 6: /* sal */
      logwarn_pc("grp2_8 SAL\n");
      _cpu.psw.cy = (_cpu.exec.src0 << _cpu.exec.src1) & 0x100 ? TRUE : FALSE;
      _cpu.exec.dst = _cpu.exec.src0 << _cpu.exec.src1;
      cpue_flag_szp8(_cpu.exec.dst);
      /* TODO: Verify this for shifts of non-1 */
      _cpu.psw.v = FALSE;
      break;
    case 7: /* sar */
      if(_cpu.exec.src1 >= 8) {
        _cpu.psw.cy = _cpu.exec.src0 & 0x80 ? TRUE : FALSE;
        _cpu.exec.dst = (_cpu.exec.src0 & 0x80) ? 0xFF : 0;
      }else{
        _cpu.psw.cy = (_cpu.exec.src0 >> (_cpu.exec.src1-1)) & 0x1 ? TRUE : FALSE;
        _cpu.exec.dst = _cpu.exec.src0 >> _cpu.exec.src1;
        if(_cpu.exec.src0 & 0x80)
          _cpu.exec.dst |= 0xFF << (8-_cpu.exec.src1);
        cpue_flag_szp8(_cpu.exec.dst);
      }
      /* TODO: Verify this for shifts of non-1 */
      _cpu.psw.v = FALSE;
      break;
  }
  _cpu.exec.dst &= 0xFF;
}
CPUHELPER void cpue_i_grp2_16(void)
{
  if(_cpu.exec.src1 == 0) {
    /* TODO */
    logwarn_pc("Shift16 with 0 count\n");
  }
  uint i;
  BOOL newcy;
  _cpu.exec.src1 &= 31;
  switch((_cpu.exec.modrm >> 3) & 7) {
    case 0: /* rol */
      _cpu.psw.cy = (_cpu.exec.src0 << _cpu.exec.src1) & 0x10000 ? TRUE : FALSE;
      _cpu.exec.src1 &= 15;
      _cpu.exec.dst = (_cpu.exec.src0 << _cpu.exec.src1) | (_cpu.exec.src0 >> (16-_cpu.exec.src1));
      /* TODO: Verify this for shifts of non-1 */
      _cpu.psw.v = (_cpu.exec.src0 & 0x8000) != (_cpu.exec.dst & 0x8000);
      break;
    case 1: /* ror */
      _cpu.psw.cy = (_cpu.exec.src0 >> (_cpu.exec.src1-1)) & 0x1 ? TRUE : FALSE;
      _cpu.exec.src1 &= 15;
      _cpu.exec.dst = (_cpu.exec.src0 >> _cpu.exec.src1) | (_cpu.exec.src0 << (16-_cpu.exec.src1));
      /* TODO: Verify this for shifts of non-1 */
      _cpu.psw.v = (_cpu.exec.src0 & 0x8000) != (_cpu.exec.dst & 0x8000);
      break;
    case 2: /* rcl */
      _cpu.exec.dst = _cpu.exec.src0;
      for(i = 0; i < _cpu.exec.src1; i++) {
        newcy = (_cpu.exec.dst & 0x8000) ? TRUE : FALSE;
        _cpu.exec.dst = (_cpu.exec.dst << 1) | (_cpu.psw.cy ? 0x01 : 0x00);
        _cpu.exec.dst &= 0xFFFF;
        _cpu.psw.cy = newcy;
      }
      /* TODO: Verify this for shifts of non-1 */
      _cpu.psw.v = (_cpu.exec.src0 & 0x8000) != (_cpu.exec.dst & 0x8000);
      break;
    case 3: /* rcr */
      _cpu.exec.dst = _cpu.exec.src0;
      for(i = 0; i < _cpu.exec.src1; i++) {
        newcy = (_cpu.exec.dst & 0x0001) ? TRUE : FALSE;
        _cpu.exec.dst = (_cpu.exec.dst >> 1) | (_cpu.psw.cy ? 0x8000 : 0x00);
        _cpu.exec.dst &= 0xFFFF;
        _cpu.psw.cy = newcy;
      }
      /* TODO: Verify this for shifts of non-1 */
      _cpu.psw.v = (_cpu.exec.src0 & 0x8000) != (_cpu.exec.dst & 0x8000);
      break;
    case 4: /* shl */
      _cpu.psw.cy = (_cpu.exec.src0 << _cpu.exec.src1) & 0x10000 ? TRUE : FALSE;
      _cpu.exec.dst = _cpu.exec.src0 << _cpu.exec.src1;
      cpue_flag_szp16(_cpu.exec.dst);
      /* TODO: Verify this for shifts of non-1 */
      _cpu.psw.v = (_cpu.exec.src0 & 0x8000) != (_cpu.exec.dst & 0x8000);
      break;
    case 5: /* shr */
      _cpu.psw.cy = (_cpu.exec.src0 >> (_cpu.exec.src1-1)) & 0x1 ? TRUE : FALSE;
      _cpu.exec.dst = _cpu.exec.src0 >> _cpu.exec.src1;
      cpue_flag_szp16(_cpu.exec.dst);
      /* TODO: Verify this for shifts of non-1 */
      _cpu.psw.v = (_cpu.exec.src0 & 0x8000) != (_cpu.exec.dst & 0x8000);
      break;
    case 6: /* sal */
      logwarn_pc("grp2_16 SAL\n");
      _cpu.psw.cy = (_cpu.exec.src0 << _cpu.exec.src1) & 0x10000 ? TRUE : FALSE;
      _cpu.exec.dst = _cpu.exec.src0 << _cpu.exec.src1;
      cpue_flag_szp16(_cpu.exec.dst);
      /* TODO: Verify this for shifts of non-1 */
      _cpu.psw.v = FALSE;
      break;
    case 7: /* sar */
      if(_cpu.exec.src1 >= 16) {
        _cpu.psw.cy = _cpu.exec.src0 & 0x8000 ? TRUE : FALSE;
        _cpu.exec.dst = (_cpu.exec.src0 & 0x8000) ? 0xFFFF : 0;
      }else{
        _cpu.psw.cy = (_cpu.exec.src0 >> (_cpu.exec.src1-1)) & 0x1 ? TRUE : FALSE;
        _cpu.exec.dst = _cpu.exec.src0 >> _cpu.exec.src1;
        if(_cpu.exec.src0 & 0x8000)
          _cpu.exec.dst |= 0xFFFF << (16-_cpu.exec.src1);
        cpue_flag_szp16(_cpu.exec.dst);
      }
      /* TODO: Verify this for shifts of non-1 */
      _cpu.psw.v = FALSE;
      break;
  }
  _cpu.exec.dst &= 0xFFFF;
}
CPUHELPER void cpue_i_neg(BOOL is_16)
{
  _cpu.exec.dst = -_cpu.exec.src0;
  _cpu.psw.ac = _cpu.exec.src0 & 0xF ? TRUE : FALSE; /* TODO: not sure */
  _cpu.psw.cy = _cpu.exec.src0 ? TRUE : FALSE;
  if(is_16) {
    _cpu.psw.v = _cpu.exec.src0 == 0x8000 ? TRUE : FALSE; /* TODO: not sure */
    cpue_flag_szp16(_cpu.exec.dst);
  }else{
    _cpu.psw.v = _cpu.exec.src0 == 0x80 ? TRUE : FALSE; /* TODO: not sure */
    cpue_flag_szp8(_cpu.exec.dst);
  }
}
CPUHELPER void cpue_i_grp3_8(void)
{
  switch((_cpu.exec.modrm >> 3) & 7) {
    case 0: /* TEST */
      _cpu.exec.src1 = cpu_fetch8();
      cpue_i_and8();
      break;
    case 1: /* unknown */
      logerr_pc("Unknown opcode grp3_8 op1\n");
      break;
    case 2: /* NOT */
      _cpu.exec.dst = ~_cpu.exec.src0;
      break;
    case 3: /* NEG */
      cpue_i_neg(FALSE);
      break;
    case 4: /* MUL */
      _cpu.reg.aw = (uint16)(_cpu.exec.src0 * _cpu.reg.al);
      _cpu.psw.cy = _cpu.psw.v = _cpu.reg.ah ? 1 : 0;
      /* TODO: Undefined flags: S P Z AC */
      break;
    case 5: /* IMUL */
      _cpu.reg.aw = ((int16)(int8)_cpu.exec.src0) * ((int16)(int8)_cpu.reg.al);
      _cpu.psw.cy = _cpu.psw.v = _cpu.reg.ah ? 1 : 0;
      /* TODO: Undefined flags: S P Z AC */
      break;
    case 6: /* DIV */
      if(_cpu.exec.src0 == 0) {
        cpue_fault(INTVEC_DIVERR);
        break;
      }
      _cpu.exec.dst = _cpu.reg.aw / _cpu.exec.src0;
      if(!cpue_divbound8u()) {
        cpue_fault(INTVEC_DIVERR);
        break;
      }
      _cpu.reg.ah = _cpu.reg.aw % _cpu.exec.src0;
      _cpu.reg.al = _cpu.exec.dst;
      break;
    case 7: /* IDIV */
      if(_cpu.exec.src0 == 0) {
        cpue_fault(INTVEC_DIVERR);
        break;
      }
      _cpu.exec.dst = (int16)_cpu.reg.aw / ((int16)(int8)_cpu.exec.src0);
      if(!cpue_divbound8s()) {
        cpue_fault(INTVEC_DIVERR);
        break;
      }
      _cpu.reg.ah = (int16)_cpu.reg.aw % ((int16)(int8)_cpu.exec.src0);
      _cpu.reg.al = _cpu.exec.dst;
      break;
  }
  if(((_cpu.exec.modrm >> 3) & 6) == 2) {
    cpue_modrm_set_rmb(0, _cpu.exec.dst);
  }
}
CPUHELPER void cpue_i_grp3_16(void)
{
  switch((_cpu.exec.modrm >> 3) & 7) {
    case 0: /* TEST */
      _cpu.exec.src1 = cpu_fetch16();
      cpue_i_and16();
      break;
    case 1: /* unknown */
      logerr_pc("Unknown opcode grp3_16 op1\n");
      break;
    case 2: /* NOT */
      _cpu.exec.dst = ~_cpu.exec.src0;
      break;
    case 3: /* NEG */
      cpue_i_neg(TRUE);
      break;
    case 4: /* MUL */
      _cpu.exec.dst = _cpu.exec.src0 * _cpu.reg.aw;
      _cpu.reg.aw = (_cpu.exec.dst >>  0) & 0xFFFF;
      _cpu.reg.dw = (_cpu.exec.dst >> 16) & 0xFFFF;
      _cpu.psw.cy = _cpu.psw.v = _cpu.reg.dw ? 1 : 0;
      /* TODO: Undefined flags: S P Z AC */
      break;
    case 5: /* IMUL */
      _cpu.exec.dst = ((int32)(int16)_cpu.exec.src0) * ((int32)(int16)_cpu.reg.aw);
      _cpu.reg.aw = (_cpu.exec.dst >>  0) & 0xFFFF;
      _cpu.reg.dw = (_cpu.exec.dst >> 16) & 0xFFFF;
      _cpu.psw.cy = _cpu.psw.v = _cpu.reg.dw ? 1 : 0;
      /* TODO: Undefined flags: S P Z AC */
      break;
    case 6: /* DIV */
      if(_cpu.exec.src0 == 0) {
        cpue_fault(INTVEC_DIVERR);
        break;
      }
      _cpu.exec.src1 = (_cpu.reg.dw << 16) | _cpu.reg.aw;
      _cpu.exec.dst = _cpu.exec.src1 / _cpu.exec.src0;
      if(!cpue_divbound16u()) {
        cpue_fault(INTVEC_DIVERR);
        break;
      }
      _cpu.reg.dw = _cpu.exec.src1 % _cpu.exec.src0;
      _cpu.reg.aw = _cpu.exec.dst;
      break;
    case 7: /* IDIV */
      if(_cpu.exec.src0 == 0) {
        cpue_fault(INTVEC_DIVERR);
        break;
      }
      _cpu.exec.src1 = (_cpu.reg.dw << 16) | _cpu.reg.aw;
      _cpu.exec.dst = (int32)_cpu.exec.src1 / ((int32)(int16)_cpu.exec.src0);
      if(!cpue_divbound16s()) {
        cpue_fault(INTVEC_DIVERR);
        break;
      }
      _cpu.reg.dw = (int32)_cpu.exec.src1 % ((int32)(int16)_cpu.exec.src0);
      _cpu.reg.aw = _cpu.exec.dst;
      break;
  }
  if(((_cpu.exec.modrm >> 3) & 6) == 2) {
    cpue_modrm_set_rmw(0, _cpu.exec.dst);
  }
}
CPUHELPER void cpue_i_grp4_8(void)
{
  switch((_cpu.exec.modrm >> 3) & 7) {
    case 0: /* inc */
      cpue_i_inc8();
      break;
    case 1: /* dec */
      cpue_i_dec8();
      break;
    case 2: /* call */
      logerr_pc("Unknown opcode grp4_8 op2\n");
      cpue_i_call();
      break;
    case 3: /* call Mp */
      logerr_pc("Unknown opcode grp4_8 op3\n");
      _cpu.exec.src1 = cpue_modrm_get_rmb(1);
      cpue_i_callf();
      break;
    case 4: /* jmp */
      logerr_pc("Unknown opcode grp4_8 op4\n");
      _cpu.reg.ip = _cpu.exec.src0;
      break;
    case 5: /* jmp Mp */
      logerr_pc("Unknown opcode grp4_8 op5\n");
      _cpu.exec.src1 = cpue_modrm_get_rmb(1);
      cpue_i_jmpf();
      break;
    case 6: /* push */
      logerr_pc("Unknown opcode grp4_8 op6\n");
      cpue_i_push();
      break;
    case 7: /* pop?? */
      logerr_pc("Unknown opcode grp4_8 op7\n");
      break;
  }
  if(((_cpu.exec.modrm >> 3) & 6) == 0) {
    cpue_modrm_set_rmb(0, _cpu.exec.dst);
  }
}
CPUHELPER void cpue_i_grp4_16(void)
{
  switch((_cpu.exec.modrm >> 3) & 7) {
    case 0: /* inc */
      cpue_i_inc16();
      break;
    case 1: /* dec */
      cpue_i_dec16();
      break;
    case 2: /* call */
      cpue_i_call();
      break;
    case 3: /* call Mp */
      /* TODO: Need to actually push before reading destination */
      _cpu.exec.src1 = cpue_modrm_get_rmw(2);
      cpue_i_callf();
      break;
    case 4: /* jmp */
      _cpu.reg.ip = _cpu.exec.src0;
      break;
    case 5: /* jmp Mp */
      _cpu.exec.src1 = cpue_modrm_get_rmw(2);
      cpue_i_jmpf();
      break;
    case 6: /* push */
      cpue_i_push();
      break;
    case 7: /* pop?? */
      logerr_pc("Unknown opcode grp4_16 op7\n");
      break;
  }
  if(((_cpu.exec.modrm >> 3) & 6) == 0) {
    cpue_modrm_set_rmw(0, _cpu.exec.dst);
  }
}
CPUHELPER void cpue_i_imul(void)
{
  _cpu.exec.dst = ((int32)(int16)_cpu.exec.src0) * ((int32)(int16)_cpu.exec.src1);
  _cpu.psw.cy = _cpu.psw.v = TRUE;
  if((_cpu.exec.dst & 0xFFFF8000) == 0)
    _cpu.psw.cy = _cpu.psw.v = FALSE;
  if((_cpu.exec.dst & 0xFFFF8000) == 0xFFFF8000)
    _cpu.psw.cy = _cpu.psw.v = FALSE;
}
CPUHELPER void cpue_i_bound(void)
{
  uint16 lo, hi, val;
  lo = cpue_modrm_get_rmw(0);
  hi = cpue_modrm_get_rmw(2);
  val = cpue_modrm_get_regw();
  if(val < lo || val > hi) {
    cpu_interrupt(INTVEC_BOUND);
  }
}

/* v30mz doesn't support: (undefined result)
 *   CALLN: ED ED
 *   FPO2:  66 / 67
 *   REPC:  65
 *   REPNC: 64
 *   RETEM: ED FD
 *   0F prefix instructions
 */
int cpu_exec(void)
{
  switch(_cpu.insn) {
OP(0x00,add8_E_G    ,1) { cpue_modrm_r8_mem_reg();    cpue_i_add8(0);  cpue_modrm_set_rmb(0,_cpu.exec.dst); } OP_END();
OP(0x01,add16_E_G   ,1) { cpue_modrm_r16_mem_reg();   cpue_i_add16(0); cpue_modrm_set_rmw(0,_cpu.exec.dst); } OP_END();
OP(0x02,add8_G_E    ,1) { cpue_modrm_r8_reg_mem();    cpue_i_add8(0);  cpue_modrm_set_regb(_cpu.exec.dst); } OP_END();
OP(0x03,add16_G_E   ,1) { cpue_modrm_r16_reg_mem();   cpue_i_add16(0); cpue_modrm_set_regw(_cpu.exec.dst); } OP_END();
OP(0x04,add8_rAL_I  ,1) { cpue_al_imm8();             cpue_i_add8(0);  _cpu.reg.al = _cpu.exec.dst; } OP_END();
OP(0x05,add16_rAW_I ,1) { cpue_aw_imm16();            cpue_i_add16(0); _cpu.reg.aw = _cpu.exec.dst; } OP_END();
OP(0x06,push_rES    ,2) { _cpu.exec.src0=_cpu.reg.es; cpue_i_push(); } OP_END();
OP(0x07,pop_rES     ,3) {                             cpue_i_pop();    _cpu.reg.es = _cpu.exec.dst; } OP_END();

OP(0x08,or8_E_G     ,1) { cpue_modrm_r8_mem_reg();    cpue_i_or8();    cpue_modrm_set_rmb(0,_cpu.exec.dst); } OP_END();
OP(0x09,or16_E_G    ,1) { cpue_modrm_r16_mem_reg();   cpue_i_or16();   cpue_modrm_set_rmw(0,_cpu.exec.dst); } OP_END();
OP(0x0A,or8_G_E     ,1) { cpue_modrm_r8_reg_mem();    cpue_i_or8();    cpue_modrm_set_regb(_cpu.exec.dst); } OP_END();
OP(0x0B,or16_G_E    ,1) { cpue_modrm_r16_reg_mem();   cpue_i_or16();   cpue_modrm_set_regw(_cpu.exec.dst); } OP_END();
OP(0x0C,or8_rAL_I   ,1) { cpue_al_imm8();             cpue_i_or8();    _cpu.reg.al = _cpu.exec.dst; } OP_END();
OP(0x0D,or16_rAW_I  ,1) { cpue_aw_imm16();            cpue_i_or16();   _cpu.reg.aw = _cpu.exec.dst; } OP_END();
OP(0x0E,push_rCS    ,2) { _cpu.exec.src0=_cpu.reg.cs; cpue_i_push(); } OP_END();

OP(0x10,adc8_E_G    ,1) { cpue_modrm_r8_mem_reg();    cpue_i_add8(1);  cpue_modrm_set_rmb(0,_cpu.exec.dst); } OP_END();
OP(0x11,adc16_E_G   ,1) { cpue_modrm_r16_mem_reg();   cpue_i_add16(1); cpue_modrm_set_rmw(0,_cpu.exec.dst); } OP_END();
OP(0x12,adc8_G_E    ,1) { cpue_modrm_r8_reg_mem();    cpue_i_add8(1);  cpue_modrm_set_regb(_cpu.exec.dst); } OP_END();
OP(0x13,adc16_G_E   ,1) { cpue_modrm_r16_reg_mem();   cpue_i_add16(1); cpue_modrm_set_regw(_cpu.exec.dst); } OP_END();
OP(0x14,adc8_rAL_I  ,1) { cpue_al_imm8();             cpue_i_add8(1);  _cpu.reg.al = _cpu.exec.dst; } OP_END();
OP(0x15,adc16_rAW_I ,1) { cpue_aw_imm16();            cpue_i_add16(1); _cpu.reg.aw = _cpu.exec.dst; } OP_END();
OP(0x16,push_rSS    ,2) { _cpu.exec.src0=_cpu.reg.ss; cpue_i_push(); } OP_END();
OP(0x17,pop_rSS     ,3) {                             cpue_i_pop();    _cpu.reg.ss = _cpu.exec.dst; _cpu.irq_disable = TRUE; } OP_END();

OP(0x18,sbb8_E_G    ,1) { cpue_modrm_r8_mem_reg();    cpue_i_sub8(1);  cpue_modrm_set_rmb(0,_cpu.exec.dst); } OP_END();
OP(0x19,sbb16_E_G   ,1) { cpue_modrm_r16_mem_reg();   cpue_i_sub16(1); cpue_modrm_set_rmw(0,_cpu.exec.dst); } OP_END();
OP(0x1A,sbb8_G_E    ,1) { cpue_modrm_r8_reg_mem();    cpue_i_sub8(1);  cpue_modrm_set_regb(_cpu.exec.dst); } OP_END();
OP(0x1B,sbb16_G_E   ,1) { cpue_modrm_r16_reg_mem();   cpue_i_sub16(1); cpue_modrm_set_regw(_cpu.exec.dst); } OP_END();
OP(0x1C,sbb8_rAL_I  ,1) { cpue_al_imm8();             cpue_i_sub8(1);  _cpu.reg.al = _cpu.exec.dst; } OP_END();
OP(0x1D,sbb16_rAW_I ,1) { cpue_aw_imm16();            cpue_i_sub16(1); _cpu.reg.aw = _cpu.exec.dst; } OP_END();
OP(0x1E,push_rDS    ,2) { _cpu.exec.src0=_cpu.reg.ds; cpue_i_push(); } OP_END();
OP(0x1F,pop_rDS     ,3) {                             cpue_i_pop();    _cpu.reg.ds = _cpu.exec.dst; } OP_END();

OP(0x20,and8_E_G    ,1) { cpue_modrm_r8_mem_reg();    cpue_i_and8();   cpue_modrm_set_rmb(0,_cpu.exec.dst); } OP_END();
OP(0x21,and16_E_G   ,1) { cpue_modrm_r16_mem_reg();   cpue_i_and16();  cpue_modrm_set_rmw(0,_cpu.exec.dst); } OP_END();
OP(0x22,and8_G_E    ,1) { cpue_modrm_r8_reg_mem();    cpue_i_and8();   cpue_modrm_set_regb(_cpu.exec.dst); } OP_END();
OP(0x23,and16_G_E   ,1) { cpue_modrm_r16_reg_mem();   cpue_i_and16();  cpue_modrm_set_regw(_cpu.exec.dst); } OP_END();
OP(0x24,and8_rAL_I  ,1) { cpue_al_imm8();             cpue_i_and8();   _cpu.reg.al = _cpu.exec.dst; } OP_END();
OP(0x25,and16_rAW_I ,1) { cpue_aw_imm16();            cpue_i_and16();  _cpu.reg.aw = _cpu.exec.dst; } OP_END();
OP(0x26,pfxES       ,1) { cpue_i_segpfx(SREG_ES); } OP_END();
OP(0x27,daa        ,10) { cpue_i_daa_das(FALSE); } OP_END();

OP(0x28,sub8_E_G    ,1) { cpue_modrm_r8_mem_reg();    cpue_i_sub8(0);  cpue_modrm_set_rmb(0,_cpu.exec.dst); } OP_END();
OP(0x29,sub16_E_G   ,1) { cpue_modrm_r16_mem_reg();   cpue_i_sub16(0); cpue_modrm_set_rmw(0,_cpu.exec.dst); } OP_END();
OP(0x2A,sub8_G_E    ,1) { cpue_modrm_r8_reg_mem();    cpue_i_sub8(0);  cpue_modrm_set_regb(_cpu.exec.dst); } OP_END();
OP(0x2B,sub16_G_E   ,1) { cpue_modrm_r16_reg_mem();   cpue_i_sub16(0); cpue_modrm_set_regw(_cpu.exec.dst); } OP_END();
OP(0x2C,sub8_rAL_I  ,1) { cpue_al_imm8();             cpue_i_sub8(0);  _cpu.reg.al = _cpu.exec.dst; } OP_END();
OP(0x2D,sub16_rAW_I ,1) { cpue_aw_imm16();            cpue_i_sub16(0); _cpu.reg.aw = _cpu.exec.dst; } OP_END();
OP(0x2E,pfxCS       ,1) { cpue_i_segpfx(SREG_CS); } OP_END();
OP(0x2F,das        ,10) { cpue_i_daa_das(TRUE); } OP_END();

OP(0x30,xor8_E_G    ,1) { cpue_modrm_r8_mem_reg();    cpue_i_xor8();   cpue_modrm_set_rmb(0,_cpu.exec.dst); } OP_END();
OP(0x31,xor16_E_G   ,1) { cpue_modrm_r16_mem_reg();   cpue_i_xor16();  cpue_modrm_set_rmw(0,_cpu.exec.dst); } OP_END();
OP(0x32,xor8_G_E    ,1) { cpue_modrm_r8_reg_mem();    cpue_i_xor8();   cpue_modrm_set_regb(_cpu.exec.dst); } OP_END();
OP(0x33,xor16_G_E   ,1) { cpue_modrm_r16_reg_mem();   cpue_i_xor16();  cpue_modrm_set_regw(_cpu.exec.dst); } OP_END();
OP(0x34,xor8_rAL_I  ,1) { cpue_al_imm8();             cpue_i_xor8();   _cpu.reg.al = _cpu.exec.dst; } OP_END();
OP(0x35,xor16_rAW_I ,1) { cpue_aw_imm16();            cpue_i_xor16();  _cpu.reg.aw = _cpu.exec.dst; } OP_END();
OP(0x36,pfxSS       ,1) { cpue_i_segpfx(SREG_SS); } OP_END();
OP(0x37,aaa         ,9) { cpue_i_aaa_aas(FALSE); } OP_END();

OP(0x38,cmp8_E_G    ,1) { cpue_modrm_r8_mem_reg();    cpue_i_sub8(0);  } OP_END();
OP(0x39,cmp16_E_G   ,1) { cpue_modrm_r16_mem_reg();   cpue_i_sub16(0); } OP_END();
OP(0x3A,cmp8_G_E    ,1) { cpue_modrm_r8_reg_mem();    cpue_i_sub8(0);  } OP_END();
OP(0x3B,cmp16_G_E   ,1) { cpue_modrm_r16_reg_mem();   cpue_i_sub16(0); } OP_END();
OP(0x3C,cmp8_rAL_I  ,1) { cpue_al_imm8();             cpue_i_sub8(0);  } OP_END();
OP(0x3D,cmp16_rAW_I ,1) { cpue_aw_imm16();            cpue_i_sub16(0); } OP_END();
OP(0x3E,pfxDS       ,1) { cpue_i_segpfx(SREG_DS); } OP_END();
OP(0x3F,aas         ,9) { cpue_i_aaa_aas(TRUE); } OP_END();

OP(0x40,inc16_rAW   ,1) { _cpu.exec.src0=_cpu.reg.aw; cpue_i_inc16(); _cpu.reg.aw = _cpu.exec.dst; } OP_END();
OP(0x41,inc16_rCW   ,1) { _cpu.exec.src0=_cpu.reg.cw; cpue_i_inc16(); _cpu.reg.cw = _cpu.exec.dst; } OP_END();
OP(0x42,inc16_rDW   ,1) { _cpu.exec.src0=_cpu.reg.dw; cpue_i_inc16(); _cpu.reg.dw = _cpu.exec.dst; } OP_END();
OP(0x43,inc16_rBW   ,1) { _cpu.exec.src0=_cpu.reg.bw; cpue_i_inc16(); _cpu.reg.bw = _cpu.exec.dst; } OP_END();
OP(0x44,inc16_rSP   ,1) { _cpu.exec.src0=_cpu.reg.sp; cpue_i_inc16(); _cpu.reg.sp = _cpu.exec.dst; } OP_END();
OP(0x45,inc16_rBP   ,1) { _cpu.exec.src0=_cpu.reg.bp; cpue_i_inc16(); _cpu.reg.bp = _cpu.exec.dst; } OP_END();
OP(0x46,inc16_rIX   ,1) { _cpu.exec.src0=_cpu.reg.ix; cpue_i_inc16(); _cpu.reg.ix = _cpu.exec.dst; } OP_END();
OP(0x47,inc16_rIY   ,1) { _cpu.exec.src0=_cpu.reg.iy; cpue_i_inc16(); _cpu.reg.iy = _cpu.exec.dst; } OP_END();

OP(0x48,dec16_rAW   ,1) { _cpu.exec.src0=_cpu.reg.aw; cpue_i_dec16(); _cpu.reg.aw = _cpu.exec.dst; } OP_END();
OP(0x49,dec16_rCW   ,1) { _cpu.exec.src0=_cpu.reg.cw; cpue_i_dec16(); _cpu.reg.cw = _cpu.exec.dst; } OP_END();
OP(0x4A,dec16_rDW   ,1) { _cpu.exec.src0=_cpu.reg.dw; cpue_i_dec16(); _cpu.reg.dw = _cpu.exec.dst; } OP_END();
OP(0x4B,dec16_rBW   ,1) { _cpu.exec.src0=_cpu.reg.bw; cpue_i_dec16(); _cpu.reg.bw = _cpu.exec.dst; } OP_END();
OP(0x4C,dec16_rSP   ,1) { _cpu.exec.src0=_cpu.reg.sp; cpue_i_dec16(); _cpu.reg.sp = _cpu.exec.dst; } OP_END();
OP(0x4D,dec16_rBP   ,1) { _cpu.exec.src0=_cpu.reg.bp; cpue_i_dec16(); _cpu.reg.bp = _cpu.exec.dst; } OP_END();
OP(0x4E,dec16_rIX   ,1) { _cpu.exec.src0=_cpu.reg.ix; cpue_i_dec16(); _cpu.reg.ix = _cpu.exec.dst; } OP_END();
OP(0x4F,dec16_rIY   ,1) { _cpu.exec.src0=_cpu.reg.iy; cpue_i_dec16(); _cpu.reg.iy = _cpu.exec.dst; } OP_END();

OP(0x50,push_rAW    ,1) { _cpu.exec.src0=_cpu.reg.aw; cpue_i_push(); } OP_END();
OP(0x51,push_rCW    ,1) { _cpu.exec.src0=_cpu.reg.cw; cpue_i_push(); } OP_END();
OP(0x52,push_rDW    ,1) { _cpu.exec.src0=_cpu.reg.dw; cpue_i_push(); } OP_END();
OP(0x53,push_rBW    ,1) { _cpu.exec.src0=_cpu.reg.bw; cpue_i_push(); } OP_END();
OP(0x54,push_rSP    ,1) { _cpu.exec.src0=_cpu.reg.sp; cpue_i_push(); } OP_END();
OP(0x55,push_rBP    ,1) { _cpu.exec.src0=_cpu.reg.bp; cpue_i_push(); } OP_END();
OP(0x56,push_rIX    ,1) { _cpu.exec.src0=_cpu.reg.ix; cpue_i_push(); } OP_END();
OP(0x57,push_rIY    ,1) { _cpu.exec.src0=_cpu.reg.iy; cpue_i_push(); } OP_END();

OP(0x58,pop_rAW     ,1) {                             cpue_i_pop();  _cpu.reg.aw = _cpu.exec.dst; } OP_END();
OP(0x59,pop_rCW     ,1) {                             cpue_i_pop();  _cpu.reg.cw = _cpu.exec.dst; } OP_END();
OP(0x5A,pop_rDW     ,1) {                             cpue_i_pop();  _cpu.reg.dw = _cpu.exec.dst; } OP_END();
OP(0x5B,pop_rBW     ,1) {                             cpue_i_pop();  _cpu.reg.bw = _cpu.exec.dst; } OP_END();
OP(0x5C,pop_rSP     ,1) {                             cpue_i_pop();  _cpu.reg.sp = _cpu.exec.dst; } OP_END();
OP(0x5D,pop_rBP     ,1) {                             cpue_i_pop();  _cpu.reg.bp = _cpu.exec.dst; } OP_END();
OP(0x5E,pop_rIX     ,1) {                             cpue_i_pop();  _cpu.reg.ix = _cpu.exec.dst; } OP_END();
OP(0x5F,pop_rIY     ,1) {                             cpue_i_pop();  _cpu.reg.iy = _cpu.exec.dst; } OP_END();

OP(0x60,pusha       ,9) { cpue_i_pusha(); } OP_END();
OP(0x61,popa        ,8) { cpue_i_popa(); } OP_END();
OP(0x62,bound_G_M  ,13) { cpue_modrm_get(); cpue_i_bound(); } OP_END();

OP(0x68,push_Iw     ,1) { cpue_imm16(); cpue_i_push(); } OP_END();
OP(0x69,imul_G_E_Iw ,3) { cpue_modrm_r16_reg_mem(); cpue_imm16(); cpue_i_imul(); cpue_modrm_set_regw(_cpu.exec.dst); } OP_END();
OP(0x6A,push_Ib     ,1) { cpue_imm8s(); cpue_i_push(); } OP_END();
OP(0x6B,imul_G_E_Ib ,3) { cpue_modrm_r16_reg_mem(); cpue_imm8s(); cpue_i_imul(); cpue_modrm_set_regw(_cpu.exec.dst); } OP_END();
OP(0x6C,insb        ,6) { cpue_i_insb(); } OP_END();
OP(0x6D,insw        ,6) { cpue_i_insw(); } OP_END();
OP(0x6E,outsb       ,7) { cpue_i_outsb(); } OP_END();
OP(0x6F,outsw       ,7) { cpue_i_outsw(); } OP_END();

OP(0x70,jo          ,1) { cpue_imm8s(); cpue_i_jmp( _cpu.psw.v); } OP_END();
OP(0x71,jno         ,1) { cpue_imm8s(); cpue_i_jmp(!_cpu.psw.v); } OP_END();
OP(0x72,jc          ,1) { cpue_imm8s(); cpue_i_jmp( _cpu.psw.cy); } OP_END();
OP(0x73,jnc         ,1) { cpue_imm8s(); cpue_i_jmp(!_cpu.psw.cy); } OP_END();
OP(0x74,jz          ,1) { cpue_imm8s(); cpue_i_jmp( _cpu.psw.z); } OP_END();
OP(0x75,jnz         ,1) { cpue_imm8s(); cpue_i_jmp(!_cpu.psw.z); } OP_END();
OP(0x76,jcz         ,1) { cpue_imm8s(); cpue_i_jmp( (_cpu.psw.z || _cpu.psw.cy)); } OP_END();
OP(0x77,jncz        ,1) { cpue_imm8s(); cpue_i_jmp(!(_cpu.psw.z || _cpu.psw.cy)); } OP_END();
OP(0x78,js          ,1) { cpue_imm8s(); cpue_i_jmp( _cpu.psw.s); } OP_END();
OP(0x79,jns         ,1) { cpue_imm8s(); cpue_i_jmp(!_cpu.psw.s); } OP_END();
OP(0x7A,jp          ,1) { cpue_imm8s(); cpue_i_jmp( _cpu.psw.p); } OP_END();
OP(0x7B,jnp         ,1) { cpue_imm8s(); cpue_i_jmp(!_cpu.psw.p); } OP_END();
OP(0x7C,jl          ,1) { cpue_imm8s(); cpue_i_jmp((_cpu.psw.s != _cpu.psw.v) && !_cpu.psw.z); } OP_END();
OP(0x7D,jnl         ,1) { cpue_imm8s(); cpue_i_jmp((_cpu.psw.s == _cpu.psw.v) ||  _cpu.psw.z); } OP_END();
OP(0x7E,jle         ,1) { cpue_imm8s(); cpue_i_jmp((_cpu.psw.s != _cpu.psw.v) ||  _cpu.psw.z); } OP_END();
OP(0x7F,jnle        ,1) { cpue_imm8s(); cpue_i_jmp((_cpu.psw.s == _cpu.psw.v) && !_cpu.psw.z); } OP_END();

OP(0x80,grp1u8      ,1) { cpue_modrmEb_imm8();  cpue_i_grp1_8(); } OP_END();
OP(0x81,grp1u16     ,1) { cpue_modrmEw_imm16(); cpue_i_grp1_16(); } OP_END();
OP(0x82,grp1s8      ,1) { cpue_modrmEb_imm8s(); cpue_i_grp1_8(); } OP_END();
OP(0x83,grp1s16     ,1) { cpue_modrmEw_imm8s(); cpue_i_grp1_16(); } OP_END();
OP(0x84,test8_G_E   ,1) { cpue_modrm_r8_mem_reg();    cpue_i_and8(); } OP_END();
OP(0x85,test16_G_E  ,1) { cpue_modrm_r16_mem_reg();   cpue_i_and16(); } OP_END();
OP(0x86,xchg8_G_E   ,3) { cpue_modrm_r8_mem_reg();  cpue_modrm_set_rmb(0,_cpu.exec.src1); cpue_modrm_set_regb(_cpu.exec.src0); } OP_END();
OP(0x87,xchg16_G_E  ,3) { cpue_modrm_r16_mem_reg(); cpue_modrm_set_rmw(0,_cpu.exec.src1); cpue_modrm_set_regw(_cpu.exec.src0); } OP_END();

OP(0x88,mov8_E_G    ,1) { cpue_modrm_get(); cpue_modrm_set_rmb(0,cpue_modrm_get_regb()); } OP_END();
OP(0x89,mov16_E_G   ,1) { cpue_modrm_get(); cpue_modrm_set_rmw(0,cpue_modrm_get_regw()); } OP_END();
OP(0x8A,mov8_G_E    ,1) { cpue_modrm_get(); cpue_modrm_set_regb(cpue_modrm_get_rmb(0)); } OP_END();
OP(0x8B,mov16_G_E   ,1) { cpue_modrm_get(); cpue_modrm_set_regw(cpue_modrm_get_rmw(0)); } OP_END();
OP(0x8C,mov16_E_Seg ,1) { cpue_modrm_get(); cpue_modrm_set_rmw(0,_cpu.reg.sreg[(_cpu.exec.modrm >> 3) & 3]); _cpu.irq_disable = TRUE;
  if((_cpu.exec.modrm >> 3) & 4)
    logerr_pc("Bad segment register [%d]\n", (_cpu.exec.modrm >> 3) & 7);
} OP_END();
OP(0x8D,lea_G_M     ,1) { cpue_modrm_get(); cpue_modrm_get_ea(); cpue_modrm_set_regw(_cpu.exec.ea);
  if(_cpu.exec.ea_seg == -1)
    logerr_pc("Bad LEA ModR/M [%02X]\n", _cpu.exec.modrm);
} OP_END();
OP(0x8E,mov16_Seg_E ,2) { cpue_modrm_get(); _cpu.reg.sreg[(_cpu.exec.modrm >> 3) & 3] = cpue_modrm_get_rmw(0);
  if(((_cpu.exec.modrm >> 3) & 3) == SREG_SS)
    _cpu.irq_disable = TRUE;
  if((_cpu.exec.modrm >> 3) & 4)
    logerr_pc("Bad segment register [%d]\n", (_cpu.exec.modrm >> 3) & 7);
} OP_END();
OP(0x8F,pop_Ew      ,1) { cpue_modrm_get(); cpue_i_pop(); cpue_modrm_set_rmw(0,_cpu.exec.dst); } OP_END();

OP(0x90,nop         ,1) { } OP_END();
OP(0x91,xchg_rCW_rAW,3) { uint16 tmp = _cpu.reg.aw; _cpu.reg.aw = _cpu.reg.cw; _cpu.reg.cw = tmp; } OP_END();
OP(0x92,xchg_rDW_rAW,3) { uint16 tmp = _cpu.reg.aw; _cpu.reg.aw = _cpu.reg.dw; _cpu.reg.dw = tmp; } OP_END();
OP(0x93,xchg_rBW_rAW,3) { uint16 tmp = _cpu.reg.aw; _cpu.reg.aw = _cpu.reg.bw; _cpu.reg.bw = tmp; } OP_END();
OP(0x94,xchg_rSP_rAW,3) { uint16 tmp = _cpu.reg.aw; _cpu.reg.aw = _cpu.reg.sp; _cpu.reg.sp = tmp; } OP_END();
OP(0x95,xchg_rBP_rAW,3) { uint16 tmp = _cpu.reg.aw; _cpu.reg.aw = _cpu.reg.bp; _cpu.reg.bp = tmp; } OP_END();
OP(0x96,xchg_rIX_rAW,3) { uint16 tmp = _cpu.reg.aw; _cpu.reg.aw = _cpu.reg.ix; _cpu.reg.ix = tmp; } OP_END();
OP(0x97,xchg_rIY_rAW,3) { uint16 tmp = _cpu.reg.aw; _cpu.reg.aw = _cpu.reg.iy; _cpu.reg.iy = tmp; } OP_END();

OP(0x98,cbw         ,1) { _cpu.reg.ah = (_cpu.reg.al & 0x80) ? 0xFF : 0; } OP_END();
OP(0x99,cwd         ,1) { _cpu.reg.dw = (_cpu.reg.ah & 0x80) ? 0xFFFF : 0; } OP_END();
OP(0x9A,call_Ap    ,10) { cpue_immfar(); cpue_i_callf(); } OP_END();
OP(0x9B,wait        ,1) { logdbg_pc("WAIT instruction\n"); } OP_END();
OP(0x9C,pushf       ,2) { cpue_i_pushf(); } OP_END();
OP(0x9D,popf        ,3) { cpue_i_popf(); _cpu.irq_disable = TRUE; } OP_END();
OP(0x9E,sahf        ,4) { cpue_i_sahf(); } OP_END();
OP(0x9F,lahf        ,2) { cpue_i_lahf(); } OP_END();

OP(0xA0,mov8_rAL_O  ,1) { cpue_imm16(); cpue_i_read8(SREG_DS);  _cpu.reg.al = _cpu.exec.dst; } OP_END();
OP(0xA1,mov16_rAW_O ,1) { cpue_imm16(); cpue_i_read16(SREG_DS); _cpu.reg.aw = _cpu.exec.dst; } OP_END();
OP(0xA2,mov8_O_rAL  ,1) { cpue_imm16(); _cpu.exec.dst = _cpu.reg.al; cpue_i_write8(SREG_DS); } OP_END();
OP(0xA3,mov16_O_rAW ,1) { cpue_imm16(); _cpu.exec.dst = _cpu.reg.aw; cpue_i_write16(SREG_DS); } OP_END();
OP(0xA4,movsb       ,5) { cpue_i_movsb(); } OP_END();
OP(0xA5,movsw       ,5) { cpue_i_movsw(); } OP_END();
OP(0xA6,cmpsb       ,6) { cpue_i_cmpsb(); } OP_END();
OP(0xA7,cmpsw       ,6) { cpue_i_cmpsw(); } OP_END();

OP(0xA8,test8_rAL_I ,1) { cpue_al_imm8();             cpue_i_and8();  } OP_END();
OP(0xA9,test16_rAW_I,1) { cpue_aw_imm16();            cpue_i_and16(); } OP_END();
OP(0xAA,stosb       ,3) { cpue_i_stosb(); } OP_END();
OP(0xAB,stosw       ,3) { cpue_i_stosw(); } OP_END();
OP(0xAC,lodsb       ,3) { cpue_i_lodsb(); } OP_END();
OP(0xAD,lodsw       ,3) { cpue_i_lodsw(); } OP_END();
OP(0xAE,scasb       ,4) { cpue_i_scasb(); } OP_END();
OP(0xAF,scasw       ,4) { cpue_i_scasw(); } OP_END();

OP(0xB0,mov8_rAL_I  ,1) { cpue_imm8(); _cpu.reg.al = _cpu.exec.src0; } OP_END();
OP(0xB1,mov8_rCL_I  ,1) { cpue_imm8(); _cpu.reg.cl = _cpu.exec.src0; } OP_END();
OP(0xB2,mov8_rDL_I  ,1) { cpue_imm8(); _cpu.reg.dl = _cpu.exec.src0; } OP_END();
OP(0xB3,mov8_rBL_I  ,1) { cpue_imm8(); _cpu.reg.bl = _cpu.exec.src0; } OP_END();
OP(0xB4,mov8_rAH_I  ,1) { cpue_imm8(); _cpu.reg.ah = _cpu.exec.src0; } OP_END();
OP(0xB5,mov8_rCH_I  ,1) { cpue_imm8(); _cpu.reg.ch = _cpu.exec.src0; } OP_END();
OP(0xB6,mov8_rDH_I  ,1) { cpue_imm8(); _cpu.reg.dh = _cpu.exec.src0; } OP_END();
OP(0xB7,mov8_rBH_I  ,1) { cpue_imm8(); _cpu.reg.bh = _cpu.exec.src0; } OP_END();

OP(0xB8,mov16_rAW_I ,1) { cpue_imm16(); _cpu.reg.aw = _cpu.exec.src0; } OP_END();
OP(0xB9,mov16_rCW_I ,1) { cpue_imm16(); _cpu.reg.cw = _cpu.exec.src0; } OP_END();
OP(0xBA,mov16_rDW_I ,1) { cpue_imm16(); _cpu.reg.dw = _cpu.exec.src0; } OP_END();
OP(0xBB,mov16_rBW_I ,1) { cpue_imm16(); _cpu.reg.bw = _cpu.exec.src0; } OP_END();
OP(0xBC,mov16_rSP_I ,1) { cpue_imm16(); _cpu.reg.sp = _cpu.exec.src0; } OP_END();
OP(0xBD,mov16_rBP_I ,1) { cpue_imm16(); _cpu.reg.bp = _cpu.exec.src0; } OP_END();
OP(0xBE,mov16_rIX_I ,1) { cpue_imm16(); _cpu.reg.ix = _cpu.exec.src0; } OP_END();
OP(0xBF,mov16_rIY_I ,1) { cpue_imm16(); _cpu.reg.iy = _cpu.exec.src0; } OP_END();

OP(0xC0,grp2_8_E_I  ,3) { cpue_modrmEb_imm8(); cpue_i_grp2_8();  cpue_modrm_set_rmb(0,_cpu.exec.dst); } OP_END();
OP(0xC1,grp2_16_E_I ,3) { cpue_modrmEw_imm8(); cpue_i_grp2_16(); cpue_modrm_set_rmw(0,_cpu.exec.dst); } OP_END();
OP(0xC2,ret_Iw      ,6) { cpue_imm16(); cpue_i_ret(TRUE); } OP_END();
OP(0xC3,ret         ,6) {               cpue_i_ret(FALSE); } OP_END();
OP(0xC4,les_Gw_Mp   ,6) { cpue_modrm_get(); cpue_i_ldes(); _cpu.reg.es = _cpu.exec.dst; } OP_END();
OP(0xC5,lds_Gw_Mp   ,6) { cpue_modrm_get(); cpue_i_ldes(); _cpu.reg.ds = _cpu.exec.dst; } OP_END();
OP(0xC6,mov8_E_I    ,1) { cpue_modrm_get(); cpue_imm8();  cpue_modrm_set_rmb(0,_cpu.exec.src0); } OP_END();
OP(0xC7,mov16_E_I   ,1) { cpue_modrm_get(); cpue_imm16(); cpue_modrm_set_rmw(0,_cpu.exec.src0); } OP_END();

OP(0xC8,enter_Iw_Ib ,8) { cpue_imm16_imm8(); cpue_i_enter(); } OP_END();
OP(0xC9,leave       ,2) { cpue_i_leave(); } OP_END();
OP(0xCA,retf_Iw     ,9) { cpue_imm16(); cpue_i_retf(TRUE); } OP_END();
OP(0xCB,retf        ,8) {               cpue_i_retf(FALSE); } OP_END();
OP(0xCC,int3        ,9) { _cpu.exec.src0 = INTVEC_BRK3; cpue_i_int(); } OP_END();
OP(0xCD,int_Ib     ,10) { cpue_imm8();                  cpue_i_int(); } OP_END();
OP(0xCE,into        ,6) { cpue_i_into(); } OP_END();
OP(0xCF,iret       ,10) { cpue_i_retf(FALSE); cpue_i_popf(); _cpu.irq_disable = TRUE; } OP_END();

OP(0xD0,grp2_8_E_1  ,1) { cpue_modrmEb(); _cpu.exec.src1 = 1; cpue_i_grp2_8();  cpue_modrm_set_rmb(0,_cpu.exec.dst); } OP_END();
OP(0xD1,grp2_16_E_1 ,1) { cpue_modrmEw(); _cpu.exec.src1 = 1; cpue_i_grp2_16(); cpue_modrm_set_rmw(0,_cpu.exec.dst); } OP_END();
OP(0xD2,grp2_8_E_rCL,3) { cpue_modrmEb(); _cpu.exec.src1 = _cpu.reg.cl; cpue_i_grp2_8();  cpue_modrm_set_rmb(0,_cpu.exec.dst); } OP_END();
OP(0xD3,grp2_16_E_rCL,3){ cpue_modrmEw(); _cpu.exec.src1 = _cpu.reg.cl; cpue_i_grp2_16(); cpue_modrm_set_rmw(0,_cpu.exec.dst); } OP_END();
OP(0xD4,aam8_I     ,17) { cpue_imm8(); cpue_i_aam(); } OP_END();
OP(0xD5,aad8_I      ,6) { cpue_imm8(); cpue_i_aad(); } OP_END();
OP(0xD7,xlat        ,5) { cpue_i_xlat(); _cpu.reg.al = _cpu.exec.dst; } OP_END();

/* FPO1 instructions are handled as NOPs, according to manual */
// HACK!!
OP(0xD8,fpo1_D8     ,1) { cpu_fetch8(); } OP_END();
OP(0xD9,fpo1_D9     ,1) { cpu_fetch8(); } OP_END();
OP(0xDA,fpo1_DA     ,1) { cpu_fetch8(); } OP_END();
OP(0xDB,fpo1_DB     ,1) { cpu_fetch8(); } OP_END();
OP(0xDC,fpo1_DC     ,1) { cpu_fetch8(); } OP_END();
OP(0xDD,fpo1_DD     ,1) { cpu_fetch8(); cpu_fetch16(); } OP_END();
OP(0xDE,fpo1_DE     ,1) { cpu_fetch8(); } OP_END();
OP(0xDF,fpo1_DF     ,1) { cpu_fetch8(); } OP_END();

OP(0xE0,loopnz      ,3) { cpue_imm8s(); cpue_i_loop(!_cpu.psw.z,TRUE); } OP_END();
OP(0xE1,loopz       ,3) { cpue_imm8s(); cpue_i_loop( _cpu.psw.z,TRUE); } OP_END();
OP(0xE2,loop        ,2) { cpue_imm8s(); cpue_i_loop(TRUE,FALSE); } OP_END();
OP(0xE3,jcxz        ,1) { cpue_imm8s(); cpue_i_jcxz(); } OP_END();
OP(0xE4,in8_rAL_I   ,6) { cpue_imm8();  cpue_i_in8();  _cpu.reg.al = _cpu.exec.dst; } OP_END();
OP(0xE5,in16_rAW_I  ,6) { cpue_imm8();  cpue_i_in16(); _cpu.reg.aw = _cpu.exec.dst; } OP_END();
OP(0xE6,out8_I_rAL  ,6) { cpue_imm8(); _cpu.exec.src1 = _cpu.reg.al; cpue_i_out8(); } OP_END();
OP(0xE7,out16_I_rAW ,6) { cpue_imm8(); _cpu.exec.src1 = _cpu.reg.aw; cpue_i_out16(); } OP_END();

OP(0xE8,call_near   ,5) { cpue_imm16();  cpue_i_calln(); } OP_END();
OP(0xE9,jmp_near    ,4) { cpue_imm16();  cpue_i_jmpn();  } OP_END();
OP(0xEA,jmp_far     ,7) { cpue_immfar(); cpue_i_jmpf();  } OP_END();
OP(0xEB,jmp_short   ,4) { cpue_imm8s();  cpue_i_jmps();  } OP_END();
OP(0xEC,in8_rAL_rDW ,6) { _cpu.exec.src0 = _cpu.reg.dw; cpue_i_in8();  _cpu.reg.al = _cpu.exec.dst; } OP_END();
OP(0xED,in16_rAW_rDW,6) { _cpu.exec.src0 = _cpu.reg.dw; cpue_i_in16(); _cpu.reg.aw = _cpu.exec.dst; } OP_END();
OP(0xEE,out8_rDW_rAL,6) { _cpu.exec.src0 = _cpu.reg.dw; _cpu.exec.src1 = _cpu.reg.al; cpue_i_out8(); } OP_END();
OP(0xEF,out16_rDW_rAW,6){ _cpu.exec.src0 = _cpu.reg.dw; _cpu.exec.src1 = _cpu.reg.aw; cpue_i_out16(); } OP_END();

OP(0xF0,lock        ,1) { loginfo_pc("LOCK prefix\n"); _cpu.irq_disable = TRUE; } OP_END();
OP(0xF2,repnz       ,5) { cpue_i_rep(FALSE); } OP_END();
OP(0xF3,repz        ,5) { cpue_i_rep(TRUE); } OP_END();
OP(0xF4,hlt         ,9) { cpue_i_halt(); } OP_END();
OP(0xF5,cmc         ,4) { _cpu.psw.cy = !_cpu.psw.cy; } OP_END();
OP(0xF6,grp3_8_E    ,1) { cpue_modrmEb(); cpue_i_grp3_8(); } OP_END();
OP(0xF7,grp3_16_E   ,1) { cpue_modrmEw(); cpue_i_grp3_16(); } OP_END();

OP(0xF8,clc         ,4) { _cpu.psw.cy = FALSE; } OP_END();
OP(0xF9,stc         ,4) { _cpu.psw.cy = TRUE; } OP_END();
OP(0xFA,cli         ,4) { _cpu.psw.ie = FALSE; } OP_END();
OP(0xFB,sti         ,4) { _cpu.psw.ie = TRUE; _cpu.irq_disable = TRUE; } OP_END();
OP(0xFC,cld         ,4) { _cpu.psw.dir= FALSE; } OP_END();
OP(0xFD,std         ,4) { _cpu.psw.dir= TRUE; } OP_END();
OP(0xFE,grp4_8_E    ,1) { cpue_modrmEb(); cpue_i_grp4_8(); } OP_END();
OP(0xFF,grp4_16_E   ,1) { cpue_modrmEw(); cpue_i_grp4_16(); } OP_END();
    default:
      logerr_pc("Unhandled opcode [%02X]\n", _cpu.insn);
      break;
  }
  return 0;
}
