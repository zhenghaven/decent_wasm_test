// build:
// clang --target=wasm32 --no-standard-libraries -Wl,--entry=main -Wl,--allow-undefined-file=decent_wasm_native_list.txt -o test.wasm test.c

extern int decent_wasm_sum(int a, int b);
extern void decent_wasm_print(char * msg);

int main()
{
	decent_wasm_print("Hello World!\n");

	int tmp = decent_wasm_sum(1, 2);
	if (tmp == 3)
	{
		decent_wasm_print("Correct summation result\n");
	}
	else
	{
		decent_wasm_print("Wrong summation result\n");
	}

	for(int i = 0; i < 10; ++i)
	{
		decent_wasm_print("Loop\n");
	}

	return 0;
}
