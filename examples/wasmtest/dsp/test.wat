(module
  (type $swap_t (func (param i32 i64) (result i64 i32)))
  (func $swap (type $swap_t) (param $x i32) (param $y i64) (result i64 i32)
    (local.get $y)
    (local.get $x))
  (export "swap" (func $swap)))
