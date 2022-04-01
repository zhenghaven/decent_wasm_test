// build:
// clang --target=wasm32 --no-standard-libraries -Wl,--entry=main -Wl,--allow-undefined-file=decent_wasm_native_list.txt -o test.wasm test.c

extern int decent_wasm_sum(int a, int b);
extern void decent_wasm_print(char * msg);

int main()
{
	decent_wasm_print("Hello World!\n");
	return 0;
}
