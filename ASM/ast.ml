type const =
  | Imm16 of int (* int Array si c'est pas pratique en Ocaml de manipuler des 0x ? *)

type mem =
| Reg     of int
| Mem     of int
| Flag    of int
| PC
| Const   of const

type triop =
  And | Or | Nor | Xor | Add | Sub | Mul |  Div |
          Sll | Sra | Srl | Jmpc | Loadi

and binop = 
  Mov | Load | Neg | Not | Store

and unop =
  Ret | Call | Pop | Push | Test | Jmp

and nop = Nop

type inst =
  | Inop
  | Iunop of unop * mem
  | Ibinop of binop * mem * mem
  | Itriop of triop * mem * mem * mem
  | Ilist of inst list

and file = {
inst: inst;
}