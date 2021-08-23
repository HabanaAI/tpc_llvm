// RUN: %clang -c -target tpc -E %s | FileCheck -check-prefix TPC %s
void main(){
 // TPC:int a = 5;
 // TPC:int b = 5;
 #if (VERSION2DEC(0,1,9) > __HABANA_TOOL_VERSION)
	int a = 1;
 #else
	int a = 5;
 #endif
 #if (VERSION2DEC(14,0,0) > __TPC_DROP_VERSION)
	int b = 1;
 #else
	int b = 5;
 #endif
}
