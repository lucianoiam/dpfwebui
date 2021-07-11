;; wasmer compile --enable-multi-value main.wat -o main.wasm
;; Validation error: Bad magic number (at offset 0)
;; Wasmer compiler is broken? use online tool instead
;; https://webassembly.github.io/wabt/demo/wat2wasm/
(module
  (type $swap_t (func (param i32 i64) (result i64 i32)))
  (func $swap (type $swap_t) (param $x i32) (param $y i64) (result i64 i32)
    (local.get $y)
    (local.get $x))
  (export "swap" (func $swap)))
