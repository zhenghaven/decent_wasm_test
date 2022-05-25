(module
  (type (;0;) (func (param i32)))
  (type (;1;) (func (param i32 i32) (result i32)))
  (type (;2;) (func))
  (type (;3;) (func (param i32 i32 i32 i32) (result i32)))
  (import "wasi_snapshot_preview1" "proc_exit" (func $__wasi_proc_exit (type 0)))
  (import "env" "decent_wasm_print" (func $decent_wasm_print (type 0)))
  (import "env" "decent_wasm_sum" (func $decent_wasm_sum (type 1)))
  (func $decent_wasm_prerequisite_imports (type 2)
    i32.const 0
    call $__wasi_proc_exit
    unreachable)
  (func $decent_wasm_main (type 3) (param i32 i32 i32 i32) (result i32)
    (local i32)
    i32.const 1079
    call $decent_wasm_print
    i32.const 1029
    i32.const 1055
    i32.const 1
    i32.const 2
    call $decent_wasm_sum
    i32.const 3
    i32.eq
    select
    call $decent_wasm_print
    i32.const 10
    local.set 4
    loop  ;; label = @1
      i32.const 1024
      call $decent_wasm_print
      local.get 4
      i32.const -1
      i32.add
      local.tee 4
      br_if 0 (;@1;)
    end
    i64.const 123
    call 6
    i32.const 0)
  (memory (;0;) 2)
  (global $__stack_pointer (mut i32) (i32.const 66640))
  (export "memory" (memory 0))
  (export "decent_wasm_prerequisite_imports" (func $decent_wasm_prerequisite_imports))
  (export "decent_wasm_main" (func $decent_wasm_main))
  (data $.rodata (i32.const 1024) "Loop\00Correct summation result\0a\00Wrong summation result\0a\00Hello World!\0a\00")
  (type (;4;) (func (param i32 i32 i32 i32 i64) (result i32)))
  (func $decent_wasm_injected_main (type 4) (param i32 i32 i32 i32 i64) (result i32)
    local.get 4
    global.set 1
    local.get 0
    local.get 1
    local.get 2
    local.get 3
    call $decent_wasm_main
  )
  (export "decent_wasm_injected_main" (func $decent_wasm_injected_main))
  (global (;1;) (mut i64) (i64.const 0)) (;threshold;)
  (global (;2;) (mut i64) (i64.const 0)) (;counter;)
  (type (;5;) (func (param i64)))
  (func (;6;) (param i64)
    local.get 0
    global.get 2
    i64.add
    global.set 2
    block  ;; label = @1
      global.get 2
      global.get 1
      i64.le_u
      br_if 0 (;@1;)
      i32.const -1
      call 0
    end)
  (type (;6;) (func (result i64)))
  (func $decent_wasm_get_icounter (result i64)
    global.get 2)
  (export "decent_wasm_get_icounter" (func $decent_wasm_get_icounter)))
