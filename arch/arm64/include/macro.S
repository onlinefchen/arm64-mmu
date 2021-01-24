.macro _mov_imm16 _reg, _val, _shift
        .if (\_val >> \_shift) & 0xffff
                .if (\_val & (1 << \_shift - 1))
                        movk    \_reg, (\_val >> \_shift) & 0xffff, LSL \_shift
                .else
                        mov     \_reg, \_val & (0xffff << \_shift)
                .endif
        .endif
.endm

.macro mov_imm _reg, _val
        .if (\_val) == 0
                mov     \_reg, #0
        .else
                _mov_imm16      \_reg, (\_val), 0
                _mov_imm16      \_reg, (\_val), 16
                _mov_imm16      \_reg, (\_val), 32
                _mov_imm16      \_reg, (\_val), 48
        .endif
.endm
