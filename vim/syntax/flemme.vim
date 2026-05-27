" Vim syntax file
" Language:     Flemme
" Description:  Langage de script pédagogique (.flm)

if exists("b:current_syntax")
  finish
endif

" ── Mots-clés ────────────────────────────────────────────────────────────────

syntax keyword flemmeControl   if else while for in break continue return
syntax keyword flemmeKeyword   let fn
syntax keyword flemmeBuiltin   print
syntax keyword flemmeConstant  true false

" ── Fonctions natives ─────────────────────────────────────────────────────────

syntax keyword flemmeBuiltinMath  sin cos tan sqrt exp ln log10 abs floor ceil round
syntax keyword flemmeBuiltinFunc  len range push pop input num str type print_raw assert
syntax keyword flemmeBuiltinStr   upper lower trim split find replace

" ── Commentaires  # ──────────────────────────────────────────────────────────

syntax match flemmeComment /#.*$/

" ── Chaînes  "..."  avec interpolation {var} et {var:.2f} ────────────────────
" oneline : interdit les chaînes sur plusieurs lignes (non supporté en flemme)

syntax region flemmeString start=+"+ end=+"+ oneline contains=flemmeInterp,flemmeEscape

" Placeholder {variable}, {variable:.2f}, {obj.membre}
syntax match flemmeInterp contained /{[A-Za-z_][A-Za-z0-9_.]*\%(:[^}]*\)\?}/

" Guillemet échappé \"
syntax match flemmeEscape contained /\\"/

" ── Nom de fonction après fn ──────────────────────────────────────────────────
" \zs positionne le début du match après "fn " : seul le nom est coloré

syntax match flemmeFunction /\<fn\s\+\zs[A-Za-z_][A-Za-z0-9_]*/

" ── Nombres (décimaux avant entiers) ─────────────────────────────────────────

syntax match flemmeFloat   /\<\d\+\.\d\+\>/
syntax match flemmeInteger /\<\d\+\>/

" ── Opérateurs ────────────────────────────────────────────────────────────────
" Alternation ordonnée (les plus longs en premier).
" En mode magic : \| = alternation, \* = astérisque littéral, \/ = slash littéral.
" Bi-caract : ++ -- == != <= >= && ||
" Affectation composée : += -= *= // /=
" Mono-caract : + - * % ^ < > ! / =

syntax match flemmeOperator /++\|--\|==\|!=\|<=\|>=\|&&\|||\|+=\|-=\|\*=\|\/\/\|\/=\|[+\-*%^<>!]\|\/\|=/

" ── Liaison aux groupes de coloration standard de Vim ─────────────────────────

highlight def link flemmeControl     Conditional
highlight def link flemmeKeyword     Keyword
highlight def link flemmeBuiltin     Statement
highlight def link flemmeConstant    Boolean
highlight def link flemmeBuiltinMath Special
highlight def link flemmeBuiltinFunc Special
highlight def link flemmeBuiltinStr  Special
highlight def link flemmeComment     Comment
highlight def link flemmeString      String
highlight def link flemmeInterp      Identifier
highlight def link flemmeEscape      SpecialChar
highlight def link flemmeFloat       Float
highlight def link flemmeInteger     Number
highlight def link flemmeOperator    Operator
highlight def link flemmeFunction    Function

let b:current_syntax = "flemme"
