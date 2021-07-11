;; Validation error: Bad magic number (at offset 0)
;; Wasmer compiler is broken? use online tool instead
;; https://webassembly.github.io/wabt/demo/wat2wasm/
;; INFO asc module.ts --textFile module.wat --binaryFile module.wasm -O3 --runtime stub --enable reference-types
(module
 (type $none_=>_i32 (func (result i32)))
 (type $i32_i32_i32_i32_=>_none (func (param i32 i32 i32 i32)))
 (type $none_=>_none (func))
 (global $~lib/rt/stub/offset (mut i32) (i32.const 0))
 (memory $0 1)
 (data (i32.const 1036) ",")
 (data (i32.const 1048) "\01\00\00\00\10\00\00\00W\00a\00s\00m\00T\00e\00s\00t")
 (data (i32.const 1084) "<")
 (data (i32.const 1096) "\01\00\00\00(\00\00\00A\00l\00l\00o\00c\00a\00t\00i\00o\00n\00 \00t\00o\00o\00 \00l\00a\00r\00g\00e")
 (data (i32.const 1148) "<")
 (data (i32.const 1160) "\01\00\00\00\1e\00\00\00~\00l\00i\00b\00/\00r\00t\00/\00s\00t\00u\00b\00.\00t\00s")
 (data (i32.const 1212) "<")
 (data (i32.const 1224) "\01\00\00\00$\00\00\00U\00n\00p\00a\00i\00r\00e\00d\00 \00s\00u\00r\00r\00o\00g\00a\00t\00e")
 (data (i32.const 1276) ",")
 (data (i32.const 1288) "\01\00\00\00\1c\00\00\00~\00l\00i\00b\00/\00s\00t\00r\00i\00n\00g\00.\00t\00s")
 (export "getLabel" (func $module/getLabel))
 (export "memory" (memory $0))
 (start $~start)
 (func $~lib/string/String.UTF8.encode (result i32)
  (local $0 i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  i32.const 1056
  local.set $2
  i32.const 1052
  i32.load
  i32.const 1056
  i32.add
  local.set $1
  i32.const 1
  local.set $0
  loop $while-continue|0
   local.get $1
   local.get $2
   i32.gt_u
   if
    block $while-break|0
     local.get $2
     i32.load16_u
     local.tee $5
     i32.const 128
     i32.lt_u
     if (result i32)
      local.get $5
      i32.eqz
      br_if $while-break|0
      local.get $0
      i32.const 1
      i32.add
     else
      local.get $5
      i32.const 2048
      i32.lt_u
      if (result i32)
       local.get $0
       i32.const 2
       i32.add
      else
       local.get $1
       local.get $2
       i32.const 2
       i32.add
       i32.gt_u
       i32.const 0
       local.get $5
       i32.const 64512
       i32.and
       i32.const 55296
       i32.eq
       select
       if
        local.get $2
        i32.load16_u offset=2
        i32.const 64512
        i32.and
        i32.const 56320
        i32.eq
        if
         local.get $0
         i32.const 4
         i32.add
         local.set $0
         local.get $2
         i32.const 4
         i32.add
         local.set $2
         br $while-continue|0
        end
       end
       local.get $0
       i32.const 3
       i32.add
      end
     end
     local.set $0
     local.get $2
     i32.const 2
     i32.add
     local.set $2
     br $while-continue|0
    end
   end
  end
  local.get $0
  i32.const 1073741804
  i32.gt_u
  if
   i32.const 1104
   i32.const 1168
   i32.const 86
   i32.const 30
   unreachable
  end
  local.get $0
  i32.const 16
  i32.add
  local.tee $1
  i32.const 1073741820
  i32.gt_u
  if
   i32.const 1104
   i32.const 1168
   i32.const 33
   i32.const 29
   unreachable
  end
  global.get $~lib/rt/stub/offset
  local.tee $2
  i32.const 4
  i32.add
  local.tee $6
  local.get $1
  i32.const 19
  i32.add
  i32.const -16
  i32.and
  i32.const 4
  i32.sub
  local.tee $5
  i32.add
  local.tee $3
  memory.size
  local.tee $4
  i32.const 16
  i32.shl
  i32.const 15
  i32.add
  i32.const -16
  i32.and
  local.tee $1
  i32.gt_u
  if
   local.get $4
   local.get $3
   local.get $1
   i32.sub
   i32.const 65535
   i32.add
   i32.const -65536
   i32.and
   i32.const 16
   i32.shr_u
   local.tee $1
   local.get $1
   local.get $4
   i32.lt_s
   select
   memory.grow
   i32.const 0
   i32.lt_s
   if
    local.get $1
    memory.grow
    i32.const 0
    i32.lt_s
    if
     unreachable
    end
   end
  end
  local.get $3
  global.set $~lib/rt/stub/offset
  local.get $2
  local.get $5
  i32.store
  local.get $6
  i32.const 4
  i32.sub
  local.tee $1
  i32.const 0
  i32.store offset=4
  local.get $1
  i32.const 0
  i32.store offset=8
  local.get $1
  i32.const 0
  i32.store offset=12
  local.get $1
  local.get $0
  i32.store offset=16
  i32.const 1056
  local.set $4
  i32.const 1052
  i32.load
  i32.const 1
  i32.shr_u
  i32.const 1
  i32.shl
  i32.const 1056
  i32.add
  local.set $2
  local.get $6
  i32.const 16
  i32.add
  local.tee $5
  local.set $0
  loop $while-continue|00
   local.get $2
   local.get $4
   i32.gt_u
   if
    local.get $4
    i32.load16_u
    local.tee $3
    i32.const 128
    i32.lt_u
    if (result i32)
     local.get $0
     local.get $3
     i32.store8
     local.get $0
     i32.const 1
     i32.add
    else
     local.get $3
     i32.const 2048
     i32.lt_u
     if (result i32)
      local.get $0
      local.get $3
      i32.const 6
      i32.shr_u
      i32.const 192
      i32.or
      local.get $3
      i32.const 63
      i32.and
      i32.const 128
      i32.or
      i32.const 8
      i32.shl
      i32.or
      i32.store16
      local.get $0
      i32.const 2
      i32.add
     else
      local.get $3
      i32.const 63488
      i32.and
      i32.const 55296
      i32.eq
      if
       local.get $2
       local.get $4
       i32.const 2
       i32.add
       i32.gt_u
       i32.const 0
       local.get $3
       i32.const 56320
       i32.lt_u
       select
       if
        local.get $4
        i32.load16_u offset=2
        local.tee $1
        i32.const 64512
        i32.and
        i32.const 56320
        i32.eq
        if
         local.get $0
         local.get $3
         i32.const 1023
         i32.and
         i32.const 10
         i32.shl
         i32.const 65536
         i32.add
         local.get $1
         i32.const 1023
         i32.and
         i32.or
         local.tee $1
         i32.const 63
         i32.and
         i32.const 128
         i32.or
         i32.const 24
         i32.shl
         local.get $1
         i32.const 6
         i32.shr_u
         i32.const 63
         i32.and
         i32.const 128
         i32.or
         i32.const 16
         i32.shl
         i32.or
         local.get $1
         i32.const 12
         i32.shr_u
         i32.const 63
         i32.and
         i32.const 128
         i32.or
         i32.const 8
         i32.shl
         i32.or
         local.get $1
         i32.const 18
         i32.shr_u
         i32.const 240
         i32.or
         i32.or
         i32.store
         local.get $0
         i32.const 4
         i32.add
         local.set $0
         local.get $4
         i32.const 4
         i32.add
         local.set $4
         br $while-continue|00
        end
       end
      end
      local.get $0
      local.get $3
      i32.const 12
      i32.shr_u
      i32.const 224
      i32.or
      local.get $3
      i32.const 6
      i32.shr_u
      i32.const 63
      i32.and
      i32.const 128
      i32.or
      i32.const 8
      i32.shl
      i32.or
      i32.store16
      local.get $0
      local.get $3
      i32.const 63
      i32.and
      i32.const 128
      i32.or
      i32.store8 offset=2
      local.get $0
      i32.const 3
      i32.add
     end
    end
    local.set $0
    local.get $4
    i32.const 2
    i32.add
    local.set $4
    br $while-continue|00
   end
  end
  local.get $0
  i32.const 0
  i32.store8
  local.get $5
 )
 (func $module/getLabel (result i32)
  call $~lib/string/String.UTF8.encode
 )
 (func $~start
  i32.const 1324
  global.set $~lib/rt/stub/offset
 )
)


