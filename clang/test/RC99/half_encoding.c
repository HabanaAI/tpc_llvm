// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -emit-llvm -O1 -target-cpu goya2 %s -o - | FileCheck %s

void main(int ind) {
  half __local *res = (half __local *) ind;
  res[0] = 0.000000h;
  res[1] = 1.000000h;
  res[2] = 2.000000h;
  res[3] = 3.000000h;
  res[4] = 4.000000h;
  res[5] = 5.000000h;
  res[6] = 6.000000h;
  res[7] = 7.000000h;
  res[8] = 8.000000h;
  res[9] = 9.000000h;
  res[10] = 10.000000h;
  res[11] = 11.000000h;
  res[12] = 12.000000h;
  res[13] = 13.000000h;
  res[14] = 14.000000h;
  res[15] = 15.000000h;
  res[16] = 16.000000h;
  res[17] = 17.000000h;
  res[18] = 18.000000h;
  res[19] = 19.000000h;
  res[20] = 20.000000h;
  res[21] = 21.000000h;
  res[22] = 22.000000h;
  res[23] = 23.000000h;
  res[24] = 24.000000h;
  res[25] = 25.000000h;
  res[26] = 26.000000h;
  res[27] = 27.000000h;
  res[28] = 28.000000h;
  res[29] = 29.000000h;
  res[30] = 30.000000h;
  res[31] = 31.000000h;
  res[32] = 32.000000h;
  res[33] = 33.000000h;
  res[34] = 34.000000h;
  res[35] = 35.000000h;
  res[36] = 36.000000h;
  res[37] = 37.000000h;
  res[38] = 38.000000h;
  res[39] = 39.000000h;
  res[40] = 40.000000h;
  res[41] = 41.000000h;
  res[42] = 42.000000h;
  res[43] = 43.000000h;
  res[44] = 44.000000h;
  res[45] = 45.000000h;
  res[46] = 46.000000h;
  res[47] = 47.000000h;
  res[48] = 48.000000h;
  res[49] = 49.000000h;
  res[50] = 50.000000h;
  res[51] = 51.000000h;
  res[52] = 52.000000h;
  res[53] = 53.000000h;
  res[54] = 54.000000h;
  res[55] = 55.000000h;
  res[56] = 56.000000h;
  res[57] = 57.000000h;
  res[58] = 58.000000h;
  res[59] = 59.000000h;
  res[60] = 60.000000h;
  res[61] = 61.000000h;
  res[62] = 62.000000h;
  res[63] = 63.000000h;
  res[64] = 64.000000h;
  res[65] = 65.000000h;
  res[66] = 66.000000h;
  res[67] = 67.000000h;
  res[68] = 68.000000h;
  res[69] = 69.000000h;
  res[70] = 70.000000h;
  res[71] = 71.000000h;
  res[72] = 72.000000h;
  res[73] = 73.000000h;
  res[74] = 74.000000h;
  res[75] = 75.000000h;
  res[76] = 76.000000h;
  res[77] = 77.000000h;
  res[78] = 78.000000h;
  res[79] = 79.000000h;
  res[80] = 80.000000h;
  res[81] = 81.000000h;
  res[82] = 82.000000h;
  res[83] = 83.000000h;
  res[84] = 84.000000h;
  res[85] = 85.000000h;
  res[86] = 86.000000h;
  res[87] = 87.000000h;
  res[88] = 88.000000h;
  res[89] = 89.000000h;
  res[90] = 90.000000h;
  res[91] = 91.000000h;
  res[92] = 92.000000h;
  res[93] = 93.000000h;
  res[94] = 94.000000h;
  res[95] = 95.000000h;
  res[96] = 96.000000h;
  res[97] = 97.000000h;
  res[98] = 98.000000h;
  res[99] = 99.000000h;
  res[100] = 100.000000h;
  res[101] = 101.000000h;
  res[102] = 102.000000h;
  res[103] = 103.000000h;
  res[104] = 104.000000h;
  res[105] = 105.000000h;
  res[106] = 106.000000h;
  res[107] = 107.000000h;
  res[108] = 108.000000h;
  res[109] = 109.000000h;
  res[110] = 110.000000h;
  res[111] = 111.000000h;
  res[112] = 112.000000h;
  res[113] = 113.000000h;
  res[114] = 114.000000h;
  res[115] = 115.000000h;
  res[116] = 116.000000h;
  res[117] = 117.000000h;
  res[118] = 118.000000h;
  res[119] = 119.000000h;
  res[120] = 120.000000h;
  res[121] = 121.000000h;
  res[122] = 122.000000h;
  res[123] = 123.000000h;
  res[124] = 124.000000h;
  res[125] = 125.000000h;
  res[126] = 126.000000h;
  res[127] = 127.000000h;
}
// CHECK: store half 0xH0000
// CHECK: store half 0xH3C00
// CHECK: store half 0xH4000
// CHECK: store half 0xH4200
// CHECK: store half 0xH4400
// CHECK: store half 0xH4500
// CHECK: store half 0xH4600
// CHECK: store half 0xH4700
// CHECK: store half 0xH4800
// CHECK: store half 0xH4880
// CHECK: store half 0xH4900
// CHECK: store half 0xH4980
// CHECK: store half 0xH4A00
// CHECK: store half 0xH4A80
// CHECK: store half 0xH4B00
// CHECK: store half 0xH4B80
// CHECK: store half 0xH4C00
// CHECK: store half 0xH4C40
// CHECK: store half 0xH4C80
// CHECK: store half 0xH4CC0
// CHECK: store half 0xH4D00
// CHECK: store half 0xH4D40
// CHECK: store half 0xH4D80
// CHECK: store half 0xH4DC0
// CHECK: store half 0xH4E00
// CHECK: store half 0xH4E40
// CHECK: store half 0xH4E80
// CHECK: store half 0xH4EC0
// CHECK: store half 0xH4F00
// CHECK: store half 0xH4F40
// CHECK: store half 0xH4F80
// CHECK: store half 0xH4FC0
// CHECK: store half 0xH5000
// CHECK: store half 0xH5020
// CHECK: store half 0xH5040
// CHECK: store half 0xH5060
// CHECK: store half 0xH5080
// CHECK: store half 0xH50A0
// CHECK: store half 0xH50C0
// CHECK: store half 0xH50E0
// CHECK: store half 0xH5100
// CHECK: store half 0xH5120
// CHECK: store half 0xH5140
// CHECK: store half 0xH5160
// CHECK: store half 0xH5180
// CHECK: store half 0xH51A0
// CHECK: store half 0xH51C0
// CHECK: store half 0xH51E0
// CHECK: store half 0xH5200
// CHECK: store half 0xH5220
// CHECK: store half 0xH5240
// CHECK: store half 0xH5260
// CHECK: store half 0xH5280
// CHECK: store half 0xH52A0
// CHECK: store half 0xH52C0
// CHECK: store half 0xH52E0
// CHECK: store half 0xH5300
// CHECK: store half 0xH5320
// CHECK: store half 0xH5340
// CHECK: store half 0xH5360
// CHECK: store half 0xH5380
// CHECK: store half 0xH53A0
// CHECK: store half 0xH53C0
// CHECK: store half 0xH53E0
// CHECK: store half 0xH5400
// CHECK: store half 0xH5410
// CHECK: store half 0xH5420
// CHECK: store half 0xH5430
// CHECK: store half 0xH5440
// CHECK: store half 0xH5450
// CHECK: store half 0xH5460
// CHECK: store half 0xH5470
// CHECK: store half 0xH5480
// CHECK: store half 0xH5490
// CHECK: store half 0xH54A0
// CHECK: store half 0xH54B0
// CHECK: store half 0xH54C0
// CHECK: store half 0xH54D0
// CHECK: store half 0xH54E0
// CHECK: store half 0xH54F0
// CHECK: store half 0xH5500
// CHECK: store half 0xH5510
// CHECK: store half 0xH5520
// CHECK: store half 0xH5530
// CHECK: store half 0xH5540
// CHECK: store half 0xH5550
// CHECK: store half 0xH5560
// CHECK: store half 0xH5570
// CHECK: store half 0xH5580
// CHECK: store half 0xH5590
// CHECK: store half 0xH55A0
// CHECK: store half 0xH55B0
// CHECK: store half 0xH55C0
// CHECK: store half 0xH55D0
// CHECK: store half 0xH55E0
// CHECK: store half 0xH55F0
// CHECK: store half 0xH5600
// CHECK: store half 0xH5610
// CHECK: store half 0xH5620
// CHECK: store half 0xH5630
// CHECK: store half 0xH5640
// CHECK: store half 0xH5650
// CHECK: store half 0xH5660
// CHECK: store half 0xH5670
// CHECK: store half 0xH5680
// CHECK: store half 0xH5690
// CHECK: store half 0xH56A0
// CHECK: store half 0xH56B0
// CHECK: store half 0xH56C0
// CHECK: store half 0xH56D0
// CHECK: store half 0xH56E0
// CHECK: store half 0xH56F0
// CHECK: store half 0xH5700
// CHECK: store half 0xH5710
// CHECK: store half 0xH5720
// CHECK: store half 0xH5730
// CHECK: store half 0xH5740
// CHECK: store half 0xH5750
// CHECK: store half 0xH5760
// CHECK: store half 0xH5770
// CHECK: store half 0xH5780
// CHECK: store half 0xH5790
// CHECK: store half 0xH57A0
// CHECK: store half 0xH57B0
// CHECK: store half 0xH57C0
// CHECK: store half 0xH57D0
// CHECK: store half 0xH57E0
// CHECK: store half 0xH57F0
