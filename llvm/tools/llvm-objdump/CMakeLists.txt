set(LLVM_LINK_COMPONENTS
  AllTargetsDescs
  AllTargetsDisassemblers
  AllTargetsInfos
  BinaryFormat
  CodeGen
  DebugInfoDWARF
  DebugInfoPDB
  Demangle
  MC
  MCDisassembler
  Object
  Support
  Symbolize
  )

add_llvm_tool(llvm-objdump
  llvm-objdump.cpp
  COFFDump.cpp
  ELFDump.cpp
  MachODump.cpp
  WasmDump.cpp
  TPCDump.cpp
  XCOFFDump.cpp
  )

if(HAVE_LIBXAR)
  target_link_libraries(llvm-objdump PRIVATE ${XAR_LIB})
endif()

if(LLVM_INSTALL_BINUTILS_SYMLINKS)
  add_llvm_tool_symlink(objdump llvm-objdump)
endif()

if("TPC" IN_LIST LLVM_TARGETS_TO_BUILD)
  add_llvm_tool_symlink(tpc-llvm-objdump llvm-objdump ALWAYS_GENERATE)
  llvm_install_symlink(tpc-llvm-objdump llvm-objdump ALWAYS_GENERATE)
endif()
