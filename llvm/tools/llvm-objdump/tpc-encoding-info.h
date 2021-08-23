//===-tpc-encoding-info.h-------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TOOLS_LLVM_OBJDUMP_TPC_ENCODING_INFO_H
#define LLVM_TOOLS_LLVM_OBJDUMP_TPC_ENCODING_INFO_H

#ifdef LLVM_TPC_COMPILER
#include <vector>

struct Encoding {
  const char *field_name;
  uint8_t start_bit;
  uint8_t field_size;
  Encoding(const char *fn, int sb, int sz) :
    field_name(fn), start_bit(sb), field_size(sz)
  {}
};

#define SRCA_IND     9
#define SRCA_IND_G3 11
#define DEST_IND    13

#define SPU_OPCODE_IND 0
#define SPU_OPCODE_IND_G3   2

#define VPU_OPCODE_IND      8
#define VPU_OPCODE_IND_G3  10

#define LD_OPCODE_IND      20
#define LD_OPCODE_IND_G3   24
#define LD_OPCODE_IND_G3_C  4 // Compressed.

#define ST_OPCODE_IND      23
#define ST_OPCODE_IND_G3   31
#define ST_OPCODE_IND_G3_C 11 // Compressed.

#define MOV_OPCODE_LD_SLOT  5
#define MOV_OPCODE_VPU_SLOT 8
#define LOOP_OPCODE 34

const static std::vector<Encoding> dali_encoding = {
/*  0 */  {"SPU_OPCODE", 0, 6},
/*  1 */  {"SPU_SRC_A", 6, 7},
/*  2 */  {"SPU_SRC_B", 13, 7},
/*  3 */  {"SPU_DEST", 20, 7},
/*  4 */  {"SPU_OPERANDS_TYPE", 27, 4},
/*  5 */  {"SPU_PREDICATE_POLARITY", 31, 1},
/*  6 */  {"SPU_PREDICATE_ADDRESS", 32, 4},
/*  7 */  {"SPU_SWITCHES", 36, 7},
/*  8 */  {"VPU_OPCODE", 43, 6},
/*  9 */  {"VPU_SRC_A", 49, 8},
/* 10 */  {"VPU_SRC_B", 57, 8},
/* 11 */  {"VPU_SRC_C_ST_SRC_C", 65, 8},
/* 12 */  {"VPU_SRC_D_LD_SRC_B", 73, 9},
/* 13 */  {"VPU_DEST", 82, 8},
/* 14 */  {"VPU_SWITCHES", 90, 3},
/* 15 */  {"VPU_OPERANDS_TYPE", 93, 4},
/* 16 */  {"VPU_PREDICATE_POLARITY", 97, 1},
/* 17 */  {"VPU_PREDICATE_ADDRESS", 98, 5},
/* 18 */  {"LOAD_SRC_A", 103, 8},
/* 19 */  {"LOAD_DST", 111, 8},
/* 20 */  {"LOAD_OPCODE", 119, 5},
/* 21 */  {"STORE_SRC_A", 124, 8},
/* 22 */  {"STORE_SRC_B", 132, 8},
/* 23 */  {"STORE_OPCODE", 140, 5},
/* 24 */  {"LOAD_STORE_PREDICATE_POLARITY", 145, 1},
/* 25 */  {"LOAD_STORE_PREDICATE_ADDRESS", 146, 5},
/* 26 */  {"IMMEDIATE", 151, 32},
/* 27 */  {"RESERVED", 183, 73}};

const static std::vector<Encoding> gaudi_encoding = {
/*  0 */  {"SPU_OPCODE", 0, 6},
/*  1 */  {"SPU_SRC_A", 6, 7},
/*  2 */  {"SPU_SRC_B", 13, 7},
/*  3 */  {"SPU_DEST", 20, 7},
/*  4 */  {"SPU_OPERANDS_TYPE", 27, 4},
/*  5 */  {"SPU_PREDICATE_POLARITY", 31, 1},
/*  6 */  {"SPU_PREDICATE_ADDRESS", 32, 4},
/*  7 */  {"SPU_SWITCHES", 36, 7},
/*  8 */  {"VPU_OPCODE", 43, 6},
/*  9 */  {"VPU_SRC_A", 49, 8},
/* 10 */  {"VPU_SRC_B", 57, 8},
/* 11 */  {"VPU_SRC_C_ST_SRC_C", 65, 8},
/* 12 */  {"VPU_SRC_D_LD_SRC_B", 73, 9},
/* 13 */  {"VPU_DEST", 82, 8},
/* 14 */  {"VPU_SWITCHES", 90, 3},
/* 15 */  {"VPU_OPERANDS_TYPE", 93, 4},
/* 16 */  {"VPU_PREDICATE_POLARITY", 97, 1},
/* 17 */  {"VPU_PREDICATE_ADDRESS", 98, 5},
/* 18 */  {"LOAD_SRC_A", 103, 8},
/* 19 */  {"LOAD_DST", 111, 8},
/* 20 */  {"LOAD_OPCODE", 119, 5},
/* 21 */  {"STORE_SRC_A", 124, 8},
/* 22 */  {"STORE_SRC_B", 132, 8},
/* 23 */  {"STORE_OPCODE", 140, 5},
/* 24 */  {"LOAD_STORE_PREDICATE_POLARITY", 145, 1},
/* 25 */  {"LOAD_STORE_PREDICATE_ADDRESS", 146, 5},
/* 26 */  {"IMMEDIATE", 151, 32},
/* 27 */  {"LOAD_SWITCHES", 183, 4},
/* 28 */  {"STORE_SWITCHES", 187, 4},
/* 29 */  {"RESERVED", 191, 65}};

const static std::vector<Encoding> loop_encoding = {
    {"SPU_OPCODE", 0, 6},
    {"START_VALUE_SRF_SRC", 6, 7},
    {"BOUNDARY_VALUE_SRF_SRC", 13, 7},
    {"STEP_SRF_SRC", 20, 6},
    {"START_VALUE_SEL", 26, 1},
    {"BOUNDARY_VALUE_SEL", 27, 1},
    {"STEP_SEL", 28, 1},
    {"REPEAT_AT_MOST_ONCE", 29, 1},
    {"RESERVED", 30, 1},
    {"PREDICATE_POLARITY", 31, 1},
    {"PREDICATE_ADDRESS", 32, 4},
    {"BOUNDARY_VALUE_IMM", 36, 32},
    {"STEP_VALUE_IMM", 68, 32},
    {"END_PC_OFFSET", 100, 16},
    {"COMP_MODE", 116, 3},
    {"START_VALUE_IMM", 151, 32}};

#endif

#endif
