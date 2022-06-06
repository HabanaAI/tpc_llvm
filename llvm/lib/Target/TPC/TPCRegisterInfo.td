//===- TPCRegisterInfo.td - Describe the TPC Register File --*- tablegen -*-==//
//
//
//===----------------------------------------------------------------------===//
//
// This file describes the TPC Register file, defining the registers themselves,
// aliases between the registers, and the register classes built out of the
// registers.
//
//===----------------------------------------------------------------------===//

class TPCReg<string n, bits<16> Enc, list<Register> subregs = []> : Register<n> {
  let Namespace = "TPC";
  let HWEncoding = Enc;
  let SubRegs = subregs;
}

let Namespace = "TPC" in {
  def sub_0  : SubRegIndex<2048>;
  def sub_1  : SubRegIndex<2048, 2048>;
  def sub_2  : SubRegIndex<2048, 4096>;
  def sub_3  : SubRegIndex<2048, 6144>;

  // Indices for scalar register pairs.
  def sub_s0 : SubRegIndex<1, 0>;
  def sub_s1 : SubRegIndex<1, 1>;
}


// SRF Registers
def S0  : TPCReg<"S0" ,  0>, DwarfRegNum<[0]>;
def S1  : TPCReg<"S1" ,  1>, DwarfRegNum<[1]>;
def S2  : TPCReg<"S2" ,  2>, DwarfRegNum<[2]>;
def S3  : TPCReg<"S3" ,  3>, DwarfRegNum<[3]>;
def S4  : TPCReg<"S4" ,  4>, DwarfRegNum<[4]>;
def S5  : TPCReg<"S5" ,  5>, DwarfRegNum<[5]>;
def S6  : TPCReg<"S6" ,  6>, DwarfRegNum<[6]>;
def S7  : TPCReg<"S7" ,  7>, DwarfRegNum<[7]>;
def S8  : TPCReg<"S8" ,  8>, DwarfRegNum<[8]>;
def S9  : TPCReg<"S9" ,  9>, DwarfRegNum<[9]>;
def S10 : TPCReg<"S10", 10>, DwarfRegNum<[10]>;
def S11 : TPCReg<"S11", 11>, DwarfRegNum<[11]>;
def S12 : TPCReg<"S12", 12>, DwarfRegNum<[12]>;
def S13 : TPCReg<"S13", 13>, DwarfRegNum<[13]>;
def S14 : TPCReg<"S14", 14>, DwarfRegNum<[14]>;
def S15 : TPCReg<"S15", 15>, DwarfRegNum<[15]>;
def S16 : TPCReg<"S16", 16>, DwarfRegNum<[16]>;
def S17 : TPCReg<"S17", 17>, DwarfRegNum<[17]>;
def S18 : TPCReg<"S18", 18>, DwarfRegNum<[18]>;
def S19 : TPCReg<"S19", 19>, DwarfRegNum<[19]>;
def S20 : TPCReg<"S20", 20>, DwarfRegNum<[20]>;
def S21 : TPCReg<"S21", 21>, DwarfRegNum<[21]>;
def S22 : TPCReg<"S22", 22>, DwarfRegNum<[22]>;
def S23 : TPCReg<"S23", 23>, DwarfRegNum<[23]>;
def S24 : TPCReg<"S24", 24>, DwarfRegNum<[24]>;
def S25 : TPCReg<"S25", 25>, DwarfRegNum<[25]>;
def S26 : TPCReg<"S26", 26>, DwarfRegNum<[26]>;
def S27 : TPCReg<"S27", 27>, DwarfRegNum<[27]>;
def S28 : TPCReg<"S28", 28>, DwarfRegNum<[28]>;
def S29 : TPCReg<"S29", 29>, DwarfRegNum<[29]>;
def S30 : TPCReg<"S30", 30>, DwarfRegNum<[30]>;
def S31 : TPCReg<"S31", 31>, DwarfRegNum<[31]>;
def S32 : TPCReg<"S32", 32>, DwarfRegNum<[32]>;
def S33 : TPCReg<"S33", 33>, DwarfRegNum<[33]>;
def S34 : TPCReg<"S34", 34>, DwarfRegNum<[34]>;
def S35 : TPCReg<"S35", 35>, DwarfRegNum<[35]>;
def S36 : TPCReg<"S36", 36>, DwarfRegNum<[246]>;
def S37 : TPCReg<"S37", 37>, DwarfRegNum<[247]>;
def S38 : TPCReg<"S38", 38>, DwarfRegNum<[248]>;
def S39 : TPCReg<"S39", 39>, DwarfRegNum<[249]>;
// TODO: the same encoding as LSFR, isn't this a problem?
def S40 : TPCReg<"S40", 40>, DwarfRegNum<[250]>;
def S41 : TPCReg<"S41", 41>, DwarfRegNum<[251]>;
def S42 : TPCReg<"S42", 42>, DwarfRegNum<[252]>;
def S43 : TPCReg<"S43", 43>, DwarfRegNum<[253]>;
def S44 : TPCReg<"S44", 44>, DwarfRegNum<[254]>;
def S45 : TPCReg<"S45", 45>, DwarfRegNum<[255]>;
def S46 : TPCReg<"S46", 46>, DwarfRegNum<[256]>;
def S47 : TPCReg<"S47", 47>, DwarfRegNum<[257]>;
def S48 : TPCReg<"S48", 48>, DwarfRegNum<[258]>;
def S49 : TPCReg<"S49", 49>, DwarfRegNum<[259]>;
def S50 : TPCReg<"S50", 50>, DwarfRegNum<[260]>;
def S51 : TPCReg<"S51", 51>, DwarfRegNum<[261]>;
def S52 : TPCReg<"S52", 52>, DwarfRegNum<[262]>;
def S53 : TPCReg<"S53", 53>, DwarfRegNum<[263]>;
def S54 : TPCReg<"S54", 54>, DwarfRegNum<[264]>;
def S55 : TPCReg<"S55", 55>, DwarfRegNum<[265]>;
def S56 : TPCReg<"S56", 56>, DwarfRegNum<[266]>;
def S57 : TPCReg<"S57", 57>, DwarfRegNum<[267]>;
def S58 : TPCReg<"S58", 58>, DwarfRegNum<[268]>;
def S59 : TPCReg<"S59", 59>, DwarfRegNum<[269]>;
def S60 : TPCReg<"S60", 60>, DwarfRegNum<[270]>;
def S61 : TPCReg<"S61", 61>, DwarfRegNum<[271]>;
def S62 : TPCReg<"S62", 62>, DwarfRegNum<[272]>;
def S63 : TPCReg<"S63", 63>, DwarfRegNum<[273]>;
def S_LFSR           : TPCReg<"S_LFSR"          , 40>, DwarfRegNum<[40]>;
def S_LFSR_NO_CHANGE : TPCReg<"S_LFSR_NO_CHANGE", 41>, DwarfRegNum<[41]>;


// VRF Registers
def V0  : TPCReg<"V0" , 0 >, DwarfRegNum<[64]>;
def V1  : TPCReg<"V1" , 1 >, DwarfRegNum<[65]>;
def V2  : TPCReg<"V2" , 2 >, DwarfRegNum<[66]>;
def V3  : TPCReg<"V3" , 3 >, DwarfRegNum<[67]>;
def V4  : TPCReg<"V4" , 4 >, DwarfRegNum<[68]>;
def V5  : TPCReg<"V5" , 5 >, DwarfRegNum<[69]>;
def V6  : TPCReg<"V6" , 6 >, DwarfRegNum<[70]>;
def V7  : TPCReg<"V7" , 7 >, DwarfRegNum<[71]>;
def V8  : TPCReg<"V8" , 8 >, DwarfRegNum<[72]>;
def V9  : TPCReg<"V9" , 9 >, DwarfRegNum<[73]>;
def V10 : TPCReg<"V10", 10>, DwarfRegNum<[74]>;
def V11 : TPCReg<"V11", 11>, DwarfRegNum<[75]>;
def V12 : TPCReg<"V12", 12>, DwarfRegNum<[76]>;
def V13 : TPCReg<"V13", 13>, DwarfRegNum<[77]>;
def V14 : TPCReg<"V14", 14>, DwarfRegNum<[78]>;
def V15 : TPCReg<"V15", 15>, DwarfRegNum<[79]>;
def V16 : TPCReg<"V16", 16>, DwarfRegNum<[80]>;
def V17 : TPCReg<"V17", 17>, DwarfRegNum<[81]>;
def V18 : TPCReg<"V18", 18>, DwarfRegNum<[82]>;
def V19 : TPCReg<"V19", 19>, DwarfRegNum<[83]>;
def V20 : TPCReg<"V20", 20>, DwarfRegNum<[84]>;
def V21 : TPCReg<"V21", 21>, DwarfRegNum<[85]>;
def V22 : TPCReg<"V22", 22>, DwarfRegNum<[86]>;
def V23 : TPCReg<"V23", 23>, DwarfRegNum<[87]>;
def V24 : TPCReg<"V24", 24>, DwarfRegNum<[88]>;
def V25 : TPCReg<"V25", 25>, DwarfRegNum<[89]>;
def V26 : TPCReg<"V26", 26>, DwarfRegNum<[90]>;
def V27 : TPCReg<"V27", 27>, DwarfRegNum<[91]>;
def V28 : TPCReg<"V28", 28>, DwarfRegNum<[92]>;
def V29 : TPCReg<"V29", 29>, DwarfRegNum<[93]>;
def V30 : TPCReg<"V30", 30>, DwarfRegNum<[94]>;
def V31 : TPCReg<"V31", 31>, DwarfRegNum<[95]>;
def V32 : TPCReg<"V32", 32>, DwarfRegNum<[96]>;
def V33 : TPCReg<"V33", 33>, DwarfRegNum<[97]>;
def V34 : TPCReg<"V34", 34>, DwarfRegNum<[98]>;
def V35 : TPCReg<"V35", 35>, DwarfRegNum<[99]>;
def V36 : TPCReg<"V36", 36>, DwarfRegNum<[100]>;
def V37 : TPCReg<"V37", 37>, DwarfRegNum<[101]>;
def V38 : TPCReg<"V38", 38>, DwarfRegNum<[102]>;
def V39 : TPCReg<"V39", 39>, DwarfRegNum<[103]>;

def LFSR           : TPCReg<"LFSR"          , 40>, DwarfRegNum<[104]>;
def LFSR_NO_CHANGE : TPCReg<"LFSR_NO_CHANGE", 41>, DwarfRegNum<[105]>;
def V_LANE_ID_32   : TPCReg<"V_LANE_ID_32"  , 42>, DwarfRegNum<[106]>;
def V_LANE_ID_16   : TPCReg<"V_LANE_ID_16"  , 43>, DwarfRegNum<[107]>;
def V_LANE_ID_8    : TPCReg<"V_LANE_ID_8"   , 44>, DwarfRegNum<[108]>;


// IRF Registers
def I0  : TPCReg<"I0",  0 >, DwarfRegNum<[128]>;
def I1  : TPCReg<"I1",  1 >, DwarfRegNum<[129]>;
def I2  : TPCReg<"I2",  2 >, DwarfRegNum<[130]>;
def I3  : TPCReg<"I3",  3 >, DwarfRegNum<[131]>;
def I4  : TPCReg<"I4",  4 >, DwarfRegNum<[132]>;
def I5  : TPCReg<"I5",  5 >, DwarfRegNum<[133]>;
def I6  : TPCReg<"I6",  6 >, DwarfRegNum<[134]>;
def I7  : TPCReg<"I7",  7 >, DwarfRegNum<[135]>;
def I8  : TPCReg<"I8",  8 >, DwarfRegNum<[136]>;
def I9  : TPCReg<"I9",  9 >, DwarfRegNum<[137]>;
def I10 : TPCReg<"I10", 10>, DwarfRegNum<[138]>;
def I11 : TPCReg<"I11", 11>, DwarfRegNum<[139]>;
def I12 : TPCReg<"I12", 12>, DwarfRegNum<[140]>;
def I13 : TPCReg<"I13", 13>, DwarfRegNum<[141]>;
def I14 : TPCReg<"I14", 14>, DwarfRegNum<[142]>;
def I15 : TPCReg<"I15", 15>, DwarfRegNum<[143]>;
def I16 : TPCReg<"I16", 16>, DwarfRegNum<[144]>;
def I17 : TPCReg<"I17", 17>, DwarfRegNum<[145]>;
def I18 : TPCReg<"I18", 18>, DwarfRegNum<[146]>;
def I19 : TPCReg<"I19", 19>, DwarfRegNum<[147]>;
def I20 : TPCReg<"I20", 20>, DwarfRegNum<[148]>;
def I21 : TPCReg<"I21", 21>, DwarfRegNum<[149]>;
def I22 : TPCReg<"I22", 22>, DwarfRegNum<[150]>;
def I23 : TPCReg<"I23", 23>, DwarfRegNum<[151]>;
def I24 : TPCReg<"I24", 24>, DwarfRegNum<[152]>;
def I25 : TPCReg<"I25", 25>, DwarfRegNum<[153]>;
def I26 : TPCReg<"I26", 26>, DwarfRegNum<[154]>;
def I27 : TPCReg<"I27", 27>, DwarfRegNum<[155]>;
def I28 : TPCReg<"I28", 28>, DwarfRegNum<[156]>;
def I29 : TPCReg<"I29", 29>, DwarfRegNum<[157]>;
def I30 : TPCReg<"I30", 30>, DwarfRegNum<[158]>;
def I31 : TPCReg<"I31", 31>, DwarfRegNum<[159]>;
def I32 : TPCReg<"I32", 32>, DwarfRegNum<[274]>;
def I33 : TPCReg<"I33", 33>, DwarfRegNum<[275]>;
def I34 : TPCReg<"I34", 34>, DwarfRegNum<[276]>;
def I35 : TPCReg<"I35", 35>, DwarfRegNum<[277]>;
def I36 : TPCReg<"I36", 36>, DwarfRegNum<[278]>;
def I37 : TPCReg<"I37", 37>, DwarfRegNum<[279]>;
def I38 : TPCReg<"I38", 38>, DwarfRegNum<[280]>;
def I39 : TPCReg<"I39", 39>, DwarfRegNum<[290]>;
def I40 : TPCReg<"I40", 40>, DwarfRegNum<[291]>;
def I41 : TPCReg<"I41", 41>, DwarfRegNum<[292]>;
def I42 : TPCReg<"I42", 42>, DwarfRegNum<[293]>;
def I43 : TPCReg<"I43", 43>, DwarfRegNum<[294]>;
def I44 : TPCReg<"I44", 44>, DwarfRegNum<[295]>;
def I45 : TPCReg<"I45", 45>, DwarfRegNum<[296]>;
def I46 : TPCReg<"I46", 46>, DwarfRegNum<[297]>;
def I47 : TPCReg<"I47", 47>, DwarfRegNum<[298]>;
def I48 : TPCReg<"I48", 48>, DwarfRegNum<[299]>;
def I49 : TPCReg<"I49", 49>, DwarfRegNum<[300]>;
def I50 : TPCReg<"I50", 50>, DwarfRegNum<[301]>;
def I51 : TPCReg<"I51", 51>, DwarfRegNum<[302]>;
def I52 : TPCReg<"I52", 52>, DwarfRegNum<[303]>;
def I53 : TPCReg<"I53", 53>, DwarfRegNum<[304]>;
def I54 : TPCReg<"I54", 54>, DwarfRegNum<[305]>;
def I55 : TPCReg<"I55", 55>, DwarfRegNum<[306]>;
def I56 : TPCReg<"I56", 56>, DwarfRegNum<[307]>;
def I57 : TPCReg<"I57", 57>, DwarfRegNum<[308]>;
def I58 : TPCReg<"I58", 58>, DwarfRegNum<[309]>;
def I59 : TPCReg<"I59", 59>, DwarfRegNum<[310]>;
def I60 : TPCReg<"I60", 60>, DwarfRegNum<[311]>;
def I61 : TPCReg<"I61", 61>, DwarfRegNum<[312]>;
def I62 : TPCReg<"I62", 62>, DwarfRegNum<[313]>;
def I63 : TPCReg<"I63", 63>, DwarfRegNum<[314]>;


// SPRF Registers
def SP0   : TPCReg<"SP0",  0 >, DwarfRegNum<[160]>;
def SP1   : TPCReg<"SP1",  1 >, DwarfRegNum<[161]>;
def SP2   : TPCReg<"SP2",  2 >, DwarfRegNum<[162]>;
def SP3   : TPCReg<"SP3",  3 >, DwarfRegNum<[163]>;
def SP4   : TPCReg<"SP4",  4 >, DwarfRegNum<[164]>;
def SP5   : TPCReg<"SP5",  5 >, DwarfRegNum<[165]>;
def SP6   : TPCReg<"SP6",  6 >, DwarfRegNum<[166]>;
def SP7   : TPCReg<"SP7",  7 >, DwarfRegNum<[167]>;
def SP8   : TPCReg<"SP8",  8 >, DwarfRegNum<[168]>;
def SP9   : TPCReg<"SP9",  9 >, DwarfRegNum<[169]>;
def SP10  : TPCReg<"SP10", 10>, DwarfRegNum<[170]>;
def SP11  : TPCReg<"SP11", 11>, DwarfRegNum<[171]>;
def SP12  : TPCReg<"SP12", 12>, DwarfRegNum<[172]>;
def SP13  : TPCReg<"SP13", 13>, DwarfRegNum<[173]>;
def SP14  : TPCReg<"SP14", 14>, DwarfRegNum<[174]>;
def SP15  : TPCReg<"SP15", 15>, DwarfRegNum<[175]>;


// VPRF Registers
def VP0   : TPCReg<"VP0",  0 >, DwarfRegNum<[176]>;
def VP1   : TPCReg<"VP1",  1 >, DwarfRegNum<[177]>;
def VP2   : TPCReg<"VP2",  2 >, DwarfRegNum<[178]>;
def VP3   : TPCReg<"VP3",  3 >, DwarfRegNum<[179]>;
def VP4   : TPCReg<"VP4",  4 >, DwarfRegNum<[180]>;
def VP5   : TPCReg<"VP5",  5 >, DwarfRegNum<[181]>;
def VP6   : TPCReg<"VP6",  6 >, DwarfRegNum<[182]>;
def VP7   : TPCReg<"VP7",  7 >, DwarfRegNum<[183]>;
def VP8   : TPCReg<"VP8",  8 >, DwarfRegNum<[184]>;
def VP9   : TPCReg<"VP9",  9 >, DwarfRegNum<[185]>;
def VP10  : TPCReg<"VP10", 10>, DwarfRegNum<[168]>;
def VP11  : TPCReg<"VP11", 11>, DwarfRegNum<[178]>;
def VP12  : TPCReg<"VP12", 12>, DwarfRegNum<[188]>;
def VP13  : TPCReg<"VP13", 13>, DwarfRegNum<[189]>;
def VP14  : TPCReg<"VP14", 14>, DwarfRegNum<[190]>;
def VP15  : TPCReg<"VP15", 15>, DwarfRegNum<[191]>;


// ADRF Registers
def AD0  : TPCReg<"AD0",  0>,  DwarfRegNum<[192]>;
def AD1  : TPCReg<"AD1",  1>,  DwarfRegNum<[193]>;
def AD2  : TPCReg<"AD2",  2>,  DwarfRegNum<[194]>;
def AD3  : TPCReg<"AD3",  3>,  DwarfRegNum<[195]>;
def AD4  : TPCReg<"AD4",  4>,  DwarfRegNum<[196]>;
def AD5  : TPCReg<"AD5",  5>,  DwarfRegNum<[197]>;
def AD6  : TPCReg<"AD6",  6>,  DwarfRegNum<[198]>;
def AD7  : TPCReg<"AD7",  7>,  DwarfRegNum<[199]>;
def AD8  : TPCReg<"AD8",  8>,  DwarfRegNum<[315]>;
def AD9  : TPCReg<"AD9",  9>,  DwarfRegNum<[316]>;
def AD10 : TPCReg<"AD10", 10>, DwarfRegNum<[317]>;
def AD11 : TPCReg<"AD11", 11>, DwarfRegNum<[318]>;
def AD12 : TPCReg<"AD12", 12>, DwarfRegNum<[319]>;
def AD13 : TPCReg<"AD13", 13>, DwarfRegNum<[320]>;
def AD14 : TPCReg<"AD14", 14>, DwarfRegNum<[321]>;
def AD15 : TPCReg<"AD15", 15>, DwarfRegNum<[322]>;


// ZRF registers. Fictious registers for implementing pairs.
let SubRegIndices = [sub_s0, sub_s1], CoveredBySubRegs = 1 in {
def Z0  : TPCReg<"Z0",  0,  [S0, S1]>;
def Z2  : TPCReg<"Z2",  2,  [S2, S3]>;
def Z4  : TPCReg<"Z4",  4,  [S4, S5]>;
def Z6  : TPCReg<"Z6",  6,  [S6, S7]>;
def Z8  : TPCReg<"Z8",  8,  [S8, S9]>;
def Z10 : TPCReg<"Z10", 10, [S10, S11]>;
def Z12 : TPCReg<"Z12", 12, [S12, S13]>;
def Z14 : TPCReg<"Z14", 14, [S14, S15]>;
def Z16 : TPCReg<"Z16", 16, [S16, S17]>;
def Z18 : TPCReg<"Z18", 18, [S18, S19]>;
def Z20 : TPCReg<"Z20", 20, [S20, S21]>;
def Z22 : TPCReg<"Z22", 22, [S22, S23]>;
def Z24 : TPCReg<"Z24", 24, [S24, S25]>;
def Z26 : TPCReg<"Z26", 26, [S26, S27]>;
def Z28 : TPCReg<"Z28", 28, [S28, S29]>;
def Z30 : TPCReg<"Z30", 30, [S30, S31]>;
def Z32 : TPCReg<"Z32", 32, [S32, S33]>;
def Z34 : TPCReg<"Z34", 34, [S34, S35]>;
def Z36 : TPCReg<"Z36", 36, [S36, S37]>;
def Z38 : TPCReg<"Z38", 38, [S38, S39]>;
def Z40 : TPCReg<"Z40", 40, [S40, S41]>;
def Z42 : TPCReg<"Z42", 42, [S42, S43]>;
def Z44 : TPCReg<"Z44", 44, [S44, S45]>;
def Z46 : TPCReg<"Z46", 46, [S46, S47]>;
def Z48 : TPCReg<"Z48", 48, [S48, S49]>;
def Z50 : TPCReg<"Z50", 50, [S50, S51]>;
def Z52 : TPCReg<"Z52", 52, [S52, S53]>;
def Z54 : TPCReg<"Z54", 54, [S54, S55]>;
def Z56 : TPCReg<"Z56", 56, [S56, S57]>;
def Z58 : TPCReg<"Z58", 58, [S58, S59]>;
def Z60 : TPCReg<"Z60", 60, [S60, S61]>;
def Z62 : TPCReg<"Z62", 62, [S62, S63]>;
}

// DRF registers. Fictious registers for implementing pairs.
let SubRegIndices = [sub_0, sub_1], CoveredBySubRegs = 1 in {
def D0  : TPCReg<"D0",  0,  [V0, V1]>;
def D2  : TPCReg<"D2",  2,  [V2, V3]>;
def D4  : TPCReg<"D4",  4,  [V4, V5]>;
def D6  : TPCReg<"D6",  6,  [V6, V7]>;
def D8  : TPCReg<"D8",  8,  [V8, V9]>;
def D10 : TPCReg<"D10", 10, [V10, V11]>;
def D12 : TPCReg<"D12", 12, [V12, V13]>;
def D14 : TPCReg<"D14", 14, [V14, V15]>;
def D16 : TPCReg<"D16", 16, [V16, V17]>;
def D18 : TPCReg<"D18", 18, [V18, V19]>;
def D20 : TPCReg<"D20", 20, [V20, V21]>;
def D22 : TPCReg<"D22", 22, [V22, V23]>;
def D24 : TPCReg<"D24", 24, [V24, V25]>;
def D26 : TPCReg<"D26", 26, [V26, V27]>;
def D28 : TPCReg<"D28", 28, [V28, V29]>;
def D30 : TPCReg<"D30", 30, [V30, V31]>;
def D32 : TPCReg<"D32", 32, [V32, V33]>;
def D34 : TPCReg<"D34", 34, [V34, V35]>;
def D36 : TPCReg<"D36", 36, [V36, V37]>;
def D38 : TPCReg<"D38", 38, [V38, V39]>;
}


// ARF Registers. Fictious registers for implementing quads.
let SubRegIndices = [sub_0, sub_1, sub_2, sub_3], CoveredBySubRegs = 1 in {
def A0  : TPCReg<"A0",  0,  [V0 , V1 , V2 , V3 ]>;
def A4  : TPCReg<"A4",  4,  [V4 , V5 , V6 , V7 ]>;
def A8  : TPCReg<"A8",  8,  [V8 , V9 , V10, V11]>;
def A12 : TPCReg<"A12", 12, [V12, V13, V14, V15]>;
def A16 : TPCReg<"A16", 16, [V16, V17, V18, V19]>;
def A20 : TPCReg<"A20", 20, [V20, V21, V22, V23]>;
def A24 : TPCReg<"A24", 24, [V24, V25, V26, V27]>;
def A28 : TPCReg<"A28", 28, [V28, V29, V30, V31]>;
def A32 : TPCReg<"A32", 32, [V32, V33, V34, V35]>;
def A36 : TPCReg<"A36", 36, [V36, V37, V38, V39]>;
}


// HW registers.
def PC             : TPCReg<"PC"            , 0 >, DwarfRegNum<[208]>;
let AltNames=["STEP_REG"] in {
def DIV_STEP       : TPCReg<"DIV_STEP"      , 1>,  DwarfRegNum<[209]>;
}
def S_CARRY        : TPCReg<"S_CARRY"       , 2>,  DwarfRegNum<[210]>;
def V_CARRY        : TPCReg<"V_CARRY"       , 3>,  DwarfRegNum<[211]>;
def M0             : TPCReg<"M0"            , 4>,  DwarfRegNum<[212]>;
def M1             : TPCReg<"M1"            , 5>,  DwarfRegNum<[213]>;
def M2             : TPCReg<"M2"            , 6>,  DwarfRegNum<[214]>;
def M3             : TPCReg<"M3"            , 7>,  DwarfRegNum<[215]>;
def LD_TNSR_ID_REG : TPCReg<"LD_TNSR_ID_REG", 8>,  DwarfRegNum<[216]>;
def ST_TNSR_ID_REG : TPCReg<"ST_TNSR_ID_REG", 9>,  DwarfRegNum<[217]>;
def ST_RMW_REG     : TPCReg<"ST_RMW_REG"    , 10>, DwarfRegNum<[218]>;
def LD_PARTIAL_REG : TPCReg<"LD_PARTIAL_REG", 11>, DwarfRegNum<[219]>;
def ST_PARTIAL_REG : TPCReg<"ST_PARTIAL_REG", 12>, DwarfRegNum<[220]>;
def ZP_REG         : TPCReg<"ZP_REG"        , 13>, DwarfRegNum<[221]>;

def SQZ_CNTR0_LO   : TPCReg<"SQZ_CNTR0_LO"  , 16>, DwarfRegNum<[222]>;
def SQZ_CNTR0_HI   : TPCReg<"SQZ_CNTR0_HI"  , 17>, DwarfRegNum<[223]>;
def SQZ_CNTR1_LO   : TPCReg<"SQZ_CNTR1_LO"  , 18>, DwarfRegNum<[224]>;
def SQZ_CNTR1_HI   : TPCReg<"SQZ_CNTR1_HI"  , 19>, DwarfRegNum<[225]>;
def SQZ_CNTR2_LO   : TPCReg<"SQZ_CNTR2_LO"  , 20>, DwarfRegNum<[226]>;
def SQZ_CNTR2_HI   : TPCReg<"SQZ_CNTR2_HI"  , 21>, DwarfRegNum<[227]>;
def SQZ_CNTR3_LO   : TPCReg<"SQZ_CNTR3_LO"  , 22>, DwarfRegNum<[228]>;
def SQZ_CNTR3_HI   : TPCReg<"SQZ_CNTR3_HI"  , 23>, DwarfRegNum<[229]>;
def SQZ_CNTR4_LO   : TPCReg<"SQZ_CNTR4_LO"  , 24>, DwarfRegNum<[230]>;
def SQZ_CNTR4_HI   : TPCReg<"SQZ_CNTR4_HI"  , 25>, DwarfRegNum<[231]>;
def SQZ_CNTR5_LO   : TPCReg<"SQZ_CNTR5_LO"  , 26>, DwarfRegNum<[232]>;
def SQZ_CNTR5_HI   : TPCReg<"SQZ_CNTR5_HI"  , 27>, DwarfRegNum<[233]>;
def SQZ_CNTR6_LO   : TPCReg<"SQZ_CNTR6_LO"  , 28>, DwarfRegNum<[234]>;
def SQZ_CNTR6_HI   : TPCReg<"SQZ_CNTR6_HI"  , 29>, DwarfRegNum<[235]>;
def SQZ_CNTR7_LO   : TPCReg<"SQZ_CNTR7_LO"  , 30>, DwarfRegNum<[236]>;
def SQZ_CNTR7_HI   : TPCReg<"SQZ_CNTR7_HI"  , 31>, DwarfRegNum<[237]>;

let SubRegIndices = [sub_0, sub_1], CoveredBySubRegs = 1 in {
def SQZ_CNTR0      : TPCReg<"SQZ_CNTR0", 0, [SQZ_CNTR0_LO, SQZ_CNTR0_HI]>, DwarfRegNum<[238]>;
def SQZ_CNTR1      : TPCReg<"SQZ_CNTR1", 1, [SQZ_CNTR1_LO, SQZ_CNTR1_HI]>, DwarfRegNum<[239]>;
def SQZ_CNTR2      : TPCReg<"SQZ_CNTR2", 2, [SQZ_CNTR2_LO, SQZ_CNTR2_HI]>, DwarfRegNum<[240]>;
def SQZ_CNTR3      : TPCReg<"SQZ_CNTR3", 3, [SQZ_CNTR3_LO, SQZ_CNTR3_HI]>, DwarfRegNum<[241]>;
def SQZ_CNTR4      : TPCReg<"SQZ_CNTR4", 4, [SQZ_CNTR4_LO, SQZ_CNTR4_HI]>, DwarfRegNum<[242]>;
def SQZ_CNTR5      : TPCReg<"SQZ_CNTR5", 5, [SQZ_CNTR5_LO, SQZ_CNTR5_HI]>, DwarfRegNum<[243]>;
def SQZ_CNTR6      : TPCReg<"SQZ_CNTR6", 6, [SQZ_CNTR6_LO, SQZ_CNTR6_HI]>, DwarfRegNum<[244]>;
def SQZ_CNTR7      : TPCReg<"SQZ_CNTR7", 7, [SQZ_CNTR7_LO, SQZ_CNTR7_HI]>, DwarfRegNum<[245]>;
}

def VPU_LFSR       : TPCReg<"VPU_LFSR"      , 16>, DwarfRegNum<[246]>;
def SPU_LFSR       : TPCReg<"SPU_LFSR"      , 17>, DwarfRegNum<[247]>;
def VPU_LFSR_RO    : TPCReg<"VPU_LFSR_RO"   , 18>, DwarfRegNum<[248]>;
def SPU_LFSR_RO    : TPCReg<"SPU_LFSR_RO"   , 19>, DwarfRegNum<[249]>;
def LANE_ID_4B     : TPCReg<"LANE_ID_4B"    , 20>, DwarfRegNum<[250]>;
def LANE_ID_2B     : TPCReg<"LANE_ID_2B"    , 21>, DwarfRegNum<[251]>;
def LANE_ID_1B     : TPCReg<"LANE_ID_1B"    , 22>, DwarfRegNum<[251]>;
def THREAD_ID      : TPCReg<"THREAD_ID"     , 23>, DwarfRegNum<[252]>;
def INC_LD_DIM_REG : TPCReg<"INC_LD_DIM_REG", 14>, DwarfRegNum<[253]>;
def INC_ST_DIM_REG : TPCReg<"INC_ST_DIM_REG", 15>, DwarfRegNum<[254]>;

// The fake registers
def SPRF_TRUE      : TPCReg<"SPRF_TRUE"     , 16>;
def VPRF_TRUE      : TPCReg<"VPRF_TRUE"     , 16>;

// Register classes

def VRF  : RegisterClass<"TPC", [v256i8, v128i16, v64i32, v256f8_143, v256f8_152, v128f16, v128bf16, v64f32], 2048,
                        (add V0, V1, V2, V3, V4, V5, V6, V7, V8, V9, V10, V11, V12, V13, V14, V15,
                             V16, V17, V18, V19, V20, V21, V22, V23, V24, V25, V26, V27, V28, V29, V30, V31,
                             V32, V33, V34, V35, V36, V37, V38, V39,
                             LFSR, LFSR_NO_CHANGE, V_LANE_ID_32, V_LANE_ID_16, V_LANE_ID_8)>;

def ARF  : RegisterClass<"TPC", [v256i32, v256f32], 2048,
                        (add A0, A4, A8, A12, A16, A20, A24, A28, A32, A36)> {
  let CopyCost = 4;
  let AllocationPriority = 2;
}

def DRF  : RegisterClass<"TPC", [v128i32, v128f32, v256i16, v512f8_143, v512f8_152, v256f16, v256bf16, v512i8], 2048,
                        (add D0, D2, D4, D6, D8, D10, D12, D14, D16, D18, D20,
                             D22, D24, D26, D28, D30, D32, D34, D36, D38)> {
  let CopyCost = 2;
  let AllocationPriority = 1;
}

def VPRF : RegisterClass<"TPC", [v256i1, v128i1, v64i1], 2048,
                         (add (sequence "VP%u", 0, 15), VPRF_TRUE)> {
  let Size = 2048;
}

def SPRF : RegisterClass<"TPC", [i1], 32, (add (sequence "SP%u", 0, 15),
                                               SPRF_TRUE)> {
  let Size = 32;
}

def SRF : RegisterClass<"TPC", [f32, f16, bf16, f8_143, f8_152, i32, i16, i8], 32,
                        (add S0, S1, S2, S3, S4, S5, S6, S7, S8, S9, S10, S11, S12, S13, S14, S15,
                             S16, S17, S18, S19, S20, S21, S22, S23, S24, S25, S26, S27, S28, S29, S30, S31,
                             S32, S33, S34, S35, S36, S37, S38, S39, S40, S41, S42, S43, S44, S45, S46,
                             S47, S48, S49, S50, S51, S52, S53, S54, S55, S56, S57, S58, S59, S60, S61,
                             S62, S63,
                             S_LFSR, S_LFSR_NO_CHANGE)>;

// SRF subclasses for registers that can keep tensor number.
def TnsrRegLd : RegisterClass<"TPC", [i8], 32, (add S27)>;
def TnsrRegSt : RegisterClass<"TPC", [i8], 32, (add S28)>;

// SRF subclasses for registers that can keep offset+size in tensor operations.
def OffsSizeRegLd : RegisterClass<"TPC", [i32], 32, (add S30)>;
def OffsSizeRegSt : RegisterClass<"TPC", [i32], 32, (add S31)>;

// SRF subclass for register that keeps RMW info in tensor save.
def RMWReg : RegisterClass<"TPC", [i32], 8, (add S29)>;


def ZRF  : RegisterClass<"TPC", [v2i32, v2i16, v2i8], 32,
                        (add Z0, Z2, Z4, Z6, Z8, Z10, Z12, Z14, Z16, Z18, Z20,
                             Z22, Z24, Z26, Z28, Z30, Z32, Z34, Z36, Z38, Z40,
                             Z42, Z44, Z46, Z48, Z50, Z52, Z54, Z56, Z58, Z60,
                             Z62)> {
  let CopyCost = 2;
  let AllocationPriority = 1;
}

def IRF  : RegisterClass<"TPC", [v5i32], 32, (sequence "I%u", 0, 63)>;

// A hack to avoid introducing 92bit type
def ADRF : RegisterClass <"TPC", [i64], 64, (sequence "AD%u", 0, 15)>;


// HW register classes.
def HWSqzCntrSub    : RegisterClass<"TPC", [i32], 32,
                                    (add SQZ_CNTR0_LO, SQZ_CNTR0_HI,
                                         SQZ_CNTR1_LO, SQZ_CNTR1_HI,
                                         SQZ_CNTR2_LO, SQZ_CNTR2_HI,
                                         SQZ_CNTR3_LO, SQZ_CNTR3_HI,
                                         SQZ_CNTR4_LO, SQZ_CNTR4_HI,
                                         SQZ_CNTR5_LO, SQZ_CNTR5_HI,
                                         SQZ_CNTR6_LO, SQZ_CNTR6_HI,
                                         SQZ_CNTR7_LO, SQZ_CNTR7_HI)> {
  let CopyCost = -1;
  let AllocationPriority = 1;
}
def HWSqzCntr       : RegisterClass<"TPC", [i64], 64,
                                    (add SQZ_CNTR0, SQZ_CNTR1,
                                         SQZ_CNTR2, SQZ_CNTR3,
                                         SQZ_CNTR4, SQZ_CNTR5,
                                         SQZ_CNTR6, SQZ_CNTR7)> {
  let CopyCost = 4;
  let AllocationPriority = 1;
}

let CopyCost = -1 in {
  def HWPC            : RegisterClass<"TPC", [i32],    32,   (add PC)>;
  def MRF             : RegisterClass<"TPC", [i32],    32,   (add M0, M1, M2, M3)>;
  def HWTnsrRegLd     : RegisterClass<"TPC", [i8],     32,   (add LD_TNSR_ID_REG)>;
  def HWTnsrRegSt     : RegisterClass<"TPC", [i8],     32,   (add ST_TNSR_ID_REG)>;
  def HWOffsSizeRegLd : RegisterClass<"TPC", [i32],    32,   (add LD_PARTIAL_REG)>;
  def HWOffsSizeRegSt : RegisterClass<"TPC", [i32],    32,   (add ST_PARTIAL_REG)>;
  def HWRMWReg        : RegisterClass<"TPC", [i32],    32,   (add ST_RMW_REG)>;
  def HWDivStep       : RegisterClass<"TPC", [i32],    32,   (add DIV_STEP)>;
  def HWSCarry        : RegisterClass<"TPC", [i1],     32,   (add S_CARRY)>;
  def HWVCarry        : RegisterClass<"TPC", [v256i1], 2048, (add V_CARRY)>;
  def HWZPReg         : RegisterClass<"TPC", [v256i8], 2048, (add ZP_REG)>;
  def HVRF            : RegisterClass<"TPC", [v256i8, v128i16, v64i32,
                                              v256f8_143, v256f8_152, v128f16,
                                              v128bf16, v64f32], 2048,
                                      (add ZP_REG, VPU_LFSR, VPU_LFSR_RO,
                                           LANE_ID_4B, LANE_ID_2B, LANE_ID_1B)>;
  def HVRFPriorDoron1   : RegisterClass<"TPC", [v256i8], 2048,
                                      (sub HVRF,
                                           VPU_LFSR, VPU_LFSR_RO,
                                           LANE_ID_4B, LANE_ID_2B, LANE_ID_1B)>;
  def HVRFDoron1    : RegisterClass<"TPC", [v256i8, v128i16, v64i32,
                                              v256f8_143, v256f8_152, v128f16,
                                              v128bf16, v64f32], 2048,
                                      (add HVRF)>;
  // Registers available for reading and writing
  def HVRFDoron1RW  : RegisterClass<"TPC", [v256i8, v128i16, v64i32,
                                              v256f8_143, v256f8_152,
                                              v128f16, v128bf16, v64f32], 2048,
                                      (sub HVRFDoron1, VPU_LFSR_RO,
                                           LANE_ID_4B, LANE_ID_2B, LANE_ID_1B)>;
  def HVPRF           : RegisterClass<"TPC", [v256i1], 2048, (add V_CARRY)>;
  def HSRF            : RegisterClass<"TPC", [f32, f16, bf16, f8_143, f8_152,
                                              i32, i16, i8],  32,
                                      (add PC, DIV_STEP, M0, M1, M2, M3,
                                           LD_TNSR_ID_REG, ST_TNSR_ID_REG,
                                           ST_RMW_REG,
                                           LD_PARTIAL_REG, ST_PARTIAL_REG,
                                           (sequence "SQZ_CNTR%u_LO", 0, 7),
                                           (sequence "SQZ_CNTR%u_HI", 0, 7),
                                           INC_LD_DIM_REG, INC_ST_DIM_REG,
                                           SPU_LFSR, SPU_LFSR_RO,
                                           THREAD_ID)>;
 def HSRFPriorDoron1    : RegisterClass<"TPC", [i8, i32],  32,
                                      (sub HSRF,
                                           INC_LD_DIM_REG, INC_ST_DIM_REG,
                                           SPU_LFSR, SPU_LFSR_RO, THREAD_ID)>;
 def HSRFDoron1     : RegisterClass<"TPC", [f32, f16, bf16, f8_143, f8_152,
                                              i32, i16, i8],  32,
                                      (sub HSRF, DIV_STEP)>;
 // Registers available for reading and writing
 def HSRFDoron1RW   : RegisterClass<"TPC", [f32, f16, bf16, f8_143, f8_152,
                                              i32, i16, i8],  32,
                                      (sub HSRFDoron1,
                                           SPU_LFSR_RO, THREAD_ID)>;

  def HSPRF           : RegisterClass<"TPC", [i1],     32,   (add S_CARRY)>;
  def HWVpuLFSR       : RegisterClass<"TPC", [v256i8, v128i16, v64i32,
                                              v256f8_143, v256f8_152, v128f16,
                                              v128bf16, v64f32], 2048,
                                      (add VPU_LFSR)>;
  def HWVpuLFSRRo     : RegisterClass<"TPC", [v256i8, v128i16, v64i32,
                                              v256f8_143, v256f8_152, v128f16,
                                              v128bf16, v64f32], 2048,
                                      (add VPU_LFSR_RO)>;
  def HWSpuLFSR       : RegisterClass<"TPC", [f32, f16, bf16, f8_143, f8_152,
                                              i32, i16, i8], 32,
                                      (add SPU_LFSR)>;
  def HWSpuLFSRRo     : RegisterClass<"TPC", [f32, f16, bf16, f8_143, f8_152,
                                              i32, i16, i8], 32,
                                      (add SPU_LFSR_RO)>;
  def HWLaneId        : RegisterClass<"TPC", [v256i8, v128i16, v64i32], 2048,
                                      (add LANE_ID_4B, LANE_ID_2B, LANE_ID_1B)>;
  def HWThreadId      : RegisterClass<"TPC", [i32], 32, (add THREAD_ID)>;
  
  def HWIncDimRegLD   : RegisterClass<"TPC", [i32], 32, (add INC_LD_DIM_REG)>;
  
  def HWIncDimRegST   : RegisterClass<"TPC", [i32], 32, (add INC_ST_DIM_REG)>;
}


def CSR_AllRegs : CalleeSavedRegs<(add (sequence "V%u", 0, 39),
                                       (sequence "A%u", 0, 28, 4),
                                       (sequence "D%u", 0, 38, 2),
                                       (sequence "S%u", 0, 35),
                                       (sequence "I%u", 0, 31),
                                       (sequence "AD%u", 0, 7),
                                       (sequence "SP%u", 0, 15),
                                       (sequence "VP%u", 0, 15),
                                       (sequence "M%u", 0, 3))>;

def CSR_NoRegs : CalleeSavedRegs<(add)>;
