##########################
TPC IR Optimization Passes
##########################

This page describes the LLVM IR passes the TPC backend adds to the optimization
pipeline: where each one runs, what it does, and how to work on it. Machine
(MIR) passes in ``TPCPassConfig`` are not covered here.

*****************
Where passes live
*****************

* ``llvm/lib/Target/TPC/`` - target passes. Declared in ``TPCTargetMachine.h``
  and scheduled by ``TPCTargetMachine`` (see below).
* ``llvm/lib/Transforms/Scalar/`` and ``llvm/lib/Transforms/IPO/`` - TPC passes
  that live in the generic tree. Scheduled directly by
  ``PassBuilderPipelines.cpp`` when the target triple is ``tpc``.

Every pass has a new pass manager entry point (``...Pass``). The legacy entry
points (``create...()``) were removed, except for ``TPCAA``'s wrapper, which code
generation adds.

********************
Pipeline integration
********************

New pass manager
================

``TPCTargetMachine::registerPassBuilderCallbacks`` registers the target passes
at these extension points:

.. list-table::
   :header-rows: 1

   * - Extension point
     - Levels
     - Passes, in order
   * - ``PipelineStartEP``
     - all
     - ``TPCModuleLegalizerPass`` and ``TpcCopyElisionPass`` (unless compiling
       for a library), ``TPCMarkupDirectMMIOAccessPass``, ``LinkTPCHeadersPass``
   * - ``PipelineStartEP``
     - O1+
     - ``SimplifyCFGPass``, ``TPCTransformIntrinPass``, ``SROA``,
       ``TPCScalarToIRFPass``
   * - ``OptimizerEarlyEP``
     - O1+
     - ``GlobalOptPass``, ``ADCEPass``, ``SROA``,
       ``TPCEliminateRedundantLinearityPass``
   * - ``OptimizerLastEP``
     - O0
     - ``PromotePass``, ``AttributeAdjusterPass``, ``InstCombinePass``
       (2 iterations), then ``GlobalizerPass``, ``ADCEPass``,
       ``GlobalResolverPass`` (unless compiling for a library)
   * - ``OptimizerLastEP``
     - O1+
     - ``TpcLoopOptPass`` (loop), ``DCEPass``, then ``GlobalizerPass``,
       ``GlobalResolverPass`` (unless compiling for a library)

``TPCAA`` is added to the default alias analysis pipeline by
``TPCTargetMachine::registerDefaultAliasAnalyses``.

For ``tpc``, ``OptimizerLastEP`` runs after the final ``GlobalDCE`` and
``ConstantMerge`` of the module optimization pipeline, not before them as for
other targets. This keeps the legacy order: ``GlobalResolver`` assigns
local-memory addresses and leaves behind external declarations that debug info
refers to, which ``GlobalDCE`` would otherwise delete.

For the ``tpc`` triple, ``PassBuilderPipelines.cpp`` also changes the default
pipeline (look for ``isTPCTarget`` in that file):

* Only ``always_inline`` functions are inlined: the module simplification
  pipeline runs ``AlwaysInlinerPass``, ``PostOrderFunctionAttrsPass``,
  ``EvalSpecialFunctionPass``, then the function simplification pipeline,
  instead of the regular inliner pipeline.
* Function simplification: no loop rotation at ``-O1``; only trivial loop
  unswitching at every level; ``CoordUpdateSimplifyPass`` (default on,
  ``-enable-simplify-coord-update-pass``) and ``LoopSimplifyCFGPass`` after
  loop interchange; ``EliminateSwizzleCastPass`` (default off,
  ``-enable-cast-swizzle-opt``) after the loop passes; ``MergedLoadStoreMotion``
  and ``GVN`` also at ``-O1``.
* Module optimization: no loop rotation at ``-O1``; ``TPCLICMPass`` after loop
  unrolling; ``LoopSWPPass`` (default on, ``-enable-loop-swp-pass``) after the
  vectorization passes; at ``-O2`` and above, ``IterationInterleavePass``
  (default off, ``-enable-iter-ilv``) at the end of the function optimization
  pipeline.
* With ``-enable-unroll-and-jam``, ``LoopTakenPass``, another round of function
  simplification and ``LoopUnrollAndJamPass`` run right after loop rotation,
  instead of next to loop unrolling.

Legacy pass manager
===================

The legacy pipeline is gone: clang stopped using it in LLVM 15, and LLVM 16
removed ``TargetMachine::adjustPassManager``, through which TPC scheduled its
passes. The legacy entry points of the passes are gone too. Where a section below
mentions a legacy pass, a legacy placement or ``PassManagerBuilder.cpp``, it
records how the pass behaved in that pipeline, which explains why the new pipeline
is shaped this way.

Testing a pass
==============

Run a single pass with ``opt -mtriple=tpc -passes=<name>``; the names are
listed with each pass below. A pass created by name uses the same defaults as the legacy
pass created by name (for example ``scalar2irf`` folds comparisons, and
``tpc-eliminate-redundant-linearity`` assumes no tensor pack/unpack features),
not the subtarget-derived settings the pipeline passes in. ``tpc/difftest/run.py`` compares whole-kernel results against Habana's compiler.

******
Passes
******

In pipeline order. Each entry gives both pass manager entry points and the
name to use with ``opt -passes=``.

TPCModuleLegalizer
==================

:Source: ``llvm/lib/Target/TPC/TPCModuleLegalizer.cpp``
:Kind: module pass
:New PM: ``TPCModuleLegalizerPass`` (``tpclegal``)

**Purpose.** Prepares a freshly linked TPC module (the file comment says it runs
"after IR libraries have loaded"). It identifies the single kernel entry point,
forces every other defined function to be inlined into it, and deletes every
function that is not reachable from the kernel through direct calls. The code
requires exactly one kernel per module. The file does not say why non-kernel
functions must be inlined.

**How it works.**

* For every defined function: if it has the ``"tpc-kernel"`` attribute it is the
  kernel (a second one is a fatal error). Otherwise the pass removes
  ``noinline``, adds ``alwaysinline``, sets ``available_externally`` linkage and
  drops its comdat.

* If no function has ``"tpc-kernel"``, it guesses the kernel. The candidates
  are functions that had ``noinline`` or lacked ``alwaysinline`` before the
  step above. If there is exactly one candidate, it gets ``"tpc-kernel"``,
  loses ``alwaysinline`` and gets ``external`` linkage. Otherwise the pass
  reports the fatal error "Kernel functions was not found in the module".

* It then walks direct ``CallInst`` callees (``getCalledFunction()``)
  transitively from the kernel to build the set of used functions. Every
  other function in the module is unused, declarations included.

* Unused functions are erased in dependency order, callers before callees.
  If unused functions call each other in a cycle, the pass reports the fatal
  error "Recursive call found".

**Requirements and effects.** The module must contain exactly one kernel, or
one function that can be guessed as the kernel. Uses no analyses. In the legacy
PM, changes made in ``doInitialization`` never trigger analysis invalidation.
The new PM version returns ``PreservedAnalyses::none()`` when it changed
attributes or linkage, or erased any function, and ``all()`` otherwise. The
shared ``runImpl()`` does not count erased functions as a change, so the
new PM wrapper also compares ``M.size()`` before and after. It is a required
pass (``isRequired()``) because the legacy pass never skips, and
``TPCTargetMachine.cpp`` says it "must be executed at all optimization levels".

**Options.** None.

**Maintainer notes.**

* In the new PM it must be scheduled as a module pass before the function
  pipeline. In the legacy PM it is added at ``EP_EarlyAsPossible`` (only when
  ``!Options.CompileForLibrary``), together with ``TpcCopyElision``.

* A function referenced other than through a direct call from reachable code
  (address taken, called through a bitcast, used by an ``invoke``) is treated
  as unused and erased, even though it still has uses.

* ``printRecursiveCall()`` is an empty stub, so the "Recursive call found"
  error names no functions. ``findFunction()`` is unused (compiler warning).

* ``Kernel`` and ``UsedFunctions`` are fresh on each run.

TpcCopyElision
==============

:Source: ``llvm/lib/Target/TPC/TPCCopyElision.cpp``
:Kind: function pass
:New PM: ``TpcCopyElisionPass`` (``copyelide``)

**Purpose.** Removes the extra struct copies that the frontend generates for
intrinsics returning structure values and for structure-typed arguments. These
are an ``alloca`` temporary plus a ``memcpy`` (see the examples in the file
header). The file comment says it does this "in hope that it can help code
generation".

**How it works.** For every ``llvm.memcpy`` in each block:

* ``isOnlyCopy``: the size is a ``ConstantInt`` and both pointers have the same
  type. One of them must be an ``alloca`` of a structure type whose allocation
  size equals the copy size. The copy is skipped if the destination is a GEP.

* Return temporary (``isReturnTempValue``): the source is an ``alloca``. Its
  only users are ``lifetime.start``/``end``, exactly one ``memcpy`` (as its
  source) and exactly one ``store`` into it. The pass replaces all uses of the
  alloca with the memcpy destination, so the store writes straight to the
  destination. It then erases the alloca, the lifetime markers and the
  ``memcpy``.

* Argument temporary (``isArgumentTempValue``): the destination is an
  ``alloca``. Its only users are exactly one ``memcpy`` (as its destination) and
  ``load`` instructions. The pass replaces all uses of the alloca with the
  memcpy source, then erases the ``memcpy`` and the alloca.

* Dead instructions are collected per block and erased after the block is
  scanned.

**Requirements and effects.** Expects frontend-shaped IR with opaque pointers; it
does not match typed-pointer IR, whose copies go through bitcasts. Uses no
analyses. The legacy
pass declares no preserved analyses, so the new PM version returns ``none()``
on change and ``all()`` otherwise. Optional: the legacy pass calls
``skipFunction``.

**Options.** ``-tpc-copy-elision`` (bool, default ``true``, hidden): when false,
the pass does nothing.

**Maintainer notes.**

* No memory-dependence or aliasing checks are done. The store is redirected
  to the destination at the store's position, not the memcpy's, and reads of
  the temporary become reads of the source. Correctness relies on the
  frontend's patterns.

MarkupDirectMMIOAccessPass
==========================

:Source: ``llvm/lib/Target/TPC/TPCMarkupDirectMMIOAccess.cpp``
:Kind: function pass
:New PM: ``TPCMarkupDirectMMIOAccessPass`` (``tpc-markup-direct-mmio-access``)

**Purpose.** Marks functions that access MMIO directly so the backend can
record it. ``TPCElfSpecSet`` (``TPCElfSet.cpp:361``) reads the marker into
``TPCMetadata.directMMIOAccess``.

**How it works.**

* It looks for any call to ``llvm.tpc.st.l`` whose operand 2, or
  ``llvm.tpc.ld.l`` whose operand 1, is a ``ConstantInt`` with the
  ``TPCII::SW_MMIO`` bit set.

* If it finds one, it attaches function metadata
  ``!tpc.direct.mmio.access !{!"direct mmio access"}``.

**Requirements and effects.** Uses no analyses. Only adds function metadata.
The legacy pass declares no preserved analyses, so the new PM version returns
``none()`` when it added the metadata and ``all()`` otherwise. Optional: the
legacy pass calls ``skipFunction``.

**Options.** None.

**Maintainer notes.**

* The file's top comment says "Add metadata to store / load instructions with
  MMIO", but the code attaches the metadata to the function.

* Non-constant switch operands are not treated as MMIO.
* Added at ``EP_EarlyAsPossible`` at all optimization levels in the legacy
  pipeline.

* The new PM class name follows the ``<Legacy>Pass`` rule, hence the doubled
  ``PassPass``.

TPCTransformIntrin
==================

:Source: ``llvm/lib/Target/TPC/TPCTransformIntrin.cpp``
:Kind: function pass
:New PM: ``TPCTransformIntrinPass`` (``tpc-trans-intr``)

**Purpose.** Replaces TPC arithmetic intrinsic calls in their simplest form
with equivalent generic LLVM IR (for example ``add(a, b)`` becomes ``a + b``).
It only does this when the call has a constant true predicate, a constant
false polarity and no switches. The file does not state the motivation.
Presumably this exposes the operations to generic optimizations, but that is
not stated in the code.

**How it works.** Every ``IntrinsicInst`` is examined and, when it matches,
replaced (RAUW and erase):

* Masked int5 ops ``tpc_{add,sub,and,or,xor}_mask`` (8 call arguments):

  * Only when ``-tpc-trans-intr-int5`` is set (off by default).
  * Operand 0 must be ``<5 x i32>``, data type ``2`` and switches ``0``.
  * Each set bit of the constant dimmask becomes
    ``extractelement``/op/``insertelement``. A zero-splat
    ``shufflevector`` first operand is swapped and turned into a scalar.

* Binary ops ``tpc_{add,sub,mul,and,or,xor,min,max,shl,shr}`` and
  ``tpc_cmp_{eq,neq,less,leq,grt,geq}`` (7 call arguments). The switches must
  be ``0``. Operand types must match, except for shifts.

  * FP (``-tpc-trans-intr-float``, data type 0 = f32 or 1 = bf16):
    ``fadd``/``fsub``/``fmul``. ``fmul`` only when the income type equals
    the operand type.
  * FP min/max: vector f32 uses ``minnum``/``maxnum``, scalars use
    ``select`` + ``fcmp``.
  * FP compares: scalar ``fcmp`` only.
  * FP shifts: done through a bitcast to an integer type.
  * Integer vectors with at least 32 elements: ``add``, ``sub``, ``and``,
    ``or``, ``xor``, ``shl``, ``lshr``. ``mul``, min/max and compares are not
    transformed.
  * Integer scalars: data types 2/3 (i32/u32, signed ops get ``nsw``),
    7/8 (i16/u16) and 4/5 (i8/u8). The 8/16-bit types are widened to i32 and
    truncated back. ``mul`` only when the income type is i32.
  * Signed or unsigned ``icmp``/``select`` are used for compares and
    min/max.

* Unary ops ``tpc_popcnt``, ``tpc_not``, ``tpc_abs`` (6 call arguments):

  * ``popcnt``: ``llvm.ctpop`` + ``trunc``, only when counting ones and the
    result is i8 or ``<256 x i8>``.
  * ``not``: ``xor -1``, with bitcasts for f32/bf16.
  * ``abs``: ``llvm.fabs`` for f32. Scalar bf16 uses ``and 0x7fff``. Scalar
    integers use ``select(x < 0, 0 - x, x)``. bf16 vectors and integer
    vectors are not transformed.

**Requirements and effects.** Uses no analyses. Reports a change only when it
replaced at least one intrinsic. The new PM version returns ``none()`` in that
case (the legacy pass declares no preserved analyses). Optional: the legacy
pass calls ``skipFunction``. The whole file is compiled only with
``LLVM_TPC_COMPILER``.

**Options.** All bool and hidden:

* ``-tpc-trans-intr`` (default ``true``): enables the pass.
* ``-tpc-trans-intr-int5`` (default ``false``): int5 masked ops.
* ``-tpc-trans-intr-float`` (default ``true``): FP operands.
* ``-tpc-trans-intr-cmp`` (default ``true``): compares.
* ``-tpc-trans-intr-minmax`` (default ``true``): min/max.

**Maintainer notes.**

* The int5 masked-op lowering looks wrong for dimmasks with more than one bit
  set: each lane's ``insertelement`` starts from the original operand 0
  (``TPCTransformIntrin.cpp:248-272`` and the matching sub/or/and/xor blocks).
  So only the highest selected lane's update reaches the result. It is off
  by default.

* Some paths create instructions and then give up with ``continue``. Examples
  are the widening casts at ``:853-854`` followed by ``continue`` at ``:865``,
  and the splat ``extractelement`` at ``:223``. They leave dead instructions
  without reporting a change.

* The ``-tpc-trans-intr`` cl::opt has the same name as the legacy pass
  argument. With ``opt``'s legacy ``-<pass>`` flags this may conflict (not
  verified). The ``tpc_find_first`` branch is unreachable because its
  ``case`` label is commented out.

* Legacy placement: ``EP_EarlyAsPossible`` at ``-O1`` and above, after
  SimplifyCFG and before SROA and ``ScalarToIRF``.

ScalarToIRFPass
===============

:Source: ``llvm/lib/Target/TPC/ScalarToIRF.cpp``
:Kind: function pass
:New PM: ``TPCScalarToIRFPass(bool FoldCmp = true)`` (``scalar2irf``)

**Purpose.** Folds scalar arithmetic on single lanes of int5 (``<5 x i32>``,
IRF) values into TPC IRF masked intrinsics. These operate on the selected
dimensions in one instruction.

**How it works.** Per basic block:

* Collects ``extractelement <5 x i32> V, C`` with one use whose user is an
  eligible operation: ``add``, ``sub``, ``mul``, ``and``, ``or``, ``xor``,
  ``shl``, ``lshr``, and ``icmp`` only when ``FoldCmp``.

  * Non-compare: the operation must have one use, an ``insertelement`` of
    the same vector (same value or a load from the same pointer) at the same
    index.
  * Compare: both operands must be single-use extracts at the same index.

* Merges non-compare candidates into one work item when they have the same
  opcode, the same scalar operand, a different lane, and are chained through
  single-use ``insertelement`` values. The lane bits are ORed into a mask.

* Rewrites each work item:

  * ``icmp`` (eq/ne/slt/sle/sgt/sge) becomes ``llvm.tpc.cmp.*.mask``.
  * ``add`` becomes ``llvm.tpc.add.mask``.
  * ``sub`` of a constant (other than ``INT_MIN``) becomes ``add.mask`` of
    the negated constant.
  * Other ``sub``: two ``llvm.tpc.sub.mask`` calls, because the IRF operand
    order is reversed ("0 - (b - a)").
  * The replaced chain is erased where it has no remaining uses.

**Requirements and effects.** Uses no analyses. The legacy pass declares no
preserved analyses, so the new PM version returns ``none()`` on change and
``all()`` otherwise. Optional: the legacy pass calls ``skipFunction``.

**Options.** ``-scalar-to-irf`` (bool, default ``true``, not hidden): when
false, the pass does nothing. ``FoldCmp`` is a constructor argument. The
legacy pipeline passes ``Subtarget->hasGen3Plus()``.

**Maintainer notes.**

* ``FoldCmp`` is kept per object and defaults to ``true``.

* ``mul``/``and``/``or``/``xor``/``shl``/``lshr`` are accepted as candidates,
  but ``getIRFIntrinsic`` only returns intrinsics for ``add`` and ``sub``
  (``mul`` is commented out). So those candidates are never rewritten.

* Pre-existing warning: ``ICmp`` is set but not used.
* Legacy placement: ``EP_EarlyAsPossible`` at ``-O1`` and above, after SROA.
* The new PM class name follows the ``<Legacy>Pass`` rule, hence the doubled
  ``PassPass``.

LinkTPCHeadersPass
==================

:Source: ``llvm/lib/Transforms/IPO/LinkTPCHeaders.cpp``
:Kind: module pass
:New PM: ``LinkTPCHeadersPass`` (``link-tpc-headers``)

**Purpose.** Links precompiled LLVM IR implementations of TPC library
functions into the module, choosing them by the subarchitecture, and replaces
the matching intrinsic calls in ``main`` with calls to the linked functions.
The library covers special functions (sin, cos, exp, log, sqrt, rsqrt, tanh,
reciprocal for f32, bf16 and f16), vector reductions, swizzle, i32→i8 trunc,
MMIO accessors (dim size and stride, pad value, semaphore, CSR, convert CSR,
HW prefetch stride) and the RNG stage-1 seed. The IR comes from string
constants in ``llvm/include/llvm/Transforms/IPO/TPCHeaders/**`` or is
generated by ``LinkTPCFunctionsMMIOAccess``/``LinkTPCFunctionsRNG``. With
``-ILTO=<so>`` it can also fetch the IR for ``tpc_*`` calls from a shared
library at run time. The whole file is compiled only when ``_WIN32`` is not
defined.

**How it works.**

- Constructor: build ``KeyToInfo``, a map from (intrinsic ID, result type
  key, argument type keys, arch) to (IR text, function name). If ``-ILTO`` is
  set, ``dlopen`` the library. A relative path is resolved against the
  current working directory.
- ``run``: find ``main``; if it is missing, do nothing. Take the
  subarchitecture from ``main``'s ``"target-cpu"`` attribute; the default is
  ``"gaudi"``. Map it through ``ArchMap``: goya, gaudi, gaudib,
  goya2/greco, gaudi2, doron1. If the CPU is not in the map, do nothing.
- Expand every vector ``fdiv`` in ``main`` to
  ``fmul(n, llvm.tpc.reciprocal(d))``.
- Scan ``main``:

  - With ``-ILTO``: for each direct ``call`` to a function whose name starts
    with ``tpc_``, call ``<SUBARCH>_<name>_Func`` from the library to get the
    IR text. Link it, and queue the call for replacement. A call the library
    has no symbol for is left as it is. If the kernel's declaration does not
    match the linked definition's type, the pass stops with a fatal error.
  - Otherwise: for each intrinsic call whose key is in ``KeyToInfo``, parse
    and link that IR text once (``LinkedIRs``), and queue the call. Calls
    whose IR is already linked are queued without linking again. A link
    failure is ``report_fatal_error``.

- For each queued instruction:

  - Give the linked function ``available_externally`` linkage.
  - Insert a call to it with the same arguments, then RAUW and erase.
  - For ``llvm.vector.reduce.*`` whose result feeds
    ``insertelement`` → ``shufflevector``: replace the shufflevector with
    the call and erase the shufflevector, insertelement and reduction. The
    linked reduce functions return a vector, for example
    ``<64 x float> @v_f32_reduce_add``.

**Requirements and effects.** The pass requires a function named ``main``
and leaves every other function untouched. The value names in the linked IR
must be parseable. For this reason the TPC driver does not pass
``-discard-value-names`` (``clang/lib/Driver/ToolChains/Clang.cpp:4683-4691``).
The pass uses no analyses. Linking adds functions and globals to the module.
The linked bodies are ``alwaysinline`` with ``available_externally``
linkage, so later inlining and global cleanup are expected to remove them.
The legacy pass declares no preserved analyses. The NPM version returns
``none()`` on any change, the ``fdiv`` expansion included, and ``all()``
otherwise.

**Options.** ``-ILTO=<path>`` (``std::string``, default ``""``, hidden): path
to a shared object that supplies IR for ``tpc_*`` functions, resolved against
the current working directory when relative.

**Maintainer notes.**

- A vector ``fdiv`` expansion counts as a change even when nothing was linked.
- ``LinkedIRs``, ``ReplacementList``, ``SubArchName`` and the ``dlopen`` handle
  are created fresh on every ``run()``, so the table is rebuilt and ``-ILTO`` is
  ``dlopen``\ ed once per run.
- When a scalar ``llvm.vector.reduce.*`` result does not feed the
  insertelement→shufflevector broadcast pattern, the fallback RAUW
  (``LinkTPCHeaders.cpp:874``) replaces a scalar with the vector-returning
  linked call. That looks like a type mismatch (unclear whether this case can
  occur).
- The ``Arch`` parameter of ``expandSpecialCaseLLVMIR`` is unused. A
  ``todo`` notes the missing Windows support (``LinkTPCHeaders.cpp:270``).
- ``LinkTPCHeadersPass::ArchTy`` is also used by
  ``LinkTPCFunctionsMMIOAccess``.
- Pipeline (legacy): the pass runs near the start of
  ``populateModulePassManager`` for TPC targets on non-Windows hosts, before
  the ``-O0`` early exit (``PassManagerBuilder.cpp:723-727``). So it runs
  before ``EvalSpecialFunction``, which expands whatever this pass did not
  replace for ``<64 x float>`` and ``<128 x bfloat>``.

EvalSpecialFunctionPass
=======================

:Source: ``llvm/lib/Transforms/Scalar/EvalSpecialFunctionPass.cpp``
:Kind: module pass
:New PM: ``EvalSpecialFunctionPass`` (``eval-special-function``)

**Purpose.** Expands TPC "special functions" into inline sequences of TPC
intrinsics. It handles calls to ``llvm.sin``, ``llvm.cos``, ``llvm.exp``,
``llvm.log``, ``llvm.sqrt``, ``llvm.tpc.rsqrt``, ``llvm.tpc.tanh`` and
``llvm.tpc.reciprocal`` whose result type is exactly ``<64 x float>`` or
``<128 x bfloat>``, and it rewrites every vector ``fdiv``. The TPC backend in
this tree has no lowering for these operations: ``llvm/lib/Target/TPC`` does not
reference ``ISD::FSIN``, ``FCOS``, ``FEXP``, ``FLOG``, ``FSQRT`` or the
rsqrt/tanh/reciprocal intrinsics. They therefore have to be removed at the IR
level. This pass does not depend on the architecture. Compare
``LinkTPCHeadersPass``, which links a separate implementation for each
architecture.

**How it works.**

- If ``-eval-special-function`` is false, return immediately.
- ``expandSpecialCaseLLVMIR``: in every function, replace each vector
  ``fdiv %n, %d`` with ``fmul %n, (call @llvm.tpc.reciprocal(%d))`` and erase
  the ``fdiv``. The reciprocal is declared through ``getOrInsertFunction``.
- ``expandSpecialFunction``: walk every instruction of every function. If an
  ``IntrinsicInst`` has result type ``<64 x float>``, dispatch it to
  ``replaceSinCosWithTPCIntrinsics`` (sin is ``SinCond=0``, cos is
  ``SinCond=1``), ``replaceExp…``, ``replaceLog…``, ``replaceSqrt…``,
  ``replaceReciprocalSqrt…``, ``replaceTanh…`` or ``replaceReciprocal…``. If
  the result type is ``<128 x bfloat>``, use the matching ``replaceBF16…``
  routine. Each routine builds the replacement with an ``IRBuilder`` placed
  before the call, then calls RAUW and erases the original. The comments in
  the code show the IR that each routine emits.
- The emitted sequences are LUT-based approximations built from
  ``llvm.tpc.get.lut.entry``, ``llvm.tpc.lookup.1c/2c``, ``llvm.tpc.mac``,
  ``llvm.tpc.form.fp.num``, ``llvm.tpc.calc.fp.special``, ``llvm.tpc.fclass``,
  ``llvm.tpc.sel.*``, ``llvm.tpc.cmp.*``, ``llvm.tpc.extract.exp``, plus
  shufflevector and bitcast instructions.
- After a replacement, the scan continues from the newly inserted
  instructions (``It = ++PrevIt``).

**Requirements and effects.** The pass expects nothing in particular from the
input IR. Only the exact vector types listed above are expanded. It adds no
basic blocks and no branches. It adds TPC intrinsic declarations to the module.

The legacy pass requires ``AssumptionCacheTracker`` and
``DominatorTreeWrapperPass``, but the code never queries either one: there is
no ``getAnalysis`` call. It declares ``setPreservesCFG()`` and preserves
``GlobalsAAWrapperPass``.

The NPM version requests no analyses. When it changes the IR it returns
``CFGAnalyses`` and ``GlobalsAA`` as preserved. Otherwise it returns ``all()``.

**Options.** ``-eval-special-function`` (``bool``, default ``true``, hidden):
when false, the pass does nothing.

**Maintainer notes.**

- The pass has no ``DEBUG_TYPE``. The name ``eval-special-function`` is also
  used by the ``cl::opt`` above.
- The legacy pass does not call ``skipModule()``, so ``-opt-bisect-limit`` can
  never skip it. The NPM version is not marked ``isRequired()``, so NPM
  opt-bisect can skip it.
- Before the NPM port, the legacy class was declared in
  ``EvalSpecialFunctionPass.h`` inside an anonymous namespace. The port moved
  it into the ``.cpp`` as ``EvalSpecialFunctionImpl`` plus
  ``EvalSpecialFunctionPass``. The header now declares only the NPM
  class.
- If a vector ``fdiv`` has a type other than ``<64 x float>`` or
  ``<128 x bfloat>``, it becomes a ``llvm.tpc.reciprocal`` call that this pass
  does not expand further. What lowers it afterwards is unclear.
  ``LinkTPCHeaders`` runs earlier in the legacy pipeline. The TPC backend has
  custom FDIV lowering only for ``f32`` and ``v64f32``
  (``TPCISelLowering.cpp:489-490``).
- Some routines sit inside ``#if 1 // MERGE`` blocks
  (``EvalSpecialFunctionPass.cpp:108-978``, ``1163-2202``, ``2440-3439``).
  These are leftovers from a merge.
- Pipeline (legacy): the pass is added first in
  ``addFunctionSimplificationPasses`` for TPC targets
  (``PassManagerBuilder.cpp:423-426``). That function is called again right
  after ``LoopTaken`` when unroll-and-jam is enabled
  (``PassManagerBuilder.cpp:985-988``), so this pass can run twice.

CoordUpdateSimplify
===================

:Source: ``llvm/lib/Transforms/Scalar/CoordUpdateSimplify.cpp``
:Kind: loop pass
:New PM: ``CoordUpdateSimplifyPass`` (``coord-simplify``)

**Purpose.** In an innermost, single-block loop, the pass replaces a coordinate
(``<5 x i32>``) that is rebuilt every iteration by inserting the induction
variable into a loop-carried coord (``%x = insertelement %Base, %iv, dim``)
with an incremental ``llvm.tpc.add.mask`` that advances the coord by the
induction step. The coord is initialised once in the preheader, so the loop
body no longer rebuilds it from the induction variable. The pass description
calls the original form "costly TPC Coordinate update instructions"; the code
does not quantify the cost. The profitability checks restrict the pass to
element-wise unary-like kernels (comments in ``isSimplifyProfitable`` and
``hasProfitablePattern``).

**How it works.**

* Does nothing unless ``-coord-update-simplify`` is set. Bails out if the loop
  is not innermost, has more than one block, has no preheader or no unique exit
  block, or if the header has more than ``-csp-blocksize-max`` instructions.
* Candidates (``isSafeToSimplifyInst``, checks 1-10) are header
  ``insertelement`` instructions where:

  - operand 0 is a header PHI (``BasePHI``) with no other use, whose incoming
    value from the header is the ``insertelement`` itself;
  - operand 1 is an induction PHI (``InductionDescriptor::isInductionPHI``)
    with exactly one in-block non-PHI, non-``insertelement`` user, which is its
    header incoming value and an ``add`` with a ``ConstantInt`` operand;
  - ``BasePHI``'s preheader incoming value is not a ``Constant``;
  - the only user outside the header is at most one PHI in the exit block
    (LCSSA).

* Profitability: the header must contain exactly ``-csp-max-num-loads``
  ``ld_tnsr`` and at most ``-csp-max-num-computes`` "compute" instructions
  (``TPCOptUtils::isComputeInst``; ``ld_tnsr``, ``ld_g``, ``st_tnsr``,
  ``st_g``, ``add_mask`` and ``set_indx`` count as non-compute). If the header
  has more than ``-csp-blocksize-caution`` instructions, a candidate is dropped
  when its estimated coord live range after the rewrite exceeds
  ``-csp-live-range-tolerance`` x block size and is longer than before.
* Rewrite:

  - the step is the constant operand of the induction ``add`` of the first
    element of the candidate set (``computeAddMaskStep``);
  - preheader: a clone of the ``insertelement`` using the preheader values of
    ``BasePHI`` and of the induction PHI is inserted before the terminator (a
    new preheader PHI is created when that value is defined outside the
    preheader and the preheader has several predecessors); it becomes
    ``BasePHI``'s preheader incoming value, and the induction PHI's preheader
    incoming value is replaced by the preheader version (unless constant);
  - header: ``add.mask(%Base, Step, 1 << dim, i8 2, 0, %Base, true, false)`` is
    placed after the last in-block non-PHI use of the ``insertelement`` and
    becomes ``BasePHI``'s header incoming value; all uses of the
    ``insertelement`` are replaced by ``BasePHI`` and it is erased;
  - exit: the LCSSA PHI now takes the ``add.mask``; a correcting ``add.mask``
    with ``-Step`` is inserted before the exit terminator and the LCSSA PHI's
    other users are redirected to it.

**Requirements and effects.** Requires LoopSimplify and LCSSA form (legacy
``getLoopAnalysisUsage``); the code additionally checks for a preheader, a
unique exit block and LCSSA-shaped outside uses. Uses ScalarEvolution (through
``InductionDescriptor``) and LoopInfo. Does not change the CFG; it inserts
instructions and possibly PHIs in the preheader and the exit block and changes
header PHI incoming values. The instructions it inserts or erases
(``insertelement``, ``add.mask``, PHIs) have no memory effects. Legacy: preserves
ScalarEvolution and the ``getLoopAnalysisUsage`` set. New PM: returns
``getLoopPassPreservedAnalyses()`` when it changed the loop and
``PreservedAnalyses::all()`` otherwise.

**Options.**

* ``-coord-update-simplify`` (bool, default ``false``): master switch; when
  false the pass does nothing.
* ``-csp-blocksize-caution`` (unsigned, default 100): above this header size
  the live-range check is applied.
* ``-csp-blocksize-max`` (unsigned, default 200): above this header size the
  loop is skipped.
* ``-csp-max-num-loads`` (unsigned, default 1): required number of ``ld_tnsr``
  in the header (equality test, not an upper bound).
* ``-csp-max-num-computes`` (unsigned, default 4): maximum number of compute
  instructions in the header.
* ``-csp-live-range-tolerance`` (double, default 0.2): tolerated coord live
  range as a fraction of the header size.

The legacy pipeline also has ``-enable-simplify-coord-update-pass`` (default
``true``) in ``PassManagerBuilder.cpp``, which only controls whether the pass
is scheduled.

**Maintainer notes.**

* State carried across loops: ``BBInstOrder``, ``InstCloneMap`` and
  ``ExitLcssaPhiMap`` are never cleared (``resetContainers`` clears only
  ``TargetInsts``, ``LoopStructureInsts`` and ``NonComputeIntrinIDs``).
  ``TPCOptUtils::findLastUse`` fills ``BBInstOrder`` only when it is empty, so
  every loop after the first one that reaches ``findLastUse`` in the same pass
  instance uses a stale order map (unknown instructions read as 0). This affects
  the live-range check and where ``add.mask`` is placed. The other two maps keep
  keys that may dangle after instructions are erased. The new-PM pass keeps the
  implementation object alive across ``run()`` calls to reproduce the legacy
  behaviour; fixing this means clearing the maps for both pass managers at once.
* The step is taken from an arbitrary candidate (``*TargetInsts.begin()`` of a
  pointer-keyed ``std::unordered_set``) and used for all candidates. Whether
  candidates with different induction variables or steps can occur together is
  unclear.
* Check 5 in ``isSafeToSimplifyInst`` only rejects more than one induction use.
  When there is none, ``*InductionUses.begin()`` dereferences the end iterator
  of an empty set.
* ``Int32OpType = 2`` is hard-coded (TODO: use an enum once TPCII enums are
  mapped to IR). Other TODOs: the live-range estimate depends on the
  ``add.mask`` insertion point; the case with no use of the ``insertelement``
  inside the header "should have been handled in loop strength reduction";
  ``computeAddMaskStep`` should use ``TPCOptUtils::getAddend``.
* A comment assumes the pass runs before loop unrolling. In the legacy pipeline
  it sits in ``addFunctionSimplificationPasses`` after LoopDeletion and
  LoopInterchange, followed by LoopSimplifyCFG and SimpleLoopUnroll.
* The legacy pass never calls ``skipLoop()``, so it also runs on ``optnone``
  functions. The new-PM pass is marked ``isRequired()`` for the same behavior.

EliminateSwizzleCastPass
========================

:Source: ``llvm/lib/Transforms/Scalar/EliminateSwizzleCast.cpp``
:Kind: function pass
:New PM: ``EliminateSwizzleCastPass`` (``cast-swizzle-opt``)

**Purpose.** Finds pairs of mutually inverse conversions on a def-use path,
such as ``fpext`` followed later by ``fptrunc``. The pairs it recognises are
``fpext``/``fptrunc``, ``sitofp``/``fptosi`` and ``uitofp``/``fptoui``, plus
their ``llvm.tpc.*_swch`` intrinsic forms. It replaces both casts of each pair
with ``llvm.tpc.convert`` intrinsics. The file header says the goal is to use
the convert intrinsics "without swizzling", and the pass description is
"Eliminate swizzle ops". The code does not document the hardware reason why
the default lowering of these casts swizzles lanes (unclear). With
``-aggressive-cast-swizzle`` it also handles an ``fptrunc`` that feeds
``llvm.tpc.st.tnsr`` (store-pack).

**How it works.**

- Build a function-level ``DataDependenceGraph`` from a local
  ``DependenceInfo(F, AA, SE, LI)``. Only ``SimpleDDGNode`` nodes are
  considered. For each node, only the result for its first instruction is
  used (``getIntrestingInst``).
- For each (start, end) opcode pair in the global ``marker`` table:

  - Forward paths: start at nodes whose opcode is the start opcode and follow
    def-use edges that are not memory edges. A path ends at the end opcode,
    at a PHI, at ``tpc_ld_g`` fed by ``tpc_gen_addr`` (scalar broadcast), at
    ``tpc_ld_tnsr``, or at ``tpc_st_tnsr`` (store-pack mode).
  - Backward paths: the same walk in the other direction, from end-opcode
    nodes along use-def edges.
  - Map each backward path's first node to its last node (``PostDomSet``).
    Forward paths only act as an emptiness check.
  - Legality filter (``isLegalToTransform``):

    - drop entries whose end node has a 256-element integer or float vector
      result ("unsupported convert ops");
    - drop entries with mismatched opcodes;
    - drop load/load pairs;
    - drop fptrunc/store-pack entries whose fptrunc has several uses and no
      valid fptrunc/fpext partner;
    - reject the whole set if it overlaps instructions that an earlier pair
      already accepted;
    - otherwise drop entries with a cast that has no ``llvm.tpc.convert``
      form: the switch table has no entry for its types (for example
      ``i32``/``f16``), or a ``*_swch`` switch is not a constant.

  - When a pair is accepted, ``CheckLegal`` skips the reversed pair that
    follows it (for example, ``fptrunc→fpext`` after ``fpext→fptrunc``).

- ``applyTransform``: take every instruction in the accepted map. A plain cast
  is replaced with ``createConvertIntrinsic(I, 0)``. A ``*_swch`` intrinsic is
  replaced with ``createConvertIntrinsic(I, <its constant switch operand 1>)``.
  On ``tpc_st_tnsr``, bit ``1 << 2`` is ORed into switch operand 3.
  ``tpc_ld_g`` instructions are left unchanged.

**Requirements and effects.** The input IR needs TPC intrinsics for the
store, load and ``_swch`` patterns. The pass uses AA, ScalarEvolution and
LoopInfo, but only to construct ``DependenceInfo`` and the DDG. It changes no
CFG: it replaces instructions with calls and edits call operands. The legacy
pass declares no preserved analyses. The NPM version returns ``none()`` after
a change and ``all()`` otherwise. The change flag is conservative: it is true
whenever the accepted map is non-empty.

**Options.** ``-aggressive-cast-swizzle`` (``bool``, default ``false``,
hidden): turns on the ``fptrunc`` → ``st.tnsr`` store-pack pattern (the
``{FPTrunc, Call}`` entry of ``marker``).

**Maintainer notes.**

- The legacy ``runOnFunction`` always returns ``false``, even after it
  transforms the IR. The port keeps this (``EliminateSwizzleCast.cpp:1073``).
- ``marker`` is a ``std::unordered_multimap``, so the order in which it is
  iterated is implementation-defined. The "skip the reversed pair" logic in
  ``processDDG`` depends on that order.
- ``CheckLegal`` compares the member ``EndOpCode``, not its ``EndCode``
  parameter. The only caller passes the same value, so the result is the
  same.
- ``computeAllBWPath`` is only ever called with ``IsStorePackSpl=false``
  (``EliminateSwizzleCast.cpp:202``), so the "trivial store-pack" branch
  never runs.
- ``#include "llvm/Passes/PassBuilder.h"`` in a Transforms/Scalar file is a
  layering inversion that predates the port.
- Pipeline (legacy): the pass is scheduled only for TPC targets with
  ``-enable-cast-swizzle-opt`` (default false), after loop
  deletion/interchange in the function simplification pipeline
  (``PassManagerBuilder.cpp:512-519``).

TPCEliminateRedundantLinearityPass
==================================

:Source: ``llvm/lib/Target/TPC/TPCEliminateRedundantLinearity.cpp``
:Kind: function pass
:New PM: ``TPCEliminateRedundantLinearityPass(TPCEliminateRedundantLinearityPassConfig Cfg = {})`` (``tpc-eliminate-redundant-linearity``)

**Purpose.** Replaces ``tpc.convert.linear`` with the cheaper ``tpc.convert``
where the element-order (linearity) guarantee cannot change the program
output. Where allowed, it compensates by adding PACK/UNPACK switches to the
tensor stores and loads that bound the affected data flow.

**How it works.**

* Builds a function-level Data Dependence Graph (``DependenceInfo`` plus
  ``DataDependenceGraph``).

* Collects DDG components around each ``tpc.convert.linear`` node, bounded
  by other ``tpc.convert.linear``, ``tpc.st.tnsr`` or ``tpc.ld.tnsr`` nodes.
  Components are merged when they share input targets.

* Skips components that are trivial, larger than
  ``-tpc-erl-comp-size-limit``, or contain non-single-instruction or
  pi-block nodes.

* For each output index of each output node, the pass builds expression
  trees with and without linearity (``VectorExpr``) and checks that they
  are equal.

  * Supported instructions: casts, integer and FP arithmetic, logic, div/rem,
    compares, ``select``, ``phi``, ``shufflevector``.
  * Supported intrinsics include ``tpc_add/sub/mul/min/max/abs/mac/madd``,
    ``fclass``, ``calc_fp_special``, ``ld_g``, ``gen_addr`` and ``convert``.

* Components bounded only by convert.linear are processed first, then those
  that also need store-pack or load-unpack patching.

* Applies the plan: clones each ``tpc.convert.linear`` into ``tpc.convert``
  (first 6 operands) and ORs the switch patches into tensor store/load
  intrinsics.

**Requirements and effects.**

* Uses AA, ScalarEvolution and LoopInfo (new PM: ``AAManager``,
  ``ScalarEvolutionAnalysis``, ``LoopAnalysis``). They are requested only
  after the early exits for the option and for "no ``tpc.convert.linear`` in
  the function".

* Reports a change iff some ``tpc.convert.linear`` was replaced. The new PM
  version then returns ``none()``, mirroring legacy, which declares only
  required analyses.

* Optional: the legacy pass calls ``skipFunction``.
* Statistics: optimized converts, patched tensor loads, patched tensor
  stores.

**Options.**

* ``-tpc-eliminate-redundant-linearity`` (default ``true``): enables the
  pass.

* ``-tpc-erl-comp-size-limit`` (default ``250``, 0 = unlimited): maximum DDG
  component size analyzed.

* ``-tpc-erl-enable-store-pack`` (default ``true``): store PACK patching.
* ``-tpc-erl-enable-store-pack-32-to-16``, ``-32-to-8`` and ``-16-to-8``
  (each default ``true``).

* ``-tpc-erl-enable-load-unpack`` (default ``true``).

Each patch kind is also gated by the matching ``Cfg.HasFeature*`` field. The
legacy pipeline fills these from ``hasTnsrPack()``, ``hasTnsrPackDT()`` and
``hasTnsrUnpack()``. The default config has all features off.

**Maintainer notes.**

* Legacy placement is ``EP_ModuleOptimizerEarly``, immediately after adding
  ``TPCAAWrapperPass`` and the external AA hook. In the new PM, ``TPCAA``
  must be registered in the ``AAManager`` for the same alias results.

* The file header lists known limitations and possible improvements, such as
  order-agnostic reductions and avoiding recursion.

* The cl::opt ``-tpc-eliminate-redundant-linearity`` has the same name as the
  legacy pass argument (``DEBUG_TYPE``). With ``opt``'s legacy ``-<pass>``
  flags this may conflict (not verified).

* The new PM class is named ``TPCEliminateRedundantLinearityPass`` rather than
  ``EliminateRedundantLinearityLegacyPassPass``.

TPCAAWrapperPass / TPCAA
========================

:Source: ``llvm/lib/Target/TPC/TPCAliasAnalysis.cpp``, ``llvm/lib/Target/TPC/TPCAliasAnalysis.h``
:Kind: analysis (alias analysis)
:Legacy: ``createTPCAAWrapperPass()`` (``-TPC-aa``, ImmutablePass), hooked into ``AAResults`` by ``createTPCExternalAAWrapperPass()`` in ``TPCTargetMachine.cpp``
:New PM: ``TPCAA`` (``TPC-aa``), result ``TPCAAResult``; must be added to the ``AAManager``

**Purpose.** Address-space based alias analysis. Pointers into two different
known TPC address spaces cannot alias.

**How it works.**

* ``alias()`` returns ``NoAlias`` when both locations have non-zero and
  different address spaces. Address space 0 is treated as unknown.

* Otherwise it returns ``AAResultBase::alias``, which is ``MayAlias``, so the
  other AAs in the aggregation decide.

* ``pointsToConstantMemory()`` forwards to the base.

**Requirements and effects.** Stateless. The result is never invalidated
(``invalidate`` returns ``false``). The legacy wrapper preserves everything
and creates and destroys its result in ``doInitialization`` and
``doFinalization``.

**Options.** None.

**Maintainer notes.**

* ``TPCAA`` already existed in the header, but it could not be used with the
  LLVM 13 new PM. It had ``static char PassID`` instead of
  ``static AnalysisKey Key``, and a 2-argument ``invalidate`` that the
  ``AnalysisManager`` cannot call. Both are fixed; ``AnalysisKey TPCAA::Key``
  is defined in the ``.cpp``.

* The second check in ``alias()`` (``TPCAliasAnalysis.cpp:67``) is redundant
  with ``getAliasResult``.

* Legacy users: ``EP_EarlyAsPossible`` and ``EP_ModuleOptimizerEarly`` at
  ``-O1`` and above (before ``EliminateRedundantLinearity``), and
  ``TPCPassConfig::addIRPasses`` for codegen.

* ``initializeTPCAAWrapperPassPass`` is commented out in target
  initialization, but the wrapper constructor calls it.

LoopTaken
=========

:Source: ``llvm/lib/Transforms/Scalar/LoopTaken.cpp``
:Kind: function pass
:New PM: ``LoopTakenPass`` (``looptaken``)

**Purpose.** Honours ``#pragma loop_taken``. Clang emits
``!{!"llvm.loop.taken", i1 …}`` into the loop metadata
(``CGLoopInfo.cpp:458-463``). For each such loop, this pass inserts an
``llvm.assume`` stating that the loop is entered. The file header says this
keeps the CFG simple for later passes such as Unroll-and-Jam.

**How it works.**

- Call ``simplifyLoop(L, DT, LI, SE, AC, nullptr, /*PreserveLCSSA=*/true)``
  on every top-level loop. ``simplifyLoop`` also handles the nested loops.
- Put every loop, nested ones included, on a worklist
  (``appendLoopsToWorklist(LI)``).
- For each loop:

  - Read ``MD_loop`` on the latch terminator.
  - If an operand is an ``MDNode`` whose first operand is the string
    ``"llvm.loop.taken"``, and both ``Loop::getBounds(SE)`` and
    ``getInductionVariable(SE)`` succeed, insert
    ``%c = icmp sgt Final, Initial`` (or ``slt`` when the direction is not
    ``Increasing``) and ``call @llvm.assume(%c)`` before the preheader's
    terminator.

**Requirements and effects.** The loop nest must be in LCSSA form.
``simplifyLoop`` with ``PreserveLCSSA=true`` asserts this in builds with
assertions enabled (``LoopSimplify.cpp:727-731``). After ``simplifyLoop``
every loop must have a preheader. The pass uses DominatorTree, LoopInfo,
ScalarEvolution and AssumptionCache. ``simplifyLoop`` updates DT, LI and SE
in place. The legacy pass declares no preserved analyses. The NPM version
returns ``none()`` when it inserted an assume or ``simplifyLoop`` changed
anything, and ``all()`` otherwise.

**Options.** None. The ``OptLevel`` argument of ``LoopTakenPass`` is ignored.

**Maintainer notes.**

- The ``i1`` value in ``llvm.loop.taken`` is never read. Metadata with
  ``i1 false`` also produces the assume (``LoopTaken.cpp:119-126``). Clang
  emits ``false`` for ``LoopAttributes::Disable``.
- ``Direction::Unknown`` is handled like ``Decreasing`` (``slt``). The
  predicate is always signed, whatever the signedness of the loop's exit
  compare (``LoopTaken.cpp:79-82``).
- The legacy return value covers only inserted assumes. It ignores CFG
  changes made by ``simplifyLoop`` (``LoopTaken.cpp:164-168``, kept as is).
  The NPM version reports those changes too. Without that, the NPM CFG
  checker (``-verify-cfg-preserved``, on by default in builds with
  assertions) could fail.
- A merge12 comment asks whether ``reverse(LI)`` should be used for the
  worklist (``LoopTaken.cpp:144-146``).
- This pass does not remove the metadata. The TPC backend reads it again
  (``TPCHardwareLoops.cpp:1282``,
  ``TPCSoftwareLoopPipelining.cpp:1261``, ``TPCLatencyResolver.cpp:983``).
- Pipeline (legacy): the pass is added only when unroll-and-jam is enabled
  and unrolling is not disabled. It runs before
  ``addFunctionSimplificationPasses`` and ``LoopUnrollAndJam``
  (``PassManagerBuilder.cpp:985-988``). There it follows ``LoopRotate``,
  which leaves the loops in LCSSA form.

TPCLicmPass
===========

:Source: ``llvm/lib/Transforms/Scalar/TPC_LICM.cpp``
:Kind: loop pass
:New PM: ``TPCLICMPass`` (``tpc-licm``)

**Purpose.** A small LICM specialised for TPC lookup intrinsics
(``llvm.tpc.lookup``, ``lookup.1c``, ``lookup.2c``, ``lookup.c0``,
``lookup.c1c2``). It hoists such a call out of the loop, together with the
instructions that compute its first operand from ``llvm.read_register``, when
all other operands are constants. The file comment describes it as "a simple
LICM for moving a list of known intrinsics". The generic LICM pass does not
hoist these lookups: ``llvm.read_register`` reads memory, and the loop's stores
keep it in place. On a nested-loop kernel without this pass, the lookups stay
in the inner loop. Habana's 1.24.2 compiler does not run this pass but hoists
the same lookups; its pipeline has a machine-level ``tpc-lookup-licm-pass``,
which the fork does not.

**How it works.**

* Target block: the parent loop's header for a nested loop, otherwise
  ``Loop::getLoopPredecessor()``; no target means no change. Instructions are
  moved before the target block's terminator.
* For each block of the loop, it finds lookup intrinsics whose operands 1..N
  are ``Constant`` (the loop runs over all call operands, so it includes the
  callee).
* ``searchSource`` walks back from operand 0 through non-PHI instructions in
  the same block until it reaches ``llvm.read_register``. Each instruction on a
  path that reaches it is collected, and once one has, so is every instruction
  the walk visits after it; an empty result means the call is skipped.
* The chain moves only if every value its instructions use is computed by the
  chain or dominates the target block's terminator. Otherwise, for example when
  the index also adds the induction variable, the lookup stays in the loop.
* After each block is scanned, the collected chain and the intrinsics are moved
  in order (definitions first) before the target block's terminator.
* Loops are visited inner loops first, so a lookup hoisted into a parent header
  is hoisted again when the parent loop is processed.

**Requirements and effects.** The legacy pass requires only LoopInfo, which it
does not use; it does not request LoopSimplify or LCSSA form (in the legacy
pipeline it shares a loop pass manager with LoopUnroll, which does). It
preserves the CFG. Legacy: ``setPreservesCFG()`` only, so ScalarEvolution is
not preserved. New PM: ``getLoopPassPreservedAnalyses()`` plus ``CFGAnalyses``
when changed. It also calls ``ScalarEvolution::forgetLoopDispositions`` because
instructions leave the loop, as upstream LICM does. ``llvm.read_register`` is
``IntrReadMem``, so moving it invalidates MemorySSA; the lookup intrinsics are
``IntrNoMem``.

**Options.**

* ``-tpc-licm-pass`` (bool, default ``true``): when false the pass does
  nothing.

**Maintainer notes.**

* There is no memory-dependence or aliasing check before hoisting
  ``read_register``. The target block is either the parent loop's header or
  ``getLoopPredecessor()``, which, unlike ``getLoopPreheader()``, may have
  other successors. Hoisted code can therefore run on paths that never enter
  the loop. A read of the ``lfsr`` register, which changes on every read, is
  hoisted out of a loop even without this pass, by the fork, LLVM 12 and
  Habana alike.
* The legacy pass never calls ``skipLoop()``, so the new-PM pass is marked
  ``isRequired()`` and also runs on ``optnone`` functions.
* Legacy placement: ``addVectorPasses``, right after ``createLoopUnrollPass``
  and before InstCombine/LICM (TPC target only). In the new PM, LoopUnroll is a
  function pass, so this pass needs its own loop adaptor.

LoopSWP
=======

:Source: ``llvm/lib/Transforms/Scalar/LoopSWPPass.cpp``
:Kind: loop pass
:New PM: ``LoopSWPPass`` (``loop-swp``)

**Purpose.** IR-level software pipelining of unrolled, single-block innermost
TPC loops. The ``ld_tnsr`` loads of the next unrolled iteration(s) are issued
one loop iteration ahead: the preheader issues the first loads, the loop body
loads for the following iteration, and the exit block runs the computes and
stores that are left. The code describes the pass only as "Apply Software
Pipelining optimization for TPC at LLVM-IR". The expected benefit for TPC
(presumably hiding ``ld_tnsr`` latency) is not stated.

**How it works.**

* Enablement: ``(-loop-software-pipelining && loop has
  llvm.loop.unroll.disable metadata) || loop has llvm.loop.ir.pipeline
  metadata``. Clang emits the latter from the TPC loop-pipeline hint.
* Legality (``isPipeliningPossible``): the loop must be innermost and
  single-block, and must not carry ``llvm.loop.unroll.remainderloop.marker``
  (set by LoopUnroll on remainder loops). Induction and loop-structure
  instructions must be identified. ``checkRequirements`` rejects headers that
  look already pipelined (an ``ld_tnsr`` with no in-block non-PHI user) and
  headers with both ``st_tnsr`` and values live out of the header
  (accumulators).
* ``IterationClusterizer`` splits the header into N clusters, one per unrolled
  iteration, each with at least one ``ld_tnsr``. A ``LoadStoreSequence``
  (coord and income/predicate definition chains) is built for every
  load/store. No compute may be left outside the clusters. Store coord
  sequences derived from load coord sequences are split (cloned), which already
  modifies the IR. Coord update chains are collected and must have uniform
  constant steps. The prefetch count X is ``-swp-load-prefetch-count`` or N,
  forced to N when an induction PHI is part of a coord sequence, and must be in
  1..N.
* Preheader: the loads of clusters 0..X-1 are cloned with their sequences. The
  X-th coord update (``add.mask``) or the induction update is cloned, and these
  clones become the header PHIs' preheader incoming values.
* Header: each load of clusters 0..X-1 is replaced by a new PHI (preheader
  clone / load of cluster i+N-X). Uses of the loads of clusters X..N-1 are
  redirected to the loads of cluster i-X.
* ``updateLoopBounds``: the preheader applies the induction update once to the
  start value, re-evaluates the latch condition, and its unconditional branch
  is replaced by ``br %cond, %exit, %header``.
* Exit block: clones the (PHI-forwarded) loads, computes and stores or
  accumulators of all N clusters plus the trailing coord updates. It adds
  preheader incoming values to the LCSSA PHIs and redirects their users to the
  last update.

**Requirements and effects.** Requires LoopSimplify and LCSSA form (legacy
``getLoopAnalysisUsage``) and asserts LCSSA form of the loop nest afterwards
(debug builds). Uses ScalarEvolution (induction detection, clusterizer) and,
for the assertion only, LoopInfo and DominatorTree. It changes the CFG (new
preheader->exit edge). The loop is then no longer in LoopSimplify form: the
preheader has two successors and the exit is no longer dedicated. It clones
``st_tnsr`` (``IntrWriteMem``), so MemorySSA is invalidated. Legacy: claims to
preserve ScalarEvolution and the ``getLoopAnalysisUsage`` set (DT, LI,
LoopSimplify, LCSSA, AA), but does not update DT or SE. New PM: when changed,
updates the DominatorTree for the new edge, calls
``ScalarEvolution::forgetLoop`` and returns ``getLoopPassPreservedAnalyses()``
(no ``CFGAnalyses``).

**Options.**

* ``-loop-software-pipelining`` (bool, default ``false``): together with
  ``llvm.loop.unroll.disable`` loop metadata, enables the pass for a loop.
  Without it, only loops with ``llvm.loop.ir.pipeline`` are pipelined.
* ``-swp-load-prefetch-count`` (unsigned, default 0 = number of clusters):
  number of clusters whose loads are prefetched.

The legacy pipeline also has ``-enable-loop-swp-pass`` (default ``true``) in
``PassManagerBuilder.cpp``. It schedules the pass in ``populateModulePassManager``
after GlobalDCE/ConstantMerge, before LoopSink.

**Maintainer notes.**

* The legacy ``runOnLoop`` always returns ``false``, even after pipelining a
  loop (kept unchanged). The new-PM pass reports changes correctly, including
  bail-outs after ``splitLoadStoreCoords`` already modified the header.
* ``HasLoadIncome`` and ``HasInductionInCoordSeq`` are never reset (not in
  ``resetInstContainers``). Once set by one loop, they stay set for every later
  loop handled by the same pass instance, which changes the prefetch count and
  load PHI creation. The new-PM pass keeps the implementation alive across
  ``run()`` calls to reproduce this.
* ``isPipelinePragmaDefined`` checks only that ``llvm.loop.ir.pipeline`` is
  present. Clang emits it with an ``i1 false`` operand for a disabled hint, so
  a "disable" hint also enables pipelining.
* In ``isUnrollPragmaDefined``, the ``llvm.loop.unroll.count`` check after the
  ``return`` is unreachable.
* ``isPipeliningPossible`` ignores the result of ``setBasicBlocks()``, so a
  missing preheader or exit block is not caught there.
* ``updateLoopBounds`` assumes the latch compare's LHS is the induction update
  (operand 0 = induction PHI, operand 1 a ``Constant``) and that the exit is
  taken when the condition is true ("NOTE: assumes that the CondLHS is a result
  of single statement").
* ``isPrologueOrEpilogueLoop`` is dead code. ``isPipeliningProfitable`` always
  returns true (TODO). Other TODOs: ensure all clones and updates are available
  before transforming; change the strategy based on unroll factor and
  instruction mix; unnecessary preheader PHIs are assumed to be removed later.
* Interactions: depends on LoopUnroll's remainder marker. IterationInterleave
  has a dedicated strategy for loads whose only in-block user is a PHI, which
  is the shape this pass produces. Do not schedule it in a loop pass manager
  that uses MemorySSA or BlockFrequencyInfo, or before loop passes that need a
  preheader.

IterationInterleavePass
=======================

:Source: ``llvm/lib/Transforms/Scalar/IterationInterleave.cpp``
:Kind: loop pass
:New PM: ``IterationInterleavePass`` (``iter-ilv``)

**Purpose.** It reorders the instructions of an unrolled, single-block
innermost loop so that instructions of the different unrolled iterations
(clusters) are interleaved. This hides latency and gives the TPC VLIW packetizer
more independent instructions, while a cost model limits register pressure.
It also has strategies that place software-pipelined loads between computes and
stores.

**How it works.**

* ``isReasonable``: the loop must be innermost, single-block and rotated.
  ``IterationClusterizer`` (with pipelined-load detection) must succeed with
  UF = number of clusters >= 2. Coord updates (``add.mask``, ``set_indx``,
  ``<5 x i32>`` ``insertelement``), ``ld_tnsr`` and ``st_tnsr`` are collected
  into coord-update, load, compute and store containers. Only containers with
  UF equally sized per-iteration lists are kept. The pass bails out if loads per
  iteration x UF >= 16, or if clusters depend on each other other than through
  coord updates.
* Interleave state: if only coord updates and same-kind memory accesses appear
  between the first memory access of iteration 0 and of iteration 1, the loop
  is treated as already interleaved.
* SWP strategies: used when every ``ld_tnsr`` is a "SWP load" (its only
  in-block user is one PHI). Strategy A needs load, compute and store
  containers: loads are grouped after the computes, stores after the loads,
  and coord updates are spread between computes or loads/stores. Otherwise
  strategy B fills the SWP loads between computes and stores.
* Isomorphic strategy: for each remaining container, the instruction ``i`` of
  iterations ``1..IF-1`` is moved after that of the previous iteration (and
  broken SSA order inside the block is repaired). Cost-model IF: loads = UF,
  coord updates = 0, stores = 0, computes chosen by a vote based on TTI
  ``TCK_Latency`` cost and distance to the first user. ``*_swch`` conversion
  intrinsics and two-address ``mac`` intrinsics push towards IF = 0.
* Optional: ``-iter-ilv-elim-false-coord-update-deps`` bypasses an
  ``insertelement`` coord update that is overwritten at the same index.
  ``-iter-ilv-trim-coord-update-live-ranges`` moves coord updates to just
  before their first in-block user.

**Requirements and effects.** The legacy pass requires only ScalarEvolution
(clusterizer) and TargetTransformInfo (latency costs). It does not request
LoopSimplify or LCSSA form; in the legacy pipeline it shares a loop pass
manager with LoopSimplifyCFG, which does. It only reorders instructions and
rewrites operands within the header and does not change the CFG. Legacy:
``setPreservesCFG()``. New PM: ``getLoopPassPreservedAnalyses()`` plus
``CFGAnalyses``. ``run()`` reports a change whenever the loop passed
``isReasonable``, even if nothing moved. Reordering ``st_tnsr``
(``IntrWriteMem``) invalidates MemorySSA.

**Options.**

* ``-iter-ilv-coord-update-IF``, ``-iter-ilv-load-IF``, ``-iter-ilv-compute-IF``,
  ``-iter-ilv-store-IF`` (int, default -1 = use the cost model): force the
  interleave factor of a container; 0 disables interleaving of that container.
* ``-iter-ilv-elim-false-coord-update-deps`` (bool, default ``false``).
* ``-iter-ilv-trim-coord-update-live-ranges`` (bool, default ``false``).

The legacy pipeline adds the pass only for TPC at ``OptLevel > 1`` when
``-enable-iter-ilv`` (``PassManagerBuilder.cpp``, default ``false``) is set,
preceded by LoopSimplifyCFG at the end of ``populateModulePassManager``.

**Maintainer notes.**

* File-header TODOs: no global cost model choosing between strategies, no cost
  model for the SWP strategies, and O(n) ``std::distance`` on block iterators
  for ordering queries. The pass also does not know about TPC memory
  dependences: it reorders ``ld_*``/``st_*`` without checking same-address
  accesses.
* FIXME: the list of two-address intrinsics in ``generatesTwoAddrCopy`` is
  incomplete.
* The classification and clustering code was copied from LoopSWP (comment in
  the source).
* The legacy pass never calls ``skipLoop()``, so the new-PM pass is marked
  ``isRequired()`` and also runs on ``optnone`` functions.
* ``IterationInterleave.h`` previously declared an unimplemented
  function-pass ``run(Function &, FunctionAnalysisManager &)``. It now declares
  the loop-pass signature.

TpcLoopOpt
==========

:Source: ``llvm/lib/Target/TPC/TPCoptimization.cpp``
:Kind: loop pass
:New PM: ``TpcLoopOptPass`` (``tpcopt``)

**Purpose.** "TPC-IR pipelined optimization". In a two-level loop nest where
the inner loop is a single block that loads a tensor (``tpc_ld_tnsr``),
computes, and stores it (``tpc_st_tnsr``), it software-pipelines the loads.
The loads move to the outer loop (prologue), the next iteration's loads are
issued after the stores inside the inner loop, and the compute and stores for
the last iteration are duplicated into the exit block (epilogue). IRF
coordinates are advanced with ``tpc_add`` on int5 values.

**How it works.**

* Bails out in these cases:

  * ``-unroll-pipelined`` is off.
  * The module has ``llvm.dbg.cu`` ("incorrect work was found with -g").
  * The loop has neither ``llvm.loop.unroll.disable`` nor
    ``llvm.loop.unroll.count`` metadata.
  * The loop has more than one block.

* ``patternCheck``:

  * At least 2 intrinsics, alternating load/store pairs of
    ``tpc_ld_tnsr``/``tpc_st_tnsr``.
  * The same 1-4 instruction gap between each load and its store, and 1-4
    pairs (the unroll size).
  * There is a parent loop, and its first block contains no such load.

* The header has a PHI, and the first one must be an induction
  (``InductionDescriptor`` with SCEV) with a constant step.

* The IRF lane comes from the ``insertelement`` that feeds the first load's
  coordinate, so its index must be a constant.

* ``reorder()``:

  * Splits shared load/store coordinates if needed.
  * Builds the prologue in parent block 0 and rewrites the inner block
    (parent block 1) with clones, PHIs and IRF increments.
  * Builds the epilogue in parent block 2.
  * Fixes up PHI users.

**Requirements and effects.**

* Requires LoopSimplify/LCSSA form and a parent loop whose block vector
  matches the expected 3-block shape.

* Uses ScalarEvolution (new PM: ``AR.SE``).
* Like legacy (``getLoopAnalysisUsage`` plus SCEV), the new PM version
  returns ``getLoopPassPreservedAnalyses()`` (DT, LoopInfo, SCEV) on change
  and ``all()`` otherwise.

* Required (``isRequired()``) because the legacy pass never calls
  ``skipLoop``.

**Options.**

* ``-unroll-pipelined`` (bool, default ``true``, hidden): enables the pass.
* ``-TPCOPT_DEBUG`` (bool, default ``false``, hidden): prints
  "kernel pipelined" to ``dbgs()``.

**Maintainer notes.**

* Most state lives in members that are never reset between loops, so one
  pass object carries it across all loops and functions it runs on.

  * Never reset: ``Diagnostic``, ``LoadId``/``StoreId``, ``CycleSize``,
    ``IRFLocation``, ``InducationPhi``, ``InstructionBufferArray``,
    ``CloneInstruction`` and ``DuplicateInstruction``.
  * Reset per loop: only ``LoadPtrList``/``StorePtrList``.
  * The new PM version keeps this state in one ``TpcLoopOptImpl`` per pass
    object to match the legacy pass exactly. Fixing it would change
    behavior.

* It modifies blocks of the parent loop, not only the current loop, yet
  claims DT/LoopInfo/SCEV are preserved, as the legacy pass did.

* It does not update MemorySSA. In the new PM, use
  ``createFunctionToLoopPassAdaptor(..., /*UseMemorySSA=*/false)`` and keep it
  out of MemorySSA-based loop pipelines.

* Legacy placement: ``EP_OptimizerLast`` at ``-O1`` and above, before DCE,
  ``Globalizer`` and ``GlobalResolver``.

AttributeAdjuster
=================

:Source: ``llvm/lib/Target/TPC/AttributeAdjuster.cpp``
:Kind: function pass
:New PM: ``AttributeAdjusterPass`` (``adjust-attr``)

**Purpose.** Removes the ``optnone`` function attribute "so that instruction
combining can work". The TPC ``-O0`` pipeline runs InstCombine to simplify
instruction selection.

**How it works.**

* If the function has ``optnone``, the pass removes it and reports a change.
  Nothing else is touched (``noinline`` stays).

**Requirements and effects.** Uses no analyses. The new PM version returns
``none()`` on change, mirroring the legacy pass, which declares no preserved
analyses. It is required (``isRequired()``): the legacy pass never calls
``skipFunction``, and an optional new PM pass would itself be skipped on
``optnone`` functions, which would defeat its purpose.

**Options.** None.

**Maintainer notes.**

* In the legacy pipeline it is added only at ``-O0``
  (``EP_EnabledOnOptLevel0``). It runs after ``PromoteMemoryToRegs`` and before
  ``InstructionCombining(2)``, ``Globalizer``, ADCE and ``GlobalResolver``.
  Passes after it only run on the function if they come after it in the same
  pipeline.

* ``doInitialization`` and ``doFinalization`` are no-ops.

Globalizer
==========

:Source: ``llvm/lib/Target/TPC/Globalizer.cpp``
:Kind: module pass
:New PM: ``GlobalizerPass`` (``globalizer``)

**Purpose.** Turns every ``alloca`` into a private global variable. A code
comment notes that allocas live in address space 0 while these globals live in
address space 1 (scalar) or 2 (vector). ``GlobalResolver`` later assigns them
fixed local-memory addresses. The file does not say why allocas cannot be kept.

**How it works.**

* For each ``alloca`` in each function, the pass creates a private global
  with the allocated type and an ``undef`` initializer, named after the
  alloca.

* The global goes in address space 2 if ``isTpcVectorType(type)``, else 1.
* All uses are replaced with ``ValueReplacer``, which rewrites users across
  address spaces (bitcasts, GEPs, PHIs). The replaced instructions are erased
  when the replacer is destroyed at the end of the run.

**Requirements and effects.** Uses no analyses. Reports a change when any
alloca was replaced. The new PM version then returns ``none()`` (no preserved
analyses are declared in legacy). Optional: the legacy pass calls
``skipModule``.

**Options.** None.

**Maintainer notes.**

* The array-size operand of the alloca is ignored. Dynamic or array allocas
  become a single-element global.

* All invocations of a function share one global.
* ``isTpcVectorType`` hits ``llvm_unreachable`` for vector element types it
  does not know (for example i64 vectors).

* ``TPCTargetMachine.cpp`` notes that "Globalizer leaves hanging IR fragments
  that have no uses". At ``-O0`` it is followed by ADCE.

* The large commented-out debug-info block would have tagged globals with
  ``Attribute::Builtin`` for ``GlobalResolver``. ``DIBuilder DIB``,
  ``cutname`` and the ``WorkList`` member are leftovers from it.

* Legacy placement (only when ``!CompileForLibrary``): ``-O0`` at
  ``EP_EnabledOnOptLevel0``. At higher levels it is at ``EP_OptimizerLast``
  after ``TpcLoopOpt`` and DCE. In both cases it is followed by
  ``GlobalResolver``.

GlobalResolver
==============

:Source: ``llvm/lib/Target/TPC/GlobalResolver.cpp``
:Kind: module pass
:New PM: ``GlobalResolverPass`` (``glbresolver``)

**Purpose.** Assigns every used global variable a fixed address in TPC scalar
(address space 1) or vector (address space 2) local memory. It replaces the
global with a constant pointer and records the total local-memory sizes in
module metadata.

**How it works.**

* Which globals are resolved: all except those named ``llvm.*`` and, with
  ``LLVM_TPC_COMPILER``, those in sections ``.source``, ``.KernelInfo`` or
  ``.tpc_compiler``.

* Each used resolvable global gets the next address in its space. Vector
  space is chosen when ``isTpcVectorType(value type)``. Addresses are
  allocated sequentially from 0, by alloc size, with scalar sizes 1-3 rounded
  up to 4.

* All uses are replaced (``ValueReplacer``) by ``inttoptr (i32 Addr)``.
* The global itself gets section ``.sldata``/``.vldata``, external linkage
  and no initializer.

* A non-undef initializer becomes a ``store`` at the start of the entry
  block of the single defined function.

* A ``DW_OP_constu Addr`` is appended to the global's debug expression.
  Globals with ``Attribute::Builtin`` get ``DW_OP_TPC_glob_adress Addr AS``
  instead.

* Unused resolvable globals are erased.
* ``!llvm.tpc.scalar_data`` and ``!llvm.tpc.vector_data`` are replaced with
  the total sizes.

**Requirements and effects.** Exactly one defined function must remain. This
is only asserted: in release builds the last definition is used. Its entry
block must be non-empty. Uses no analyses.

* The legacy pass reports a change only when it erased unused globals, even
  though it also rewrites uses and adds stores.

* The new PM version returns ``none()`` if globals were erased or any used
  resolvable global existed, and ``all()`` otherwise (metadata-only update).

* Optional: the legacy pass calls ``skipModule``.

**Options.** None.

**Maintainer notes.**

* ``isVolatileVariable(GV)`` is evaluated after the uses of ``GV`` have
  already been replaced (``GlobalResolver.cpp:114``), so the initializer
  store appears to be always non-volatile.

* No alignment is applied beyond the 4-byte minimum for small scalars.
* Nothing in this tree sets ``Attribute::Builtin`` on globals (the setter in
  ``Globalizer`` is commented out), so the ``DW_OP_TPC_glob_adress`` path
  looks unused.

* Must run after ``TPCModuleLegalizer`` and inlining have left a single
  function, and after ``Globalizer``.
