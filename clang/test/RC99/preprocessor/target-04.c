// RUN: %tpc_clang -S -DDEFAULT %s -o - 2>&1
// RUN: %tpc_clang -S -march=goya -DGOYA %s -o - 2>&1
// RUN: %tpc_clang -S -march=gaudi -DGAUDI %s -o - 2>&1
// RUN: %tpc_clang -S -march=gaudib -DGAUDIB %s -o - 2>&1
// RUN: %tpc_clang -S -march=greco -DGRECO %s -o - 2>&1
// RUN: %tpc_clang -S -march=gaudi2 -DGAUDI2 %s -o - 2>&1

void main() {
#if defined(GOYA) || defined(DEFAULT)
  #ifndef __goya__
    #error "Macro __goya__ is not defined"
  #endif
  #ifndef __dali__
    #error "Macro __dali__ is not defined"
  #endif

  #ifdef __gaudi__
    #error "Macro __gaudi__ is defined"
  #endif
  #ifdef __gaudib__
    #error "Macro __gaudib__ is defined"
  #endif
  #ifdef __goya2__
    #error "Macro __goya2__ is defined"
  #endif
  #ifdef __greco__
    #error "Macro __greco__ is defined"
  #endif
  #ifdef __gaudi2__
    #error "Macro __gaudi2__ is defined"
  #endif

  #ifdef __gaudi_plus__
    #error "Macro __gaudi_plus__ is defined"
  #endif
  #ifdef __greco_plus__
    #error "Macro __greco_plus__ is defined"
  #endif
  #ifdef __gaudi2_plus__
    #error "Macro __gaudi2_plus__ is defined"
  #endif
#endif

#ifdef GAUDI
  #ifdef __goya__
    #error "Macro __goya__ is defined"
  #endif
  #ifdef __dali__
    #error "Macro __dali__ is defined"
  #endif

  #ifndef __gaudi__
    #error "Macro __gaudi__ is not defined"
  #endif

  #ifdef __gaudib__
    #error "Macro __gaudib__ is defined"
  #endif
  #ifdef __goya2__
    #error "Macro __goya2__ is defined"
  #endif
  #ifdef __greco__
    #error "Macro __greco__ is defined"
  #endif
  #ifdef __gaudi2__
    #error "Macro __gaudi2__ is defined"
  #endif

  #ifndef __gaudi_plus__
    #error "Macro __gaudi_plus__ is not defined"
  #endif
  #ifdef __greco_plus__
    #error "Macro __greco_plus__ is defined"
  #endif
  #ifdef __gaudi2_plus__
    #error "Macro __gaudi2_plus__ is defined"
  #endif
#endif

#ifdef GAUDIB
  #ifdef __goya__
    #error "Macro __goya__ is defined"
  #endif
  #ifdef __dali__
    #error "Macro __dali__ is defined"
  #endif
  #ifdef __gaudi__
    #error "Macro __gaudi__ is defined"
  #endif

  #ifndef __gaudib__
    #error "Macro __gaudib__ is not defined"
  #endif
  #ifdef __goya2__

    #error "Macro __goya2__ is defined"
  #endif
  #ifdef __greco__
    #error "Macro __greco__ is defined"
  #endif
  #ifdef __gaudi2__
    #error "Macro __gaudi2__ is defined"
  #endif

  #ifndef __gaudi_plus__
    #error "Macro __gaudi_plus__ is not defined"
  #endif
  #ifdef __greco_plus__
    #error "Macro __greco_plus__ is defined"
  #endif
  #ifdef __gaudi2_plus__
    #error "Macro __gaudi2_plus__ is defined"
  #endif
#endif

#ifdef GRECO
  #ifdef __goya__
    #error "Macro __goya__ is defined"
  #endif
  #ifdef __dali__
    #error "Macro __dali__ is defined"
  #endif
  #ifdef __gaudi__
    #error "Macro __gaudi__ is defined"
  #endif
  #ifdef __gaudib__
    #error "Macro __gaudib__ is defined"
  #endif

  #ifndef __goya2__
    #error "Macro __goya2__ is not defined"
  #endif
  #ifndef __greco__
    #error "Macro __greco__ is not defined"
  #endif

  #ifdef __gaudi2__
    #error "Macro __gaudi2__ is defined"
  #endif

  #ifndef __gaudi_plus__
    #error "Macro __gaudi_plus__ is not defined"
  #endif
  #ifndef __greco_plus__
    #error "Macro __greco_plus__ is not defined"
  #endif
  #ifdef __gaudi2_plus__
    #error "Macro __gaudi2_plus__ is defined"
  #endif
#endif

#ifdef GAUDI2
  #ifdef __goya__
    #error "Macro __goya__ is defined"
  #endif
  #ifdef __dali__
    #error "Macro __dali__ is defined"
  #endif
  #ifdef __gaudi__
    #error "Macro __gaudi__ is defined"
  #endif
  #ifdef __gaudib__
    #error "Macro __gaudib__ is defined"
  #endif
  #ifdef __goya2__
    #error "Macro __goya2__ is defined"
  #endif
  #ifdef __greco__
    #error "Macro __greco__ is defined"
  #endif

  #ifndef __gaudi2__
    #error "Macro __gaudi2__ is not defined"
  #endif

  #ifndef __gaudi_plus__
    #error "Macro __gaudi_plus__ is not defined"
  #endif
  #ifndef __greco_plus__
    #error "Macro __greco_plus__ is not defined"
  #endif
  #ifndef __gaudi2_plus__
    #error "Macro __gaudi2_plus__ is not defined"
  #endif
#endif
}
