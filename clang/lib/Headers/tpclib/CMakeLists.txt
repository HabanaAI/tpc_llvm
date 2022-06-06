set(converter ${LLVM_MAIN_SRC_DIR}/utils/TPC/embed_file.py)
set(compiler clang)

# Deduce compilation options from library source file name (without extension).
function(get_compilation_options bitcode_filename options)
  set(${options} "-march=${bitcode_filename}" PARENT_SCOPE)
endfunction(get_compilation_options)

function(compile_tpclib_file src_dir src_file)
  get_filename_component(filename ${src_file} NAME_WE)
  get_filename_component(filetype ${src_file} EXT)
  set(src_file ${src_dir}/${src_file})
  set(dst_file "${TPCLIB_DIR}/${filename}.bc")
  set(embedded_file "${CLANG_BINARY_DIR}/lib/Frontend/${filename}.bc.inc")
  set(options "")
  get_compilation_options(${filename} options)
  set(target_name "build-${filename}")

  set(tpcdeps ${tpclib_dependencies})
  list(APPEND tpcdeps ${target_name})
  set(tpclib_dependencies ${tpcdeps} PARENT_SCOPE)

  if(${filetype} STREQUAL ".c")
    add_custom_target(${target_name}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${TPCLIB_DIR}
      COMMAND ${compiler} -c -emit-llvm ${options} ${TPC_BITCODE_COMMON_C_OPTIONS} ${TPC_BITCODE_C_COMPILE_OPTIONS} ${src} -o ${dst_file} ${src_file}
      COMMAND ${PYTHON_EXECUTABLE} "${converter}" "${embedded_file}" "${dst_file}"
      COMMENT "Compiling TPC IR library file ${src_file}..."
    )
  elseif(${filetype} STREQUAL ".cpp")
    add_custom_target(${target_name}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${TPCLIB_DIR}
      COMMAND ${compiler} -c -emit-llvm ${options} ${TPC_BITCODE_COMMON_CXX_OPTIONS} ${TPC_BITCODE_CXX_COMPILE_OPTIONS} ${src} -o ${dst_file} ${src_file}
      COMMAND ${PYTHON_EXECUTABLE} "${converter}" "${embedded_file}" "${dst_file}"
      COMMENT "Compiling TPC IR library file ${src_file}..."
    )
  elseif(${filetype} STREQUAL ".ll")
    add_custom_target(${target_name}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${TPCLIB_DIR}
      COMMAND ${compiler} -c -emit-llvm ${options} ${TPC_BITCODE_COMMON_LL_OPTIONS} ${TPC_BITCODE_LL_COMPILE_OPTIONS} ${src} -o ${dst_file} ${src_file}
      COMMAND ${PYTHON_EXECUTABLE} "${converter}" "${embedded_file}" "${dst_file}"
      COMMENT "Compiling TPC IR library file ${src_file}..."
    )
  else()
    message(SEND_ERROR "Do not know how to compile *${filetype}")
  endif()
  set_target_properties(${target_name} PROPERTIES
    FOLDER "Misc"
  )
endfunction(compile_tpclib_file)

# Compile files from the source directory to the build directory
set(tpclib_dependencies)
foreach(f ${TPCLIB_FILES})
  compile_tpclib_file(${CMAKE_CURRENT_SOURCE_DIR} ${f})
endforeach()

add_custom_target(tpclib
  ALL
  DEPENDS
    ${tpclib_dependencies}
)
set_target_properties(tpclib PROPERTIES
  FOLDER "Misc"
  RUNTIME_OUTPUT_DIRECTORY "${TPCLIB_DIR}"
)

set(tpclib_install_dir lib${LLVM_LIBDIR_SUFFIX}/clang/${CLANG_VERSION}/lib)

install(
  FILES ${tpclib_bitcode_files}
  DESTINATION ${tpclib_install_dir}
  COMPONENT clang-tpc-libraries)

if (NOT LLVM_ENABLE_IDE)
  add_llvm_install_targets(install-tpclib
                           DEPENDS tpclib
                           COMPONENT clang-tpc-libraries)
endif()
