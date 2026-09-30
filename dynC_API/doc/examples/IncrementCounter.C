BPatch_arithExpr addOne(BPatch_assign, *intCounter,
      BPatch_arithExpr(BPatch_plus, *intCounter, BPatch_constExpr(1)));
