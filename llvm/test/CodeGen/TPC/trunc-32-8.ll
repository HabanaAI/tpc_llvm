; RUN: llc -march=tpc -mcpu=goya2 %s -o - | FileCheck %s

; CHECK: convert.i32
; CHECK: pack.i8
; CHECK: mov_dg

target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-i16:32:32-i8:32:32-n8:16:32-f16:16:16-f32:32:32-v16:32:32-v32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

@llvm.embedded.module = private constant [5216 x i8] c"BC\C0\DE5\14\00\00\05\00\00\00b\0C0$IY\BE\E6\EE\D3>-D\012\05\00\00\00\00!\0C\00\00n\04\00\00\0B\02!\00\02\00\00\00\13\00\00\00\07\81#\91A\C8\04I\06\1029\92\01\84\0C%\05\08\19\1E\04\8Bb\80\18E\02B\92\0BB\C4\102\148\08\18K\0A2b\88H\90\14 CF\88\A5\00\192B\04I\0E\90\11#\C4PAQ\81\8C\E1\83\E5\8A\041F\06Q\18\00\00\A2\00\00\00\1Bb&\F8\FF\FF\FF\FF\01\90\88qx\07y\90\87r\18\07z`\87|h\03yx\87zp\07r(\07rh\03rH\07{H\07r(\876\98\87x\90\07zh\03s\80\876h\87p\A0\07t\00\CC!\1C\D8a\1E\CA\01 \C8!\1D\E6!\1C\C4\81\1D\CA\A1\0D\E8!\1C\D2\81\1D\DA`\1C\C2\81\1D\D8a\1E\00s\08\07v\98\87r\00\08v(\87y\98\876\80\07y(\87qH\87y(\8760\07xh\87p \07\C0\1C\C2\81\1D\E6\A1\1C\00\C2\1D\DE\A1\0D\CCA\1E\C2\A1\1D\CA\A1\0D\E0\E1\1D\D2\C1\1D\E8\A1\1C\E4\A1\0D\CA\81\1D\D2\A1\1D\00z\90\87z(\07`p\87wh\03s\90\87ph\87rh\03xx\87tp\07z(\07yh\83r`\87th\876p\87wp\876`\87r\08\07s\00\08wx\876H\07w0\87yh\03s\80\876h\87p\A0\07t\00\CC!\1C\D8a\1E\CA\01 \DC\E1\1D\DA@\1D\EA\A1\1D\E0\A1\0D\E8!\1C\C4\81\1D\CAa\1E\00s\08\07v\98\87r\00\08wx\876p\87pp\87yh\03s\80\876h\87p\A0\07t\00\CC!\1C\D8a\1E\CA\01 \DC\E1\1D\DA`\1E\D2\E1\1C\DC\A1\1C\C8\A1\0D\F4\A1\1C\E4\E1\1D\E6\A1\0D\CC\01\1E\DA\A0\1D\C2\81\1E\D0\010\87p`\87y(\07\80p\87wh\03z\90\87p\80\07xH\07w8\876h\87p\A0\07t\00\CC!\1C\D8a\1E\CA\01 \E6\81\1E\C2a\1C\D6\A1\0D\E0A\1E\DE\81\1E\CAa\1C\E8\E1\1D\E4\A1\0D\C4\A1\1E\CC\C1\1C\CAA\1E\DA`\1E\D2A\1F\CA\01\C0\03\80\A0\87p\90\87s(\07zh\83q\80\87z\00\CE\E1\1D\F2!\1C\E4\00 \E8!\1C\E4\E1\1C\CA\81\1E\DA\C0\1C\CA!\1C\E8\A1\1E\E4\A1\1C\E6\01X\83sx\87|\08\079\00\88zp\87y\08\07s(\8760\07xh\83v\08\07z@\07\C0\1C\C2\81\1D\E6\A1\1C\00\A2\1E\E6\A1\1C\DA`\1E\DE\C1\1C\E8\A1\0D\CC\81\1D\DE!\1C\E8\010\87p`\87y(\07`\83!\FC\FF\FF\FF\FF\00\B4\01 m \06\01X6\18\C4\FF\FF\FF\FF\0F\80\04P\1B\8C\22\00\16\80\0E6\18\86\00,@\B5\C18\FE\FF\FF\FF\7F\00$\80\0E6\10\C8\FF\FF\FF\FF\0F\80\B4\A1H\82 \00\91\0D\85\22\04\01\88\00I\18\00\00\06\00\00\00\13\82`\82 \0C\13\02b\C2 \14\C6\84\E0\98\10 \13\84D\01\00\00\89 \00\00%\00\00\002\22\88\09 d\85\04\13#\A4\84\04\13#\E3\84\A1\90\14\12L\8C\8C\0B\84\C4L\10\98\C1\1C\01\18\0C#\14\C0 B\10\8C\00\94` b\8E\00\14\E6\08\10:\CA0\18\08%%\08h\19F\00\91c\8E (G\13\1C@\E38\F4\94b@\10\C0\A1h\18\01D\80a\04\10q\CA15\070Q\0EU\C3\08\A0\00\94c\08\8E\0Bp\1C\C2\06\02\08\A0\80\9As\A4)\A2\84\C9\FF\91f\B2s\1D\11\D7u]\E4\0D\22x\02\81DM\01\D0D\17\00\00\13\06z\80\87q\D8\90\1A\E5\D0\06\F0\00\07z0\07r\A0\07s \07z0\07r\D0\06\F0\10\07z0\07r\A0\07s \07z0\07r\D0\06\F0 \07z0\07r\A0\07s \07z0\07r\D0\06\F00\07z`\07t\A0\07v@\07z`\07t\D0\06\E90\07r\A0\07s \07z0\07r\D0\06\E9\10\07v\A0\07s \07z0\07r\D0\06\E9\80\07z0\07r\A0\07s \07m\E0\0Ex\A0\07q`\07z0\07r\D0\06\E6\10\07v\A0\07q`\07z\10\07v\D0\06\E60\07r\A0\07s \07z0\07r\D0\06\F6\10\07v\A0\07s \07z0\07r\D0\06\F60\07r\A0\07s \07z0\07r\D0\06\F6\10\07v\00\07z0\07r\A0\07s \07m`\0FrP\07v\A0\07r\00\07t\80\07z \07p@\07x\D0\06\F6 \07p@\07x\A0\07r\00\07t\80\07z \07p@\07x\D0\06\F6@\07p\90\07v\A0\07r\00\07t\80\07z \07p@\07x\D0\06\F6\80\07q\90\07r\A0\07r\00\07t\80\07z \07p@\07x\A0\F4\80\10!\09d\C8H\11\11@#\84a\99)\A2\1A\E1\BF\ED\9F\F7\85\1D\100\05\C6\02\0C\00\00\00\00\00\00\C1\10i\22\08\00\00\02\00\00\00\00\00\00\00\08\86H\17\96\00\01 \00\00\00\00\00\00\00\000Dz\83o\01\02`\00\00\00\00\00\00\00\00`\88\E4\0A\DD\02\04\C0\00\00\00\00\00\00\00\00\C0\10i\1D\B4\07\08\80\01\00\00\00\00\00\00\00\80!RId\11\10\00\04\00\00\00\00\00\00\00\00C$\9F0\83\0A\08\80\01\00\00\00\00\00\00\00\80!\12iX\09\10\00\02\00\00\00\00\00\00\00\00C$\DD\B80 \00\0A\00\00\00\00\00\00\00\00$6\08\14\96#\00\00\C8\02\01\09\00\00\002\1E\98\14\19\11L\90\8C\09&G\C6\04C\0AF\00\08(\81B c\04\A0\00\09\0A\10\A1\00\19\00\00\00\B1\18\00\00t\00\00\003\08\80\1C\C4\E1\1Cf\14\01=\88C8\84\C3\8CB\80\07yx\07s\98q\0C\E6\00\0F\ED\10\0E\F4\80\0E3\0CB\1E\C2\C1\1D\CE\A1\1Cf0\05=\88C8\84\83\1B\CC\03=\C8C=\8C\03=\CCx\8Ctp\07{\08\07yH\87pp\07zp\03vx\87p \87\19\CC\11\0E\EC\90\0E\E10\0Fn0\0F\E3\F0\0E\F0P\0E3\10\C4\1D\DE!\1C\D8!\1D\C2a\1Ef0\89;\BC\83;\D0C9\B4\03<\BC\83<\84\03;\CC\F0\14v`\07{h\077h\87rh\077\80\87p\90\87p`\07v(\07v\F8\05vx\87w\80\87_\08\87q\18\87r\98\87y\98\81,\EE\F0\0E\EE\E0\0E\F5\C0\0E\EC0\03b\C8\A1\1C\E4\A1\1C\CC\A1\1C\E4\A1\1C\DCa\1C\CA!\1C\C4\81\1D\CAa\06\D6\90C9\C8C9\98C9\C8C9\B8\C38\94C8\88\03;\94\C3/\BC\83<\FC\82;\D4\03;\B0\C3\0C\C7i\87pX\87rp\83th\07x`\87t\18\87t\A0\87\19\CES\0F\EE\00\0F\F2P\0E\E4\90\0E\E3@\0F\E1 \0E\ECP\0E3 (\1D\DC\C1\1E\C2A\1E\D2!\1C\DC\81\1E\DC\E0\1C\E4\E1\1D\EA\01\1Ef\18Q8\B0C:\9C\83;\CCP$v`\07{h\077`\87wx\07x\98QL\F4\90\0F\F0P\0E3\1Ej\1E\CAa\1C\E8!\1D\DE\C1\1D~\01\1E\E4\A1\1C\CC!\1D\F0a\06T\85\838\CC\C3;\B0C=\D0C9\FC\C2<\E4C;\88\C3;\B0\C3\8C\C5\0A\87y\98\87w\18\87t\08\07z(\07r\98\81\5C\E3\10\0E\EC\C0\0E\E5P\0E\F30#\C1\D2A\1E\E4\E1\17\D8\E1\1D\DE\01\1E\00\00\00\00y \00\00R\00\00\00b\1EH C\88\0C\199\19$\90\91@\C6\C8\C8h\22P\08\142\9E\18\19!G\C8\90Q\14\08\F6\00\00\8Ao\00\00wchar_sizeclang version 7.0.0 (ssh://sjaiswal@gerrit.habana-labs.com:29418/tpc_clang2.git d595f2b7ab1c0c55eb0a80474d13243fbaac4fe4) (ssh://sjaiswal@gerrit.habana-labs.com:29418/tpc_llvm2.git bd3d9f175777e32c1050211eaede99bdbaf8e7bf)#\08\C02\82\0003\0CC@\CC\10\082\12\98\A0\8C\D8\D8\EC\DA\5C\DA\DE\C8\EA\D8\CA\5C\CC\D8\C2\CE\E6F\09\88T\D8\D8\EC\DA\5C\D2\C8\CA\DC\E8F\09\0A\00\00\00\A9\18\00\00\0B\00\00\00\0B\0Ar(\87w\80\07zXp\98C=\B8\C38\B0C9\D0\C3\82\E6\1C\C6\A1\0D\E8A\1E\C2\C1\1D\E6!\1D\E8!\1D\DE\C1\1D\00\D1\10\00\00\06\00\00\00\07\CC<\A4\83;\9C\03;\94\03=\A0\83<\94C8\90\C3\01\00\00\00a \00\00\16\02\00\00\13\04F,\10\00\00\00k\00\00\00d\14\02\01#\00d\14\0A\01\05H@F\01BP0\03@@\11\94A9\D0Q\02#\00\D4\CC\00\90S\03#\00\04\14 \04\1D\85@@9\22\D04\03@\D7X\A0\00\04\C2@\14\C6\81$\CA\C24\CE\03E\D2DU\D6\85e\DA\C6u\DE\07\06a \06c@\06e`\06g\80\06i\A0\06k\C0\06m\E0\06o\00\07q \07s@\07u`\07w\80\07y\A0\07{\C0\07}\E0\07\7F0\16(\80B(\88\C2(\90B)\98\C2)\A0B*\A8\C2*\B0B+\B8\C2+\C0B,\C8\C2,\D0B-\D8\C2-\E0B.\E8\C2.\F0B/\F8\C2/\80C8\88\C38\90C9\98\C39\A0C:\A8\C3:\B0C;\B8\C3;\C0C<\C8\C3<\D0C=\D8\C3=\E0C>\E8\C3>\F0C?\F8\C3?\8C\05\0A \11\12\221\12$Q\12&q\12(\91\12*\B1\12,\D1\12.\F1\120\11\1321\134Q\136q\138\91\13:\B1\13<\D1\13>\F1\13`\11\16b1\16dQ\16fq\16h\91\16j\B1\16l\D1\16n\F1\16p\11\17r1\17tQ\17vq\17x\91\17z\B1\17|\D1\17~\F1\17c\81\02h\84\86h\8C\06i\94\86i\9C\06j\A4\86j\AC\06k\B4\86k\BC\06l\C4\86l\CC\06m\D4\86m\DC\06n\E4\86n\EC\06o\F4\86o\FC\06x\84\87x\8C\07y\94\87y\9C\07z\A4\87z\AC\07{\B4\87{\BC\07|\C4\87|\CC\07}\D4\87}\DC\07~\E4\87~\EC\07\7F\F4\87\7F\FC\07y3\00\00\F10\00\00\1E\00\00\00\22G\C8\90Q\12\84\1B\00\00\00\00\C3\13\1D\00intomnipotent charSimple C/C++ TBAA_int256\00\00\13\04\A3\99 \18\CE\04\C1x&\08\06\B4!H6\0C\C8\B3l\18\0Eh\D90D\D1\B2a\80\A0e\03\B30\13\C3L\0D39\CC\B4!Q\A0\05b\A0\06r6\0CU\B5\00\00\00\003\11\00\B0\8C\C2LD\00,\A30\13\11\00\CB(\CCD\00\C02\0A3\11\00\B0\8C\C2L\04\00,\A30\13\D1\00K*\CCD4\C0\92\0A3\11\0D\B0\A4\C2LD\03,\A90\13\E1\01K*\CCD\80\01\B0\8C\C2LD\00,\A30\13\01\00\CB(\CCD\1C\C02\0A3\11\1E\B0\A4\C2L\04\05,\A90\13\01\00\CB(\CCDx\C0\92\0A3\11\1E\B0\A4\C2Q\C8\8E\18\18\06\08\82A\B2\07l\10\0C\1B\10\D5\1A\0C\C0Q\C8\8E\18\18\06\08\82A\C2\07k\10\8C\18\10\00\08\82\C1\B2\07\C3\06\84\15\0C\C0U\C8\8E\18\18\06\08\82A\E2\07m\10\8C\18\10\00\08\82\C1\C2\07\84\05\A3\84\00\86\0D\88,\18\80\C3\90\1D10\0C\10\04\83D\14\E6 \186 28\18\80\C3\90\1D10\0C\10\04\83d\14\E8  .\18c\08\01\1DX0\07\220l@p\C1\00\DC\86\EC\88\81a\80 \18$\A7\80\07\01}\C1\18C\08\F0\C0\82;\10\81a\03\E2\0B\06\E0<dG\0C\0C\03\04\C1 Y\85?\08\CECv\C4\C00@\10\0C\12V\00\85\E0<dG\0C\0C\03\04\C1 i\85P\08\CECv\C4\C00@\10\0C\12W\10\85\E0<dG\0C\0C\03\04\C1 y\05>\08\CECv\C4\C00@\10\0C\12X\08\85\E0<dG\0C\0C\03\04\C1 \89\05P\08\E8\0C\801\C7\E0\07\81(\CC1\04\A3\A0\0As\0C\01)\FC\C1\1CCP\0A\7F0\C7\10\98\02+\0C\1B\10e\10\0C\C0\91\01\B2#\06\86\01\82`\90\E0B*\04\C3\06D\19\84\C2\00\8C\18\18\06\08\82A\82\0B\A9@\06\C3\06\04\19\80\C2\00\DC\18 ;b`\18 \08\06I.\98Bpc\80\EC\88\81a\80 \18$\BA\E0\0A\C1\8D\01\B2#\06\86\01\82`\90\EC\02+\04\14\07\C0\186 \CA \18\80Y\82\80\CA\00\18$\07\C0\18n\10\024\98e\18\84\E0\D0\00\D9\11\03\C3\00A0Ht!\16\82Y\82\82\D2\00\18\F4\06\C1\98c\08\04Y\186 \E2 \18\00\8A\83`\8C\18(\00\08\82\C1\13\0E\C1*\D0\82*\A4\02*\0C\1B\10y\10$\C0\B5\01\B2#\06\86\01\82`\90\98\C3,\04\A4\07M\06\DA\83&\C3\BD\01\B2G\07\C8\8E\18 \07\08\82Ad\0EB\D0\0A\AEpq@.\05S\06\B2\03*\03\DD\01\95\81\F0\80\CA0b\A0\00 \08\06\95:$\B1\00\0BD@\0B\83\0C\01,\BC\C2 \83\10\0B\AF0\C80\C8\C2+\0C2\10\B3\F0\0A\B3\0D\AF@\00\B3\0D\01\11\CC6\04\840\DB\10\10\C3\B0\01\81\0AA\02\1C( ;b`\18 \08\06\C9<\98C@\A7\10\8C\ED\0D\AA\A0\0E\EA@\C1\95a\C4@\01@\10\0C\B0z\18\C8!`\87p\00\07R\85`loh\85v\A0\07\0A\AE\0C#\06\0A\00\82`\80\E1\C3p\0E\C1;\90\C38\CC\12\10\84\0A\C0\B0\C0\1D@`\D8\80P\85`\00f\09\82[\05dG\0C\0C\03\04\C1 \E1\07{\08\AE\15\90\1D10\0C\10\04\83\A4\1F\E0!\1810\0C\10\04\83\A4\1F\E8\E1\15\0E\16\90\1D10\0C\10\04\83\C4\1F\EA!8Y@v\C4\C00@\10\0C\92\7F\A0\87\E0h\01\D9\11\03\C3\00A0H@\E2\1E\82\B3\05dG\0C\0C\03\04\C1 \09\09z\08\0E\17\90\1D10\0C\10\04\83D$\F8!8]@v\C4\C00@\10\0C\92\91\E8\87\E0x\01\D9\11\03\C3\00A0HH\C2\1F\82\F3\05dG\0C\0C\03\04\C1 )\89\7F\08\0E\1C\90\1D10\0C\10\04\83\C4$\FA!8q@v\C4\C00@\10\0C\92\93\F0\87\E0\C8\01\D9\11\03\C3\00A0HP\E2\1F\823\07dG\0C\0C\03\04\C1 I\09\7F\08\0E\1D\90\1D10\0C\10\04\83D%\FE!8u@v\C4\C00@\10\0C\92\95\10\89\00\C3\81\00\00I\00\00\00\A6tT\84@\5CSm2\04\E2P\93\09\06\10\C00\F0V\85T\84\804\C8t\9B\15R\11\02\D2 \13nWHE\08H\83L\B9a!\15! \0D2\E9\B6CL\CC\B3_\C1Q8\91O4\C3\09@\85\13\F9\82\D3\0C\BE\B0\10@C=\96\12!\CD@\5C\BF\F4\00\02\F1K\13\10MFuT\84\804\C8d+\C9@<\D3\C1N\C4s\04G\E1D>\D2\08\86\93<\111 \020\11\E62\5C4m*\0F\D2\0C\C4\F5K\0F \10?\D1\0C\F62\5Ctm0\C3E\DB\163\5C\F4m7\06\D2\08\0EC\D8\12\C1P\8Bm`\83\C0<\C6\A2\0C\C43\1D\EC\04D\93\E5\04\83\D1$H\C1p\8E\13\0D\92\F1\09UmdFE\08H#\99\D0\00\0C\83\D5 \92\85L\C2AH\F6'T\B6\B5 \03\F1L\87\D7\0C7\80\14N\E4\07\CE\80\D9\D1\C0P\8B\19\19\15!\10\D7d3\C5\838\D4t\00\05\D1L\11f6\C8\E3,@\84L\98\A1\14\03\F1L\87\E9\0C\13\F3\5C\00R8\91/8\CD\00\00\011\00\00(\00\00\00[\06+\88\B6\0CZ m\19\BC@\DA2\80A m\19\C6 \88\B6\0Cf\10H[\864\08\A2-\03\1B\04\D2\96\E1\0D\82h\CB\E0\07A\B4e \85@\DA2\9CB\10m\19R!\90\B6\0C\B0\10D[\86X\08\A2-\C3,\04\D1\96\81\16\82h\CB\A0\0BA\B4e\F0\85@\DA2\80C m\19\C6!\90\B6\0C\E4\10H[\86s(\A6-\83:\04\D2\96a\1D\02i\CB\C0\0E\81\B4e\B0\87\A0\DA2\E4C m\19\F6!\90\B6\0C\FD\10H[\86\7F\08\A4-\83H\04\D1\96\81$\82\08\00!1\00\00\0C\00\00\00\0B\90@\18\00\A20\F2`\01\05\04\C2@\14\C6\81$\CA\C24\CE\03I\11\B2\E0\02\90@\18\88\C28\12ea\1A\E7\81\22i\A2\AA\05\00\00\00\00\00\00\00q \00\00\03\00\00\002\0E\10\22\84\01\DA\04\00\00\00\00\00\00\00\00e\0C\00\00S\00\00\00\12\03\94\A0\02\00\00\00\01\00\00\00\F1\00\00\00\15\00\00\00D\00\00\00\01\00\00\00P\00\00\00\00\00\00\00P\00\00\00\0A\00\00\00@\01\00\00\00\00\00\00\06\01\00\00\03\00\00\00\09\01\00\00\0C\00\00\00\15\01\00\00\00\00\00\00\00\00\00\00\0A\00\00\00\00\00\00\00\13\00\00\00\04\00\00\00\13\00\00\00\04\00\00\00\FF\FF\FF\FF\00$\00\00\17\00\00\00\18\00\00\00\17\00\00\00\18\00\00\00\FF\FF\FF\FF\08,\00\00/\00\00\00\1F\00\00\00/\00\00\00\1F\00\00\00\FF\FF\FF\FF\08,\00\00N\00\00\00\1D\00\00\00N\00\00\00\1D\00\00\00\FF\FF\FF\FF\08,\00\00k\00\00\00\1A\00\00\00k\00\00\00\1A\00\00\00\FF\FF\FF\FF\08,\00\00\85\00\00\00\19\00\00\00\85\00\00\00\19\00\00\00\FF\FF\FF\FF\08,\00\00\9E\00\00\00&\00\00\00\9E\00\00\00&\00\00\00\FF\FF\FF\FF\08,\00\00\C4\00\00\00\16\00\00\00\C4\00\00\00\16\00\00\00\FF\FF\FF\FF\08,\00\00\DA\00\00\00\17\00\00\00\DA\00\00\00\17\00\00\00\FF\FF\FF\FF\08,\00\00\00\00\00\00\13\00\00\00\00\00\00\00\13\00\00\00\FF\FF\FF\FF\00\04\00\00\00\00\00\00]\0C\00\00I\00\00\00\12\03\945\02\00\00\00__PrintfTensorIndexmainllvm.lifetime.start.p0i8llvm.tpc.get.index.space.offsetllvm.tpc.get.index.space.sizellvm.tpc.ld.tnsr.v256i8.i1llvm.memcpy.p0i8.p0i8.i32llvm.tpc.convert.v256i32.v256i8.v256i1llvm.lifetime.end.p0i8llvm.tpc.st.tnsr.v64i327.0.0 git-bd3d9f17577tpctrunc_32_8.c\00\00\00\00\00\00\00", section ".llvmir"
@llvm.compiler.used = appending global [1 x i8*] [i8* getelementptr inbounds ([5216 x i8], [5216 x i8]* @llvm.embedded.module, i32 0, i32 0)], section "llvm.metadata"

; Function Attrs: nounwind
define dso_local void @main() local_unnamed_addr #0 {
entry:
  %0 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %1 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %add = add <5 x i32> %1, %0
  %vecext = extractelement <5 x i32> %0, i32 0
  %mul = shl nsw i32 %vecext, 8
  %vecext1 = extractelement <5 x i32> %add, i32 0
  %mul2 = shl nsw i32 %vecext1, 8
  %vecinit6 = insertelement <5 x i32> <i32 undef, i32 0, i32 0, i32 0, i32 0>, i32 %mul, i32 0
  %cmp42 = icmp slt i32 %mul, %mul2
  br i1 %cmp42, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %ifmCoords.044 = phi <5 x i32> [ %vecins, %for.body ], [ %vecinit6, %for.body.preheader ]
  %d.043 = phi i32 [ %add8, %for.body ], [ %mul, %for.body.preheader ]
  %vecins = insertelement <5 x i32> %ifmCoords.044, i32 %d.043, i32 0
  %2 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %vecins, i8 0, i32 0, <256 x i8> undef, i1 true, i1 false)
  %3 = tail call <256 x i32> @llvm.tpc.convert.v256i32.v256i8.v256i1(<256 x i8> %2, i8 4, i32 519, <256 x i32> undef, <256 x i1> undef, i1 true)
  %4 = trunc <256 x i32> %3 to <256 x i8>
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %vecins, i8 1, <256 x i8> %4, i32 0, i1 true, i1 false)
  
  %add8 = add nsw i32 %d.043, 256
  %cmp = icmp slt i32 %add8, %mul2
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

; Function Attrs: nounwind readnone
declare <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32>, i8, i32, <256 x i8>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <256 x i32> @llvm.tpc.convert.v256i32.v256i8.v256i1(<256 x i8>, i8, i32, <256 x i32>, <256 x i1>, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64i32(<5 x i32>, i8, <64 x i32>, i32, i1, i1) #2
declare void @llvm.tpc.st.tnsr.v256i8(<5 x i32>, i8, <256 x i8>, i32, i1, i1) #2

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="goya2" "target-features"="+goya2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }
