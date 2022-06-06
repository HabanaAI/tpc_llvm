#ifndef LLVM_TOOLS_LLVM_OBJDUMP_TPC_ENCODING_INFO_H
#define LLVM_TOOLS_LLVM_OBJDUMP_TPC_ENCODING_INFO_H

#ifdef LLVM_TPC_COMPILER
#include <vector>
#include <unordered_map>

enum TPCArch {
  Goya   = 1,
  Gaudi  = 2,
  Greco  = 3,
  Gaudi2 = 4,
  Gaudib = 5,
  Doron1 = 6
};

struct EncodingField {
  const char *field_name;
  uint8_t start_bit;
  uint8_t field_size;
  EncodingField(const char *fn, int sb, int sz) :
    field_name(fn), start_bit(sb), field_size(sz)
  {}
};

typedef std::vector<EncodingField> EncodingLayout;

struct Encoding {
    const EncodingLayout* generic;
    const EncodingLayout* loop;
    const EncodingLayout* comp1 = nullptr;
    const EncodingLayout* comp2 = nullptr;
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

const static std::vector<EncodingField> dali_encoding = {
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

const static std::vector<EncodingField> gaudi_encoding = {
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

// spu & vpu & immediate  part (compressed format).
const static std::vector<EncodingField> compress0_goya2_encoding = {
/*  0 */  {"IS_COMPRESSED", 0, 1},
/*  1 */  {"COMPRESSION_TYPE", 1, 1},
/*  2 */  {"SPU_OPCODE", 2, 6},
/*  3 */  {"SPU_SRC_A", 8, 7},
/*  4 */  {"SPU_SRC_B", 15, 7},
/*  5 */  {"SPU_DEST", 22, 7},
/*  6 */  {"SPU_OPERANDS_TYPE", 29, 4},
/*  7 */  {"SPU_PREDICATE_POLARITY", 33, 1},
/*  8 */  {"SPU_PREDICATE_ADDRESS", 34, 4},
/*  9 */  {"SPU_SWITCHES", 38, 7},
/* 10 */  {"VPU_OPCODE", 45, 6},
/* 11 */  {"VPU_SRC_A", 51, 8},
/* 12 */  {"VPU_SRC_B", 59, 8},
/* 13 */  {"VPU_DEST", 67, 8},
/* 14 */  {"VPU_OPERANDS_TYPE", 75, 4},
/* 15 */  {"VPU_PREDICATE_POLARITY", 79, 1},
/* 16 */  {"VPU_PREDICATE_ADDRESS", 80, 5},
/* 17 */  {"VPU_SWITCHES", 85, 7},
/* 18 */  {"RESERVED", 92, 4},
/* 19 */  {"IMMEDIATE", 96, 32}};

// load & store & immediate part (compressed format).
const static std::vector<EncodingField> compress1_goya2_encoding = {
/*  0 */  {"IS_COMPRESSED", 0, 1},
/*  1 */  {"COMPRESSION_TYPE", 1, 1},
/*  2 */  {"LOAD_SRC_A", 2, 8},
/*  3 */  {"LOAD_DST", 10, 8},
/*  4 */  {"LOAD_OPCODE", 18, 5},
/*  5 */  {"LOAD_SRC_B", 23, 9},
/*  6 */  {"LOAD_STORE_PREDICATE_POLARITY", 32, 1},
/*  7 */  {"LOAD_STORE_PREDICATE_ADDRESS", 33, 5},
/*  8 */  {"LOAD_SWITCHES", 38, 7},
/*  9 */  {"STORE_SRC_A", 45, 8},
/* 10 */  {"STORE_SRC_B", 53, 8},
/* 11 */  {"STORE_OPCODE", 61, 5},
/* 12 */  {"STORE_SRC_C", 66, 8},
/* 13 */  {"STORE_SWITCHES", 74, 6},
/* 14 */  {"RESERVED", 80, 12},
/* 15 */  {"RESERVED", 92, 4},
/* 16 */  {"IMMEDIATE", 96, 32}};

const static std::vector<EncodingField> uncompress_goya2_encoding = {
/*  0 */  {"IS_COMPRESSED", 0, 1},
/*  1 */  {"RESERVED", 1, 1},
/*  2 */  {"SPU_OPCODE", 2, 6},
/*  3 */  {"SPU_SRC_A", 8, 7},
/*  4 */  {"SPU_SRC_B", 15, 7},
/*  5 */  {"SPU_DEST", 22, 7},
/*  6 */  {"SPU_OPERANDS_TYPE", 29, 4},
/*  7 */  {"SPU_PREDICATE_POLARITY", 33, 1},
/*  8 */  {"SPU_PREDICATE_ADDRESS", 34, 4},
/*  9 */  {"SPU_SWITCHES", 38, 7},
/* 10 */  {"VPU_OPCODE", 45, 6},
/* 11 */  {"VPU_SRC_A", 51, 8},
/* 12 */  {"VPU_SRC_B", 59, 8},
/* 13 */  {"VPU_DEST", 67, 8},
/* 14 */  {"VPU_OPERANDS_TYPE", 75, 4},
/* 15 */  {"VPU_PREDICATE_POLARITY", 79, 1},
/* 16 */  {"VPU_PREDICATE_ADDRESS", 80, 5},
/* 17 */  {"VPU_SWITCHES", 85, 7},
/* 18 */  {"RESERVED", 92, 4},
/* 19 */  {"IMMEDIATE", 96, 32},
/* 20 */  {"RESERVED", 128, 1},
/* 21 */  {"RESERVED", 129, 1},
/* 22 */  {"LOAD_SRC_A", 130, 8},
/* 23 */  {"LOAD_DST", 138, 8},
/* 24 */  {"LOAD_OPCODE", 146, 5},
/* 25 */  {"LOAD_SRC_B", 151, 9},
/* 26 */  {"LOAD_STORE_PREDICATE_POLARITY", 160, 1},
/* 27 */  {"LOAD_STORE_PREDICATE_ADDRESS", 161, 5},
/* 28 */  {"LOAD_SWITCHES", 166, 7},
/* 29 */  {"STORE_SRC_A", 173, 8},
/* 30 */  {"STORE_SRC_B", 181, 8},
/* 31 */  {"STORE_OPCODE", 189, 5},
/* 32 */  {"STORE_SRC_C", 194, 8},
/* 33 */  {"STORE_SWITCHES", 202, 6},
/* 34 */  {"RESERVED", 208, 12},
/* 35 */  {"RESERVED", 220, 4},
/* 36 */  {"VPU_SRC_C", 224, 8},
/* 37 */  {"VPU_SRC_D", 232, 8},
/* 38 */  {"RESERVED", 240, 16}};

// spu & vpu & immediate  part (compressed format).
const static std::vector<EncodingField> compress0_gaudi2_encoding = {
    /*  0 */ {"IS_COMPRESSED", 0, 1},
    /*  1 */ {"COMPRESSION_TYPE", 1, 1},
    /*  2 */ {"SPU_OPCODE", 2, 6},
    /*  3 */ {"SPU_SRC_A", 8, 7},
    /*  4 */ {"SPU_SRC_B", 15, 7},
    /*  5 */ {"SPU_DEST", 22, 7},
    /*  6 */ {"SPU_OPERANDS_TYPE", 29, 4},
    /*  7 */ {"SPU_PREDICATE_POLARITY", 33, 1},
    /*  8 */ {"SPU_PREDICATE_ADDRESS", 34, 4},
    /*  9 */ {"SPU_SWITCHES", 38, 7},
    /* 10 */ {"VPU_OPCODE", 45, 6},
    /* 11 */ {"VPU_SRC_A", 51, 8},
    /* 12 */ {"VPU_SRC_B", 59, 8},
    /* 13 */ {"VPU_DEST", 67, 8},
    /* 14 */ {"VPU_OPERANDS_TYPE", 75, 4},
    /* 15 */ {"VPU_PREDICATE_POLARITY", 79, 1},
    /* 16 */ {"VPU_PREDICATE_ADDRESS", 80, 5},
    /* 17 */ {"VPU_SWITCHES", 85, 7},
    /* 18 */ {"RESERVED", 92, 4},
    /* 19 */ {"IMMEDIATE", 96, 32}};

// load & store & immediate part (compressed format).
const static std::vector<EncodingField> compress1_gaudi2_encoding = {
    /*  0 */ {"IS_COMPRESSED", 0, 1},
    /*  1 */ {"COMPRESSION_TYPE", 1, 1},
    /*  2 */ {"LOAD_SRC_A", 2, 8},
    /*  3 */ {"LOAD_DST", 10, 8},
    /*  4 */ {"LOAD_OPCODE", 18, 5},
    /*  5 */ {"LOAD_SRC_B", 23, 9},
    /*  6 */ {"LOAD_PREDICATE_POLARITY", 32, 1},
    /*  7 */ {"LOAD_PREDICATE_ADDRESS", 33, 5},
    /*  8 */ {"LOAD_SWITCHES", 38, 7},
    /*  9 */ {"STORE_SRC_A", 45, 8},
    /* 10 */ {"STORE_SRC_B", 53, 8},
    /* 11 */ {"STORE_OPCODE", 61, 5},
    /* 12 */ {"STORE_SRC_C", 66, 8},
    /* 13 */ {"STORE_SWITCHES", 74, 7},
    /* 14 */ {"RESERVED", 81, 11},
    /* 15 */ {"RESERVED", 92, 4},
    /* 16 */ {"IMMEDIATE", 96, 32}};

const static std::vector<EncodingField> uncompress_gaudi2_encoding = {
    /*  0 */ {"IS_COMPRESSED", 0, 1},
    /*  1 */ {"RESERVED", 1, 1},
    /*  2 */ {"SPU_OPCODE", 2, 6},
    /*  3 */ {"SPU_SRC_A", 8, 7},
    /*  4 */ {"SPU_SRC_B", 15, 7},
    /*  5 */ {"SPU_DEST", 22, 7},
    /*  6 */ {"SPU_OPERANDS_TYPE", 29, 4},
    /*  7 */ {"SPU_PREDICATE_POLARITY", 33, 1},
    /*  8 */ {"SPU_PREDICATE_ADDRESS", 34, 4},
    /*  9 */ {"SPU_SWITCHES", 38, 7},
    /* 10 */ {"VPU_OPCODE", 45, 6},
    /* 11 */ {"VPU_SRC_A", 51, 8},
    /* 12 */ {"VPU_SRC_B", 59, 8},
    /* 13 */ {"VPU_DEST", 67, 8},
    /* 14 */ {"VPU_OPERANDS_TYPE", 75, 4},
    /* 15 */ {"VPU_PREDICATE_POLARITY", 79, 1},
    /* 16 */ {"VPU_PREDICATE_ADDRESS", 80, 5},
    /* 17 */ {"VPU_SWITCHES", 85, 7},
    /* 18 */ {"RESERVED", 92, 4},
    /* 19 */ {"IMMEDIATE", 96, 32},
    /* 20 */ {"RESERVED", 128, 1},
    /* 21 */ {"RESERVED", 129, 1},
    /* 22 */ {"LOAD_SRC_A", 130, 8},
    /* 23 */ {"LOAD_DST", 138, 8},
    /* 24 */ {"LOAD_OPCODE", 146, 5},
    /* 25 */ {"LOAD_SRC_B", 151, 9},
    /* 26 */ {"LOAD_PREDICATE_POLARITY", 160, 1},
    /* 27 */ {"LOAD_PREDICATE_ADDRESS", 161, 5},
    /* 28 */ {"LOAD_SWITCHES", 166, 7},
    /* 29 */ {"STORE_SRC_A", 173, 8},
    /* 30 */ {"STORE_SRC_B", 181, 8},
    /* 31 */ {"STORE_OPCODE", 189, 5},
    /* 32 */ {"STORE_SRC_C", 194, 8},
    /* 33 */ {"STORE_SWITCHES", 202, 7},
    /* 34 */ {"RESERVED", 209, 11},
    /* 35 */ {"RESERVED", 220, 4},
    /* 36 */ {"VPU_SRC_C", 224, 8},
    /* 37 */ {"VPU_SRC_D", 232, 8},
    /* 38 */ {"RESERVED", 240, 16}};

// spu & vpu & immediate  part (compressed format).
const static std::vector<EncodingField> compress0_doron1_encoding = {
    /*  0 */ {"IS_COMPRESSED", 0, 1},
    /*  1 */ {"COMPRESSION_TYPE", 1, 1},
    /*  2 */ {"SPU_OPCODE", 2, 6},
    /*  3 */ {"SPU_SRC_A", 8, 8},
    /*  4 */ {"SPU_SRC_B", 16, 8},
    /*  5 */ {"SPU_DEST", 24, 8},
    /*  6 */ {"SPU_OPERANDS_TYPE", 32, 4},
    /*  7 */ {"SPU_PREDICATE_POLARITY", 36, 1},
    /*  8 */ {"SPU_PREDICATE_ADDRESS", 37, 4},
    /*  9 */ {"SPU_IS_PREDICATED", 41, 1},
    /* 10 */ {"SPU_SWITCHES", 42, 7},
    /* 11 */ {"VPU_OPCODE", 49, 6},
    /* 12 */ {"VPU_SRC_A", 55, 8},
    /* 13 */ {"VPU_SRC_B", 63, 8},
    /* 14 */ {"VPU_DEST", 71, 8},
    /* 15 */ {"VPU_OPERANDS_TYPE", 79, 4},
    /* 16 */ {"VPU_PREDICATE_POLARITY", 83, 1},
    /* 17 */ {"VPU_PREDICATE_ADDRESS", 84, 5},
    /* 18 */ {"VPU_IS_PREDICATED", 89, 1},
    /* 19 */ {"VPU_SWITCHES", 90, 5},
    /* 20 */ {"ctxt_switch_dis", 95, 1},
    /* 21 */ {"IMMEDIATE", 96, 32}};

// load & store & immediate part (compressed format).
const static std::vector<EncodingField> compress1_doron1_encoding = {
    /*  0 */ {"IS_COMPRESSED", 0, 1},
    /*  1 */ {"COMPRESSION_TYPE", 1, 1},
    /*  2 */ {"LOAD_SRC_A", 2, 8},
    /*  3 */ {"LOAD_DST", 10, 8},
    /*  4 */ {"LOAD_OPCODE", 18, 5},
    /*  5 */ {"LOAD_SRC_B", 23, 9},
    /*  6 */ {"LOAD_PREDICATE_POLARITY", 32, 1},
    /*  7 */ {"LOAD_PREDICATE_ADDRESS", 33, 5},
    /*  8 */ {"LOAD_IS_PREDICATED", 38, 1},
    /*  9 */ {"LOAD_SWITCHES", 39, 7},
    /* 10 */ {"RESERVED", 46, 3},
    /* 11 */ {"STORE_SRC_A", 49, 8},
    /* 12 */ {"STORE_SRC_B", 57, 8},
    /* 13 */ {"STORE_OPCODE", 65, 5},
    /* 14 */ {"STORE_SRC_C", 70, 8},
    /* 15 */ {"STORE_PREDICATE_POLARITY", 78, 1},
    /* 16 */ {"STORE_PREDICATE_ADDRESS", 79, 5},
    /* 17 */ {"STORE_IS_PREDICATED", 84, 1},
    /* 18 */ {"STORE_SWITCHES", 85, 7},
    /* 19 */ {"RESERVED", 92, 3},
    /* 20 */ {"ctxt_switch_dis", 95, 1},
    /* 21 */ {"IMMEDIATE", 96, 32}};

const static std::vector<EncodingField> uncompress_doron1_encoding = {
    /*  0 */ {"IS_COMPRESSED", 0, 1},
    /*  1 */ {"RESERVED", 1, 1},
    /*  2 */ {"SPU_OPCODE", 2, 6},
    /*  3 */ {"SPU_SRC_A", 8, 8},
    /*  4 */ {"SPU_SRC_B", 16, 8},
    /*  5 */ {"SPU_DEST", 24, 8},
    /*  6 */ {"SPU_OPERANDS_TYPE", 32, 4},
    /*  7 */ {"SPU_PREDICATE_POLARITY", 36, 1},
    /*  8 */ {"SPU_PREDICATE_ADDRESS", 37, 4},
    /*  9 */ {"SPU_IS_PREDICATED", 41, 1},
    /* 10 */ {"SPU_SWITCHES", 42, 7},
    /* 11 */ {"VPU_OPCODE", 49, 6},
    /* 12 */ {"VPU_SRC_A", 55, 8},
    /* 13 */ {"VPU_SRC_B", 63, 8},
    /* 14 */ {"VPU_DEST", 71, 8},
    /* 15 */ {"VPU_OPERANDS_TYPE", 79, 4},
    /* 16 */ {"VPU_PREDICATE_POLARITY", 83, 1},
    /* 17 */ {"VPU_PREDICATE_ADDRESS", 84, 5},
    /* 18 */ {"VPU_IS_PREDICATED", 89, 1},
    /* 19 */ {"VPU_SWITCHES", 90, 5},
    /* 20 */ {"RESERVED", 95, 1},
    /* 21 */ {"IMMEDIATE", 96, 32},
    /* 22 */ {"VPU_EXT_SWITCHES", 128, 2},
    /* 23 */ {"LOAD_SRC_A", 130, 8},
    /* 24 */ {"LOAD_DST", 138, 8},
    /* 25 */ {"LOAD_OPCODE", 146, 5},
    /* 26 */ {"LOAD_SRC_B", 151, 9},
    /* 27 */ {"LOAD_PREDICATE_POLARITY", 160, 1},
    /* 28 */ {"LOAD_PREDICATE_ADDRESS", 161, 5},
    /* 29 */ {"LOAD_IS_PREDICATED", 166, 1},
    /* 30 */ {"LOAD_SWITCHES", 167, 7},
    /* 31 */ {"RESERVED", 174, 3},
    /* 32 */ {"STORE_SRC_A", 177, 8},
    /* 33 */ {"STORE_SRC_B", 185, 8},
    /* 34 */ {"STORE_OPCODE", 193, 5},
    /* 35 */ {"STORE_SRC_C", 198, 8},
    /* 36 */ {"STORE_PREDICATE_POLARITY", 206, 1},
    /* 37 */ {"STORE_PREDICATE_ADDRESS", 207, 5},
    /* 38 */ {"STORE_IS_PREDICATED", 212, 1},
    /* 39 */ {"STORE_SWITCHES", 213, 7},
    /* 40 */ {"RESERVED", 220, 3},
    /* 41 */ {"ctxt_switch_dis", 223, 1},
    /* 42 */ {"VPU_SRC_C", 224, 8},
    /* 43 */ {"VPU_SRC_D", 232, 8},
    /* 44 */ {"RESERVED", 240, 16}};

const static std::vector<EncodingField> loop_encoding = {
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

const static std::vector<EncodingField> goya2_loop_encoding = {
    {"IS_COMPRESSED", 0, 1},
    {"COMPRESSION_TYPE", 1, 1},
    {"SPU_OPCODE", 2, 6},
    {"START_VALUE_SRF_SRC", 8, 7},
    {"BOUNDARY_VALUE_SRF_SRC", 15, 7},
    {"STEP_SRF_SRC", 22, 6},
    {"START_VALUE_SEL", 28, 1},
    {"BOUNDARY_VALUE_SEL", 29, 1},
    {"STEP_SEL", 30, 1},
    {"REPEAT_AT_MOST_ONCE", 31, 1},
    {"RESERVED", 32, 1},
    {"PREDICATE_POLARITY", 33, 1},
    {"PREDICATE_ADDRESS", 34, 4},
    {"BOUNDARY_VALUE_IMM", 38, 32},
    {"END_PC_OFFSET", 70, 16},
    {"RESERVED", 86, 10},
    {"START_VALUE_IMM", 96, 32},
    {"STEP_VALUE_IMM", 128, 32},
    {"COMP_MODE", 160, 3}};

const static std::vector<EncodingField> gaudi2_loop_encoding = {
    {"IS_COMPRESSED", 0, 1},
    {"COMPRESSION_TYPE", 1, 1},
    {"SPU_OPCODE", 2, 6},
    {"START_VALUE_SRF_SRC", 8, 7},
    {"BOUNDARY_VALUE_SRF_SRC", 15, 7},
    {"STEP_SRF_SRC", 22, 6},
    {"START_VALUE_SEL", 28, 1},
    {"BOUNDARY_VALUE_SEL", 29, 1},
    {"STEP_SEL", 30, 1},
    {"REPEAT_AT_MOST_ONCE", 31, 1},
    {"RESERVED", 32, 1},
    {"PREDICATE_POLARITY", 33, 1},
    {"PREDICATE_ADDRESS", 34, 4},
    {"BOUNDARY_VALUE_IMM", 38, 32},
    {"END_PC_OFFSET", 70, 16},
    {"RESERVED", 86, 10},
    {"START_VALUE_IMM", 96, 32},
    {"STEP_VALUE_IMM", 128, 32},
    {"COMP_MODE", 160, 3}};

const static std::vector<EncodingField> doron1_loop_encoding = {
    {"IS_COMPRESSED", 0, 1},
    {"COMPRESSION_TYPE", 1, 1},
    {"SPU_OPCODE", 2, 6},
    {"START_VALUE_SRF_SRC", 8, 8},
    {"BOUNDARY_VALUE_SRF_SRC", 16, 8},
    {"STEP_SRF_SRC", 24, 6},
    {"START_VALUE_SEL", 30, 1},
    {"BOUNDARY_VALUE_SEL", 31, 1},
    {"STEP_SEL", 32, 1},
    {"REPEAT_AT_MOST_ONCE", 33, 1},
    {"RESERVED", 34, 2},
    {"PREDICATE_POLARITY", 36, 1},
    {"PREDICATE_ADDRESS", 37, 4},
    {"IS_PREDICATED", 41, 1},
    {"BOUNDARY_VALUE_IMM", 42, 32},
    {"END_PC_OFFSET", 74, 16},
    {"RESERVED", 90, 6},
    {"START_VALUE_IMM", 96, 32},
    {"STEP_VALUE_IMM", 128, 32},
    {"COMP_MODE", 160, 3}};

const static std::unordered_map<TPCArch, Encoding> EncodingDict = {
    {Goya, {&dali_encoding, &loop_encoding}},
    {Gaudi, {&gaudi_encoding, &loop_encoding}},
    {Gaudib, {&gaudi_encoding, &loop_encoding}},
    {Greco, {&uncompress_goya2_encoding, &goya2_loop_encoding, &compress0_goya2_encoding, &compress1_goya2_encoding}},
    {Gaudi2, {&uncompress_gaudi2_encoding, &gaudi2_loop_encoding, &compress0_gaudi2_encoding, &compress1_gaudi2_encoding}},
    {Doron1, {&uncompress_doron1_encoding, &doron1_loop_encoding, &compress0_doron1_encoding, &compress1_doron1_encoding}},
};
#endif

#endif
