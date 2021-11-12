
longlong ********
RtlFlsAlloc(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
           undefined8 param_5,uint *FLSIndex)

{
  longlong _NT_TIB;
  longlong lVar2;
  code *pcVar3;
  uint uVar4;
  longlong ********pppppppplVar5;
  longlong ********pppppppplVar6;
  longlong ********memory1;
  longlong lVar8;
  longlong ********rtn;
  undefined *puVar10;
  undefined *puVar11;
  longlong in_GS_OFFSET;
  undefined8 uVar12;
  undefined8 extraout_XMM0_Qa;
  undefined8 extraout_XMM0_Qa_00;
  undefined auStack72 [8];
  undefined auStack64 [24];
  
                    /* 0x35e20  987  RtlFlsAlloc */
  puVar11 = auStack72;
  puVar10 = auStack72;
  _NT_TIB = *(longlong *)(in_GS_OFFSET + 0x30);
  rtn = (longlong ********)0x0;

  
  uVar4 = 0;
  memory1 = rtn;
  if (*(longlong *)(_NT_TIB + FlsData) == 0) { //_NT_TIB.FlsData
    memory1 =
         (longlong ********)
         RtlAllocateHeap(*(longlong *********)(*(longlong *)(in_GS_OFFSET + 0x60) + 0x30),
                         NtdllBaseTag + 0x2c0000U | 8,(longlong ********)0x410);
    if (memory1 == (longlong ********)0x0) {
      return (longlong ********)0xc0000017;
    }
    *(longlong *********)(_NT_TIB + FlsData) = memory1;
  }

  lVar2 = *(longlong *)(_NT_TIB + 0x60);
  pppppppplVar5 = rtn;
  if (*(longlong *)(lVar2 + 800) == 0) {
    pppppppplVar5 =
         (longlong ********)
         RtlAllocateHeap(*(longlong *********)(*(longlong *)(in_GS_OFFSET + 0x60) + 0x30),
                         NtdllBaseTag + 0x2c0000,(longlong ********)0x800);
    if (pppppppplVar5 == (longlong ********)0x0) {
      rtn = (longlong ********)0xc0000017;
      uVar12 = extraout_XMM0_Qa;
      goto LAB_1800b62b4;
    }
    lVar8 = 0x80;
    pppppppplVar6 = pppppppplVar5;
    do {
      *pppppppplVar6 = (longlong *******)0x0;
      pppppppplVar6[1] = (longlong *******)0x0;
      pppppppplVar6 = pppppppplVar6 + 2;
      lVar8 = lVar8 + -1;
    } while (lVar8 != 0);
  }
  RtlAcquireSRWLockExclusive((ulonglong *)&RtlpFlsLock);
  if ((pppppppplVar5 != (longlong ********)0x0) && (*(longlong *)(lVar2 + 800) == 0)) {
    *(longlong *********)(lVar2 + 800) = pppppppplVar5;
    pppppppplVar5 = rtn;
  }
  if (memory1 == (longlong ********)0x0) {
LAB_180035e93:
    uVar4 = RtlFindClearBitsAndSet(*(uint **)(lVar2 + 0x338),1,1);
    if (uVar4 == 0xffffffff) goto LAB_1800b628a;
    *(undefined8 *)(*(longlong *)(lVar2 + 800) + (ulonglong)uVar4 * 0x10) = param_5;
    *(undefined8 *)(*(longlong *)(_NT_TIB + FlsData) + 0x10 + (ulonglong)uVar4 * 8) = 0;
    puVar11 = auStack72;
    if (*(uint *)(lVar2 + 0x350) <= uVar4 && uVar4 != *(uint *)(lVar2 + 0x350)) {
      *(uint *)(lVar2 + 0x350) = uVar4;
      rtn = (longlong ********)0x0;
      puVar11 = auStack72;
    }
  }
  else {
    pppppppplVar6 = *(longlong *********)(lVar2 + 0x330);
    if (*pppppppplVar6 == (longlong *******)(lVar2 + 0x328)) {
      *memory1 = (longlong *******)(lVar2 + 0x328);
      memory1[1] = (longlong *******)pppppppplVar6;
      *pppppppplVar6 = (longlong *******)memory1;
      *(longlong *********)(lVar2 + 0x330) = memory1;
      memory1 = rtn;
      goto LAB_180035e93;
    }
    pcVar3 = (code *)swi(0x29);
    (*pcVar3)(3);
    puVar10 = auStack64;
LAB_1800b628a:
    rtn = (longlong ********)0xc0000017;
    puVar11 = puVar10;
  }
  *(undefined8 *)(puVar11 + -8) = 0x180035eec;
  uVar12 = RtlReleaseSRWLockExclusive((ulonglong *)&RtlpFlsLock);
  if (pppppppplVar5 != (longlong ********)0x0) {
    pppppppplVar6 = *(longlong *********)(*(longlong *)(in_GS_OFFSET + 0x60) + 0x30);
    *(undefined8 *)(puVar11 + -8) = 0x1800b62ab;
    RtlFreeHeap(uVar12,param_2,param_3,param_4,pppppppplVar6,0,pppppppplVar5);
    uVar12 = extraout_XMM0_Qa_00;
  }
  if (-1 < (int)rtn) {
    *FLSIndex = uVar4;
    return rtn;
  }
LAB_1800b62b4:
  if (memory1 != (longlong ********)0x0) {
    *(undefined8 *)(_NT_TIB + FlsData) = 0;
    pppppppplVar5 = *(longlong *********)(*(longlong *)(in_GS_OFFSET + 0x60) + 0x30);
    *(undefined8 *)(puVar11 + -8) = 0x1800b62db;
    RtlFreeHeap(uVar12,param_2,param_3,param_4,pppppppplVar5,0,memory1);
  }
  return rtn;
}
