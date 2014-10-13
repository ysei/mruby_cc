    CASE(OP_RESCUE) { // TODO
      /* A      R(A) := exc; clear(exc) */
      SET_OBJ_VALUE(regs[GETARG_A(i)], mrb->exc);
      mrb->exc = 0;
      NEXT;
    }

  CASE(OP_SEND) {
      int a = GETARG_A(i);
      int n = GETARG_C(i);
      mrb_value ret;

      ret = mrbb_send_r(mrb, syms[GETARG_B(i)], n, &regs, a, 0);
      if (mrb->c->ci->proc == (struct RProc *) -1) {
        //cipush(mrb);
        return ret;
      }
      regs[a] = ret;
      mrb->arena_idx = ai; // TODO do we need (because of break;)?
      NEXT;
  }
  CASE(OP_SENDB) {
      int a = GETARG_A(i);
      int n = GETARG_C(i);
      mrb_value ret;

      ret = mrbb_send_r(mrb, syms[GETARG_B(i)], n, &regs, a, 1);
      if (mrb->c->ci->proc == (struct RProc *) -1) {
        //cipush(mrb);
        return ret;
      }
      regs[a] = ret;
      mrb->arena_idx = ai; // TODO probably can remove
      NEXT;
  }


    CASE(OP_STOP) {
      /*        stop VM */
      if (mrb->exc) {
        printf("OP_STOP reached with Exception:\n");
        mrb_p(mrb, mrb_obj_value(mrb->exc));
      }
/*    L_STOP:
      {
  int n = mrb->c->ci->eidx;

  while (n--) {
    ecall(mrb, n);
  }
      }
      mrb->jmp = prev_jmp;
      if (mrb->exc) {
  return mrb_obj_value(mrb->exc);
      }
      return regs[irep->nlocals];*/
    }

    CASE(OP_RETURN) {
      {
        mrb_callinfo *ci = mrb->c->ci;
        int eidx = mrb->c->ci->eidx;
        int ridx = mrb->c->ci->ridx;
        mrb_value v = regs[GETARG_A(i)];
        struct RProc *proc = mrb->c->ci->proc;

        if (mrb->exc) {
          mrbb_raise(mrb);
        }

        switch (GETARG_B(i)) {
        case OP_R_RETURN:
          if (proc->env || !MRB_PROC_STRICT_P(proc)) {
            struct REnv *e = top_env(mrb, proc);

            if (e->cioff < 0) {
              localjump_error(mrb, LOCALJUMP_ERROR_RETURN);
              goto L_RAISE;
            }
            mrb->c->ci = mrb->c->cibase + e->cioff;
            if (ci == mrb->c->cibase) {
              localjump_error(mrb, LOCALJUMP_ERROR_RETURN);
              goto L_RAISE;
            }
            break;
          }
        case OP_R_NORMAL:
          if (ci == mrb->c->cibase) {
            localjump_error(mrb, LOCALJUMP_ERROR_RETURN);
            goto L_RAISE;
          }
          break;
        case OP_R_BREAK:
          if (proc->env->cioff < 0) {
            localjump_error(mrb, LOCALJUMP_ERROR_BREAK);
            goto L_RAISE;
          }
          mrb->c->ci = mrb->c->cibase + proc->env->cioff + 1;
          break;
        default:
          /* cannot happen */
          break;
        }

        while (eidx > mrb->c->ci[-1].eidx) {
          mrbb_ecall(mrb, mrb->c->ensure[--eidx]);
        }
        while (ridx > mrb->c->ci[-1].ridx) { // just in case?
          mrbb_rescue_pop(mrb);
        }
        /*
        if (acc < 0) {
          mrb->jmp = prev_jmp;
          return v;
        }
        */
        return v;
      }
    }
