//------------------------------------------------------------------------------
//-   This file is a part of TPC compiler build system, it is *NOT* intended to
//-   be used by end users. Documentation presented in this file must be
//-   extracted by Doxygen or similar documentation tool to produce end users
//-   builtin functions reference.
//-
//-   Declarations presented in this file are used by TPC intrinsic generator to
//-   produce intrinsic definitions for TPC compiler. Syntax used here is a
//-   very restricted subset of C declaration syntax.
//------------------------------------------------------------------------------

typedef signed   int   int32_t;
typedef unsigned int   uint32_t;
typedef signed   short int16_t;
typedef unsigned short uint16_t;
typedef signed   char  int8_t;
typedef unsigned char  uint8_t;
typedef signed   char  int4_t;
typedef unsigned char  uint4_t;
typedef _BFloat16 bf16;
typedef _Float8_143 minifloat;
typedef _Float8_152 minihalf;

typedef struct _float64_int64_pair_t  float64_int64;
typedef struct _float64_uint64_pair_t  float64_uint64;
typedef struct _int64_float64_pair_t  int64_float64;
typedef struct _int64_uint64_pair_t  int64_uint64;
typedef struct _uint64_float64_pair_t  uint64_float64;
typedef struct _uint64_int64_pair_t  uint64_int64;
typedef struct _bfloat128_half128_pair_t  bfloat128_half128;
typedef struct _bfloat128_short128_pair_t  bfloat128_short128;
typedef struct _bfloat128_ushort128_pair_t  bfloat128_ushort128;
typedef struct _half128_bfloat128_pair_t  half128_bfloat128;
typedef struct _half128_short128_pair_t  half128_short128;
typedef struct _half128_ushort128_pair_t  half128_ushort128;
typedef struct _short128_bfloat128_pair_t  short128_bfloat128;
typedef struct _short128_half128_pair_t  short128_half128;
typedef struct _short128_ushort128_pair_t  short128_ushort128;
typedef struct _ushort128_bfloat128_pair_t  ushort128_bfloat128;
typedef struct _ushort128_half128_pair_t  ushort128_halft128;
typedef struct _ushort128_short128_pair_t  ushort128_short128;
typedef struct _minifloat256_char256_pair_t minifloat256_char256;
typedef struct _minifloat256_uchar256_pair_t minifloat256_uchar256;
typedef struct _minihalf256_char256_pair_t minihalf256_char256;
typedef struct _minihalf256_uchar256_pair_t minihalf256_uchar256;
typedef struct _char256_minifloat256_pair_t char256_minifloat256;
typedef struct _char256_minihalf256_pair_t  char256_minihalf256;
typedef struct _uchar256_minifloat256_pair_t uchar256_minifloat256;
typedef struct _uchar256_minihalf256_pair_t uchar256_minihalf256;
typedef struct _minifloat256_minihalf256_pair_t minifloat256_minihalf256;
typedef struct _minihalf256_minifloat256_pair_t minihalf256_minifloat256;


/// @file
/// @brief Intrinsic functions
///
/// Most of the intrinsic functions represent particular instruction available
/// in TPC cores and are translated to single instruction.
///
/// Intrinsic names
/// ---------------
///
/// Intrinsic name has generally several components:
/// - scalar/vector prefix (`s_` or `v_`),
/// - operand type (like `f32_`),
/// - instruction mnemonic (like `add`),
/// - switch indicator (like `_acc32`),
/// - distinguishing suffix (like `_vb`).
/// All of them except instruction mnemonic are optional.
///
/// Scalar/vector prefix helps to distinguish between instruction that do the
/// same operation but for scalar and vector data, like `s_i32_add` and
/// `v_i32_add_b`.
///
/// Operand type designates type of operands of the corresponding instruction.
/// In general it should not be considered as the return value type, although in
/// many cases it is so. In some cases operand type is encoded in the instruction
/// mnemonic, for convenience or for backward compatibility, as, for instance, in
/// `s_convert_i8_to_bf16`. So it may absent even in instructions for which
/// operand type is applicable.
///
/// Instruction mnemonic is same as the corresponding instruction name. In some
/// cases the mnemonic is amended with additional information that helps to
/// represent instruction. One example is the aforementioned 'convert' intrinsic.
/// Another case is SEL instructions. Corresponding intrinsics have names like
/// `v_f32_sel_eq_i32_b`. Type are provided to mimic function result and arguments.
/// This instruction compares values of type 'vector of i32' ('arguments') and
/// returns value of type `f32` ('result').
///
/// Switches usually are encoded in a separate argument. However there are
/// switches that change function signature. For instance, if MAC instruction
/// has switch `ACC_F32`, corresponding intrinsic must return `float128`, not
/// `bfloat128` as in the absence of this switch. Different functions must
/// represent the instruction with and without the switch. In such cases the
/// switch is encoded in the intrinsic name: `s_bf16_mac` and `s_bf16_mac_acc32`.
///
/// Distinguishing suffix allows to have different names for the same instruction
/// but with different arguments, line `v_add_b` and `v_add_vb`. In many cases these
/// suffixes are `_b` and `_vb` if instruction allows both scalar and vector
/// predicates.
///
/// Intrinsic arguments
/// -------------------
///
/// The set of argument accepted by an intrinsic is determined by the operands of
/// corresponding instruction. Most intrinsics have the arguments:
/// - switch set,
/// - income value,
/// - predicate,
/// - polarity.
/// They are used just in this sequence at the end of argument list, like:
///
/// @code
///     int32_t s_i32_add(int32_t a, int32_t b, int switches, int32_t income, bool predicate, bool polarity);
/// @endcode
///
/// Switch set represent modifiers provided for instruction. It must be an
/// integer value, known at compile time. It is a combination of constants, like:
///
/// @code
///     s_i32_sub(a, b, SW_SAT | SW_NEG, 0, 1, 0);
/// @endcode
///
/// Each intrinsic allows its own set of switches. If no special switches are
/// needed, zero value should be used.
/// Some switches change type or number of intrinsic arguments, for example,
/// ACC_F32, PARTIAL, TO_16. They cannot be specified in switch set
/// argument, to use them another intrinsic should be used, for instance,
/// `s_bf16_mac_acc32` instead of `s_bf16_mac`.
///
/// The income attribute represent the income value of DEST ('destination')
/// to be updated by the instruction.
/// Predicate, polarity and income value are needed to support conditional
/// execution. Almost every instruction can be executed conditionally
/// depending on the value of some predicate. Intrinsic as a C function must
/// always return appropriate value even if the instruction is not executed. The
/// value returned in this case is determined by income argument.
/// Hypothetical intrinsic for add operation may be represented in pseudocode as:
/// @code
///     int add(int a, int b, int income, bool predicate, bool polarity) {
///       if (predicate != polarity)
///         return a + b;
///       else
///         return income;
///     }
/// @endcode
/// For intrinsic function that receive vector predicate the returned vector will
/// be composed from instruction result elements where predicate == true,
/// and income value elements where predicate == false.
///
/// Some instructions (like SET_INDX or MAC) do not produce new value, but modify
/// some of its input arguments. In these cases separate input value is not
/// required, as the intrinsic may return the argument to modify. Such intrinsics do
/// not have input argument.
///

//
// ------ High-level intrinsics
//

/// @brief Access to LFSR register which is used to generate random numbers.
/// Once the register is read, it randomizes a new number.
///
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return A value of LFSR (VPU_LFSR).
///
/// @switches Allowed switches are:
///     \li SW_READ_ONLY - Don't reseed value.
///
/// @{
char256 read_lfsr_b(int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 read_lfsr_vb(int switches, char256 income, bool256 predicate, bool polarity=0);
/// @}


/// @brief Write to LFSR register which is used to generate random numbers.
///
/// @param src                The argument (SRC1).
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @{
void write_lfsr_b(char256 src, bool predicate=1, bool polarity=0);
void write_lfsr_vb(char256 src, bool256 predicate, bool polarity=0);
/// @}


#if defined(__greco_plus__)
/// @brief Access to S_LFSR register which is used to generate random numbers.
/// Once the register is read, it randomizes a new number.
///
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return A value of S_LFSR (SPU_LFSR).
///
/// @switches Allowed switches are:
///     \li SW_READ_ONLY - Don't reseed value.
///
/// @{
int32_t s_read_lfsr(int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
/// @}
#endif


#if defined(__greco_plus__)
/// @brief Write to S_LFSR register which is used to generate random numbers.
///
/// @param src                The argument (SRC1).
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @{
void write_lfsr_s(int32_t src, bool predicate=1, bool polarity=0);
/// @}
#endif


/// @brief Access to a LANE_ID_* register.
///
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return A value of LANE_ID_*.
///
/// @{
uint64 read_lane_id_4b_b(uint64 income={}, bool predicate=1, bool polarity=0);
uint64 read_lane_id_4b_vb(uint64 income, bool64 predicate, bool polarity=0);

ushort128 read_lane_id_2b_b(ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 read_lane_id_2b_vb(ushort128 income, bool128 predicate, bool polarity=0);

uchar256 read_lane_id_1b_b(uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 read_lane_id_1b_vb(uchar256 income, bool256 predicate, bool polarity=0);
/// @}

/// @section sec1 Convert
/// @subsection subsec1_1 High-level convert
/// @brief Converts the value from one datatype to another data type.
/// The source and destination operands can be vectors of 8/16/32 bit signed and
/// unsigned integers, 32 bit float ,16 bit bfloat.
///
/// @param a        The value to convert.
/// @param options  OR'ed set of constants. \n
///                 Only constants that represent rounding mode are supported now. \n
///                 They are the same constants, that are used as switches in CONVERT intrinsics.
/// @return Converted value
///
/// @switches Allowed switches are:
///     \li SW_LINEAR - Keep linear order of elements (relevant for up- or down-conversions). 
///                     By default, elements are swizzled (grouped by interleaved lanes).
/// \par
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
#if defined(__gaudi_plus__)
///         \li SW_SR - Stochastic Rounding.
///         \li SW_RHAZ - Round half away from zero.
#endif
///         \li SW_CSR - Take rounding mode from
#if defined(__gaudi__) || defined(__gaudib__) || defined(__goya__)
///                    ROUND_CSR register
#endif
#if defined(__greco_plus__)
///                    CONV_ROUND_CSR register
#endif
#if defined(__gaudi2_plus__)
///         \li SW_SR_RNE - Stochastic Rounding, but use RNE when rounding denormals.
/// \par
///     \li SW_CLIP_FP - Convert FP with clipping. \n
///         When the resulting FP value should be +/-inf, it will be clipped to +/-max_normal_value_fp
///
#endif
/// @{
float64 convert_int64_to_float64(int64 a, const int options);
float64 convert_uint64_to_float64(uint64 a, const int options);

#if defined(__gaudib__) || defined(__greco_plus__)
float128 convert_half128_to_float128(half128 a, const int options);
#endif
#if defined(__gaudi_plus__)
float128 convert_bfloat128_to_float128(bfloat128 a, const int options);
#endif
float128 convert_int128_to_float128(int128 a, const int options);
float128 convert_uint128_to_float128(uint128 a, const int options);
float128 convert_short128_to_float128(short128 a, const int options);
float128 convert_ushort128_to_float128(ushort128 a, const int options);

#if defined(__gaudi2_plus__)
float256 convert_minifloat256_to_float256(minifloat256 a, const int options);
float256 convert_minihalf256_to_float256(minihalf256 a, const int options);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
float256 convert_half256_to_float256(half256 a, const int options);
#endif
#if defined(__gaudi_plus__)
float256 convert_bfloat256_to_float256(bfloat256 a, const int options);
#endif
float256 convert_int256_to_float256(int256 a, const int options);
float256 convert_uint256_to_float256(uint256 a, const int options);
float256 convert_short256_to_float256(short256 a, const int options);
float256 convert_ushort256_to_float256(ushort256 a, const int options);
float256 convert_char256_to_float256(char256 a, const int options);
float256 convert_uchar256_to_float256(uchar256 a, const int options);

#if defined(__gaudib__) || defined(__greco_plus__)
bfloat128 convert_half128_to_bfloat128(half128 a, const int options);
#endif
#if defined(__gaudi_plus__)
bfloat128 convert_float128_to_bfloat128(float128 a, const int options);
bfloat128 convert_int128_to_bfloat128(int128 a, const int options);
bfloat128 convert_uint128_to_bfloat128(uint128 a, const int options);
bfloat128 convert_short128_to_bfloat128(short128 a, const int options);
bfloat128 convert_ushort128_to_bfloat128(ushort128 a, const int options);
#endif

#if defined(__gaudib__) || defined(__greco_plus__)
bfloat256 convert_half256_to_bfloat256(half256 a, const int options);
#endif
#if defined(__gaudi2_plus__)
bfloat256 convert_minifloat256_to_bfloat256(minifloat256 a, const int options);
bfloat256 convert_minihalf256_to_bfloat256(minihalf256 a, const int options);
#endif
#if defined(__gaudi_plus__)
bfloat256 convert_float256_to_bfloat256(float256 a, const int options);
bfloat256 convert_int256_to_bfloat256(int256 a, const int options);
bfloat256 convert_uint256_to_bfloat256(uint256 a, const int options);
bfloat256 convert_short256_to_bfloat256(short256 a, const int options);
bfloat256 convert_ushort256_to_bfloat256(ushort256 a, const int options);
bfloat256 convert_char256_to_bfloat256(char256 a, const int options);
bfloat256 convert_uchar256_to_bfloat256(uchar256 a, const int options);
#endif

#if defined(__gaudib__) || defined(__greco_plus__)
half128 convert_float128_to_half128(float128 a, const int options);
half128 convert_bfloat128_to_half128(bfloat128 a, const int options);
half128 convert_int128_to_half128(int128 a, const int options);
half128 convert_uint128_to_half128(uint128 a, const int options);
half128 convert_short128_to_half128(short128 a, const int options);
half128 convert_ushort128_to_half128(ushort128 a, const int options);
#endif

#if defined(__gaudib__) || defined(__greco_plus__)
half256 convert_float256_to_half256(float256 a, const int options);
#endif
#if defined(__gaudi2_plus__)
half256 convert_minifloat256_to_half256(minifloat256 a, const int options);
half256 convert_minihalf256_to_half256(minihalf256 a, const int options);
#endif
#if defined(__gaudi_plus__)
half256 convert_bfloat256_to_half256(bfloat256 a, const int options);
half256 convert_int256_to_half256(int256 a, const int options);
half256 convert_uint256_to_half256(uint256 a, const int options);
half256 convert_short256_to_half256(short256 a, const int options);
half256 convert_ushort256_to_half256(ushort256 a, const int options);
half256 convert_char256_to_half256(char256 a, const int options);
half256 convert_uchar256_to_half256(uchar256 a, const int options);
#endif

#if defined(__gaudi2_plus__)
minifloat256 convert_float256_to_minifloat256(float256 a, const int options);
minifloat256 convert_bfloat256_to_minifloat256(bfloat256 a, const int options);

minifloat256 convert_half256_to_minifloat256(half256 a, const int options);
minifloat256 convert_minihalf256_to_minifloat256(minihalf256 a, const int options);
minifloat256 convert_char256_to_minifloat256(char256 a, const int options);
minifloat256 convert_uchar256_to_minifloat256(uchar256 a, const int options);
minifloat256 convert_short256_to_minifloat256(short256 a, const int options);
minifloat256 convert_ushort256_to_minifloat256(ushort256 a, const int options);

minifloat256 convert_int256_to_minifloat256(int256 a, const int options);
minifloat256 convert_uint256_to_minifloat256(uint256 a, const int options);


minifloat512 convert_minihalf512_to_minifloat512(minihalf512 a, const int options);
minifloat512 convert_char512_to_minifloat512(char512 a, const int options);
minifloat512 convert_uchar512_to_minifloat512(uchar512 a, const int options);

minihalf256 convert_float256_to_minihalf256(float256 a, const int options);
minihalf256 convert_bfloat256_to_minihalf256(bfloat256 a, const int options);
minihalf256 convert_half256_to_minihalf256(half256 a, const int options);
minihalf256 convert_minifloat256_to_minihalf256(minifloat256 a, const int options);
minihalf256 convert_char256_to_minihalf256(char256 a, const int options);
minihalf256 convert_uchar256_to_minihalf256(uchar256 a, const int options);

minihalf256 convert_short256_to_minihalf256(short256 a, const int options);
minihalf256 convert_ushort256_to_minihalf256(ushort256 a, const int options);
minihalf256 convert_int256_to_minihalf256(int256 a, const int options);
minihalf256 convert_uint256_to_minihalf256(uint256 a, const int options);


minihalf512 convert_minifloat512_to_minihalf512(minifloat512 a, const int options);
minihalf512 convert_char512_to_minihalf512(char512 a, const int options);
minihalf512 convert_uchar512_to_minihalf512(uchar512 a, const int options);
#endif
int64 convert_float64_to_int64(float64 a, const int options);
int64 convert_uint64_to_int64(uint64 a, const int options);

#if defined(__gaudi_plus__)
int128 convert_bfloat128_to_int128(bfloat128 a, const int options);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
int128 convert_half128_to_int128(half128 a, const int options);
#endif
int128 convert_float128_to_int128(float128 a, const int options);
int128 convert_uint128_to_int128(uint128 a, const int options);
int128 convert_short128_to_int128(short128 a, const int options);
int128 convert_ushort128_to_int128(ushort128 a, const int options);

#if defined(__gaudi_plus__)
int256 convert_bfloat256_to_int256(bfloat256 a, const int options);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
int256 convert_half256_to_int256(half256 a, const int options);
#endif
#if defined(__gaudi2_plus__)
int256 convert_minifloat256_to_int256(minifloat256 a, const int options);
int256 convert_minihalf256_to_int256(minihalf256 a, const int options);
#endif
int256 convert_float256_to_int256(float256 a, const int options);
int256 convert_uint256_to_int256(uint256 a, const int options);
int256 convert_short256_to_int256(short256 a, const int options);
int256 convert_ushort256_to_int256(ushort256 a, const int options);
int256 convert_char256_to_int256(char256 a, const int options);
int256 convert_uchar256_to_int256(uchar256 a, const int options);

uint64 convert_float64_to_uint64(float64 a, const int options);
uint64 convert_int64_to_uint64(int64 a, const int options);

#if defined(__gaudi_plus__)
uint128 convert_bfloat128_to_uint128(bfloat128 a, const int options);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
uint128 convert_half128_to_uint128(half128 a, const int options);
#endif
uint128 convert_float128_to_uint128(float128 a, const int options);
uint128 convert_int128_to_uint128(int128 a, const int options);
uint128 convert_short128_to_uint128(short128 a, const int options);
uint128 convert_ushort128_to_uint128(ushort128 a, const int options);

#if defined(__gaudi_plus__)
uint256 convert_bfloat256_to_uint256(bfloat256 a, const int options);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
uint256 convert_half256_to_uint256(half256 a, const int options);
#endif
#if defined(__gaudi2_plus__)
uint256 convert_minifloat256_to_uint256(minifloat256 a, const int options);
uint256 convert_minihalf256_to_uint256(minihalf256 a, const int options);
#endif
uint256 convert_float256_to_uint256(float256 a, const int options);
uint256 convert_int256_to_uint256(int256 a, const int options);
uint256 convert_short256_to_uint256(short256 a, const int options);
uint256 convert_ushort256_to_uint256(ushort256 a, const int options);
uint256 convert_char256_to_uint256(char256 a, const int options);
uint256 convert_uchar256_to_uint256(uchar256 a, const int options);

#if defined(__gaudi_plus__)
short128 convert_bfloat128_to_short128(bfloat128 a, const int options);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
short128 convert_half128_to_short128(half128 a, const int options);
#endif
short128 convert_float128_to_short128(float128 a, const int options);
short128 convert_int128_to_short128(int128 a, const int options);
short128 convert_uint128_to_short128(uint128 a, const int options);
short128 convert_ushort128_to_short128(ushort128 a, const int options);

#if defined(__gaudi_plus__)
short256 convert_bfloat256_to_short256(bfloat256 a, const int options);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
short256 convert_half256_to_short256(half256 a, const int options);
#endif
#if defined(__gaudi2_plus__)
short256 convert_minifloat256_to_short256(minifloat256 a, const int options);
short256 convert_minihalf256_to_short256(minihalf256 a, const int options);
#endif
short256 convert_float256_to_short256(float256 a, const int options);
short256 convert_int256_to_short256(int256 a, const int options);
short256 convert_uint256_to_short256(uint256 a, const int options);
short256 convert_ushort256_to_short256(ushort256 a, const int options);
short256 convert_char256_to_short256(char256 a, const int options);
short256 convert_uchar256_to_short256(uchar256 a, const int options);

#if defined(__gaudi_plus__)
ushort128 convert_bfloat128_to_ushort128(bfloat128 a, const int options);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
ushort128 convert_half128_to_ushort128(half128 a, const int options);
#endif
ushort128 convert_float128_to_ushort128(float128 a, const int options);
ushort128 convert_int128_to_ushort128(int128 a, const int options);
ushort128 convert_uint128_to_ushort128(uint128 a, const int options);
ushort128 convert_short128_to_ushort128(short128 a, const int options);

#if defined(__gaudi_plus__)
ushort256 convert_bfloat256_to_ushort256(bfloat256 a, const int options);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
ushort256 convert_half256_to_ushort256(half256 a, const int options);
#endif
#if defined(__gaudi2_plus__)
ushort256 convert_minifloat256_to_ushort256(minifloat256 a, const int options);
ushort256 convert_minihalf256_to_ushort256(minihalf256 a, const int options);
#endif
ushort256 convert_float256_to_ushort256(float256 a, const int options);
ushort256 convert_int256_to_ushort256(int256 a, const int options);
ushort256 convert_uint256_to_ushort256(uint256 a, const int options);
ushort256 convert_short256_to_ushort256(short256 a, const int options);
ushort256 convert_char256_to_ushort256(char256 a, const int options);
ushort256 convert_uchar256_to_ushort256(uchar256 a, const int options);

#if defined(__gaudi_plus__)
char256 convert_bfloat256_to_char256(bfloat256 a, const int options);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
char256 convert_half256_to_char256(half256 a, const int options);
#endif
#if defined(__gaudi2_plus__)
char256 convert_minifloat256_to_char256(minifloat256 a, const int options);
char256 convert_minihalf256_to_char256(minihalf256 a, const int options);
#endif
char256 convert_float256_to_char256(float256 a, const int options);
char256 convert_int256_to_char256(int256 a, const int options);
char256 convert_uint256_to_char256(uint256 a, const int options);
char256 convert_short256_to_char256(short256 a, const int options);
char256 convert_ushort256_to_char256(ushort256 a, const int options);
char256 convert_uchar256_to_char256(uchar256 a, const int options);

#if defined(__gaudi_plus__)
uchar256 convert_bfloat256_to_uchar256(bfloat256 a, const int options);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
uchar256 convert_half256_to_uchar256(half256 a, const int options);
#endif
#if defined(__gaudi2_plus__)
uchar256 convert_minifloat256_to_uchar256(minifloat256 a, const int options);
uchar256 convert_minihalf256_to_uchar256(minihalf256 a, const int options);
#endif
uchar256 convert_float256_to_uchar256(float256 a, const int options);
uchar256 convert_int256_to_uchar256(int256 a, const int options);
uchar256 convert_uint256_to_uchar256(uint256 a, const int options);
uchar256 convert_short256_to_uchar256(short256 a, const int options);
uchar256 convert_ushort256_to_uchar256(ushort256 a, const int options);
uchar256 convert_char256_to_uchar256(char256 a, const int options);
/// @}


//
// ------ ABS
//


/// @section sec2 Arithmetic
/// @subsection subsec2_1 Absolute
/// @brief Represents ABS instruction - Calculates absolute value.
///
/// @param a           The argument (SRC1).
/// @param switches    Switches of ABS instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Absolute value of operand \p a.
///
/// This operation is implemented as instruction ABS for integer operands and 
/// FORM_FP_NUMBER for floating point (actually AND on SPU).
///
#if defined(__greco_plus__)
/// @switches Allowed switches are:
///     \li SW_NO_SAT - Don't saturate ABS(MIN_INT) to MAX_INT.
#endif
///
/// @{
float s_f32_abs(float a, int switches=0, float income=0, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_bf16_abs(bf16 a, int switches=0, bf16 income=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_f16_abs(half a, int switches=0, half income=0, bool predicate=1, bool polarity=0);
#endif
int32_t s_i32_abs(int32_t a, int switches=0, int32_t income=0, bool predicate=1, bool polarity=0);
int16_t s_i16_abs(int16_t a, int switches=0, int16_t income=0, bool predicate=1, bool polarity=0);
int8_t s_i8_abs(int8_t a, int switches=0, int8_t income=0, bool predicate=1, bool polarity=0);
#if defined(__gaudi2_plus__)
minifloat s_f8_abs(minifloat a, int switches=0, minifloat income=0, bool predicate=1, bool polarity=0);
minihalf s_h8_abs(minihalf a, int switches=0, minihalf income=0, bool predicate=1, bool polarity=0);
#endif
float64 v_f32_abs_b(float64 a, int switches=0, float64 income=0, bool predicate=1, bool polarity=0);
float64 v_f32_abs_vb(float64 a, int switches, float64 income, bool64 predicate, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_abs_b(bfloat128 a, int switches=0, bfloat128 income=0, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_abs_vb(bfloat128 a, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_f16_abs_b(half128 a, int switches=0, half128 income=0, bool predicate=1, bool polarity=0);
half128 v_f16_abs_vb(half128 a, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
int64 v_i32_abs_b(int64 a, int switches=0, int64 income=0, bool predicate=1, bool polarity=0);
int64 v_i32_abs_vb(int64 a, int switches, int64 income, bool64 predicate, bool polarity=0);
short128 v_i16_abs_b(short128 a, int switches=0, short128 income=0, bool predicate=1, bool polarity=0);
short128 v_i16_abs_vb(short128 a, int switches, short128 income, bool128 predicate, bool polarity=0);
char256 v_i8_abs_b(char256 a, int switches=0, char256 income=0, bool predicate=1, bool polarity=0);
char256 v_i8_abs_vb(char256 a, int switches, char256 income, bool256 predicate, bool polarity=0);
#if defined(__gaudi2_plus__)
minifloat256 v_f8_abs_b(minifloat256 a, int switches=0, minifloat256 income=0, bool predicate=1, bool polarity=0);
minifloat256 v_f8_abs_vb(minifloat256 a, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_abs_b(minihalf256 a, int switches=0, minihalf256 income=0, bool predicate=1, bool polarity=0);
minihalf256 v_h8_abs_vb(minihalf256 a, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
/// @}

/// @section sec11 IRF
/// @subsection subsec11_5 Abs
/// @brief Represents ABS instruction for int5 operands.
///
/// @param a           The argument (SRC1).
/// @param dimmask     Selects IRF lanes participated in the operation.
/// @param switches    Switches of ABS instructions.
/// @param income      This value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return IRF of absolute values of argument \p a.
///
#if defined(__greco_plus__)
/// @switches Allowed switches are:
///     \li SW_NO_SAT - Don't saturate ABS(MIN_INT) to MAX_INT.
#endif
///
/// @{
int5 i_i32_abs(int5 a, const int dimmask, const int switches, int5 income, bool predicate=1, bool polarity=0);
/// @}


//
// ------ ADD
//

/// @section sec2 Arithmetic
/// @subsection subsec2_3 Add
/// @brief Represents ADD instruction.
///
/// @param a           The first SRC operand to ADD (SRC1).
/// @param b           The second SRC operand to ADD (SRC2).
/// @param switches    Switches of ADD instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Sum of operands \p a and \p b.
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturate (integer types only). \n
///
/// @{
float s_f32_add(float a, float b, int switches=0, float income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_bf16_add(bf16 a, bf16 b, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_f16_add(half a, half b, int switches=0, half income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
float s_f8_add(float a, minifloat b, int switches=0, float income={}, bool predicate=1, bool polarity=0);
float s_h8_add(float a, minihalf  b, int switches=0, float income={}, bool predicate=1, bool polarity=0);
#endif
int32_t s_i32_add(int32_t a, int32_t b, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_u32_add(uint32_t a, uint32_t b, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_i16_add(int16_t a, int16_t b, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_u16_add(uint16_t a, uint16_t b, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_i8_add(int8_t a, int8_t b, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u8_add(uint8_t a, uint8_t b, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
float64 v_f32_add_vb(float64 a, float64 b, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_add_b(float64 a, float64 b, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_add_vb(bfloat128 a, bfloat128 b, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_add_b(bfloat128 a, bfloat128 b, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
half128 v_f16_add_vb(half128 a, half128 b, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_add_b(half128 a, half128 b, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
float256 v_f8_add_b (float256 a, minifloat256 b, int switches=0, float256 income={}, bool    predicate=1, bool polarity=0);
float256 v_f8_add_vb(float256 a, minifloat256 b, int switches,   float256 income,    bool256 predicate,   bool polarity=0);
float256 v_h8_add_b (float256 a, minihalf256  b, int switches=0, float256 income={}, bool    predicate=1, bool polarity=0);
float256 v_h8_add_vb(float256 a, minihalf256  b, int switches,   float256 income,    bool256 predicate,   bool polarity=0);
#endif
int64 v_i32_add_vb(int64 a, int64 b, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_add_b(int64 a, int64 b, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_add_vb(uint64 a, uint64 b, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_add_b(uint64 a, uint64 b, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_add_vb(short128 a, short128 b, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_add_b(short128 a, short128 b, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_add_vb(ushort128 a, ushort128 b, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_add_b(ushort128 a, ushort128 b, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_add_vb(char256 a, char256 b, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_add_b(char256 a, char256 b, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_add_vb(uchar256 a, uchar256 b, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_add_b(uchar256 a, uchar256 b, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
/// @}

/// @section sec11 IRF
/// @subsection subsec11_6 Add
/// @brief Represents scalar ADD instruction for integer operands in index registers.
///
/// @param a           The first SRC operand to ADD (SRC1).
/// @param b           The second SRC operand to ADD (SRC2).
/// @param dimmask     Selects IRF lanes participated in the operation.
/// @param switches    Switches of ADD instructions.
/// @param income      This value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of ADD operation between \p a and \p b.
///
/// @{
int5 i_i32_add(int5 a, int5 b, const int dimmask, int switches, int5 income, bool predicate=1, bool polarity=0);
/// @}

#if defined(__gaudi2_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_5 Add with X2 switch
/// @brief Represents ADD instruction with X2 switch.
///
/// @param a           The first SRC operand to ADD (SRC1).
/// @param b           The second SRC operand to ADD (SRC2).
/// @param c           The third SRC operand to ADD (SRC3).
/// @param switches    Switches of ADD instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation - \c DEST.v1=a.v1+b \c DEST.v2=a.v2+c.
///
/// @note \p a is float128 data-type that map to 2 vector registers of 64 float elements each. \n
/// In the formula above \p a.v1 and \p a.v2 refers to the first and second vectors respectively.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
float128 v_f32_add_x2_vb(float128 a, float64 b, float64 c, int switches, float128 income, bool128 predicate, bool polarity=0);
float128 v_f32_add_x2_b(float128 a, float64 b, float64 c, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ AND
//

/// @section sec2 Arithmetic
/// @subsection subsec2_9 And
/// @brief Represents AND instruction, performs bitwise and.
///
/// @param a           The first SRC operand to AND (SRC1).
/// @param b           The second SRC operand to AND (SRC2).
/// @param switches    Switches of AND instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of AND operation between \p a and \p b.
///
#if defined(__greco_plus__)
/// @switches Allowed switches are:
///     \li SW_ANDN - And not (relevant only for VPU).
#endif
///
/// @{
float s_f32_and(float a, float b, int switches=0, float income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_bf16_and(bf16 a, bf16 b, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_f16_and(half a, half b, int switches=0, half income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat s_f8_and(minifloat a, minifloat b, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_h8_and(minihalf a, minihalf b, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
#endif
int32_t s_i32_and(int32_t a, int32_t b, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_u32_and(uint32_t a, uint32_t b, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_i16_and(int16_t a, int16_t b, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_u16_and(uint16_t a, uint16_t b, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_i8_and(int8_t a, int8_t b, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u8_and(uint8_t a, uint8_t b, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
bool s_i1_and(bool a, bool b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
float64 v_f32_and_vb(float64 a, float64 b, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_and_b(float64 a, float64 b, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_and_vb(bfloat128 a, bfloat128 b, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_and_b(bfloat128 a, bfloat128 b, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_f16_and_vb(half128 a, half128 b, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_and_b(half128 a, half128 b, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_and_vb(minifloat256 a, minifloat256 b, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_and_b(minifloat256 a, minifloat256 b, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_and_vb(minihalf256 a, minihalf256 b, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_and_b(minihalf256 a, minihalf256 b, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
int64 v_i32_and_vb(int64 a, int64 b, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_and_b(int64 a, int64 b, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_and_vb(uint64 a, uint64 b, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_and_b(uint64 a, uint64 b, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_and_vb(short128 a, short128 b, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_and_b(short128 a, short128 b, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_and_vb(ushort128 a, ushort128 b, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_and_b(ushort128 a, ushort128 b, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_and_vb(char256 a, char256 b, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_and_b(char256 a, char256 b, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_and_vb(uchar256 a, uchar256 b, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_and_b(uchar256 a, uchar256 b, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_and_b(bool256 a, bool256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_and_vb(bool256 a, bool256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
/// @}

/// @section sec11 IRF
/// @subsection subsec11_7 And
/// @brief Represents AND instruction for int5, performs bitwise and.
///
/// @param a           The first SRC operand to AND (SRC1).
/// @param b           The second SRC operand to AND (SRC2).
/// @param dimmask     Selects IRF lanes participated in the operation.
/// @param switches    Switches of AND instructions.
/// @param income      This value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of AND operation between \p a and \p b.
///
/// @{
int5 i_i32_and(int5 a, int5 b, const int dimmask, int switches, int5 income, bool predicate=1, bool polarity=0);
/// @}


//
// ------ ASH
//

/// @section sec2 Arithmetic
/// @subsection subsec2_6 Arithmetic Shift
/// @brief Represents ASH instruction.
///
/// @param a           The value to shift (SRC1).
/// @param b           The number of bits to shift (SRC2).
/// @param switches    Switches of ASH instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation - \c DEST=a*(2^b).
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturate (should always be set, left shift saturation is enabled by default).
///     \li SW_RHU - Round half up.
///
/// @{
int32_t s_i32_ash(int32_t a, int8_t b, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_u32_ash(uint32_t a, int8_t b, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_i16_ash(int16_t a, int8_t b, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_u16_ash(uint16_t a, int8_t b, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_i8_ash(int8_t a, int8_t b, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u8_ash(uint8_t a, int8_t b, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
int64 v_i32_ash_b(int64 a, char256 b, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_ash_vb(int64 a, char256 b, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ash_b(uint64 a, char256 b, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ash_vb(uint64 a, char256 b, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_ash_b(short128 a, char256 b, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_ash_vb(short128 a, char256 b, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_ash_b(ushort128 a, char256 b, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_ash_vb(ushort128 a, char256 b, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_ash_b(char256 a, char256 b, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_ash_vb(char256 a, char256 b, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_ash_b(uchar256 a, char256 b, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_ash_vb(uchar256 a, char256 b, int switches, uchar256 income, bool256 predicate, bool polarity=0);
/// @}

#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_7 Arithmetic shift with RHAZ_RS switch
/// @brief Represents ASH instruction with RHAZ_RS switch.
///
///
/// @param a           The value to shift (SRC1).
/// @param b           The number of bits to shift (SRC2).
/// @param switches    Switches of ASH instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation - \c DEST=Round(a*(2^b), RHAZ, R_BIT, S_BIT).
///
/// Performs round half away from zero, using R/S discarded bits indication from MUL instruction. \n
/// The R/S bits are coming from another Implicit source (Src1+1). Relevant only if Src2<0.
///
/// Allowed switches are:
///     - SW_SAT - Saturate (should always be set, left shift saturation is enabled by default).
///
/// @{
int64 v_i32_ash_rhaz_b(int128 a, char256 b, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_ash_rhaz_vb(int128 a, char256 b, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ash_rhaz_b(uint128 a, char256 b, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ash_rhaz_vb(uint128 a, char256 b, int switches, uint64 income, bool64 predicate, bool polarity=0);
/// @}
#endif

//
// ------ ASO
//

/// @section sec4 Cache
/// @subsection subsec4_1 Aso
/// @brief Represents ASO instruction - Used to establish memory consistency between vector loads and vector stores.
///
/// @param switches    Switches of ASO instructions.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed switches are:
///     \li SW_INC - Increment the semaphore value once all older writes to memory are observable.
#if defined(__gaudi_plus__)
///     \li SW_DEC - Decrement the semaphore value once all older writes to memory are observable.
#endif
///     \li SW_SPU - Semaphore update is done in SPU stage.
///     \li SW_VPU - Semaphore update is done in VPU stage.
///
/// @{
void aso(int switches=0, bool predicate=1, bool polarity=0);
/// @}


//
// ------ BREV
//

#if defined(__gaudi_plus__)
/// @section sec5 Bitwise
/// @subsection subsec5_1 Brev
/// @brief Represents BREV instruction.
///
/// @param a           The source operand (SRC1).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The source operand bits in reversed order.
///
#endif
/// @{
#if defined(__gaudi_plus__)
uint32_t s_f32_brev(float a, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_bf16_brev(bf16 a, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int32_t s_i32_brev(int32_t a, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_u32_brev(uint32_t a, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_i16_brev(int16_t a, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_u16_brev(uint16_t a, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_i8_brev(int8_t a, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u8_brev(uint8_t a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
int16_t s_f16_brev(half a, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
int8_t s_f8_brev(minifloat a, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
int8_t s_h8_brev(minihalf a, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi_plus__)
uint64 v_f32_brev_b(float64 a, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_f32_brev_vb(float64 a, int switches, uint64 income, bool64 predicate, bool polarity=0);
ushort128 v_bf16_brev_b(bfloat128 a, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_bf16_brev_vb(bfloat128 a, int switches, ushort128 income, bool128 predicate, bool polarity=0);
int64 v_i32_brev_b(int64 a, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_brev_vb(int64 a, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_brev_b(uint64 a, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_brev_vb(uint64 a, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_brev_b(short128 a, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_brev_vb(short128 a, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_brev_b(ushort128 a, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_brev_vb(ushort128 a, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_brev_b(char256 a, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_brev_vb(char256 a, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_brev_b(uchar256 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_brev_vb(uchar256 a, int switches, uchar256 income, bool256 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
ushort128 v_f16_brev_vb(half128 a, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_f16_brev_b(half128 a, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
uchar256 v_f8_brev_vb(minifloat256 a, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_f8_brev_b(minifloat256 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_h8_brev_vb(minihalf256 a, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_h8_brev_b(minihalf256 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ CACHE_FLUSH
//

/// @section sec4 Cache
/// @subsection subsec4_2 Cache Flush
/// @brief Represents CACHE_FLUSH instruction - Flush the Data-cache. This instruction also invalidates the caceh after flush.
///
/// @param switches    Switches of the instructions.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
#if defined(__doron1__)
/// @switches Allowed switches are :
///     \li SW_NO_INV - do not invalidate D$ (writeback only).
#endif
/// @{
void cache_flush(int switches=0, bool predicate=1, bool polarity=0);
/// @}

//
// ------ CACHE_FLUSH_ADDR
//
/// @section sec4 Cache
/// @subsection subsec4_2 Cache Flush
/// @brief Represents CACHE_FLUSH instruction - Flush the Data-cache.
///
/// @param addr        ADRF of the specific CL.
/// @param switches    Switches of the instructions.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
#if defined(__doron1__)
/// @switches Allowed switches are:
///     \li SW_NO_INV - do not invalidate D$ (writeback only).
///     \li SW_CL     - invalidate only a specific CL (specified in ADRF)
#endif
/// @{
#if defined(__doron1__)
void cache_flush_addr(__global void *addr, int switches = 0, bool predicate = 1, bool polarity = 0);
#endif
/// @}

//
// ------ CACHE_INVALIDATE
//

/// @section sec4 Cache
/// @subsection subsec4_3 Cache Invalidate
/// @brief Represents CACHE_INVALIDATE instruction - Invalidate Data/LUT/SB cache. This instruction does not flush out the data cache.
///
/// @param switches    Switches of the instructions.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
#if defined(__greco_plus__)
/// @switches Allowed switches are:
///     \li SW_SB - Invalidate SB$
#endif
#if defined(__gaudi2_plus__)
///     \li SW_D - Invalidate D$, including reset of HW prefetcher status.
///     \li SW_LU - Invalidate LU$.
///     \li SW_RST_LU - Reset LU$ pLRU bits.
///     \li SW_RST_D_PREF - Reset D$ HW prefetcher status.
#endif
///
#if defined(__doron1__)
///     \li SW_LOG - invalidate consistency logging table and meta-data.
#endif
/// @{
void cache_invalidate(int switches=0, bool predicate=1, bool polarity=0);
/// @}

//
// ------ CACHE_INVALIDATE_ADDR
//
/// @section sec4 Cache
/// @subsection subsec4_3 Cache Invalidate
/// @brief Represents CACHE_INVALIDATE instruction - Invalidate Data/LUT/SB cache
///
/// @param addr        ADRF of the specific CL.
/// @param switches    Switches of the instructions.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
#if defined(__doron1__)
/// @switches Allowed switches are:
///     \li SW_D  - Invalidate D$, including reset of HW prefetcher status.
///     \li SW_CL - invalidate only a specific CL (specified in ADRF)
#endif
/// @{
#if defined(__doron1__)
void cache_invalidate_addr(__global void *addr, int switches = 0, bool predicate = 1, bool polarity = 0);
#endif
/// @}

//
// ------ THREAD_SYNC
//

#if defined(__doron1__)
/// @brief Represents THREAD_SYNC instruction
void thread_sync();
#endif

//
// ------ CALC_FP_SPECIAL
//

#if defined(__gaudi_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_8 Calc Fp Special
/// @brief Represents CALC_FP_SPECIAL instruction.
///
/// @param src1        The first operand (SRC1).
/// @param src2        The second SRC operand (SRC2). \n
///                    For unary functions it is ignored and may be of any value, for instance, zero. \n
///                    \p src1 and \p src2 are 8/10bit-masks format (vector or scalar) representing the \n
///                    class of the operands - returned value of FCLASS instruction.
/// @param switches    Code of the function.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Calculate the choosen function for all special class elements. \n
///         (for non special class elements returns income).
///
/// @switches Allowed switches are:
///     - [SPECIAL_FUNC]
///         \li SW_RECIP - reciprocal - \c 1/x
///         \li SW_RSQRT - reciprocal square root of x
///         \li SW_SQRT - square root of x
///         \li SW_LOG - logarithm of x
///         \li SW_EXP - exponent - \c e^x
///         \li SW_TANH - hyperbolic tangent of x
///         \li SW_DIV - divide - \c x/y
///         \li SW_POW - power - \c x^y
///
#endif
/// @{
#if defined(__gaudi_plus__)
float s_f32_calc_fp_special(float src1, float src2, int switches, float income, bool predicate=1, bool polarity=0);
bf16 s_bf16_calc_fp_special(bf16 src1, bf16 src2, int switches, bf16 income, bool predicate=1, bool polarity=0);
float64 v_f32_calc_fp_special_b(float64 src1, float64 src2, int switches, float64 income, bool predicate=1, bool polarity=0);
float64 v_f32_calc_fp_special_vb(float64 src1, float64 src2, int switches, float64 income, bool64 predicate, bool polarity=0);
bfloat128 v_bf16_calc_fp_special_b(bfloat128 src1, bfloat128 src2, int switches, bfloat128 income, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_calc_fp_special_vb(bfloat128 src1, bfloat128 src2, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_f16_calc_fp_special(half src1, half src2, int switches, half income, bool predicate=1, bool polarity=0);
half128 v_f16_calc_fp_special_b(half128 src1, half128 src2, int switches, half128 income, bool predicate=1, bool polarity=0);
half128 v_f16_calc_fp_special_vb(half128 src1, half128 src2, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
/// @}


//
// ------ CMP_EQ
//

/// @section sec3 Logical
/// @subsection subsec3_1 Compare Equal
/// @brief Represents CMP_EQ instruction.
///
/// @param a           The first SRC operand (SRC1).
/// @param b           The second SRC operand (SRC2).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Boolean result of comparison - \c a==b (scalar or vector).
///
#if defined(__gaudi_plus__)
/// @switches Allowed switches are:
///     \li SW_MASK_EQ_ZERO - Compare between (a & b) and 0.
#endif
///
/// @{
bool s_f32_cmp_eq(float a, float b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bool s_bf16_cmp_eq(bf16 a, bf16 b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
bool s_f16_cmp_eq(half a, half b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
bool s_f8_cmp_eq(minifloat a, minifloat b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_h8_cmp_eq(minihalf a, minihalf b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
bool s_i32_cmp_eq(int32_t a, int32_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u32_cmp_eq(uint32_t a, uint32_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_i16_cmp_eq(int16_t a, int16_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u16_cmp_eq(uint16_t a, uint16_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_i8_cmp_eq(int8_t a, int8_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u8_cmp_eq(uint8_t a, uint8_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#if defined(__doron1__)
bool s_i32_x2_cmp_eq(int32_t_pair_t a, int32_t_pair_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u32_x2_cmp_eq(uint32_t_pair_t a, uint32_t_pair_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
bool64 v_f32_cmp_eq_vb(float64 a, float64 b, int switches, bool64 income, bool64 predicate, bool polarity=0);
bool64 v_f32_cmp_eq_b(float64 a, float64 b, int switches=0, bool64 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bool128 v_bf16_cmp_eq_vb(bfloat128 a, bfloat128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_bf16_cmp_eq_b(bfloat128 a, bfloat128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
bool128 v_f16_cmp_eq_vb(half128 a, half128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_f16_cmp_eq_b(half128 a, half128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
bool256 v_f8_cmp_eq_vb(minifloat256 a, minifloat256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_f8_cmp_eq_b(minifloat256 a, minifloat256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
bool256 v_h8_cmp_eq_vb(minihalf256 a, minihalf256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_h8_cmp_eq_b(minihalf256 a, minihalf256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
#endif
bool64 v_i32_cmp_eq_vb(int64 a, int64 b, int switches, bool64 income, bool64 predicate, bool polarity=0);
bool64 v_i32_cmp_eq_b(int64 a, int64 b, int switches=0, bool64 income={}, bool predicate=1, bool polarity=0);
bool64 v_u32_cmp_eq_vb(uint64 a, uint64 b, int switches, bool64 income, bool64 predicate, bool polarity=0);
bool64 v_u32_cmp_eq_b(uint64 a, uint64 b, int switches=0, bool64 income={}, bool predicate=1, bool polarity=0);
bool128 v_i16_cmp_eq_vb(short128 a, short128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_i16_cmp_eq_b(short128 a, short128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
bool128 v_u16_cmp_eq_vb(ushort128 a, ushort128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_u16_cmp_eq_b(ushort128 a, ushort128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
bool256 v_i8_cmp_eq_vb(char256 a, char256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_i8_cmp_eq_b(char256 a, char256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
bool256 v_u8_cmp_eq_vb(uchar256 a, uchar256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_u8_cmp_eq_b(uchar256 a, uchar256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
/// @}

#if defined(__greco_plus__)
/// @section sec11 IRF
/// @subsection subsec11_8 Compare Equal
/// @brief Represents CMP_EQ instruction for int5.
///
/// @param a           The first SRC operand (SRC1).
/// @param b           The second SRC operand (SRC2).
/// @param dimmask     Dimension mask.
/// @param income      This value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Boolean result of comparison - \c a==b.
///
#endif
/// @{
#if defined(__greco_plus__)
bool i_i32_cmp_eq(int5 a, int5 b, int dimmask, bool income, bool predicate=1, bool polarity=0);
#endif
/// @}

//
// ------ CMP_GEQ
//

/// @section sec3 Logical
/// @subsection subsec3_3 Compare Greater Equal
/// @brief Represents CMP_GEQ instruction.
///
/// @param a           The first SRC operand (SRC1).
/// @param b           The second SRC operand (SRC2).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Boolean result of comparison - \c a>=b (scalar or vector).
///
/// @{
bool s_f32_cmp_geq(float a, float b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bool s_bf16_cmp_geq(bf16 a, bf16 b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
bool s_f16_cmp_geq(half a, half b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
bool s_f8_cmp_geq(minifloat a, minifloat b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_h8_cmp_geq(minihalf a, minihalf b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
bool s_i32_cmp_geq(int32_t a, int32_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u32_cmp_geq(uint32_t a, uint32_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_i16_cmp_geq(int16_t a, int16_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u16_cmp_geq(uint16_t a, uint16_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_i8_cmp_geq(int8_t a, int8_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u8_cmp_geq(uint8_t a, uint8_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#if defined(__doron1__)
bool s_i32_x2_cmp_geq(int32_t_pair_t a, int32_t_pair_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u32_x2_cmp_geq(uint32_t_pair_t a, uint32_t_pair_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
bool64 v_f32_cmp_geq_vb(float64 a, float64 b, int switches, bool64 income, bool64 predicate, bool polarity=0);
bool64 v_f32_cmp_geq_b(float64 a, float64 b, int switches=0, bool64 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bool128 v_bf16_cmp_geq_vb(bfloat128 a, bfloat128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_bf16_cmp_geq_b(bfloat128 a, bfloat128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
bool128 v_f16_cmp_geq_vb(half128 a, half128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_f16_cmp_geq_b(half128 a, half128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
bool256 v_f8_cmp_geq_vb(minifloat256 a, minifloat256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_f8_cmp_geq_b(minifloat256 a, minifloat256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
bool256 v_h8_cmp_geq_vb(minihalf256 a, minihalf256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_h8_cmp_geq_b(minihalf256 a, minihalf256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
#endif
bool64 v_i32_cmp_geq_vb(int64 a, int64 b, int switches, bool64 income, bool64 predicate, bool polarity=0);
bool64 v_i32_cmp_geq_b(int64 a, int64 b, int switches=0, bool64 income={}, bool predicate=1, bool polarity=0);
bool64 v_u32_cmp_geq_vb(uint64 a, uint64 b, int switches, bool64 income, bool64 predicate, bool polarity=0);
bool64 v_u32_cmp_geq_b(uint64 a, uint64 b, int switches=0, bool64 income={}, bool predicate=1, bool polarity=0);
bool128 v_i16_cmp_geq_vb(short128 a, short128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_i16_cmp_geq_b(short128 a, short128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
bool128 v_u16_cmp_geq_vb(ushort128 a, ushort128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_u16_cmp_geq_b(ushort128 a, ushort128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
bool256 v_i8_cmp_geq_vb(char256 a, char256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_i8_cmp_geq_b(char256 a, char256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
bool256 v_u8_cmp_geq_vb(uchar256 a, uchar256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_u8_cmp_geq_b(uchar256 a, uchar256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
/// @}

#if defined(__greco_plus__)
/// @section sec11 IRF
/// @subsection subsec11_9 Compare Greater Equal
/// @brief Represents CMP_GEQ instruction for int5.
///
/// @param a           The first SRC operand (SRC1).
/// @param b           The second SRC operand (SRC2).
/// @param dimmask     Dimension mask.
/// @param income      This value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Boolean result of comparison - \c a>=b.
///
#endif
/// @{
#if defined(__greco_plus__)
bool i_i32_cmp_geq(int5 a, int5 b, int dimmask, bool income, bool predicate=1, bool polarity=0);
#endif
/// @}

//
// ------ CMP_GRT
//

/// @section sec3 Logical
/// @subsection subsec3_5 Compare Greater
/// @brief Represents CMP_GRT instruction.
///
/// @param a           The first SRC operand (SRC1).
/// @param b           The second SRC operand (SRC2).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Boolean result of comparison - \c a>b (scalar or vector).
///
/// @{
bool s_f32_cmp_grt(float a, float b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bool s_bf16_cmp_grt(bf16 a, bf16 b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
bool s_f16_cmp_grt(half a, half b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
bool s_f8_cmp_grt(minifloat a, minifloat b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_h8_cmp_grt(minihalf a, minihalf b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
bool s_i32_cmp_grt(int32_t a, int32_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u32_cmp_grt(uint32_t a, uint32_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_i16_cmp_grt(int16_t a, int16_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u16_cmp_grt(uint16_t a, uint16_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_i8_cmp_grt(int8_t a, int8_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u8_cmp_grt(uint8_t a, uint8_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#if defined(__doron1__)
bool s_i32_x2_cmp_grt(int32_t_pair_t a, int32_t_pair_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u32_x2_cmp_grt(uint32_t_pair_t a, uint32_t_pair_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
bool64 v_f32_cmp_grt_vb(float64 a, float64 b, int switches, bool64 income, bool64 predicate, bool polarity=0);
bool64 v_f32_cmp_grt_b(float64 a, float64 b, int switches=0, bool64 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bool128 v_bf16_cmp_grt_vb(bfloat128 a, bfloat128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_bf16_cmp_grt_b(bfloat128 a, bfloat128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
bool128 v_f16_cmp_grt_vb(half128 a, half128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_f16_cmp_grt_b(half128 a, half128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
bool256 v_f8_cmp_grt_vb(minifloat256 a, minifloat256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_f8_cmp_grt_b(minifloat256 a, minifloat256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
bool256 v_h8_cmp_grt_vb(minihalf256 a, minihalf256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_h8_cmp_grt_b(minihalf256 a, minihalf256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
#endif
bool64 v_i32_cmp_grt_vb(int64 a, int64 b, int switches, bool64 income, bool64 predicate, bool polarity=0);
bool64 v_i32_cmp_grt_b(int64 a, int64 b, int switches=0, bool64 income={}, bool predicate=1, bool polarity=0);
bool64 v_u32_cmp_grt_vb(uint64 a, uint64 b, int switches, bool64 income, bool64 predicate, bool polarity=0);
bool64 v_u32_cmp_grt_b(uint64 a, uint64 b, int switches=0, bool64 income={}, bool predicate=1, bool polarity=0);
bool128 v_i16_cmp_grt_vb(short128 a, short128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_i16_cmp_grt_b(short128 a, short128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
bool128 v_u16_cmp_grt_vb(ushort128 a, ushort128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_u16_cmp_grt_b(ushort128 a, ushort128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
bool256 v_i8_cmp_grt_vb(char256 a, char256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_i8_cmp_grt_b(char256 a, char256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
bool256 v_u8_cmp_grt_vb(uchar256 a, uchar256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_u8_cmp_grt_b(uchar256 a, uchar256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
/// @}

#if defined(__greco_plus__)
/// @section sec11 Logical
/// @subsection subsec11_10 Compare Greater
/// @brief Represents CMP_GRT instruction for int5.
///
/// @param a           The first SRC operand (SRC1).
/// @param b           The second SRC operand (SRC2).
/// @param dimmask     Dimension mask.
/// @param income      This value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Boolean result of comparison - \c a>b.
///
#endif
/// @{
#if defined(__greco_plus__)
bool i_i32_cmp_grt(int5 a, int5 b, int dimmask, bool income, bool predicate=1, bool polarity=0);
#endif
/// @}

//
// ------ CMP_LEQ
//

/// @section sec3 Logical
/// @subsection subsec3_7 Compare Less Equal
/// @brief Represents CMP_LEQ instruction.
///
/// @param a           The first SRC operand (SRC1).
/// @param b           The second SRC operand (SRC2).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Boolean result of comparison - \c a<=b (scalar or vector).
///
/// @{
bool s_f32_cmp_leq(float a, float b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bool s_bf16_cmp_leq(bf16 a, bf16 b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
bool s_f16_cmp_leq(half a, half b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
bool s_f8_cmp_leq(minifloat a, minifloat b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_h8_cmp_leq(minihalf a, minihalf b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif

bool s_i32_cmp_leq(int32_t a, int32_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u32_cmp_leq(uint32_t a, uint32_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_i16_cmp_leq(int16_t a, int16_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u16_cmp_leq(uint16_t a, uint16_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_i8_cmp_leq(int8_t a, int8_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u8_cmp_leq(uint8_t a, uint8_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#if defined(__doron1__)
bool s_i32_x2_cmp_leq(int32_t_pair_t a, int32_t_pair_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u32_x2_cmp_leq(uint32_t_pair_t a, uint32_t_pair_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
bool64 v_f32_cmp_leq_vb(float64 a, float64 b, int switches, bool64 income, bool64 predicate, bool polarity=0);
bool64 v_f32_cmp_leq_b(float64 a, float64 b, int switches=0, bool64 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bool128 v_bf16_cmp_leq_vb(bfloat128 a, bfloat128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_bf16_cmp_leq_b(bfloat128 a, bfloat128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
bool128 v_f16_cmp_leq_vb(half128 a, half128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_f16_cmp_leq_b(half128 a, half128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
bool256 v_f8_cmp_leq_vb(minifloat256 a, minifloat256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_f8_cmp_leq_b(minifloat256 a, minifloat256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
bool256 v_h8_cmp_leq_vb(minihalf256 a, minihalf256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_h8_cmp_leq_b(minihalf256 a, minihalf256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
#endif
bool64 v_i32_cmp_leq_vb(int64 a, int64 b, int switches, bool64 income, bool64 predicate, bool polarity=0);
bool64 v_i32_cmp_leq_b(int64 a, int64 b, int switches=0, bool64 income={}, bool predicate=1, bool polarity=0);
bool64 v_u32_cmp_leq_vb(uint64 a, uint64 b, int switches, bool64 income, bool64 predicate, bool polarity=0);
bool64 v_u32_cmp_leq_b(uint64 a, uint64 b, int switches=0, bool64 income={}, bool predicate=1, bool polarity=0);
bool128 v_i16_cmp_leq_vb(short128 a, short128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_i16_cmp_leq_b(short128 a, short128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
bool128 v_u16_cmp_leq_vb(ushort128 a, ushort128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_u16_cmp_leq_b(ushort128 a, ushort128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
bool256 v_i8_cmp_leq_vb(char256 a, char256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_i8_cmp_leq_b(char256 a, char256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
bool256 v_u8_cmp_leq_vb(uchar256 a, uchar256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_u8_cmp_leq_b(uchar256 a, uchar256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
/// @}

#if defined(__greco_plus__)
/// @section sec11 Logical
/// @subsection subsec11_11 Compare Less Equal
/// @brief Represents CMP_LEQ instruction for int5.
///
/// @param a           The first SRC operand (SRC1).
/// @param b           The second SRC operand (SRC2).
/// @param dimmask     Dimension mask.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Boolean result of comparison - \c a<=b.
///
#endif
/// @{
#if defined(__greco_plus__)
bool i_i32_cmp_leq(int5 a, int5 b, int dimmask, bool income, bool predicate=1, bool polarity=0);
#endif
/// @}

//
// ------ CMP_LESS
//

/// @section sec3 Logical
/// @subsection subsec3_9 Compare Less
/// @brief Represents CMP_LESS instruction.
///
/// @param a           The first SRC operand (SRC1).
/// @param b           The second SRC operand (SRC2).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Boolean result of comparison - \c a<b (scalar or vector).
///
/// @{
bool s_f32_cmp_less(float a, float b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bool s_bf16_cmp_less(bf16 a, bf16 b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
bool s_f16_cmp_less(half a, half b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
bool s_f8_cmp_less(minifloat a, minifloat b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_h8_cmp_less(minihalf a, minihalf b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
bool s_i32_cmp_less(int32_t a, int32_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u32_cmp_less(uint32_t a, uint32_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_i16_cmp_less(int16_t a, int16_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u16_cmp_less(uint16_t a, uint16_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_i8_cmp_less(int8_t a, int8_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u8_cmp_less(uint8_t a, uint8_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#if defined(__doron1__)
bool s_i32_x2_cmp_less(int32_t_pair_t a, int32_t_pair_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u32_x2_cmp_less(uint32_t_pair_t a, uint32_t_pair_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
bool64 v_f32_cmp_less_vb(float64 a, float64 b, int switches, bool64 income, bool64 predicate, bool polarity=0);
bool64 v_f32_cmp_less_b(float64 a, float64 b, int switches=0, bool64 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bool128 v_bf16_cmp_less_vb(bfloat128 a, bfloat128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_bf16_cmp_less_b(bfloat128 a, bfloat128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
bool128 v_f16_cmp_less_vb(half128 a, half128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_f16_cmp_less_b(half128 a, half128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
bool256 v_f8_cmp_less_vb(minifloat256 a, minifloat256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_f8_cmp_less_b(minifloat256 a, minifloat256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
bool256 v_h8_cmp_less_vb(minihalf256 a, minihalf256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_h8_cmp_less_b(minihalf256 a, minihalf256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
#endif
bool64 v_i32_cmp_less_vb(int64 a, int64 b, int switches, bool64 income, bool64 predicate, bool polarity=0);
bool64 v_i32_cmp_less_b(int64 a, int64 b, int switches=0, bool64 income={}, bool predicate=1, bool polarity=0);
bool64 v_u32_cmp_less_vb(uint64 a, uint64 b, int switches, bool64 income, bool64 predicate, bool polarity=0);
bool64 v_u32_cmp_less_b(uint64 a, uint64 b, int switches=0, bool64 income={}, bool predicate=1, bool polarity=0);
bool128 v_i16_cmp_less_vb(short128 a, short128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_i16_cmp_less_b(short128 a, short128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
bool128 v_u16_cmp_less_vb(ushort128 a, ushort128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_u16_cmp_less_b(ushort128 a, ushort128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
bool256 v_i8_cmp_less_vb(char256 a, char256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_i8_cmp_less_b(char256 a, char256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
bool256 v_u8_cmp_less_vb(uchar256 a, uchar256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_u8_cmp_less_b(uchar256 a, uchar256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
/// @}

#if defined(__greco_plus__)
/// @section sec11 Logical
/// @subsection subsec11_12 Compare Less
/// @brief Represents CMP_LESS instruction for int5.
///
/// @param a           The first SRC operand (SRC1).
/// @param b           The second SRC operand (SRC2).
/// @param dimmask     Dimension mask.
/// @param income      This value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Boolean result of comparison - \c a<b.
///
#endif
/// @{
#if defined(__greco_plus__)
bool i_i32_cmp_less(int5 a, int5 b, int dimmask, bool income, bool predicate=1, bool polarity=0);
#endif
/// @}

//
// ------ CMP_NEQ
//

/// @section sec3 Logical
/// @subsection subsec3_11 Compare Not Equal
/// @brief Represents CMP_NEQ instruction.
///
/// @param a           The first SRC operand (SRC1).
/// @param b           The second SRC operand (SRC2).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Boolean result of comparison - \c a!=b (scalar or vector).
///
/// @{
bool s_f32_cmp_neq(float a, float b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bool s_bf16_cmp_neq(bf16 a, bf16 b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
bool s_f16_cmp_neq(half a, half b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
bool s_f8_cmp_neq(minifloat a, minifloat b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_h8_cmp_neq(minihalf a, minihalf b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
bool s_i32_cmp_neq(int32_t a, int32_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u32_cmp_neq(uint32_t a, uint32_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_i16_cmp_neq(int16_t a, int16_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u16_cmp_neq(uint16_t a, uint16_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_i8_cmp_neq(int8_t a, int8_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u8_cmp_neq(uint8_t a, uint8_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#if defined(__doron1__)
bool s_i32_x2_cmp_neq(int32_t_pair_t a, int32_t_pair_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
bool s_u32_x2_cmp_neq(uint32_t_pair_t a, uint32_t_pair_t b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif
bool64 v_f32_cmp_neq_vb(float64 a, float64 b, int switches, bool64 income, bool64 predicate, bool polarity=0);
bool64 v_f32_cmp_neq_b(float64 a, float64 b, int switches=0, bool64 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bool128 v_bf16_cmp_neq_vb(bfloat128 a, bfloat128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_bf16_cmp_neq_b(bfloat128 a, bfloat128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
bool128 v_f16_cmp_neq_vb(half128 a, half128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_f16_cmp_neq_b(half128 a, half128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
bool256 v_f8_cmp_neq_vb(minifloat256 a, minifloat256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_f8_cmp_neq_b(minifloat256 a, minifloat256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
bool256 v_h8_cmp_neq_vb(minihalf256 a, minihalf256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_h8_cmp_neq_b(minihalf256 a, minihalf256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
#endif
bool64 v_i32_cmp_neq_vb(int64 a, int64 b, int switches, bool64 income, bool64 predicate, bool polarity=0);
bool64 v_i32_cmp_neq_b(int64 a, int64 b, int switches=0, bool64 income={}, bool predicate=1, bool polarity=0);
bool64 v_u32_cmp_neq_vb(uint64 a, uint64 b, int switches, bool64 income, bool64 predicate, bool polarity=0);
bool64 v_u32_cmp_neq_b(uint64 a, uint64 b, int switches=0, bool64 income={}, bool predicate=1, bool polarity=0);
bool128 v_i16_cmp_neq_vb(short128 a, short128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_i16_cmp_neq_b(short128 a, short128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
bool128 v_u16_cmp_neq_vb(ushort128 a, ushort128 b, int switches, bool128 income, bool128 predicate, bool polarity=0);
bool128 v_u16_cmp_neq_b(ushort128 a, ushort128 b, int switches=0, bool128 income={}, bool predicate=1, bool polarity=0);
bool256 v_i8_cmp_neq_vb(char256 a, char256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_i8_cmp_neq_b(char256 a, char256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
bool256 v_u8_cmp_neq_vb(uchar256 a, uchar256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_u8_cmp_neq_b(uchar256 a, uchar256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
/// @}

#if defined(__greco_plus__)
/// @section sec11 Logical
/// @subsection subsec11_13 Compare Not Equal
/// @brief Represents CMP_NEQ instruction for int5.
///
/// @param a           The first SRC operand (SRC1).
/// @param b           The second SRC operand (SRC2).
/// @param dimmask     Dimension mask.
/// @param income      This value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Boolean result of comparison - \c a!=b.
///
#endif
/// @{
#if defined(__greco_plus__)
bool i_i32_cmp_neq(int5 a, int5 b, int dimmask, bool income, bool predicate=1, bool polarity=0);
#endif
/// @}

//
// ------ CONVERT
//

/// @section sec1 Convert
/// @subsection subsec1_2 Scalar Convert
/// @brief Represents CONVERT instruction in scalar slot.
///
/// @param src       The value to convert (SRC1).
/// @param switches  Switches of the instructions.
/// @param income    This value is returned if the predicate is false.
/// @param predicate Predicate value for the instruction.
/// @param polarity  True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
#if defined(__gaudi_plus__)
///         \li SW_SR - Stochastic Rounding.
///         \li SW_RHAZ - Round half away from zero.
#endif
///         \li SW_CSR - Take rounding mode from
#if defined(__gaudi__) || defined(__gaudib__) || defined(__goya__)
///                    ROUND_CSR register
#endif
#if defined(__greco_plus__)
///                    CONV_ROUND_CSR register
#endif
#if defined(__gaudi2_plus__)
///         \li SW_SR_RNE - Stochastic Rounding, but use RNE when rounding denormals.
/// \par
///     \li SW_CLIP_FP - Convert FP with clipping. \n
///         When the resulting FP value should be +/-inf, it will be clipped to +/-max_normal_value_fp
#endif
///
/// @{
int32_t s_convert_f32_to_i32(float src, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_convert_f32_to_i16(float src, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_convert_f32_to_i8(float src, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_convert_f32_to_bf16(float src, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_convert_f32_to_f16(float src, int switches=0, half income={}, bool predicate=1, bool polarity=0);
half s_convert_bf16_to_f16(bf16 src, int switches=0, half income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
uint32_t s_convert_f32_to_u32(float src, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_convert_f32_to_u16(float src, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_convert_f32_to_u8(float src, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
minifloat s_convert_f32_to_f8(float src, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_convert_f32_to_h8(float src, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi_plus__)
float s_convert_bf16_to_f32(bf16 src, int switches=0, float income={}, bool predicate=1, bool polarity=0);
int16_t s_convert_bf16_to_i16(bf16 src, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
int8_t s_convert_bf16_to_i8(bf16 src, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
int32_t s_convert_bf16_to_i32(bf16 src, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_convert_bf16_to_u32(bf16 src, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_convert_bf16_to_u16(bf16 src, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_convert_bf16_to_u8(bf16 src, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
minifloat s_convert_bf16_to_f8(bf16 src, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_convert_bf16_to_h8(bf16 src, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
float s_convert_f16_to_f32(half src, int switches=0, float income={}, bool predicate=1, bool polarity=0);
bf16 s_convert_f16_to_bf16(half src, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
int16_t s_convert_f16_to_i16(half src, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_convert_f16_to_i8(half src, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
int32_t s_convert_f16_to_i32(half src, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_convert_f16_to_u32(half src, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_convert_f16_to_u16(half src, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_convert_f16_to_u8(half src, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
minifloat s_convert_f16_to_f8(half src, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_convert_f16_to_h8(half src, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
float s_convert_f8_to_f32(minifloat src, int switches=0, float income={}, bool predicate=1, bool polarity=0);
bf16 s_convert_f8_to_bf16(minifloat src, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
half s_convert_f8_to_f16(minifloat src, int switches=0, half income={}, bool predicate=1, bool polarity=0);
int32_t s_convert_f8_to_i32(minifloat src, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_convert_f8_to_u32(minifloat src, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_convert_f8_to_i16(minifloat src, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_convert_f8_to_u16(minifloat src, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_convert_f8_to_i8(minifloat src, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_convert_f8_to_u8(minifloat src, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
minihalf s_convert_f8_to_h8(minifloat src, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
float s_convert_h8_to_f32(minihalf src, int switches=0, float income={}, bool predicate=1, bool polarity=0);
bf16 s_convert_h8_to_bf16(minihalf src, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
half s_convert_h8_to_f16(minihalf src, int switches=0, half income={}, bool predicate=1, bool polarity=0);
int32_t s_convert_h8_to_i32(minihalf src, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_convert_h8_to_u32(minihalf src, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_convert_h8_to_i16(minihalf src, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_convert_h8_to_u16(minihalf src, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_convert_h8_to_i8(minihalf src, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_convert_h8_to_u8(minihalf src, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
minifloat s_convert_h8_to_f8(minihalf src, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
#endif
float s_convert_i32_to_f32(int32_t src, int switches=0, float income={}, bool predicate=1, bool polarity=0);
uint32_t s_convert_i32_to_u32(int32_t src, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
#if defined(__goya__) || defined(__greco_plus__)
uint8_t s_convert_i32_to_u8(int32_t src, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi_plus__)
bf16 s_convert_i32_to_bf16(int32_t src, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_convert_i32_to_f16(int32_t src, int switches=0, half income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
int16_t s_convert_i32_to_i16(int32_t src, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_convert_i32_to_u16(int32_t src, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_convert_i32_to_i8(int32_t src, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
minifloat s_convert_i32_to_f8(int32_t src, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_convert_i32_to_h8(int32_t src, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
float s_convert_u32_to_f32(uint32_t src, int switches=0, float income={}, bool predicate=1, bool polarity=0);
bf16 s_convert_u32_to_bf16(uint32_t src, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
half s_convert_u32_to_f16(uint32_t src, int switches=0, half income={}, bool predicate=1, bool polarity=0);
int32_t s_convert_u32_to_i32(uint32_t src, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_convert_u32_to_i16(uint32_t src, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_convert_u32_to_u16(uint32_t src, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_convert_u32_to_i8(uint32_t src, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_convert_u32_to_u8(uint32_t src, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
minifloat s_convert_u32_to_f8(uint32_t src, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_convert_u32_to_h8(uint32_t src, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
#endif
float s_convert_i16_to_f32(int16_t src, int switches=0, float income={}, bool predicate=1, bool polarity=0);
int32_t s_convert_i16_to_i32(int16_t src, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_convert_i16_to_u32(int16_t src, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_convert_i16_to_u16(int16_t src, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_convert_i16_to_u8(int16_t src, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_convert_i16_to_bf16(int16_t src, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_convert_i16_to_f16(int16_t src, int switches=0, half income={}, bool predicate=1, bool polarity=0);
half s_convert_i8_to_f16(uint8_t src, int switches=0, half income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
int8_t s_convert_i16_to_i8(int16_t src, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
minifloat s_convert_i16_to_f8(int16_t src, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_convert_i16_to_h8(int16_t src, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
#endif
float s_convert_i8_to_f32(int8_t src, int switches=0, float income={}, bool predicate=1, bool polarity=0);
int32_t s_convert_i8_to_i32(int8_t src, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_convert_i8_to_u32(int8_t src, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_convert_i8_to_i16(int8_t src, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_convert_i8_to_u16(int8_t src, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_convert_i8_to_u8(int8_t src, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_convert_i8_to_bf16(uint8_t src, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat s_convert_i8_to_f8(int8_t src, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_convert_i8_to_h8(int8_t src, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi_plus__)
bf16 s_convert_u16_to_bf16(uint16_t src, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_convert_u16_to_f16(uint16_t src, int switches=0, half income={}, bool predicate=1, bool polarity=0);
half s_convert_u8_to_f16(uint8_t src, int switches=0, half income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
int32_t s_convert_u16_to_i32(uint16_t src, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_convert_u16_to_u32(uint16_t src, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_convert_u16_to_i16(uint16_t src, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_convert_u16_to_i8(uint16_t src, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_convert_u16_to_u8(uint16_t src, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
float s_convert_u16_to_f32(uint16_t src, int switches=0, float income={}, bool predicate=1, bool polarity=0);
minifloat s_convert_u16_to_f8(uint16_t src, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_convert_u16_to_h8(uint16_t src, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
float s_convert_u8_to_f32(uint8_t src, int switches=0, float income={}, bool predicate=1, bool polarity=0);
bf16 s_convert_u8_to_bf16(uint8_t src, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
int32_t s_convert_u8_to_i32(uint8_t src, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_convert_u8_to_i16(uint8_t src, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_convert_u8_to_u32(uint8_t src, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_convert_u8_to_u16(uint8_t src, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_convert_i4_to_i8(int4_t src, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_convert_u4_to_u8(uint4_t src, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
int32_t s_convert_i64_to_i32(int32_t src1, int32_t src2, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
int8_t s_convert_u8_to_i8(uint8_t src, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
minifloat s_convert_u8_to_f8(uint8_t src, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_convert_u8_to_h8(uint8_t src, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


/// @section sec1 Convert
/// @subsection subsec1_3 Vector Convert
/// @brief Represents CONVERT instruction in vector slot, in which the result \n
/// vector element is of the same size as the source one.
///
/// @param src       The value to convert (SRC1).
/// @param switches  Switches of the instructions.
/// @param income    This value is returned if the predicate is false.
/// @param predicate Predicate value for the instruction.
/// @param polarity  True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
#if defined(__gaudi_plus__)
///         \li SW_SR - Stochastic Rounding.
///         \li SW_RHAZ - Round half away from zero.
#endif
///         \li SW_CSR - Take rounding mode from
#if defined(__gaudi__) || defined(__gaudib__) || defined(__goya__)
///                    ROUND_CSR register
#endif
#if defined(__greco_plus__)
///                    CONV_ROUND_CSR register
#endif
#if defined(__gaudi2_plus__)
///         \li SW_SR_RNE - Stochastic Rounding, but use RNE when rounding denormals.
/// \par
///     \li SW_CLIP_FP - Convert FP with clipping. \n
///         When the resulting FP value should be +/-inf, it will be clipped to +/-max_normal_value_fp
#endif
///
/// @{
int64 v_convert_f32_to_i32_b(float64 src, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_convert_f32_to_i32_vb(float64 src, int switches, int64 income, bool64 predicate, bool polarity=0);
#if defined(__gaudi2_plus__)
uint64 v_convert_f32_to_u32_b(float64 src, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_convert_f32_to_u32_vb(float64 src, int switches, uint64 income, bool64 predicate, bool polarity=0);
#endif
#if defined(__gaudi_plus__)
short128 v_convert_bf16_to_i16_b(bfloat128 src, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_convert_bf16_to_i16_vb(bfloat128 src, int switches, short128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_convert_bf16_to_f16_b(bfloat128 src, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_convert_bf16_to_f16_vb(bfloat128 src, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
ushort128 v_convert_bf16_to_u16_b(bfloat128 src, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_convert_bf16_to_u16_vb(bfloat128 src, int switches, ushort128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
bfloat128 v_convert_f16_to_bf16_b(half128 src, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_convert_f16_to_bf16_vb(half128 src, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
short128 v_convert_f16_to_i16_b(half128 src, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_convert_f16_to_i16_vb(half128 src, int switches, short128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
ushort128 v_convert_f16_to_u16_b(half128 src, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_convert_f16_to_u16_vb(half128 src, int switches, ushort128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
char256 v_convert_f8_to_i8_b(minifloat256 src, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_convert_f8_to_i8_vb(minifloat256 src, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_convert_h8_to_i8_b(minihalf256 src, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_convert_h8_to_i8_vb(minihalf256 src, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_convert_f8_to_u8_b(minifloat256 src, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_convert_f8_to_u8_vb(minifloat256 src, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_convert_h8_to_u8_b(minihalf256 src, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_convert_h8_to_u8_vb(minihalf256 src, int switches, uchar256 income, bool256 predicate, bool polarity=0);
minihalf256 v_convert_f8_to_h8_b(minifloat256 src, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_convert_f8_to_h8_vb(minifloat256 src, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minifloat256 v_convert_h8_to_f8_b(minihalf256 src, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_convert_h8_to_f8_vb(minihalf256 src, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
#endif
float64 v_convert_i32_to_f32_b(int64 src, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_convert_i32_to_f32_vb(int64 src, int switches, float64 income, bool64 predicate, bool polarity=0);
uint64 v_convert_i32_to_u32_b(int64 src, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_convert_i32_to_u32_vb(int64 src, int switches, uint64 income, bool64 predicate, bool polarity=0);
#if defined(__gaudi2_plus__)
float64 v_convert_u32_to_f32_b(uint64 src, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_convert_u32_to_f32_vb(uint64 src, int switches, float64 income, bool64 predicate, bool polarity=0);
int64 v_convert_u32_to_i32_b(uint64 src, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_convert_u32_to_i32_vb(uint64 src, int switches, int64 income, bool64 predicate, bool polarity=0);
#endif
ushort128 v_convert_i16_to_u16_b(short128 src, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_convert_i16_to_u16_vb(short128 src, int switches, ushort128 income, bool128 predicate, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_convert_i16_to_bf16_b(short128 src, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_convert_i16_to_bf16_vb(short128 src, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_convert_i16_to_f16_b(short128 src, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_convert_i16_to_f16_vb(short128 src, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi_plus__)
bfloat128 v_convert_u16_to_bf16_b(ushort128 src, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_convert_u16_to_bf16_vb(ushort128 src, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_convert_u16_to_f16_b(ushort128 src, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_convert_u16_to_f16_vb(ushort128 src, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
short128 v_convert_u16_to_i16_b(ushort128 src, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_convert_u16_to_i16_vb(ushort128 src, int switches, short128 income, bool128 predicate, bool polarity=0);
#endif
uchar256 v_convert_i8_to_u8_b(char256 src, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_convert_i8_to_u8_vb(char256 src, int switches, uchar256 income, bool256 predicate, bool polarity=0);
#if defined(__gaudi2_plus__)
minifloat256 v_convert_i8_to_f8_b(char256 src, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_convert_i8_to_f8_vb(char256 src, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_convert_i8_to_h8_b(char256 src, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_convert_i8_to_h8_vb(char256 src, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
char256 v_convert_u8_to_i8_b(uchar256 src, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_convert_u8_to_i8_vb(uchar256 src, int switches, char256 income, bool256 predicate, bool polarity=0);
minifloat256 v_convert_u8_to_f8_b(uchar256 src, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_convert_u8_to_f8_vb(uchar256 src, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_convert_u8_to_h8_b(uchar256 src, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_convert_u8_to_h8_vb(uchar256 src, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


/// @section sec1 Convert
/// @subsection subsec1_4 Upconvert 
/// @brief Represents CONVERT instruction in vector slot in which result vector \n
/// element is wider then the source one (up-convert).
///
/// @param src       The value to convert (SRC1).
/// @param switches  Switches of the instructions.
/// @param income    This value is returned if the predicate is false.
/// @param predicate Predicate value for the instruction.
/// @param polarity  True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
#if defined(__gaudi_plus__)
///         \li SW_SR - Stochastic Rounding.
///         \li SW_RHAZ - Round half away from zero.
#endif
///         \li SW_CSR - Take rounding mode from
#if defined(__gaudi__) || defined(__gaudib__) || defined(__goya__)
///                    ROUND_CSR register
#endif
#if defined(__greco_plus__)
///                    CONV_ROUND_CSR register
#endif
#if defined(__gaudi2_plus__)
///         \li SW_SR_RNE - Stochastic Rounding, but use RNE when rounding denormals.
/// \par
///     \li SW_CLIP_FP - Convert FP with clipping. \n
///         When the resulting FP value should be +/-inf, it will be clipped to +/-max_normal_value_fp
#endif
///
/// @{
float64 v_convert_i16_to_f32_b(short128 src, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_convert_i16_to_f32_vb(short128 src, int switches, float64 income, bool64 predicate, bool polarity=0);
int64 v_convert_i16_to_i32_b(short128 src, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_convert_i16_to_i32_vb(short128 src, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_convert_i16_to_u32_b(short128 src, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_convert_i16_to_u32_vb(short128 src, int switches, uint64 income, bool64 predicate, bool polarity=0);
#if defined(__gaudi2_plus__)
int64 v_convert_u16_to_i32_b(ushort128 src, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_convert_u16_to_i32_vb(ushort128 src, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_convert_u16_to_u32_b(ushort128 src, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_convert_u16_to_u32_vb(ushort128 src, int switches, uint64 income, bool64 predicate, bool polarity=0);
float64 v_convert_u16_to_f32_b(ushort128 src, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_convert_u16_to_f32_vb(ushort128 src, int switches, float64 income, bool64 predicate, bool polarity=0);
#endif
float64 v_convert_i8_to_f32_b(char256 src, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_convert_i8_to_f32_vb(char256 src, int switches, float64 income, bool64 predicate, bool polarity=0);
int64 v_convert_i8_to_i32_b(char256 src, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_convert_i8_to_i32_vb(char256 src, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_convert_i8_to_u32_b(char256 src, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_convert_i8_to_u32_vb(char256 src, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_convert_i8_to_i16_b(char256 src, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_convert_i8_to_i16_vb(char256 src, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_convert_i8_to_u16_b(char256 src, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_convert_i8_to_u16_vb(char256 src, int switches, ushort128 income, bool128 predicate, bool polarity=0);
#if defined(__gaudib__) || defined(__greco_plus__)
float64 v_convert_bf16_to_f32_b(bfloat128 src, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_convert_bf16_to_f32_vb(bfloat128 src, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_convert_f16_to_f32_b(half128 src, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_convert_f16_to_f32_vb(half128 src, int switches, float64 income, bool64 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
bfloat128 v_convert_i8_to_bf16_b(char256 src, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_convert_i8_to_bf16_vb(char256 src, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
half128 v_convert_i8_to_f16_b(char256 src, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_convert_i8_to_f16_vb(char256 src, int switches, half128 income, bool128 predicate, bool polarity=0);
float64 v_convert_u8_to_f32_b(uchar256 src, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_convert_u8_to_f32_vb(uchar256 src, int switches, float64 income, bool64 predicate, bool polarity=0);
bfloat128 v_convert_u8_to_bf16_b(uchar256 src, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_convert_u8_to_bf16_vb(uchar256 src, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
int64 v_convert_u8_to_i32_b(uchar256 src, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_convert_u8_to_i32_vb(uchar256 src, int switches, int64 income, bool64 predicate, bool polarity=0);
short128 v_convert_u8_to_i16_b(uchar256 src, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_convert_u8_to_i16_vb(uchar256 src, int switches, short128 income, bool128 predicate, bool polarity=0);
uint64 v_convert_u8_to_u32_b(uchar256 src, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_convert_u8_to_u32_vb(uchar256 src, int switches, uint64 income, bool64 predicate, bool polarity=0);
ushort128 v_convert_u8_to_u16_b(uchar256 src, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_convert_u8_to_u16_vb(uchar256 src, int switches, ushort128 income, bool128 predicate, bool polarity=0);
half128 v_convert_u8_to_f16_b(uchar256 src, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_convert_u8_to_f16_vb(uchar256 src, int switches, half128 income, bool128 predicate, bool polarity=0);
char256 v_convert_i4_to_i8_b(nibble512 src, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_convert_i4_to_i8_vb(nibble512 src, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_convert_u4_to_u8_b(unibble512 src, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_convert_u4_to_u8_vb(unibble512 src, int switches, uchar256 income, bool256 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
int64 v_convert_bf16_to_i32_b(bfloat128 src, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_convert_bf16_to_i32_vb(bfloat128 src, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_convert_bf16_to_u32_b(bfloat128 src, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_convert_bf16_to_u32_vb(bfloat128 src, int switches, uint64 income, bool64 predicate, bool polarity=0);
int64 v_convert_f16_to_i32_b(half128 src, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_convert_f16_to_i32_vb(half128 src, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_convert_f16_to_u32_b(half128 src, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_convert_f16_to_u32_vb(half128 src, int switches, uint64 income, bool64 predicate, bool polarity=0);
float64 v_convert_f8_to_f32_b(minifloat256 src, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_convert_f8_to_f32_vb(minifloat256 src, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_convert_h8_to_f32_b(minihalf256 src, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_convert_h8_to_f32_vb(minihalf256 src, int switches, float64 income, bool64 predicate, bool polarity=0);
bfloat128 v_convert_f8_to_bf16_b(minifloat256 src, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_convert_f8_to_bf16_vb(minifloat256 src, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_convert_h8_to_bf16_b(minihalf256 src, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_convert_h8_to_bf16_vb(minihalf256 src, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
half128 v_convert_f8_to_f16_b(minifloat256 src, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_convert_f8_to_f16_vb(minifloat256 src, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_convert_h8_to_f16_b(minihalf256 src, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_convert_h8_to_f16_vb(minihalf256 src, int switches, half128 income, bool128 predicate, bool polarity=0);
int64 v_convert_f8_to_i32_b(minifloat256 src, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_convert_f8_to_i32_vb(minifloat256 src, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_convert_h8_to_i32_b(minihalf256 src, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_convert_h8_to_i32_vb(minihalf256 src, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_convert_f8_to_u32_b(minifloat256 src, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_convert_f8_to_u32_vb(minifloat256 src, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_convert_h8_to_u32_b(minihalf256 src, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_convert_h8_to_u32_vb(minihalf256 src, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_convert_f8_to_i16_b(minifloat256 src, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_convert_f8_to_i16_vb(minifloat256 src, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_convert_h8_to_i16_b(minihalf256 src, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_convert_h8_to_i16_vb(minihalf256 src, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_convert_f8_to_u16_b(minifloat256 src, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_convert_f8_to_u16_vb(minifloat256 src, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_convert_h8_to_u16_b(minihalf256 src, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_convert_h8_to_u16_vb(minihalf256 src, int switches, ushort128 income, bool128 predicate, bool polarity=0);
#endif
/// @}


/// @section sec1 Convert
/// @subsection subsec1_5 Down Convert
/// @brief Represents CONVERT instruction in vector slot in which result vector \n
/// element is shorter then the source one (down-convert).
///
/// @param src       The value to convert (SRC1).
/// @param lane      The lane in output vector to which result of conversion is written, (0: all even lanes, 1: all odd lanes). \n
/// For f32 to i16, only lanes 0-1 are available. For f32 to i8, all lanes 0-3 are available.
/// @param switches  Switches of the instructions.
/// @param income    This value is returned if the predicate is false.
/// @param predicate Predicate value for the instruction.
/// @param polarity  True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
#if defined(__gaudi_plus__)
///         \li SW_SR - Stochastic Rounding.
///         \li SW_RHAZ - Round half away from zero.
#endif
///         \li SW_CSR - Take rounding mode from
#if defined(__gaudi__) || defined(__gaudib__) || defined(__goya__)
///                    ROUND_CSR register
#endif
#if defined(__greco_plus__)
///                    CONV_ROUND_CSR register
#endif
#if defined(__gaudi2_plus__)
///         \li SW_SR_RNE - Stochastic Rounding, but use RNE when rounding denormals.
/// \par
///     \li SW_CLIP_FP - Convert FP with clipping. \n
///         When the resulting FP value should be +/-inf, it will be clipped to +/-max_normal_value_fp
#endif
///
/// @{
short128 v_convert_f32_to_i16_b(float64 src, const int lane, int switches, short128 income, bool predicate=1, bool polarity=0);
short128 v_convert_f32_to_i16_vb(float64 src, const int lane, int switches, short128 income, bool128 predicate, bool polarity=0);
char256 v_convert_f32_to_i8_b(float64 src, const int lane, int switches, char256 income, bool predicate=1, bool polarity=0);
char256 v_convert_f32_to_i8_vb(float64 src, const int lane, int switches, char256 income, bool256 predicate, bool polarity=0);
#if defined(__gaudib__) || defined(__greco_plus__)
bfloat128 v_convert_f32_to_bf16_b(float64 src, const int lane, int switches, bfloat128 income, bool predicate=1, bool polarity=0);
bfloat128 v_convert_f32_to_bf16_vb(float64 src, const int lane, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
half128 v_convert_f32_to_f16_b(float64 src, const int lane, int switches, half128 income, bool predicate=1, bool polarity=0);
half128 v_convert_f32_to_f16_vb(float64 src, const int lane, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
ushort128 v_convert_f32_to_u16_b(float64 src, const int lane, int switches, ushort128 income, bool predicate=1, bool polarity=0);
ushort128 v_convert_f32_to_u16_vb(float64 src, const int lane, int switches, ushort128 income, bool64 predicate, bool polarity=0);
uchar256 v_convert_f32_to_u8_b(float64 src, const int lane, int switches, uchar256 income, bool predicate=1, bool polarity=0);
uchar256 v_convert_f32_to_u8_vb(float64 src, const int lane, int switches, uchar256 income, bool64 predicate, bool polarity=0);
minifloat256 v_convert_f32_to_f8_b(float64 src, const int lane, int switches, minifloat256 income, bool predicate=1, bool polarity=0);
minifloat256 v_convert_f32_to_f8_vb(float64 src, const int lane, int switches, minifloat256 income, bool64 predicate, bool polarity=0);
minihalf256 v_convert_f32_to_h8_b(float64 src, const int lane, int switches, minihalf256 income, bool predicate=1, bool polarity=0);
minihalf256 v_convert_f32_to_h8_vb(float64 src, const int lane, int switches, minihalf256 income, bool64 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
char256 v_convert_bf16_to_i8_b(bfloat128 src, const int lane, int switches, char256 income, bool predicate=1, bool polarity=0);
char256 v_convert_bf16_to_i8_vb(bfloat128 src, const int lane, int switches, char256 income, bool128 predicate, bool polarity=0);
char256 v_convert_f16_to_i8_b(half128 src, const int lane, int switches, char256 income, bool predicate=1, bool polarity=0);
char256 v_convert_f16_to_i8_vb(half128 src, const int lane, int switches, char256 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
uchar256 v_convert_bf16_to_u8_b(bfloat128 src, const int lane, int switches, uchar256 income, bool predicate=1, bool polarity=0);
uchar256 v_convert_bf16_to_u8_vb(bfloat128 src, const int lane, int switches, uchar256 income, bool128 predicate, bool polarity=0);
minifloat256 v_convert_bf16_to_f8_b(bfloat128 src, const int lane, int switches, minifloat256 income, bool predicate=1, bool polarity=0);
minifloat256 v_convert_bf16_to_f8_vb(bfloat128 src, const int lane, int switches, minifloat256 income, bool128 predicate, bool polarity=0);
minihalf256 v_convert_bf16_to_h8_b(bfloat128 src, const int lane, int switches, minihalf256 income, bool predicate=1, bool polarity=0);
minihalf256 v_convert_bf16_to_h8_vb(bfloat128 src, const int lane, int switches, minihalf256 income, bool128 predicate, bool polarity=0);
uchar256 v_convert_f16_to_u8_b(half128 src, const int lane, int switches, uchar256 income, bool predicate=1, bool polarity=0);
uchar256 v_convert_f16_to_u8_vb(half128 src, const int lane, int switches, uchar256 income, bool128 predicate, bool polarity=0);
minifloat256 v_convert_f16_to_f8_b(half128 src, const int lane, int switches, minifloat256 income, bool predicate=1, bool polarity=0);
minifloat256 v_convert_f16_to_f8_vb(half128 src, const int lane, int switches, minifloat256 income, bool128 predicate, bool polarity=0);
minihalf256 v_convert_f16_to_h8_b(half128 src, const int lane, int switches, minihalf256 income, bool predicate=1, bool polarity=0);
minihalf256 v_convert_f16_to_h8_vb(half128 src, const int lane, int switches, minihalf256 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi_plus__)
bfloat128 v_convert_i32_to_bf16_b(int64 src, const int lane, int switches, bfloat128 income, bool predicate=1, bool polarity=0);
bfloat128 v_convert_i32_to_bf16_vb(int64 src, const int lane, int switches, bfloat128 income, bool64 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_convert_i32_to_f16_b(int64 src, const int lane, int switches, half128 income, bool predicate=1, bool polarity=0);
half128 v_convert_i32_to_f16_vb(int64 src, const int lane, int switches, half128 income, bool64 predicate, bool polarity=0);
#endif
#if defined(__goya__) || defined(__greco_plus__)
uchar256 v_convert_i32_to_u8_b(int64 src, const int lane, int switches, uchar256 income, bool predicate=1, bool polarity=0);
uchar256 v_convert_i32_to_u8_vb(int64 src, const int lane, int switches, uchar256 income, bool64 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
short128 v_convert_i32_to_i16_b(int64 src, const int lane, int switches, short128 income, bool predicate=1, bool polarity=0);
short128 v_convert_i32_to_i16_vb(int64 src, const int lane, int switches, short128 income, bool64 predicate, bool polarity=0);
ushort128 v_convert_i32_to_u16_b(int64 src, const int lane, int switches, ushort128 income, bool predicate=1, bool polarity=0);
ushort128 v_convert_i32_to_u16_vb(int64 src, const int lane, int switches, ushort128 income, bool64 predicate, bool polarity=0);
char256 v_convert_i32_to_i8_b(int64 src, const int lane, int switches, char256 income, bool predicate=1, bool polarity=0);
char256 v_convert_i32_to_i8_vb(int64 src, const int lane, int switches, char256 income, bool64 predicate, bool polarity=0);
minifloat256 v_convert_i32_to_f8_b(int64 src, const int lane, int switches, minifloat256 income, bool predicate=1, bool polarity=0);
minifloat256 v_convert_i32_to_f8_vb(int64 src, const int lane, int switches, minifloat256 income, bool64 predicate, bool polarity=0);
minihalf256 v_convert_i32_to_h8_b(int64 src, const int lane, int switches, minihalf256 income, bool predicate=1, bool polarity=0);
minihalf256 v_convert_i32_to_h8_vb(int64 src, const int lane, int switches, minihalf256 income, bool64 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
bfloat128 v_convert_u32_to_bf16_b(uint64 src, const int lane, int switches, bfloat128 income, bool predicate=1, bool polarity=0);
bfloat128 v_convert_u32_to_bf16_vb(uint64 src, const int lane, int switches, bfloat128 income, bool64 predicate, bool polarity=0);
half128 v_convert_u32_to_f16_b(uint64 src, const int lane, int switches, half128 income, bool predicate=1, bool polarity=0);
half128 v_convert_u32_to_f16_vb(uint64 src, const int lane, int switches, half128 income, bool64 predicate, bool polarity=0);
short128 v_convert_u32_to_i16_b(uint64 src, const int lane, int switches, short128 income, bool predicate=1, bool polarity=0);
short128 v_convert_u32_to_i16_vb(uint64 src, const int lane, int switches, short128 income, bool64 predicate, bool polarity=0);
ushort128 v_convert_u32_to_u16_b(uint64 src, const int lane, int switches, ushort128 income, bool predicate=1, bool polarity=0);
ushort128 v_convert_u32_to_u16_vb(uint64 src, const int lane, int switches, ushort128 income, bool64 predicate, bool polarity=0);
char256 v_convert_u32_to_i8_b(uint64 src, const int lane, int switches, char256 income, bool predicate=1, bool polarity=0);
char256 v_convert_u32_to_i8_vb(uint64 src, const int lane, int switches, char256 income, bool64 predicate, bool polarity=0);
uchar256 v_convert_u32_to_u8_b(uint64 src, const int lane, int switches, uchar256 income, bool predicate=1, bool polarity=0);
uchar256 v_convert_u32_to_u8_vb(uint64 src, const int lane, int switches, uchar256 income, bool64 predicate, bool polarity=0);
minifloat256 v_convert_u32_to_f8_b(uint64 src, const int lane, int switches, minifloat256 income, bool predicate=1, bool polarity=0);
minifloat256 v_convert_u32_to_f8_vb(uint64 src, const int lane, int switches, minifloat256 income, bool64 predicate, bool polarity=0);
minihalf256 v_convert_u32_to_h8_b(uint64 src, const int lane, int switches, minihalf256 income, bool predicate=1, bool polarity=0);
minihalf256 v_convert_u32_to_h8_vb(uint64 src, const int lane, int switches, minihalf256 income, bool64 predicate, bool polarity=0);
#endif
uchar256 v_convert_i16_to_u8_b(short128 src, const int lane, int switches, uchar256 income, bool predicate=1, bool polarity=0);
uchar256 v_convert_i16_to_u8_vb(short128 src, const int lane, int switches, uchar256 income, bool128 predicate, bool polarity=0);
char256 v_convert_i16_to_i8_b(short128 src, const int lane, int switches, char256 income, bool predicate=1, bool polarity=0);
char256 v_convert_i16_to_i8_vb(short128 src, const int lane, int switches, char256 income, bool128 predicate, bool polarity=0);
#if defined(__gaudi2_plus__)
minifloat256 v_convert_i16_to_f8_b(short128 src, const int lane, int switches, minifloat256 income, bool predicate=1, bool polarity=0);
minifloat256 v_convert_i16_to_f8_vb(short128 src, const int lane, int switches, minifloat256 income, bool128 predicate, bool polarity=0);
minihalf256 v_convert_i16_to_h8_b(short128 src, const int lane, int switches, minihalf256 income, bool predicate=1, bool polarity=0);
minihalf256 v_convert_i16_to_h8_vb(short128 src, const int lane, int switches, minihalf256 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
char256 v_convert_u16_to_i8_b(ushort128 src, const int lane, int switches, char256 income, bool predicate=1, bool polarity=0);
char256 v_convert_u16_to_i8_vb(ushort128 src, const int lane, int switches, char256 income, bool128 predicate, bool polarity=0);
uchar256 v_convert_u16_to_u8_b(ushort128 src, const int lane, int switches, uchar256 income, bool predicate=1, bool polarity=0);
uchar256 v_convert_u16_to_u8_vb(ushort128 src, const int lane, int switches, uchar256 income, bool128 predicate, bool polarity=0);
minifloat256 v_convert_u16_to_f8_b(ushort128 src, const int lane, int switches, minifloat256 income, bool predicate=1, bool polarity=0);
minifloat256 v_convert_u16_to_f8_vb(ushort128 src, const int lane, int switches, minifloat256 income, bool128 predicate, bool polarity=0);
minihalf256 v_convert_u16_to_h8_b(ushort128 src, const int lane, int switches, minihalf256 income, bool predicate=1, bool polarity=0);
minihalf256 v_convert_u16_to_h8_vb(ushort128 src, const int lane, int switches, minihalf256 income, bool128 predicate, bool polarity=0);
#endif
/// @}


#if defined(__gaudi_plus__)
/// @section sec1 Convert
/// @subsection subsec1_6 Convert on Single Lane
/// @brief Represents CONVERT instruction in vector slot that make move 1xFP32
/// to 1xBF16/F16 (lane 0 only), which means the 64 values will be assigned into the
/// 128 elements vector in every other lane. E.g. 0,2,4,6 ... 126.
///
/// @param src       The value to convert (SRC1).
/// @param switches  Switches of the instructions.
/// @param income    This value is returned if the predicate is false.
/// @param predicate Predicate value for the instruction.
/// @param polarity  True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///         \li SW_RHAZ - Round half away from zero.
///         \li SW_CSR - Take rounding mode from
#endif
#if defined(__gaudi__) || defined(__gaudib__)
///                    ROUND_CSR register
#endif
#if defined(__greco_plus__)
///                    CONV_ROUND_CSR register
#endif
#if defined(__gaudi2_plus__)
///         \li SW_SR_RNE - Stochastic Rounding, but use RNE when rounding denormals.
/// \par
///     \li SW_CLIP_FP - Convert FP with clipping. \n
///         When the resulting FP value should be +/-inf, it will be clipped to +/-max_normal_value_fp
///
#endif
/// @{
#if defined(__gaudi_plus__)
bfloat128 v_convert_f32_to_bf16_single_b(float64 src, int switches, bfloat128 income, bool predicate=1, bool polarity=0);
bfloat128 v_convert_f32_to_bf16_single_vb(float64 src, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif

#if defined(__greco_plus__)
half128 v_convert_f32_to_f16_single_b(float64 src, int switches, half128 income, bool predicate=1, bool polarity=0);
half128 v_convert_f32_to_f16_single_vb(float64 src, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec1 Convert
/// @subsection subsec1_7 Convert with X4 switch
/// @brief Represents CONVERT instruction in vector slot with X4 switch (converts 4 VRFs).
///
/// @param src       The value to convert (SRC1).
/// @param switches  Switches of the instructions.
/// @param income    This value is returned if the predicate is false.
/// @param predicate Predicate value for the instruction.
/// @param polarity  True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///         \li SW_CSR - Take rounding mode from CONV_ROUND_CSR register.
///         \li SW_RHAZ - Round half away from zero.
#endif
#if defined(__gaudi2_plus__)
///         \li SW_SR_RNE - Stochastic Rounding, but use RNE when rounding denormals.
/// \par
///     \li SW_CLIP_FP - Convert FP with clipping. \n
///         When the resulting FP value should be +/-inf, it will be clipped to +/-max_normal_value_fp
///
#endif
/// @{
#if defined(__greco_plus__)
float256 v_convert_i32_to_f32_x4_b(int256 src, int switches=0, float256 income={}, bool predicate=1, bool polarity=0);
float256 v_convert_i32_to_f32_x4_vb(int256 src, int switches, float256 income, bool256 predicate, bool polarity=0);
#endif

#if defined(__gaudi2_plus__)
float256 v_convert_u32_to_f32_x4_b(uint256 src, int switches=0, float256 income={}, bool predicate=1, bool polarity=0);
float256 v_convert_u32_to_f32_x4_vb(uint256 src, int switches, float256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec1 Convert
/// @subsection subsec1_8 Convert with X2 switch
/// @brief Represents CONVERT instruction in vector slot with X2 switch (converts 2 VRFs).
///
/// @param src       The value to convert (SRC1).
/// @param switches  Switches of the instructions.
/// @param income    This value is returned if the predicate is false.
/// @param predicate Predicate value for the instruction.
/// @param polarity  True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///         \li SW_CSR - Take rounding mode from CONV_ROUND_CSR register.
///         \li SW_RHAZ - Round half away from zero.
#endif
#if defined(__gaudi2_plus__)
///         \li SW_SR_RNE - Stochastic Rounding, but use RNE when rounding denormals.
/// \par
///     \li SW_CLIP_FP - Convert FP with clipping. \n
///         When the resulting FP value should be +/-inf, it will be clipped to +/-max_normal_value_fp
///
#endif
/// @{
#if defined(__greco_plus__)
float128 v_convert_i32_to_f32_x2_b(int128 src, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
float128 v_convert_i32_to_f32_x2_vb(int128 src, int switches, float128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
float128 v_convert_u32_to_f32_x2_b(uint128 src, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
float128 v_convert_u32_to_f32_x2_vb(uint128 src, int switches, float128 income, bool128 predicate, bool polarity=0);
#endif
/// @}

#if defined(__gaudi_plus__)
/// @section sec1 Convert
/// @subsection subsec1_9 Convert with All Lanes
/// @brief Represents CONVERT instruction in vector slot with ALL_LANES switch, \n
/// converts all lanes and vector element size of source and destination is different.
///
/// @param src       The value to convert (SRC1).
/// @param switches  Switches of the instructions.
/// @param income    This value is returned if the predicate is false.
/// @param predicate Predicate value for the instruction.
/// @param polarity  True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///         \li SW_RHAZ - Round half away from zero.
///         \li SW_CSR - Take rounding mode from
#endif
#if defined(__gaudi__) || defined(__gaudib__)
///                    ROUND_CSR register
#endif
#if defined(__greco_plus__)
///                    CONV_ROUND_CSR register
#endif
#if defined(__gaudi2_plus__)
///         \li SW_SR_RNE - Stochastic Rounding, but use RNE when rounding denormals.
/// \par
///     \li SW_CLIP_FP - Convert FP with clipping. \n
///         When the resulting FP value should be +/-inf, it will be clipped to +/-max_normal_value_fp
///
#endif
/// @{
#if defined(__greco_plus__)
bfloat128 v_convert_f32_to_bf16_all_vb(float128 src, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi_plus__)
bfloat128 v_convert_f32_to_bf16_all_b(float128 src, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
float128 v_convert_bf16_to_f32_all_b(bfloat128 src, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
float128 v_convert_bf16_to_f32_all_vb(bfloat128 src, int switches, float128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_convert_f32_to_f16_all_b(float128 src, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_convert_f32_to_f16_all_vb(float128 src, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
float128 v_convert_f16_to_f32_all_b(half128 src, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
float128 v_convert_f16_to_f32_all_vb(half128 src, int switches, float128 income, bool128 predicate, bool polarity=0);
#endif

#if defined(__greco_plus__)
short128 v_convert_f32_to_i16_all_b(float128 src, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_convert_f32_to_i16_all_vb(float128 src, int switches, short128 income, bool128 predicate, bool polarity=0);
char256 v_convert_f32_to_i8_all_b(float256 src, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_convert_f32_to_i8_all_vb(float256 src, int switches, char256 income, bool256 predicate, bool polarity=0);

char256 v_convert_bf16_to_i8_all_b(bfloat256 src, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_convert_bf16_to_i8_all_vb(bfloat256 src, int switches, char256 income, bool256 predicate, bool polarity=0);

char256 v_convert_f16_to_i8_all_b(half256 src, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_convert_f16_to_i8_all_vb(half256 src, int switches, char256 income, bool256 predicate, bool polarity=0);

bfloat128 v_convert_i32_to_bf16_all_b(int128 src, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_convert_i32_to_bf16_all_vb(int128 src, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
half128 v_convert_i32_to_f16_all_b(int128 src, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_convert_i32_to_f16_all_vb(int128 src, int switches, half128 income, bool128 predicate, bool polarity=0);

float128 v_convert_i16_to_f32_all_b(short128 src, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
float128 v_convert_i16_to_f32_all_vb(short128 src, int switches, float128 income, bool128 predicate, bool polarity=0);
int128 v_convert_i16_to_i32_all_b(short128 src, int switches=0, int128 income={}, bool predicate=1, bool polarity=0);
int128 v_convert_i16_to_i32_all_vb(short128 src, int switches, int128 income, bool128 predicate, bool polarity=0);
uint128 v_convert_i16_to_u32_all_b(short128 src, int switches=0, uint128 income={}, bool predicate=1, bool polarity=0);
uint128 v_convert_i16_to_u32_all_vb(short128 src, int switches, uint128 income, bool128 predicate, bool polarity=0);
uchar256 v_convert_i16_to_u8_all_b(short256 src, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_convert_i16_to_u8_all_vb(short256 src, int switches, uchar256 income, bool256 predicate, bool polarity=0);

float256 v_convert_i8_to_f32_all_b(char256 src, int switches=0, float256 income={}, bool predicate=1, bool polarity=0);
float256 v_convert_i8_to_f32_all_vb(char256 src, int switches, float256 income, bool256 predicate, bool polarity=0);
int256 v_convert_i8_to_i32_all_b(char256 src, int switches=0, int256 income={}, bool predicate=1, bool polarity=0);
int256 v_convert_i8_to_i32_all_vb(char256 src, int switches, int256 income, bool256 predicate, bool polarity=0);
uint256 v_convert_i8_to_u32_all_b(char256 src, int switches=0, uint256 income={}, bool predicate=1, bool polarity=0);
uint256 v_convert_i8_to_u32_all_vb(char256 src, int switches, uint256 income, bool256 predicate, bool polarity=0);
short256 v_convert_i8_to_i16_all_b(char256 src, int switches=0, short256 income={}, bool predicate=1, bool polarity=0);
short256 v_convert_i8_to_i16_all_vb(char256 src, int switches, short256 income, bool256 predicate, bool polarity=0);
ushort256 v_convert_i8_to_u16_all_b(char256 src, int switches=0, ushort256 income={}, bool predicate=1, bool polarity=0);
ushort256 v_convert_i8_to_u16_all_vb(char256 src, int switches, ushort256 income, bool256 predicate, bool polarity=0);
bfloat256 v_convert_i8_to_bf16_all_b(char256 src, int switches=0, bfloat256 income={}, bool predicate=1, bool polarity=0);
bfloat256 v_convert_i8_to_bf16_all_vb(char256 src, int switches, bfloat256 income, bool256 predicate, bool polarity=0);

float256 v_convert_u8_to_f32_all_b(uchar256 src, int switches=0, float256 income={}, bool predicate=1, bool polarity=0);
float256 v_convert_u8_to_f32_all_vb(uchar256 src, int switches, float256 income, bool256 predicate, bool polarity=0);
bfloat256 v_convert_u8_to_bf16_all_b(uchar256 src, int switches=0, bfloat256 income={}, bool predicate=1, bool polarity=0);
bfloat256 v_convert_u8_to_bf16_all_vb(uchar256 src, int switches, bfloat256 income, bool256 predicate, bool polarity=0);
int256 v_convert_u8_to_i32_all_b(uchar256 src, int switches=0, int256 income={}, bool predicate=1, bool polarity=0);
int256 v_convert_u8_to_i32_all_vb(uchar256 src, int switches, int256 income, bool256 predicate, bool polarity=0);
short256 v_convert_u8_to_i16_all_b(uchar256 src, int switches=0, short256 income={}, bool predicate=1, bool polarity=0);
short256 v_convert_u8_to_i16_all_vb(uchar256 src, int switches, short256 income, bool256 predicate, bool polarity=0);
uint256 v_convert_u8_to_u32_all_b(uchar256 src, int switches=0, uint256 income={}, bool predicate=1, bool polarity=0);
uint256 v_convert_u8_to_u32_all_vb(uchar256 src, int switches, uint256 income, bool256 predicate, bool polarity=0);
ushort256 v_convert_u8_to_u16_all_b(uchar256 src, int switches=0, ushort256 income={}, bool predicate=1, bool polarity=0);
ushort256 v_convert_u8_to_u16_all_vb(uchar256 src, int switches, ushort256 income, bool256 predicate, bool polarity=0);

char512 v_convert_i4_to_i8_all_b(nibble512 src, int switches=0, char512 income={}, bool predicate=1, bool polarity=0);
char512 v_convert_i4_to_i8_all_vb(nibble512 src, int switches, char512 income, bool256 predicate, bool polarity=0);

uchar512 v_convert_u4_to_u8_all_b(unibble512 src, int switches=0, uchar512 income={}, bool predicate=1, bool polarity=0);
uchar512 v_convert_u4_to_u8_all_vb(unibble512 src, int switches, uchar512 income, bool256 predicate, bool polarity=0);
#endif

#if defined(__gaudi2_plus__)
ushort128 v_convert_f32_to_u16_all_b(float128 src, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_convert_f32_to_u16_all_vb(float128 src, int switches, ushort128 income, bool128 predicate, bool polarity=0);
uchar256 v_convert_f32_to_u8_all_b(float256 src, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_convert_f32_to_u8_all_vb(float256 src, int switches, uchar256 income, bool256 predicate, bool polarity=0);
minifloat256 v_convert_f32_to_f8_all_b(float256 src, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_convert_f32_to_f8_all_vb(float256 src, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_convert_f32_to_h8_all_b(float256 src, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_convert_f32_to_h8_all_vb(float256 src, int switches, minihalf256 income, bool256 predicate, bool polarity=0);

int128 v_convert_bf16_to_i32_all_b(bfloat128 src, int switches=0, int128 income={}, bool predicate=1, bool polarity=0);
int128 v_convert_bf16_to_i32_all_vb(bfloat128 src, int switches, int128 income, bool128 predicate, bool polarity=0);
uint128 v_convert_bf16_to_u32_all_b(bfloat128 src, int switches=0, uint128 income={}, bool predicate=1, bool polarity=0);
uint128 v_convert_bf16_to_u32_all_vb(bfloat128 src, int switches, uint128 income, bool128 predicate, bool polarity=0);
uchar256 v_convert_bf16_to_u8_all_b(bfloat256 src, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_convert_bf16_to_u8_all_vb(bfloat256 src, int switches, uchar256 income, bool256 predicate, bool polarity=0);
minifloat256 v_convert_bf16_to_f8_all_b(bfloat256 src, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_convert_bf16_to_f8_all_vb(bfloat256 src, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_convert_bf16_to_h8_all_b(bfloat256 src, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_convert_bf16_to_h8_all_vb(bfloat256 src, int switches, minihalf256 income, bool256 predicate, bool polarity=0);

int128 v_convert_f16_to_i32_all_b(half128 src, int switches=0, int128 income={}, bool predicate=1, bool polarity=0);
int128 v_convert_f16_to_i32_all_vb(half128 src, int switches, int128 income, bool128 predicate, bool polarity=0);
uint128 v_convert_f16_to_u32_all_b(half128 src, int switches=0, uint128 income={}, bool predicate=1, bool polarity=0);
uint128 v_convert_f16_to_u32_all_vb(half128 src, int switches, uint128 income, bool128 predicate, bool polarity=0);
uchar256 v_convert_f16_to_u8_all_b(half256 src, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_convert_f16_to_u8_all_vb(half256 src, int switches, uchar256 income, bool256 predicate, bool polarity=0);
minifloat256 v_convert_f16_to_f8_all_b(half256 src, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_convert_f16_to_f8_all_vb(half256 src, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_convert_f16_to_h8_all_b(half256 src, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_convert_f16_to_h8_all_vb(half256 src, int switches, minihalf256 income, bool256 predicate, bool polarity=0);

float256 v_convert_f8_to_f32_all_b(minifloat256 src, int switches=0, float256 income={}, bool predicate=1, bool polarity=0);
float256 v_convert_f8_to_f32_all_vb(minifloat256 src, int switches, float256 income, bool256 predicate, bool polarity=0);
float256 v_convert_h8_to_f32_all_b(minihalf256 src, int switches=0, float256 income={}, bool predicate=1, bool polarity=0);
float256 v_convert_h8_to_f32_all_vb(minihalf256 src, int switches, float256 income, bool256 predicate, bool polarity=0);
bfloat256 v_convert_f8_to_bf16_all_b(minifloat256 src, int switches=0, bfloat256 income={}, bool predicate=1, bool polarity=0);
bfloat256 v_convert_f8_to_bf16_all_vb(minifloat256 src, int switches, bfloat256 income, bool256 predicate, bool polarity=0);
bfloat256 v_convert_h8_to_bf16_all_b(minihalf256 src, int switches=0, bfloat256 income={}, bool predicate=1, bool polarity=0);
bfloat256 v_convert_h8_to_bf16_all_vb(minihalf256 src, int switches, bfloat256 income, bool256 predicate, bool polarity=0);
half256 v_convert_f8_to_f16_all_b(minifloat256 src, int switches=0, half256 income={}, bool predicate=1, bool polarity=0);
half256 v_convert_f8_to_f16_all_vb(minifloat256 src, int switches, half256 income, bool256 predicate, bool polarity=0);
half256 v_convert_h8_to_f16_all_b(minihalf256 src, int switches=0, half256 income={}, bool predicate=1, bool polarity=0);
half256 v_convert_h8_to_f16_all_vb(minihalf256 src, int switches, half256 income, bool256 predicate, bool polarity=0);
int256 v_convert_f8_to_i32_all_b(minifloat256 src, int switches=0, int256 income={}, bool predicate=1, bool polarity=0);
int256 v_convert_f8_to_i32_all_vb(minifloat256 src, int switches, int256 income, bool256 predicate, bool polarity=0);
int256 v_convert_h8_to_i32_all_b(minihalf256 src, int switches=0, int256 income={}, bool predicate=1, bool polarity=0);
int256 v_convert_h8_to_i32_all_vb(minihalf256 src, int switches, int256 income, bool256 predicate, bool polarity=0);
uint256 v_convert_f8_to_u32_all_b(minifloat256 src, int switches=0, uint256 income={}, bool predicate=1, bool polarity=0);
uint256 v_convert_f8_to_u32_all_vb(minifloat256 src, int switches, uint256 income, bool256 predicate, bool polarity=0);
uint256 v_convert_h8_to_u32_all_b(minihalf256 src, int switches=0, uint256 income={}, bool predicate=1, bool polarity=0);
uint256 v_convert_h8_to_u32_all_vb(minihalf256 src, int switches, uint256 income, bool256 predicate, bool polarity=0);
short256 v_convert_f8_to_i16_all_b(minifloat256 src, int switches=0, short256 income={}, bool predicate=1, bool polarity=0);
short256 v_convert_f8_to_i16_all_vb(minifloat256 src, int switches, short256 income, bool256 predicate, bool polarity=0);
short256 v_convert_h8_to_i16_all_b(minihalf256 src, int switches=0, short256 income={}, bool predicate=1, bool polarity=0);
short256 v_convert_h8_to_i16_all_vb(minihalf256 src, int switches, short256 income, bool256 predicate, bool polarity=0);
ushort256 v_convert_f8_to_u16_all_b(minifloat256 src, int switches=0, ushort256 income={}, bool predicate=1, bool polarity=0);
ushort256 v_convert_f8_to_u16_all_vb(minifloat256 src, int switches, ushort256 income, bool256 predicate, bool polarity=0);
ushort256 v_convert_h8_to_u16_all_b(minihalf256 src, int switches=0, ushort256 income={}, bool predicate=1, bool polarity=0);
ushort256 v_convert_h8_to_u16_all_vb(minihalf256 src, int switches, ushort256 income, bool256 predicate, bool polarity=0);

short128 v_convert_i32_to_i16_all_b(int128 src, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_convert_i32_to_i16_all_vb(int128 src, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_convert_i32_to_u16_all_b(int128 src, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_convert_i32_to_u16_all_vb(int128 src, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_convert_i32_to_i8_all_b(int256 src, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_convert_i32_to_i8_all_vb(int256 src, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_convert_i32_to_u8_all_b(int256 src, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_convert_i32_to_u8_all_vb(int256 src, int switches, uchar256 income, bool256 predicate, bool polarity=0);
minifloat256 v_convert_i32_to_f8_all_b(int256 src, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_convert_i32_to_f8_all_vb(int256 src, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_convert_i32_to_h8_all_b(int256 src, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_convert_i32_to_h8_all_vb(int256 src, int switches, minihalf256 income, bool256 predicate, bool polarity=0);

bfloat128 v_convert_u32_to_bf16_all_b(uint128 src, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_convert_u32_to_bf16_all_vb(uint128 src, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
half128 v_convert_u32_to_f16_all_b(uint128 src, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_convert_u32_to_f16_all_vb(uint128 src, int switches, half128 income, bool128 predicate, bool polarity=0);
short128 v_convert_u32_to_i16_all_b(uint128 src, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_convert_u32_to_i16_all_vb(uint128 src, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_convert_u32_to_u16_all_b(uint128 src, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_convert_u32_to_u16_all_vb(uint128 src, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_convert_u32_to_i8_all_b(uint256 src, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_convert_u32_to_i8_all_vb(uint256 src, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_convert_u32_to_u8_all_b(uint256 src, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_convert_u32_to_u8_all_vb(uint256 src, int switches, uchar256 income, bool256 predicate, bool polarity=0);
minifloat256 v_convert_u32_to_f8_all_b(uint256 src, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_convert_u32_to_f8_all_vb(uint256 src, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_convert_u32_to_h8_all_b(uint256 src, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_convert_u32_to_h8_all_vb(uint256 src, int switches, minihalf256 income, bool256 predicate, bool polarity=0);

char256 v_convert_i16_to_i8_all_b(short256 src, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_convert_i16_to_i8_all_vb(short256 src, int switches, char256 income, bool256 predicate, bool polarity=0);
minifloat256 v_convert_i16_to_f8_all_b(short256 src, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_convert_i16_to_f8_all_vb(short256 src, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_convert_i16_to_h8_all_b(short256 src, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_convert_i16_to_h8_all_vb(short256 src, int switches, minihalf256 income, bool256 predicate, bool polarity=0);

float128 v_convert_u16_to_f32_all_b(ushort128 src, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
float128 v_convert_u16_to_f32_all_vb(ushort128 src, int switches, float128 income, bool128 predicate, bool polarity=0);
int128 v_convert_u16_to_i32_all_b(ushort128 src, int switches=0, int128 income={}, bool predicate=1, bool polarity=0);
int128 v_convert_u16_to_i32_all_vb(ushort128 src, int switches, int128 income, bool128 predicate, bool polarity=0);
uint128 v_convert_u16_to_u32_all_b(ushort128 src, int switches=0, uint128 income={}, bool predicate=1, bool polarity=0);
uint128 v_convert_u16_to_u32_all_vb(ushort128 src, int switches, uint128 income, bool128 predicate, bool polarity=0);
char256 v_convert_u16_to_i8_all_b(ushort256 src, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_convert_u16_to_i8_all_vb(ushort256 src, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_convert_u16_to_u8_all_b(ushort256 src, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_convert_u16_to_u8_all_vb(ushort256 src, int switches, uchar256 income, bool256 predicate, bool polarity=0);
minifloat256 v_convert_u16_to_f8_all_b(ushort256 src, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_convert_u16_to_f8_all_vb(ushort256 src, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_convert_u16_to_h8_all_b(ushort256 src, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_convert_u16_to_h8_all_vb(ushort256 src, int switches, minihalf256 income, bool256 predicate, bool polarity=0);

half256 v_convert_i8_to_f16_all_b(char256 src, int switches=0, half256 income={}, bool predicate=1, bool polarity=0);
half256 v_convert_i8_to_f16_all_vb(char256 src, int switches, half256 income, bool256 predicate, bool polarity=0);

half256 v_convert_u8_to_f16_all_b(uchar256 src, int switches=0, half256 income={}, bool predicate=1, bool polarity=0);
half256 v_convert_u8_to_f16_all_vb(uchar256 src, int switches, half256 income, bool256 predicate, bool polarity=0);
#endif
/// @}

// high level int conversions
short128 int_convert_int128_to_short128(int128 a, short128 shift, const int options);
ushort128 uint_convert_uint128_to_ushort128(uint128 a, ushort128 shift, const int options);

char256 int_convert_int256_to_char256(int256 a, char256 shift, const int options);
uchar256 uint_convert_uint256_to_uchar256(uint256 a, uchar256 shift, const int options);


char256 int_convert_short256_to_char256(int256 a, short256 shift, const int options);
uchar256 uint_convert_ushort256_to_uchar256(ushort256 a, uchar256 shift, const int options);


//
// ------ CONVERT_INT32
//

/// @section sec1 Convert
/// @subsection subsec1_10 Scalar Convert INT32
/// @brief Represents scalar CONVERT_INT32 instruction.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_INT32 (SRC2).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
#if defined(__gaudi_plus__)
///         \li SW_RZ - Round zero.
#endif
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///
/// @{

// Although the second argument (shift) is declared as INT8 in Goya and Gaudi,
// we use INT32 for all architectures to have the single function.
int16_t s_convert_int32_to_i16(int32_t value, int32_t shift, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
int8_t  s_convert_int32_to_i8 (int32_t value, int32_t shift, int switches=0, int8_t  income={}, bool predicate=1, bool polarity=0);
/// @}


#if defined(__goya__) || defined(__gaudi__) || defined(__gaudib__)
/// @section sec1 Convert
/// @subsection subsec1_11 Vector Convert INT32
/// @brief Represents vector CONVERT_INT32 instruction.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_INT32 (SRC2).
/// @param lane        Lane number in the output vector, (0: all even lanes, 1: all odd lanes). \n
/// For i32 to i16, only lanes 0-1 are available. For i32 to i8, all lanes 0-3 are available.
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
#endif
#if defined(__gaudi__) || defined(__gaudib__)
///         \li SW_RZ - Round zero.
///
#endif
/// @{

// These functions are defined for Goya and Gaudi only. On Goya2+ type of
// argument 'shift' is changed, so corresponding intrinsics are incompatible.
// They are available as intrinsics with suffix '_single_'.
#if defined(__dali__) || defined(__gaudi__) || defined(__gaudib__)
short128 v_convert_int32_to_i16_b (int64 value, char256 shift, const int lane, int switches, short128 income, bool   predicate=1, bool polarity=0);
short128 v_convert_int32_to_i16_vb(int64 value, char256 shift, const int lane, int switches, short128 income, bool64 predicate, bool polarity=0);
char256  v_convert_int32_to_i8_b  (int64 value, char256 shift, const int lane, int switches, char256  income, bool   predicate=1, bool polarity=0);
char256  v_convert_int32_to_i8_vb (int64 value, char256 shift, const int lane, int switches, char256  income, bool64 predicate, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec1 Convert
/// @subsection subsec1_12 Vector Convert INT32 with Single Lane
/// @brief Represents vector CONVERT_INT32 instruction with SINGLE_LANE switch.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_INT32 (SRC2).
/// @param lane        Lane number in the output vector, (0: all even lanes, 1: all odd lanes). \n
/// For i32 to i16, only lanes 0-1 are available. For i32 to i8, all lanes 0-3 are available.
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///
#endif
/// @{
#if defined(__greco_plus__)
short128 v_convert_int32_to_i16_single_b(int64 value, int64 shift, const int lane, int switches, short128 income, bool predicate=1, bool polarity=0);
short128 v_convert_int32_to_i16_single_vb(int64 value, int64 shift, const int lane, int switches, short128 income, bool64 predicate, bool polarity=0);
char256 v_convert_int32_to_i8_single_b(int64 value, int64 shift, const int lane, int switches, char256 income, bool predicate=1, bool polarity=0);
char256 v_convert_int32_to_i8_single_vb(int64 value, int64 shift, const int lane, int switches, char256 income, bool64 predicate, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec1 Convert
/// @subsection subsec1_13 Vector Convert INT32 with All Lanes
/// @brief Represents vector CONVERT_INT32 instruction with ALL_LANES switch.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_INT32 (SRC2).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///
#endif
/// @{
#if defined(__greco_plus__)
short128 v_convert_int32_to_i16_all_b(int128 value, short128 shift, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_convert_int32_to_i16_all_vb(int128 value, short128 shift, int switches, short128 income, bool128 predicate, bool polarity=0);
char256 v_convert_int32_to_i8_all_b(int256 value, char256 shift, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_convert_int32_to_i8_all_vb(int256 value, char256 shift, int switches, char256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


//
// ------ CONVERT_UINT32
//


/// @section sec1 Convert
/// @subsection subsec1_14 Scalar CONVERT_UINT32
/// @brief Represents scalar CONVERT_UINT32 instruction.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_UINT32 (SRC2).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
#if defined(__gaudi_plus__)
///         \li SW_RZ - Round zero.
#endif
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///
/// @{

// Although the second argument (shift) is declared as INT8 in Goya and Gaudi,
// we use INT32 for all architectures to have the single function for all architectures.
uint16_t s_convert_uint32_to_u16(uint32_t value, int32_t shift, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
uint8_t  s_convert_uint32_to_u8 (uint32_t value, int32_t shift, int switches=0, uint8_t  income={}, bool predicate=1, bool polarity=0);
/// @}


#if defined(__goya__) || defined(__gaudi__) || defined(__gaudib__)
/// @section sec1 Convert
/// @subsection subsec1_15 Vector CONVERT_UINT32
/// @brief Represents vector CONVERT_UINT32 instruction.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_UINT32 (SRC2).
/// @param lane        Lane number in the output vector, (0: all even lanes, 1: all odd lanes). \n
/// For u32 to u16, only lanes 0-1 are available. For u32 to u8, all lanes 0-3 are available.
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
#endif
#if defined(__gaudi__) || defined(__gaudib__)
///         \li SW_RZ - Round zero.
///
#endif
/// @{

// These functions are defined for Goya and Gaudi only. On Goya2+ type of
// argument 'shift' is changed, so corresponding intrinsics are incompatible.
// They are available as intrinsics with suffix '_single_'.
#if defined(__dali__) || defined(__gaudi__) || defined(__gaudib__)
ushort128 v_convert_uint32_to_u16_b (uint64 value, char256 shift, const int lane, int switches, ushort128 income, bool predicate=1, bool polarity=0);
ushort128 v_convert_uint32_to_u16_vb(uint64 value, char256 shift, const int lane, int switches, ushort128 income, bool64 predicate, bool polarity=0);
uchar256  v_convert_uint32_to_u8_b  (uint64 value, char256 shift, const int lane, int switches, uchar256  income, bool predicate=1, bool polarity=0);
uchar256  v_convert_uint32_to_u8_vb (uint64 value, char256 shift, const int lane, int switches, uchar256  income, bool64 predicate, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec1 Convert
/// @subsection subsec1_16 Vector CONVERT_UINT32 with Single Lane
/// @brief Represents vector CONVERT_UINT32 instruction with SINGLE_LANE switch.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_UINT32 (SRC2).
/// @param lane        Lane number in the output vector, (0: all even lanes, 1: all odd lanes). \n
/// For u32 to u16, only lanes 0-1 are available. For u32 to u8, all lanes 0-3 are available.
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///
#endif
/// @{
#if defined(__greco_plus__)
ushort128 v_convert_uint32_to_u16_single_b(uint64 value, int64 shift, const int lane, int switches, ushort128 income, bool predicate=1, bool polarity=0);
ushort128 v_convert_uint32_to_u16_single_vb(uint64 value, int64 shift, const int lane, int switches, ushort128 income, bool64 predicate, bool polarity=0);
uchar256 v_convert_uint32_to_u8_single_b(uint64 value, int64 shift, const int lane, int switches, uchar256 income, bool predicate=1, bool polarity=0);
uchar256 v_convert_uint32_to_u8_single_vb(uint64 value, int64 shift, const int lane, int switches, uchar256 income, bool64 predicate, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec1 Convert
/// @subsection subsec1_17 Vector CONVERT_UINT32 with ALL_LANES
/// @brief Represents vector CONVERT_UINT32 instruction with ALL_LANES switch.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_UINT32 (SRC2).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///
#endif
/// @{
#if defined(__greco_plus__)
ushort128 v_convert_uint32_to_u16_all_b(uint128 value, short128 shift, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_convert_uint32_to_u16_all_vb(uint128 value, short128 shift, int switches, ushort128 income, bool128 predicate, bool polarity=0);
uchar256 v_convert_uint32_to_u8_all_b(uint256 value, char256 shift, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_convert_uint32_to_u8_all_vb(uint256 value, char256 shift, int switches, uchar256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


//
// ------ CONVERT_INT16
//

/// @section sec1 Convert
/// @subsection subsec1_18 Scalar CONVERT_INT16
/// @brief Represents scalar CONVERT_INT16 instruction.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_INT16 (SRC2).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
#if defined(__gaudi_plus__)
///         \li SW_RZ - Round zero.
#endif
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///
/// @{

// Although the second argument (shift) is declared as INT8 in Goya and Gaudi,
// we use INT32 for all architectures to have the single function for all architectures.
int8_t s_convert_int16_to_i8(int16_t value, int32_t shift, int switches, int8_t income, bool predicate, bool polarity);
/// @}


#if defined(__goya__) || defined(__gaudi__) || defined(__gaudib__)
/// @section sec1 Convert
/// @subsection subsec1_19 Vector CONVERT_INT16
/// @brief Represents vector CONVERT_INT16 instruction.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_INT16 (SRC2).
/// @param lane        Lane number in the output vector, (0: all even lanes, 1: all odd lanes). \n
/// For i16 to i8, only lanes 0-1 are available.
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
#endif
#if defined(__gaudi__) || defined(__gaudib__)
///         \li SW_RZ - Round zero.
///
#endif
/// @{

// These functions are defined for Goya and Gaudi only. On Goya2+ type of
// argument 'shift' is changed, so corresponding intrinsics are incompatible.
// They are available as intrinsics with suffix '_single_'.
#if defined(__dali__) || defined(__gaudi__) || defined(__gaudib__)
char256 v_convert_int16_to_i8_b (short128 value, char256 shift, const int lane, int switches, char256 income, bool    predicate, bool polarity);
char256 v_convert_int16_to_i8_vb(short128 value, char256 shift, const int lane, int switches, char256 income, bool128 predicate, bool polarity);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec1 Convert
/// @subsection subsec1_20 Vector CONVERT_INT16 with SINGLE_LANE
/// @brief Represents vector CONVERT_INT16 instruction with SINGLE_LANE switch.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_INT16 (SRC2).
/// @param lane        Lane number in the output vector, (0: all even lanes, 1: all odd lanes). \n
/// For i16 to i8, only lanes 0-1 are available.
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///
#endif
/// @{
#if defined(__greco_plus__)
char256 v_convert_int16_to_i8_single_b(short128 value, short128 shift, const int lane, int switches, char256 income, bool predicate=1, bool polarity=0);
char256 v_convert_int16_to_i8_single_vb(short128 value, short128 shift, const int lane, int switches, char256 income, bool128 predicate, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec1 Convert
/// @subsection subsec1_21 Vector CONVERT_INT16 with ALL_LANES
/// @brief Represents vector CONVERT_INT16 instruction with ALL_LANES switch.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_INT16 (SRC2).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///
#endif
/// @{
#if defined(__greco_plus__)
char256 v_convert_int16_to_i8_all_b(short256 value, char256 shift, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_convert_int16_to_i8_all_vb(short256 value, char256 shift, int switches, char256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


//
// ------ CONVERT_UINT16
//


/// @section sec1 Convert
/// @subsection subsec1_22 Scalar CONVERT_UINT16
/// @brief Represents scalar CONVERT_UINT16 instruction.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_UINT16 (SRC2).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
#if defined(__gaudi_plus__)
///         \li SW_RZ - Round zero.
#endif
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///
/// @{

// Although the second argument (shift) is declared as INT8 in Goya and Gaudi,
// we use INT32 for all architectures to have the single function for all architectures.
uint8_t s_convert_uint16_to_u8(uint16_t value, int32_t shift, int switches, uint8_t income, bool predicate, bool polarity);
/// @}


#if defined(__goya__) || defined(__gaudi__) || defined(__gaudib__)
/// @section sec1 Convert
/// @subsection subsec1_23 Vector CONVERT_UINT16
/// @brief Represents vector CONVERT_UINT16 instruction.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_UINT16 (SRC2).
/// @param lane        Lane number in the output vector, (0:all even lanes, 1:all odd lanes). \n
/// For u16 to u8, only lanes 0-1 are available.
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
#endif
#if defined(__gaudi__) || defined(__gaudib__)
///         \li SW_RZ - Round zero.
///
#endif
/// @{

// These functions are defined for Goya and Gaudi only. On Goya2+ type of
// argument 'shift' is changed, so corresponding intrinsics are incompatible.
// They are available as intrinsics with suffix '_single_'.
#if defined(__dali__) || defined(__gaudi__) || defined(__gaudib__)
uchar256 v_convert_uint16_to_u8_b (ushort128 value, char256 shift, const int lane, int switches, uchar256 income, bool    predicate, bool polarity);
uchar256 v_convert_uint16_to_u8_vb(ushort128 value, char256 shift, const int lane, int switches, uchar256 income, bool128 predicate, bool polarity);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec1 Convert
/// @subsection subsec1_24 Vector CONVERT_UINT16 with SINGLE_LANE
/// @brief Represents vector CONVERT_UINT16 instruction with SINGLE_LANE switch.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_UINT16 (SRC2).
/// @param lane        Lane number in the output vector, (0:all even lanes, 1:all odd lanes). \n
/// For u16 to u8, only lanes 0-1 are available.
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///
#endif
/// @{
#if defined(__greco_plus__)
uchar256 v_convert_uint16_to_u8_single_b(ushort128 value, short128 shift, const int lane, int switches, uchar256 income, bool predicate=1, bool polarity=0);
uchar256 v_convert_uint16_to_u8_single_vb(ushort128 value, short128 shift, const int lane, int switches, uchar256 income, bool128 predicate, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec1 Convert
/// @subsection subsec1_25 Vector CONVERT_UINT16 with ALL_LANE
/// @brief Represents vector CONVERT_UINT16 instruction with ALL_LANES switch.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_UINT16 (SRC2).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///
#endif
/// @{
#if defined(__greco_plus__)
uchar256 v_convert_uint16_to_u8_all_b(ushort256 value, char256 shift, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_convert_uint16_to_u8_all_vb(ushort256 value, char256 shift, int switches, uchar256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


//
// ------ CONVERT_INT8
//


#if defined(__greco_plus__)
/// @section sec1 Convert
/// @subsection subsec1_26 Scalar CONVERT_INT8
/// @brief Represents scalar CONVERT_INT8 instruction.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_INT8 (SRC2).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///
#endif
/// @{
#if defined(__greco_plus__)
int8_t s_convert_int8_to_i4(int8_t value, int32_t shift, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec1 Convert
/// @subsection subsec1_27 Vector CONVERT_INT8
/// @brief Represents vector CONVERT_INT8 instruction.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_INT8 (SRC2).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///
#endif
/// @{
#if defined(__greco_plus__)
nibble512 v_convert_int8_to_i4_all_b(char512 value, char256 shift, int switches=0, nibble512 income={}, bool predicate=1, bool polarity=0);
nibble512 v_convert_int8_to_i4_all_vb(char512 value, char256 shift, int switches, nibble512 income, bool256 predicate, bool polarity=0);
#endif
/// @}


//
// ------ CONVERT_UINT8
//

#if defined(__greco_plus__)
/// @section sec1 Convert
/// @subsection subsec1_28 Scalar CONVERT_UINT8
/// @brief Represents scalar CONVERT_UINT8 instruction.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_UINT8 (SRC2).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///
#endif
/// @{
#if defined(__greco_plus__)
uint8_t s_convert_uint8_to_u4(int8_t value, int32_t shift, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec1 Convert
/// @subsection subsec1_29 Vector CONVERT_UINT8
/// @brief Represents vector CONVERT_UINT8 instruction.
///
/// @param value       The value to convert (SRC1).
/// @param shift       The shift argument to CONVERT_UINT8 (SRC2).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Converted value.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///         \li SW_SR - Stochastic Rounding.
///
#endif
/// @{
#if defined(__greco_plus__)
unibble512 v_convert_uint8_to_u4_all_b(uchar512 value, char256 shift, int switches=0, unibble512 income={}, bool predicate=1, bool polarity=0);
unibble512 v_convert_uint8_to_u4_all_vb(uchar512 value, char256 shift, int switches, unibble512 income, bool256 predicate, bool polarity=0);
#endif
/// @}


//
// ------ EVENT
//

#if defined(__gaudi2_plus__)
/// @section sec6 Program Control Unit Instruction
/// @subsection subsec6_1 Event
/// @brief Represents EVENT instruction.
///
/// @param a           payload (SRC1).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Generates an event that is written to STM memory.
///
/// @switches Allowed switches are:
///     \li SW_SPU - Execute on SPU slot.
///     \li SW_VPU - Execute on VPU slot.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
void event(uint16_t a, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ EXTRACT_EXP
//

/// @section sec2 Arithmetic
/// @subsection subsec2_11 EXTRACT_EXP
/// @brief Represents EXTRACT_EXP instruction.
///
/// @param a           Input float number (SRC1).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Mantissa of the input value.
///
/// @switches Allowed switches are:
///     \li SW_BIASED  - When set, the exponent should be biased (unsigned). When cleared, the exponent should be non-biased (signed).
#if defined(__doron1__)
///     \li SW_PRE_LOG - When set, the special operation is used as preparation for LOG calculation.
#endif
///
/// @{
int32_t s_f32_extract_exp(float a, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
int16_t s_bf16_extract_exp(bf16 a, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
int16_t s_f16_extract_exp(half a, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
int8_t s_f8_extract_exp(minifloat a, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
int8_t s_h8_extract_exp(minihalf a, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
#endif
int64 v_f32_extract_exp_vb(float64 a, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_f32_extract_exp_b(float64 a, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
short128 v_bf16_extract_exp_vb(bfloat128 a, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_bf16_extract_exp_b(bfloat128 a, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
short128 v_f16_extract_exp_vb(half128 a, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_f16_extract_exp_b(half128 a, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
char256 v_f8_extract_exp_vb(minifloat256 a, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_f8_extract_exp_b(minifloat256 a, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_h8_extract_exp_vb(minihalf256 a, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_h8_extract_exp_b(minihalf256 a, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ FCLASS
//

#if defined(__gaudi_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_12 FCLASS 
/// @brief Represents FCLASS instruction.
///
/// @param a           The input value (SRC1).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return 8/10bit-masks format describing the input value.
///
/// 10bit mask maps to the following classes: (returned for 16/32 bit datatypes) \n
/// | bit   | 0    | 1       | 2         | 3  | 4  | 5         | 6       | 7    | 8    | 9    |
/// |-------|------|---------|-----------|----|----|-----------|---------|------|------|------|
/// | class | -Inf | -normal | -denormal | -0 | +0 | +denormal | +normal | +Inf | sNaN | qNaN |
///
/// 8bit mask maps to the following classes: (returned for 8 bit datatypes) \n
/// | bit   | 0    | 1       | 2         | 3        | 4         | 5       | 6    | 7            |
/// |-------|------|---------|-----------|----------|-----------|---------|------|--------------|
/// | class | -Inf | -normal | -denormal | -0 or +0 | +denormal | +normal | +Inf | sNaN or qNaN |
///
#endif
/// @{
#if defined(__gaudi_plus__)
float s_f32_fclass(float a, int switches=0, float income={}, bool predicate=1, bool polarity=0);
bf16 s_bf16_fclass(bf16 a, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_f16_fclass(half a, int switches=0, half income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat s_f8_fclass(minifloat a, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_h8_fclass(minihalf a, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi_plus__)
float64 v_f32_fclass_b(float64 a, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_fclass_vb(float64 a, int switches, float64 income, bool64 predicate, bool polarity=0);
bfloat128 v_bf16_fclass_b(bfloat128 a, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_fclass_vb(bfloat128 a, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_f16_fclass_vb(half128 a, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_fclass_b(half128 a, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_fclass_vb(minifloat256 a, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_fclass_b(minifloat256 a, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_fclass_vb(minihalf256 a, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_fclass_b(minihalf256 a, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_13 FCLASS with LIMIT switch.
/// @brief Represents FCLASS instruction with LIMIT switch.
///
/// @param a           The input value (SRC1).
/// @param b           The high threshold (SRC2).
/// @param c           The low threshold (SRC3).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return 10bit-masks format describing the input value.
///
/// In addition to regular classification input is compared with the thresholds:
/// @code if abs(a) >= b { a < 0 ? -inf : +inf } @endcode
/// @code if exp(a) <= c { a < 0 ? -0   : +0   } @endcode
///
/// 10bit mask maps to the following classes: (returned for 16/32 bit datatypes) \n
/// | bit   | 0    | 1       | 2         | 3  | 4  | 5         | 6       | 7    | 8    | 9    |
/// |-------|------|---------|-----------|----|----|-----------|---------|------|------|------|
/// | class | -Inf | -normal | -denormal | -0 | +0 | +denormal | +normal | +Inf | sNaN | qNaN |
///
#endif
/// @{
#if defined(__greco_plus__)
float64 v_f32_fclass_limit_b(float64 a, float64 b, float64 c, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_fclass_limit_vb(float64 a, float64 b, float64 c, int switches, float64 income, bool64 predicate, bool polarity=0);
bfloat128 v_bf16_fclass_limit_b(bfloat128 a, bfloat128 b, bfloat128 c, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_fclass_limit_vb(bfloat128 a, bfloat128 b, bfloat128 c, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
half128 v_f16_fclass_limit_b(half128 a, half128 b, half128 c, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_fclass_limit_vb(half128 a, half128 b, half128 c, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_fclass_limit_b(minifloat256 a, minifloat256 b, minifloat256 c, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_fclass_limit_vb(minifloat256 a, minifloat256 b, minifloat256 c, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_fclass_limit_b(minihalf256 a, minihalf256 b, minihalf256 c, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_fclass_limit_vb(minihalf256 a, minihalf256 b, minihalf256 c, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


/// @section sec2 Arithmetic
/// @subsection subsec2_14 FIND_FIRST
/// @brief Represents FIND_FIRST instruction.
///
/// @param a           Input value (SRC1).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return First 0/1 bit position.
///
/// @switches Allowed switches are:
///     - [SET] - Indicates whether the first bit to look for is 0 or 1.
///         \li SW_FIND_ZERO
///         \li SW_FIND_ONE
///     - [DIR] - Indicates the direction of the search.
///         \li SW_LSB
///         \li SW_MSB
///
/// @{
uint8_t s_f32_find_first(float a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
uint8_t s_bf16_find_first(bf16 a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
uint8_t s_f16_find_first(half a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
uint8_t s_f8_find_first(minifloat a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_h8_find_first(minihalf a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
#endif
uint8_t s_i32_find_first(int32_t a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u32_find_first(uint32_t a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_i16_find_first(int16_t a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u16_find_first(uint16_t a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_i8_find_first(int8_t a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u8_find_first(uint8_t a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
uchar256 v_f32_find_first_b(float64 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_f32_find_first_vb(float64 a, int switches, uchar256 income, bool64 predicate, bool polarity=0);
#if defined(__gaudi_plus__)
uchar256 v_bf16_find_first_b(bfloat128 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_bf16_find_first_vb(bfloat128 a, int switches, uchar256 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
uchar256 v_f16_find_first_b(half128 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_f16_find_first_vb(half128 a, int switches, uchar256 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
uchar256 v_f8_find_first_b(minifloat256 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_f8_find_first_vb(minifloat256 a, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_h8_find_first_b(minihalf256 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_h8_find_first_vb(minihalf256 a, int switches, uchar256 income, bool256 predicate, bool polarity=0);
#endif
uchar256 v_i32_find_first_b(int64 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_i32_find_first_vb(int64 a, int switches, uchar256 income, bool64 predicate, bool polarity=0);
uchar256 v_u32_find_first_b(uint64 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u32_find_first_vb(uint64 a, int switches, uchar256 income, bool64 predicate, bool polarity=0);
uchar256 v_i16_find_first_b(short128 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_i16_find_first_vb(short128 a, int switches, uchar256 income, bool128 predicate, bool polarity=0);
uchar256 v_u16_find_first_b(ushort128 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u16_find_first_vb(ushort128 a, int switches, uchar256 income, bool128 predicate, bool polarity=0);
uchar256 v_i8_find_first_b(char256 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_i8_find_first_vb(char256 a, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_find_first_b(uchar256 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_find_first_vb(uchar256 a, int switches, uchar256 income, bool256 predicate, bool polarity=0);
/// @}


//
// ------ FORM_FP_NUMBER
//

/// @section sec1 Arithmetic
/// @subsection subsec2_15 Form Floating point number
/// @brief Represents FORM_FP_NUMBER instruction.
///
/// @param a           Exponent (SRC1).
/// @param b           Sign (SRC2).
/// @param c           Mantissa (SRC3).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Constructed floating number.
///
/// @switches Allowed switches are:
///     \li SW_ADD_BIAS - Add bias to exponent (SRC1) prior to operation.
///     \li SW_FORCE_SIGN0 - Force sign bit at DEST to 0.
///     \li SW_FORCE_SIGN1 - Force sign bit at DEST to 1.
///     \li SW_EXP_IS_NUM - Treat the value in the source as an INT8 number, shift left by mantissa size and mask un-relevant bits.
///     \li SW_SIGN_LSB - When set, takes the SIGN from the LSB of the src instead from its sign bit.
#if defined(__gaudi2_plus__)
/// @switches
///     - [SPECIAL_FUNC] - Acceleration for special function calculations.
///         \li SW_PRE_SQRT_RSQRT
///         \li SW_POST_SQRT
///         \li SW_POST_RSQRT
///         \li SW_POST_RECIP
#endif
#if defined(__doron1__)
///         \li SW_PRE_LOG_FUNC - it is used only for FP32.
#endif
///
/// SW_FORCE_SIGN0, SW_FORCE_SIGN1, SW_SIGN_LSB are mutually exclusive. \n
/// @note In intrinsic functions with '_ie_' infix, SW_EXP_IS_NUM switch is set by default.
///
/// @{
float64 v_f32_form_fp_num_b(float64 a, float64 b, float64 c, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_form_fp_num_vb(float64 a, float64 b, float64 c, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_form_fp_num_ie_b(char256 a, float64 b, float64 c, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_form_fp_num_ie_vb(char256 a, float64 b, float64 c, int switches, float64 income, bool64 predicate, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_form_fp_num_b(bfloat128 a, bfloat128 b, bfloat128 c, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_form_fp_num_vb(bfloat128 a, bfloat128 b, bfloat128 c, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_form_fp_num_ie_b(char256 a, bfloat128 b, bfloat128 c, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_form_fp_num_ie_vb(char256 a, bfloat128 b, bfloat128 c, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_f16_form_fp_num_b(half128 a, half128 b, half128 c, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_form_fp_num_vb(half128 a, half128 b, half128 c, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_form_fp_num_ie_b(char256 a, half128 b, half128 c, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_form_fp_num_ie_vb(char256 a, half128 b, half128 c, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_form_fp_num_b(minifloat256 a, minifloat256 b, minifloat256 c, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_form_fp_num_vb(minifloat256 a, minifloat256 b, minifloat256 c, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_form_fp_num_ie_b(char256 a, minifloat256 b, minifloat256 c, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_form_fp_num_ie_vb(char256 a, minifloat256 b, minifloat256 c, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_form_fp_num_b(minihalf256 a, minihalf256 b, minihalf256 c, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_form_fp_num_vb(minihalf256 a, minihalf256 b, minihalf256 c, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_form_fp_num_ie_b(char256 a, minihalf256 b, minihalf256 c, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_form_fp_num_ie_vb(char256 a, minihalf256 b, minihalf256 c, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


//
// ------ GEN_ADDR
//

/// @section sec7 Load
/// @subsection subsec7_1 GEN_ADDR
/// @brief Represents GEN_ADDR instruction.
///
/// @param inx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Pointer in global address space, pointing to the selected tensor element.
///
#if defined(__greco_plus__)
/// @switches Allowed switches are:
///     - [DT]
///         \li SW_DT_INT8
///         \li SW_DT_INT16
///         \li SW_DT_INT32
///         \li SW_DT_UINT8
///         \li SW_DT_UINT16
///         \li SW_DT_UINT32
///         \li SW_DT_BF16
///         \li SW_DT_FP32
///         \li SW_DT_FP16
/// @switches
///     \li SW_DT_OVERRIDE - When set, tensor data-type is taken from DT switch instead of from tensor descriptor.
///
#endif
#if defined(__doron1__)
///     \li SW_PAD - padding mode to verify whether address calculation results in out-of-bound (even partially) or not.
#endif
/// @{
__global void *gen_addr(int5 inx, int8_t tensor, int switches=0, __global void *income={}, bool predicate=1, bool polarity=0);
/// @}


//
// ------ GET_LUT_ENTRY_AND_INTERVAL_START
//


/// @section sec8 LUT
/// @subsection subsec8_1 GET_LUT_ENTRY_AND_INTERVAL_START
/// @brief Represents GET_LUT_ENTRY_AND_INTERVAL_START instruction.
///
/// @param src         Input value (SRC1).
/// @param shift       Significand shift
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Pair of the interval and its offset for the special function approximation.
///
/// @switches Allowed switches are:
///     - [FUNC_VARIANT] - Determines the variant of the function, when no switch is set -
///                        all functions other then tanh/sqrt/rsqrt/sin/cos.
///         \li SW_LUT_TANH
///         \li SW_LUT_SQRT_RSQRT
///         \li SW_LUT_SIN_COS
///         \li SW_LUT_LOG
#if defined(__greco_plus__)
///         \li SW_LUT_OPT
///         \li SW_LUT_EXP0
#endif
///
/// @{
uint64_float64 v_f32_get_lut_entry_and_interval_start_b(float64 src, const int8_t shift, int switches=0, uint64_float64 income={}, bool predicate=1, bool polarity=0);
uint64_float64 v_f32_get_lut_entry_and_interval_start_vb(float64 src, const int8_t shift, int switches, uint64_float64 income, bool64 predicate, bool polarity=0);
#if defined(__gaudi_plus__)
ushort128_bfloat128 v_bf16_get_lut_entry_and_interval_start_b(bfloat128 src, const int8_t shift, int switches=0, ushort128_bfloat128 income={}, bool predicate=1, bool polarity=0);
ushort128_bfloat128 v_bf16_get_lut_entry_and_interval_start_vb(bfloat128 src, const int8_t shift, int switches, ushort128_bfloat128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
ushort128_half128 v_f16_get_lut_entry_and_interval_start_b(half128 src, const int8_t shift, int switches=0, ushort128_half128 income={}, bool predicate=1, bool polarity=0);
ushort128_half128 v_f16_get_lut_entry_and_interval_start_vb(half128 src, const int8_t shift, int switches, ushort128_half128 income, bool128 predicate, bool polarity=0);
#endif
/// @}


//
// ------ LD_L
//


/// @section sec7 Load
/// @subsection subsec7_2 LD_L
/// @brief Represents LD_L instruction.
///
/// @param addr        Address to read from (SRC1).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded scalar value from local memory.
///
/// @switches Allowed switches are:
///     \li SW_MMIO - load from MMIO.
///     \li SW_SLM  - load from SLM (the default).
#if defined(__greco_plus__)
///     \li SW_LOCK - Lock the address - relevant when SW_MMIO is set, and only for lockable registers.
#endif
#if defined(__doron1__)
///     \li SW_ABS_ADDR 
#endif
///
/// @{
float s_f32_ld_l(uint32_t addr, int switches=0, float income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_bf16_ld_l(uint32_t addr, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_f16_ld_l(uint32_t addr, int switches=0, half income={}, bool predicate=1, bool polarity=0);
#endif
int32_t s_i32_ld_l(uint32_t addr, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_u32_ld_l(uint32_t addr, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_i16_ld_l(uint32_t addr, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_u16_ld_l(uint32_t addr, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_i8_ld_l(uint32_t addr, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u8_ld_l(uint32_t addr, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
bool s_i1_ld_l(uint32_t addr, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi2_plus__)
minifloat s_f8_ld_l(uint32_t addr, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_h8_ld_l(uint32_t addr, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ LD_L_V
//


/// @section sec7 Load
/// @subsection subsec7_3 LD_L_V
/// @brief Represents LD_L_V instruction.
///
/// @param addr        Address to read from (SRC1).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded vector value from local memory.
///
#if defined(__gaudi2_plus__)
/// @switches Allowed switches are:
///     - [AUTO_INC_V]
///         \li SW_AUTO_INC_1_V - Increment \p addr by 1x256B.
///         \li SW_AUTO_INC_2_V - Increment \p addr by 2x256B.
///         \li SW_AUTO_INC_4_V - Increment \p addr by 4x256B.
#endif
#if defined(__doron1__)
///     \li SW_ABS_ADDR
///     \li SW_ADDR_CALC
#endif
///
/// @{
float64 v_f32_ld_l_v_b(uint32_t addr, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_ld_l_v_vb(uint32_t addr, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_ld_l_v_b(uint32_t addr, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
half128 v_f16_ld_l_v_vb(uint32_t addr, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_ld_l_v_b(uint32_t addr, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
#endif
int64 v_i32_ld_l_v_b(uint32_t addr, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ld_l_v_b(uint32_t addr, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_ld_l_v_b(uint32_t addr, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_ld_l_v_b(uint32_t addr, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_ld_l_v_b(uint32_t addr, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_ld_l_v_b(uint32_t addr, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_ld_l_v_b(uint32_t addr, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
float64 v_f32_ld_l_v_vb(uint32_t addr, int switches, float64 income, bool64 predicate, bool polarity=0);
int64 v_i32_ld_l_v_vb(uint32_t addr, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ld_l_v_vb(uint32_t addr, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_ld_l_v_vb(uint32_t addr, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_ld_l_v_vb(uint32_t addr, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_ld_l_v_vb(uint32_t addr, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_ld_l_v_vb(uint32_t addr, int switches, uchar256 income, bool256 predicate, bool polarity=0);
bool256 v_i1_ld_l_v_vb(uint32_t addr, int switches, bool256 income, bool256 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_ld_l_v_vb(uint32_t addr, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_ld_l_v_b(uint32_t addr, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_ld_l_v_vb(uint32_t addr, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_ld_l_v_b(uint32_t addr, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ LD_L_V_HIGH
//

/// @section sec7 Load
/// @subsection subsec7_4 LD_L_V_HIGH
/// @brief Represents LD_L_V_HIGH instruction.
///
/// @param addr        Address to read from (SRC1).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded half vector value from local memory.
///
#if defined(__gaudi2_plus__)
/// @switches Allowed switches are:
///     - [AUTO_INC_V]
///         \li SW_AUTO_INC_1_V - Increment \p addr by 1x256B.
///         \li SW_AUTO_INC_2_V - Increment \p addr by 2x256B.
///         \li SW_AUTO_INC_4_V - Increment \p addr by 4x256B.
#endif
///
/// @{
float64 v_f32_ld_l_v_high_vb(uint32_t addr, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_ld_l_v_high_b(uint32_t addr, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_ld_l_v_high_vb(uint32_t addr, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_ld_l_v_high_b(uint32_t addr, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
half128 v_f16_ld_l_v_high_vb(uint32_t addr, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_ld_l_v_high_b(uint32_t addr, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
#endif
int64 v_i32_ld_l_v_high_vb(uint32_t addr, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_ld_l_v_high_b(uint32_t addr, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ld_l_v_high_vb(uint32_t addr, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ld_l_v_high_b(uint32_t addr, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_ld_l_v_high_vb(uint32_t addr, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_ld_l_v_high_b(uint32_t addr, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_ld_l_v_high_vb(uint32_t addr, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_ld_l_v_high_b(uint32_t addr, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_ld_l_v_high_vb(uint32_t addr, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_ld_l_v_high_b(uint32_t addr, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_ld_l_v_high_vb(uint32_t addr, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_ld_l_v_high_b(uint32_t addr, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_ld_l_v_high_vb(uint32_t addr, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_i1_ld_l_v_high_b(uint32_t addr, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi2_plus__)
minifloat256 v_f8_ld_l_v_high_vb(uint32_t addr, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_ld_l_v_high_b(uint32_t addr, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_ld_l_v_high_vb(uint32_t addr, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_ld_l_v_high_b(uint32_t addr, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ LD_L_V_LOW
//


/// @section sec7 Load
/// @subsection subsec7_5 LD_L_V_LOW
/// @brief Represents LD_L_V_LOW instruction.
///
/// @param addr        Address to read from (SRC1).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded half vector value from local memory.
///
#if defined(__gaudi2_plus__)
/// @switches Allowed switches are:
///     - [AUTO_INC_V]
///         \li SW_AUTO_INC_1_V - Increment \p addr by 1x256B.
///         \li SW_AUTO_INC_2_V - Increment \p addr by 2x256B.
///         \li SW_AUTO_INC_4_V - Increment \p addr by 4x256B.
#endif
///
/// @{
float64 v_f32_ld_l_v_low_vb(uint32_t addr, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_ld_l_v_low_b(uint32_t addr, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_ld_l_v_low_vb(uint32_t addr, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_ld_l_v_low_b(uint32_t addr, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
half128 v_f16_ld_l_v_low_vb(uint32_t addr, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_ld_l_v_low_b(uint32_t addr, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
#endif
int64 v_i32_ld_l_v_low_vb(uint32_t addr, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_ld_l_v_low_b(uint32_t addr, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ld_l_v_low_vb(uint32_t addr, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ld_l_v_low_b(uint32_t addr, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_ld_l_v_low_vb(uint32_t addr, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_ld_l_v_low_b(uint32_t addr, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_ld_l_v_low_vb(uint32_t addr, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_ld_l_v_low_b(uint32_t addr, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_ld_l_v_low_vb(uint32_t addr, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_ld_l_v_low_b(uint32_t addr, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_ld_l_v_low_vb(uint32_t addr, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_ld_l_v_low_b(uint32_t addr, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_ld_l_v_low_vb(uint32_t addr, int switches, bool256 income, bool256 predicate, bool polarity=0);
bool256 v_i1_ld_l_v_low_b(uint32_t addr, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi2_plus__)
minifloat256 v_f8_ld_l_v_low_vb(uint32_t addr, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_ld_l_v_low_b(uint32_t addr, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_ld_l_v_low_vb(uint32_t addr, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_ld_l_v_low_b(uint32_t addr, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ LD_G
//

/// @section sec7 Load
/// @subsection subsec7_6 LD_G
/// @brief Represents LD_G instruction. Loads a single scalar value from global memory, if the destination is a vector register, the value is being replicated across all lanes.
///
/// @param addr        Address to read from (SRC1).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded scalar value from global memory.
///
#if defined(__greco_plus__)
/// @switches Allowed switches are:
///     \li SW_L0CS - Loaded data won't be written in data-cache.
///     \li SW_EV_HINT - Loaded cache-line will be the data-cache eviction candidate in next LD_G/ST_G/PREFETCH.
#endif
#if defined(__gaudi2_plus__)
///     \li SW_PD - Prefetch disable - the HW prefetcher will not perform prefetch based on this operation.
#endif
#if defined(__doron1__)
///     \li SW_EXC - Exclusive Read from an address in global memory.
#endif
/// @{
float s_f32_ld_g(__global void *addr, int switches=0, float income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_bf16_ld_g(__global void *addr, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_f16_ld_g(__global void *addr, int switches=0, half income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat s_f8_ld_g(__global void *addr, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_h8_ld_g(__global void *addr, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
#endif
int32_t s_i32_ld_g(__global void *addr, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_u32_ld_g(__global void *addr, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);

#if defined(__doron1__)
int32_t_pair_t  s_i32_x2_ld_g(__global void *addr, int switches=0, int32_t_pair_t  income={}, bool predicate=1, bool polarity=0);
uint32_t_pair_t s_u32_x2_ld_g(__global void *addr, int switches=0, uint32_t_pair_t income={}, bool predicate=1, bool polarity=0);
#endif

int16_t s_i16_ld_g(__global void *addr, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_u16_ld_g(__global void *addr, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_i8_ld_g(__global void *addr, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u8_ld_g(__global void *addr, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
bool s_i1_ld_g(__global void *addr, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_ld_g(__global void *addr, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
half128 v_f16_ld_g(__global void *addr, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_ld_g(__global void *addr, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_ld_g(__global void *addr, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_ld_g(__global void *addr, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);

half128 v_f16_ld_g_vb(__global void *addr, int switches, half128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_ld_g_vb(__global void *addr, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
minifloat256 v_f8_ld_g_vb(__global void *addr, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_ld_g_vb(__global void *addr, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
bool256 v_i1_ld_g_vb(__global void *addr, int switches, bool256 income, bool256 predicate, bool polarity=0);
#endif
float64 v_f32_ld_g(__global void *addr, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_ld_g(__global void *addr, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ld_g(__global void *addr, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_ld_g(__global void *addr, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_ld_g(__global void *addr, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_ld_g(__global void *addr, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_ld_g(__global void *addr, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi2_plus__)
float64 v_f32_ld_g_vb(__global void *addr, int switches, float64 income, bool64 predicate, bool polarity=0);
int64 v_i32_ld_g_vb(__global void *addr, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ld_g_vb(__global void *addr, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_ld_g_vb(__global void *addr, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_ld_g_vb(__global void *addr, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_ld_g_vb(__global void *addr, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_ld_g_vb(__global void *addr, int switches, uchar256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec7 Load
/// @subsection subsec7_7 LD_G for IRF
/// @brief Represents LD_G instruction for loading int5 data.
///
/// @param addr        Address to read from (SRC1).
/// @param dimmask     Dimension mask.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded tuple of 5 from global memory.
///
/// @switches Allowed switches are:
///     \li SW_BV64 - Load 64bit into 2 consecutive dimensions of IRF.
///     \li SW_L0CS - Loaded data won't be written in data-cache.
///     \li SW_EV_HINT - Loaded cache-line will be the data-cache eviction candidate in next LD_G/ST_G/PREFETCH.
#endif
#if defined(__gaudi2_plus__)
///     \li SW_PD - Prefetch disable - the HW prefetcher will not perform prefetch based on this operation.
///
#endif
/// @{
#if defined(__greco_plus__)
int5 i_i32_ld_g(__global void *addr, int dimmask, int switches, int5 income, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec7 Load
/// @subsection subsec7_8 LD_G for IRF with INCREMENT switch
/// @brief Represents LD_G instruction for loading int5 data with INCREMENT switch.
///
/// @param addr        Address to read from (SRC1).
/// @param dimmask     Dimension mask.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded tuple of 5 from global memory.
///
/// @switches Allowed switches are:
///     \li SW_BV64 - Load 64bit into 2 consecutive dimensions of IRF.
///     \li SW_L0CS - Loaded data won't be written in data-cache.
///     \li SW_EV_HINT - Loaded cache-line will be the data-cache eviction candidate in next LD_G/ST_G/PREFETCH.
#endif
#if defined(__gaudi2_plus__)
///     \li SW_PD - Prefetch disable - the HW prefetcher will not perform prefetch based on this operation.
///
#endif
/// @{
#if defined(__greco_plus__)
int5 i_i32_ld_g_inc(__global void **addr, int dimmask, int switches, int5 income, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec7 Load
/// @subsection subsec7_9 LD_G for IRF with SW_INC_* switch
/// @brief Represents LD_G instruction with SW_INC_* switch.
///
/// @param addr        Points to the address to write to (SRC1).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded scalar value from global memory.
///
/// @switches Allowed switches are:
///     - [AUTO_INC]
///         \li SW_INC_1 - Increment \p addr by 1B.
///         \li SW_INC_2 - Increment \p addr by 2B.
///         \li SW_INC_4 - Increment \p addr by 4B.
///         \li SW_INC_8 - Increment \p addr by 8B.
/// @switches
///     \li SW_L0CS - Stored data won't be written in data-cache.
///     \li SW_EV_HINT - Stored cache-line will be the data-cache eviction candidate in next LD_G/ST_G/PREFETCH.
///     \li SW_PD - Prefetch disable - the HW prefetcher will not perform prefetch based on this operation.
///
/// @note If none of the switches SW_INC_* is specified, increment value is calculated automatically
/// using the size of the value to load.
///
#endif
/// @{
#if defined(__greco_plus__)
float s_f32_ld_g_inc(__global float **addr, int switches=0, float income={}, bool predicate=1, bool polarity=0);
bf16 s_bf16_ld_g_inc(__global bf16 **addr, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
half s_f16_ld_g_inc(__global half **addr, int switches=0, half income={}, bool predicate=1, bool polarity=0);
minifloat s_f8_ld_g_inc(__global minifloat **addr, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_h8_ld_g_inc(__global minihalf **addr, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
int32_t s_i32_ld_g_inc(__global int32_t **addr, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_u32_ld_g_inc(__global uint32_t **addr, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_i16_ld_g_inc(__global int16_t **addr, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_u16_ld_g_inc(__global uint16_t **addr, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_i8_ld_g_inc(__global int8_t **addr, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u8_ld_g_inc(__global uint8_t **addr, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
bool s_i1_ld_g_inc(__global bool **addr, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
#endif

#if defined(__doron1__)
int32_t_pair_t  s_i32_x2_ld_g_inc(__global int32_t **addr,  int switches=0, int32_t_pair_t  income={}, bool predicate=1, bool polarity=0);
uint32_t_pair_t s_u32_x2_ld_g_inc(__global uint32_t **addr, int switches=0, uint32_t_pair_t income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec7 Load
/// @subsection subsec7_10 LD_G with PARTIAL switch
/// @brief Represents LD_G instruction with PARTIAL switch. Loads a single scalar value from global memory and replicates it across a subset of lanes of a vector register.
///
/// @param addr        Address to read from (SRC1).
/// @param size        Size in elements minus 1.
/// @param offset      Offset in elements.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded value.
///
/// @switches Allowed switches are:
///     \li SW_L0CS - Loaded data won't be written in data-cache.
///
#endif
/// @{
#if defined(__greco_plus__)
float64 v_f32_ld_g_partial_b(__global void *addr, int8_t size, int8_t offset, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_ld_g_partial_b(__global void *addr, int8_t size, int8_t offset, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_ld_g_partial_b(__global void *addr, int8_t size, int8_t offset, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_ld_g_partial_b(__global void *addr, int8_t size, int8_t offset, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ld_g_partial_b(__global void *addr, int8_t size, int8_t offset, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_ld_g_partial_b(__global void *addr, int8_t size, int8_t offset, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_ld_g_partial_b(__global void *addr, int8_t size, int8_t offset, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_ld_g_partial_b(__global void *addr, int8_t size, int8_t offset, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_ld_g_partial_b(__global void *addr, int8_t size, int8_t offset, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
float64 v_f32_ld_g_partial_vb(__global void *addr, int8_t size, int8_t offset, int switches, float64 income, bool64 predicate, bool polarity=0);
half128 v_f16_ld_g_partial_vb(__global void *addr, int8_t size, int8_t offset, int switches, half128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_ld_g_partial_vb(__global void *addr, int8_t size, int8_t offset, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
minifloat256 v_f8_ld_g_partial_vb(__global void *addr, int8_t size, int8_t offset, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_ld_g_partial_vb(__global void *addr, int8_t size, int8_t offset, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
int64 v_i32_ld_g_partial_vb(__global void *addr, int8_t size, int8_t offset, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ld_g_partial_vb(__global void *addr, int8_t size, int8_t offset, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_ld_g_partial_vb(__global void *addr, int8_t size, int8_t offset, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_ld_g_partial_vb(__global void *addr, int8_t size, int8_t offset, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_ld_g_partial_vb(__global void *addr, int8_t size, int8_t offset, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_ld_g_partial_vb(__global void *addr, int8_t size, int8_t offset, int switches, uchar256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec7 Load
/// @subsection subsec7_11 LD_G with PARTIAL and INCREMENT switch
/// @brief Represents LD_G instruction with PARTIAL and INCREMENT switch. Represents LD_G instruction with PARTIAL switch. Loads a single scalar value from global memory and replicates it across a subset of lanes of a vector register,it also increments the input address point.
///
/// @param addr        Address to read from (SRC1).
/// @param size        Size in elements minus 1.
/// @param offset      Offset in elements.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed switches are:
///     \li SW_L0CS - Loaded data won't be written in data-cache.
///
#endif
/// @{
#if defined(__greco_plus__)
float64 v_f32_ld_g_inc_partial_b(__global void **addr, int8_t size, int8_t offset, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_ld_g_inc_partial_b(__global void **addr, int8_t size, int8_t offset, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_ld_g_inc_partial_b(__global void **addr, int8_t size, int8_t offset, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_ld_g_inc_partial_b(__global void **addr, int8_t size, int8_t offset, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ld_g_inc_partial_b(__global void **addr, int8_t size, int8_t offset, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_ld_g_inc_partial_b(__global void **addr, int8_t size, int8_t offset, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_ld_g_inc_partial_b(__global void **addr, int8_t size, int8_t offset, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_ld_g_inc_partial_b(__global void **addr, int8_t size, int8_t offset, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_ld_g_inc_partial_b(__global void **addr, int8_t size, int8_t offset, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
float64 v_f32_ld_g_inc_partial_vb(__global void **addr, int8_t size, int8_t offset, int switches, float64 income, bool64 predicate, bool polarity=0);
half128 v_f16_ld_g_inc_partial_vb(__global void **addr, int8_t size, int8_t offset, int switches, half128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_ld_g_inc_partial_vb(__global void **addr, int8_t size, int8_t offset, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
minifloat256 v_f8_ld_g_inc_partial_vb(__global void **addr, int8_t size, int8_t offset, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_ld_g_inc_partial_vb(__global void **addr, int8_t size, int8_t offset, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
int64 v_i32_ld_g_inc_partial_vb(__global void **addr, int8_t size, int8_t offset, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ld_g_inc_partial_vb(__global void **addr, int8_t size, int8_t offset, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_ld_g_inc_partial_vb(__global void **addr, int8_t size, int8_t offset, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_ld_g_inc_partial_vb(__global void **addr, int8_t size, int8_t offset, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_ld_g_inc_partial_vb(__global void **addr, int8_t size, int8_t offset, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_ld_g_inc_partial_vb(__global void **addr, int8_t size, int8_t offset, int switches, uchar256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


//
// ------ LD_TNSR
//

/// @section sec7 Load
/// @subsection subsec7_12 LD_TNSR
/// @brief Represents LD_TNSR instruction.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded values.
///
#if defined(__greco_plus__)
/// @switches Allowed switches are:
///     - [UNPACK_DT]
///         \li SW_UNPCK_16_TO_32
///         \li SW_UNPCK_8_TO_16
///         \li SW_UNPCK_8_TO_32
///         \li SW_UNPCK_4_TO_8
/// @switches
///     \li SW_UNPACK - When set, load pipe will perform unpacking according to UNPACK_DT.
///     \li SW_L0CS - Loaded data won't be written in SB-cache.
///
#endif
/// @{
float64 v_f32_ld_tnsr_b(int5 ndx, const int8_t tensor, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_ld_tnsr_b(int5 ndx, const int8_t tensor, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ld_tnsr_b(int5 ndx, const int8_t tensor, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_ld_tnsr_b(int5 ndx, const int8_t tensor, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_ld_tnsr_b(int5 ndx, const int8_t tensor, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_ld_tnsr_b(int5 ndx, const int8_t tensor, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_ld_tnsr_b(int5 ndx, const int8_t tensor, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_ld_tnsr_b(int5 ndx, const int8_t tensor, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
float64 v_f32_ld_tnsr_vb(int5 ndx, int8_t tensor, int switches, float64 income, bool64 predicate, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_vb(int5 ndx, int8_t tensor, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
int64 v_i32_ld_tnsr_vb(int5 ndx, int8_t tensor, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ld_tnsr_vb(int5 ndx, int8_t tensor, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_ld_tnsr_vb(int5 ndx, int8_t tensor, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_ld_tnsr_vb(int5 ndx, int8_t tensor, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_ld_tnsr_vb(int5 ndx, int8_t tensor, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_ld_tnsr_vb(int5 ndx, int8_t tensor, int switches, uchar256 income, bool256 predicate, bool polarity=0);
bool256 v_i1_ld_tnsr_vb(int5 ndx, int8_t tensor, int switches, bool256 income, bool256 predicate, bool polarity=0);
#endif
#if defined(__gaudi_plus__)
bfloat128 v_bf16_ld_tnsr_b(int5 ndx, int8_t tensor, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
half128 v_f16_ld_tnsr_b(int5 ndx, int8_t tensor, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_ld_tnsr_vb(int5 ndx, int8_t tensor, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined (__gaudi2_plus__)
minifloat256 v_f8_ld_tnsr_b(int5 ndx, int8_t tensor, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_ld_tnsr_vb(int5 ndx, int8_t tensor, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_ld_tnsr_b(int5 ndx, int8_t tensor, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_ld_tnsr_vb(int5 ndx, int8_t tensor, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
/// @}

#if defined(__gaudi2_plus__)
/// @section sec7 Load
/// @subsection subsec7_13 LD_TNSR with DIRECT switch
/// @brief Represents LD_TNSR instruction with DIRECT switch.
///
/// @param addr1       Tensor load address low (SRC1).
/// @param addr2       Tensor load address high (SRC2).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded values.
///
/// @switches Allowed switches are:
///     - [UNPACK_DT]
///         \li SW_UNPCK_16_TO_32
///         \li SW_UNPCK_8_TO_16
///         \li SW_UNPCK_8_TO_32
///         \li SW_UNPCK_4_TO_8
/// @switches
///     \li SW_UNPACK - When set, load pipe will perform unpacking according to UNPACK_DT.
///     \li SW_L0CS - Loaded data won't be written in SB-cache.
#endif
///
/// @{
#if defined (__gaudi2_plus__)
float64 v_f32_ld_tnsr_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_ld_tnsr_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ld_tnsr_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_ld_tnsr_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_ld_tnsr_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_ld_tnsr_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_ld_tnsr_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_ld_tnsr_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_ld_tnsr_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_ld_tnsr_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_ld_tnsr_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_ld_tnsr_direct_vb(uint32_t addr1, uint32_t addr2, int switches, float64 income, bool64 predicate, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_direct_vb(uint32_t addr1, uint32_t addr2, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
int64 v_i32_ld_tnsr_direct_vb(uint32_t addr1, uint32_t addr2, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ld_tnsr_direct_vb(uint32_t addr1, uint32_t addr2, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_ld_tnsr_direct_vb(uint32_t addr1, uint32_t addr2, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_ld_tnsr_direct_vb(uint32_t addr1, uint32_t addr2, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_ld_tnsr_direct_vb(uint32_t addr1, uint32_t addr2, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_ld_tnsr_direct_vb(uint32_t addr1, uint32_t addr2, int switches, uchar256 income, bool256 predicate, bool polarity=0);
bool256 v_i1_ld_tnsr_direct_vb(uint32_t addr1, uint32_t addr2, int switches, bool256 income, bool256 predicate, bool polarity=0);
half128 v_f16_ld_tnsr_direct_vb(uint32_t addr1, uint32_t addr2, int switches, half128 income, bool128 predicate, bool polarity=0);
minifloat256 v_f8_ld_tnsr_direct_vb(uint32_t addr1, uint32_t addr2, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_ld_tnsr_direct_vb(uint32_t addr1, uint32_t addr2, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


#if defined(__gaudi_plus__)
/// @section sec7 Load
/// @subsection subsec7_14 LD_TNSR with PARTIAL switch
/// @brief Represents LD_TNSR instruction with PARTIAL switch. Loads elements from global memory into a subset of vector lanes.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param size        Size in elements minus 1.
/// @param offset      Offset in elements.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded values.
///
#endif
#if defined(__greco_plus__)
/// @switches Allowed switches are:
///     - [UNPACK_DT]
///         \li SW_UNPCK_16_TO_32
///         \li SW_UNPCK_8_TO_16
///         \li SW_UNPCK_8_TO_32
///         \li SW_UNPCK_4_TO_8
/// @switches
///     \li SW_UNPACK - When set, load pipe will perform unpacking according to UNPACK_DT.
///     \li SW_L0CS - Loaded data won't be written in SB-cache.
///
#endif
/// @{
#if defined(__gaudi_plus__)
float64 v_f32_ld_tnsr_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_ld_tnsr_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ld_tnsr_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_ld_tnsr_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_ld_tnsr_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_ld_tnsr_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_ld_tnsr_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_ld_tnsr_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_ld_tnsr_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, float64 income, bool64 predicate, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
int64 v_i32_ld_tnsr_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ld_tnsr_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_ld_tnsr_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_ld_tnsr_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_ld_tnsr_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_ld_tnsr_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, uchar256 income, bool256 predicate, bool polarity=0);
bool256 v_i1_ld_tnsr_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, bool256 income, bool256 predicate, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
half128 v_f16_ld_tnsr_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_ld_tnsr_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_ld_tnsr_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_ld_tnsr_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_ld_tnsr_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_ld_tnsr_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


#if defined(__gaudi2_plus__)
/// @section sec7 Load
/// @subsection subsec7_15 LD_TNSR with PARTIAL and DIRECT
/// @brief Represents LD_TNSR instruction with PARTIAL and DIRECT switches.
///
/// @param addr1       Tensor load address low (SRC1).
/// @param addr2       Tensor load address high (SRC2).
/// @param size        Size in elements minus 1.
/// @param offset      Offset in elements.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded values.
///
/// @switches Allowed switches are:
///     - [UNPACK_DT]
///         \li SW_UNPCK_16_TO_32
///         \li SW_UNPCK_8_TO_16
///         \li SW_UNPCK_8_TO_32
///         \li SW_UNPCK_4_TO_8
/// @switches
///     \li SW_UNPACK - When set, load pipe will perform unpacking according to UNPACK_DT.
///     \li SW_L0CS - Loaded data won't be written in SB-cache.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
float64 v_f32_ld_tnsr_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_ld_tnsr_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ld_tnsr_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_ld_tnsr_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_ld_tnsr_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_ld_tnsr_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_ld_tnsr_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_ld_tnsr_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_ld_tnsr_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_ld_tnsr_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_ld_tnsr_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_ld_tnsr_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, float64 income, bool64 predicate, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
int64 v_i32_ld_tnsr_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ld_tnsr_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_ld_tnsr_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_ld_tnsr_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_ld_tnsr_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_ld_tnsr_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, uchar256 income, bool256 predicate, bool polarity=0);
bool256 v_i1_ld_tnsr_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, bool256 income, bool256 predicate, bool polarity=0);
half128 v_f16_ld_tnsr_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, half128 income, bool128 predicate, bool polarity=0);
minifloat256 v_f8_ld_tnsr_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_ld_tnsr_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, minihalf256 income, bool128 predicate, bool polarity=0);
#endif
/// @}


//
// ------ LD_TNSR_HIGH
//

/// @section sec7 Load
/// @subsection subsec7_16 LD_TNSR_HIGH
/// @brief Represents LD_TNSR_HIGH instruction.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded values.
///
#if defined(__greco_plus__)
/// @switches Allowed switches are:
///     - [UNPACK_DT]
///         \li SW_UNPCK_16_TO_32
///         \li SW_UNPCK_8_TO_16
///         \li SW_UNPCK_8_TO_32
///         \li SW_UNPCK_4_TO_8
/// @switches
///     \li SW_UNPACK - When set, load pipe will perform unpacking according to UNPACK_DT.
///     \li SW_L0CS - Loaded data won't be written in SB-cache.
#endif
///
/// @{
float64 v_f32_ld_tnsr_high_b(int5 ndx, const int8_t tensor, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_ld_tnsr_high_b(int5 ndx, const int8_t tensor, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ld_tnsr_high_b(int5 ndx, const int8_t tensor, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_ld_tnsr_high_b(int5 ndx, const int8_t tensor, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_ld_tnsr_high_b(int5 ndx, const int8_t tensor, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_ld_tnsr_high_b(int5 ndx, const int8_t tensor, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_ld_tnsr_high_b(int5 ndx, const int8_t tensor, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_ld_tnsr_high_b(int5 ndx, const int8_t tensor, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
float64 v_f32_ld_tnsr_high_vb(int5 ndx, int8_t tensor, int switches, float64 income, bool64 predicate, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_high_vb(int5 ndx, int8_t tensor, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
int64 v_i32_ld_tnsr_high_vb(int5 ndx, int8_t tensor, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ld_tnsr_high_vb(int5 ndx, int8_t tensor, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_ld_tnsr_high_vb(int5 ndx, int8_t tensor, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_ld_tnsr_high_vb(int5 ndx, int8_t tensor, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_ld_tnsr_high_vb(int5 ndx, int8_t tensor, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_ld_tnsr_high_vb(int5 ndx, int8_t tensor, int switches, uchar256 income, bool256 predicate, bool polarity=0);
bool256 v_i1_ld_tnsr_high_vb(int5 ndx, int8_t tensor, int switches, bool256 income, bool256 predicate, bool polarity=0);
#endif
#if defined(__gaudi_plus__)
bfloat128 v_bf16_ld_tnsr_high_b(int5 ndx, int8_t tensor, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
half128 v_f16_ld_tnsr_high_b(int5 ndx, int8_t tensor, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_ld_tnsr_high_vb(int5 ndx, int8_t tensor, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_ld_tnsr_high_b(int5 ndx, int8_t tensor, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_ld_tnsr_high_b(int5 ndx, int8_t tensor, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_ld_tnsr_high_vb(int5 ndx, int8_t tensor, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_ld_tnsr_high_vb(int5 ndx, int8_t tensor, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


#if defined(__gaudi2_plus__)
/// @section sec7 Load
/// @subsection subsec7_17 LD_TNSR_HIGH with DIRECT switch
/// @brief Represents LD_TNSR_HIGH instruction with DIRECT switch.
///
/// @param addr1       Tensor load address low (SRC1).
/// @param addr2       Tensor load address high (SRC2).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded value.
///
/// @switches Allowed switches are:
///     - [UNPACK_DT]
///         \li SW_UNPCK_16_TO_32
///         \li SW_UNPCK_8_TO_16
///         \li SW_UNPCK_8_TO_32
///         \li SW_UNPCK_4_TO_8
/// @switches
///     \li SW_UNPACK - When set, load pipe will perform unpacking according to UNPACK_DT.
///     \li SW_L0CS - Loaded data won't be written in SB-cache.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
float64 v_f32_ld_tnsr_high_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_high_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_ld_tnsr_high_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ld_tnsr_high_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_ld_tnsr_high_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_ld_tnsr_high_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_ld_tnsr_high_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_ld_tnsr_high_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_ld_tnsr_high_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_ld_tnsr_high_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_ld_tnsr_high_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_ld_tnsr_high_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_ld_tnsr_high_direct_vb(uint32_t addr1, uint32_t addr2, int switches, float64 income, bool64 predicate, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_high_direct_vb(uint32_t addr1, uint32_t addr2, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
int64 v_i32_ld_tnsr_high_direct_vb(uint32_t addr1, uint32_t addr2, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ld_tnsr_high_direct_vb(uint32_t addr1, uint32_t addr2, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_ld_tnsr_high_direct_vb(uint32_t addr1, uint32_t addr2, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_ld_tnsr_high_direct_vb(uint32_t addr1, uint32_t addr2, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_ld_tnsr_high_direct_vb(uint32_t addr1, uint32_t addr2, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_ld_tnsr_high_direct_vb(uint32_t addr1, uint32_t addr2, int switches, uchar256 income, bool256 predicate, bool polarity=0);
bool256 v_i1_ld_tnsr_high_direct_vb(uint32_t addr1, uint32_t addr2, int switches, bool256 income, bool256 predicate, bool polarity=0);
half128 v_f16_ld_tnsr_high_direct_vb(uint32_t addr1, uint32_t addr2, int switches, half128 income, bool128 predicate, bool polarity=0);
minifloat256 v_f8_ld_tnsr_high_direct_vb(uint32_t addr1, uint32_t addr2, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_ld_tnsr_high_direct_vb(uint32_t addr1, uint32_t addr2, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec7 Load
/// @subsection subsec7_18 LD_TNSR_HIGH with PARTIAL switch
/// @brief Represents LD_TNSR_HIGH instruction with PARTIAL switch.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param size        Size in elements minus 1.
/// @param offset      Offset in elements.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded value.
///
/// @switches Allowed switches are:
///     - [UNPACK_DT]
///         \li SW_UNPCK_16_TO_32
///         \li SW_UNPCK_8_TO_16
///         \li SW_UNPCK_8_TO_32
///         \li SW_UNPCK_4_TO_8
/// @switches
///     \li SW_UNPACK - When set, load pipe will perform unpacking according to UNPACK_DT.
///     \li SW_L0CS - Loaded data won't be written in SB-cache.
///
#endif
/// @{
#if defined(__greco_plus__)
float64 v_f32_ld_tnsr_high_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_high_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_ld_tnsr_high_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ld_tnsr_high_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_ld_tnsr_high_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_ld_tnsr_high_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_ld_tnsr_high_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_ld_tnsr_high_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_ld_tnsr_high_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_ld_tnsr_high_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_ld_tnsr_high_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, float64 income, bool64 predicate, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_high_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
int64 v_i32_ld_tnsr_high_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ld_tnsr_high_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_ld_tnsr_high_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_ld_tnsr_high_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_ld_tnsr_high_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_ld_tnsr_high_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, uchar256 income, bool256 predicate, bool polarity=0);
bool256 v_i1_ld_tnsr_high_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, bool256 income, bool256 predicate, bool polarity=0);
half128 v_f16_ld_tnsr_high_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_ld_tnsr_high_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_ld_tnsr_high_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_ld_tnsr_high_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_ld_tnsr_high_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


#if defined(__gaudi2_plus__)
/// @section sec7 Load
/// @subsection subsec7_19 LD_TNSR_HIGH with PARTIAL and DIRECT switch
/// @brief Represents LD_TNSR_HIGH instruction with PARTIAL and DIRECT switches.
///
/// @param addr1       Tensor load address low (SRC1).
/// @param addr2       Tensor load address high (SRC2).
/// @param size        Size in elements minus 1.
/// @param offset      Offset in elements.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded value.
///
/// @switches Allowed switches are:
///     - [UNPACK_DT]
///         \li SW_UNPCK_16_TO_32
///         \li SW_UNPCK_8_TO_16
///         \li SW_UNPCK_8_TO_32
///         \li SW_UNPCK_4_TO_8
/// @switches
///     \li SW_UNPACK - When set, load pipe will perform unpacking according to UNPACK_DT.
///     \li SW_L0CS - Loaded data won't be written in SB-cache.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
float64 v_f32_ld_tnsr_high_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_high_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_ld_tnsr_high_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ld_tnsr_high_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_ld_tnsr_high_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_ld_tnsr_high_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_ld_tnsr_high_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_ld_tnsr_high_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_ld_tnsr_high_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_ld_tnsr_high_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_ld_tnsr_high_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_ld_tnsr_high_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_ld_tnsr_high_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, float64 income, bool64 predicate, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_high_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
int64 v_i32_ld_tnsr_high_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ld_tnsr_high_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_ld_tnsr_high_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_ld_tnsr_high_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_ld_tnsr_high_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_ld_tnsr_high_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, uchar256 income, bool256 predicate, bool polarity=0);
bool256 v_i1_ld_tnsr_high_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, bool256 income, bool256 predicate, bool polarity=0);
half128 v_f16_ld_tnsr_high_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, half128 income, bool128 predicate, bool polarity=0);
minifloat256 v_f8_ld_tnsr_high_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_ld_tnsr_high_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


//
// ------ LD_TNSR_LOW
//

/// @section sec7 Load
/// @subsection subsec7_20 LD_TNSR_LOW
/// @brief Represents LD_TNSR_LOW instruction.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded value.
///
#if defined(__greco_plus__)
/// @switches Allowed switches are:
///     - [UNPACK_DT]
///         \li SW_UNPCK_16_TO_32
///         \li SW_UNPCK_8_TO_16
///         \li SW_UNPCK_8_TO_32
///         \li SW_UNPCK_4_TO_8
/// \par
///     \li SW_UNPACK - When set, load pipe will perform unpacking according to UNPACK_DT.
///     \li SW_L0CS - Loaded data won't be written in SB-cache.
#endif
///
/// @{
float64 v_f32_ld_tnsr_low_b(int5 ndx, const int8_t tensor, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_ld_tnsr_low_b(int5 ndx, const int8_t tensor, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ld_tnsr_low_b(int5 ndx, const int8_t tensor, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_ld_tnsr_low_b(int5 ndx, const int8_t tensor, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_ld_tnsr_low_b(int5 ndx, const int8_t tensor, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_ld_tnsr_low_b(int5 ndx, const int8_t tensor, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_ld_tnsr_low_b(int5 ndx, const int8_t tensor, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_ld_tnsr_low_b(int5 ndx, const int8_t tensor, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
float64 v_f32_ld_tnsr_low_vb(int5 ndx, int8_t tensor, int switches, float64 income, bool64 predicate, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_low_vb(int5 ndx, int8_t tensor, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
int64 v_i32_ld_tnsr_low_vb(int5 ndx, int8_t tensor, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ld_tnsr_low_vb(int5 ndx, int8_t tensor, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_ld_tnsr_low_vb(int5 ndx, int8_t tensor, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_ld_tnsr_low_vb(int5 ndx, int8_t tensor, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_ld_tnsr_low_vb(int5 ndx, int8_t tensor, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_ld_tnsr_low_vb(int5 ndx, int8_t tensor, int switches, uchar256 income, bool256 predicate, bool polarity=0);
bool256 v_i1_ld_tnsr_low_vb(int5 ndx, int8_t tensor, int switches, bool256 income, bool256 predicate, bool polarity=0);
#endif
#if defined(__gaudi_plus__)
bfloat128 v_bf16_ld_tnsr_low_b(int5 ndx, int8_t tensor, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
half128 v_f16_ld_tnsr_low_b(int5 ndx, int8_t tensor, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_ld_tnsr_low_vb(int5 ndx, int8_t tensor, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_ld_tnsr_low_b(int5 ndx, int8_t tensor, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_ld_tnsr_low_vb(int5 ndx, int8_t tensor, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_ld_tnsr_low_b(int5 ndx, int8_t tensor, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_ld_tnsr_low_vb(int5 ndx, int8_t tensor, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


#if defined(__gaudi2_plus__)
/// @section sec7 Load
/// @subsection subsec7_21 LD_TNSR_LOW with DIRECT switch
/// @brief Represents LD_TNSR_LOW instruction with DIRECT switch.
///
/// @param addr1       Tensor load address low (SRC1).
/// @param addr2       Tensor load address high (SRC2).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded value.
///
/// @switches Allowed switches are:
///     - [UNPACK_DT]
///         \li SW_UNPCK_16_TO_32
///         \li SW_UNPCK_8_TO_16
///         \li SW_UNPCK_8_TO_32
///         \li SW_UNPCK_4_TO_8
/// \par
///     \li SW_UNPACK - When set, load pipe will perform unpacking according to UNPACK_DT.
///     \li SW_L0CS - Loaded data won't be written in SB-cache.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
float64 v_f32_ld_tnsr_low_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_low_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_ld_tnsr_low_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ld_tnsr_low_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_ld_tnsr_low_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_ld_tnsr_low_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_ld_tnsr_low_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_ld_tnsr_low_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_ld_tnsr_low_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_ld_tnsr_low_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_ld_tnsr_low_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_ld_tnsr_low_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_ld_tnsr_low_direct_vb(uint32_t addr1, uint32_t addr2, int switches, float64 income, bool64 predicate, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_low_direct_vb(uint32_t addr1, uint32_t addr2, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
int64 v_i32_ld_tnsr_low_direct_vb(uint32_t addr1, uint32_t addr2, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ld_tnsr_low_direct_vb(uint32_t addr1, uint32_t addr2, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_ld_tnsr_low_direct_vb(uint32_t addr1, uint32_t addr2, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_ld_tnsr_low_direct_vb(uint32_t addr1, uint32_t addr2, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_ld_tnsr_low_direct_vb(uint32_t addr1, uint32_t addr2, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_ld_tnsr_low_direct_vb(uint32_t addr1, uint32_t addr2, int switches, uchar256 income, bool256 predicate, bool polarity=0);
bool256 v_i1_ld_tnsr_low_direct_vb(uint32_t addr1, uint32_t addr2, int switches, bool256 income, bool256 predicate, bool polarity=0);
half128 v_f16_ld_tnsr_low_direct_vb(uint32_t addr1, uint32_t addr2, int switches, half128 income, bool128 predicate, bool polarity=0);
minifloat256 v_f8_ld_tnsr_low_direct_vb(uint32_t addr1, uint32_t addr2, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_ld_tnsr_low_direct_vb(uint32_t addr1, uint32_t addr2, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec7 Load
/// @subsection subsec7_22 LD_TNSR_LOW with PARTIAL switch
/// @brief Represents LD_TNSR_LOW instruction with PARTIAL switch.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param size        Size in elements minus 1.
/// @param offset      Offset in elements.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded value.
///
/// @switches Allowed switches are:
///     - [UNPACK_DT]
///         \li SW_UNPCK_16_TO_32
///         \li SW_UNPCK_8_TO_16
///         \li SW_UNPCK_8_TO_32
///         \li SW_UNPCK_4_TO_8
/// \par
///     \li SW_UNPACK - When set, load pipe will perform unpacking according to UNPACK_DT.
///     \li SW_L0CS - Loaded data won't be written in SB-cache.
///
#endif
/// @{
#if defined(__greco_plus__)
float64 v_f32_ld_tnsr_low_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_low_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_ld_tnsr_low_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ld_tnsr_low_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_ld_tnsr_low_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_ld_tnsr_low_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_ld_tnsr_low_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_ld_tnsr_low_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_ld_tnsr_low_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_ld_tnsr_low_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_ld_tnsr_low_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, float64 income, bool64 predicate, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_low_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
int64 v_i32_ld_tnsr_low_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ld_tnsr_low_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_ld_tnsr_low_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_ld_tnsr_low_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_ld_tnsr_low_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_ld_tnsr_low_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, uchar256 income, bool256 predicate, bool polarity=0);
bool256 v_i1_ld_tnsr_low_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, bool256 income, bool256 predicate, bool polarity=0);
half128 v_f16_ld_tnsr_low_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_ld_tnsr_low_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_ld_tnsr_low_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_ld_tnsr_low_partial_b(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_ld_tnsr_low_partial_vb(int5 ndx, int8_t tensor, int8_t size, int8_t offset, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


#if defined(__gaudi2_plus__)
/// @section sec7 Load
/// @subsection subsec7_23 LD_TNSR_LOW with PARTIAL and DIRECT switch
/// @brief Represents LD_TNSR_LOW instruction with PARTIAL and DIRECT switches.
///
/// @param addr1       Tensor load address low (SRC1).
/// @param addr2       Tensor load address high (SRC2).
/// @param size        Size in elements minus 1.
/// @param offset      Offset in elements.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded value.
///
/// @switches Allowed switches are:
///     - [UNPACK_DT]
///         \li SW_UNPCK_16_TO_32
///         \li SW_UNPCK_8_TO_16
///         \li SW_UNPCK_8_TO_32
///         \li SW_UNPCK_4_TO_8
/// \par
///     \li SW_UNPACK - When set, load pipe will perform unpacking according to UNPACK_DT.
///     \li SW_L0CS - Loaded data won't be written in SB-cache.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
float64 v_f32_ld_tnsr_low_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_low_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_ld_tnsr_low_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_ld_tnsr_low_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_ld_tnsr_low_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_ld_tnsr_low_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_ld_tnsr_low_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_ld_tnsr_low_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_ld_tnsr_low_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_ld_tnsr_low_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_ld_tnsr_low_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_ld_tnsr_low_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_ld_tnsr_low_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, float64 income, bool64 predicate, bool polarity=0);
bfloat128 v_bf16_ld_tnsr_low_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
int64 v_i32_ld_tnsr_low_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_ld_tnsr_low_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_ld_tnsr_low_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_ld_tnsr_low_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_ld_tnsr_low_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_ld_tnsr_low_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, uchar256 income, bool256 predicate, bool polarity=0);
bool256 v_i1_ld_tnsr_low_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, bool256 income, bool256 predicate, bool polarity=0);
half128 v_f16_ld_tnsr_low_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, half128 income, bool128 predicate, bool polarity=0);
minifloat256 v_f8_ld_tnsr_low_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_ld_tnsr_low_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


//
// ------ LD_TNSR_CNVRT
//

#if defined(__doron1__)
/// @brief Represents LD_TNSR_CNVRT instruction.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded value.
///
/// @switches Allowed switches are:
///     - [PART]
///         \li SW_LOW - Write to low 128B of VRF
///         \li SW_HIGH - Write to high 128B of VRF
/// \par
///     - [AUTO_INC_DIM]
///         \li SW_INC_DIM0 - Increment dim by 0
///         \li SW_INC_DIM1 - Increment dim by 1
///         \li SW_INC_DIM2 - Increment dim by 2
///         \li SW_INC_DIM3 - Increment dim by 3
///         \li SW_INC_DIM4 - Increment dim by 4
/// \par
///     \li SW_CLIP_FP  - Clip +/-inf to +/-max_normal_value_fp
///     \li SW_L0CS - Loaded data won't be written in SB-cache.
///
#endif
/// @{
#if defined(__doron1__)
bfloat128 v_bf16_ld_tnsr_cnvrt_b(int5 ndx, const int8_t tensor, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_ld_tnsr_cnvrt_b(int5 ndx, const int8_t tensor, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);

bfloat128 v_bf16_ld_tnsr_cnvrt_vb(int5 ndx, const int8_t tensor, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
half128 v_f16_ld_tnsr_cnvrt_vb(int5 ndx, const int8_t tensor, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
/// @}


#if defined(__doron1__)
/// @brief Represents LD_TNSR_CNVRT instruction with DIRECT switch.
///
/// @param addr1       Tensor load address low (SRC1).
/// @param addr2       Tensor load address high (SRC2).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded value.
///
/// @switches Allowed switches are:
///     - [PART]
///         \li SW_LOW - Write to low 128B of VRF
///         \li SW_HIGH - Write to high 128B of VRF
/// \par
///     \li SW_CLIP_FP  - Clip +/-inf to +/-max_normal_value_fp
///     \li SW_L0CS - Loaded data won't be written in SB-cache.
///
#endif
/// @{
#if defined(__doron1__)
bfloat128 v_bf16_ld_tnsr_cnvrt_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_ld_tnsr_cnvrt_direct_b(uint32_t addr1, uint32_t addr2, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);

bfloat128 v_bf16_ld_tnsr_cnvrt_direct_vb(uint32_t addr1, uint32_t addr2, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
half128 v_f16_ld_tnsr_cnvrt_direct_vb(uint32_t addr1, uint32_t addr2, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
/// @}


#if defined(__doron1__)
/// @brief Represents LD_TNSR_CNVRT instruction with PARTIAL switch.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param size        Size in elements minus 1.
/// @param offset      Offset in elements.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded value.
///
/// @switches Allowed switches are:
///     - [PART]
///         \li SW_LOW - Write to low 128B of VRF
///         \li SW_HIGH - Write to high 128B of VRF
/// \par
///     - [AUTO_INC_DIM]
///         \li SW_INC_DIM0 - Increment dim by 0
///         \li SW_INC_DIM1 - Increment dim by 1
///         \li SW_INC_DIM2 - Increment dim by 2
///         \li SW_INC_DIM3 - Increment dim by 3
///         \li SW_INC_DIM4 - Increment dim by 4
/// \par
///     \li SW_CLIP_FP  - Clip +/-inf to +/-max_normal_value_fp
///     \li SW_L0CS - Loaded data won't be written in SB-cache.
///
#endif
/// @{
#if defined(__doron1__)
bfloat128 v_bf16_ld_tnsr_cnvrt_partial_b(int5 ndx, const int8_t tensor, int8_t size, int8_t offset, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_ld_tnsr_cnvrt_partial_b(int5 ndx, const int8_t tensor, int8_t size, int8_t offset, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);

bfloat128 v_bf16_ld_tnsr_cnvrt_partial_vb(int5 ndx, const int8_t tensor, int8_t size, int8_t offset, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
half128 v_f16_ld_tnsr_cnvrt_partial_vb(int5 ndx, const int8_t tensor, int8_t size, int8_t offset, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
/// @}


#if defined(__doron1__)
/// @brief Represents LD_TNSR_CNVRT instruction with PARTIAL and DIRECT switch.
///
/// @param addr1       Tensor load address low (SRC1).
/// @param addr2       Tensor load address high (SRC2).
/// @param size        Size in elements minus 1.
/// @param offset      Offset in elements.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The loaded value.
///
/// @switches Allowed switches are:
///     - [PART]
///         \li SW_LOW - Write to low 128B of VRF
///         \li SW_HIGH - Write to high 128B of VRF
/// \par
///     \li SW_CLIP_FP  - Clip +/-inf to +/-max_normal_value_fp
///     \li SW_L0CS - Loaded data won't be written in SB-cache.
///
#endif
/// @{
#if defined(__doron1__)
bfloat128 v_bf16_ld_tnsr_cnvrt_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_ld_tnsr_cnvrt_partial_direct_b(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);

bfloat128 v_bf16_ld_tnsr_cnvrt_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
half128 v_f16_ld_tnsr_cnvrt_partial_direct_vb(uint32_t addr1, uint32_t addr2, int8_t size, int8_t offset, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
/// @}


//
// ------ LOOKUP
//

/// @section sec8 LUT
/// @subsection subsec8_2 Lookup
/// @brief Represents LOOKUP instruction.
///
/// @param a           The offsets from the beginning of the entry (SRC1).
/// @param fid         Describes the function_id being accessed, \n
///                    and the part in case of a 16 bits operation.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @switches Allowed switches:
///     - [LOOKUP_DT]
///         \li SW_BV32
#if defined(__goya__)
///         \li SW_BV16_LOW
///         \li SW_BV16_HIGH
///         \li SW_BV8_0
///         \li SW_BV8_1
///         \li SW_BV8_2
///         \li SW_BV8_3
#endif
///
#if defined(__gaudi_plus__)
/// \par
///     \li SW_UPPER_HALF - Force lookup in upper half of 64-entries slot.
#endif
#if defined(__greco_plus__)
///     \li SW_LUT_PTR - When set FUNC_ID should be taken from the given SRF.
#endif
#if defined(__gaudi2_plus__)
///     \li SW_SBCD - Loaded data won't be written in SB-cache.
#endif
///
/// @{
float64 v_f32_lookup(uint64 a, int fid, int switches, float64 income, bool predicate=1, bool polarity=0);
int64 v_i32_lookup(uint64 a, int fid, int switches, int64 income, bool predicate=1, bool polarity=0);
uint64 v_u32_lookup(uint64 a, int fid, int switches, uint64 income, bool predicate=1, bool polarity=0);
short128 v_i16_lookup(ushort128 a, int fid, int switches, short128 income, bool predicate=1, bool polarity=0);
ushort128 v_u16_lookup(ushort128 a, int fid, int switches, ushort128 income, bool predicate=1, bool polarity=0);
char256 v_i8_lookup(uchar256 a, int fid, int switches, char256 income, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_lookup(uint64 a, int fid, int switches, bfloat128 income, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi_plus__)
half128 v_f16_lookup(uint64 a, int fid, int switches, half128 income, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__goya__)
/// @section sec8 Lookup
/// @subsection subsec8_3 LOOKUP_C0
/// @brief Represents LOOKUP_C0 instruction.
///
/// @param a           The offsets from the beginning of the entry (SRC1).
/// @param fid         Describes the function_id being accessed, \n
///                    and the part in case of a 16 bits operation.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @switches Allowed switches:
///     - [LOOKUP_DT]
///         \li SW_BV32
///         \li SW_BV16_LOW
///         \li SW_BV16_HIGH
///         \li SW_BV8_0
///         \li SW_BV8_1
///         \li SW_BV8_2
///         \li SW_BV8_3
///
#endif
/// @{
#if defined(__goya__)
float64 v_f32_lookup_c0(uint64 a, int fid, int switches, float64 income, bool predicate=1, bool polarity=0);
int64 v_i32_lookup_c0(uint64 a, int fid, int switches, int64 income, bool predicate=1, bool polarity=0);
uint64 v_u32_lookup_c0(uint64 a, int fid, int switches, uint64 income, bool predicate=1, bool polarity=0);
short128 v_i16_lookup_c0(ushort128 a, int fid, int switches, short128 income, bool predicate=1, bool polarity=0);
ushort128 v_u16_lookup_c0(ushort128 a, int fid, int switches, ushort128 income, bool predicate=1, bool polarity=0);
char256 v_i8_lookup_c0(uchar256 a, int fid, int switches, char256 income, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__goya__)
/// @section sec8 Lookup
/// @subsection subsec8_4 LOOKUP_C1C2
/// @brief Represents LOOKUP_C1C2 instruction.
///
/// @param a           The offsets from the beginning of the entry (SRC1).
/// @param fid         Describes the function_id being accessed, \n
///                    and the part in case of a 16 bits operation.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @switches Allowed switches:
///     - [LOOKUP_DT]
///         \li SW_BV32
///         \li SW_BV16_LOW
///         \li SW_BV16_HIGH
///         \li SW_BV8_0
///         \li SW_BV8_1
///         \li SW_BV8_2
///         \li SW_BV8_3
///
#endif
/// @{
#if defined(__goya__)
float128 v_f32_lookup_c1c2(uint64 a, int fid, int switches, float128 income, bool predicate=1, bool polarity=0);
short256 v_i16_lookup_c1c2(ushort128 a, int fid, int switches, short256 income, bool predicate=1, bool polarity=0);
ushort256 v_u16_lookup_c1c2(ushort128 a, int fid, int switches, ushort256 income, bool predicate=1, bool polarity=0);
char512 v_i8_lookup_c1c2(uchar256 a, int fid, int switches, char512 income, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__gaudi_plus__)
/// @section sec8 Lookup
/// @subsection subsec8_5 LOOKUP_1C
/// @brief Represents LOOKUP_1C instruction.
///
/// @param a           The offsets from the beginning of the entry (SRC1).
/// @param fid         Describes the function_id being accessed, \n
///                    and the part in case of a 16 bits operation.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @switches Allowed switches:
///     - [LOOKUP_DT]
///         \li SW_BV32
///         \li SW_BV16
#endif
#if defined(__greco_plus__)
///         \li SW_BV8_0
///         \li SW_BV8_1
#endif
#if defined(__gaudi_plus__)
/// \par
///     \li SW_UPPER_HALF - Force lookup in upper half of 64-entries slot.
#endif
#if defined(__greco_plus__)
///     \li SW_LUT_PTR - When set FUNC_ID should be taken from the given SRF.
#endif
#if defined(__gaudi2_plus__)
///     \li SW_X2 - LOOKUP at double performance, LOOKUP of 2 lanes in 1 cycle (BV8_0/BV8_1/BV16 data types).
///     \li SW_SBCD - Loaded data won't be written in SB-cache.
///
#endif
/// @{
#if defined(__gaudi_plus__)
float64 v_f32_lookup_1c(uint64 a, int fid, int switches, float64 income, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_lookup_1c(ushort128 a, int fid, int switches, bfloat128 income, bool predicate=1, bool polarity=0);
int64 v_i32_lookup_1c(uint64 a, int fid, int switches, int64 income, bool predicate=1, bool polarity=0);
short128 v_i16_lookup_1c(ushort128 a, int fid, int switches, short128 income, bool predicate=1, bool polarity=0);
uint64 v_u32_lookup_1c(uint64 a, int fid, int switches, uint64 income, bool predicate=1, bool polarity=0);
ushort128 v_u16_lookup_1c(ushort128 a, int fid, int switches, ushort128 income, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_f16_lookup_1c(ushort128 a, int fid, int switches, half128 income, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__gaudi_plus__)
/// @section sec8 Lookup
/// @subsection subsec8_6 LOOKUP_2C
/// @brief Represents LOOKUP_2C instruction.
///
/// @param a           The offsets from the beginning of the entry (SRC1).
/// @param fid         Describes the function_id being accessed, \n
///                    and the part in case of a 16 bits operation.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @switches Allowed switches:
///     - [LOOKUP_DT]
///         \li SW_BV32
///         \li SW_BV16
#endif
#if defined(__greco_plus__)
///         \li SW_BV8_0
///         \li SW_BV8_1
#endif
#if defined(__gaudi_plus__)
/// \par
///     \li SW_UPPER_HALF - Force lookup in upper half of 64-entries slot.
#endif
#if defined(__greco_plus__)
///     \li SW_LUT_PTR - When set FUNC_ID should be taken from the given SRF.
#endif
#if defined(__gaudi2_plus__)
///     \li SW_X2 - LOOKUP at double performance, LOOKUP of 2 lanes in 1 cycle (BV8_0/BV8_1/BV16 data types).
///     \li SW_SBCD - Loaded data won't be written in SB-cache.
///
#endif
/// @{
#if defined(__gaudi_plus__)
float128 v_f32_lookup_2c(uint64 a, int fid, int switches, float128 income, bool predicate=1, bool polarity=0);
bfloat256 v_bf16_lookup_2c(ushort128 a, int fid, int switches, bfloat256 income, bool predicate=1, bool polarity=0);
int128 v_i32_lookup_2c(uint64 a, int fid, int switches, int128 income, bool predicate=1, bool polarity=0);
uint128 v_u32_lookup_2c(uint64 a, int fid, int switches, uint128 income, bool predicate=1, bool polarity=0);
short256 v_i16_lookup_2c(ushort128 a, int fid, int switches, short256 income, bool predicate=1, bool polarity=0);
ushort256 v_u16_lookup_2c(ushort128 a, int fid, int switches, ushort256 income, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half256 v_f16_lookup_2c(ushort128 a, int fid, int switches, half256 income, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ MAC
//

/// @section sec2 Arithmetic
/// @subsection subsec2_16 MAC
/// @brief Represents MAC instruction.
///
/// @param a           The first SRC operand to MAC (SRC1).
/// @param b           The second SRC operand to MAC (SRC2).
/// @param accumulator DEST operand to MAC.
/// @param switches    Switches of MAC instructions.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Updated value of accumulator \c acc=acc+(a*b).
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturates the result (valid for integer data-types only).
///     \li SW_NO_NEG - default, \c acc=acc+a*b.
///     \li SW_NEG - When set, \c acc=acc-(a*b).
#if defined(__gaudi__) || defined(__gaudib__) || defined(__goya__)
///                 (valid for float data-types only).
#endif
///
/// @{
float s_f32_mac(float a, float b, float accumulator, int switches=0, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_bf16_mac(bf16 a, bf16 b, bf16 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_f16_mac(half a, half b, half accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
int32_t s_i16_mac(int16_t a, int16_t b, int32_t accumulator, int switches=0, bool predicate=1, bool polarity=0);
uint32_t s_u16_mac(uint16_t a, uint16_t b, uint32_t accumulator, int switches=0, bool predicate=1, bool polarity=0);
#if defined(__goya__) || defined(__greco_plus__)
int32_t s_i8_mac(int8_t a, int8_t b, int32_t accumulator, int switches=0, bool predicate=1, bool polarity=0);
uint32_t s_u8_mac(uint8_t a, uint8_t b, uint32_t accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
float64 v_f32_mac_vb(float64 a, float64 b, float64 accumulator, int switches, bool64 predicate, bool polarity=0);
float64 v_f32_mac_b(float64 a, float64 b, float64 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_mac_vb(bfloat128 a, bfloat128 b, bfloat128 accumulator, int switches, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_mac_b(bfloat128 a, bfloat128 b, bfloat128 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
half128 v_f16_mac_vb(half128 a, half128 b, half128 accumulator, int switches, bool128 predicate, bool polarity=0);
half128 v_f16_mac_b(half128 a, half128 b, half128 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
int128 v_i16_mac_vb(short128 a, short128 b, int128 accumulator, int switches, bool128 predicate, bool polarity=0);
int128 v_i16_mac_b(short128 a, short128 b, int128 accumulator, int switches=0, bool predicate=1, bool polarity=0);
uint128 v_u16_mac_vb(ushort128 a, ushort128 b, uint128 accumulator, int switches, bool128 predicate, bool polarity=0);
uint128 v_u16_mac_b(ushort128 a, ushort128 b, uint128 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#if defined(__goya__) || defined(__greco_plus__)
int256 v_i8_mac_vb(char256 a, char256 b, int256 accumulator, int switches, bool256 predicate, bool polarity=0);
int256 v_i8_mac_b(char256 a, char256 b, int256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
uint256 v_u8_mac_vb(uchar256 a, uchar256 b, uint256 accumulator, int switches, bool256 predicate, bool polarity=0);
uint256 v_u8_mac_b(uchar256 a, uchar256 b, uint256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__gaudi_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_17 MAC with ACC_F32 switch
/// @brief Represents MAC instruction with ACC_F32 switch (32-bit accumulator).
///
/// @param a           The first SRC operand to MAC (SRC1).
/// @param b           The second SRC operand to MAC (SRC2).
/// @param accumulator DEST operand to MAC.
/// @param switches    Switches of MAC instructions.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Updated value of accumulator \c acc=acc+(a*b).
///
/// @switches Allowed switches are:
///     \li SW_NO_NEG - default, \c acc=acc+(a*b).
///     \li SW_NEG - When set, \c acc=acc-(a*b)
///
#endif
/// @{
#if defined(__gaudi_plus__)
float s_bf16_mac_acc32(bf16 a, bf16 b, float accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
float s_f16_mac_acc32(half a, half b, float accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi_plus__)
float128 v_bf16_mac_acc32_vb(bfloat128 a, bfloat128 b, float128 accumulator, int switches, bool128 predicate, bool polarity=0);
float128 v_bf16_mac_acc32_b(bfloat128 a, bfloat128 b, float128 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
float128 v_f16_mac_acc32_vb(half128 a, half128 b, float128 accumulator, int switches, bool128 predicate, bool polarity=0);
float128 v_f16_mac_acc32_b(half128 a, half128 b, float128 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
float s_f8_mac_acc32(minifloat a, minifloat b, float accumulator, int switches=0, bool predicate=1, bool polarity=0);
float s_h8_mac_acc32(minihalf a, minihalf b, float accumulator, int switches=0, bool predicate=1, bool polarity=0);
float256 v_f8_mac_acc32_vb(minifloat256 a, minifloat256 b, float256 accumulator, int switches, bool256 predicate, bool polarity=0);
float256 v_f8_mac_acc32_b(minifloat256 a, minifloat256 b, float256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
float256 v_h8_mac_acc32_vb(minihalf256 a, minihalf256 b, float256 accumulator, int switches, bool256 predicate, bool polarity=0);
float256 v_h8_mac_acc32_b(minihalf256 a, minihalf256 b, float256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}

#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_18 MAC with ACC_I16 switch
/// @brief Represents MAC instruction with ACC_I16 switch (16-bit accumulator).
///
/// @param a           The first SRC operand to MAC (SRC1).
/// @param b           The second SRC operand to MAC (SRC2).
/// @param accumulator DEST operand to MAC.
/// @param switches    Switches of MAC instructions.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Updated value of accumulator \c acc=acc+(a*b).
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturates the result (integer data-types only).
///     \li SW_NO_NEG - default, \c acc=acc+(a*b).
///     \li SW_NEG - When set, \c acc=acc-(a*b).
///
#endif
/// @{
#if defined(__greco_plus__)
int16_t s_i8_mac_acc16(int8_t a, int8_t b, int16_t accumulator, int switches=0, bool predicate=1, bool polarity=0);
uint16_t s_u8_mac_acc16(uint8_t a, uint8_t b, uint16_t accumulator, int switches=0, bool predicate=1, bool polarity=0);
short256 v_i8_mac_acc16_vb(char256 a, char256 b, short256 accumulator, int switches, bool256 predicate, bool polarity=0);
short256 v_i8_mac_acc16_b(char256 a, char256 b, short256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
ushort256 v_u8_mac_acc16_vb(uchar256 a, uchar256 b, ushort256 accumulator, int switches, bool256 predicate, bool polarity=0);
ushort256 v_u8_mac_acc16_b(uchar256 a, uchar256 b, ushort256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_19 MAC with ACC_I32 switch
/// @brief Represents MAC instruction with ACC_I32 switch (signed accumulator).
///
/// @param a           The first SRC operand to MAC (SRC1).
/// @param b           The second SRC operand to MAC (SRC2).
/// @param accumulator DEST operand to MAC.
/// @param switches    Switches of MAC instructions.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Updated value of accumulator \c acc=acc+(a*b).
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturates the result (integer data-types only).
///     \li SW_NO_NEG - default, \c acc=acc+(a*b).
///     \li SW_NEG - When set, \c acc=acc-(a*b).
///
#endif
/// @{
#if defined(__greco_plus__)
int32_t s_u8_mac_acc32(uint8_t a, uint8_t b, int32_t accumulator, int switches=0, bool predicate=1, bool polarity=0);
int32_t s_u16_mac_acc32(uint16_t a, uint16_t b, int32_t accumulator, int switches=0, bool predicate=1, bool polarity=0);
int256 v_u8_mac_acc32_vb(uchar256 a, uchar256 b, int256 accumulator, int switches, bool256 predicate, bool polarity=0);
int256 v_u8_mac_acc32_b(uchar256 a, uchar256 b, int256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
int128 v_u16_mac_acc32_vb(ushort128 a, ushort128 b, int128 accumulator, int switches, bool128 predicate, bool polarity=0);
int128 v_u16_mac_acc32_b(ushort128 a, ushort128 b, int128 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_20 MAC with ZP switch
/// @brief Represents MAC instruction with ZP switch.
///
/// @param a           The first SRC operand to MAC (SRC1).
/// @param b           The second SRC operand to MAC (SRC2).
/// @param zp          Zero-point value (to be passed in ZP_REG).
/// @param accumulator DEST operand to MAC.
/// @param switches    Switches of MAC instructions.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Updated value of accumulator \c acc=acc+(a*b)+(a*zp).
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturates the result (integer data-types only).
///     \li SW_NO_NEG - default, \c acc=acc+(a*b)+(a*zp).
///     \li SW_NEG - When set, \c acc=acc-(a*b)-(a*zp).
///     \li SW_NEG_ZP - When set, \c acc=acc+(a*b)-(a*zp).
///
/// When both SW_NEG and SW_NEG_ZP are set, \c acc=acc-(a*b)+(a*zp).
///
#endif
/// @{
#if defined(__greco_plus__)
int256 v_i8_mac_zp_vb(char256 a, char256 b, char256 zp, int256 accumulator, int switches, bool256 predicate, bool polarity=0);
int256 v_i8_mac_zp_b(char256 a, char256 b, char256 zp, int256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
uint256 v_u8_mac_zp_vb(uchar256 a, uchar256 b, uchar256 zp, uint256 accumulator, int switches, bool256 predicate, bool polarity=0);
uint256 v_u8_mac_zp_b(uchar256 a, uchar256 b, uchar256 zp, uint256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_21 MAC with ZP and ACC_I16 switches
/// @brief Represents MAC instruction with ZP and ACC_I16 switches.
///
/// @param a           The first SRC operand to MAC (SRC1).
/// @param b           The second SRC operand to MAC (SRC2).
/// @param zp          Zero-point value (to be passed in ZP_REG).
/// @param accumulator DEST operand to MAC.
/// @param switches    Switches of MAC instructions.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Updated value of accumulator \c acc=acc+(a*b)+(a*zp).
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturates the result (integer data-types only).
///     \li SW_NO_NEG - default, \c acc=acc+(a*b)+(a*zp).
///     \li SW_NEG - When set, \c acc=acc-(a*b)-(a*zp).
///     \li SW_NEG_ZP - When set, \c acc=acc+(a*b)-(a*zp).
///
/// When both SW_NEG and SW_NEG_ZP are set, \c acc=acc-(a*b)+(a*zp).
///
#endif
/// @{
#if defined(__greco_plus__)
short256 v_i8_mac_zp_acc16_vb(char256 a, char256 b, char256 zp, short256 accumulator, int switches, bool256 predicate, bool polarity=0);
short256 v_i8_mac_zp_acc16_b(char256 a, char256 b, char256 zp, short256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
ushort256 v_u8_mac_zp_acc16_vb(uchar256 a, uchar256 b, uchar256 zp, ushort256 accumulator, int switches, bool256 predicate, bool polarity=0);
ushort256 v_u8_mac_zp_acc16_b(uchar256 a, uchar256 b, uchar256 zp, ushort256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_22 MAC with ZP and ACC_I32 switches
/// @brief Represents MAC instruction with ZP and ACC_I32 switch.
///
/// @param a           The first SRC operand to MAC (SRC1).
/// @param b           The second SRC operand to MAC (SRC2).
/// @param zp          Zero-point value (to be passed in ZP_REG).
/// @param accumulator DEST operand to MAC.
/// @param switches    Switches of MAC instructions.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Updated value of accumulator \c acc=acc+(a*b)+(a*zp).
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturates the result (integer data-types only).
///     \li SW_NO_NEG - default, \c acc=acc+(a*b)+(a*zp).
///     \li SW_NEG - When set, \c acc=acc-(a*b)-(a*zp).
///     \li SW_NEG_ZP - When set, \c acc=acc+(a*b)-(a*zp).
///
/// When both SW_NEG and SW_NEG_ZP are set, \c acc=acc-(a*b)+(a*zp).
///
#endif
/// @{
#if defined(__greco_plus__)
int256 v_u8_mac_zp_acc32_vb(uchar256 a, uchar256 b, uchar256 zp, int256 accumulator, int switches, bool256 predicate, bool polarity=0);
int256 v_u8_mac_zp_acc32_b(uchar256 a, uchar256 b, uchar256 zp, int256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_23 MAC with X2 switch
/// @brief Represents MAC instruction with X2 switch.
///
/// @param a           The first SRC operand to MAC (SRC1).
/// @param b           The second SRC operand to MAC (SRC2).
/// @param c           The third SRC operand to MAC (SRC3).
/// @param d           The fourth SRC operand to MAC (SRC4).
/// @param accumulator DEST operand to MAC. (aka income)
/// @param switches    Switches of MAC instructions, like ST or NEG.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Updated value of accumulator.
/// For I8/U8 - \c acc=acc+(a*b)+(c*d), For F32 - \c acc.v1=acc+(a*b), \c acc.v2=acc+(c*d).
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturates the result (integer data-types only).
///     \li SW_NO_NEG - default, For I8/U8 - \c acc=acc+(a*b)+(c*d), \n
///     \li SW_NEG - When set, For I8/U8 - \c acc=acc-(a*b)-(c*d), \n
///                  For F32 - \c acc.v1=acc-(a*b), \c acc.v2=acc-(c*d).
///
/// @note float128 data-type that map to 2 vector registers of 64 float elements each. \n
/// In the formula above \p acc.v1 and \p acc.v2 refers to the first and second vectors respectively.
///
#endif
/// @{
#if defined(__greco_plus__)
int256 v_i8_mac_x2_vb(char256 a, char256 b, char256 c, char256 d, int256 accumulator, int switches, bool256 predicate, bool polarity=0);
int256 v_i8_mac_x2_b(char256 a, char256 b, char256 c, char256 d, int256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
uint256 v_u8_mac_x2_vb(uchar256 a, uchar256 b, uchar256 c, uchar256 d, uint256 accumulator, int switches, bool256 predicate, bool polarity=0);
uint256 v_u8_mac_x2_b(uchar256 a, uchar256 b, uchar256 c, uchar256 d, uint256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
float128 v_f32_mac_x2_vb(float128 a, float64 b, float64 c, float128 accumulator, int switches, bool128 predicate, bool polarity=0);
float128 v_f32_mac_x2_b(float128 a, float64 b, float64 c, float128 accumulator, int switches=0, bool predicate=1, bool polarity=0);
float128 v_f32_mac_x2_svv_vb(float a, float64 b, float64 c, float128 accumulator, int switches, bool128 predicate, bool polarity=0);
float128 v_f32_mac_x2_svv_b(float a, float64 b, float64 c, float128 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_24 MAC with X2 and ACC_I16 switches
/// @brief Represents MAC instruction with X2 and ACC_I16 switches.
///
/// @param a           The first SRC operand to MAC (SRC1).
/// @param b           The second SRC operand to MAC (SRC2).
/// @param c           The third SRC operand to MAC (SRC3).
/// @param d           The forth SRC operand to MAC (SRC4).
/// @param accumulator DEST operand to MAC.
/// @param switches    Switches of MAC instructions.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Updated value of accumulator \c acc=acc+(a*b)+(c*d).
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturates the result (integer data-types only).
///     \li SW_NO_NEG - default, \c acc=acc+(a*b)+(c*d).
///     \li SW_NEG - When set, \c acc=acc-(a*b)-(c*d).
///
#endif
/// @{
#if defined(__greco_plus__)
short256 v_i8_mac_x2_acc16_vb(char256 a, char256 b, char256 c, char256 d, short256 accumulator, int switches, bool256 predicate, bool polarity=0);
short256 v_i8_mac_x2_acc16_b(char256 a, char256 b, char256 c, char256 d, short256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
ushort256 v_u8_mac_x2_acc16_vb(uchar256 a, uchar256 b, uchar256 c, uchar256 d, ushort256 accumulator, int switches, bool256 predicate, bool polarity=0);
ushort256 v_u8_mac_x2_acc16_b(uchar256 a, uchar256 b, uchar256 c, uchar256 d, ushort256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_25 MAC with X2 and ACC_I32 switches
/// @brief Represents MAC instruction with X2 and ACC_I32 switches.
///
/// @param a           The first SRC operand to MAC (SRC1).
/// @param b           The second SRC operand to MAC (SRC2).
/// @param c           The third SRC operand to MAC (SRC3).
/// @param d           The forth SRC operand to MAC (SRC4).
/// @param accumulator DEST operand to MAC.
/// @param switches    Switches of MAC instructions.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Updated value of accumulator \c acc=acc+(a*b)+(c*d).
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturates the result (integer data-types only).
///     \li SW_NO_NEG - default, \c acc=acc+(a*b)+(c*d).
///     \li SW_NEG - When set, \c acc=acc-(a*b)-(c*d).
///
#endif
/// @{
#if defined(__greco_plus__)
int256 v_u8_mac_x2_acc32_vb(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int256 accumulator, int switches, bool256 predicate, bool polarity=0);
int256 v_u8_mac_x2_acc32_b(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_26 MAC with X2 and ZP switches
/// @brief Represents MAC instruction with X2 and ZP switches.
///
/// @param a           The first SRC operand to MAC (SRC1).
/// @param b           The second SRC operand to MAC (SRC2).
/// @param c           The third SRC operand to MAC (SRC3).
/// @param d           The forth SRC operand to MAC (SRC4).
/// @param zp          Zero-point value (to be passed in ZP_REG).
/// @param accumulator DEST operand to MAC.
/// @param switches    Switches of MAC instructions.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Updated value of accumulator \c acc=acc+(a*b)+(c*d)+(a*zp)+(c*zp).
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturates the result (integer data-types only).
///     \li SW_NO_NEG - default, \c acc=acc+(a*b)+(c*d)+(a*zp)+(c*zp).
///     \li SW_NEG - When set, \c acc=acc-(a*b)-(c*d)-(a*zp)-(c*zp).
///     \li SW_NEG_ZP - When set, \c acc=acc+(a*b)+(c*d)-(a*zp)-(c*zp).
///
/// When both SW_NEG and SW_NEG_ZP are set, \c acc=acc-(a*b)-(c*d)+(a*zp)+(c*zp).
///
#endif
/// @{
#if defined(__greco_plus__)
int256 v_i8_mac_x2_zp_vb(char256 a, char256 b, char256 c, char256 d, char256 zp, int256 accumulator, int switches, bool256 predicate, bool polarity=0);
int256 v_i8_mac_x2_zp_b(char256 a, char256 b, char256 c, char256 d, char256 zp, int256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
uint256 v_u8_mac_x2_zp_vb(uchar256 a, uchar256 b, uchar256 c, uchar256 d, uchar256 zp, uint256 accumulator, int switches, bool256 predicate, bool polarity=0);
uint256 v_u8_mac_x2_zp_b(uchar256 a, uchar256 b, uchar256 c, uchar256 d, uchar256 zp, uint256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_27 MAC with X2, ZP and ACC_I16 switches
/// @brief Represents MAC instruction with X2, ZP and ACC_I16 switches.
///
/// @param a           The first SRC operand to MAC (SRC1).
/// @param b           The second SRC operand to MAC (SRC2).
/// @param c           The third SRC operand to MAC (SRC3).
/// @param d           The forth SRC operand to MAC (SRC4).
/// @param zp          Zero-point value (to be passed in ZP_REG).
/// @param accumulator DEST operand to MAC.
/// @param switches    Switches of MAC instructions.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Updated value of accumulator \c acc=acc+(a*b)+(c*d)+(a*zp)+(c*zp).
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturates the result (integer data-types only).
///     \li SW_NO_NEG - default, \c acc=acc+(a*b)+(c*d)+(a*zp)+(c*zp).
///     \li SW_NEG - When set, \c acc=acc-(a*b)-(c*d)-(a*zp)-(c*zp).
///     \li SW_NEG_ZP - When set, \c acc=acc+(a*b)+(c*d)-(a*zp)-(c*zp).
///
/// When both SW_NEG and SW_NEG_ZP are set, \c acc=acc-(a*b)-(c*d)+(a*zp)+(c*zp).
///
#endif
/// @{
#if defined(__greco_plus__)
short256 v_i8_mac_x2_zp_acc16_vb(char256 a, char256 b, char256 c, char256 d, char256 zp, short256 accumulator, int switches, bool256 predicate, bool polarity=0);
short256 v_i8_mac_x2_zp_acc16_b(char256 a, char256 b, char256 c, char256 d, char256 zp, short256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
ushort256 v_u8_mac_x2_zp_acc16_vb(uchar256 a, uchar256 b, uchar256 c, uchar256 d, uchar256 zp, ushort256 accumulator, int switches, bool256 predicate, bool polarity=0);
ushort256 v_u8_mac_x2_zp_acc16_b(uchar256 a, uchar256 b, uchar256 c, uchar256 d, uchar256 zp, ushort256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_28 MAC with X2, ZP and ACC_I32 switches
/// @brief Represents MAC instruction with X2, ZP and ACC_I32 switches.
///
/// @param a           The first SRC operand to MAC (SRC1).
/// @param b           The second SRC operand to MAC (SRC2).
/// @param c           The third SRC operand to MAC (SRC3).
/// @param d           The forth SRC operand to MAC (SRC4).
/// @param zp          Zero-point value (to be passed in ZP_REG).
/// @param accumulator DEST operand to MAC.
/// @param switches    Switches of MAC instructions.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Updated value of accumulator \c acc=acc+(a*b)+(c*d)+(a*zp)+(c*zp).
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturates the result (integer data-types only).
///     \li SW_NO_NEG - default, \c acc=acc+(a*b)+(c*d)+(a*zp)+(c*zp).
///     \li SW_NEG - When set, \c acc=acc-(a*b)-(c*d)-(a*zp)-(c*zp).
///     \li SW_NEG_ZP - When set, \c acc=acc+(a*b)+(c*d)-(a*zp)-(c*zp).
///
/// When both SW_NEG and SW_NEG_ZP are set, \c acc=acc-(a*b)-(c*d)+(a*zp)+(c*zp).
///
#endif
/// @{
#if defined(__greco_plus__)
int256 v_u8_mac_x2_zp_acc32_vb(uchar256 a, uchar256 b, uchar256 c, uchar256 d, uchar256 zp, int256 accumulator, int switches, bool256 predicate, bool polarity=0);
int256 v_u8_mac_x2_zp_acc32_b(uchar256 a, uchar256 b, uchar256 c, uchar256 d, uchar256 zp, int256 accumulator, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}

//
// ------ MADD
//


#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_29 MADD
/// @brief Represents MADD instruction.
///
/// @param a           The first operand to MADD (SRC1).
/// @param b           The second operand to MADD (SRC2).
/// @param c           The third operand to MADD (SRC3).
/// @param switches    Switches of MADD instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation \c dest=c+(a*b).
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturates the result (integer data-types only).
///     \li SW_NO_NEG - default, \c dest=c+(a*b).
///     \li SW_NEG - When set, \c dest=c-(a*b).
///
#endif
/// @{
#if defined(__greco_plus__)
float64 v_f32_madd_vb(float64 a, float64 b, float64 c, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_madd_b(float64 a, float64 b, float64 c, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_madd_vb(bfloat128 a, bfloat128 b, bfloat128 c, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_madd_b(bfloat128 a, bfloat128 b, bfloat128 c, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_madd_vb(half128 a, half128 b, half128 c, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_madd_b(half128 a, half128 b, half128 c, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
int128 v_i16_madd_vb(short128 a, short128 b, int128 c, int switches, int128 income, bool128 predicate, bool polarity=0);
int128 v_i16_madd_b(short128 a, short128 b, int128 c, int switches=0, int128 income={}, bool predicate=1, bool polarity=0);
uint128 v_u16_madd_vb(ushort128 a, ushort128 b, uint128 c, int switches, uint128 income, bool128 predicate, bool polarity=0);
uint128 v_u16_madd_b(ushort128 a, ushort128 b, uint128 c, int switches=0, uint128 income={}, bool predicate=1, bool polarity=0);
int256 v_i8_madd_vb(char256 a, char256 b, int256 c, int switches, int256 income, bool256 predicate, bool polarity=0);
int256 v_i8_madd_b(char256 a, char256 b, int256 c, int switches=0, int256 income={}, bool predicate=1, bool polarity=0);
uint256 v_u8_madd_vb(uchar256 a, uchar256 b, uint256 c, int switches, uint256 income, bool256 predicate, bool polarity=0);
uint256 v_u8_madd_b(uchar256 a, uchar256 b, uint256 c, int switches=0, uint256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_30 MADD with ACC_F32 switch
/// @brief Represents MADD instruction with ACC_F32 switch.
///
/// @param a           The first operand to MADD (SRC1).
/// @param b           The second operand to MADD (SRC2).
/// @param c           The third operand to MADD (SRC3).
/// @param switches    Switches of MADD instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation \c dest=c+(a*b).
///
/// @switches Allowed switches are:
///     \li SW_NO_NEG - default, \c dest=c+(a*b).
///     \li SW_NEG - When set, \c dest=c-(a*b).
///
#endif
/// @{
#if defined(__greco_plus__)
float128 v_bf16_madd_acc32_vb(bfloat128 a, bfloat128 b, float128 c, int switches, float128 income, bool128 predicate, bool polarity=0);
float128 v_bf16_madd_acc32_b(bfloat128 a, bfloat128 b, float128 c, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
float128 v_f16_madd_acc32_vb(half128 a, half128 b, float128 c, int switches, float128 income, bool128 predicate, bool polarity=0);
float128 v_f16_madd_acc32_b(half128 a, half128 b, float128 c, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
float256 v_f8_madd_acc32_vb(minifloat256 a, minifloat256 b, float256 c, int switches, float256 income, bool256 predicate, bool polarity=0);
float256 v_f8_madd_acc32_b(minifloat256 a, minifloat256 b, float256 c, int switches=0, float256 income={}, bool predicate=1, bool polarity=0);
float256 v_h8_madd_acc32_vb(minihalf256 a, minihalf256 b, float256 c, int switches, float256 income, bool256 predicate, bool polarity=0);
float256 v_h8_madd_acc32_b(minihalf256 a, minihalf256 b, float256 c, int switches=0, float256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_31 MADD with ACC_I16 switch
/// @brief Represents MADD instruction with ACC_I16 switch.
///
/// @param a           The first operand to MADD (SRC1).
/// @param b           The second operand to MADD (SRC2).
/// @param c           The third operand to MADD (SRC3).
/// @param switches    Switches of MADD instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation \c dest=c+(a*b).
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturates the result (integer data-types only).
///     \li SW_NO_NEG - default, \c dest=c+(a*b).
///     \li SW_NEG - When set, \c dest=c-(a*b).
///
#endif
/// @{
#if defined(__greco_plus__)
short256 v_i8_madd_acc16_vb(char256 a, char256 b, short256 c, int switches, short256 income, bool256 predicate, bool polarity=0);
short256 v_i8_madd_acc16_b(char256 a, char256 b, short256 c, int switches=0, short256 income={}, bool predicate=1, bool polarity=0);
ushort256 v_u8_madd_acc16_vb(uchar256 a, uchar256 b, ushort256 c, int switches, ushort256 income, bool256 predicate, bool polarity=0);
ushort256 v_u8_madd_acc16_b(uchar256 a, uchar256 b, ushort256 c, int switches=0, ushort256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_32 MADD with ACC_I32 switch
/// @brief Represents MADD instruction with ACC_I32 switch.
///
/// @param a           The first operand to MADD (SRC1).
/// @param b           The second operand to MADD (SRC2).
/// @param c           The third operand to MADD (SRC3).
/// @param switches    Switches of MADD instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation \c dest=c+(a*b).
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturates the result (integer data-types only).
///     \li SW_NO_NEG - default, \c dest=c+(a*b).
///     \li SW_NEG - When set, \c dest=c-(a*b).
///
#endif
/// @{
#if defined(__greco_plus__)
int256 v_u8_madd_acc32_vb(uchar256 a, uchar256 b, int256 c, int switches, int256 income, bool256 predicate, bool polarity=0);
int256 v_u8_madd_acc32_b(uchar256 a, uchar256 b, int256 c, int switches=0, int256 income={}, bool predicate=1, bool polarity=0);
int128 v_u16_madd_acc32_vb(ushort128 a, ushort128 b, int128 c, int switches, int128 income, bool128 predicate, bool polarity=0);
int128 v_u16_madd_acc32_b(ushort128 a, ushort128 b, int128 c, int switches=0, int128 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_33 MADD with ZP switch
/// @brief Represents MADD instruction with ZP switch.
///
/// @param a           The first operand to MADD (SRC1).
/// @param b           The second operand to MADD (SRC2).
/// @param c           The third operand to MADD (SRC3).
/// @param zp          Zero-point value (to be passed in ZP_REG).
/// @param switches    Switches of MADD instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation \c dest=c+(a*b)+(a*zp).
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturates the result (integer data-types only).
///     \li SW_NO_NEG - default, \c dest=c+(a*b)+(a*zp).
///     \li SW_NEG - When set, \c dest=c-(a*b)-(a*zp).
///     \li SW_NEG_ZP - When set, \c dest=c+(a*b)-(a*zp).
///
/// When both SW_NEG and SW_NEG_ZP are set, \c dest=c-(a*b)+(a*zp).
///
#endif
/// @{
#if defined(__greco_plus__)
int256 v_i8_madd_zp_vb(char256 a, char256 b, int256 c, char256 zp, int switches, int256 income, bool256 predicate, bool polarity=0);
int256 v_i8_madd_zp_b(char256 a, char256 b, int256 c, char256 zp, int switches=0, int256 income={}, bool predicate=1, bool polarity=0);
uint256 v_u8_madd_zp_vb(uchar256 a, uchar256 b, uint256 c, uchar256 zp, int switches, uint256 income, bool256 predicate, bool polarity=0);
uint256 v_u8_madd_zp_b(uchar256 a, uchar256 b, uint256 c, uchar256 zp, int switches=0, uint256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_34 MADD with ZP and ACC_I16 switches
/// @brief Represents MADD instruction with ZP and ACC_I16 switches.
///
/// @param a           The first operand to MADD (SRC1).
/// @param b           The second operand to MADD (SRC2).
/// @param c           The third operand to MADD (SRC3).
/// @param zp          Zero-point value (to be passed in ZP_REG).
/// @param switches    Switches of MADD instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation \c dest=c+(a*b)+(a*zp).
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturates the result (integer data-types only).
///     \li SW_NO_NEG - default, \c dest=c+(a*b)+(a*zp).
///     \li SW_NEG - When set, \c dest=c-(a*b)-(a*zp).
///     \li SW_NEG_ZP - When set, \c dest=c+(a*b)-(a*zp).
///
/// When both SW_NEG and SW_NEG_ZP are set, \c dest=c-(a*b)+(a*zp).
///
#endif
/// @{
#if defined(__greco_plus__)
short256 v_i8_madd_zp_acc16_vb(char256 a, char256 b, short256 c, char256 zp, int switches, short256 income, bool256 predicate, bool polarity=0);
short256 v_i8_madd_zp_acc16_b(char256 a, char256 b, short256 c, char256 zp, int switches=0, short256 income={}, bool predicate=1, bool polarity=0);
ushort256 v_u8_madd_zp_acc16_vb(uchar256 a, uchar256 b, ushort256 c, uchar256 zp, int switches, ushort256 income, bool256 predicate, bool polarity=0);
ushort256 v_u8_madd_zp_acc16_b(uchar256 a, uchar256 b, ushort256 c, uchar256 zp, int switches=0, ushort256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_35 MADD with ZP and ACC_I32 switches
/// @brief Represents MADD instruction with ZP and ACC_I32 switch.
///
/// @param a           The first operand to MADD (SRC1).
/// @param b           The second operand to MADD (SRC2).
/// @param c           The third operand to MADD (SRC3).
/// @param zp          Zero-point value (to be passed in ZP_REG).
/// @param switches    Switches of MADD instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation \c dest=c+(a*b)+(a*zp).
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturates the result (integer data-types only).
///     \li SW_NO_NEG - default, \c dest=c+(a*b)+(a*zp).
///     \li SW_NEG - When set, \c dest=c-(a*b)-(a*zp).
///     \li SW_NEG_ZP - When set, \c dest=c+(a*b)-(a*zp).
///
/// When both SW_NEG and SW_NEG_ZP are set, \c dest=c-(a*b)+(a*zp).
///
#endif
/// @{
#if defined(__greco_plus__)
int256 v_u8_madd_zp_acc32_vb(uchar256 a, uchar256 b, int256 c, uchar256 zp, int switches, int256 income, bool256 predicate, bool polarity=0);
int256 v_u8_madd_zp_acc32_b(uchar256 a, uchar256 b, int256 c, uchar256 zp, int switches=0, int256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__gaudi2_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_36 MADD with X2 switch
/// @brief Represents instruction MADD with switch x2.
///
/// @param a           The first operand to MADD (SRC1).
/// @param b           The second operand to MADD (SRC2).
/// @param c           The third operand to MADD (SRC3).
/// @param d           The fourth operand to MADD (SRC4).
/// @param switches    Switches of MADD instructions, like ST or NEG.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation \c DEST.v1=c.v1+(a.v1*b) \c DEST.v2=c.v2+(a.v2*d)
///
/// @note float128 data-type that map to 2 vector registers of 64 float elements each. \n
/// In the formula above \p a.v1 and \p a.v2 refers to the first and second vectors respectively.
///
/// @switches Allowed switches are:
///     \li SW_NO_NEG - default, \c DEST.v1=c.v1+(a.v1*b) \c DEST.v2=c.v2+(a.v2*d).
///     \li SW_NEG - When set, \c DEST.v1=c.v1-(a.v1*b) \c DEST.v2=c.v2-(a.v2*d).
///
#endif
/// @{
#if defined(__gaudi2_plus__)
float128 v_f32_madd_x2_vb(float128 a, float64 b, float128 c, float64 d, int switches, float128 income, bool128 predicate, bool polarity=0);
float128 v_f32_madd_x2_b(float128 a, float64 b, float128 c, float64 d, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
float128 v_f32_madd_x2_svvv_vb(float a, float64 b, float128 c, float64 d, int switches, float128 income, bool128 predicate, bool polarity=0);
float128 v_f32_madd_x2_svvv_b(float a, float64 b, float128 c, float64 d, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
float128 v_f32_madd_x2_vvsv_vb(float128 a, float64 b, float c, float64 d, int switches, float128 income, bool128 predicate, bool polarity=0);
float128 v_f32_madd_x2_vvsv_b(float128 a, float64 b, float c, float64 d, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ MAX
//

/// @section sec2 Arithmetic
/// @subsection subsec2_37 MAX
/// @brief Represents MAX instruction.
///
/// @param a           The first SRC operand to MAX (SRC1).
/// @param b           The second SRC operand to MAX (SRC2).
/// @param switches    Switches of MAX instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
#if defined(__doron1__)
/// @switches Allowed switches are:
///     \li SW_SUP_NAN - if only one of the inputs in NaN and the other input is a number, the result will be the number.
#endif
///
/// @{
float s_f32_max(float a, float b, int switches=0, float income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_bf16_max(bf16 a, bf16 b, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_f16_max(half a, half b, int switches=0, half income={}, bool predicate=1, bool polarity=0);
#endif
int32_t s_i32_max(int32_t a, int32_t b, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_u32_max(uint32_t a, uint32_t b, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_i16_max(int16_t a, int16_t b, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_u16_max(uint16_t a, uint16_t b, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_i8_max(int8_t a, int8_t b, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u8_max(uint8_t a, uint8_t b, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
float64 v_f32_max_vb(float64 a, float64 b, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_max_b(float64 a, float64 b, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_max_vb(bfloat128 a, bfloat128 b, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_max_b(bfloat128 a, bfloat128 b, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_f16_max_vb(half128 a, half128 b, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_max_b(half128 a, half128 b, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat s_f8_max(minifloat a, minifloat b, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_max_vb(minifloat256 a, minifloat256 b, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_max_b(minifloat256 a, minifloat256 b, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf s_h8_max(minihalf a, minihalf b, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_max_vb(minihalf256 a, minihalf256 b, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_max_b(minihalf256 a, minihalf256 b, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
int64 v_i32_max_vb(int64 a, int64 b, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_max_b(int64 a, int64 b, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_max_vb(uint64 a, uint64 b, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_max_b(uint64 a, uint64 b, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_max_vb(char256 a, char256 b, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_max_b(char256 a, char256 b, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_max_vb(uchar256 a, uchar256 b, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_max_b(uchar256 a, uchar256 b, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_max_vb(short128 a, short128 b, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_max_b(short128 a, short128 b, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_max_vb(ushort128 a, ushort128 b, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_max_b(ushort128 a, ushort128 b, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
/// @}


/// @section sec2 Arithmetic
/// @subsection subsec2_38 Max for IRF
/// @brief Represents scalar MAX instruction for integer operands in index registers.
///
/// @param a           The first SRC operand to MAX (SRC1).
/// @param b           The second SRC operand to MAX (SRC2).
/// @param dimmask     Selects IRF lanes participated in the operation.
/// @param switches    Switches of MAX instructions.
/// @param income      This value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @{
int5 i_i32_max(int5 a, int5 b, const int dimmask, int switches, int5 income, bool predicate=1, bool polarity=0);
/// @}


//
// ------ MIN
//

/// @section sec2 Arithmetic
/// @subsection subsec2_39 MIN
/// @brief Represents MIN instruction.
///
/// @param a           The first SRC operand to MIN (SRC1).
/// @param b           The second SRC operand to MIN (SRC2).
/// @param switches    Switches of MIN instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
#if defined(__doron1__)
/// @switches Allowed switches are:
///     \li SW_SUP_NAN - if only one of the inputs in NaN and the other input is a number, the result will be the number.
#endif
///
/// @{
float s_f32_min(float a, float b, int switches=0, float income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_bf16_min(bf16 a, bf16 b, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_f16_min(half a, half b, int switches=0, half income={}, bool predicate=1, bool polarity=0);
#endif
int32_t s_i32_min(int32_t a, int32_t b, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_u32_min(uint32_t a, uint32_t b, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_i16_min(int16_t a, int16_t b, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_u16_min(uint16_t a, uint16_t b, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_i8_min(int8_t a, int8_t b, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u8_min(uint8_t a, uint8_t b, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
float64 v_f32_min_vb(float64 a, float64 b, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_min_b(float64 a, float64 b, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_min_vb(bfloat128 a, bfloat128 b, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_min_b(bfloat128 a, bfloat128 b, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_f16_min_vb(half128 a, half128 b, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_min_b(half128 a, half128 b, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat s_f8_min(minifloat a, minifloat b, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_min_vb(minifloat256 a, minifloat256 b, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_min_b(minifloat256 a, minifloat256 b, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf s_h8_min(minihalf a, minihalf b, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_min_vb(minihalf256 a, minihalf256 b, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_min_b(minihalf256 a, minihalf256 b, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
int64 v_i32_min_vb(int64 a, int64 b, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_min_b(int64 a, int64 b, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_min_vb(uint64 a, uint64 b, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_min_b(uint64 a, uint64 b, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_min_vb(char256 a, char256 b, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_min_b(char256 a, char256 b, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_min_vb(uchar256 a, uchar256 b, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_min_b(uchar256 a, uchar256 b, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_min_vb(short128 a, short128 b, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_min_b(short128 a, short128 b, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_min_vb(ushort128 a, ushort128 b, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_min_b(ushort128 a, ushort128 b, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
/// @}

/// @section sec2 Arithmetic
/// @subsection subsec2_40 MIN for IRF
/// @brief Represents scalar MIN instruction for integer operands in index registers.
///
/// @param a           The first SRC operand to MIN (SRC1).
/// @param b           The second SRC operand to MIN (SRC2).
/// @param dimmask     Selects IRF lanes participated in the operation.
/// @param switches    Switches of MIN instructions.
/// @param income      TThis value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @{
int5 i_i32_min(int5 a, int5 b, const int dimmask, int switches, int5 income, bool predicate=1, bool polarity=0);
/// @}

//
// ------ MOV
//

/// @section sec9 Move
/// @subsection subsec9_1 Mov
/// @brief Represents MOV instruction.
///
/// @param a           Source (SRC1).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation - move \p a to DEST.
///
/// @{
float s_f32_mov(float a, int switches=0, float income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_bf16_mov(bf16 a, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_f16_mov(half a, int switches=0, half income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat s_f8_mov(minifloat a, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_h8_mov(minihalf a, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
#endif
int32_t s_i32_mov(int32_t a, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_u32_mov(uint32_t a, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_i16_mov(int16_t a, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_u16_mov(uint16_t a, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_i8_mov(int8_t a, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u8_mov(uint8_t a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
bool s_i1_mov(bool a, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
float64 v_f32_mov_vb(float64 a, int switches, float64 income, bool64 predicate, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_mov_vb(bfloat128 a, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_f16_mov_vb(half128 a, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_mov_vb(minifloat256 a, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_mov_vb(minihalf256 a, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
int64 v_i32_mov_vb(int64 a, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_mov_vb(uint64 a, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_mov_vb(short128 a, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_mov_vb(ushort128 a, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_mov_vb(char256 a, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_mov_vb(uchar256 a, int switches, uchar256 income, bool256 predicate, bool polarity=0);
bool256 v_i1_mov_vb(bool256 a, int switches, bool256 income, bool256 predicate, bool polarity=0);
float64 v_f32_mov_b(float64 a, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_mov_b(bfloat128 a, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_f16_mov_b(half128 a, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_mov_b(minifloat256 a, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_mov_b(minihalf256 a, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
int64 v_i32_mov_b(int64 a, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_mov_b(uint64 a, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_mov_b(short128 a, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_mov_b(ushort128 a, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_mov_b(char256 a, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_mov_b(uchar256 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_mov_b(bool256 a, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
/// @}


/// @section sec9 Move
/// @subsection subsec9_2 Mov for IRF
/// @brief Represents MOV instruction for int5 type.
///
/// @param a           Source (SRC1).
/// @param dimmask     Selects IRF lanes participated in the operation.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation - move \p a to DEST.
///
/// @{
int5 i_i32_mov(int5 a, int dimmask, int switches, int5 income, bool predicate=1, bool polarity=0);
/// @}


/// @section sec9 Move
/// @subsection subsec9_3 Mov with argument FLAVOR
/// @brief Represents MOV instruction with argument FLAVOR.
///
/// @param a           Source (SRC1).
/// @param flavor      Selects section of VPRF participated in the operation [0-8]. \n
///                    0-7 map to 8 VPRF sections, when \p flavor is set to 8 - mov to all 8 sections.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation - move \p a to one or all 8 sections of DEST.
///
/// @{
bool256 v_i1_mov_flavor_b(int32_t a, const int flavor, int switches, bool256 income, bool predicate=1, bool polarity=0);
bool256 v_i1_mov_flavor_vb(int32_t a, const int flavor, int switches, bool256 income, bool256 predicate, bool polarity=0);
/// @}


/// @section sec9 Move
/// @subsection subsec9_4 Move from scalar predicate register to vector predicate register
/// @brief Represent MOV SPRF to VPRF instruction.
///
/// @param a           Source (SRC1).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation - broadcast \p a to DEST.
///
/// @{
bool256 v_i1_mov_i1_b(bool a, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_mov_i1_vb(bool a, int switches, bool256 income, bool256 predicate, bool polarity=0);
/// @}


#if defined(__greco_plus__)
/// @section sec9 Move
/// @subsection subsec9_5 Move VPRF to/from VRF
/// @brief Represents MOV VPRF to/from VRF instruction.
///
/// @param a           Source (SRC1).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation - move \p a to DEST.
///
/// Move VRF to VPRF - moves 4 lower bits of each 4B VRF element to the 4 appropriate bits of VPRF. \n
/// Move VPRF to VRF - moves each 4 bits of VPRF to the 4 lower bits of the appropriate 4B element of VRF.
///
#endif
/// @{
#if defined(__greco_plus__)
bool256 v_i1_mov_u32_b(uint64 a, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_mov_u32_vb(uint64 a, int switches, bool256 income, bool256 predicate, bool polarity=0);
uint64 v_u32_mov_i1_b(bool256 a, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_mov_i1_vb(bool256 a, int switches, uint64 income, bool256 predicate, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec9 Move
/// @subsection subsec9_6 Move with X2 switch
/// @brief Represents MOV instruction with X2 switch.
///
/// @param a           Source (SRC1).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation - move a pair of VRFs - \p a, to a pair of VRFs - DEST.
///
#endif
/// @{
#if defined(__greco_plus__)
float128 v_f32_mov_x2_vb(float128 a, int switches, float128 income, bool64 predicate, bool polarity=0);
bfloat256 v_bf16_mov_x2_vb(bfloat256 a, int switches, bfloat256 income, bool128 predicate, bool polarity=0);
half256 v_f16_mov_x2_vb(half256 a, int switches, half256 income, bool128 predicate, bool polarity=0);
int128 v_i32_mov_x2_vb(int128 a, int switches, int128 income, bool64 predicate, bool polarity=0);
uint128 v_u32_mov_x2_vb(uint128 a, int switches, uint128 income, bool64 predicate, bool polarity=0);
short256 v_i16_mov_x2_vb(short256 a, int switches, short256 income, bool128 predicate, bool polarity=0);
ushort256 v_u16_mov_x2_vb(ushort256 a, int switches, ushort256 income, bool128 predicate, bool polarity=0);
char512 v_i8_mov_x2_vb(char512 a, int switches, char512 income, bool256 predicate, bool polarity=0);
uchar512 v_u8_mov_x2_vb(uchar512 a, int switches, uchar512 income, bool256 predicate, bool polarity=0);
float128 v_f32_mov_x2_b(float128 a, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
bfloat256 v_bf16_mov_x2_b(bfloat256 a, int switches=0, bfloat256 income={}, bool predicate=1, bool polarity=0);
half256 v_f16_mov_x2_b(half256 a, int switches=0, half256 income={}, bool predicate=1, bool polarity=0);
int128 v_i32_mov_x2_b(int128 a, int switches=0, int128 income={}, bool predicate=1, bool polarity=0);
uint128 v_u32_mov_x2_b(uint128 a, int switches=0, uint128 income={}, bool predicate=1, bool polarity=0);
short256 v_i16_mov_x2_b(short256 a, int switches=0, short256 income={}, bool predicate=1, bool polarity=0);
ushort256 v_u16_mov_x2_b(ushort256 a, int switches=0, ushort256 income={}, bool predicate=1, bool polarity=0);
char512 v_i8_mov_x2_b(char512 a, int switches=0, char512 income={}, bool predicate=1, bool polarity=0);
uchar512 v_u8_mov_x2_b(uchar512 a, int switches=0, uchar512 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
// Instruction with scalar predicate is absent. Is it ok?
minifloat512 v_f8_mov_x2_vb(minifloat512 a, int switches, minifloat512 income, bool256 predicate, bool polarity=0);
minihalf512 v_h8_mov_x2_vb(minihalf512 a, int switches, minihalf512 income, bool256 predicate, bool polarity=0);
#endif
/// @}

#if defined(__greco_plus__)
/// @section sec7 Load
/// @subsection subsec7_24 get_addr
/// @brief Retrieves 64-bit address value of the global pointer.
///
/// @param addr        Points to the address (SRC1).
/// @param switches    Instruction switches.
/// @param income      Address value (Income value of DEST).
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Address value for input pointer.
///
/// @{
uint32_t_pair_t get_addr(__global void *addr, int switches=0, uint32_t_pair_t income = {}, bool predicate=1, bool polarity=0);
/// @}
#endif


#if defined(__greco_plus__)
/// @section sec7 Load
/// @subsection subsec7_25 update address
/// @brief Updates 64-bit address value of the global pointer but does not affect other ADRF attributes (such as data-type width, L0CD, tensor ID).
///
/// @param addr        Points to the address.
/// @param value       Address value (SRC1).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @{
void update_addr(__global void **addr, uint32_t_pair_t value, int switches=0, bool predicate=1, bool polarity=0);
/// @}
#endif


/// @section sec9 Move
/// @subsection subsec9_7 Move Dual Group
/// @brief Represents MOV_DUAL_GROUP instruction.
///
/// @param a           Source (SRC1).
/// @param b           Byte write mask. 
/// @param src_dg      Source dual group {0, 1, 2, 3}.
/// @param dest_dg     Destination dual group {0, 1, 2, 3}.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false or when no elements are moved to the target lanes.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @switches Allowed switches:
///     \li SW_WR_LOWER_GROUP If set, the lower group in the output dual group is written with respect to the Byte write mask.
///     \li SW_WR_UPPER_GROUP If set, the lower group in the output dual group is written with respect to the Byte write mask.
///
/// @{
float64 v_f32_mov_dual_group_vb(float64 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_mov_dual_group_b(float64 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, float64 income, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_mov_dual_group_vb(bfloat128 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_mov_dual_group_b(bfloat128 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, bfloat128 income, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_f16_mov_dual_group_vb(half128 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_mov_dual_group_b(half128 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, half128 income, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_mov_dual_group_vb(minifloat256 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_mov_dual_group_b(minifloat256 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, minifloat256 income, bool predicate=1, bool polarity=0);
minihalf256 v_h8_mov_dual_group_vb(minihalf256 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_mov_dual_group_b(minihalf256 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, minihalf256 income, bool predicate=1, bool polarity=0);
#endif
int64 v_i32_mov_dual_group_vb(int64 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_mov_dual_group_b(int64 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, int64 income, bool predicate=1, bool polarity=0);
uint64 v_u32_mov_dual_group_vb(uint64 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_mov_dual_group_b(uint64 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, uint64 income, bool predicate=1, bool polarity=0);
short128 v_i16_mov_dual_group_vb(short128 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_mov_dual_group_b(short128 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, short128 income, bool predicate=1, bool polarity=0);
ushort128 v_u16_mov_dual_group_vb(ushort128 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_mov_dual_group_b(ushort128 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, ushort128 income, bool predicate=1, bool polarity=0);
char256 v_i8_mov_dual_group_vb(char256 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_mov_dual_group_b(char256 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, char256 income, bool predicate=1, bool polarity=0);
uchar256 v_u8_mov_dual_group_vb(uchar256 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_mov_dual_group_b(uchar256 a, const uint32_t b, const int src_dg, const int dest_dg, int switches, uchar256 income, bool predicate=1, bool polarity=0);
/// @}

#if defined(__doron1__)
/// @section sec9 Move
/// @subsection subsec9_7 Move Dual Group
/// @brief Represents MOV_DUAL_GROUP instruction with dynamic control value.
///
/// @param a           Source (SRC1).
/// @param b           Byte write mask. 
/// @param ctrl_val    Instruction control value. This value can be calculated in runtime, use helper function 'MdgCtrlSingle(src_dual_group, dst_dual_group, write_lower_group, write_upper_group)'
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false or when no elements are moved to the target lanes.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @switches Allowed switches:
///     \li SW_WR_LOWER_GROUP If set, the lower group in the output dual group is written with respect to the Byte write mask.
///     \li SW_WR_UPPER_GROUP If set, the lower group in the output dual group is written with respect to the Byte write mask.
///
/// @{
float64      v_f32_mov_dg_vb(float64 a, const uint32_t b, uint32_t ctrl_val, int switches, float64 income, bool64 predicate, bool polarity=0);
float64      v_f32_mov_dg_b(float64 a, const uint32_t b, uint32_t ctrl_val, int switches, float64 income, bool predicate=1, bool polarity=0);
bfloat128    v_bf16_mov_dg_vb(bfloat128 a, const uint32_t b, uint32_t ctrl_val, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128    v_bf16_mov_dg_b(bfloat128 a, const uint32_t b, uint32_t ctrl_val, int switches, bfloat128 income, bool predicate=1, bool polarity=0);
half128      v_f16_mov_dg_vb(half128 a, const uint32_t b, uint32_t ctrl_val, int switches, half128 income, bool128 predicate, bool polarity=0);
half128      v_f16_mov_dg_b(half128 a, const uint32_t b, uint32_t ctrl_val, int switches, half128 income, bool predicate=1, bool polarity=0);
minifloat256 v_f8_mov_dg_vb(minifloat256 a, const uint32_t b, uint32_t ctrl_val, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_mov_dg_b(minifloat256 a, const uint32_t b, uint32_t ctrl_val, int switches, minifloat256 income, bool predicate=1, bool polarity=0);
minihalf256  v_h8_mov_dg_vb(minihalf256 a, const uint32_t b, uint32_t ctrl_val, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256  v_h8_mov_dg_b(minihalf256 a, const uint32_t b, uint32_t ctrl_val, int switches, minihalf256 income, bool predicate=1, bool polarity=0);
int64        v_i32_mov_dg_vb(int64 a, const uint32_t b, uint32_t ctrl_val, int switches, int64 income, bool64 predicate, bool polarity=0);
int64        v_i32_mov_dg_b(int64 a, const uint32_t b, uint32_t ctrl_val, int switches, int64 income, bool predicate=1, bool polarity=0);
uint64       v_u32_mov_dg_vb(uint64 a, const uint32_t b, uint32_t ctrl_val, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64       v_u32_mov_dg_b(uint64 a, const uint32_t b, uint32_t ctrl_val, int switches, uint64 income, bool predicate=1, bool polarity=0);
short128     v_i16_mov_dg_vb(short128 a, const uint32_t b, uint32_t ctrl_val, int switches, short128 income, bool128 predicate, bool polarity=0);
short128     v_i16_mov_dg_b(short128 a, const uint32_t b, uint32_t ctrl_val, int switches, short128 income, bool predicate=1, bool polarity=0);
ushort128    v_u16_mov_dg_vb(ushort128 a, const uint32_t b, uint32_t ctrl_val, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128    v_u16_mov_dg_b(ushort128 a, const uint32_t b, uint32_t ctrl_val, int switches, ushort128 income, bool predicate=1, bool polarity=0);
char256      v_i8_mov_dg_vb(char256 a, const uint32_t b, uint32_t ctrl_val, int switches, char256 income, bool256 predicate, bool polarity=0);
char256      v_i8_mov_dg_b(char256 a, const uint32_t b, uint32_t ctrl_val, int switches, char256 income, bool predicate=1, bool polarity=0);
uchar256     v_u8_mov_dg_vb(uchar256 a, const uint32_t b, uint32_t ctrl_val, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256     v_u8_mov_dg_b(uchar256 a, const uint32_t b, uint32_t ctrl_val, int switches, uchar256 income, bool predicate=1, bool polarity=0);
/// @}
#endif

#if defined(__gaudi_plus__)
/// @section sec9 Move
/// @subsection subsec9_8 Move Dual Group with ALL switch
/// @brief Represents MOV_DUAL_GROUP instruction with ALL switch.
///
/// @param a           Source (SRC1).
/// @param b           Byte write mask.
/// @param sdg0        Source dual group 0 {0, 1, 2, 3}.
/// @param sdg1        Source dual group 1 {0, 1, 2, 3}.
/// @param sdg2        Source dual group 2 {0, 1, 2, 3}.
/// @param sdg3        Source dual group 3 {0, 1, 2, 3}.
/// @param switches    Instruction switches. See also helper macro MkWrA(weg0, weg1, weg2, weg3).
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @switches Allowed switches:
///     \li SW_WR_LOWER_GROUP0 If set, the lower group in the output dual group 0 is written with respect to the Byte write mask.
///     \li SW_WR_LOWER_GROUP1 If set, the lower group in the output dual group 1 is written with respect to the Byte write mask.
///     \li SW_WR_LOWER_GROUP2 If set, the lower group in the output dual group 2 is written with respect to the Byte write mask.
///     \li SW_WR_LOWER_GROUP3 If set, the lower group in the output dual group 3 is written with respect to the Byte write mask.
///     \li SW_WR_UPPER_GROUP0 If set, the upper group in the output dual group 0 is written with respect to the Byte write mask.
///     \li SW_WR_UPPER_GROUP1 If set, the upper group in the output dual group 1 is written with respect to the Byte write mask.
///     \li SW_WR_UPPER_GROUP2 If set, the upper group in the output dual group 2 is written with respect to the Byte write mask.
///     \li SW_WR_UPPER_GROUP3 If set, the upper group in the output dual group 3 is written with respect to the Byte write mask.
///
#endif
/// @{
#if defined(__gaudi_plus__)
float64 v_f32_mov_dual_group_all_vb(float64 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_mov_dual_group_all_b(float64 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, float64 income, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_mov_dual_group_all_vb(bfloat128 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_mov_dual_group_all_b(bfloat128 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, bfloat128 income, bool predicate=1, bool polarity=0);
int64 v_i32_mov_dual_group_all_vb(int64 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_mov_dual_group_all_b(int64 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, int64 income, bool predicate=1, bool polarity=0);
uint64 v_u32_mov_dual_group_all_vb(uint64 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_mov_dual_group_all_b(uint64 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, uint64 income, bool predicate=1, bool polarity=0);
short128 v_i16_mov_dual_group_all_vb(short128 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_mov_dual_group_all_b(short128 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, short128 income, bool predicate=1, bool polarity=0);
ushort128 v_u16_mov_dual_group_all_vb(ushort128 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_mov_dual_group_all_b(ushort128 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, ushort128 income, bool predicate=1, bool polarity=0);
char256 v_i8_mov_dual_group_all_vb(char256 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_mov_dual_group_all_b(char256 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, char256 income, bool predicate=1, bool polarity=0);
uchar256 v_u8_mov_dual_group_all_vb(uchar256 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_mov_dual_group_all_b(uchar256 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, uchar256 income, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
half128 v_f16_mov_dual_group_all_vb(half128 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_mov_dual_group_all_b(half128 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, half128 income, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_mov_dual_group_all_vb(minifloat256 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_mov_dual_group_all_b(minifloat256 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, minifloat256 income, bool predicate=1, bool polarity=0);
minihalf256 v_h8_mov_dual_group_all_vb(minihalf256 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_mov_dual_group_all_b(minihalf256 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, minihalf256 income, bool predicate=1, bool polarity=0);
#endif
/// @}

#if defined(__doron1__)
/// @section sec9 Move
/// @subsection subsec9_8 Move Dual Group with ALL switch
/// @brief Represents MOV_DUAL_GROUP instruction with ALL switch and dynamic control value.
///
/// @param a           Source (SRC1).
/// @param b           Byte write mask.
/// @param ctrl_val    Source control value. This value can be calculated in runtime, use helper function 'MdgCtrlAll(src_dual_group_0, src_dual_group_1, src_dual_group_2, src_dual_group_3)'
/// @param switches    Instruction switches. See also helper macro MkWrA(weg0, weg1, weg2, weg3)
/// @param income      This value is returned if the predicate is false or when no elements are moved to the target lanes.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @switches Allowed switches:
///     \li SW_WR_LOWER_GROUP0 If set, the lower group in the output dual group 0 is written with respect to the Byte write mask.
///     \li SW_WR_LOWER_GROUP1 If set, the lower group in the output dual group 1 is written with respect to the Byte write mask.
///     \li SW_WR_LOWER_GROUP2 If set, the lower group in the output dual group 2 is written with respect to the Byte write mask.
///     \li SW_WR_LOWER_GROUP3 If set, the lower group in the output dual group 3 is written with respect to the Byte write mask.
///     \li SW_WR_UPPER_GROUP0 If set, the upper group in the output dual group 0 is written with respect to the Byte write mask.
///     \li SW_WR_UPPER_GROUP1 If set, the upper group in the output dual group 1 is written with respect to the Byte write mask.
///     \li SW_WR_UPPER_GROUP2 If set, the upper group in the output dual group 2 is written with respect to the Byte write mask.
///     \li SW_WR_UPPER_GROUP3 If set, the upper group in the output dual group 3 is written with respect to the Byte write mask.
/// @{
float64      v_f32_mov_dg_all_vb(float64 a, const uint32_t b, uint32_t ctrl_val, int switches, float64 income, bool64 predicate, bool polarity=0);
float64      v_f32_mov_dg_all_b(float64 a, const uint32_t b, uint32_t ctrl_val, int switches, float64 income, bool64 predicate, bool polarity=0);
bfloat128    v_bf16_mov_dg_all_vb(bfloat128 a, const uint32_t b, uint32_t ctrl_val, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128    v_bf16_mov_dg_all_b(bfloat128 a, const uint32_t b, uint32_t ctrl_val, int switches, bfloat128 income, bool predicate=1, bool polarity=0);
int64 		 v_i32_mov_dg_all_vb(int64 a, const uint32_t b, uint32_t ctrl_val, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 		 v_i32_mov_dg_all_b(int64 a, const uint32_t b, uint32_t ctrl_val, int switches, int64 income, bool predicate=1, bool polarity=0);
uint64 		 v_u32_mov_dg_all_vb(uint64 a, const uint32_t b, uint32_t ctrl_val, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 		 v_u32_mov_dg_all_b(uint64 a, const uint32_t b, uint32_t ctrl_val, int switches, uint64 income, bool predicate=1, bool polarity=0);
short128 	 v_i16_mov_dg_all_vb(short128 a, const uint32_t b, uint32_t ctrl_val, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 	 v_i16_mov_dg_all_b(short128 a, const uint32_t b, uint32_t ctrl_val, int switches, short128 income, bool predicate=1, bool polarity=0);
ushort128 	 v_u16_mov_dg_all_vb(ushort128 a, const uint32_t b, uint32_t ctrl_val, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 	 v_u16_mov_dg_all_b(ushort128 a, const uint32_t b, uint32_t ctrl_val, int switches, ushort128 income, bool predicate=1, bool polarity=0);
char256 	 v_i8_mov_dg_all_vb(char256 a, const uint32_t b, uint32_t ctrl_val, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 	 v_i8_mov_dg_all_b(char256 a, const uint32_t b, uint32_t ctrl_val, int switches, char256 income, bool predicate=1, bool polarity=0);
uchar256 	 v_u8_mov_dg_all_vb(uchar256 a, const uint32_t b, uint32_t ctrl_val, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 	 v_u8_mov_dg_all_b(uchar256 a, const uint32_t b, uint32_t ctrl_val, int switches, uchar256 income, bool predicate=1, bool polarity=0);
half128 	 v_f16_mov_dg_all_vb(half128 a, const uint32_t b, uint32_t ctrl_val, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 	 v_f16_mov_dg_all_b(half128 a, const uint32_t b, uint32_t ctrl_val, int switches, half128 income, bool predicate=1, bool polarity=0);
minifloat256 v_f8_mov_dg_all_vb(minifloat256 a, const uint32_t b, uint32_t ctrl_val, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_mov_dg_all_b(minifloat256 a, const uint32_t b, uint32_t ctrl_val, int switches, minifloat256 income, bool predicate=1, bool polarity=0);
minihalf256  v_h8_mov_dg_all_vb(minihalf256 a, const uint32_t b, uint32_t ctrl_val, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256  v_h8_mov_dg_all_b(minihalf256 a, const uint32_t b, uint32_t ctrl_val, int switches, minihalf256 income, bool predicate=1, bool polarity=0);
/// @}
#endif

#if defined(__gaudib__) || defined(__gaudi2_plus__)
/// @section sec9 Move
/// @subsection subsec9_9 Move Dual Group with UNPACK switch
/// @brief Represents MOV_DUAL_GROUP instruction with UNPACK switch.
///
/// @param a           Source (SRC1).
/// @param b           Byte write mask.
/// @param sdg0        Source dual group 0 {0, 1, 2, 3}.
/// @param sdg1        Source dual group 1 {0, 1, 2, 3}.
/// @param sdg2        Source dual group 2 {0, 1, 2, 3}.
/// @param sdg3        Source dual group 3 {0, 1, 2, 3}.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @switches Allowed switches:
///     \li SW_WR_LOWER_GROUP0
///     \li SW_WR_LOWER_GROUP1
///     \li SW_WR_LOWER_GROUP2
///     \li SW_WR_LOWER_GROUP3
///     \li SW_WR_UPPER_GROUP0
///     \li SW_WR_UPPER_GROUP1
///     \li SW_WR_UPPER_GROUP2
///     \li SW_WR_UPPER_GROUP3
///     \li SW_UNPACK_EVEN_LANES
///     \li SW_UNPACK_ODD_LANES
///
#endif
/// @{
#if defined(__gaudib__) || defined(__gaudi2_plus__)
float64 v_f32_mov_dual_group_unpack_vb(float64 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_mov_dual_group_unpack_b(float64 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, float64 income, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_mov_dual_group_unpack_vb(bfloat128 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_mov_dual_group_unpack_b(bfloat128 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, bfloat128 income, bool predicate=1, bool polarity=0);
int64 v_i32_mov_dual_group_unpack_vb(int64 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_mov_dual_group_unpack_b(int64 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, int64 income, bool predicate=1, bool polarity=0);
uint64 v_u32_mov_dual_group_unpack_vb(uint64 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_mov_dual_group_unpack_b(uint64 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, uint64 income, bool predicate=1, bool polarity=0);
short128 v_i16_mov_dual_group_unpack_vb(short128 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_mov_dual_group_unpack_b(short128 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, short128 income, bool predicate=1, bool polarity=0);
ushort128 v_u16_mov_dual_group_unpack_vb(ushort128 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_mov_dual_group_unpack_b(ushort128 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, ushort128 income, bool predicate=1, bool polarity=0);
char256 v_i8_mov_dual_group_unpack_vb(char256 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_mov_dual_group_unpack_b(char256 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, char256 income, bool predicate=1, bool polarity=0);
uchar256 v_u8_mov_dual_group_unpack_vb(uchar256 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_mov_dual_group_unpack_b(uchar256 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, uchar256 income, bool predicate=1, bool polarity=0);
half128 v_f16_mov_dual_group_unpack_vb(half128 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_mov_dual_group_unpack_b(half128 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, half128 income, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_mov_dual_group_unpack_vb(minifloat256 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_mov_dual_group_unpack_b(minifloat256 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, minifloat256 income, bool predicate=1, bool polarity=0);
minihalf256 v_h8_mov_dual_group_unpack_vb(minihalf256 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_mov_dual_group_unpack_b(minihalf256 a, const uint32_t b, const int sdg0, const int sdg1, const int sdg2, const int sdg3, int switches, minihalf256 income, bool predicate=1, bool polarity=0);
#endif
/// @}

#if defined(__greco_plus__)
/// @section sec9 Move
/// @subsection subsec9_10 Move Dual Group with PACK switch
/// @brief Represents MOV_DUAL_GROUP instruction with PACK switch.
///
/// @param a           Source (SRC1).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @switches Allowed switches:
///     \li SW_PACK21 - Pack ratio 2:1 (result of PACK 32->16, or 16->8), \n
///                     result will be in the 2 lower dual-groups of destination
///     \li SW_PACK41 - Pack ratio 4:1 (results of PACK 32->8), \n
///                     result will be in the lowest dual-group of the destination
///
#endif
/// @{
#if defined(__greco_plus__)
float64 v_f32_mov_dual_group_pack_vb(float64 a, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_mov_dual_group_pack_b(float64 a, int switches, float64 income, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_mov_dual_group_pack_vb(bfloat128 a, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_mov_dual_group_pack_b(bfloat128 a, int switches, bfloat128 income, bool predicate=1, bool polarity=0);
half128 v_f16_mov_dual_group_pack_vb(half128 a, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_mov_dual_group_pack_b(half128 a, int switches, half128 income, bool predicate=1, bool polarity=0);
int64 v_i32_mov_dual_group_pack_vb(int64 a, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_mov_dual_group_pack_b(int64 a, int switches, int64 income, bool predicate=1, bool polarity=0);
uint64 v_u32_mov_dual_group_pack_vb(uint64 a, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_mov_dual_group_pack_b(uint64 a, int switches, uint64 income, bool predicate=1, bool polarity=0);
short128 v_i16_mov_dual_group_pack_vb(short128 a, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_mov_dual_group_pack_b(short128 a, int switches, short128 income, bool predicate=1, bool polarity=0);
ushort128 v_u16_mov_dual_group_pack_vb(ushort128 a, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_mov_dual_group_pack_b(ushort128 a, int switches, ushort128 income, bool predicate=1, bool polarity=0);
char256 v_i8_mov_dual_group_pack_vb(char256 a, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_mov_dual_group_pack_b(char256 a, int switches, char256 income, bool predicate=1, bool polarity=0);
uchar256 v_u8_mov_dual_group_pack_vb(uchar256 a, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_mov_dual_group_pack_b(uchar256 a, int switches, uchar256 income, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_mov_dual_group_pack_vb(minifloat256 a, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_mov_dual_group_pack_b(minifloat256 a, int switches, minifloat256 income, bool predicate=1, bool polarity=0);
minihalf256 v_h8_mov_dual_group_pack_vb(minihalf256 a, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_mov_dual_group_pack_b(minihalf256 a, int switches, minihalf256 income, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ MOV_IRF_DIM
//

/// @section sec9 Move
/// @subsection subsec9_11 MOV_IRF_DIM
/// @brief Represents MOV_IRF_DIM instruction.
///
/// @param src         The index vector (SRC1).
/// @param dim         The number of extracted dimension.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation - move \p src to DEST.
///
/// @{
int32_t mov_irf_dim(int5 src, const int8_t dim, int switches, int32_t income={}, bool predicate=1, bool polarity=0);
/// @}


/// @section sec9 Move
/// @subsection subsec9_12 MOV_IRF_DIM with BOTH switch
/// @brief Represents MOV_IRF_DIM instruction with BOTH switch.\n
/// Relevant only for long-irf mode.
///
/// @param src         The index vector (SRC1).
/// @param dim         The number of extracted dimension.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation - move \p src to DEST.
///
/// @{
#if defined(__gaudi2_plus__)
int32_t_pair_t long_irf_dim(int5 src, const int8_t dim, int switches, int32_t_pair_t income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


/// @section sec9 Move
/// @subsection subsec9_13 Move Group
/// @brief Represents MOV_GROUP instruction.
///
/// @param a           Source (SRC1).
/// @param b           Byte write mask.
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false or when no elements are moved to the target lanes.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @switches Allowed switches:
///     - [GROUP_EN] - when set group is written according to \p b.
///         \li SW_GROUP0_EN
///         \li SW_GROUP1_EN
///     - [DUAL_GROUP_EN] - when set dual group is written according to GROUP_EN and \p b.
///         \li SW_DUAL_GROUP0_EN
///         \li SW_DUAL_GROUP1_EN
///         \li SW_DUAL_GROUP2_EN
///         \li SW_DUAL_GROUP3_EN
///
/// @{
float64 v_f32_mov_group_vb(float64 a, uint32_t b, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_mov_group_b(float64 a, uint32_t b, int switches, float64 income, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_mov_group_vb(bfloat128 a, uint32_t b, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_mov_group_b(bfloat128 a, uint32_t b, int switches, bfloat128 income, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
half128 v_f16_mov_group_vb(half128 a, uint32_t b, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_mov_group_b(half128 a, uint32_t b, int switches, half128 income, bool predicate=1, bool polarity=0);
#endif
#if  defined(__gaudi2_plus__)
minifloat256 v_f8_mov_group_vb(minifloat256 a, uint32_t b, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_mov_group_b(minifloat256 a, uint32_t b, int switches, minifloat256 income, bool predicate=1, bool polarity=0);
minihalf256 v_h8_mov_group_vb(minihalf256 a, uint32_t b, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_mov_group_b(minihalf256 a, uint32_t b, int switches, minihalf256 income, bool predicate=1, bool polarity=0);
#endif
int64 v_i32_mov_group_vb(int64 a, uint32_t b, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_mov_group_b(int64 a, uint32_t b, int switches, int64 income, bool predicate=1, bool polarity=0);
uint64 v_u32_mov_group_vb(uint64 a, uint32_t b, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_mov_group_b(uint64 a, uint32_t b, int switches, uint64 income, bool predicate=1, bool polarity=0);
short128 v_i16_mov_group_vb(short128 a, uint32_t b, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_mov_group_b(short128 a, uint32_t b, int switches, short128 income, bool predicate=1, bool polarity=0);
ushort128 v_u16_mov_group_vb(ushort128 a, uint32_t b, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_mov_group_b(ushort128 a, uint32_t b, int switches, ushort128 income, bool predicate=1, bool polarity=0);
char256 v_i8_mov_group_vb(char256 a, uint32_t b, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_mov_group_b(char256 a, uint32_t b, int switches, char256 income, bool predicate=1, bool polarity=0);
uchar256 v_u8_mov_group_vb(uchar256 a, uint32_t b, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_mov_group_b(uchar256 a, uint32_t b, int switches, uchar256 income, bool predicate=1, bool polarity=0);
/// @}


//
// ------ MSAC
//


#if defined(__goya__) || defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_41 MSAC
/// @brief Represents MSAC instruction.
///
/// @param a           Source #1 (SRC1)
/// @param b           Source #2 (SRC2)
/// @param n1          ABCin norm factor, range is [-32, 0] (SRC3)
/// @param n2          CinCout norm factor, range is [-25, 0] (SRC4)
/// @param switches    Switches of MSAC instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Normalized result after multiplication of two source operands (a & b).
///
/// @switches Allowed switches:
///     \li SW_RHU - Round half up.
/// \par
///     - [SW_NORMALIZE] - \p c refers to DEST
///         \li SW_NORMALIZE_AB - \c c=((a*b)*2^n1+c)*2^n2)
///         \li SW_NORMALIZE_C - \c c=((a*b)+c*2^n1)*2^n2)
///
#endif
/// @{
#if defined(__goya__) || defined(__greco_plus__)
short128 v_i16_msac_b(short128 a, short128 b, char256 n1, char256 n2, int switches, short128 income, bool predicate=1, bool polarity=0);
short128 v_i16_msac_vb(short128 a, short128 b, char256 n1, char256 n2, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_msac_b(ushort128 a, ushort128 b, char256 n1, char256 n2, int switches, ushort128 income, bool predicate=1, bool polarity=0);
ushort128 v_u16_msac_vb(ushort128 a, ushort128 b, char256 n1, char256 n2, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_msac_b(char256 a, char256 b, char256 n1, char256 n2, int switches, char256 income, bool predicate=1, bool polarity=0);
char256 v_i8_msac_vb(char256 a, char256 b, char256 n1, char256 n2, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_msac_b(uchar256 a, uchar256 b, char256 n1, char256 n2, int switches, uchar256 income, bool predicate=1, bool polarity=0);
uchar256 v_u8_msac_vb(uchar256 a, uchar256 b, char256 n1, char256 n2, int switches, uchar256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


//
// ------ MUL
//


/// @section sec2 Arithmetic
/// @subsection subsec2_42 Mul
/// @brief Represents MUL instruction for float arguments.
///
/// @param a           The first SRC operand to MUL (SRC1).
/// @param b           The second SRC operand to MUL (SRC2).
/// @param switches    Switches of MUL instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @{
float s_f32_mul(float a, float b, int switches=0, float income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_bf16_mul(bf16 a, bf16 b, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_f16_mul(half a, half b, int switches=0, half income={}, bool predicate=1, bool polarity=0);
#endif
float64 v_f32_mul_vb(float64 a, float64 b, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_mul_b(float64 a, float64 b, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_mul_vb(bfloat128 a, bfloat128 b, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_mul_b(bfloat128 a, bfloat128 b, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
half128 v_f16_mul_vb(half128 a, half128 b, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_mul_b(half128 a, half128 b, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);

// variant for f8/h8 w/o acc is absent in hardware  see 2.3.2

#endif
/// @}


#if defined(__gaudi_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_43 MUL with ACC_F32 switch
/// @brief Represents MUL instruction with ACC_F32 switch (32-bit accumulator).
///
/// @param a           The first SRC operand to MUL (SRC1).
/// @param b           The second SRC operand to MUL (SRC2).
/// @param switches    Switches of MUL instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
#endif
/// @{
#if defined(__gaudi_plus__)
float s_bf16_mul_acc32(bf16 a, bf16 b, int switches=0, float income={}, bool predicate=1, bool polarity=0);
float128 v_bf16_mul_acc32_vb(bfloat128 a, bfloat128 b, int switches, float128 income, bool128 predicate, bool polarity=0);
float128 v_bf16_mul_acc32_b(bfloat128 a, bfloat128 b, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
float s_f16_mul_acc32(half a, half b, int switches=0, float income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
float128 v_f16_mul_acc32_vb(half128 a, half128 b, int switches, float128 income, bool128 predicate, bool polarity=0);
float128 v_f16_mul_acc32_b(half128 a, half128 b, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
float s_f8_mul_acc32(minifloat a, minifloat b, int switches=0, float income={}, bool predicate=1, bool polarity=0);
float256 v_f8_mul_acc32_vb(minifloat256 a, minifloat256 b, int switches, float256 income, bool256 predicate, bool polarity=0);
float256 v_f8_mul_acc32_b(minifloat256 a, minifloat256 b, int switches=0, float256 income={}, bool predicate=1, bool polarity=0);
float s_h8_mul_acc32(minihalf a, minihalf b, int switches=0, float income={}, bool predicate=1, bool polarity=0);
float256 v_h8_mul_acc32_vb(minihalf256 a, minihalf256 b, int switches, float256 income, bool256 predicate, bool polarity=0);
float256 v_h8_mul_acc32_b(minihalf256 a, minihalf256 b, int switches=0, float256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}

/// @section sec2 Arithmetic
/// @subsection subsec2_44 Scalar MUL
/// @brief Represents scalar MUL instruction for integer arguments.
///
/// @param a           The first SRC operand to MUL (SRC1).
/// @param b           The second SRC operand to MUL (SRC2).
/// @param switches    Switches of MUL instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @switches Allowed switches are:
///     \li SW_LOWER32 - When set lower 32 bit of 64 bit result are stored (INT32, UINT32 only).
///     \li SW_UPPER32 - When set upper 32 bit of 64 bit result are stored (INT32, UINT32 only).
///
/// @{
int32_t s_i32_mul(int32_t a, int32_t b, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_u32_mul(uint32_t a, uint32_t b, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int32_t s_i16_mul(int16_t a, int16_t b, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_u16_mul(uint16_t a, uint16_t b, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
#if defined(__goya__) || defined(__greco_plus__)
int32_t s_i8_mul(int8_t a, int8_t b, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_u8_mul(uint8_t a, uint8_t b, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


/// @section sec2 Arithmetic
/// @subsection subsec2_45 Vector MUL
/// @brief Represents vector MUL instruction for integer arguments.
///
/// @param a           The first SRC operand to MUL (SRC1).
/// @param b           The second SRC operand to MUL (SRC2).
/// @param switches    Switches of MUL instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
#if defined(__greco_plus__)
/// @switches Allowed switches are:
///     - [ROUND32]
///         \li SW_KEEP_RS - Keep round/sticky bits for RHAZ using ASH instruction - INT32/UINT32 only.
///         \li SW_KEEP_RS_FOR_ADD - Keep round/sticky bits for RHAZ using ADD instruction - INT32/UINT32 only.
#endif
///
/// @{
int128 v_i32_mul_b(int64 a, int64 b, int switches=0, int128 income={}, bool predicate=1, bool polarity=0);
int128 v_i32_mul_vb(int64 a, int64 b, int switches, int128 income, bool64 predicate, bool polarity=0);
uint128 v_u32_mul_b(uint64 a, uint64 b, int switches=0, uint128 income={}, bool predicate=1, bool polarity=0);
uint128 v_u32_mul_vb(uint64 a, uint64 b, int switches, uint128 income, bool64 predicate, bool polarity=0);
int128 v_i16_mul_b(short128 a, short128 b, int switches=0, int128 income={}, bool predicate=1, bool polarity=0);
int128 v_i16_mul_vb(short128 a, short128 b, int switches, int128 income, bool128 predicate, bool polarity=0);
uint128 v_u16_mul_vb(ushort128 a, ushort128 b, int switches, uint128 income, bool128 predicate, bool polarity=0);
uint128 v_u16_mul_b(ushort128 a, ushort128 b, int switches=0, uint128 income={}, bool predicate=1, bool polarity=0);
#if defined(__goya__) || defined(__greco_plus__)
int256 v_i8_mul_vb(char256 a, char256 b, int switches, int256 income, bool256 predicate, bool polarity=0);
int256 v_i8_mul_b(char256 a, char256 b, int switches=0, int256 income={}, bool predicate=1, bool polarity=0);
uint256 v_u8_mul_vb(uchar256 a, uchar256 b, int switches, uint256 income, bool256 predicate, bool polarity=0);
uint256 v_u8_mul_b(uchar256 a, uchar256 b, int switches=0, uint256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


/// @section sec2 Arithmetic
/// @subsection subsec2_46 Vector MUL with DOUBLE_AND_ROUND32 switch
/// @brief Represents vector MUL instruction with DOUBLE_AND_ROUND32 switch (RHU).
///
/// @param a           The first SRC operand to MUL (SRC1).
/// @param b           The second SRC operand to MUL (SRC2).
/// @param switches    Switches of MUL instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @{
int64 v_i32_mul_round_b(int64 a, int64 b, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_mul_round_vb(int64 a, int64 b, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_mul_round_b(uint64 a, uint64 b, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_mul_round_vb(uint64 a, uint64 b, int switches, uint64 income, bool64 predicate, bool polarity=0);
/// @}


/// @section sec2 Arithmetic
/// @subsection subsec2_47 Scalar Mul for IRF
/// @brief Represents scalar MUL instruction for integer operands in index registers.
///
/// @param a           The first SRC operand to MUL (SRC1).
/// @param b           The second SRC operand to MUL (SRC2).
/// @param dimmask     Selects IRF lanes participated in the operation.
/// @param switches    Switches of MUL instructions.
/// @param income      This value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @{
int5 i_i32_mul(int5 a, int5 b, const int dimmask, int switches, int5 income, bool predicate=1, bool polarity=0);
/// @}


#if defined(__gaudi2_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_48 MUL with X2 switch
/// @brief Represents MUL instruction with X2 switch.
///
/// @param a           The first SRC operand to MUL.
/// @param b           The second SRC operand to MUL.
/// @param c           The third SRC operand to MUL.
/// @param switches    Switches of MUL instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation - \c DEST.v1=a*b \c DEST.v2=c*d.
///
/// @note \p DEST is float128 data-type that map to 2 vector registers of 64 float elements each. \n
/// In the formula above \p DEST.v1 and \p DEST.v2 refers to the first and second vectors respectively.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
float128 v_f32_mul_x2_vb(float128 a, float64 b, float64 c, int switches, float128 income, bool128 predicate, bool polarity=0);
float128 v_f32_mul_x2_b(float128 a, float64 b, float64 c, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
float128 v_f32_mul_x2_svv_vb(float a, float64 b, float64 c, int switches, float128 income, bool128 predicate, bool polarity=0);
float128 v_f32_mul_x2_svv_b(float a, float64 b, float64 c, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ NEARBYINT
//

/// @section sec2 Arithmetic
/// @subsection subsec2_49 Nearby integer
/// @brief Represents NEARBYINT instruction.
///
/// @param a           Input value (SRC1).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Returns the nearest integer value to \p a.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///
/// @{
float s_f32_nearbyint(float a, int switches=0, float income={}, bool predicate=1, bool polarity=0);
float64 v_f32_nearbyint_b(float64 a, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_nearbyint_vb(float64 a, int switches, float64 income, bool64 predicate, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_bf16_nearbyint(bf16 a, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_nearbyint_b(bfloat128 a, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_nearbyint_vb(bfloat128 a, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_f16_nearbyint(half a, int switches=0, half income={}, bool predicate=1, bool polarity=0);
half128 v_f16_nearbyint_b(half128 a, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_nearbyint_vb(half128 a, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat s_f8_nearbyint(minifloat a, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_nearbyint_b(minifloat256 a, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_nearbyint_vb(minifloat256 a, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf s_h8_nearbyint(minihalf a, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_nearbyint_b(minihalf256 a, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_nearbyint_vb(minihalf256 a, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


#if defined(__greco_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_50 NEARBYINt with CNVRT switch
/// @brief Represents NEARBYINT instruction with CNVRT switch.
///
/// @param a           Input value (SRC1).
/// @param switches    Switches of the instruction other than CNVRT.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Returns the nearest integer value to \p a in the first destination \n
///         and convert to int and store the result in the second destination.
///
/// @switches Allowed switches are:
///     - [ROUND]
///         \li SW_RHNE - Round half to nearest even.
///         \li SW_RZ - Round zero.
///         \li SW_RU - Round up.
///         \li SW_RD - Round down.
///
#endif
/// @{
#if defined(__greco_plus__)
float64_int64 v_f32_nearbyint_cnvrt_b(float64 a, int switches=0, float64_int64 income={}, bool predicate=1, bool polarity=0);
float64_int64 v_f32_nearbyint_cnvrt_vb(float64 a, int switches, float64_int64 income, bool64 predicate, bool polarity=0);
bfloat128_short128 v_bf16_nearbyint_cnvrt_b(bfloat128 a, int switches=0, bfloat128_short128 income={}, bool predicate=1, bool polarity=0);
bfloat128_short128 v_bf16_nearbyint_cnvrt_vb(bfloat128 a, int switches, bfloat128_short128 income, bool128 predicate, bool polarity=0);
half128_short128 v_f16_nearbyint_cnvrt_b(half128 a, int switches=0, half128_short128 income={}, bool predicate=1, bool polarity=0);
half128_short128 v_f16_nearbyint_cnvrt_vb(half128 a, int switches, half128_short128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256_char256 v_f8_nearbyint_cnvrt_b(minifloat256 a, int switches=0, minifloat256_char256 income={}, bool predicate=1, bool polarity=0);
minifloat256_char256 v_f8_nearbyint_cnvrt_vb(minifloat256 a, int switches, minifloat256_char256 income, bool256 predicate, bool polarity=0);
minihalf256_char256 v_h8_nearbyint_cnvrt_b(minihalf256 a, int switches=0, minihalf256_char256 income={}, bool predicate=1, bool polarity=0);
minihalf256_char256 v_h8_nearbyint_cnvrt_vb(minihalf256 a, int switches, minihalf256_char256 income, bool256 predicate, bool polarity=0);
#endif
/// @}


//
// ------ NOT
//

/// @section sec2 Arithmetic
/// @subsection subsec2_51 Not
/// @brief Represents NOT instruction.
///
/// @param a           The argument (SRC1).
/// @param switches    Switches of NOT instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @{
float s_f32_not(float a, int switches=0, float income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_bf16_not(bf16 a, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_f16_not(half a, int switches=0, half income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat s_f8_not(minifloat a, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_h8_not(minihalf a, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
#endif
int32_t s_i32_not(int32_t a, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_u32_not(uint32_t a, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_i16_not(int16_t a, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_u16_not(uint16_t a, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_i8_not(int8_t a, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u8_not(uint8_t a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
bool s_i1_not(bool a, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
float64 v_f32_not_b(float64 a, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_not_vb(float64 a, int switches, float64 income, bool64 predicate, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_not_b(bfloat128 a, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_not_vb(bfloat128 a, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
half128 v_f16_not_b(half128 a, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_not_vb(half128 a, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_not_b(minifloat256 a, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_not_vb(minifloat256 a, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_not_b(minihalf256 a, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_not_vb(minihalf256 a, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
int64 v_i32_not_b(int64 a, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_not_vb(int64 a, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_not_b(uint64 a, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_not_vb(uint64 a, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_not_b(short128 a, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_not_vb(short128 a, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_not_b(ushort128 a, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_not_vb(ushort128 a, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_not_b(char256 a, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_not_vb(char256 a, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_not_b(uchar256 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_not_vb(uchar256 a, int switches, uchar256 income, bool256 predicate, bool polarity=0);
bool256 v_i1_not_b(bool256 a, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_not_vb(bool256 a, int switches, bool256 income, bool256 predicate, bool polarity=0);
/// @}


/// @section sec2 Arithmetic
/// @subsection subsec2_52 Not for IRF
/// @brief Represents NOT instruction for int5 operands.
///
/// @param a           The argument (SRC1).
/// @param dimmask     Selects IRF lanes participated in the operation.
/// @param switches    Switches of NOT instructions.
/// @param income      This value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @{
int5 i_i32_not(int5 a, int dimmask, const int switches, int5 income, bool predicate=1, bool polarity=0);
/// @}


//
// ------ OR
//

/// @section sec2 Arithmetic
/// @subsection subsec2_53 OR
/// @brief Represents OR instruction.
///
/// @param a           The first SRC operand to OR (SRC1).
/// @param b           The second SRC operand to OR (SRC2).
/// @param switches    Switches of OR instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @{
float s_f32_or(float a, float b, int switches=0, float income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_bf16_or(bf16 a, bf16 b, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_f16_or(half a, half b, int switches=0, half income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat s_f8_or(minifloat a, minifloat b, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_h8_or(minihalf a, minihalf b, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
#endif
int32_t s_i32_or(int32_t a, int32_t b, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_u32_or(uint32_t a, uint32_t b, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_i16_or(int16_t a, int16_t b, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_u16_or(uint16_t a, uint16_t b, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_i8_or(int8_t a, int8_t b, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u8_or(uint8_t a, uint8_t b, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
bool s_i1_or(bool a, bool b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);
float64 v_f32_or_vb(float64 a, float64 b, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_or_b(float64 a, float64 b, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_or_vb(bfloat128 a, bfloat128 b, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_or_b(bfloat128 a, bfloat128 b, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_f16_or_vb(half128 a, half128 b, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_or_b(half128 a, half128 b, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_or_vb(minifloat256 a, minifloat256 b, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_or_b(minifloat256 a, minifloat256 b, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_or_vb(minihalf256 a, minihalf256 b, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_or_b(minihalf256 a, minihalf256 b, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
int64 v_i32_or_vb(int64 a, int64 b, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_or_b(int64 a, int64 b, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_or_vb(uint64 a, uint64 b, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_or_b(uint64 a, uint64 b, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_or_vb(short128 a, short128 b, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_or_b(short128 a, short128 b, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_or_vb(ushort128 a, ushort128 b, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_or_b(ushort128 a, ushort128 b, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_or_vb(char256 a, char256 b, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_or_b(char256 a, char256 b, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_or_vb(uchar256 a, uchar256 b, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_or_b(uchar256 a, uchar256 b, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_or_b(bool256 a, bool256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_or_vb(bool256 a, bool256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
/// @}


/// @section sec2 Arithmetic
/// @subsection subsec2_54 OR for IRF
/// @brief Represents OR instruction for int5.
///
/// @param a           The first SRC operand to OR (SRC1).
/// @param b           The second SRC operand to OR (SRC2).
/// @param dimmask     Selects IRF lanes participated in the operation.
/// @param switches    Switches of OR instructions.
/// @param income      This value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @{
int5 i_i32_or(int5 a, int5 b, const int dimmask, int switches, int5 income, bool predicate=1, bool polarity=0);
/// @}


//
// ------ PACK
//

/// @section sec10 Pack/Unpack
/// @subsection subsec10_1 Pack
/// @brief Represents PACK instruction.
///
/// @param a           Value in which population is counted (SRC1).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Packed value.
///
/// @switches Allowed switches are:
///     - [GROUP_SOURCE] - Indicates which of the groups in the dual group is to be used as a source.
///         \li SW_GROUP_0
///         \li SW_GROUP_1
///     - [ELEMENT_STRIDE]
///         \li SW_STRIDE_2 - Every second element is valid.
///         \li SW_STRIDE_4 - Every forth element is valid.
///
/// @{
#if defined(__gaudi_plus__)
bfloat128 v_bf16_pack_b(bfloat128 a,int switches, bfloat128 income, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_pack_vb(bfloat128 a, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
half128 v_f16_pack_b(half128 a,int switches, half128 income, bool predicate=1, bool polarity=0);
half128 v_f16_pack_vb(half128 a, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_pack_b(minifloat256 a,int switches, minifloat256 income, bool predicate=1, bool polarity=0);
minifloat256 v_f8_pack_vb(minifloat256 a, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_pack_b(minihalf256 a,int switches, minihalf256 income, bool predicate=1, bool polarity=0);
minihalf256 v_h8_pack_vb(minihalf256 a, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
short128 v_i16_pack_b(short128 a,int switches, short128 income, bool predicate=1, bool polarity=0);
short128 v_i16_pack_vb(short128 a, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_pack_b(ushort128 a,int switches, ushort128 income, bool predicate=1, bool polarity=0);
ushort128 v_u16_pack_vb(ushort128 a, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_pack_b(char256 a,int switches, char256 income, bool predicate=1, bool polarity=0);
char256 v_i8_pack_vb(char256 a, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_pack_b(uchar256 a,int switches, uchar256 income, bool predicate=1, bool polarity=0);
uchar256 v_u8_pack_vb(uchar256 a, int switches, uchar256 income, bool256 predicate, bool polarity=0);
/// @}


//
// ------ POPCNT
//

/// @section sec2 Arithmetic
/// @subsection subsec2_55 Popcnt
/// @brief Represents POPCNT instruction.
///
/// @param a           Value in which population is counted (SRC1).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return The number of 0s or 1s in the elemenet - \p a.
///
/// @switches Allowed switches are:
///     - [SET] - Indicates whether to count 0s or 1s.
///         \li SW_COUNT_ZEROS
///         \li SW_COUNT_ONES
///
/// @{
uint8_t s_f32_popcnt(float a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
uint8_t s_bf16_popcnt(bf16 a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
uint8_t s_f16_popcnt(half a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
uint8_t s_f8_popcnt(minifloat a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_h8_popcnt(minihalf a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
#endif
uint8_t s_i32_popcnt(int32_t a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u32_popcnt(uint32_t a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_i16_popcnt(int16_t a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u16_popcnt(uint16_t a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_i8_popcnt(int8_t a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u8_popcnt(uint8_t a, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
uchar256 v_f32_popcnt_b(float64 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_f32_popcnt_vb(float64 a, int switches, uchar256 income, bool64 predicate, bool polarity=0);
#if defined(__gaudi_plus__)
uchar256 v_bf16_popcnt_b(bfloat128 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_bf16_popcnt_vb(bfloat128 a, int switches, uchar256 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
uchar256 v_f16_popcnt_b(half128 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_f16_popcnt_vb(half128 a, int switches, uchar256 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
uchar256 v_f8_popcnt_b(minifloat256 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_f8_popcnt_vb(minifloat256 a, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_h8_popcnt_b(minihalf256 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_h8_popcnt_vb(minihalf256 a, int switches, uchar256 income, bool256 predicate, bool polarity=0);
#endif
uchar256 v_i32_popcnt_b(int64 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_i32_popcnt_vb(int64 a, int switches, uchar256 income, bool64 predicate, bool polarity=0);
uchar256 v_u32_popcnt_b(uint64 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u32_popcnt_vb(uint64 a, int switches, uchar256 income, bool64 predicate, bool polarity=0);
uchar256 v_i16_popcnt_b(short128 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_i16_popcnt_vb(short128 a, int switches, uchar256 income, bool128 predicate, bool polarity=0);
uchar256 v_u16_popcnt_b(ushort128 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u16_popcnt_vb(ushort128 a, int switches, uchar256 income, bool128 predicate, bool polarity=0);
uchar256 v_i8_popcnt_b(char256 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_i8_popcnt_vb(char256 a, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_popcnt_b(uchar256 a, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_popcnt_vb(uchar256 a, int switches, uchar256 income, bool256 predicate, bool polarity=0);
/// @}


//
// ------ PREFETCH
//

#if defined(__gaudi_plus__)
/// @section sec4 Cache
/// @subsection subsec4_4 Prefetch
/// @brief Represents PREFETCH instruction. Prefetch data from global memory to scalar cache.
///
/// @param a           Address to prefetch (SRC1).
/// @param switches    Switches of the instruction.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
#endif
#if defined(__doron1__)
/// @switches Allowed switches are:
///     \li SW_L2 - prefetch into L2 cache.
#endif 
/// @{
#if defined(__gaudi_plus__)
void prefetch(__global void *a, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ PRMT_INDX
//


/// @section sec11 IRF
/// @subsection subsec11_1 PRMT_INDX
/// @brief Represents PRMT_INDX instruction.
///
/// @param ndx         Source tensor coordinates (SRC1).
/// @param prmt_type   Permutation type - each 3 bits (X5) represent a src dimension in \p ndx (SRC2).
/// @param switches    Switches of the instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Tensor coordinates after permutation.
///
/// @{
int5 prmt_indx(int5 ndx, int prmt_type, int switches, int5 income={}, bool predicate=1, bool polarity=0);
/// @}


//
// ------ SEL_EQ
//


/// @section sec12 Select
/// @subsection subsec12_1 Select Equal
/// @brief Represents SEL_EQ instruction.
///
/// @param a           Source #1 to compare (SRC1).
/// @param b           Source #2 to compare (SRC2).
/// @param c           Source #1 to select (SRC3).
/// @param d           Source #2 to select (SRC4).
/// @param switches    Switches of the instruction.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return One of the sources - \p c or \p d with respect to the comparison of the two other sources - \p a and \p b.
///
#if defined(__gaudi_plus__)
/// @switches Allowed switches are:
///     \li SW_MASK_EQ_ZERO - Compare between (a & b) and 0.
#endif
#if defined(__doron1__)
/// @switches Allowed switches are:
///     \li SW_SUP_NAN - if only one of the inputs in NaN and the other input is a number, the result will be the number.
#endif
///
/// @{
float64 v_f32_sel_eq_f32_vb(float64 a, float64 b, float64 c, float64 d, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_sel_eq_f32_b(float64 a, float64 b, float64 c, float64 d, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_sel_eq_i32_vb(int64 a, int64 b, float64 c, float64 d, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_sel_eq_i32_b(int64 a, int64 b, float64 c, float64 d, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_sel_eq_u32_vb(uint64 a, uint64 b, float64 c, float64 d, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_sel_eq_u32_b(uint64 a, uint64 b, float64 c, float64 d, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
bfloat128 v_bf16_sel_eq_bf16_b(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_sel_eq_bf16_vb(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_sel_eq_i16_vb(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_sel_eq_i16_b(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_sel_eq_u16_vb(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_sel_eq_u16_b(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
bfloat128 v_bf16_sel_eq_f16_b(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_sel_eq_f16_vb(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif

#if defined(__greco_plus__)
half128 v_f16_sel_eq_bf16_b(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_sel_eq_bf16_vb(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_eq_f16_b(half128 a, half128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_sel_eq_f16_vb(half128 a, half128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_eq_i16_vb(short128 a, short128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_eq_i16_b(short128 a, short128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_sel_eq_u16_vb(ushort128 a, ushort128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_eq_u16_b(ushort128 a, ushort128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
#endif

int64 v_i32_sel_eq_f32_vb(float64 a, float64 b, int64 c, int64 d, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_sel_eq_f32_b(float64 a, float64 b, int64 c, int64 d, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_sel_eq_i32_vb(int64 a, int64 b, int64 c, int64 d, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_sel_eq_i32_b(int64 a, int64 b, int64 c, int64 d, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_sel_eq_u32_vb(uint64 a, uint64 b, int64 c, int64 d, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_sel_eq_u32_b(uint64 a, uint64 b, int64 c, int64 d, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);

uint64 v_u32_sel_eq_f32_vb(float64 a, float64 b, uint64 c, uint64 d, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_sel_eq_f32_b(float64 a, float64 b, uint64 c, uint64 d, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_sel_eq_i32_vb(int64 a, int64 b, uint64 c, uint64 d, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_sel_eq_i32_b(int64 a, int64 b, uint64 c, uint64 d, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_sel_eq_u32_vb(uint64 a, uint64 b, uint64 c, uint64 d, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_sel_eq_u32_b(uint64 a, uint64 b, uint64 c, uint64 d, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
short128 v_i16_sel_eq_bf16_vb(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_eq_bf16_b(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#endif
short128 v_i16_sel_eq_i16_vb(short128 a, short128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_eq_i16_b(short128 a, short128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_sel_eq_u16_vb(ushort128 a, ushort128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_eq_u16_b(ushort128 a, ushort128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#if defined(__greco_plus__)
short128 v_i16_sel_eq_f16_vb(half128 a, half128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_eq_f16_b(half128 a, half128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#endif

#if defined(__gaudi_plus__)
ushort128 v_u16_sel_eq_bf16_vb(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_eq_bf16_b(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
#endif
ushort128 v_u16_sel_eq_i16_vb(short128 a, short128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_eq_i16_b(short128 a, short128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_sel_eq_u16_vb(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_eq_u16_b(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
#if defined(__greco_plus__)
ushort128 v_u16_sel_eq_f16_vb(half128 a, half128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_eq_f16_b(half128 a, half128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
#endif

char256 v_i8_sel_eq_i8_vb(char256 a, char256 b, char256 c, char256 d, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_sel_eq_i8_b(char256 a, char256 b, char256 c, char256 d, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_sel_eq_u8_vb(uchar256 a, uchar256 b, char256 c, char256 d, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_sel_eq_u8_b(uchar256 a, uchar256 b, char256 c, char256 d, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);

uchar256 v_u8_sel_eq_i8_vb(char256 a, char256 b, uchar256 c, uchar256 d, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_sel_eq_i8_b(char256 a, char256 b, uchar256 c, uchar256 d, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_sel_eq_u8_vb(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_sel_eq_u8_b(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi2_plus__)
minifloat256 v_f8_sel_eq_h8_b(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_sel_eq_h8_vb(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_eq_f8_b(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_sel_eq_f8_vb(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_eq_i8_vb(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_eq_i8_b(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_sel_eq_u8_vb(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_eq_u8_b(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);

minihalf256 v_h8_sel_eq_f8_b(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_sel_eq_f8_vb(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_eq_h8_b(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_sel_eq_h8_vb(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_eq_i8_vb(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_eq_i8_b(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_sel_eq_u8_vb(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_eq_u8_b(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ SEL_NEQ
//

/// @section sec12 Select
/// @subsection subsec12_2 Select Not Equal
/// @brief Represents SEL_NEQ instruction.
///
/// @param a           Source #1 to compare (SRC1).
/// @param b           Source #2 to compare (SRC2).
/// @param c           Source #1 to select (SRC3).
/// @param d           Source #2 to select (SRC4).
/// @param switches    Switches of the instruction.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return One of the sources - \p c or \p d with respect to the comparison of the two other sources - \p a and \p b.
#if defined(__doron1__)
/// @switches Allowed switches are:
///     \li SW_SUP_NAN - if only one of the inputs in NaN and the other input is
///     a number, the result will be the number.
#endif
///
/// @{
float64 v_f32_sel_neq_f32_vb(float64 a, float64 b, float64 c, float64 d, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_sel_neq_f32_b(float64 a, float64 b, float64 c, float64 d, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_sel_neq_i32_vb(int64 a, int64 b, float64 c, float64 d, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_sel_neq_i32_b(int64 a, int64 b, float64 c, float64 d, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_sel_neq_u32_vb(uint64 a, uint64 b, float64 c, float64 d, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_sel_neq_u32_b(uint64 a, uint64 b, float64 c, float64 d, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
bfloat128 v_bf16_sel_neq_bf16_b(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_sel_neq_bf16_vb(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_sel_neq_i16_vb(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_sel_neq_i16_b(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_sel_neq_u16_vb(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_sel_neq_u16_b(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
bfloat128 v_bf16_sel_neq_f16_b(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_sel_neq_f16_vb(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif

#if defined(__greco_plus__)
half128 v_f16_sel_neq_bf16_b(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_sel_neq_bf16_vb(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_neq_f16_b(half128 a, half128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_sel_neq_f16_vb(half128 a, half128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_neq_i16_vb(short128 a, short128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_neq_i16_b(short128 a, short128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_sel_neq_u16_vb(ushort128 a, ushort128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_neq_u16_b(ushort128 a, ushort128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
#endif

int64 v_i32_sel_neq_f32_vb(float64 a, float64 b, int64 c, int64 d, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_sel_neq_f32_b(float64 a, float64 b, int64 c, int64 d, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_sel_neq_i32_vb(int64 a, int64 b, int64 c, int64 d, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_sel_neq_i32_b(int64 a, int64 b, int64 c, int64 d, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_sel_neq_u32_vb(uint64 a, uint64 b, int64 c, int64 d, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_sel_neq_u32_b(uint64 a, uint64 b, int64 c, int64 d, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);

uint64 v_u32_sel_neq_f32_vb(float64 a, float64 b, uint64 c, uint64 d, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_sel_neq_f32_b(float64 a, float64 b, uint64 c, uint64 d, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_sel_neq_i32_vb(int64 a, int64 b, uint64 c, uint64 d, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_sel_neq_i32_b(int64 a, int64 b, uint64 c, uint64 d, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_sel_neq_u32_vb(uint64 a, uint64 b, uint64 c, uint64 d, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_sel_neq_u32_b(uint64 a, uint64 b, uint64 c, uint64 d, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
short128 v_i16_sel_neq_bf16_vb(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_neq_bf16_b(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#endif
short128 v_i16_sel_neq_i16_vb(short128 a, short128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_neq_i16_b(short128 a, short128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_sel_neq_u16_vb(ushort128 a, ushort128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_neq_u16_b(ushort128 a, ushort128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#if defined(__greco_plus__)
short128 v_i16_sel_neq_f16_vb(half128 a, half128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_neq_f16_b(half128 a, half128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#endif

#if defined(__gaudi_plus__)
ushort128 v_u16_sel_neq_bf16_vb(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_neq_bf16_b(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
#endif
ushort128 v_u16_sel_neq_i16_vb(short128 a, short128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_neq_i16_b(short128 a, short128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_sel_neq_u16_vb(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_neq_u16_b(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
#if defined(__greco_plus__)
ushort128 v_u16_sel_neq_f16_vb(half128 a, half128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_neq_f16_b(half128 a, half128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
#endif

char256 v_i8_sel_neq_i8_vb(char256 a, char256 b, char256 c, char256 d, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_sel_neq_i8_b(char256 a, char256 b, char256 c, char256 d, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_sel_neq_u8_vb(uchar256 a, uchar256 b, char256 c, char256 d, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_sel_neq_u8_b(uchar256 a, uchar256 b, char256 c, char256 d, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);

uchar256 v_u8_sel_neq_i8_vb(char256 a, char256 b, uchar256 c, uchar256 d, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_sel_neq_i8_b(char256 a, char256 b, uchar256 c, uchar256 d, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_sel_neq_u8_vb(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_sel_neq_u8_b(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi2_plus__)
minifloat256 v_f8_sel_neq_h8_b(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_sel_neq_h8_vb(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_neq_f8_b(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_sel_neq_f8_vb(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_neq_i8_vb(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_neq_i8_b(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_sel_neq_u8_vb(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_neq_u8_b(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);

minihalf256 v_h8_sel_neq_f8_b(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_sel_neq_f8_vb(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_neq_h8_b(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_sel_neq_h8_vb(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_neq_i8_vb(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_neq_i8_b(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_sel_neq_u8_vb(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_neq_u8_b(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ SEL_LESS
//

/// @section sec12 Select
/// @subsection subsec12_3 Select Less
/// @brief Represents SEL_LESS instruction.
///
/// @param a           Source #1 to compare (SRC1).
/// @param b           Source #2 to compare (SRC2).
/// @param c           Source #1 to select (SRC3).
/// @param d           Source #2 to select (SRC4).
/// @param switches    Switches of the instruction.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return One of the sources - \p c or \p d with respect to the comparison of the two other sources - \p a and \p b.
#if defined(__doron1__)
/// @switches Allowed switches are:
///     \li SW_SUP_NAN - if only one of the inputs in NaN and the other input is a number, the result will be the number.
#endif
///
/// @{
float64 v_f32_sel_less_f32_vb(float64 a, float64 b, float64 c, float64 d, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_sel_less_f32_b(float64 a, float64 b, float64 c, float64 d, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_sel_less_i32_vb(int64 a, int64 b, float64 c, float64 d, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_sel_less_i32_b(int64 a, int64 b, float64 c, float64 d, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_sel_less_u32_vb(uint64 a, uint64 b, float64 c, float64 d, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_sel_less_u32_b(uint64 a, uint64 b, float64 c, float64 d, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
bfloat128 v_bf16_sel_less_bf16_b(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_sel_less_bf16_vb(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_sel_less_i16_vb(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_sel_less_i16_b(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_sel_less_u16_vb(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_sel_less_u16_b(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
bfloat128 v_bf16_sel_less_f16_b(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_sel_less_f16_vb(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif

#if defined(__greco_plus__)
half128 v_f16_sel_less_bf16_b(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_sel_less_bf16_vb(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_less_f16_b(half128 a, half128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_sel_less_f16_vb(half128 a, half128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_less_i16_vb(short128 a, short128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_less_i16_b(short128 a, short128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_sel_less_u16_vb(ushort128 a, ushort128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_less_u16_b(ushort128 a, ushort128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
#endif

int64 v_i32_sel_less_f32_vb(float64 a, float64 b, int64 c, int64 d, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_sel_less_f32_b(float64 a, float64 b, int64 c, int64 d, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_sel_less_i32_vb(int64 a, int64 b, int64 c, int64 d, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_sel_less_i32_b(int64 a, int64 b, int64 c, int64 d, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_sel_less_u32_vb(uint64 a, uint64 b, int64 c, int64 d, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_sel_less_u32_b(uint64 a, uint64 b, int64 c, int64 d, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);

uint64 v_u32_sel_less_f32_vb(float64 a, float64 b, uint64 c, uint64 d, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_sel_less_f32_b(float64 a, float64 b, uint64 c, uint64 d, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_sel_less_i32_vb(int64 a, int64 b, uint64 c, uint64 d, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_sel_less_i32_b(int64 a, int64 b, uint64 c, uint64 d, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_sel_less_u32_vb(uint64 a, uint64 b, uint64 c, uint64 d, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_sel_less_u32_b(uint64 a, uint64 b, uint64 c, uint64 d, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
short128 v_i16_sel_less_bf16_vb(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_less_bf16_b(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#endif
short128 v_i16_sel_less_i16_vb(short128 a, short128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_less_i16_b(short128 a, short128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_sel_less_u16_vb(ushort128 a, ushort128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_less_u16_b(ushort128 a, ushort128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#if defined(__greco_plus__)
short128 v_i16_sel_less_f16_vb(half128 a, half128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_less_f16_b(half128 a, half128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#endif

#if defined(__gaudi_plus__)
ushort128 v_u16_sel_less_bf16_vb(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_less_bf16_b(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
#endif
ushort128 v_u16_sel_less_i16_vb(short128 a, short128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_less_i16_b(short128 a, short128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_sel_less_u16_vb(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_less_u16_b(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
#if defined(__greco_plus__)
ushort128 v_u16_sel_less_f16_vb(half128 a, half128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_less_f16_b(half128 a, half128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
#endif

char256 v_i8_sel_less_i8_vb(char256 a, char256 b, char256 c, char256 d, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_sel_less_i8_b(char256 a, char256 b, char256 c, char256 d, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_sel_less_u8_vb(uchar256 a, uchar256 b, char256 c, char256 d, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_sel_less_u8_b(uchar256 a, uchar256 b, char256 c, char256 d, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);

uchar256 v_u8_sel_less_i8_vb(char256 a, char256 b, uchar256 c, uchar256 d, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_sel_less_i8_b(char256 a, char256 b, uchar256 c, uchar256 d, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_sel_less_u8_vb(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_sel_less_u8_b(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi2_plus__)
minifloat256 v_f8_sel_less_h8_b(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_sel_less_h8_vb(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_less_f8_b(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_sel_less_f8_vb(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_less_i8_vb(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_less_i8_b(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_sel_less_u8_vb(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_less_u8_b(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);

minihalf256 v_h8_sel_less_f8_b(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_sel_less_f8_vb(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_less_h8_b(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_sel_less_h8_vb(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_less_i8_vb(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_less_i8_b(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_sel_less_u8_vb(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_less_u8_b(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ SEL_LEQ
//

/// @section sec12 Select
/// @subsection subsec12_4 Select less Equal
/// @brief Represents SEL_LEQ instruction.
///
/// @param a           Source #1 to compare (SRC1).
/// @param b           Source #2 to compare (SRC2).
/// @param c           Source #1 to select (SRC3).
/// @param d           Source #2 to select (SRC4).
/// @param switches    Switches of the instruction.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return One of the sources - \p c or \p d with respect to the comparison of the two other sources - \p a and \p b.
#if defined(__doron1__)
/// @switches Allowed switches are:
///     \li SW_SUP_NAN - if only one of the inputs in NaN and the other input is a number, the result will be the number.
#endif
///
/// @{
float64 v_f32_sel_leq_f32_vb(float64 a, float64 b, float64 c, float64 d, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_sel_leq_f32_b(float64 a, float64 b, float64 c, float64 d, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_sel_leq_i32_vb(int64 a, int64 b, float64 c, float64 d, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_sel_leq_i32_b(int64 a, int64 b, float64 c, float64 d, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_sel_leq_u32_vb(uint64 a, uint64 b, float64 c, float64 d, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_sel_leq_u32_b(uint64 a, uint64 b, float64 c, float64 d, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
bfloat128 v_bf16_sel_leq_bf16_b(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_sel_leq_bf16_vb(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_sel_leq_i16_vb(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_sel_leq_i16_b(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_sel_leq_u16_vb(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_sel_leq_u16_b(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
bfloat128 v_bf16_sel_leq_f16_b(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_sel_leq_f16_vb(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif

#if defined(__greco_plus__)
half128 v_f16_sel_leq_bf16_b(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_sel_leq_bf16_vb(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_leq_f16_b(half128 a, half128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_sel_leq_f16_vb(half128 a, half128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_leq_i16_vb(short128 a, short128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_leq_i16_b(short128 a, short128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_sel_leq_u16_vb(ushort128 a, ushort128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_leq_u16_b(ushort128 a, ushort128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
#endif

int64 v_i32_sel_leq_f32_vb(float64 a, float64 b, int64 c, int64 d, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_sel_leq_f32_b(float64 a, float64 b, int64 c, int64 d, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_sel_leq_i32_vb(int64 a, int64 b, int64 c, int64 d, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_sel_leq_i32_b(int64 a, int64 b, int64 c, int64 d, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_sel_leq_u32_vb(uint64 a, uint64 b, int64 c, int64 d, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_sel_leq_u32_b(uint64 a, uint64 b, int64 c, int64 d, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);

uint64 v_u32_sel_leq_f32_vb(float64 a, float64 b, uint64 c, uint64 d, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_sel_leq_f32_b(float64 a, float64 b, uint64 c, uint64 d, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_sel_leq_i32_vb(int64 a, int64 b, uint64 c, uint64 d, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_sel_leq_i32_b(int64 a, int64 b, uint64 c, uint64 d, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_sel_leq_u32_vb(uint64 a, uint64 b, uint64 c, uint64 d, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_sel_leq_u32_b(uint64 a, uint64 b, uint64 c, uint64 d, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
short128 v_i16_sel_leq_bf16_vb(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_leq_bf16_b(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#endif
short128 v_i16_sel_leq_i16_vb(short128 a, short128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_leq_i16_b(short128 a, short128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_sel_leq_u16_vb(ushort128 a, ushort128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_leq_u16_b(ushort128 a, ushort128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#if defined(__greco_plus__)
short128 v_i16_sel_leq_f16_vb(half128 a, half128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_leq_f16_b(half128 a, half128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#endif

#if defined(__gaudi_plus__)
ushort128 v_u16_sel_leq_bf16_vb(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_leq_bf16_b(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
#endif
ushort128 v_u16_sel_leq_i16_vb(short128 a, short128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_leq_i16_b(short128 a, short128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_sel_leq_u16_vb(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_leq_u16_b(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
#if defined(__greco_plus__)
ushort128 v_u16_sel_leq_f16_vb(half128 a, half128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_leq_f16_b(half128 a, half128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
#endif

char256 v_i8_sel_leq_i8_vb(char256 a, char256 b, char256 c, char256 d, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_sel_leq_i8_b(char256 a, char256 b, char256 c, char256 d, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_sel_leq_u8_vb(uchar256 a, uchar256 b, char256 c, char256 d, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_sel_leq_u8_b(uchar256 a, uchar256 b, char256 c, char256 d, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);

uchar256 v_u8_sel_leq_i8_vb(char256 a, char256 b, uchar256 c, uchar256 d, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_sel_leq_i8_b(char256 a, char256 b, uchar256 c, uchar256 d, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_sel_leq_u8_vb(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_sel_leq_u8_b(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi2_plus__)
minifloat256 v_f8_sel_leq_h8_b(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_sel_leq_h8_vb(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_leq_f8_b(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_sel_leq_f8_vb(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_leq_i8_vb(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_leq_i8_b(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_sel_leq_u8_vb(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_leq_u8_b(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);

minihalf256 v_h8_sel_leq_f8_b(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_sel_leq_f8_vb(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_leq_h8_b(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_sel_leq_h8_vb(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_leq_i8_vb(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_leq_i8_b(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_sel_leq_u8_vb(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_leq_u8_b(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ SEL_GRT
//

/// @section sec12 Select
/// @subsection subsec12_5 Select Greater
/// @brief Represents SEL_GRT instruction.
///
/// @param a           Source #1 to compare (SRC1).
/// @param b           Source #2 to compare (SRC2).
/// @param c           Source #1 to select (SRC3).
/// @param d           Source #2 to select (SRC4).
/// @param switches    Switches of the instruction.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return One of the sources - \p c or \p d with respect to the comparison of the two other sources - \p a and \p b.
#if defined(__doron1__)
/// @switches Allowed switches are:
///     \li SW_SUP_NAN - if only one of the inputs in NaN and the other input is a number, the result will be the number.
#endif
///
/// @{
float64 v_f32_sel_grt_f32_vb(float64 a, float64 b, float64 c, float64 d, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_sel_grt_f32_b(float64 a, float64 b, float64 c, float64 d, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_sel_grt_i32_vb(int64 a, int64 b, float64 c, float64 d, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_sel_grt_i32_b(int64 a, int64 b, float64 c, float64 d, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_sel_grt_u32_vb(uint64 a, uint64 b, float64 c, float64 d, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_sel_grt_u32_b(uint64 a, uint64 b, float64 c, float64 d, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
bfloat128 v_bf16_sel_grt_bf16_b(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_sel_grt_bf16_vb(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_sel_grt_i16_vb(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_sel_grt_i16_b(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_sel_grt_u16_vb(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_sel_grt_u16_b(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
bfloat128 v_bf16_sel_grt_f16_b(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_sel_grt_f16_vb(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif

#if defined(__greco_plus__)
half128 v_f16_sel_grt_bf16_b(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_sel_grt_bf16_vb(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_grt_f16_b(half128 a, half128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_sel_grt_f16_vb(half128 a, half128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_grt_i16_vb(short128 a, short128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_grt_i16_b(short128 a, short128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_sel_grt_u16_vb(ushort128 a, ushort128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_grt_u16_b(ushort128 a, ushort128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
#endif

int64 v_i32_sel_grt_f32_vb(float64 a, float64 b, int64 c, int64 d, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_sel_grt_f32_b(float64 a, float64 b, int64 c, int64 d, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_sel_grt_i32_vb(int64 a, int64 b, int64 c, int64 d, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_sel_grt_i32_b(int64 a, int64 b, int64 c, int64 d, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_sel_grt_u32_vb(uint64 a, uint64 b, int64 c, int64 d, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_sel_grt_u32_b(uint64 a, uint64 b, int64 c, int64 d, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);

uint64 v_u32_sel_grt_f32_vb(float64 a, float64 b, uint64 c, uint64 d, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_sel_grt_f32_b(float64 a, float64 b, uint64 c, uint64 d, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_sel_grt_i32_vb(int64 a, int64 b, uint64 c, uint64 d, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_sel_grt_i32_b(int64 a, int64 b, uint64 c, uint64 d, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_sel_grt_u32_vb(uint64 a, uint64 b, uint64 c, uint64 d, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_sel_grt_u32_b(uint64 a, uint64 b, uint64 c, uint64 d, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
short128 v_i16_sel_grt_bf16_vb(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_grt_bf16_b(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#endif
short128 v_i16_sel_grt_i16_vb(short128 a, short128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_grt_i16_b(short128 a, short128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_sel_grt_u16_vb(ushort128 a, ushort128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_grt_u16_b(ushort128 a, ushort128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#if defined(__greco_plus__)
short128 v_i16_sel_grt_f16_vb(half128 a, half128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_grt_f16_b(half128 a, half128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#endif

#if defined(__gaudi_plus__)
ushort128 v_u16_sel_grt_bf16_vb(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_grt_bf16_b(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
#endif
ushort128 v_u16_sel_grt_i16_vb(short128 a, short128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_grt_i16_b(short128 a, short128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_sel_grt_u16_vb(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_grt_u16_b(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
#if defined(__greco_plus__)
ushort128 v_u16_sel_grt_f16_vb(half128 a, half128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_grt_f16_b(half128 a, half128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
#endif

char256 v_i8_sel_grt_i8_vb(char256 a, char256 b, char256 c, char256 d, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_sel_grt_i8_b(char256 a, char256 b, char256 c, char256 d, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_sel_grt_u8_vb(uchar256 a, uchar256 b, char256 c, char256 d, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_sel_grt_u8_b(uchar256 a, uchar256 b, char256 c, char256 d, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);

uchar256 v_u8_sel_grt_i8_vb(char256 a, char256 b, uchar256 c, uchar256 d, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_sel_grt_i8_b(char256 a, char256 b, uchar256 c, uchar256 d, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_sel_grt_u8_vb(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_sel_grt_u8_b(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi2_plus__)
minifloat256 v_f8_sel_grt_h8_b(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_sel_grt_h8_vb(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_grt_f8_b(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_sel_grt_f8_vb(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_grt_i8_vb(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_grt_i8_b(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_sel_grt_u8_vb(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_grt_u8_b(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);

minihalf256 v_h8_sel_grt_f8_b(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_sel_grt_f8_vb(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_grt_h8_b(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_sel_grt_h8_vb(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_grt_i8_vb(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_grt_i8_b(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_sel_grt_u8_vb(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_grt_u8_b(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ SEL_GEQ
//

/// @section sec12 Select
/// @subsection subsec12_6 Select Greater Equal
/// @brief Represents SEL_GEQ instruction.
///
/// @param a           Source #1 to compare (SRC1).
/// @param b           Source #2 to compare (SRC2).
/// @param c           Source #1 to select (SRC3).
/// @param d           Source #2 to select (SRC4).
/// @param switches    Switches of the instruction.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return One of the sources - \p c or \p d with respect to the comparison of the two other sources - \p a and \p b.
#if defined(__doron1__)
/// @switches Allowed switches are:
///     \li SW_SUP_NAN - if only one of the inputs in NaN and the other input is a number, the result will be the number.
#endif
///
/// @{
float64 v_f32_sel_geq_f32_vb(float64 a, float64 b, float64 c, float64 d, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_sel_geq_f32_b(float64 a, float64 b, float64 c, float64 d, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_sel_geq_i32_vb(int64 a, int64 b, float64 c, float64 d, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_sel_geq_i32_b(int64 a, int64 b, float64 c, float64 d, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_sel_geq_u32_vb(uint64 a, uint64 b, float64 c, float64 d, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_sel_geq_u32_b(uint64 a, uint64 b, float64 c, float64 d, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
bfloat128 v_bf16_sel_geq_bf16_b(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_sel_geq_bf16_vb(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_sel_geq_i16_vb(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_sel_geq_i16_b(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_sel_geq_u16_vb(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_sel_geq_u16_b(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
bfloat128 v_bf16_sel_geq_f16_b(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_sel_geq_f16_vb(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif

#if defined(__greco_plus__)
half128 v_f16_sel_geq_bf16_b(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_sel_geq_bf16_vb(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_geq_f16_b(half128 a, half128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_sel_geq_f16_vb(half128 a, half128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_geq_i16_vb(short128 a, short128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_geq_i16_b(short128 a, short128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_sel_geq_u16_vb(ushort128 a, ushort128 b, half128 c, half128 d, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sel_geq_u16_b(ushort128 a, ushort128 b, half128 c, half128 d, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
#endif

int64 v_i32_sel_geq_f32_vb(float64 a, float64 b, int64 c, int64 d, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_sel_geq_f32_b(float64 a, float64 b, int64 c, int64 d, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_sel_geq_i32_vb(int64 a, int64 b, int64 c, int64 d, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_sel_geq_i32_b(int64 a, int64 b, int64 c, int64 d, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_sel_geq_u32_vb(uint64 a, uint64 b, int64 c, int64 d, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_sel_geq_u32_b(uint64 a, uint64 b, int64 c, int64 d, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);

uint64 v_u32_sel_geq_f32_vb(float64 a, float64 b, uint64 c, uint64 d, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_sel_geq_f32_b(float64 a, float64 b, uint64 c, uint64 d, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_sel_geq_i32_vb(int64 a, int64 b, uint64 c, uint64 d, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_sel_geq_i32_b(int64 a, int64 b, uint64 c, uint64 d, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_sel_geq_u32_vb(uint64 a, uint64 b, uint64 c, uint64 d, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_sel_geq_u32_b(uint64 a, uint64 b, uint64 c, uint64 d, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
short128 v_i16_sel_geq_bf16_vb(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_geq_bf16_b(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#endif
short128 v_i16_sel_geq_i16_vb(short128 a, short128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_geq_i16_b(short128 a, short128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_sel_geq_u16_vb(ushort128 a, ushort128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_geq_u16_b(ushort128 a, ushort128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#if defined(__greco_plus__)
short128 v_i16_sel_geq_f16_vb(half128 a, half128 b, short128 c, short128 d, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sel_geq_f16_b(half128 a, half128 b, short128 c, short128 d, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
#endif

#if defined(__gaudi_plus__)
ushort128 v_u16_sel_geq_bf16_vb(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_geq_bf16_b(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
#endif
ushort128 v_u16_sel_geq_i16_vb(short128 a, short128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_geq_i16_b(short128 a, short128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_sel_geq_u16_vb(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_geq_u16_b(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
#if defined(__greco_plus__)
ushort128 v_u16_sel_geq_f16_vb(half128 a, half128 b, ushort128 c, ushort128 d, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sel_geq_f16_b(half128 a, half128 b, ushort128 c, ushort128 d, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
#endif

char256 v_i8_sel_geq_i8_vb(char256 a, char256 b, char256 c, char256 d, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_sel_geq_i8_b(char256 a, char256 b, char256 c, char256 d, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_sel_geq_u8_vb(uchar256 a, uchar256 b, char256 c, char256 d, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_sel_geq_u8_b(uchar256 a, uchar256 b, char256 c, char256 d, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);

uchar256 v_u8_sel_geq_i8_vb(char256 a, char256 b, uchar256 c, uchar256 d, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_sel_geq_i8_b(char256 a, char256 b, uchar256 c, uchar256 d, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_sel_geq_u8_vb(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_sel_geq_u8_b(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi2_plus__)
minifloat256 v_f8_sel_geq_h8_b(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_sel_geq_h8_vb(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_geq_f8_b(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_sel_geq_f8_vb(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_geq_i8_vb(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_geq_i8_b(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_sel_geq_u8_vb(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_sel_geq_u8_b(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);

minihalf256 v_h8_sel_geq_f8_b(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_sel_geq_f8_vb(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_geq_h8_b(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_sel_geq_h8_vb(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_geq_i8_vb(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_geq_i8_b(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_sel_geq_u8_vb(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_sel_geq_u8_b(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ SEL2_LESS
//

/// @section sec12 Select
/// @subsection subsec12_7 SEL2_LESS
/// @brief Represents SEL2_LESS instruction.
///
/// @param a           Source #1 to compare and select (SRC1).
/// @param b           Source #2 to compare and select (SRC2).
/// @param c           Source #1 to select (SRC3).
/// @param d           Source #2 to select (SRC4).
/// @param switches    Switches of the instruction.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Pair (c, a) or (d, b) with respect to the comparison of \p a and \p b.
#if defined(__doron1__)
/// @switches Allowed switches are:
///     \li SW_SUP_NAN - if only one of the inputs in NaN and the other input is a number, the result will be the number.
#endif
///
/// @{
float128 v_f32_sel2_less_f32_vb(float64 a, float64 b, float64 c, float64 d, int switches, float128 income, bool64 predicate, bool polarity=0);
float128 v_f32_sel2_less_f32_b(float64 a, float64 b, float64 c, float64 d, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
float64_int64 v_f32_sel2_less_i32_vb(int64 a, int64 b, float64 c, float64 d, int switches, float64_int64 income, bool64 predicate, bool polarity=0);
float64_int64 v_f32_sel2_less_i32_b(int64 a, int64 b, float64 c, float64 d, int switches=0, float64_int64 income={}, bool predicate=1, bool polarity=0);
float64_uint64 v_f32_sel2_less_u32_vb(uint64 a, uint64 b, float64 c, float64 d, int switches, float64_uint64 income, bool64 predicate, bool polarity=0);
float64_uint64 v_f32_sel2_less_u32_b(uint64 a, uint64 b, float64 c, float64 d, int switches=0, float64_uint64 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
bfloat256 v_bf16_sel2_less_bf16_b(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat256 income={}, bool predicate=1, bool polarity=0);
bfloat256 v_bf16_sel2_less_bf16_vb(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches, bfloat256 income, bool128 predicate, bool polarity=0);
bfloat128_short128 v_bf16_sel2_less_i16_b(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128_short128 income={}, bool predicate=1, bool polarity=0);
bfloat128_short128 v_bf16_sel2_less_i16_vb(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches, bfloat128_short128 income, bool128 predicate, bool polarity=0);
bfloat128_ushort128 v_bf16_sel2_less_u16_b(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128_ushort128 income={}, bool predicate=1, bool polarity=0);
bfloat128_ushort128 v_bf16_sel2_less_u16_vb(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches, bfloat128_ushort128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
bfloat128_half128 v_bf16_sel2_less_f16_vb(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches, bfloat128_half128 income, bool128 predicate, bool polarity=0);
bfloat128_half128 v_bf16_sel2_less_f16_b(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128_half128 income={}, bool predicate=1, bool polarity=0);
#endif

#if defined(__greco_plus__)
half128_bfloat128 v_f16_sel2_less_bf16_b(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches=0, half128_bfloat128 income={}, bool predicate=1, bool polarity=0);
half128_bfloat128 v_f16_sel2_less_bf16_vb(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches, half128_bfloat128 income, bool128 predicate, bool polarity=0);
half256 v_f16_sel2_less_f16_b(half128 a, half128 b, half128 c, half128 d, int switches=0, half256 income={}, bool predicate=1, bool polarity=0);
half256 v_f16_sel2_less_f16_vb(half128 a, half128 b, half128 c, half128 d, int switches, half256 income, bool128 predicate, bool polarity=0);
half128_short128 v_f16_sel2_less_i16_b(short128 a, short128 b, half128 c, half128 d, int switches=0, half128_short128 income={}, bool predicate=1, bool polarity=0);
half128_short128 v_f16_sel2_less_i16_vb(short128 a, short128 b, half128 c, half128 d, int switches, half128_short128 income, bool128 predicate, bool polarity=0);
half128_ushort128 v_f16_sel2_less_u16_b(ushort128 a, ushort128 b, half128 c, half128 d, int switches=0, half128_ushort128 income={}, bool predicate=1, bool polarity=0);
half128_ushort128 v_f16_sel2_less_u16_vb(ushort128 a, ushort128 b, half128 c, half128 d, int switches, half128_ushort128 income, bool128 predicate, bool polarity=0);
#endif

int64_float64 v_i32_sel2_less_f32_vb(float64 a, float64 b, int64 c, int64 d, int switches, int64_float64 income, bool64 predicate, bool polarity=0);
int64_float64 v_i32_sel2_less_f32_b(float64 a, float64 b, int64 c, int64 d, int switches=0, int64_float64 income={}, bool predicate=1, bool polarity=0);
int128 v_i32_sel2_less_i32_vb(int64 a, int64 b, int64 c, int64 d, int switches, int128 income, bool64 predicate, bool polarity=0);
int128 v_i32_sel2_less_i32_b(int64 a, int64 b, int64 c, int64 d, int switches=0, int128 income={}, bool predicate=1, bool polarity=0);
int64_uint64 v_i32_sel2_less_u32_vb(uint64 a, uint64 b, int64 c, int64 d, int switches, int64_uint64 income, bool64 predicate, bool polarity=0);
int64_uint64 v_i32_sel2_less_u32_b(uint64 a, uint64 b, int64 c, int64 d, int switches=0, int64_uint64 income={}, bool predicate=1, bool polarity=0);

uint64_float64 v_u32_sel2_less_f32_vb(float64 a, float64 b, uint64 c, uint64 d, int switches, uint64_float64 income, bool64 predicate, bool polarity=0);
uint64_float64 v_u32_sel2_less_f32_b(float64 a, float64 b, uint64 c, uint64 d, int switches=0, uint64_float64 income={}, bool predicate=1, bool polarity=0);
uint64_int64 v_u32_sel2_less_i32_vb(int64 a, int64 b, uint64 c, uint64 d, int switches, uint64_int64 income, bool64 predicate, bool polarity=0);
uint64_int64 v_u32_sel2_less_i32_b(int64 a, int64 b, uint64 c, uint64 d, int switches=0, uint64_int64 income={}, bool predicate=1, bool polarity=0);
uint128 v_u32_sel2_less_u32_vb(uint64 a, uint64 b, uint64 c, uint64 d, int switches, uint128 income, bool64 predicate, bool polarity=0);
uint128 v_u32_sel2_less_u32_b(uint64 a, uint64 b, uint64 c, uint64 d, int switches=0, uint128 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
short128_bfloat128 v_i16_sel2_less_bf16_vb(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches, short128_bfloat128 income, bool128 predicate, bool polarity=0);
short128_bfloat128 v_i16_sel2_less_bf16_b(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches=0, short128_bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
short128_half128 v_i16_sel2_less_f16_vb(half128 a, half128 b, short128 c, short128 d, int switches, short128_half128 income, bool128 predicate, bool polarity=0);
short128_half128 v_i16_sel2_less_f16_b(half128 a, half128 b, short128 c, short128 d, int switches=0, short128_half128 income={}, bool predicate=1, bool polarity=0);
#endif
short256 v_i16_sel2_less_i16_vb(short128 a, short128 b, short128 c, short128 d, int switches, short256 income, bool128 predicate, bool polarity=0);
short256 v_i16_sel2_less_i16_b(short128 a, short128 b, short128 c, short128 d, int switches=0, short256 income={}, bool predicate=1, bool polarity=0);
short128_ushort128 v_i16_sel2_less_u16_vb(ushort128 a, ushort128 b, short128 c, short128 d, int switches, short128_ushort128 income, bool128 predicate, bool polarity=0);
short128_ushort128 v_i16_sel2_less_u16_b(ushort128 a, ushort128 b, short128 c, short128 d, int switches=0, short128_ushort128 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
ushort128_bfloat128 v_u16_sel2_less_bf16_vb(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches, ushort128_bfloat128 income, bool128 predicate, bool polarity=0);
ushort128_bfloat128 v_u16_sel2_less_bf16_b(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches=0, ushort128_bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
ushort128_half128 v_u16_sel2_less_f16_vb(half128 a, half128 b, ushort128 c, ushort128 d, int switches, ushort128_half128 income, bool128 predicate, bool polarity=0);
ushort128_half128 v_u16_sel2_less_f16_b(half128 a, half128 b, ushort128 c, ushort128 d, int switches=0, ushort128_half128 income={}, bool predicate=1, bool polarity=0);
#endif
ushort128_short128 v_u16_sel2_less_i16_vb(short128 a, short128 b, ushort128 c, ushort128 d, int switches, ushort128_short128 income, bool128 predicate, bool polarity=0);
ushort128_short128 v_u16_sel2_less_i16_b(short128 a, short128 b, ushort128 c, ushort128 d, int switches=0, ushort128_short128 income={}, bool predicate=1, bool polarity=0);
ushort256 v_u16_sel2_less_u16_vb(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches, ushort256 income, bool128 predicate, bool polarity=0);
ushort256 v_u16_sel2_less_u16_b(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches=0, ushort256 income={}, bool predicate=1, bool polarity=0);

char512 v_i8_sel2_less_i8_vb(char256 a, char256 b, char256 c, char256 d, int switches, char512 income, bool256 predicate, bool polarity=0);
char512 v_i8_sel2_less_i8_b(char256 a, char256 b, char256 c, char256 d, int switches=0, char512 income={}, bool predicate=1, bool polarity=0);
char256_uchar256 v_i8_sel2_less_u8_vb(uchar256 a, uchar256 b, char256 c, char256 d, int switches, char256_uchar256 income, bool256 predicate, bool polarity=0);
char256_uchar256 v_i8_sel2_less_u8_b(uchar256 a, uchar256 b, char256 c, char256 d, int switches=0, char256_uchar256 income={}, bool predicate=1, bool polarity=0);

uchar256_char256 v_u8_sel2_less_i8_vb(char256 a, char256 b, uchar256 c, uchar256 d, int switches, uchar256_char256 income, bool256 predicate, bool polarity=0);
uchar256_char256 v_u8_sel2_less_i8_b(char256 a, char256 b, uchar256 c, uchar256 d, int switches=0, uchar256_char256 income={}, bool predicate=1, bool polarity=0);
uchar512 v_u8_sel2_less_u8_vb(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches, uchar512 income, bool256 predicate, bool polarity=0);
uchar512 v_u8_sel2_less_u8_b(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches=0, uchar512 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi2_plus__)
minifloat512 v_f8_sel2_less_f8_b(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat512 income={}, bool predicate=1, bool polarity=0);
minifloat512 v_f8_sel2_less_f8_vb(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches, minifloat512 income, bool256 predicate, bool polarity=0);

minifloat256_char256 v_f8_sel2_less_i8_b(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256_char256 income={}, bool predicate=1, bool polarity=0);
minifloat256_char256 v_f8_sel2_less_i8_vb(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches, minifloat256_char256 income, bool256 predicate, bool polarity=0);
minifloat256_uchar256 v_f8_sel2_less_u8_b(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256_uchar256 income={}, bool predicate=1, bool polarity=0);
minifloat256_uchar256 v_f8_sel2_less_u8_vb(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches, minifloat256_uchar256 income, bool256 predicate, bool polarity=0);

minifloat256_minihalf256 v_f8_sel2_less_h8_b(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256_minihalf256 income={}, bool predicate=1, bool polarity=0);
minifloat256_minihalf256 v_f8_sel2_less_h8_vb(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches, minifloat256_minihalf256 income, bool256 predicate, bool polarity=0);

minihalf256_minifloat256 v_h8_sel2_less_f8_b(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256_minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256_minifloat256 v_h8_sel2_less_f8_vb(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches, minihalf256_minifloat256 income, bool256 predicate, bool polarity=0);

minihalf512 v_h8_sel2_less_h8_b(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf512 income={}, bool predicate=1, bool polarity=0);
minihalf512 v_h8_sel2_less_h8_vb(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches, minihalf512 income, bool256 predicate, bool polarity=0);

minihalf256_char256 v_h8_sel2_less_i8_b(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256_char256 income={}, bool predicate=1, bool polarity=0);
minihalf256_char256 v_h8_sel2_less_i8_vb(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches, minihalf256_char256 income, bool256 predicate, bool polarity=0);
minihalf256_uchar256 v_h8_sel2_less_u8_b(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256_uchar256 income={}, bool predicate=1, bool polarity=0);
minihalf256_uchar256 v_h8_sel2_less_u8_vb(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches, minihalf256_uchar256 income, bool256 predicate, bool polarity=0);

char256_minifloat256 v_i8_sel2_less_f8_vb(minifloat256 a, minifloat256 b, char256 c, char256 d, int switches, char256_minifloat256 income, bool256 predicate, bool polarity=0);
char256_minifloat256 v_i8_sel2_less_f8_b(minifloat256 a, minifloat256 b, char256 c, char256 d, int switches=0, char256_minifloat256 income={}, bool predicate=1, bool polarity=0);
char256_minihalf256 v_i8_sel2_less_h8_vb(char256_minihalf256 a, char256_minihalf256 b, char256 c, char256 d, int switches, char256_minifloat256 income, bool256 predicate, bool polarity=0);
char256_minifloat256 v_i8_sel2_less_h8_b(char256_minihalf256 a, char256_minihalf256 b, char256 c, char256 d, int switches=0, char256_minifloat256 income={}, bool predicate=1, bool polarity=0);

uchar256_minifloat256 v_u8_sel2_less_f8_vb(minifloat256 a, minifloat256 b, uchar256 c, uchar256 d, int switches, ushort128_bfloat128 income, bool256 predicate, bool polarity=0);
uchar256_minifloat256 v_u8_sel2_less_f8_b(minifloat256 a, minifloat256 b, uchar256 c, uchar256 d, int switches=0, uchar256_minifloat256 income={}, bool predicate=1, bool polarity=0);
uchar256_minihalf256 v_u8_sel2_less_h8_vb(minihalf256 a, minihalf256 b, uchar256 c, uchar256 d, int switches, uchar256_minihalf256 income, bool256 predicate, bool polarity=0);
uchar256_minihalf256 v_u8_sel2_less_h8_b(minihalf256 a, minihalf256 b, uchar256 c, uchar256 d, int switches=0, uchar256_minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ SEL2_LEQ
//

/// @section sec12 Select
/// @subsection subsec12_8 SEL2 Less Equal
/// @brief Represents SEL2_LEQ instruction.
///
/// @param a           Source #1 to compare and select (SRC1).
/// @param b           Source #2 to compare and select (SRC2).
/// @param c           Source #1 to select (SRC3).
/// @param d           Source #2 to select (SRC4).
/// @param switches    Switches of the instruction.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Pair (c, a) or (d, b) with respect to the comparison of \p a and \p b.
#if defined(__doron1__)
/// @switches Allowed switches are:
///     \li SW_SUP_NAN - if only one of the inputs in NaN and the other input is a number, the result will be the number.
#endif
///
/// @{
float128 v_f32_sel2_leq_f32_vb(float64 a, float64 b, float64 c, float64 d, int switches, float128 income, bool64 predicate, bool polarity=0);
float128 v_f32_sel2_leq_f32_b(float64 a, float64 b, float64 c, float64 d, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
float64_int64 v_f32_sel2_leq_i32_vb(int64 a, int64 b, float64 c, float64 d, int switches, float64_int64 income, bool64 predicate, bool polarity=0);
float64_int64 v_f32_sel2_leq_i32_b(int64 a, int64 b, float64 c, float64 d, int switches=0, float64_int64 income={}, bool predicate=1, bool polarity=0);
float64_uint64 v_f32_sel2_leq_u32_vb(uint64 a, uint64 b, float64 c, float64 d, int switches, float64_uint64 income, bool64 predicate, bool polarity=0);
float64_uint64 v_f32_sel2_leq_u32_b(uint64 a, uint64 b, float64 c, float64 d, int switches=0, float64_uint64 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
bfloat256 v_bf16_sel2_leq_bf16_b(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat256 income={}, bool predicate=1, bool polarity=0);
bfloat256 v_bf16_sel2_leq_bf16_vb(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches, bfloat256 income, bool128 predicate, bool polarity=0);
bfloat128_short128 v_bf16_sel2_leq_i16_b(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128_short128 income={}, bool predicate=1, bool polarity=0);
bfloat128_short128 v_bf16_sel2_leq_i16_vb(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches, bfloat128_short128 income, bool128 predicate, bool polarity=0);
bfloat128_ushort128 v_bf16_sel2_leq_u16_b(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128_ushort128 income={}, bool predicate=1, bool polarity=0);
bfloat128_ushort128 v_bf16_sel2_leq_u16_vb(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches, bfloat128_ushort128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
bfloat128_half128 v_bf16_sel2_leq_f16_vb(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches, bfloat128_half128 income, bool128 predicate, bool polarity=0);
bfloat128_half128 v_bf16_sel2_leq_f16_b(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128_half128 income={}, bool predicate=1, bool polarity=0);
#endif

#if defined(__greco_plus__)
half128_bfloat128 v_f16_sel2_leq_bf16_b(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches=0, half128_bfloat128 income={}, bool predicate=1, bool polarity=0);
half128_bfloat128 v_f16_sel2_leq_bf16_vb(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches, half128_bfloat128 income, bool128 predicate, bool polarity=0);
half256 v_f16_sel2_leq_f16_b(half128 a, half128 b, half128 c, half128 d, int switches=0, half256 income={}, bool predicate=1, bool polarity=0);
half256 v_f16_sel2_leq_f16_vb(half128 a, half128 b, half128 c, half128 d, int switches, half256 income, bool128 predicate, bool polarity=0);
half128_short128 v_f16_sel2_leq_i16_b(short128 a, short128 b, half128 c, half128 d, int switches=0, half128_short128 income={}, bool predicate=1, bool polarity=0);
half128_short128 v_f16_sel2_leq_i16_vb(short128 a, short128 b, half128 c, half128 d, int switches, half128_short128 income, bool128 predicate, bool polarity=0);
half128_ushort128 v_f16_sel2_leq_u16_b(ushort128 a, ushort128 b, half128 c, half128 d, int switches=0, half128_ushort128 income={}, bool predicate=1, bool polarity=0);
half128_ushort128 v_f16_sel2_leq_u16_vb(ushort128 a, ushort128 b, half128 c, half128 d, int switches, half128_ushort128 income, bool128 predicate, bool polarity=0);
#endif

int64_float64 v_i32_sel2_leq_f32_vb(float64 a, float64 b, int64 c, int64 d, int switches, int64_float64 income, bool64 predicate, bool polarity=0);
int64_float64 v_i32_sel2_leq_f32_b(float64 a, float64 b, int64 c, int64 d, int switches=0, int64_float64 income={}, bool predicate=1, bool polarity=0);
int128 v_i32_sel2_leq_i32_vb(int64 a, int64 b, int64 c, int64 d, int switches, int128 income, bool64 predicate, bool polarity=0);
int128 v_i32_sel2_leq_i32_b(int64 a, int64 b, int64 c, int64 d, int switches=0, int128 income={}, bool predicate=1, bool polarity=0);
int64_uint64 v_i32_sel2_leq_u32_vb(uint64 a, uint64 b, int64 c, int64 d, int switches, int64_uint64 income, bool64 predicate, bool polarity=0);
int64_uint64 v_i32_sel2_leq_u32_b(uint64 a, uint64 b, int64 c, int64 d, int switches=0, int64_uint64 income={}, bool predicate=1, bool polarity=0);

uint64_float64 v_u32_sel2_leq_f32_vb(float64 a, float64 b, uint64 c, uint64 d, int switches, uint64_float64 income, bool64 predicate, bool polarity=0);
uint64_float64 v_u32_sel2_leq_f32_b(float64 a, float64 b, uint64 c, uint64 d, int switches=0, uint64_float64 income={}, bool predicate=1, bool polarity=0);
uint64_int64 v_u32_sel2_leq_i32_vb(int64 a, int64 b, uint64 c, uint64 d, int switches, uint64_int64 income, bool64 predicate, bool polarity=0);
uint64_int64 v_u32_sel2_leq_i32_b(int64 a, int64 b, uint64 c, uint64 d, int switches=0, uint64_int64 income={}, bool predicate=1, bool polarity=0);
uint128 v_u32_sel2_leq_u32_vb(uint64 a, uint64 b, uint64 c, uint64 d, int switches, uint128 income, bool64 predicate, bool polarity=0);
uint128 v_u32_sel2_leq_u32_b(uint64 a, uint64 b, uint64 c, uint64 d, int switches=0, uint128 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
short128_bfloat128 v_i16_sel2_leq_bf16_vb(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches, short128_bfloat128 income, bool128 predicate, bool polarity=0);
short128_bfloat128 v_i16_sel2_leq_bf16_b(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches=0, short128_bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
short128_half128 v_i16_sel2_leq_f16_vb(half128 a, half128 b, short128 c, short128 d, int switches, short128_half128 income, bool128 predicate, bool polarity=0);
short128_half128 v_i16_sel2_leq_f16_b(half128 a, half128 b, short128 c, short128 d, int switches=0, short128_half128 income={}, bool predicate=1, bool polarity=0);
#endif
short256 v_i16_sel2_leq_i16_vb(short128 a, short128 b, short128 c, short128 d, int switches, short256 income, bool128 predicate, bool polarity=0);
short256 v_i16_sel2_leq_i16_b(short128 a, short128 b, short128 c, short128 d, int switches=0, short256 income={}, bool predicate=1, bool polarity=0);
short128_ushort128 v_i16_sel2_leq_u16_vb(ushort128 a, ushort128 b, short128 c, short128 d, int switches, short128_ushort128 income, bool128 predicate, bool polarity=0);
short128_ushort128 v_i16_sel2_leq_u16_b(ushort128 a, ushort128 b, short128 c, short128 d, int switches=0, short128_ushort128 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
ushort128_bfloat128 v_u16_sel2_leq_bf16_vb(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches, ushort128_bfloat128 income, bool128 predicate, bool polarity=0);
ushort128_bfloat128 v_u16_sel2_leq_bf16_b(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches=0, ushort128_bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
ushort128_half128 v_u16_sel2_leq_f16_vb(half128 a, half128 b, ushort128 c, ushort128 d, int switches, ushort128_half128 income, bool128 predicate, bool polarity=0);
ushort128_half128 v_u16_sel2_leq_f16_b(half128 a, half128 b, ushort128 c, ushort128 d, int switches=0, ushort128_half128 income={}, bool predicate=1, bool polarity=0);
#endif
ushort128_short128 v_u16_sel2_leq_i16_vb(short128 a, short128 b, ushort128 c, ushort128 d, int switches, ushort128_short128 income, bool128 predicate, bool polarity=0);
ushort128_short128 v_u16_sel2_leq_i16_b(short128 a, short128 b, ushort128 c, ushort128 d, int switches=0, ushort128_short128 income={}, bool predicate=1, bool polarity=0);
ushort256 v_u16_sel2_leq_u16_vb(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches, ushort256 income, bool128 predicate, bool polarity=0);
ushort256 v_u16_sel2_leq_u16_b(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches=0, ushort256 income={}, bool predicate=1, bool polarity=0);

char512 v_i8_sel2_leq_i8_vb(char256 a, char256 b, char256 c, char256 d, int switches, char512 income, bool256 predicate, bool polarity=0);
char512 v_i8_sel2_leq_i8_b(char256 a, char256 b, char256 c, char256 d, int switches=0, char512 income={}, bool predicate=1, bool polarity=0);
char256_uchar256 v_i8_sel2_leq_u8_vb(uchar256 a, uchar256 b, char256 c, char256 d, int switches, char256_uchar256 income, bool256 predicate, bool polarity=0);
char256_uchar256 v_i8_sel2_leq_u8_b(uchar256 a, uchar256 b, char256 c, char256 d, int switches=0, char256_uchar256 income={}, bool predicate=1, bool polarity=0);

uchar256_char256 v_u8_sel2_leq_i8_vb(char256 a, char256 b, uchar256 c, uchar256 d, int switches, uchar256_char256 income, bool256 predicate, bool polarity=0);
uchar256_char256 v_u8_sel2_leq_i8_b(char256 a, char256 b, uchar256 c, uchar256 d, int switches=0, uchar256_char256 income={}, bool predicate=1, bool polarity=0);
uchar512 v_u8_sel2_leq_u8_vb(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches, uchar512 income, bool256 predicate, bool polarity=0);
uchar512 v_u8_sel2_leq_u8_b(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches=0, uchar512 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi2_plus__)
minifloat512 v_f8_sel2_leq_f8_b(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat512 income={}, bool predicate=1, bool polarity=0);
minifloat512 v_f8_sel2_leq_f8_vb(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches, minifloat512 income, bool256 predicate, bool polarity=0);

minifloat256_char256 v_f8_sel2_leq_i8_b(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256_char256 income={}, bool predicate=1, bool polarity=0);
minifloat256_char256 v_f8_sel2_leq_i8_vb(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches, minifloat256_char256 income, bool256 predicate, bool polarity=0);
minifloat256_uchar256 v_f8_sel2_leq_u8_b(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256_uchar256 income={}, bool predicate=1, bool polarity=0);
minifloat256_uchar256 v_f8_sel2_leq_u8_vb(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches, minifloat256_uchar256 income, bool256 predicate, bool polarity=0);

minifloat256_minihalf256 v_f8_sel2_leq_h8_b(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256_minihalf256 income={}, bool predicate=1, bool polarity=0);
minifloat256_minihalf256 v_f8_sel2_leq_h8_vb(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches, minifloat256_minihalf256 income, bool256 predicate, bool polarity=0);

minihalf256_minifloat256 v_h8_sel2_leq_f8_b(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256_minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256_minifloat256 v_h8_sel2_leq_f8_vb(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches, minihalf256_minifloat256 income, bool256 predicate, bool polarity=0);

minihalf512 v_h8_sel2_leq_h8_b(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf512 income={}, bool predicate=1, bool polarity=0);
minihalf512 v_h8_sel2_leq_h8_vb(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches, minihalf512 income, bool256 predicate, bool polarity=0);

minihalf256_char256 v_h8_sel2_leq_i8_b(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256_char256 income={}, bool predicate=1, bool polarity=0);
minihalf256_char256 v_h8_sel2_leq_i8_vb(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches, minihalf256_char256 income, bool256 predicate, bool polarity=0);
minihalf256_uchar256 v_h8_sel2_leq_u8_b(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256_uchar256 income={}, bool predicate=1, bool polarity=0);
minihalf256_uchar256 v_h8_sel2_leq_u8_vb(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches, minihalf256_uchar256 income, bool256 predicate, bool polarity=0);

char256_minifloat256 v_i8_sel2_leq_f8_vb(minifloat256 a, minifloat256 b, char256 c, char256 d, int switches, char256_minifloat256 income, bool256 predicate, bool polarity=0);
char256_minifloat256 v_i8_sel2_leq_f8_b(minifloat256 a, minifloat256 b, char256 c, char256 d, int switches=0, char256_minifloat256 income={}, bool predicate=1, bool polarity=0);
char256_minihalf256 v_i8_sel2_leq_h8_vb(char256_minihalf256 a, char256_minihalf256 b, char256 c, char256 d, int switches, char256_minifloat256 income, bool256 predicate, bool polarity=0);
char256_minifloat256 v_i8_sel2_leq_h8_b(char256_minihalf256 a, char256_minihalf256 b, char256 c, char256 d, int switches=0, char256_minifloat256 income={}, bool predicate=1, bool polarity=0);

uchar256_minifloat256 v_u8_sel2_leq_f8_vb(minifloat256 a, minifloat256 b, uchar256 c, uchar256 d, int switches, ushort128_bfloat128 income, bool256 predicate, bool polarity=0);
uchar256_minifloat256 v_u8_sel2_leq_f8_b(minifloat256 a, minifloat256 b, uchar256 c, uchar256 d, int switches=0, uchar256_minifloat256 income={}, bool predicate=1, bool polarity=0);
uchar256_minihalf256 v_u8_sel2_leq_h8_vb(minihalf256 a, minihalf256 b, uchar256 c, uchar256 d, int switches, uchar256_minihalf256 income, bool256 predicate, bool polarity=0);
uchar256_minihalf256 v_u8_sel2_leq_h8_b(minihalf256 a, minihalf256 b, uchar256 c, uchar256 d, int switches=0, uchar256_minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ SEL2_GRT
//

/// @section sec12 Select
/// @subsection subsec12_9 SEL2 Greater
/// @brief Represents SEL2_GRT instruction.
///
/// @param a           Source #1 to compare and select (SRC1).
/// @param b           Source #2 to compare and select (SRC2).
/// @param c           Source #1 to select (SRC3).
/// @param d           Source #2 to select (SRC4).
/// @param switches    Switches of the instruction.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Pair (c, a) or (d, b) with respect to the comparison of \p a and \p b.
#if defined(__doron1__)
/// @switches Allowed switches are:
///     \li SW_SUP_NAN - if only one of the inputs in NaN and the other input is a number, the result will be the number.
#endif
///
/// @{
float128 v_f32_sel2_grt_f32_vb(float64 a, float64 b, float64 c, float64 d, int switches, float128 income, bool64 predicate, bool polarity=0);
float128 v_f32_sel2_grt_f32_b(float64 a, float64 b, float64 c, float64 d, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
float64_int64 v_f32_sel2_grt_i32_vb(int64 a, int64 b, float64 c, float64 d, int switches, float64_int64 income, bool64 predicate, bool polarity=0);
float64_int64 v_f32_sel2_grt_i32_b(int64 a, int64 b, float64 c, float64 d, int switches=0, float64_int64 income={}, bool predicate=1, bool polarity=0);
float64_uint64 v_f32_sel2_grt_u32_vb(uint64 a, uint64 b, float64 c, float64 d, int switches, float64_uint64 income, bool64 predicate, bool polarity=0);
float64_uint64 v_f32_sel2_grt_u32_b(uint64 a, uint64 b, float64 c, float64 d, int switches=0, float64_uint64 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
bfloat256 v_bf16_sel2_grt_bf16_b(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat256 income={}, bool predicate=1, bool polarity=0);
bfloat256 v_bf16_sel2_grt_bf16_vb(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches, bfloat256 income, bool128 predicate, bool polarity=0);
bfloat128_short128 v_bf16_sel2_grt_i16_b(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128_short128 income={}, bool predicate=1, bool polarity=0);
bfloat128_short128 v_bf16_sel2_grt_i16_vb(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches, bfloat128_short128 income, bool128 predicate, bool polarity=0);
bfloat128_ushort128 v_bf16_sel2_grt_u16_b(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128_ushort128 income={}, bool predicate=1, bool polarity=0);
bfloat128_ushort128 v_bf16_sel2_grt_u16_vb(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches, bfloat128_ushort128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
bfloat128_half128 v_bf16_sel2_grt_f16_vb(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches, bfloat128_half128 income, bool128 predicate, bool polarity=0);
bfloat128_half128 v_bf16_sel2_grt_f16_b(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128_half128 income={}, bool predicate=1, bool polarity=0);
#endif

#if defined(__greco_plus__)
half128_bfloat128 v_f16_sel2_grt_bf16_b(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches=0, half128_bfloat128 income={}, bool predicate=1, bool polarity=0);
half128_bfloat128 v_f16_sel2_grt_bf16_vb(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches, half128_bfloat128 income, bool128 predicate, bool polarity=0);
half256 v_f16_sel2_grt_f16_b(half128 a, half128 b, half128 c, half128 d, int switches=0, half256 income={}, bool predicate=1, bool polarity=0);
half256 v_f16_sel2_grt_f16_vb(half128 a, half128 b, half128 c, half128 d, int switches, half256 income, bool128 predicate, bool polarity=0);
half128_short128 v_f16_sel2_grt_i16_b(short128 a, short128 b, half128 c, half128 d, int switches=0, half128_short128 income={}, bool predicate=1, bool polarity=0);
half128_short128 v_f16_sel2_grt_i16_vb(short128 a, short128 b, half128 c, half128 d, int switches, half128_short128 income, bool128 predicate, bool polarity=0);
half128_ushort128 v_f16_sel2_grt_u16_b(ushort128 a, ushort128 b, half128 c, half128 d, int switches=0, half128_ushort128 income={}, bool predicate=1, bool polarity=0);
half128_ushort128 v_f16_sel2_grt_u16_vb(ushort128 a, ushort128 b, half128 c, half128 d, int switches, half128_ushort128 income, bool128 predicate, bool polarity=0);
#endif

int64_float64 v_i32_sel2_grt_f32_vb(float64 a, float64 b, int64 c, int64 d, int switches, int64_float64 income, bool64 predicate, bool polarity=0);
int64_float64 v_i32_sel2_grt_f32_b(float64 a, float64 b, int64 c, int64 d, int switches=0, int64_float64 income={}, bool predicate=1, bool polarity=0);
int128 v_i32_sel2_grt_i32_vb(int64 a, int64 b, int64 c, int64 d, int switches, int128 income, bool64 predicate, bool polarity=0);
int128 v_i32_sel2_grt_i32_b(int64 a, int64 b, int64 c, int64 d, int switches=0, int128 income={}, bool predicate=1, bool polarity=0);
int64_uint64 v_i32_sel2_grt_u32_vb(uint64 a, uint64 b, int64 c, int64 d, int switches, int64_uint64 income, bool64 predicate, bool polarity=0);
int64_uint64 v_i32_sel2_grt_u32_b(uint64 a, uint64 b, int64 c, int64 d, int switches=0, int64_uint64 income={}, bool predicate=1, bool polarity=0);

uint64_float64 v_u32_sel2_grt_f32_vb(float64 a, float64 b, uint64 c, uint64 d, int switches, uint64_float64 income, bool64 predicate, bool polarity=0);
uint64_float64 v_u32_sel2_grt_f32_b(float64 a, float64 b, uint64 c, uint64 d, int switches=0, uint64_float64 income={}, bool predicate=1, bool polarity=0);
uint64_int64 v_u32_sel2_grt_i32_vb(int64 a, int64 b, uint64 c, uint64 d, int switches, uint64_int64 income, bool64 predicate, bool polarity=0);
uint64_int64 v_u32_sel2_grt_i32_b(int64 a, int64 b, uint64 c, uint64 d, int switches=0, uint64_int64 income={}, bool predicate=1, bool polarity=0);
uint128 v_u32_sel2_grt_u32_vb(uint64 a, uint64 b, uint64 c, uint64 d, int switches, uint128 income, bool64 predicate, bool polarity=0);
uint128 v_u32_sel2_grt_u32_b(uint64 a, uint64 b, uint64 c, uint64 d, int switches=0, uint128 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
short128_bfloat128 v_i16_sel2_grt_bf16_vb(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches, short128_bfloat128 income, bool128 predicate, bool polarity=0);
short128_bfloat128 v_i16_sel2_grt_bf16_b(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches=0, short128_bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
short128_half128 v_i16_sel2_grt_f16_vb(half128 a, half128 b, short128 c, short128 d, int switches, short128_half128 income, bool128 predicate, bool polarity=0);
short128_half128 v_i16_sel2_grt_f16_b(half128 a, half128 b, short128 c, short128 d, int switches=0, short128_half128 income={}, bool predicate=1, bool polarity=0);
#endif
short256 v_i16_sel2_grt_i16_vb(short128 a, short128 b, short128 c, short128 d, int switches, short256 income, bool128 predicate, bool polarity=0);
short256 v_i16_sel2_grt_i16_b(short128 a, short128 b, short128 c, short128 d, int switches=0, short256 income={}, bool predicate=1, bool polarity=0);
short128_ushort128 v_i16_sel2_grt_u16_vb(ushort128 a, ushort128 b, short128 c, short128 d, int switches, short128_ushort128 income, bool128 predicate, bool polarity=0);
short128_ushort128 v_i16_sel2_grt_u16_b(ushort128 a, ushort128 b, short128 c, short128 d, int switches=0, short128_ushort128 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
ushort128_bfloat128 v_u16_sel2_grt_bf16_vb(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches, ushort128_bfloat128 income, bool128 predicate, bool polarity=0);
ushort128_bfloat128 v_u16_sel2_grt_bf16_b(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches=0, ushort128_bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
ushort128_half128 v_u16_sel2_grt_f16_vb(half128 a, half128 b, ushort128 c, ushort128 d, int switches, ushort128_half128 income, bool128 predicate, bool polarity=0);
ushort128_half128 v_u16_sel2_grt_f16_b(half128 a, half128 b, ushort128 c, ushort128 d, int switches=0, ushort128_half128 income={}, bool predicate=1, bool polarity=0);
#endif
ushort128_short128 v_u16_sel2_grt_i16_vb(short128 a, short128 b, ushort128 c, ushort128 d, int switches, ushort128_short128 income, bool128 predicate, bool polarity=0);
ushort128_short128 v_u16_sel2_grt_i16_b(short128 a, short128 b, ushort128 c, ushort128 d, int switches=0, ushort128_short128 income={}, bool predicate=1, bool polarity=0);
ushort256 v_u16_sel2_grt_u16_vb(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches, ushort256 income, bool128 predicate, bool polarity=0);
ushort256 v_u16_sel2_grt_u16_b(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches=0, ushort256 income={}, bool predicate=1, bool polarity=0);

char512 v_i8_sel2_grt_i8_vb(char256 a, char256 b, char256 c, char256 d, int switches, char512 income, bool256 predicate, bool polarity=0);
char512 v_i8_sel2_grt_i8_b(char256 a, char256 b, char256 c, char256 d, int switches=0, char512 income={}, bool predicate=1, bool polarity=0);
char256_uchar256 v_i8_sel2_grt_u8_vb(uchar256 a, uchar256 b, char256 c, char256 d, int switches, char256_uchar256 income, bool256 predicate, bool polarity=0);
char256_uchar256 v_i8_sel2_grt_u8_b(uchar256 a, uchar256 b, char256 c, char256 d, int switches=0, char256_uchar256 income={}, bool predicate=1, bool polarity=0);

uchar256_char256 v_u8_sel2_grt_i8_vb(char256 a, char256 b, uchar256 c, uchar256 d, int switches, uchar256_char256 income, bool256 predicate, bool polarity=0);
uchar256_char256 v_u8_sel2_grt_i8_b(char256 a, char256 b, uchar256 c, uchar256 d, int switches=0, uchar256_char256 income={}, bool predicate=1, bool polarity=0);
uchar512 v_u8_sel2_grt_u8_vb(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches, uchar512 income, bool256 predicate, bool polarity=0);
uchar512 v_u8_sel2_grt_u8_b(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches=0, uchar512 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi2_plus__)
minifloat512 v_f8_sel2_grt_f8_b(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat512 income={}, bool predicate=1, bool polarity=0);
minifloat512 v_f8_sel2_grt_f8_vb(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches, minifloat512 income, bool256 predicate, bool polarity=0);

minifloat256_char256 v_f8_sel2_grt_i8_b(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256_char256 income={}, bool predicate=1, bool polarity=0);
minifloat256_char256 v_f8_sel2_grt_i8_vb(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches, minifloat256_char256 income, bool256 predicate, bool polarity=0);
minifloat256_uchar256 v_f8_sel2_grt_u8_b(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256_uchar256 income={}, bool predicate=1, bool polarity=0);
minifloat256_uchar256 v_f8_sel2_grt_u8_vb(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches, minifloat256_uchar256 income, bool256 predicate, bool polarity=0);

minifloat256_minihalf256 v_f8_sel2_grt_h8_b(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256_minihalf256 income={}, bool predicate=1, bool polarity=0);
minifloat256_minihalf256 v_f8_sel2_grt_h8_vb(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches, minifloat256_minihalf256 income, bool256 predicate, bool polarity=0);

minihalf256_minifloat256 v_h8_sel2_grt_f8_b(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256_minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256_minifloat256 v_h8_sel2_grt_f8_vb(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches, minihalf256_minifloat256 income, bool256 predicate, bool polarity=0);

minihalf512 v_h8_sel2_grt_h8_b(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf512 income={}, bool predicate=1, bool polarity=0);
minihalf512 v_h8_sel2_grt_h8_vb(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches, minihalf512 income, bool256 predicate, bool polarity=0);

minihalf256_char256 v_h8_sel2_grt_i8_b(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256_char256 income={}, bool predicate=1, bool polarity=0);
minihalf256_char256 v_h8_sel2_grt_i8_vb(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches, minihalf256_char256 income, bool256 predicate, bool polarity=0);
minihalf256_uchar256 v_h8_sel2_grt_u8_b(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256_uchar256 income={}, bool predicate=1, bool polarity=0);
minihalf256_uchar256 v_h8_sel2_grt_u8_vb(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches, minihalf256_uchar256 income, bool256 predicate, bool polarity=0);

char256_minifloat256 v_i8_sel2_grt_f8_vb(minifloat256 a, minifloat256 b, char256 c, char256 d, int switches, char256_minifloat256 income, bool256 predicate, bool polarity=0);
char256_minifloat256 v_i8_sel2_grt_f8_b(minifloat256 a, minifloat256 b, char256 c, char256 d, int switches=0, char256_minifloat256 income={}, bool predicate=1, bool polarity=0);
char256_minihalf256 v_i8_sel2_grt_h8_vb(char256_minihalf256 a, char256_minihalf256 b, char256 c, char256 d, int switches, char256_minifloat256 income, bool256 predicate, bool polarity=0);
char256_minifloat256 v_i8_sel2_grt_h8_b(char256_minihalf256 a, char256_minihalf256 b, char256 c, char256 d, int switches=0, char256_minifloat256 income={}, bool predicate=1, bool polarity=0);

uchar256_minifloat256 v_u8_sel2_grt_f8_vb(minifloat256 a, minifloat256 b, uchar256 c, uchar256 d, int switches, ushort128_bfloat128 income, bool256 predicate, bool polarity=0);
uchar256_minifloat256 v_u8_sel2_grt_f8_b(minifloat256 a, minifloat256 b, uchar256 c, uchar256 d, int switches=0, uchar256_minifloat256 income={}, bool predicate=1, bool polarity=0);
uchar256_minihalf256 v_u8_sel2_grt_h8_vb(minihalf256 a, minihalf256 b, uchar256 c, uchar256 d, int switches, uchar256_minihalf256 income, bool256 predicate, bool polarity=0);
uchar256_minihalf256 v_u8_sel2_grt_h8_b(minihalf256 a, minihalf256 b, uchar256 c, uchar256 d, int switches=0, uchar256_minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}

//
// ------ SEL2_GEQ
//

/// @section sec12 Select
/// @subsection subsec12_10 SEL2 Greater Equal
/// @brief Represents SEL2_GEQ instruction.
///
/// @param a           Source #1 to compare and select (SRC1).
/// @param b           Source #2 to compare and select (SRC2).
/// @param c           Source #1 to select (SRC3).
/// @param d           Source #2 to select (SRC4).
/// @param switches    Switches of the instruction.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Pair (c, a) or (d, b) with respect to the comparison of \p a and \p b.
#if defined(__doron1__)
/// @switches Allowed switches are:
///     \li SW_SUP_NAN - if only one of the inputs in NaN and the other input is a number, the result will be the number.
#endif
///
/// @{
float128 v_f32_sel2_geq_f32_vb(float64 a, float64 b, float64 c, float64 d, int switches, float128 income, bool64 predicate, bool polarity=0);
float128 v_f32_sel2_geq_f32_b(float64 a, float64 b, float64 c, float64 d, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
float64_int64 v_f32_sel2_geq_i32_vb(int64 a, int64 b, float64 c, float64 d, int switches, float64_int64 income, bool64 predicate, bool polarity=0);
float64_int64 v_f32_sel2_geq_i32_b(int64 a, int64 b, float64 c, float64 d, int switches=0, float64_int64 income={}, bool predicate=1, bool polarity=0);
float64_uint64 v_f32_sel2_geq_u32_vb(uint64 a, uint64 b, float64 c, float64 d, int switches, float64_uint64 income, bool64 predicate, bool polarity=0);
float64_uint64 v_f32_sel2_geq_u32_b(uint64 a, uint64 b, float64 c, float64 d, int switches=0, float64_uint64 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
bfloat256 v_bf16_sel2_geq_bf16_b(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat256 income={}, bool predicate=1, bool polarity=0);
bfloat256 v_bf16_sel2_geq_bf16_vb(bfloat128 a, bfloat128 b, bfloat128 c, bfloat128 d, int switches, bfloat256 income, bool128 predicate, bool polarity=0);
bfloat128_short128 v_bf16_sel2_geq_i16_b(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128_short128 income={}, bool predicate=1, bool polarity=0);
bfloat128_short128 v_bf16_sel2_geq_i16_vb(short128 a, short128 b, bfloat128 c, bfloat128 d, int switches, bfloat128_short128 income, bool128 predicate, bool polarity=0);
bfloat128_ushort128 v_bf16_sel2_geq_u16_b(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128_ushort128 income={}, bool predicate=1, bool polarity=0);
bfloat128_ushort128 v_bf16_sel2_geq_u16_vb(ushort128 a, ushort128 b, bfloat128 c, bfloat128 d, int switches, bfloat128_ushort128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
bfloat128_half128 v_bf16_sel2_geq_f16_vb(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches, bfloat128_half128 income, bool128 predicate, bool polarity=0);
bfloat128_half128 v_bf16_sel2_geq_f16_b(half128 a, half128 b, bfloat128 c, bfloat128 d, int switches=0, bfloat128_half128 income={}, bool predicate=1, bool polarity=0);
#endif

#if defined(__greco_plus__)
half128_bfloat128 v_f16_sel2_geq_bf16_b(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches=0, half128_bfloat128 income={}, bool predicate=1, bool polarity=0);
half128_bfloat128 v_f16_sel2_geq_bf16_vb(bfloat128 a, bfloat128 b, half128 c, half128 d, int switches, half128_bfloat128 income, bool128 predicate, bool polarity=0);
half256 v_f16_sel2_geq_f16_b(half128 a, half128 b, half128 c, half128 d, int switches=0, half256 income={}, bool predicate=1, bool polarity=0);
half256 v_f16_sel2_geq_f16_vb(half128 a, half128 b, half128 c, half128 d, int switches, half256 income, bool128 predicate, bool polarity=0);
half128_short128 v_f16_sel2_geq_i16_b(short128 a, short128 b, half128 c, half128 d, int switches=0, half128_short128 income={}, bool predicate=1, bool polarity=0);
half128_short128 v_f16_sel2_geq_i16_vb(short128 a, short128 b, half128 c, half128 d, int switches, half128_short128 income, bool128 predicate, bool polarity=0);
half128_ushort128 v_f16_sel2_geq_u16_b(ushort128 a, ushort128 b, half128 c, half128 d, int switches=0, half128_ushort128 income={}, bool predicate=1, bool polarity=0);
half128_ushort128 v_f16_sel2_geq_u16_vb(ushort128 a, ushort128 b, half128 c, half128 d, int switches, half128_ushort128 income, bool128 predicate, bool polarity=0);
#endif

int64_float64 v_i32_sel2_geq_f32_vb(float64 a, float64 b, int64 c, int64 d, int switches, int64_float64 income, bool64 predicate, bool polarity=0);
int64_float64 v_i32_sel2_geq_f32_b(float64 a, float64 b, int64 c, int64 d, int switches=0, int64_float64 income={}, bool predicate=1, bool polarity=0);
int128 v_i32_sel2_geq_i32_vb(int64 a, int64 b, int64 c, int64 d, int switches, int128 income, bool64 predicate, bool polarity=0);
int128 v_i32_sel2_geq_i32_b(int64 a, int64 b, int64 c, int64 d, int switches=0, int128 income={}, bool predicate=1, bool polarity=0);
int64_uint64 v_i32_sel2_geq_u32_vb(uint64 a, uint64 b, int64 c, int64 d, int switches, int64_uint64 income, bool64 predicate, bool polarity=0);
int64_uint64 v_i32_sel2_geq_u32_b(uint64 a, uint64 b, int64 c, int64 d, int switches=0, int64_uint64 income={}, bool predicate=1, bool polarity=0);

uint64_float64 v_u32_sel2_geq_f32_vb(float64 a, float64 b, uint64 c, uint64 d, int switches, uint64_float64 income, bool64 predicate, bool polarity=0);
uint64_float64 v_u32_sel2_geq_f32_b(float64 a, float64 b, uint64 c, uint64 d, int switches=0, uint64_float64 income={}, bool predicate=1, bool polarity=0);
uint64_int64 v_u32_sel2_geq_i32_vb(int64 a, int64 b, uint64 c, uint64 d, int switches, uint64_int64 income, bool64 predicate, bool polarity=0);
uint64_int64 v_u32_sel2_geq_i32_b(int64 a, int64 b, uint64 c, uint64 d, int switches=0, uint64_int64 income={}, bool predicate=1, bool polarity=0);
uint128 v_u32_sel2_geq_u32_vb(uint64 a, uint64 b, uint64 c, uint64 d, int switches, uint128 income, bool64 predicate, bool polarity=0);
uint128 v_u32_sel2_geq_u32_b(uint64 a, uint64 b, uint64 c, uint64 d, int switches=0, uint128 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
short128_bfloat128 v_i16_sel2_geq_bf16_vb(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches, short128_bfloat128 income, bool128 predicate, bool polarity=0);
short128_bfloat128 v_i16_sel2_geq_bf16_b(bfloat128 a, bfloat128 b, short128 c, short128 d, int switches=0, short128_bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
short128_half128 v_i16_sel2_geq_f16_vb(half128 a, half128 b, short128 c, short128 d, int switches, short128_half128 income, bool128 predicate, bool polarity=0);
short128_half128 v_i16_sel2_geq_f16_b(half128 a, half128 b, short128 c, short128 d, int switches=0, short128_half128 income={}, bool predicate=1, bool polarity=0);
#endif
short256 v_i16_sel2_geq_i16_vb(short128 a, short128 b, short128 c, short128 d, int switches, short256 income, bool128 predicate, bool polarity=0);
short256 v_i16_sel2_geq_i16_b(short128 a, short128 b, short128 c, short128 d, int switches=0, short256 income={}, bool predicate=1, bool polarity=0);
short128_ushort128 v_i16_sel2_geq_u16_vb(ushort128 a, ushort128 b, short128 c, short128 d, int switches, short128_ushort128 income, bool128 predicate, bool polarity=0);
short128_ushort128 v_i16_sel2_geq_u16_b(ushort128 a, ushort128 b, short128 c, short128 d, int switches=0, short128_ushort128 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi_plus__)
ushort128_bfloat128 v_u16_sel2_geq_bf16_vb(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches, ushort128_bfloat128 income, bool128 predicate, bool polarity=0);
ushort128_bfloat128 v_u16_sel2_geq_bf16_b(bfloat128 a, bfloat128 b, ushort128 c, ushort128 d, int switches=0, ushort128_bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
ushort128_half128 v_u16_sel2_geq_f16_vb(half128 a, half128 b, ushort128 c, ushort128 d, int switches, ushort128_half128 income, bool128 predicate, bool polarity=0);
ushort128_half128 v_u16_sel2_geq_f16_b(half128 a, half128 b, ushort128 c, ushort128 d, int switches=0, ushort128_half128 income={}, bool predicate=1, bool polarity=0);
#endif
ushort128_short128 v_u16_sel2_geq_i16_vb(short128 a, short128 b, ushort128 c, ushort128 d, int switches, ushort128_short128 income, bool128 predicate, bool polarity=0);
ushort128_short128 v_u16_sel2_geq_i16_b(short128 a, short128 b, ushort128 c, ushort128 d, int switches=0, ushort128_short128 income={}, bool predicate=1, bool polarity=0);
ushort256 v_u16_sel2_geq_u16_vb(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches, ushort256 income, bool128 predicate, bool polarity=0);
ushort256 v_u16_sel2_geq_u16_b(ushort128 a, ushort128 b, ushort128 c, ushort128 d, int switches=0, ushort256 income={}, bool predicate=1, bool polarity=0);

char512 v_i8_sel2_geq_i8_vb(char256 a, char256 b, char256 c, char256 d, int switches, char512 income, bool256 predicate, bool polarity=0);
char512 v_i8_sel2_geq_i8_b(char256 a, char256 b, char256 c, char256 d, int switches=0, char512 income={}, bool predicate=1, bool polarity=0);
char256_uchar256 v_i8_sel2_geq_u8_vb(uchar256 a, uchar256 b, char256 c, char256 d, int switches, char256_uchar256 income, bool256 predicate, bool polarity=0);
char256_uchar256 v_i8_sel2_geq_u8_b(uchar256 a, uchar256 b, char256 c, char256 d, int switches=0, char256_uchar256 income={}, bool predicate=1, bool polarity=0);

uchar256_char256 v_u8_sel2_geq_i8_vb(char256 a, char256 b, uchar256 c, uchar256 d, int switches, uchar256_char256 income, bool256 predicate, bool polarity=0);
uchar256_char256 v_u8_sel2_geq_i8_b(char256 a, char256 b, uchar256 c, uchar256 d, int switches=0, uchar256_char256 income={}, bool predicate=1, bool polarity=0);
uchar512 v_u8_sel2_geq_u8_vb(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches, uchar512 income, bool256 predicate, bool polarity=0);
uchar512 v_u8_sel2_geq_u8_b(uchar256 a, uchar256 b, uchar256 c, uchar256 d, int switches=0, uchar512 income={}, bool predicate=1, bool polarity=0);

#if defined(__gaudi2_plus__)
minifloat512 v_f8_sel2_geq_f8_b(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat512 income={}, bool predicate=1, bool polarity=0);
minifloat512 v_f8_sel2_geq_f8_vb(minifloat256 a, minifloat256 b, minifloat256 c, minifloat256 d, int switches, minifloat512 income, bool256 predicate, bool polarity=0);

minifloat256_char256 v_f8_sel2_geq_i8_b(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256_char256 income={}, bool predicate=1, bool polarity=0);
minifloat256_char256 v_f8_sel2_geq_i8_vb(char256 a, char256 b, minifloat256 c, minifloat256 d, int switches, minifloat256_char256 income, bool256 predicate, bool polarity=0);
minifloat256_uchar256 v_f8_sel2_geq_u8_b(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256_uchar256 income={}, bool predicate=1, bool polarity=0);
minifloat256_uchar256 v_f8_sel2_geq_u8_vb(uchar256 a, uchar256 b, minifloat256 c, minifloat256 d, int switches, minifloat256_uchar256 income, bool256 predicate, bool polarity=0);

minifloat256_minihalf256 v_f8_sel2_geq_h8_b(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches=0, minifloat256_minihalf256 income={}, bool predicate=1, bool polarity=0);
minifloat256_minihalf256 v_f8_sel2_geq_h8_vb(minihalf256 a, minihalf256 b, minifloat256 c, minifloat256 d, int switches, minifloat256_minihalf256 income, bool256 predicate, bool polarity=0);

minihalf256_minifloat256 v_h8_sel2_geq_f8_b(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256_minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256_minifloat256 v_h8_sel2_geq_f8_vb(minifloat256 a, minifloat256 b, minihalf256 c, minihalf256 d, int switches, minihalf256_minifloat256 income, bool256 predicate, bool polarity=0);

minihalf512 v_h8_sel2_geq_h8_b(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf512 income={}, bool predicate=1, bool polarity=0);
minihalf512 v_h8_sel2_geq_h8_vb(minihalf256 a, minihalf256 b, minihalf256 c, minihalf256 d, int switches, minihalf512 income, bool256 predicate, bool polarity=0);

minihalf256_char256 v_h8_sel2_geq_i8_b(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256_char256 income={}, bool predicate=1, bool polarity=0);
minihalf256_char256 v_h8_sel2_geq_i8_vb(char256 a, char256 b, minihalf256 c, minihalf256 d, int switches, minihalf256_char256 income, bool256 predicate, bool polarity=0);
minihalf256_uchar256 v_h8_sel2_geq_u8_b(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches=0, minihalf256_uchar256 income={}, bool predicate=1, bool polarity=0);
minihalf256_uchar256 v_h8_sel2_geq_u8_vb(uchar256 a, uchar256 b, minihalf256 c, minihalf256 d, int switches, minihalf256_uchar256 income, bool256 predicate, bool polarity=0);

char256_minifloat256 v_i8_sel2_geq_f8_vb(minifloat256 a, minifloat256 b, char256 c, char256 d, int switches, char256_minifloat256 income, bool256 predicate, bool polarity=0);
char256_minifloat256 v_i8_sel2_geq_f8_b(minifloat256 a, minifloat256 b, char256 c, char256 d, int switches=0, char256_minifloat256 income={}, bool predicate=1, bool polarity=0);
char256_minihalf256 v_i8_sel2_geq_h8_vb(char256_minihalf256 a, char256_minihalf256 b, char256 c, char256 d, int switches, char256_minifloat256 income, bool256 predicate, bool polarity=0);
char256_minifloat256 v_i8_sel2_geq_h8_b(char256_minihalf256 a, char256_minihalf256 b, char256 c, char256 d, int switches=0, char256_minifloat256 income={}, bool predicate=1, bool polarity=0);

uchar256_minifloat256 v_u8_sel2_geq_f8_vb(minifloat256 a, minifloat256 b, uchar256 c, uchar256 d, int switches, ushort128_bfloat128 income, bool256 predicate, bool polarity=0);
uchar256_minifloat256 v_u8_sel2_geq_f8_b(minifloat256 a, minifloat256 b, uchar256 c, uchar256 d, int switches=0, uchar256_minifloat256 income={}, bool predicate=1, bool polarity=0);
uchar256_minihalf256 v_u8_sel2_geq_h8_vb(minihalf256 a, minihalf256 b, uchar256 c, uchar256 d, int switches, uchar256_minihalf256 income, bool256 predicate, bool polarity=0);
uchar256_minihalf256 v_u8_sel2_geq_h8_b(minihalf256 a, minihalf256 b, uchar256 c, uchar256 d, int switches=0, uchar256_minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ SET_INDX
//

/// @section sec11 IRF
/// @subsection subsec11_2 set index
/// @brief Represents SET_INDX instruction. Assigns a scalar to IRF register lanes.
///
/// @param value       Value assigned to selected dimensions (SRC1).
/// @param ndx         This value is returned if the predicate is false.
/// @param dimmask     Mask of IRF lanes participated in the operation.
/// @param switches    Switches of the instruction.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Tensor coordinates after assignment.
///
#if defined(__gaudi2_plus__)
/// @switches Allowed switches are:
///     \li SW_IRF44_HIGH - Write SRF/IMM[11:0] to IRF[dim][43:32] \n
///                                   Relevant only if long-irf specified.
///
#endif
/// @{
int5 set_indx(int value, int5 ndx, const int dimmask, int switches, bool predicate=1, bool polarity=0);
/// @}


//
// ------ SHL
//

/// @section sec2 Arithmetic
/// @subsection subsec2_56 Shift Left
/// @brief Represents SHL instruction.
///
/// @param a           The value to shift (SRC1).
/// @param b           The number of bits to shift left (SRC2).
/// @param switches    Switches of SHL instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @{
float s_f32_shl(float a, int8_t b, int switches=0, float income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_bf16_shl(bf16 a, uint8_t b, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_f16_shl(half a, uint8_t b, int switches=0, half income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat s_f8_shl(minifloat a, uint8_t b, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_h8_shl(minihalf a, uint8_t b, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
#endif
int32_t s_i32_shl(int32_t a, int8_t b, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_u32_shl(uint32_t a, int8_t b, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_i16_shl(int16_t a, int8_t b, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_u16_shl(uint16_t a, int8_t b, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_i8_shl(int8_t a, int8_t b, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u8_shl(uint8_t a, int8_t b, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);

float64 v_f32_shl_b(float64 a, int64 b, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_shl_vb(float64 a, int64 b, int switches, float64 income, bool64 predicate, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_shl_b(bfloat128 a, short128 b, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_shl_vb(bfloat128 a, short128 b, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_f16_shl_b(half128 a, short128 b, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_shl_vb(half128 a, short128 b, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_shl_b(minifloat256 a, char256 b, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_shl_vb(minifloat256 a, char256 b, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_shl_b(minihalf256 a, char256 b, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_shl_vb(minihalf256 a, char256 b, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
int64 v_i32_shl_b(int64 a, int64 b, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_shl_vb(int64 a, int64 b, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_shl_b(uint64 a, int64 b, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_shl_vb(uint64 a, int64 b, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_shl_b(short128 a, short128 b, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_shl_vb(short128 a, short128 b, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_shl_b(ushort128 a, short128 b, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_shl_vb(ushort128 a, short128 b, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_shl_b(char256 a, char256 b, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_shl_vb(char256 a, char256 b, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_shl_b(uchar256 a, char256 b, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_shl_vb(uchar256 a, char256 b, int switches, uchar256 income, bool256 predicate, bool polarity=0);
/// @}


#if defined(__gaudi_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_57 Shift Left for IRF
/// @brief Represents SHL instruction for int5 operands.
///
/// @param a           The value to shift (SRC1).
/// @param b           The number of bits to shift left (SRC2).
/// @param dimmask     Selects IRF lanes participated in the operation.
/// @param switches    Switches of SHL instructions.
/// @param income      This value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
#endif
/// @{
#if defined(__gaudi_plus__)
int5 i_i32_shl(int5 a, int5 b, int dimmask, const int switches, int5 income, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ SHR
//


/// @section sec2 Arithmetic
/// @subsection subsec2_58 Shift Right
/// @brief Represents SHR instruction.
///
/// @param a           The value to shift (SRC1).
/// @param b           The number of bits to shift right (SRC2).
/// @param switches    Switches of SHR instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @{
float s_f32_shr(float a, int8_t b, int switches=0, float income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_bf16_shr(bf16 a, uint8_t b, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_f16_shr(half a, uint8_t b, int switches=0, half income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat s_f8_shr(minifloat a, uint8_t b, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_h8_shr(minihalf a, uint8_t b, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
#endif
int32_t s_i32_shr(int32_t a, int8_t b, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_u32_shr(uint32_t a, int8_t b, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_i16_shr(int16_t a, int8_t b, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_u16_shr(uint16_t a, int8_t b, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_i8_shr(int8_t a, int8_t b, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u8_shr(uint8_t a, int8_t b, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);

float64 v_f32_shr_b(float64 a, int64 b, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
float64 v_f32_shr_vb(float64 a, int64 b, int switches, float64 income, bool64 predicate, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_shr_b(bfloat128 a, short128 b, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_shr_vb(bfloat128 a, short128 b, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_f16_shr_b(half128 a, short128 b, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
half128 v_f16_shr_vb(half128 a, short128 b, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_shr_b(minifloat256 a, char256 b, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minifloat256 v_f8_shr_vb(minifloat256 a, char256 b, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_shr_b(minihalf256 a, char256 b, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_shr_vb(minihalf256 a, char256 b, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
int64 v_i32_shr_b(int64 a, int64 b, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
int64 v_i32_shr_vb(int64 a, int64 b, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_shr_b(uint64 a, int64 b, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_shr_vb(uint64 a, int64 b, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_shr_b(short128 a, short128 b, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_shr_vb(short128 a, short128 b, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_shr_b(ushort128 a, short128 b, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_shr_vb(ushort128 a, short128 b, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_shr_b(char256 a, char256 b, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_shr_vb(char256 a, char256 b, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_shr_b(uchar256 a, char256 b, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_shr_vb(uchar256 a, char256 b, int switches, uchar256 income, bool256 predicate, bool polarity=0);
/// @}


#if defined(__gaudi_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_59 Shift Right for IRF
/// @brief Represents SHR instruction for int5 operands.
///
/// @param a           The value to shift (SRC1).
/// @param b           The number of bits to shift right (SRC2).
/// @param dimmask     Selects IRF lanes participated in the operation.
/// @param switches    Switches of SHR instructions.
/// @param income      This value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
#endif
/// @{
#if defined(__gaudi_plus__)
int5 i_i32_shr(int5 a, int5 b, int dimmask, const int switches, int5 income, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ SHUFFLE
//

/// @section sec2 Arithmetic
/// @subsection subsec2_60 Shuffle
/// @brief Represents SHUFFLE instruction.
///
/// @param a           Value to shuffle (SRC1).
/// @param b           SHUFFLE directions (SRC2).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false or when no elements are moved to the target lanes.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Reshuffled vector.
///
/// For each element in DEST b encodes the src element in the following format \n
/// | bits        | 0-4            | 5            | 6        | 7               |
/// |-------------|----------------|--------------|----------|-----------------|
/// | description | source-element | source-group | reserved | shuffle-enabled |
///
/// Only if shuffle-enabled bit is set the output element should be written.
///
/// \c DEST[i]=a[b[i]]
///
/// @{
float64 v_f32_shuffle_b(float64 a, uchar256 b, int switches, float64 income, bool predicate=1, bool polarity=0);
float64 v_f32_shuffle_vb(float64 a, uchar256 b, int switches, float64 income, bool64 predicate, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_shuffle_b(bfloat128 a, uchar256 b, int switches, bfloat128 income, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_shuffle_vb(bfloat128 a, uchar256 b, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_f16_shuffle_b(half128 a, uchar256 b, int switches, half128 income, bool predicate=1, bool polarity=0);
half128 v_f16_shuffle_vb(half128 a, uchar256 b, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_shuffle_b(minifloat256 a, uchar256 b, int switches, minifloat256 income, bool predicate=1, bool polarity=0);
minifloat256 v_f8_shuffle_vb(minifloat256 a, uchar256 b, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_shuffle_b(minihalf256 a, uchar256 b, int switches, minihalf256 income, bool predicate=1, bool polarity=0);
minihalf256 v_h8_shuffle_vb(minihalf256 a, uchar256 b, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
int64 v_i32_shuffle_b(int64 a, uchar256 b, int switches, int64 income, bool predicate=1, bool polarity=0);
int64 v_i32_shuffle_vb(int64 a, uchar256 b, int switches, int64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_shuffle_b(uint64 a, uchar256 b, int switches, uint64 income, bool predicate=1, bool polarity=0);
uint64 v_u32_shuffle_vb(uint64 a, uchar256 b, int switches, uint64 income, bool64 predicate, bool polarity=0);
short128 v_i16_shuffle_b(short128 a, uchar256 b, int switches, short128 income, bool predicate=1, bool polarity=0);
short128 v_i16_shuffle_vb(short128 a, uchar256 b, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_shuffle_b(ushort128 a, uchar256 b, int switches, ushort128 income, bool predicate=1, bool polarity=0);
ushort128 v_u16_shuffle_vb(ushort128 a, uchar256 b, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_shuffle_b(char256 a, uchar256 b, int switches, char256 income, bool predicate=1, bool polarity=0);
char256 v_i8_shuffle_vb(char256 a, uchar256 b, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_shuffle_b(uchar256 a, uchar256 b, int switches, uchar256 income, bool predicate=1, bool polarity=0);
uchar256 v_u8_shuffle_vb(uchar256 a, uchar256 b, int switches, uchar256 income, bool256 predicate, bool polarity=0);
/// @}


//
// ------ ST_G
//

/// @section sec13 Store
/// @subsection subsec13_1 Store Global
/// @brief Represents ST_G instruction. Stores a scalar to global memory.
///
/// @param addr        Address to write to (SRC1).
/// @param value       Value to write (SRC2).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
#if defined(__greco_plus__)
/// @switches Allowed switches are:
///     \li SW_L0CS - Stored data won't be written in data-cache.
#endif
#if defined(__gaudi2_plus__)
///     \li SW_EV_HINT - Stored cache-line will be the data-cache eviction candidate in next LD_G/ST_G/PREFETCH.
///     \li SW_PD - Prefetch disable - the HW prefetcher will not perform prefetch based on this operation.
#endif
#if defined(__doron1__)
///     \li SW_EXC - Exclusive Write to an address in global memory.
#endif
/// @{
void s_f32_st_g(__global void *addr, float value, int switches=0, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
void s_bf16_st_g(__global void *addr, bf16 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
void s_f16_st_g(__global void *addr, half value, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
void s_f8_st_g(__global void *addr, minifloat value, int switches=0, bool predicate=1, bool polarity=0);
void s_h8_st_g(__global void *addr, minihalf value, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__doron1__)
void  s_i32_x2_st_g(__global void *addr, int32_t_pair_t  value, int switches=0, bool predicate=1, bool polarity=0);
void  s_u32_x2_st_g(__global void *addr, uint32_t_pair_t value, int switches=0, bool predicate=1, bool polarity=0);
#endif
void s_i32_st_g(__global void *addr, int32_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_u32_st_g(__global void *addr, uint32_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_i16_st_g(__global void *addr, int16_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_u16_st_g(__global void *addr, uint16_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_i8_st_g(__global void *addr, int8_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_u8_st_g(__global void *addr, uint8_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_i1_st_g(__global void *addr, bool value, int switches=0, bool predicate=1, bool polarity=0);
/// @}


#if defined(__gaudi2_plus__)
/// @section sec13 Store
/// @subsection subsec13_2 Store Global with SW_INC_* switch
/// @brief Represents ST_G instruction with SW_INC_* switch. Stores a scalar to global memory.
///
/// @param addr        Points to the address to write to (SRC1).
/// @param value       Value to write (SRC2).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed switches are:
///     - [AUTO_INC]
///         \li SW_INC_1 - Increment \p addr by 1B.
///         \li SW_INC_2 - Increment \p addr by 2B.
///         \li SW_INC_4 - Increment \p addr by 4B.
///         \li SW_INC_8 - Increment \p addr by 8B.
/// \par
///     \li SW_L0CS - Stored data won't be written in data-cache.
///     \li SW_EV_HINT - Stored cache-line will be the data-cache eviction candidate in next LD_G/ST_G/PREFETCH.
///     \li SW_PD - Prefetch disable - the HW prefetcher will not perform prefetch based on this operation.
///
/// @note If none of the switches SW_INC_* is specified, increment value is calculated
/// using the size of the value to store.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
void s_f32_st_g_inc(__global float **addr, float value, int switches=0, bool predicate=1, bool polarity=0);
void s_bf16_st_g_inc(__global bf16 **addr, bf16 value, int switches=0, bool predicate=1, bool polarity=0);
void s_f16_st_g_inc(__global half **addr, half value, int switches=0, bool predicate=1, bool polarity=0);
void s_f8_st_g_inc(__global minifloat **addr, minifloat value, int switches=0, bool predicate=1, bool polarity=0);
void s_h8_st_g_inc(__global minihalf **addr, minihalf value, int switches=0, bool predicate=1, bool polarity=0);
void s_i32_st_g_inc(__global int32_t **addr, int32_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_u32_st_g_inc(__global uint32_t **addr, uint32_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_i16_st_g_inc(__global int16_t **addr, int16_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_u16_st_g_inc(__global uint16_t **addr, uint16_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_i8_st_g_inc(__global int8_t **addr, int8_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_u8_st_g_inc(__global uint8_t **addr, uint8_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_i1_st_g_inc(__global bool **addr, bool value, int switches=0, bool predicate=1, bool polarity=0);
#endif

#if defined(__doron1__)
void s_i32_x2_st_g_inc(__global int32_t  **addr,  int32_t_pair_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_u32_x2_st_g_inc(__global uint32_t **addr, uint32_t_pair_t value, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ ST_L
//

/// @section sec13 Store
/// @subsection subsec13_3 Store Local
/// @brief Represents ST_L instruction. Stores a scalar to scalar local memory.
///
/// @param addr        Address to write to (SRC1).
/// @param value       Value to write (SRC2).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed switches are:
///     \li SW_MMIO - store to MMIO.
///     \li SW_SLM  - store to SLM (the default).
#if defined(__greco_plus__)
///     \li SW_UNLOCK - Unlock the address - relevant when SW_MMIO is set, and only for lockable registers.
#endif
#if defined(__doron1__)
///     \li SW_ABS_ADDR
#endif
///
/// @{
void s_f32_st_l(uint32_t addr, float value, int switches=0, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
void s_bf16_st_l(uint32_t addr, bf16 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
void s_f16_st_l(uint32_t addr, half value, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
void s_f8_st_l(uint32_t addr, minifloat value, int switches=0, bool predicate=1, bool polarity=0);
void s_h8_st_l(uint32_t addr, minihalf value, int switches=0, bool predicate=1, bool polarity=0);
#endif
void s_i32_st_l(uint32_t addr, int32_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_u32_st_l(uint32_t addr, uint32_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_i16_st_l(uint32_t addr, int16_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_u16_st_l(uint32_t addr, uint16_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_i8_st_l(uint32_t addr, int8_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_u8_st_l(uint32_t addr, uint8_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_i1_st_l(uint32_t addr, bool value, int switches=0, bool predicate=1, bool polarity=0);
/// @}


//
// ------ ST_L_V
//

/// @section sec13 Store
/// @subsection subsec13_4 Store Local vector
/// @brief Represents ST_L_V instruction. Stores a vector to vector local memory.
///
/// @param addr        Address to write to (SRC1).
/// @param value       Value to write (SRC2).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
#if defined(__gaudi2_plus__)
/// @switches Allowed switches are:
///     - [AUTO_INC_V]
///         \li SW_INC_1 - Increment \p addr by 1x256B.
///         \li SW_INC_2 - Increment \p addr by 2x256B.
///         \li SW_INC_4 - Increment \p addr by 4x256B.
#endif
#if defined(__doron1__)
///     \li SW_ABS_ADDR
///     \li SW_ADDR_CALC
#endif
///
/// @{
void v_f32_st_l_v(uint32_t addr, float64 value, int switches=0, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
void v_bf16_st_l_v(uint32_t addr, bfloat128 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
void v_f16_st_l_v(uint32_t addr, half128 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
void v_i32_st_l_v(uint32_t addr, int64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u32_st_l_v(uint32_t addr, uint64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i16_st_l_v(uint32_t addr, short128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u16_st_l_v(uint32_t addr, ushort128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i8_st_l_v(uint32_t addr, char256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u8_st_l_v(uint32_t addr, uchar256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i1_st_l_v(uint32_t addr, bool256 value, int switches=0, bool predicate=1, bool polarity=0);
#if defined(__greco_plus__)
void v_f8_st_l_v_b(uint32_t addr, minifloat256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_h8_st_l_v_b(uint32_t addr, minihalf256 value, int switches=0, bool predicate=1, bool polarity=0);

void v_f32_st_l_v_vb(uint32_t addr, float64 value, int switches, bool64 predicate, bool polarity=0);
void v_bf16_st_l_v_vb(uint32_t addr, bfloat128 value, int switches, bool128 predicate, bool polarity=0);
void v_f16_st_l_v_vb(uint32_t addr, half128 value, int switches, bool128 predicate, bool polarity=0);
void v_i32_st_l_v_vb(uint32_t addr, int64 value, int switches, bool64 predicate, bool polarity=0);
void v_u32_st_l_v_vb(uint32_t addr, uint64 value, int switches, bool64 predicate, bool polarity=0);
void v_i16_st_l_v_vb(uint32_t addr, short128 value, int switches, bool128 predicate, bool polarity=0);
void v_u16_st_l_v_vb(uint32_t addr, ushort128 value, int switches, bool128 predicate, bool polarity=0);
void v_i8_st_l_v_vb(uint32_t addr, char256 value, int switches, bool256 predicate, bool polarity=0);
void v_u8_st_l_v_vb(uint32_t addr, uchar256 value, int switches, bool256 predicate, bool polarity=0);
void v_i1_st_l_v_vb(uint32_t addr, bool256 value, int switches, bool256 predicate, bool polarity=0);
void v_f8_st_l_v_vb(uint32_t addr, minifloat256 value, int switches, bool256 predicate, bool polarity=0);
void v_h8_st_l_v_vb(uint32_t addr, minihalf256 value, int switches, bool256 predicate, bool polarity=0);
#endif
/// @}


//
// ------ ST_L_V_LOW
//

/// @section sec13 Store
/// @subsection subsec13_5 ST_L_V_LOW
/// @brief Represents ST_L_V_LOW instruction. Stores half a vector to vector local memory.
///
/// @param addr        Address to write to (SRC1).
/// @param value       Value to write (SRC2).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
#if defined(__gaudi2_plus__)
/// @switches Allowed switches are:
///     - [AUTO_INC_V]
///         \li SW_INC_1 - Increment \p addr by 1x256B.
///         \li SW_INC_2 - Increment \p addr by 2x256B.
///         \li SW_INC_4 - Increment \p addr by 4x256B.
#endif
///
/// @{
void v_f32_st_l_v_low(uint32_t addr, float64 value, int switches=0, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
void v_bf16_st_l_v_low(uint32_t addr, bfloat128 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
void v_f16_st_l_v_low(uint32_t addr, half128 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
void v_i32_st_l_v_low(uint32_t addr, int64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u32_st_l_v_low(uint32_t addr, uint64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i16_st_l_v_low(uint32_t addr, short128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u16_st_l_v_low(uint32_t addr, ushort128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i8_st_l_v_low(uint32_t addr, char256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u8_st_l_v_low(uint32_t addr, uchar256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i1_st_l_v_low(uint32_t addr, bool256 value, int switches=0, bool predicate=1, bool polarity=0);
#if defined(__greco_plus__)
void v_f8_st_l_v_low_b(uint32_t addr, minifloat256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_h8_st_l_v_low_b(uint32_t addr, minihalf256 value, int switches=0, bool predicate=1, bool polarity=0);

void v_f32_st_l_v_low_vb(uint32_t addr, float64 value, int switches, bool64 predicate, bool polarity=0);
void v_bf16_st_l_v_low_vb(uint32_t addr, bfloat128 value, int switches, bool128 predicate, bool polarity=0);
void v_f16_st_l_v_low_vb(uint32_t addr, half128 value, int switches, bool128 predicate, bool polarity=0);
void v_i32_st_l_v_low_vb(uint32_t addr, int64 value, int switches, bool64 predicate, bool polarity=0);
void v_u32_st_l_v_low_vb(uint32_t addr, uint64 value, int switches, bool64 predicate, bool polarity=0);
void v_i16_st_l_v_low_vb(uint32_t addr, short128 value, int switches, bool128 predicate, bool polarity=0);
void v_u16_st_l_v_low_vb(uint32_t addr, ushort128 value, int switches, bool128 predicate, bool polarity=0);
void v_i8_st_l_v_low_vb(uint32_t addr, char256 value, int switches, bool256 predicate, bool polarity=0);
void v_u8_st_l_v_low_vb(uint32_t addr, uchar256 value, int switches, bool256 predicate, bool polarity=0);
void v_i1_st_l_v_low_vb(uint32_t addr, bool256 value, int switches, bool256 predicate, bool polarity=0);
void v_f8_st_l_v_low_vb(uint32_t addr, minifloat256 value, int switches, bool256 predicate, bool polarity=0);
void v_h8_st_l_v_low_vb(uint32_t addr, minihalf256 value, int switches, bool256 predicate, bool polarity=0);
#endif
/// @}


//
// ------ ST_L_V_HIGH
//

/// @section sec13 Store
/// @subsection subsec13_6 ST_L_V_HIGH
/// @brief Represents ST_L_V_HIGH instruction. Stores half a vector to vector local memory.
///
/// @param addr        Address to write to (SRC1).
/// @param value       Value to write (SRC2).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
#if defined(__gaudi2_plus__)
/// @switches Allowed switches are:
///     - [AUTO_INC_V]
///         \li SW_INC_1 - Increment \p addr by 1x256B.
///         \li SW_INC_2 - Increment \p addr by 2x256B.
///         \li SW_INC_4 - Increment \p addr by 4x256B.
#endif
///
/// @{
void v_f32_st_l_v_high(uint32_t addr, float64 value, int switches=0, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
void v_bf16_st_l_v_high(uint32_t addr, bfloat128 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
void v_f16_st_l_v_high(uint32_t addr, half128 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
void v_i32_st_l_v_high(uint32_t addr, int64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u32_st_l_v_high(uint32_t addr, uint64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i16_st_l_v_high(uint32_t addr, short128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u16_st_l_v_high(uint32_t addr, ushort128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i8_st_l_v_high(uint32_t addr, char256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u8_st_l_v_high(uint32_t addr, uchar256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i1_st_l_v_high(uint32_t addr, bool256 value, int switches=0, bool predicate=1, bool polarity=0);
#if defined(__greco_plus__)
void v_f8_st_l_v_high_b(uint32_t addr, minifloat256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_h8_st_l_v_high_b(uint32_t addr, minihalf256 value, int switches=0, bool predicate=1, bool polarity=0);

void v_f32_st_l_v_high_vb(uint32_t addr, float64 value, int switches, bool64 predicate, bool polarity=0);
void v_bf16_st_l_v_high_vb(uint32_t addr, bfloat128 value, int switches, bool128 predicate, bool polarity=0);
void v_f16_st_l_v_high_vb(uint32_t addr, half128 value, int switches, bool128 predicate, bool polarity=0);
void v_i32_st_l_v_high_vb(uint32_t addr, int64 value, int switches, bool64 predicate, bool polarity=0);
void v_u32_st_l_v_high_vb(uint32_t addr, uint64 value, int switches, bool64 predicate, bool polarity=0);
void v_i16_st_l_v_high_vb(uint32_t addr, short128 value, int switches, bool128 predicate, bool polarity=0);
void v_u16_st_l_v_high_vb(uint32_t addr, ushort128 value, int switches, bool128 predicate, bool polarity=0);
void v_i8_st_l_v_high_vb(uint32_t addr, char256 value, int switches, bool256 predicate, bool polarity=0);
void v_u8_st_l_v_high_vb(uint32_t addr, uchar256 value, int switches, bool256 predicate, bool polarity=0);
void v_i1_st_l_v_high_vb(uint32_t addr, bool256 value, int switches, bool256 predicate, bool polarity=0);
void v_f8_st_l_v_high_vb(uint32_t addr, minifloat256 value, int switches, bool256 predicate, bool polarity=0);
void v_h8_st_l_v_high_vb(uint32_t addr, minihalf256 value, int switches, bool256 predicate, bool polarity=0);
#endif
/// @}

//
// ------ ST_TNSR_S
//

#if defined(__gaudi2_plus__)
/// @section sec13 Store
/// @subsection subsec13_7 ST_TNSR_S
/// @brief Represents ST_TNSR_S instruction. Stores to global memory a scalar or internal HW register value using the vector pipe.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param value       Value to store to a tensor (SRC2).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
void s_f32_st_tnsr_s(int5 ndx, int8_t tensor, float value, int switches=0, bool predicate=1, bool polarity=0);
void s_bf16_st_tnsr_s(int5 ndx, int8_t tensor, bf16 value, int switches=0, bool predicate=1, bool polarity=0);
void s_f16_st_tnsr_s(int5 ndx, int8_t tensor, half value, int switches=0, bool predicate=1, bool polarity=0);
void s_f8_st_tnsr_s(int5 ndx, int8_t tensor, minifloat value, int switches=0, bool predicate=1, bool polarity=0);
void s_h8_st_tnsr_s(int5 ndx, int8_t tensor, minihalf value, int switches=0, bool predicate=1, bool polarity=0);
void s_i32_st_tnsr_s(int5 ndx, int8_t tensor, int32_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_u32_st_tnsr_s(int5 ndx, int8_t tensor, uint32_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_i16_st_tnsr_s(int5 ndx, int8_t tensor, int16_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_u16_st_tnsr_s(int5 ndx, int8_t tensor, uint16_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_i8_st_tnsr_s(int5 ndx, int8_t tensor, int8_t value, int switches=0, bool predicate=1, bool polarity=0);
void s_u8_st_tnsr_s(int5 ndx, int8_t tensor, uint8_t value, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}

#if defined(__gaudi2_plus__)
/// @section sec13 Store
/// @subsection subsec13_8 ST_TNSR_S with RMW_SEL switch
/// @brief Represents ST_TNSR_S instruction with RMW_SEL switch. Stores to global memory a scalar or internal HW register value using the vector pipe.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param value       Value to store to a tensor (SRC2).
/// @param rmw         Information for RMW operation (switches).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed RMW switches are:
///     - [RMW_DT]
///         \li RMW_DT_INT8
///         \li RMW_DT_INT16
///         \li RMW_DT_INT32
///         \li RMW_DT_UINT8
///         \li RMW_DT_UINT16
///         \li RMW_DT_UINT32
///         \li RMW_DT_BF16
///         \li RMW_DT_FP32
///         \li RMW_DT_FP16
///         \li RMW_DT_FP8_152
///     - [RMW_OP]
///         \li RMW_OP_ADD
///         \li RMW_OP_SUB
///         \li RMW_OP_MIN
///         \li RMW_OP_MAX
///         \li RMW_OP_MAX_0_ADD
/// \par
///     \li RMW_TNSR_DT - When set RMW data type is taken from tensor descriptor, otherwise taken from switch value.
///     \li RMW_SET - Enable RMW.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
void s_f32_st_tnsr_s_rmw(int5 ndx, int8_t tensor, float value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void s_bf16_st_tnsr_s_rmw(int5 ndx, int8_t tensor, bf16 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void s_f16_st_tnsr_s_rmw(int5 ndx, int8_t tensor, half value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void s_h8_st_tnsr_s_rmw(int5 ndx, int8_t tensor, minihalf value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void s_i32_st_tnsr_s_rmw(int5 ndx, int8_t tensor, int32_t value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void s_u32_st_tnsr_s_rmw(int5 ndx, int8_t tensor, uint32_t value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void s_i16_st_tnsr_s_rmw(int5 ndx, int8_t tensor, int16_t value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void s_u16_st_tnsr_s_rmw(int5 ndx, int8_t tensor, uint16_t value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void s_i8_st_tnsr_s_rmw(int5 ndx, int8_t tensor, int8_t value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void s_u8_st_tnsr_s_rmw(int5 ndx, int8_t tensor, uint8_t value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__gaudi2_plus__)
/// @section sec13 Store
/// @subsection subsec13_9 ST_TNSR_S with HW_REG switch and with/without RMW_SEL switch
/// @brief Represents ST_TNSR_S instruction with HW_REG switch and with/without RMW_SEL switch.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param sqz         Squeeze counter.
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed switches are:
///     \li SW_ST_TNSR_S_BV64 - Store 64bit (default is lower 32bit).
///
/// @switches Allowed RMW switches are:
///     - [RMW_DT]
///         \li RMW_DT_INT8
///         \li RMW_DT_INT16
///         \li RMW_DT_INT32
///         \li RMW_DT_UINT8
///         \li RMW_DT_UINT16
///         \li RMW_DT_UINT32
///         \li RMW_DT_BF16
///         \li RMW_DT_FP32
///         \li RMW_DT_FP16
///         \li RMW_DT_FP8_152
///     - [RMW_OP]
///         \li RMW_OP_ADD
///         \li RMW_OP_SUB
///         \li RMW_OP_MIN
///         \li RMW_OP_MAX
///         \li RMW_OP_MAX_0_ADD
/// \par
///     \li RMW_TNSR_DT - When set RMW data type is taken from tensor descriptor, otherwise taken from switch value.
///     \li RMW_SET - Enable RMW.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
void st_tnsr_s_hwr(int5 ndx, int8_t tensor, squeeze_cntr sqz, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__gaudi2_plus__)
/// @section sec13 Store
/// @subsection subsec13_10 ST_TNSR_S with HW_REG switch and without RMW_SEL switch
/// @brief Represents ST_TNSR_S instruction with HW_REG switch and without RMW_SEL switch.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param sqz         Squeeze counter.
/// @param rmw         RMW value.
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed switches are:
///     \li SW_ST_TNSR_S_BV64 - Store 64bit (default is lower 32bit).
///
/// @switches Allowed RMW switches are:
///     - [RMW_DT]
///         \li RMW_DT_INT8
///         \li RMW_DT_INT16
///         \li RMW_DT_INT32
///         \li RMW_DT_UINT8
///         \li RMW_DT_UINT16
///         \li RMW_DT_UINT32
///         \li RMW_DT_BF16
///         \li RMW_DT_FP32
///         \li RMW_DT_FP16
///         \li RMW_DT_FP8_152
///     - [RMW_OP]
///         \li RMW_OP_ADD
///         \li RMW_OP_SUB
///         \li RMW_OP_MIN
///         \li RMW_OP_MAX
///         \li RMW_OP_MAX_0_ADD
/// \par
///     \li RMW_TNSR_DT - When set RMW data type is taken from tensor descriptor, otherwise taken from switch value.
///     \li RMW_SET - Enable RMW.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
void st_tnsr_s_hwr_rmw(int5 ndx, int8_t tensor, squeeze_cntr sqz, int rmw, int switches = 0, bool predicate = 1, bool polarity = 0);
#endif
/// @}


//
// ------ ST_TNSR
//

/// @section sec13 Store
/// @subsection subsec13_11 ST_TNSR
/// @brief Represents ST_TNSR instruction.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param value       Value to write (SRC3).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
#if defined(__gaudi_plus__)
/// @switches Allowed switches are:
///     \li SW_PACK - When set, store pipe will perform packing according to PACK_DT.
#endif
#if defined(__greco_plus__)
/// \par
///     - [PACK_DT]
///         \li SW_PCK_32_TO_16
///         \li SW_PCK_16_TO_8
///         \li SW_PCK_32_TO_8
///         \li SW_PCK_8_TO_4
#endif
///
/// @{
void v_f32_st_tnsr(int5 ndx, const int8_t tensor, float64 value, int switches=0, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
void v_bf16_st_tnsr(int5 ndx, const int8_t tensor, bfloat128 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
void v_f16_st_tnsr(int5 ndx, const int8_t tensor, half128 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
void v_f8_st_tnsr(int5 ndx, int8_t tensor, minifloat256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_h8_st_tnsr(int5 ndx, int8_t tensor, minihalf256 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
void v_i32_st_tnsr(int5 ndx, const int8_t tensor, int64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u32_st_tnsr(int5 ndx, const int8_t tensor, uint64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i16_st_tnsr(int5 ndx, const int8_t tensor, short128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u16_st_tnsr(int5 ndx, const int8_t tensor, ushort128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i8_st_tnsr(int5 ndx, const int8_t tensor, char256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u8_st_tnsr(int5 ndx, const int8_t tensor, uchar256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i1_st_tnsr(int5 ndx, const int8_t tensor, bool256 value, int switches=0, bool predicate=1, bool polarity=0);
/// @}


#if defined(__gaudi2_plus__)
/// @section sec13 Store
/// @subsection subsec13_12 ST_TNSR with DIRECT switch
/// @brief Represents ST_TNSR instruction with DIRECT switch.
///
/// @param addr1       Tensor store address low (SRC1).
/// @param addr2       Tensor store address high (SRC2).
/// @param value       Value to write (SRC3).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed switches are:
///     \li SW_PACK- When set, store pipe will perform packing according to PACK_DT.
/// \par
///     - [PACK_DT]
///         \li SW_PCK_32_TO_16
///         \li SW_PCK_16_TO_8
///         \li SW_PCK_32_TO_8
///         \li SW_PCK_8_TO_4
///
#endif
/// @{
#if defined(__gaudi2_plus__)
void v_f32_st_tnsr_direct(uint32_t addr1, uint32_t addr2, float64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_bf16_st_tnsr_direct(uint32_t addr1, uint32_t addr2, bfloat128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_f16_st_tnsr_direct(uint32_t addr1, uint32_t addr2, half128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_f8_st_tnsr_direct(uint32_t addr1, uint32_t addr2, minifloat256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_h8_st_tnsr_direct(uint32_t addr1, uint32_t addr2, minihalf256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i32_st_tnsr_direct(uint32_t addr1, uint32_t addr2, int64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u32_st_tnsr_direct(uint32_t addr1, uint32_t addr2, uint64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i16_st_tnsr_direct(uint32_t addr1, uint32_t addr2, short128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u16_st_tnsr_direct(uint32_t addr1, uint32_t addr2, ushort128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i8_st_tnsr_direct(uint32_t addr1, uint32_t addr2, char256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u8_st_tnsr_direct(uint32_t addr1, uint32_t addr2, uchar256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i1_st_tnsr_direct(uint32_t addr1, uint32_t addr2, bool256 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__gaudi_plus__)
/// @section sec13 Store
/// @subsection subsec13_13 ST_TNSR with RMW_SEL switch
/// @brief Represents ST_TNSR instruction with RMW_SEL switch.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param value       Value to write (SRC3).
/// @param rmw         Information for RMW operation (switches).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed switches are:
///     \li SW_PACK - When set, store pipe will perform packing according to PACK_DT.
#endif
#if defined(__greco_plus__)
/// \par
///     - [PACK_DT]
///         \li SW_PCK_32_TO_16
///         \li SW_PCK_16_TO_8
///         \li SW_PCK_32_TO_8
///         \li SW_PCK_8_TO_4
///
#endif
#if defined(__gaudi_plus__)
/// @switches Allowed RMW switches are:
///     - [RMW_DT]
///         \li RMW_DT_INT8
///         \li RMW_DT_INT16
///         \li RMW_DT_INT32
///         \li RMW_DT_UINT8
///         \li RMW_DT_UINT16
///         \li RMW_DT_UINT32
///         \li RMW_DT_BF16
///         \li RMW_DT_FP32
#endif
#if defined(__greco_plus__)
///         \li RMW_DT_FP16
#endif
#if defined(__gaudi2_plus__)
///         \li RMW_DT_FP8_152
#endif
#if defined(__gaudi_plus__)
/// \par
///     - [RMW_OP]
///         \li RMW_OP_ADD
///         \li RMW_OP_SUB
///         \li RMW_OP_MIN
///         \li RMW_OP_MAX
#endif
#if defined(__gaudi2_plus__)
///         \li RMW_OP_MAX_0_ADD
#endif
#if defined(__gaudi_plus__)
/// \par
///     \li RMW_TNSR_DT - When set RMW data type is taken from tensor descriptor, otherwise taken from switch value.
///     \li RMW_SET - Enable RMW.
///
#endif
/// @{
#if defined(__gaudi_plus__)
void v_f32_st_tnsr_rmw(int5 ndx, int8_t tensor, float64 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_bf16_st_tnsr_rmw(int5 ndx, int8_t tensor, bfloat128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_i32_st_tnsr_rmw(int5 ndx, int8_t tensor, int64 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_u32_st_tnsr_rmw(int5 ndx, int8_t tensor, uint64 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_i16_st_tnsr_rmw(int5 ndx, int8_t tensor, short128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_u16_st_tnsr_rmw(int5 ndx, int8_t tensor, ushort128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_i8_st_tnsr_rmw(int5 ndx, int8_t tensor, char256 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_u8_st_tnsr_rmw(int5 ndx, int8_t tensor, uchar256 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
void v_f16_st_tnsr_rmw(int5 ndx, int8_t tensor, half128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
void v_h8_st_tnsr_rmw(int5 ndx, int8_t tensor, minihalf256 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__gaudi2_plus__)
/// @section sec13 Store
/// @subsection subsec13_14 ST_TNSR with RMW_SEL and DIRECT switches
/// @brief Represents ST_TNSR instruction with RMW_SEL and DIRECT switches.
///
/// @param addr1       Tensor store address low (SRC1).
/// @param addr2       Tensor store address high (SRC2).
/// @param value       Value to write (SRC3).
/// @param rmw         Information for RMW operation (switches).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed switches are:
///     \li SW_PACK - When set, store pipe will perform packing according to PACK_DT.
/// \par
///     - [PACK_DT]
///         \li SW_PCK_32_TO_16
///         \li SW_PCK_16_TO_8
///         \li SW_PCK_32_TO_8
///         \li SW_PCK_8_TO_4
///
/// @switches Allowed RMW switches are:
///     - [RMW_DT]
///         \li RMW_DT_INT8
///         \li RMW_DT_INT16
///         \li RMW_DT_INT32
///         \li RMW_DT_UINT8
///         \li RMW_DT_UINT16
///         \li RMW_DT_UINT32
///         \li RMW_DT_BF16
///         \li RMW_DT_FP32
///         \li RMW_DT_FP16
///         \li RMW_DT_FP8_152
///     - [RMW_OP]
///         \li RMW_OP_ADD
///         \li RMW_OP_SUB
///         \li RMW_OP_MIN
///         \li RMW_OP_MAX
///         \li RMW_OP_MAX_0_ADD
/// \par
///     \li RMW_TNSR_DT - When set RMW data type is taken from tensor descriptor, otherwise taken from switch value.
///     \li RMW_SET - Enable RMW.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
void v_f32_st_tnsr_rmw_direct(uint32_t addr1, uint32_t addr2, float64 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_bf16_st_tnsr_rmw_direct(uint32_t addr1, uint32_t addr2, bfloat128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_f16_st_tnsr_rmw_direct(uint32_t addr1, uint32_t addr2, half128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_h8_st_tnsr_rmw_direct(uint32_t addr1, uint32_t addr2, minihalf256 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_i32_st_tnsr_rmw_direct(uint32_t addr1, uint32_t addr2, int64 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_u32_st_tnsr_rmw_direct(uint32_t addr1, uint32_t addr2, uint64 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_i16_st_tnsr_rmw_direct(uint32_t addr1, uint32_t addr2, short128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_u16_st_tnsr_rmw_direct(uint32_t addr1, uint32_t addr2, ushort128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_i8_st_tnsr_rmw_direct(uint32_t addr1, uint32_t addr2, char256 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_u8_st_tnsr_rmw_direct(uint32_t addr1, uint32_t addr2, uchar256 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__gaudi_plus__)
/// @section sec13 Store
/// @subsection subsec13_15 ST_TNSR with PARTIAL switch
/// @brief Represents ST_TNSR instruction with PARTIAL switch.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param value       Value to write (SRC3).
/// @param size        Size in elements minus 1.
/// @param offset      Offset in elements.
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed switches are:
///     \li SW_PACK - When set, store pipe will perform packing according to PACK_DT.
#endif
///
#if defined(__greco_plus__)
/// \par
///     - [PACK_DT]
///         \li SW_PCK_32_TO_16
///         \li SW_PCK_16_TO_8
///         \li SW_PCK_32_TO_8
///         \li SW_PCK_8_TO_4
///
#endif
/// @{
#if defined(__gaudi_plus__)
void v_f32_st_tnsr_partial(int5 ndx, int8_t tensor, float64 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_bf16_st_tnsr_partial(int5 ndx, int8_t tensor, bfloat128 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_i32_st_tnsr_partial(int5 ndx, int8_t tensor, int64 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_u32_st_tnsr_partial(int5 ndx, int8_t tensor, uint64 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_i16_st_tnsr_partial(int5 ndx, int8_t tensor, short128 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_u16_st_tnsr_partial(int5 ndx, int8_t tensor, ushort128 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_i8_st_tnsr_partial(int5 ndx, int8_t tensor, char256 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_u8_st_tnsr_partial(int5 ndx, int8_t tensor, uchar256 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_i1_st_tnsr_partial(int5 ndx, int8_t tensor, bool256 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
void v_f16_st_tnsr_partial(int5 ndx, int8_t tensor, half128 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
void v_f8_st_tnsr_partial(int5 ndx, int8_t tensor, minifloat256 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_h8_st_tnsr_partial(int5 ndx, int8_t tensor, minihalf256 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__gaudi2_plus__)
/// @section sec13 Store
/// @subsection subsec13_16 ST_TNSR with PARTIAL and DIRECT switches
/// @brief Represents ST_TNSR instruction with PARTIAL and DIRECT switches.
///
/// @param addr1       Tensor store address low (SRC1).
/// @param addr2       Tensor store address high (SRC2).
/// @param value       Value to write (SRC3).
/// @param size        Size in elements minus 1.
/// @param offset      Offset in elements.
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed switches are:
///     \li SW_PACK - When set, store pipe will perform packing according to PACK_DT.
/// \par
///     - [PACK_DT]
///         \li SW_PCK_32_TO_16
///         \li SW_PCK_16_TO_8
///         \li SW_PCK_32_TO_8
///         \li SW_PCK_8_TO_4
///
///
#endif
/// @{
#if defined(__gaudi2_plus__)
void v_f32_st_tnsr_partial_direct(uint32_t addr1, uint32_t addr2, float64 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_bf16_st_tnsr_partial_direct(uint32_t addr1, uint32_t addr2, bfloat128 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_f16_st_tnsr_partial_direct(uint32_t addr1, uint32_t addr2, half128 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_f8_st_tnsr_partial_direct(uint32_t addr1, uint32_t addr2, minifloat256 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_h8_st_tnsr_partial_direct(uint32_t addr1, uint32_t addr2, minihalf256 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_i32_st_tnsr_partial_direct(uint32_t addr1, uint32_t addr2, int64 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_u32_st_tnsr_partial_direct(uint32_t addr1, uint32_t addr2, uint64 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_i16_st_tnsr_partial_direct(uint32_t addr1, uint32_t addr2, short128 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_u16_st_tnsr_partial_direct(uint32_t addr1, uint32_t addr2, ushort128 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_i8_st_tnsr_partial_direct(uint32_t addr1, uint32_t addr2, char256 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_u8_st_tnsr_partial_direct(uint32_t addr1, uint32_t addr2, uchar256 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_i1_st_tnsr_partial_direct(uint32_t addr1, uint32_t addr2, bool256 value, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__gaudi_plus__)
/// @section sec13 Store
/// @subsection subsec13_17 ST_TNSR with PARTIAL and RMW_SEL switches
/// @brief Represents ST_TNSR instruction with PARTIAL and RMW_SEL switches.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param value       Value to write (SRC3).
/// @param rmw         Information for RMW operation (switches).
/// @param size        Size in elements minus 1.
/// @param offset      Offset in elements.
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed switches are:
///     \li SW_PACK - When set, store pipe will perform packing according to PACK_DT.
#endif
#if defined(__greco_plus__)
/// \par
///     - [PACK_DT]
///         \li SW_PCK_32_TO_16
///         \li SW_PCK_16_TO_8
///         \li SW_PCK_32_TO_8
///         \li SW_PCK_8_TO_4
///
#endif
#if defined(__gaudi_plus__)
/// @switches Allowed RMW switches are:
///     - [RMW_DT]
///         \li RMW_DT_INT8
///         \li RMW_DT_INT16
///         \li RMW_DT_INT32
///         \li RMW_DT_UINT8
///         \li RMW_DT_UINT16
///         \li RMW_DT_UINT32
///         \li RMW_DT_BF16
///         \li RMW_DT_FP32
#endif
#if defined(__greco_plus__)
///         \li RMW_DT_FP16
#endif
#if defined(__gaudi2_plus__)
///         \li RMW_DT_FP8_152
#endif
#if defined(__gaudi_plus__)
/// \par
///     - [RMW_OP]
///         \li RMW_OP_ADD
///         \li RMW_OP_SUB
///         \li RMW_OP_MIN
///         \li RMW_OP_MAX
#endif
#if defined(__gaudi2_plus__)
///         \li RMW_OP_MAX_0_ADD
#endif
#if defined(__gaudi_plus__)
/// \par
///     \li RMW_TNSR_DT - When set RMW data type is taken from tensor descriptor, otherwise taken from switch value.
///     \li RMW_SET - Enable RMW.
///
#endif
/// @{
#if defined(__gaudi_plus__)
void v_f32_st_tnsr_partial_rmw(int5 ndx, int8_t tensor, float64 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_bf16_st_tnsr_partial_rmw(int5 ndx, int8_t tensor, bfloat128 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_i32_st_tnsr_partial_rmw(int5 ndx, int8_t tensor, int64 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_u32_st_tnsr_partial_rmw(int5 ndx, int8_t tensor, uint64 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_i16_st_tnsr_partial_rmw(int5 ndx, int8_t tensor, short128 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_u16_st_tnsr_partial_rmw(int5 ndx, int8_t tensor, ushort128 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_i8_st_tnsr_partial_rmw(int5 ndx, int8_t tensor, char256 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_u8_st_tnsr_partial_rmw(int5 ndx, int8_t tensor, uchar256 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
void v_f16_st_tnsr_partial_rmw(int5 ndx, int8_t tensor, half128 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
void v_h8_st_tnsr_partial_rmw(int5 ndx, int8_t tensor, minihalf256 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__gaudi2_plus__)
/// @section sec13 Store
/// @subsection subsec13_18 ST_TNSR with PARTIAL, RMW_SEL and DIRECT switches
/// @brief Represents ST_TNSR instruction with PARTIAL, RMW_SEL and DIRECT switches.
///
/// @param addr1       Tensor store address low (SRC1).
/// @param addr2       Tensor store address high (SRC2).
/// @param value       Value to write (SRC3).
/// @param rmw         Information for RMW operation (switches).
/// @param size        Size in elements minus 1.
/// @param offset      Offset in elements.
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed switches are:
///     \li SW_PACK - When set, store pipe will perform packing according to PACK_DT.
/// \par
///     - [PACK_DT]
///         \li SW_PCK_32_TO_16
///         \li SW_PCK_16_TO_8
///         \li SW_PCK_32_TO_8
///         \li SW_PCK_8_TO_4
///
/// @switches Allowed RMW switches are:
///     - [RMW_DT]
///         \li RMW_DT_INT8
///         \li RMW_DT_INT16
///         \li RMW_DT_INT32
///         \li RMW_DT_UINT8
///         \li RMW_DT_UINT16
///         \li RMW_DT_UINT32
///         \li RMW_DT_BF16
///         \li RMW_DT_FP32
///         \li RMW_DT_FP16
///         \li RMW_DT_FP8_152
///     - [RMW_OP]
///         \li RMW_OP_ADD
///         \li RMW_OP_SUB
///         \li RMW_OP_MIN
///         \li RMW_OP_MAX
///         \li RMW_OP_MAX_0_ADD
/// \par
///     \li RMW_TNSR_DT - When set RMW data type is taken from tensor descriptor, otherwise taken from switch value.
///     \li RMW_SET - Enable RMW.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
void v_f32_st_tnsr_partial_rmw_direct(uint32_t addr1, uint32_t addr2, float64 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_bf16_st_tnsr_partial_rmw_direct(uint32_t addr1, uint32_t addr2, bfloat128 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_f16_st_tnsr_partial_rmw_direct(uint32_t addr1, uint32_t addr2, half128 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_h8_st_tnsr_partial_rmw_direct(uint32_t addr1, uint32_t addr2, minihalf256 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_i32_st_tnsr_partial_rmw_direct(uint32_t addr1, uint32_t addr2, int64 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_u32_st_tnsr_partial_rmw_direct(uint32_t addr1, uint32_t addr2, uint64 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_i16_st_tnsr_partial_rmw_direct(uint32_t addr1, uint32_t addr2, short128 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_u16_st_tnsr_partial_rmw_direct(uint32_t addr1, uint32_t addr2, ushort128 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_i8_st_tnsr_partial_rmw_direct(uint32_t addr1, uint32_t addr2, char256 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
void v_u8_st_tnsr_partial_rmw_direct(uint32_t addr1, uint32_t addr2, uchar256 value, int rmw, int8_t size, int8_t offset, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ ST_TNSR_HIGH
//

/// @section sec13 Store
/// @subsection subsec13_19 ST_TNSR_HIGH
/// @brief Represents ST_TNSR_HIGH instruction.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param value       Value to write (SRC3).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
#if defined(__gaudi_plus__)
/// @switches Allowed switches are:
///     \li SW_PACK - When set, store pipe will perform packing according to PACK_DT.
#endif
#if defined(__greco_plus__)
/// \par
///     - [PACK_DT]
///         \li SW_PCK_32_TO_16
///         \li SW_PCK_16_TO_8
///         \li SW_PCK_32_TO_8
///         \li SW_PCK_8_TO_4
#endif
///
/// @{
void v_f32_st_tnsr_high(int5 ndx, const int8_t tensor, float64 value, int switches=0, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
void v_bf16_st_tnsr_high(int5 ndx, const int8_t tensor, bfloat128 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
void v_f16_st_tnsr_high(int5 ndx, const int8_t tensor, half128 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
void v_i32_st_tnsr_high(int5 ndx, const int8_t tensor, int64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u32_st_tnsr_high(int5 ndx, const int8_t tensor, uint64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i16_st_tnsr_high(int5 ndx, const int8_t tensor, short128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u16_st_tnsr_high(int5 ndx, const int8_t tensor, ushort128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i8_st_tnsr_high(int5 ndx, const int8_t tensor, char256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u8_st_tnsr_high(int5 ndx, const int8_t tensor, uchar256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i1_st_tnsr_high(int5 ndx, const int8_t tensor, bool256 value, int switches=0, bool predicate=1, bool polarity=0);
#if defined(__gaudi2_plus__)
void v_f8_st_tnsr_high(int5 ndx, int8_t tensor, minifloat256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_h8_st_tnsr_high(int5 ndx, int8_t tensor, minihalf256 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__gaudi2_plus__)
/// @section sec13 Store
/// @subsection subsec13_20 ST_TNSR_HIGH with DIRECT switch
/// @brief Represents ST_TNSR_HIGH instruction with DIRECT switch.
///
/// @param addr1       Tensor store address low (SRC1).
/// @param addr2       Tensor store address high (SRC2).
/// @param value       Value to write (SRC3).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed switches are:
///     \li SW_PACK- When set, store pipe will perform packing according to PACK_DT.
/// \par
///     - [PACK_DT]
///         \li SW_PCK_32_TO_16
///         \li SW_PCK_16_TO_8
///         \li SW_PCK_32_TO_8
///         \li SW_PCK_8_TO_4
#endif
///
/// @{
#if defined(__gaudi2_plus__)
void v_f32_st_tnsr_high_direct(uint32_t addr1, uint32_t addr2, float64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_bf16_st_tnsr_high_direct(uint32_t addr1, uint32_t addr2, bfloat128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_f16_st_tnsr_high_direct(uint32_t addr1, uint32_t addr2, half128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_f8_st_tnsr_high_direct(uint32_t addr1, uint32_t addr2, minifloat256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_h8_st_tnsr_high_direct(uint32_t addr1, uint32_t addr2, minihalf256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i32_st_tnsr_high_direct(uint32_t addr1, uint32_t addr2, int64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u32_st_tnsr_high_direct(uint32_t addr1, uint32_t addr2, uint64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i16_st_tnsr_high_direct(uint32_t addr1, uint32_t addr2, short128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u16_st_tnsr_high_direct(uint32_t addr1, uint32_t addr2, ushort128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i8_st_tnsr_high_direct(uint32_t addr1, uint32_t addr2, char256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u8_st_tnsr_high_direct(uint32_t addr1, uint32_t addr2, uchar256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i1_st_tnsr_high_direct(uint32_t addr1, uint32_t addr2, bool256 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__gaudi_plus__)
/// @section sec13 Store
/// @subsection subsec13_21 ST_TNSR_HIGH with RMW_SEL switch
/// @brief Represents ST_TNSR_HIGH instruction with RMW_SEL switch.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param value       Value to write (SRC3).
/// @param rmw         Information for RMW operation (switches).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed switches are:
///     \li SW_PACK - When set, store pipe will perform packing according to PACK_DT.
#endif
#if defined(__greco_plus__)
/// \par
///     - [PACK_DT]
///         \li SW_PCK_32_TO_16
///         \li SW_PCK_16_TO_8
///         \li SW_PCK_32_TO_8
///         \li SW_PCK_8_TO_4
///
#endif
#if defined(__gaudi_plus__)
/// @switches Allowed RMW switches are:
///     - [RMW_DT]
///         \li RMW_DT_INT8
///         \li RMW_DT_INT16
///         \li RMW_DT_INT32
///         \li RMW_DT_UINT8
///         \li RMW_DT_UINT16
///         \li RMW_DT_UINT32
///         \li RMW_DT_BF16
///         \li RMW_DT_FP32
#endif
#if defined(__greco_plus__)
///         \li RMW_DT_FP16
#endif
#if defined(__gaudi2_plus__)
///         \li RMW_DT_FP8_152
#endif
#if defined(__gaudi_plus__)
/// \par
///     - [RMW_OP]
///         \li RMW_OP_ADD
///         \li RMW_OP_SUB
///         \li RMW_OP_MIN
///         \li RMW_OP_MAX
#endif
#if defined(__gaudi2_plus__)
///         \li RMW_OP_MAX_0_ADD
#endif
#if defined(__gaudi_plus__)
/// \par
///     \li RMW_TNSR_DT - When set RMW data type is taken from tensor descriptor, otherwise taken from switch value.
///     \li RMW_SET - Enable RMW.
///
#endif
/// @{
#if defined(__gaudi_plus__)
void v_f32_st_tnsr_high_rmw(int5 ndx, int8_t tensor, float64 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_bf16_st_tnsr_high_rmw(int5 ndx, int8_t tensor, bfloat128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_i32_st_tnsr_high_rmw(int5 ndx, int8_t tensor, int64 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_u32_st_tnsr_high_rmw(int5 ndx, int8_t tensor, uint64 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_i16_st_tnsr_high_rmw(int5 ndx, int8_t tensor, short128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_u16_st_tnsr_high_rmw(int5 ndx, int8_t tensor, ushort128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_i8_st_tnsr_high_rmw(int5 ndx, int8_t tensor, char256 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_u8_st_tnsr_high_rmw(int5 ndx, int8_t tensor, uchar256 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
void v_f16_st_tnsr_high_rmw(int5 ndx, int8_t tensor, half128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
void v_h8_st_tnsr_high_rmw(int5 ndx, int8_t tensor, minihalf256 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__gaudi2_plus__)
/// @section sec13 Store
/// @subsection subsec13_22 ST_TNSR_HIGH with RMW_SEL switch
/// @brief Represents ST_TNSR_HIGH instruction with RMW_SEL switch.
///
/// @param addr1       Tensor store address low (SRC1).
/// @param addr2       Tensor store address high (SRC2).
/// @param value       Value to write (SRC3).
/// @param rmw         Information for RMW operation (switches).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed switches are:
///     \li SW_PACK - When set, store pipe will perform packing according to PACK_DT.
/// \par
///     - [PACK_DT]
///         \li SW_PCK_32_TO_16
///         \li SW_PCK_16_TO_8
///         \li SW_PCK_32_TO_8
///         \li SW_PCK_8_TO_4
///
/// @switches Allowed RMW switches are:
///     - [RMW_DT]
///         \li RMW_DT_INT8
///         \li RMW_DT_INT16
///         \li RMW_DT_INT32
///         \li RMW_DT_UINT8
///         \li RMW_DT_UINT16
///         \li RMW_DT_UINT32
///         \li RMW_DT_BF16
///         \li RMW_DT_FP32
///         \li RMW_DT_FP16
///         \li RMW_DT_FP8_152
///     - [RMW_OP]
///         \li RMW_OP_ADD
///         \li RMW_OP_SUB
///         \li RMW_OP_MIN
///         \li RMW_OP_MAX
///         \li RMW_OP_MAX_0_ADD
/// \par
///     \li RMW_TNSR_DT - When set RMW data type is taken from tensor descriptor, otherwise taken from switch value.
///     \li RMW_SET - Enable RMW.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
void v_f32_st_tnsr_high_rmw_direct(uint32_t addr1, uint32_t addr2, float64 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_bf16_st_tnsr_high_rmw_direct(uint32_t addr1, uint32_t addr2, bfloat128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_f16_st_tnsr_high_rmw_direct(uint32_t addr1, uint32_t addr2, half128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_h8_st_tnsr_high_rmw_direct(uint32_t addr1, uint32_t addr2, minihalf256 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_i32_st_tnsr_high_rmw_direct(uint32_t addr1, uint32_t addr2, int64 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_u32_st_tnsr_high_rmw_direct(uint32_t addr1, uint32_t addr2, uint64 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_i16_st_tnsr_high_rmw_direct(uint32_t addr1, uint32_t addr2, short128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_u16_st_tnsr_high_rmw_direct(uint32_t addr1, uint32_t addr2, ushort128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_i8_st_tnsr_high_rmw_direct(uint32_t addr1, uint32_t addr2, char256 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_u8_st_tnsr_high_rmw_direct(uint32_t addr1, uint32_t addr2, uchar256 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ ST_TNSR_LOW
//

/// @section sec13 Store
/// @subsection subsec13_23 ST_TNSR_LOW
/// @brief Represents ST_TNSR_LOW instruction.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param value       Value to write (SRC3).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
#if defined(__gaudi_plus__)
/// @switches Allowed switches are:
///     \li SW_PACK - When set, store pipe will perform packing according to PACK_DT.
#endif
#if defined(__greco_plus__)
/// \par
///     - [PACK_DT]
///         \li SW_PCK_32_TO_16
///         \li SW_PCK_16_TO_8
///         \li SW_PCK_32_TO_8
///         \li SW_PCK_8_TO_4
#endif
///
/// @{
void v_f32_st_tnsr_low(int5 ndx, const int8_t tensor, float64 value, int switches=0, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
void v_bf16_st_tnsr_low(int5 ndx, const int8_t tensor, bfloat128 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
void v_f16_st_tnsr_low(int5 ndx, const int8_t tensor, half128 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
void v_f8_st_tnsr_low(int5 ndx, int8_t tensor, minifloat256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_h8_st_tnsr_low(int5 ndx, int8_t tensor, minihalf256 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
void v_i32_st_tnsr_low(int5 ndx, const int8_t tensor, int64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u32_st_tnsr_low(int5 ndx, const int8_t tensor, uint64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i16_st_tnsr_low(int5 ndx, const int8_t tensor, short128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u16_st_tnsr_low(int5 ndx, const int8_t tensor, ushort128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i8_st_tnsr_low(int5 ndx, const int8_t tensor, char256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u8_st_tnsr_low(int5 ndx, const int8_t tensor, uchar256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i1_st_tnsr_low(int5 ndx, const int8_t tensor, bool256 value, int switches=0, bool predicate=1, bool polarity=0);
/// @}


#if defined(__gaudi2_plus__)
/// @section sec13 Store
/// @subsection subsec13_24 ST_TNSR_LOW with DIRECT switch
/// @brief Represents ST_TNSR_LOW instruction with DIRECT switch.
///
/// @param addr1       Tensor store address low (SRC1).
/// @param addr2       Tensor store address high (SRC2).
/// @param value       Value to write (SRC3).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed switches are:
///     \li SW_PACK - When set, store pipe will perform packing according to PACK_DT.
/// \par
///     - [PACK_DT]
///         \li SW_PCK_32_TO_16
///         \li SW_PCK_16_TO_8
///         \li SW_PCK_32_TO_8
///         \li SW_PCK_8_TO_4
#endif
///
/// @{
#if defined(__gaudi2_plus__)
void v_f32_st_tnsr_low_direct(uint32_t addr1, uint32_t addr2, float64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_bf16_st_tnsr_low_direct(uint32_t addr1, uint32_t addr2, bfloat128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_f16_st_tnsr_low_direct(uint32_t addr1, uint32_t addr2, half128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_f8_st_tnsr_low_direct(uint32_t addr1, uint32_t addr2, minifloat256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_h8_st_tnsr_low_direct(uint32_t addr1, uint32_t addr2, minihalf256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i32_st_tnsr_low_direct(uint32_t addr1, uint32_t addr2, int64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u32_st_tnsr_low_direct(uint32_t addr1, uint32_t addr2, uint64 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i16_st_tnsr_low_direct(uint32_t addr1, uint32_t addr2, short128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u16_st_tnsr_low_direct(uint32_t addr1, uint32_t addr2, ushort128 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i8_st_tnsr_low_direct(uint32_t addr1, uint32_t addr2, char256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_u8_st_tnsr_low_direct(uint32_t addr1, uint32_t addr2, uchar256 value, int switches=0, bool predicate=1, bool polarity=0);
void v_i1_st_tnsr_low_direct(uint32_t addr1, uint32_t addr2, bool256 value, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__gaudi_plus__)
/// @section sec13 Store
/// @subsection subsec13_25 ST_TNSR_LOW with RMW_SEL switch
/// @brief Represents ST_TNSR_LOW instruction with RMW_SEL switch.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param value       Value to write (SRC3).
/// @param rmw         Information for RMW operation (switches).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed switches are:
///     \li SW_PACK - When set, store pipe will perform packing according to PACK_DT.
#endif
#if defined(__greco_plus__)
/// \par
///     - [PACK_DT]
///         \li SW_PCK_32_TO_16
///         \li SW_PCK_16_TO_8
///         \li SW_PCK_32_TO_8
///         \li SW_PCK_8_TO_4
///
#endif
#if defined(__gaudi_plus__)
/// @switches Allowed RMW switches are:
///     - [RMW_DT]
///         \li RMW_DT_INT8
///         \li RMW_DT_INT16
///         \li RMW_DT_INT32
///         \li RMW_DT_UINT8
///         \li RMW_DT_UINT16
///         \li RMW_DT_UINT32
///         \li RMW_DT_BF16
///         \li RMW_DT_FP32
#endif
#if defined(__greco_plus__)
///         \li RMW_DT_FP16
#endif
#if defined(__gaudi2_plus__)
///         \li RMW_DT_FP8_152
#endif
#if defined(__gaudi_plus__)
/// \par
///     - [RMW_OP]
///         \li RMW_OP_ADD
///         \li RMW_OP_SUB
///         \li RMW_OP_MIN
///         \li RMW_OP_MAX
#endif
#if defined(__gaudi2_plus__)
///         \li RMW_OP_MAX_0_ADD
#endif
#if defined(__gaudi_plus__)
/// \par
///     \li RMW_TNSR_DT - When set RMW data type is taken from tensor descriptor, otherwise taken from switch value.
///     \li RMW_SET - Enable RMW.
///
#endif
/// @{
#if defined(__gaudi_plus__)
void v_f32_st_tnsr_low_rmw(int5 ndx, int8_t tensor, float64 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_bf16_st_tnsr_low_rmw(int5 ndx, int8_t tensor, bfloat128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_i32_st_tnsr_low_rmw(int5 ndx, int8_t tensor, int64 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_u32_st_tnsr_low_rmw(int5 ndx, int8_t tensor, uint64 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_i16_st_tnsr_low_rmw(int5 ndx, int8_t tensor, short128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_u16_st_tnsr_low_rmw(int5 ndx, int8_t tensor, ushort128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_i8_st_tnsr_low_rmw(int5 ndx, int8_t tensor, char256 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_u8_st_tnsr_low_rmw(int5 ndx, int8_t tensor, uchar256 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
void v_f16_st_tnsr_low_rmw(int5 ndx, int8_t tensor, half128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
void v_h8_st_tnsr_low_rmw(int5 ndx, int8_t tensor, minihalf256 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}


#if defined(__gaudi2_plus__)
/// @section sec13 Store
/// @subsection subsec13_26 ST_TNSR_LOW with RMW_SEL  and DIRECT switches
/// @brief Represents ST_TNSR_LOW instruction with RMW_SEL and DIRECT switches.
///
/// @param addr1       Tensor store address low (SRC1).
/// @param addr2       Tensor store address high (SRC2).
/// @param value       Value to write (SRC3).
/// @param rmw         Information for RMW operation (switches).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// @switches Allowed switches are:
///     \li SW_PACK - When set, store pipe will perform packing according to PACK_DT.
/// \par
///     - [PACK_DT]
///         \li SW_PCK_32_TO_16
///         \li SW_PCK_16_TO_8
///         \li SW_PCK_32_TO_8
///         \li SW_PCK_8_TO_4
///
/// @switches Allowed RMW switches are:
///     - [RMW_DT]
///         \li RMW_DT_INT8
///         \li RMW_DT_INT16
///         \li RMW_DT_INT32
///         \li RMW_DT_UINT8
///         \li RMW_DT_UINT16
///         \li RMW_DT_UINT32
///         \li RMW_DT_BF16
///         \li RMW_DT_FP32
///         \li RMW_DT_FP16
///         \li RMW_DT_FP8_152
///     - [RMW_OP]
///         \li RMW_OP_ADD
///         \li RMW_OP_SUB
///         \li RMW_OP_MIN
///         \li RMW_OP_MAX
///         \li RMW_OP_MAX_0_ADD
/// \par
///     \li RMW_TNSR_DT - When set RMW data type is taken from tensor descriptor, otherwise taken from switch value.
///     \li RMW_SET - Enable RMW.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
void v_f32_st_tnsr_low_rmw_direct(uint32_t addr1, uint32_t addr2, float64 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_bf16_st_tnsr_low_rmw_direct(uint32_t addr1, uint32_t addr2, bfloat128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_f16_st_tnsr_low_rmw_direct(uint32_t addr1, uint32_t addr2, half128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_h8_st_tnsr_low_rmw_direct(uint32_t addr1, uint32_t addr2, minihalf256 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_i32_st_tnsr_low_rmw_direct(uint32_t addr1, uint32_t addr2, int64 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_u32_st_tnsr_low_rmw_direct(uint32_t addr1, uint32_t addr2, uint64 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_i16_st_tnsr_low_rmw_direct(uint32_t addr1, uint32_t addr2, short128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_u16_st_tnsr_low_rmw_direct(uint32_t addr1, uint32_t addr2, ushort128 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_i8_st_tnsr_low_rmw_direct(uint32_t addr1, uint32_t addr2, char256 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
void v_u8_st_tnsr_low_rmw_direct(uint32_t addr1, uint32_t addr2, uchar256 value, int rmw, int switches=0, bool predicate=1, bool polarity=0);
#endif
/// @}

#if defined(__gaudi2_plus__)
/// @brief Resets a specific SQZ_CNTR register
///
/// @param cntr_idx    SQZ_CNTR register number.
/// @param val         The new value of the register.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Updated value of the squeeze counter.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
void reset_sqz_cntr(const int8_t cntr_idx, int32_t val, bool predicate=1, bool polarity=0);
#endif
/// @}

//
// ------ ST_TNSR_SQZ
//

#if defined(__gaudi2_plus__)
/// @section sec13 Store
/// @subsection subsec13_27 ST_TNSR_SQZ
/// @brief Represents ST_TNSR_SQZ instruction.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor ID.
/// @param sqz         Squeeze counter.
/// @param value       Value to write (SRC3).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
///
/// Write consecutively ("squeeze") the \p value data bytes with \c predicate[i]=1 to the lower side of the vector. \n
/// Squeezed data bytes are kept in the same order as they were in the \p value before squeezing.
///
#endif
#if defined(__doron1__)
/// @switches Allowed switches are:
///     \li SW_CNT_ONLY - Performs squeeze and updates the appropriate squeeze counter, \n
///         but at the end of the operation discards the data and does not store it to memory.
///     \li SW_FLUSH    - No new data will be stored. \n
///         Instead, only the data already in the partial buffer of this SQUEEZE_ID will be flushed to memory.
#endif
/// @{
#if defined(__gaudi2_plus__)
void v_f32_st_tnsr_sqz(int5 ndx, int8_t tensor, const int8_t sqz, float64 value, int switches, bool256 predicate, bool polarity=0);
void v_i32_st_tnsr_sqz(int5 ndx, int8_t tensor, const int8_t sqz, int64 value, int switches, bool256 predicate, bool polarity=0);
void v_u32_st_tnsr_sqz(int5 ndx, int8_t tensor, const int8_t sqz, uint64 value, int switches, bool256 predicate, bool polarity=0);
#endif
/// @}


#if defined(__gaudi2_plus__)
/// @section sec13 Store
/// @subsection subsec13_28 ST_TNSR_SQZ with RMW_SEL switch
/// @brief Represents ST_TNSR_SQZ instruction with RMW_SEL switch.
///
/// @param ndx         Tensor coordinates (SRC1).
/// @param tensor      Tensor number.
/// @param sqz         Squeeze counter.
/// @param value       Value to write (SRC3).
/// @param rmw         Information for RMW operation (switches).
/// @param switches    Instruction switches.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Updated value of the squeeze counter.
///
/// Write consecutively ("squeeze") the \p value data bytes with \c predicate[i]=1 to the lower side of the vector. \n
/// Squeezed data bytes are kept in the same order as they were in the \p value before squeezing.
///
/// @switches Allowed RMW switches are:
///     - [RMW_DT]
///         \li RMW_DT_INT8
///         \li RMW_DT_INT16
///         \li RMW_DT_INT32
///         \li RMW_DT_UINT8
///         \li RMW_DT_UINT16
///         \li RMW_DT_UINT32
///         \li RMW_DT_BF16
///         \li RMW_DT_FP32
///         \li RMW_DT_FP16
///         \li RMW_DT_FP8_152
///     - [RMW_OP]
///         \li RMW_OP_ADD
///         \li RMW_OP_SUB
///         \li RMW_OP_MIN
///         \li RMW_OP_MAX
///         \li RMW_OP_MAX_0_ADD
/// \par
///     \li RMW_TNSR_DT - When set RMW data type is taken from tensor descriptor, otherwise taken from switch value.
///     \li RMW_SET - Enable RMW.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
void v_f32_st_tnsr_sqz_rmw(int5 ndx, int8_t tensor, const int8_t sqz, float64 value, int rmw, int switches, bool256 predicate, bool polarity=0);
void v_i32_st_tnsr_sqz_rmw(int5 ndx, int8_t tensor, const int8_t sqz, int64 value, int rmw, int switches, bool256 predicate, bool polarity=0);
void v_u32_st_tnsr_sqz_rmw(int5 ndx, int8_t tensor, const int8_t sqz, uint64 value, int rmw, int switches, bool256 predicate, bool polarity=0);
#endif
/// @}


//
// ------ SUB
//

/// @section sec2 Arithmetic
/// @subsection subsec2_61 Subtract
/// @brief Represents SUB instruction.
///
/// @param a           The first SRC operand to SUB (SRC1).
/// @param b           The second SRC operand to SUB (SRC2).
/// @param switches    Switches of SUB instructions.
/// @param income      This value is returned if the predicate is false.
/// @param switches    Switches of SUB instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of the operation.
///
/// @switches Allowed switches are:
///     \li SW_SAT - Saturate (integer types only).
///     \li SW_NEG - Negates the destination after the operation.
///
/// @{
float s_f32_sub(float a, float b, int switches=0, float income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_bf16_sub(bf16 a, bf16 b, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_f16_sub(half a, half b, int switches=0, half income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
float s_f8_sub(float a, minifloat b, int switches=0, float income={}, bool predicate=1, bool polarity=0);
float s_h8_sub(float a, minihalf  b, int switches=0, float income={}, bool predicate=1, bool polarity=0);
#endif
int32_t s_i32_sub(int32_t a, int32_t b, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_u32_sub(uint32_t a, uint32_t b, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_i16_sub(int16_t a, int16_t b, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_u16_sub(uint16_t a, uint16_t b, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_i8_sub(int8_t a, int8_t b, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u8_sub(uint8_t a, uint8_t b, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);

float64 v_f32_sub_vb(float64 a, float64 b, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_sub_b(float64 a, float64 b, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_sub_vb(bfloat128 a, bfloat128 b, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_sub_b(bfloat128 a, bfloat128 b, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudib__) || defined(__greco_plus__)
half128 v_f16_sub_vb(half128 a, half128 b, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_sub_b(half128 a, half128 b, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
float256 v_f8_sub_vb(float256 a, minifloat256 b, int switches,   float256 income,    bool256 predicate, bool polarity=0);
float256 v_f8_sub_b (float256 a, minifloat256 b, int switches=0, float256 income={}, bool predicate=1,  bool polarity=0);
float256 v_h8_sub_vb(float256 a, minihalf256  b, int switches,   float256 income,    bool256 predicate, bool polarity=0);
float256 v_h8_sub_b (float256 a, minihalf256  b, int switches=0, float256 income={}, bool predicate=1,  bool polarity=0);
#endif
int64 v_i32_sub_vb(int64 a, int64 b, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_sub_b(int64 a, int64 b, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_sub_vb(uint64 a, uint64 b, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_sub_b(uint64 a, uint64 b, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_sub_vb(short128 a, short128 b, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_sub_b(short128 a, short128 b, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_sub_vb(ushort128 a, ushort128 b, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_sub_b(ushort128 a, ushort128 b, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_sub_vb(char256 a, char256 b, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_sub_b(char256 a, char256 b, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_sub_vb(uchar256 a, uchar256 b, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_sub_b(uchar256 a, uchar256 b, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
/// @}


/// @section sec11 IRF
/// @subsection subsec11_3 Sub
/// @brief Represents SUB instruction for int5.
///
/// @param a           The first SRC operand to SUB (SRC1).
/// @param b           The second SRC operand to SUB (SRC2).
/// @param dimmask     Selects IRF lanes participated in the operation.
/// @param switches    Switches of SUB instructions.
/// @param income      This value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of the operation.
///
/// @{
int5 i_i32_sub(int5 a, int5 b, const int dimmask, int switches, int5 income, bool predicate=1, bool polarity=0);
/// @}


#if defined(__gaudi2_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_62 Subtract with X2 switch
/// @brief Represents SUB instruction with X2 switch.
///
/// @param a           The first SRC operand to SUB (SRC1).
/// @param b           The second SRC operand to SUB (SRC2).
/// @param c           The third SRC operand to SUB (SRC3).
/// @param switches    Switches of SUB instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation - \c DEST.v1=a.v1-b \c DEST.v2=a.v2-c.
///
/// @note \p a is float128 data-type that map to 2 vector registers of 64 float elements each. \n
/// In the formula above \p a.v1 and \p a.v2 refers to the first and second vectors respectively.
///
/// @switches Allowed switches are:
///     \li SW_NEG - Negates the destination after the operation.
///
#endif
/// @{
#if defined(__gaudi2_plus__)
float128 v_f32_sub_x2_vb(float128 a, float64 b, float64 c, int switches, float128 income, bool128 predicate, bool polarity=0);
float128 v_f32_sub_x2_b(float128 a, float64 b, float64 c, int switches=0, float128 income={}, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ UDIV_STEP, UDIV_4STEP, UDIV
//

#if defined(__goya__) || defined(__gaudi__) || defined(__gaudib__) || defined(__greco__)
/// @section sec2 Arithmetic
/// @subsection subsec2_63 udiv
/// @brief Represents UDIV_STEP and UDIV_4STEP instructions.
///
/// This instruction performs 1 or 4 division steps of UINT32, UINT8, or UINT16.
/// @param a           Division denominator (SRC1).
/// @param step        The current iteration number. \n
///                    For udiv_4step the instruction will perform this step as well as the 3 consecutive steps.
/// @param switches    Instruction switches.
/// @param income      Income value of DEST is an in-out pair where: \n
///                    In the first iteration \p incom.v1 is the division numerator. \n
///                    After 32/16/8 iterations (for UINT32/UINT16/UINT8 respectivly) \n
///                    \p DEST.v2 is the division quotient, and \p DEST.v1 is the division reminder.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of the operation.
///
#endif
#if defined(__greco__)
/// @switches Allowed switches are:
///     \li SW_X2_UDIV_4STEP - Calculate 8 steps (for udiv_4step only)
///
#endif
/// @{
#if defined(__goya__)
uint32_t_pair_t u32_udiv_step(uint32_t a, uint32_t step, int switches=0, uint32_t_pair_t income={}, bool predicate=1, bool polarity=0);
uint16_t_pair_t u16_udiv_step(uint16_t a, uint32_t step, int switches=0, uint16_t_pair_t income={}, bool predicate=1, bool polarity=0);
uint8_t_pair_t u8_udiv_step(uint8_t a, uint32_t step, int switches=0, uint8_t_pair_t income={}, bool predicate=1, bool polarity=0);
#endif
//TODO: add support Goya2
#if defined(__gaudi__) || defined(__gaudib__) || defined(__greco__)
uint32_t_pair_t u32_udiv_4step(uint32_t a, uint32_t step, int switches=0, uint32_t_pair_t income={}, bool predicate=1, bool polarity=0);
uint16_t_pair_t u16_udiv_4step(uint16_t a, uint32_t step, int switches=0, uint16_t_pair_t income={}, bool predicate=1, bool polarity=0);
uint8_t_pair_t u8_udiv_4step(uint8_t a, uint32_t step, int switches=0, uint8_t_pair_t income={}, bool predicate=1, bool polarity=0);
#endif
/// @}

#if defined(__gaudi2_plus__)
/// @section sec2 Arithmetic
/// @subsection subsec2_64 udiv both
/// @brief Represents UDIV instruction.
///
/// @param a           The first SRC operand to UDIV (SRC1).
/// @param b           The second SRC operand to UDIV (SRC1).
/// @param switches    Instruction switches.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of the operation.
///
/// @switches Allowed switches are:
///     - [DIV_MODE]
///         \li SW_DIV_MODE_DIV - Return \p a / \p b
///         \li SW_DIV_MODE_MOD - Return \p a % \p b
///         \li SW_DIV_MODE_BOTH - Return a pair ( \p a / \p b , \p a % \p b ) (when using variations with _both suffix)
///
#endif
/// @{
#if defined(__gaudi2_plus__)
uint32_t_pair_t u32_udiv_both(uint32_t a, uint32_t b, int switches, uint32_t_pair_t income, bool predicate=1, bool polarity=0);
uint16_t_pair_t u16_udiv_both(uint16_t a, uint16_t b, int switches, uint16_t_pair_t income, bool predicate=1, bool polarity=0);
uint8_t_pair_t u8_udiv_both(uint8_t a, uint8_t b, int switches, uint8_t_pair_t income, bool predicate=1, bool polarity=0);

uint32_t u32_udiv(uint32_t a, uint32_t b, int switches, uint32_t income, bool predicate=1, bool polarity=0);
uint16_t u16_udiv(uint16_t a, uint16_t b, int switches, uint16_t income, bool predicate=1, bool polarity=0);
uint8_t u8_udiv(uint8_t a, uint8_t b, int switches, uint8_t income, bool predicate=1, bool polarity=0);
#endif
/// @}


//
// ------ UNPACK
//

/// @section sec10 Pack/Unpack
/// @subsection subsec10_2 Unpack
/// @brief Represents UNPACK instruction.
///
/// @param a           Value to be transformed (SRC1).
/// @param switches    Switches of the instruction.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Unpacked value.
///
/// @switches Allowed switches are:
///     - [GROUP_SOURCE]
///         \li SW_GROUP_0
///         \li SW_GROUP_1
///     - [ELEMENT_STRIDE]
///         \li SW_STRIDE_2 - Every second element is valid.
///         \li SW_STRIDE_4 - Every forth element is valid.
///     - [GROUP_HALF]
///         \li SW_GROUP_HALF_0 - The lower half of the input group is taken.
///         \li SW_GROUP_HALF_1 - The upper half of the input group is taken.
#if defined(__gaudi2_plus__)
/// \par
///     - [UNPACK_LANE]
///         \li SW_UNPACK_LANE_0 - Unpack to lane 0.
///         \li SW_UNPACK_LANE_1 - Unpack to lane 1.
///         \li SW_UNPACK_LANE_2 - Unpack to lane 2 (only for 8bit to 32bit).
///         \li SW_UNPACK_LANE_3 - Unpack to lane 3 (only for 8bit to 32bit).
#endif
///
/// @{
#if defined(__gaudi_plus__)
bfloat128 v_bf16_unpack_b(bfloat128 a, int switches, bfloat128 income, bool predicate=1, bool polarity=0);
bfloat128 v_bf16_unpack_vb(bfloat128 a, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_f16_unpack_b(half128 a, int switches, half128 income, bool predicate=1, bool polarity=0);
half128 v_f16_unpack_vb(half128 a, int switches, half128 income, bool128 predicate, bool polarity=0);
#endif
#if defined(__greco_plus__)
minifloat256 v_f8_unpack_b(minifloat256 a, int switches, minifloat256 income, bool predicate=1, bool polarity=0);
minifloat256 v_f8_unpack_vb(minifloat256 a, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_unpack_b(minihalf256 a, int switches, minihalf256 income, bool predicate=1, bool polarity=0);
minihalf256 v_h8_unpack_vb(minihalf256 a, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
#endif
short128 v_i16_unpack_b(short128 a, int switches, short128 income, bool predicate=1, bool polarity=0);
short128 v_i16_unpack_vb(short128 a, int switches, short128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_unpack_b(ushort128 a, int switches, ushort128 income, bool predicate=1, bool polarity=0);
ushort128 v_u16_unpack_vb(ushort128 a, int switches, ushort128 income, bool128 predicate, bool polarity=0);
char256 v_i8_unpack_b(char256 a, int switches, char256 income, bool predicate=1, bool polarity=0);
char256 v_i8_unpack_vb(char256 a, int switches, char256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_unpack_b(uchar256 a, int switches, uchar256 income, bool predicate=1, bool polarity=0);
uchar256 v_u8_unpack_vb(uchar256 a, int switches, uchar256 income, bool256 predicate, bool polarity=0);
/// @}


//
// ------ XOR
//

/// @section sec2 Arithmetic
/// @subsection subsec2_65 Xor
/// @brief Represents XOR instruction.
///
/// @param a           The first SRC operand to XOR (SRC1).
/// @param b           The second SRC operand to XOR (SRC2).
/// @param switches    Switches of XOR instructions.
/// @param income      This value is returned if the predicate is false.
/// @param predicate   Predicate value for the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @{
float s_f32_xor(float a, float b, int switches=0, float income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bf16 s_bf16_xor(bf16 a, bf16 b, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half s_f16_xor(half a, half b, int switches=0, bf16 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat s_f8_xor(minifloat a, minifloat b, int switches=0, minifloat income={}, bool predicate=1, bool polarity=0);
minihalf s_h8_xor(minihalf a, minihalf b, int switches=0, minihalf income={}, bool predicate=1, bool polarity=0);
#endif
int32_t s_i32_xor(int32_t a, int32_t b, int switches=0, int32_t income={}, bool predicate=1, bool polarity=0);
uint32_t s_u32_xor(uint32_t a, uint32_t b, int switches=0, uint32_t income={}, bool predicate=1, bool polarity=0);
int16_t s_i16_xor(int16_t a, int16_t b, int switches=0, int16_t income={}, bool predicate=1, bool polarity=0);
uint16_t s_u16_xor(uint16_t a, uint16_t b, int switches=0, uint16_t income={}, bool predicate=1, bool polarity=0);
int8_t s_i8_xor(int8_t a, int8_t b, int switches=0, int8_t income={}, bool predicate=1, bool polarity=0);
uint8_t s_u8_xor(uint8_t a, uint8_t b, int switches=0, uint8_t income={}, bool predicate=1, bool polarity=0);
bool s_i1_xor(bool a, bool b, int switches=0, bool income={}, bool predicate=1, bool polarity=0);

float64 v_f32_xor_vb(float64 a, float64 b, int switches, float64 income, bool64 predicate, bool polarity=0);
float64 v_f32_xor_b(float64 a, float64 b, int switches=0, float64 income={}, bool predicate=1, bool polarity=0);
#if defined(__gaudi_plus__)
bfloat128 v_bf16_xor_vb(bfloat128 a, bfloat128 b, int switches, bfloat128 income, bool128 predicate, bool polarity=0);
bfloat128 v_bf16_xor_b(bfloat128 a, bfloat128 b, int switches=0, bfloat128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__greco_plus__)
half128 v_f16_xor_vb(half128 a, half128 b, int switches, half128 income, bool128 predicate, bool polarity=0);
half128 v_f16_xor_b(half128 a, half128 b, int switches=0, half128 income={}, bool predicate=1, bool polarity=0);
#endif
#if defined(__gaudi2_plus__)
minifloat256 v_f8_xor_vb(minifloat256 a, minifloat256 b, int switches, minifloat256 income, bool256 predicate, bool polarity=0);
minifloat256 v_f8_xor_b(minifloat256 a, minifloat256 b, int switches=0, minifloat256 income={}, bool predicate=1, bool polarity=0);
minihalf256 v_h8_xor_vb(minihalf256 a, minihalf256 b, int switches, minihalf256 income, bool256 predicate, bool polarity=0);
minihalf256 v_h8_xor_b(minihalf256 a, minihalf256 b, int switches=0, minihalf256 income={}, bool predicate=1, bool polarity=0);
#endif
int64 v_i32_xor_vb(int64 a, int64 b, int switches, int64 income, bool64 predicate, bool polarity=0);
int64 v_i32_xor_b(int64 a, int64 b, int switches=0, int64 income={}, bool predicate=1, bool polarity=0);
uint64 v_u32_xor_vb(uint64 a, uint64 b, int switches, uint64 income, bool64 predicate, bool polarity=0);
uint64 v_u32_xor_b(uint64 a, uint64 b, int switches=0, uint64 income={}, bool predicate=1, bool polarity=0);
short128 v_i16_xor_vb(short128 a, short128 b, int switches, short128 income, bool128 predicate, bool polarity=0);
short128 v_i16_xor_b(short128 a, short128 b, int switches=0, short128 income={}, bool predicate=1, bool polarity=0);
ushort128 v_u16_xor_vb(ushort128 a, ushort128 b, int switches, ushort128 income, bool128 predicate, bool polarity=0);
ushort128 v_u16_xor_b(ushort128 a, ushort128 b, int switches=0, ushort128 income={}, bool predicate=1, bool polarity=0);
char256 v_i8_xor_vb(char256 a, char256 b, int switches, char256 income, bool256 predicate, bool polarity=0);
char256 v_i8_xor_b(char256 a, char256 b, int switches=0, char256 income={}, bool predicate=1, bool polarity=0);
uchar256 v_u8_xor_vb(uchar256 a, uchar256 b, int switches, uchar256 income, bool256 predicate, bool polarity=0);
uchar256 v_u8_xor_b(uchar256 a, uchar256 b, int switches=0, uchar256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_xor_b(bool256 a, bool256 b, int switches=0, bool256 income={}, bool predicate=1, bool polarity=0);
bool256 v_i1_xor_vb(bool256 a, bool256 b, int switches, bool256 income, bool256 predicate, bool polarity=0);
/// @}


/// @section sec11 IRF
/// @subsection subsec11_4 Xor for IRF
/// @brief Represents scalar XOR instruction for int5.
///
/// @param a           The first SRC operand to XOR (SRC1).
/// @param b           The second SRC operand to XOR (SRC2).
/// @param dimmask     Selects IRF lanes participated in the operation.
/// @param switches    Switches of XOR instructions.
/// @param income      This value is returned if the predicate is false or when dimmask does not apply.
/// @param predicate   Predicate value fxor the instruction.
/// @param polarity    True if polarity of the predicate is inverted.
/// @return Result of operation.
///
/// @{
int5 i_i32_xor(int5 a, int5 b, const int dimmask, int switches, int5 income, bool predicate=1, bool polarity=0);
/// @}

/// @brief Return dimension size for the given tensor id.
///
/// @param a           Tensor id.
/// @param dim         Dimension.
/// @return Dimension size.
///
/// @{
uint32_t get_dim_size(int32_t a, uint32_t dim);
/// @}

/// @brief Set dimension size for the given tensor id.
///
/// @param a           Tensor id.
/// @param dim         Dimension.
/// @param value       The new value of size.
///
/// @{
void set_dim_size(int32_t a, uint32_t dim, uint32_t value);
/// @}

/// @brief Return dimension stride for the given tensor id.
///
/// @param a           Tensor id.
/// @param dim         Dimension.
/// @return Dimension stride.
///
/// @{
uint32_t get_dim_stride(int32_t a, uint32_t dim);
/// @}

/// @brief Set dimension stride for the given tensor id.
///
/// @param a           Tensor id.
/// @param dim         Dimension.
/// @param value       The new value of stride.
///
/// @{
void set_dim_stride(int32_t a, uint32_t dim, uint32_t value);
/// @}

/// @brief Return tensor hw pref stride.
///
/// @param tensor      Tensor id.
/// @return Stride.
///
/// @{
#if defined(__gaudi2_plus__)
uint32_t get_tensor_hwpref_stride(int32_t tensor);
#endif
/// @}

/// @brief Set tensor hw pref stride.
///
/// @param tensor      Tensor id.
/// @param value       The new value of stride.
///
/// @{
#if defined(__gaudi2_plus__)
void set_tensor_hwpref_stride(int32_t tensor, uint32_t value);
#endif
/// @}

/// @brief Return padding value for the given tensor id.
///
/// @param a           Tensor id.
/// @return Padding value.
///
/// @{
uint32_t get_pad_value_uint(int32_t a);
int32_t  get_pad_value_int(int32_t a);
float    get_pad_value_float(int32_t a);
uint16_t get_pad_value_ushort(int32_t a);
int16_t  get_pad_value_short(int32_t a);
uint8_t  get_pad_value_uchar(int32_t a);
int8_t   get_pad_value_char(int32_t a);
#if defined(__gaudi_plus__)
bf16     get_pad_value_bf16(int32_t a);
#endif
/// @}

/// @brief Set padding value for the given tensor id.
///
/// @param a           Tensor id.
/// @param value       The new padding value.
///
/// @{
void set_pad_value_uint(int32_t a, uint32_t value);
void set_pad_value_int(int32_t a, int32_t value);
void set_pad_value_float(int32_t a, float value);
void set_pad_value_ushort(int32_t a, uint16_t value);
void set_pad_value_short(int32_t a, int16_t value);
void set_pad_value_uchar(int32_t a, uint8_t value);
void set_pad_value_char(int32_t a, int8_t value);
#if defined(__gaudi_plus__)
void set_pad_value_bf16(int32_t a, bf16 value);
#endif
/// @}

/// @brief Return semaphore value.
///
/// @return Semaphore value.
///
/// @{
int32_t get_semaphore_value();
/// @}

/// @brief Set semaphore value.
///
/// @param value       The new padding value.
///
/// @{
void set_semaphore_value(int32_t val);
/// @}

/// @brief Return arithmetic round csr value.
///
/// @return arithmetic round csr value.
///
/// @{
int32_t get_csr_value();
/// @}

/// @brief Set arithmetic round csr value.
///
/// @param value       The new round csr value.
///
/// @{
void set_csr_value(int32_t val);
/// @}

/// @brief Return convert round csr value.
///
/// @return convert round csr value.
///
/// @{
int32_t get_convert_csr_value();
/// @}

/// @brief Set convert round csr value.
///
/// @param value       The new round csr value.
///
/// @{
void set_convert_csr_value(int32_t val);
/// @}
