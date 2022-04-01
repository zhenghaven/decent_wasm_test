(module
  (type (;0;) (func (param i32)))
  (type (;1;) (func (param i32 i32) (result i32)))
  (type (;2;) (func (result i32)))
  (import "env" "decent_wasm_print" (func $decent_wasm_print (type 0)))
  (import "env" "decent_wasm_sum" (func $decent_wasm_sum (type 1)))
  (func $__original_main (type 2) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32)
    global.get 0
    local.set 0
    i32.const 16
    local.set 1
    local.get 0
    local.get 1
    i32.sub
    local.set 2
    local.get 2
    global.set 0
    i32.const 3
    local.set 3
    i32.const 1
    local.set 4
    i32.const 2
    local.set 5
    i32.const 1024
    local.set 6
    i32.const 0
    local.set 7
    local.get 2
    local.get 7
    i32.store offset=12
    local.get 6
    call $decent_wasm_print
    local.get 4
    local.get 5
    call $decent_wasm_sum
    local.set 8
    local.get 2
    local.get 8
    i32.store offset=8
    local.get 2
    i32.load offset=8
    local.set 9
    local.get 9
    local.set 10
    local.get 3
    local.set 11
    local.get 10
    local.get 11
    i32.eq
    local.set 12
    i32.const 1
    local.set 13
    local.get 12
    local.get 13
    i32.and
    local.set 14
    block  ;; label = @1
      block  ;; label = @2
        local.get 14
        i32.eqz
        br_if 0 (;@2;)
        i32.const 1038
        local.set 15
        local.get 15
        call $decent_wasm_print
        br 1 (;@1;)
      end
      i32.const 1064
      local.set 16
      local.get 16
      call $decent_wasm_print
    end
    i32.const 0
    local.set 17
    local.get 2
    local.get 17
    i32.store offset=4
    block  ;; label = @1
      loop  ;; label = @2
        i32.const 10
        local.set 18
        local.get 2
        i32.load offset=4
        local.set 19
        local.get 19
        local.set 20
        local.get 18
        local.set 21
        local.get 20
        local.get 21
        i32.lt_s
        local.set 22
        i32.const 1
        local.set 23
        local.get 22
        local.get 23
        i32.and
        local.set 24
        local.get 24
        i32.eqz
        br_if 1 (;@1;)
        i32.const 1088
        local.set 25
        local.get 25
        call $decent_wasm_print
        local.get 2
        i32.load offset=4
        local.set 26
        i32.const 1
        local.set 27
        local.get 26
        local.get 27
        i32.add
        local.set 28
        local.get 2
        local.get 28
        i32.store offset=4
        br 0 (;@2;)
      end
    end
    i32.const 0
    local.set 29
    i32.const 16
    local.set 30
    local.get 2
    local.get 30
    i32.add
    local.set 31
    local.get 31
    global.set 0
    local.get 29
    return)
  (func $main (type 1) (param i32 i32) (result i32)
    (local i32)
    call $__original_main
    local.set 2
    local.get 2
    return)
  (table (;0;) 1 1 funcref)
  (memory (;0;) 2)
  (global (;0;) (mut i32) (i32.const 66640))
  (export "memory" (memory 0))
  (export "main" (func $main))
  (data (;0;) (i32.const 1024) "Hello World!\0a\00Correct summation result\0a\00Wrong summation result\0a\00Loop\0a\00"))
