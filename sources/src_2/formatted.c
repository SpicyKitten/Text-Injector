void luaV_execute(lua_State *L, CallInfo *ci)
{
    LClosure *cl;
    TValue *k;
    StkId base;
    const Instruction *pc;
    int trap;
    tailcall trap = L->hookmask;
    cl = clLvalue(s2v(ci->func));
    k = cl->p->k;
    pc = ci->u.l.savedpc;
    if (trap) {
	if (cl->p->is_vararg) {
	    trap = 0;
	}
	if (pc == cl->p->code) {
	    luaD_hookcall(L, ci);
	}
	ci->u.l.trap = 1;
    }
    base = ci->func + 1;
    for (;;) {
	Instruction i;
	StkId ra;
	vmfetch();
	lua_assert(base == ci->func + 1);
	lua_assert(base <= L->top && L->top < L->stack + L->stacksize);
	lua_assert(isIT(i) || (cast_void(L->top = base), 1));
	vmdispatch()
	{
	    vmcase()
	    {
		setobjs2s(L, ra, RB(i));
		vmbreak;
	    }
	    vmcase()
	    {
		lua_Integer b = GETARG_sBx(i);
		setivalue(s2v(ra), b);
		vmbreak;
	    }
	    vmcase()
	    {
		int b = GETARG_sBx(i);
		setfltvalue(s2v(ra), cast_num(b));
		vmbreak;
	    }
	    vmcase()
	    {
		TValue *rb = k + GETARG_Bx(i);
		setobj2s(L, ra, rb);
		vmbreak;
	    }
	    vmcase()
	    {
		TValue *rb;
		rb = k + GETARG_Ax(*pc);
		pc++;
		setobj2s(L, ra, rb);
		vmbreak;
	    }
	    vmcase()
	    {
		setbfvalue(s2v(ra));
		vmbreak;
	    }
	    vmcase()
	    {
		setbfvalue(s2v(ra));
		pc++;
		vmbreak;
	    }
	    vmcase()
	    {
		setbtvalue(s2v(ra));
		vmbreak;
	    }
	    vmcase()
	    {
		int b = GETARG_B(i);
		{
		    setnilvalue(s2v(ra++));
		}
		b--;
		vmbreak;
	    }
	    vmcase()
	    {
		int b = GETARG_B(i);
		setobj2s(L, ra, cl->upvals[b]->v);
		vmbreak;
	    }
	    vmcase()
	    {
		UpVal *uv = cl->upvals[GETARG_B(i)];
		setobj(L, uv->v, s2v(ra));
		luaC_barrier(L, uv, s2v(ra));
		vmbreak;
	    }
	    vmcase()
	    {
		const TValue *slot;
		TValue *upval = cl->upvals[GETARG_B(i)]->v;
		TValue *rc = KC(i);
		TString *key = tsvalue(rc);
		if (luaV_fastget(L, upval, key, slot, luaH_getshortstr)) {
		    setobj2s(L, ra, slot);
		} else {
		    Protect(luaV_finishget(L, upval, rc, ra, slot));
		}
		vmbreak;
	    }
	    vmcase()
	    {
		const TValue *slot;
		TValue *rb = vRB(i);
		TValue *rc = vRC(i);
		lua_Unsigned n;
		if (ttisinteger(rc) ? (cast_void(n = ivalue(rc)),
				       luaV_fastgeti(L, rb, n, slot))
				    : luaV_fastget(L, rb, rc, slot, luaH_get)) {
		    setobj2s(L, ra, slot);
		} else {
		    Protect(luaV_finishget(L, rb, rc, ra, slot));
		}
		vmbreak;
	    }
	    vmcase()
	    {
		const TValue *slot;
		TValue *rb = vRB(i);
		int c = GETARG_C(i);
		if (luaV_fastgeti(L, rb, c, slot)) {
		    setobj2s(L, ra, slot);
		} else {
		    TValue key;
		    setivalue(&key, c);
		    Protect(luaV_finishget(L, rb, &key, ra, slot));
		}
		vmbreak;
	    }
	    vmcase()
	    {
		const TValue *slot;
		TValue *rb = vRB(i);
		TValue *rc = KC(i);
		TString *key = tsvalue(rc);
		if (luaV_fastget(L, rb, key, slot, luaH_getshortstr)) {
		    setobj2s(L, ra, slot);
		} else {
		    Protect(luaV_finishget(L, rb, rc, ra, slot));
		}
		vmbreak;
	    }
	    vmcase()
	    {
		const TValue *slot;
		TValue *upval = cl->upvals[GETARG_A(i)]->v;
		TValue *rb = KB(i);
		TValue *rc = RKC(i);
		TString *key = tsvalue(rb);
		if (luaV_fastget(L, upval, key, slot, luaH_getshortstr)) {
		    luaV_finishfastset(L, upval, slot, rc);
		} else {
		    Protect(luaV_finishset(L, upval, rb, rc, slot));
		}
		vmbreak;
	    }
	    vmcase()
	    {
		const TValue *slot;
		TValue *rb = vRB(i);
		TValue *rc = RKC(i);
		lua_Unsigned n;
		if (ttisinteger(rb)
			? (cast_void(n = ivalue(rb)),
			   luaV_fastgeti(L, s2v(ra), n, slot))
			: luaV_fastget(L, s2v(ra), rb, slot, luaH_get)) {
		    luaV_finishfastset(L, s2v(ra), slot, rc);
		} else {
		    Protect(luaV_finishset(L, s2v(ra), rb, rc, slot));
		}
		vmbreak;
	    }
	    vmcase()
	    {
		const TValue *slot;
		int c = GETARG_B(i);
		TValue *rc = RKC(i);
		if (luaV_fastgeti(L, s2v(ra), c, slot)) {
		    luaV_finishfastset(L, s2v(ra), slot, rc);
		} else {
		    TValue key;
		    setivalue(&key, c);
		    Protect(luaV_finishset(L, s2v(ra), &key, rc, slot));
		}
		vmbreak;
	    }
	    vmcase()
	    {
		const TValue *slot;
		TValue *rb = KB(i);
		TValue *rc = RKC(i);
		TString *key = tsvalue(rb);
		if (luaV_fastget(L, s2v(ra), key, slot, luaH_getshortstr)) {
		    luaV_finishfastset(L, s2v(ra), slot, rc);
		} else {
		    Protect(luaV_finishset(L, s2v(ra), rb, rc, slot));
		}
		vmbreak;
	    }
	    vmcase()
	    {
		int b = GETARG_B(i);
		int c = GETARG_C(i);
		Table *t;
		if (b > 0) {
		    b = 1 << (b - 1);
		}
		lua_assert((!TESTARG_k(i)) == (GETARG_Ax(*pc) == 0));
		if (TESTARG_k(i)) {
		    c += GETARG_Ax(*pc) * (MAXARG_C + 1);
		}
		pc++;
		L->top = ra + 1;
		t = luaH_new(L);
		sethvalue2s(L, ra, t);
		if (b != 0 || c != 0) {
		    luaH_resize(L, t, c, b);
		}
		checkGC(L, ra + 1);
		vmbreak;
	    }
	    vmcase()
	    {
		const TValue *slot;
		TValue *rb = vRB(i);
		TValue *rc = RKC(i);
		TString *key = tsvalue(rc);
		setobj2s(L, ra + 1, rb);
		if (luaV_fastget(L, rb, key, slot, luaH_getstr)) {
		    setobj2s(L, ra, slot);
		} else {
		    Protect(luaV_finishget(L, rb, rc, ra, slot));
		}
		vmbreak;
	    }
	    vmcase()
	    {
		op_arithI(L, l_addi, luai_numadd);
		vmbreak;
	    }
	    vmcase()
	    {
		op_arithK(L, l_addi, luai_numadd);
		vmbreak;
	    }
	    vmcase()
	    {
		op_arithK(L, l_subi, luai_numsub);
		vmbreak;
	    }
	    vmcase()
	    {
		op_arithK(L, l_muli, luai_nummul);
		vmbreak;
	    }
	    vmcase()
	    {
		op_arithK(L, luaV_mod, luaV_modf);
		vmbreak;
	    }
	    vmcase()
	    {
		op_arithfK(L, luai_numpow);
		vmbreak;
	    }
	    vmcase()
	    {
		op_arithfK(L, luai_numdiv);
		vmbreak;
	    }
	    vmcase()
	    {
		op_arithK(L, luaV_idiv, luai_numidiv);
		vmbreak;
	    }
	    vmcase()
	    {
		op_bitwiseK(L, l_band);
		vmbreak;
	    }
	    vmcase()
	    {
		op_bitwiseK(L, l_bor);
		vmbreak;
	    }
	    vmcase()
	    {
		op_bitwiseK(L, l_bxor);
		vmbreak;
	    }
	    vmcase()
	    {
		TValue *rb = vRB(i);
		int ic = GETARG_sC(i);
		lua_Integer ib;
		if (tointegerns(rb, &ib)) {
		    pc++;
		    setivalue(s2v(ra), luaV_shiftl(ib, -ic));
		}
		vmbreak;
	    }
	    vmcase()
	    {
		TValue *rb = vRB(i);
		int ic = GETARG_sC(i);
		lua_Integer ib;
		if (tointegerns(rb, &ib)) {
		    pc++;
		    setivalue(s2v(ra), luaV_shiftl(ic, ib));
		}
		vmbreak;
	    }
	    vmcase()
	    {
		op_arith(L, l_addi, luai_numadd);
		vmbreak;
	    }
	    vmcase()
	    {
		op_arith(L, l_subi, luai_numsub);
		vmbreak;
	    }
	    vmcase()
	    {
		op_arith(L, l_muli, luai_nummul);
		vmbreak;
	    }
	    vmcase()
	    {
		op_arith(L, luaV_mod, luaV_modf);
		vmbreak;
	    }
	    vmcase()
	    {
		op_arithf(L, luai_numpow);
		vmbreak;
	    }
	    vmcase()
	    {
		op_arithf(L, luai_numdiv);
		vmbreak;
	    }
	    vmcase()
	    {
		op_arith(L, luaV_idiv, luai_numidiv);
		vmbreak;
	    }
	    vmcase()
	    {
		op_bitwise(L, l_band);
		vmbreak;
	    }
	    vmcase()
	    {
		op_bitwise(L, l_bor);
		vmbreak;
	    }
	    vmcase()
	    {
		op_bitwise(L, l_bxor);
		vmbreak;
	    }
	    vmcase()
	    {
		op_bitwise(L, luaV_shiftr);
		vmbreak;
	    }
	    vmcase()
	    {
		op_bitwise(L, luaV_shiftl);
		vmbreak;
	    }
	    vmcase()
	    {
		Instruction pi = *(pc - 2);
		TValue *rb = vRB(i);
		TMS tm = (TMS)GETARG_C(i);
		StkId result = RA(pi);
		lua_assert(OP_ADD <= GET_OPCODE(pi) &&
			   GET_OPCODE(pi) <= OP_SHR);
		Protect(luaT_trybinTM(L, s2v(ra), rb, result, tm));
		vmbreak;
	    }
	    vmcase()
	    {
		Instruction pi = *(pc - 2);
		int imm = GETARG_sB(i);
		TMS tm = (TMS)GETARG_C(i);
		int flip = GETARG_k(i);
		StkId result = RA(pi);
		Protect(luaT_trybiniTM(L, s2v(ra), imm, flip, result, tm));
		vmbreak;
	    }
	    vmcase()
	    {
		Instruction pi = *(pc - 2);
		TValue *imm = KB(i);
		TMS tm = (TMS)GETARG_C(i);
		int flip = GETARG_k(i);
		StkId result = RA(pi);
		Protect(luaT_trybinassocTM(L, s2v(ra), imm, flip, result, tm));
		vmbreak;
	    }
	    vmcase()
	    {
		TValue *rb = vRB(i);
		lua_Number nb;
		if (ttisinteger(rb)) {
		    lua_Integer ib = ivalue(rb);
		    setivalue(s2v(ra), intop(-, 0, ib));
		}
		if (tonumberns(rb, nb)) {
		    setfltvalue(s2v(ra), luai_numunm(L, nb));
		} else {
		    Protect(luaT_trybinTM(L, rb, rb, ra, TM_UNM));
		}
		vmbreak;
	    }
	    vmcase()
	    {
		TValue *rb = vRB(i);
		lua_Integer ib;
		if (tointegerns(rb, &ib)) {
		    setivalue(s2v(ra), intop(^, ~l_castS2U(0), ib));
		} else {
		    Protect(luaT_trybinTM(L, rb, rb, ra, TM_BNOT));
		}
		vmbreak;
	    }
	    vmcase()
	    {
		TValue *rb = vRB(i);
		if (l_isfalse(rb)) {
		    setbtvalue(s2v(ra));
		} else {
		    setbfvalue(s2v(ra));
		}
		vmbreak;
	    }
	    vmcase()
	    {
		Protect(luaV_objlen(L, ra, vRB(i)));
		vmbreak;
	    }
	    vmcase()
	    {
		int n = GETARG_B(i);
		L->top = ra + n;
		ProtectNT(luaV_concat(L, n));
		checkGC(L, L->top);
		vmbreak;
	    }
	    vmcase()
	    {
		Protect(luaF_close(L, ra, LUA_OK));
		vmbreak;
	    }
	    vmcase()
	    {
		halfProtect(luaF_newtbcupval(L, ra));
		vmbreak;
	    }
	    vmcase()
	    {
		dojump(ci, i, 0);
		vmbreak;
	    }
	    vmcase()
	    {
		int cond;
		TValue *rb = vRB(i);
		Protect(cond = luaV_equalobj(L, s2v(ra), rb));
		docondjump();
		vmbreak;
	    }
	    vmcase()
	    {
		op_order(L, l_lti, LTnum, lessthanothers);
		vmbreak;
	    }
	    vmcase()
	    {
		op_order(L, l_lei, LEnum, lessequalothers);
		vmbreak;
	    }
	    vmcase()
	    {
		TValue *rb = KB(i);
		int cond = luaV_rawequalobj(s2v(ra), rb);
		docondjump();
		vmbreak;
	    }
	    vmcase()
	    {
		int cond;
		int im = GETARG_sB(i);
		if (ttisinteger(s2v(ra))) {
		    cond = (ivalue(s2v(ra)) == im);
		}
		if (ttisfloat(s2v(ra))) {
		    cond = luai_numeq(fltvalue(s2v(ra)), cast_num(im));
		} else {
		    cond = 0;
		}
		docondjump();
		vmbreak;
	    }
	    vmcase()
	    {
		op_orderI(L, l_lti, luai_numlt, 0, TM_LT);
		vmbreak;
	    }
	    vmcase()
	    {
		op_orderI(L, l_lei, luai_numle, 0, TM_LE);
		vmbreak;
	    }
	    vmcase()
	    {
		op_orderI(L, l_gti, luai_numgt, 1, TM_LT);
		vmbreak;
	    }
	    vmcase()
	    {
		op_orderI(L, l_gei, luai_numge, 1, TM_LE);
		vmbreak;
	    }
	    vmcase()
	    {
		int cond = !l_isfalse(s2v(ra));
		docondjump();
		vmbreak;
	    }
	    vmcase()
	    {
		TValue *rb = vRB(i);
		if (l_isfalse(rb) == GETARG_k(i)) {
		    pc++;
		} else {
		    setobj2s(L, ra, rb);
		    donextjump(ci);
		}
		vmbreak;
	    }
	    vmcase()
	    {
		int b = GETARG_B(i);
		int nresults = GETARG_C(i) - 1;
		if (b != 0) {
		    L->top = ra + b;
		}
		ProtectNT(luaD_call(L, ra, nresults));
		vmbreak;
	    }
	    vmcase()
	    {
		int b = GETARG_B(i);
		int nparams1 = GETARG_C(i);
		int delta = (nparams1) ? ci->u.l.nextraargs + nparams1 : 0;
		if (b != 0) {
		    L->top = ra + b;
		} else {
		    b = cast_int(L->top - ra);
		}
		savepc(ci);
		if (TESTARG_k(i)) {
		    luaF_close(L, base, NOCLOSINGMETH);
		    lua_assert(base == ci->func + 1);
		}
		while (!ttisfunction(s2v(ra))) {
		    luaD_tryfuncTM(L, ra);
		    b++;
		    checkstackGCp(L, 1, ra);
		}
		if (!ttisLclosure(s2v(ra))) {
		    luaD_call(L, ra, LUA_MULTRET);
		    updatetrap(ci);
		    updatestack(ci);
		    ci->func -= delta;
		    luaD_poscall(L, ci, cast_int(L->top - ra));
		    return;
		}
		ci->func -= delta;
		luaD_pretailcall(L, ci, ra, b);
		tailcall
	    }
	    vmcase()
	    {
		int n = GETARG_B(i) - 1;
		int nparams1 = GETARG_C(i);
		if (n < 0) {
		    n = cast_int(L->top - ra);
		}
		savepc(ci);
		if (TESTARG_k(i)) {
		    if (L->top < ci->top) {
			L->top = ci->top;
		    }
		    luaF_close(L, base, LUA_OK);
		    updatetrap(ci);
		    updatestack(ci);
		}
		if (nparams1) {
		    ci->func -= ci->u.l.nextraargs + nparams1;
		}
		L->top = ra + n;
		luaD_poscall(L, ci, n);
		return;
	    }
	    vmcase()
	    {
		if (L->hookmask) {
		    L->top = ra;
		    halfProtectNT(luaD_poscall(L, ci, 0));
		} else {
		    int nres = ci->nresults;
		    L->ci = ci->previous;
		    L->top = base - 1;
		    while (nres-- > 0) {
			setnilvalue(s2v(L->top++));
		    }
		}
		return;
	    }
	    vmcase()
	    {
		if (L->hookmask) {
		    L->top = ra + 1;
		    halfProtectNT(luaD_poscall(L, ci, 1));
		} else {
		    int nres = ci->nresults;
		    L->ci = ci->previous;
		    if (nres == 0) {
			L->top = base - 1;
		    } else {
			setobjs2s(L, base - 1, ra);
			L->top = base;
			while (--nres > 0) {
			    setnilvalue(s2v(L->top++));
			}
		    }
		}
		return;
	    }
	    vmcase()
	    {
		if (ttisinteger(s2v(ra + 2))) {
		    lua_Unsigned count = l_castS2U(ivalue(s2v(ra + 1)));
		    if (count > 0) {
			lua_Integer step = ivalue(s2v(ra + 2));
			lua_Integer idx = ivalue(s2v(ra));
			chgivalue(s2v(ra + 1), count - 1);
			idx = intop(+, idx, step);
			chgivalue(s2v(ra), idx);
			setivalue(s2v(ra + 3), idx);
			pc -= GETARG_Bx(i);
		    }
		}
		if (floatforloop(ra)) {
		    pc -= GETARG_Bx(i);
		}
		updatetrap(ci);
		vmbreak;
	    }
	    vmcase()
	    {
		savestate(L, ci);
		if (forprep(L, ra)) {
		    pc += GETARG_Bx(i) + 1;
		}
		vmbreak;
	    }
	    vmcase()
	    {
		halfProtect(luaF_newtbcupval(L, ra + 3));
		pc += GETARG_Bx(i);
		i = *(pc++);
		lua_assert(GET_OPCODE(i) == OP_TFORCALL && ra == RA(i));
		l_tforcall
	    }
	    vmcase()
	    {
		l_tforcall memcpy(ra + 4, ra, 3 * sizeof(*ra));
		L->top = ra + 4 + 3;
		ProtectNT(luaD_call(L, ra + 4, GETARG_C(i)));
		updatestack(ci);
		i = *(pc++);
		lua_assert(GET_OPCODE(i) == OP_TFORLOOP && ra == RA(i));
		l_tforloop
	    }
	    vmcase()
	    {
		l_tforloop if (!ttisnil(s2v(ra + 4)))
		{
		    setobjs2s(L, ra + 2, ra + 4);
		    pc -= GETARG_Bx(i);
		}
		vmbreak;
	    }
	    vmcase()
	    {
		int n = GETARG_B(i);
		unsigned int last = GETARG_C(i);
		Table *h = hvalue(s2v(ra));
		if (n == 0) {
		    n = cast_int(L->top - ra) - 1;
		} else {
		    L->top = ci->top;
		}
		last += n;
		if (TESTARG_k(i)) {
		    last += GETARG_Ax(*pc) * (MAXARG_C + 1);
		    pc++;
		}
		if (last > luaH_realasize(h)) {
		    luaH_resizearray(L, h, last);
		}
		for (; n > 0; n--) {
		    TValue *val = s2v(ra + n);
		    setobj2t(L, &h->array[last - 1], val);
		    last--;
		    luaC_barrierback(L, obj2gco(h), val);
		}
		vmbreak;
	    }
	    vmcase()
	    {
		Proto *p = cl->p->p[GETARG_Bx(i)];
		halfProtect(pushclosure(L, p, cl->upvals, base, ra));
		checkGC(L, ra + 1);
		vmbreak;
	    }
	    vmcase()
	    {
		int n = GETARG_C(i) - 1;
		Protect(luaT_getvarargs(L, ci, ra, n));
		vmbreak;
	    }
	    vmcase()
	    {
		luaT_adjustvarargs(L, GETARG_A(i), ci, cl->p);
		updatetrap(ci);
		if (trap) {
		    luaD_hookcall(L, ci);
		    L->oldpc = pc + 1;
		}
		updatebase(ci);
		vmbreak;
	    }
	    vmcase()
	    {
		lua_assert(0);
		vmbreak;
	    }
	}
    }
}