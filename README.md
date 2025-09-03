# YASLC (Yet Another Sugared Lambda Calculus)

Tree-walk interpreter for a call-by-value simple functional language inspired by core ML features.
A program evaluates to a function or a number.

## About the Language
### Interesting things you can make with it:

Recursion without built-in recursion (using the Z combinator)
```
let Z
    fn g (fn r g(fn y r(r)(y)))(fn r g(fn y r(r)(y))) # Z combinator
in 
let my_add 
    fn f fn x fn y if y then f(+x)(-y) else x
in 
Z(my_add)(5)(4) # evaluates to 9
```

Closures as a poor man's Object (encoding data structures)
```
let pair 
    fn x fn y fn f f(x)(y)
in 
let fst 
    fn x fn y x
in 
let snd 
    fn x fn y y
in
let p 
    pair(1)(5)
in
let this_is_1 p(fst) in
let this_is_5 p(snd) in
...
```

## Language features

### First class objects: fixnum32 numbers and functions

#### unary operators on numbers
`+1 # evaluates to 2`

`-4 # evaluates to 3`

`'>1' # evaluates to 0 (is negative operator)`

`'<5' # evaluates to 1 (is positive operator)`

#### lambda abstraction/application
`(fn x ++x)(3) # evaluates to 5`

### conditionals
`if 5 then 3 else 0 # evaluates to 3`

### bindings
`let x 5 in ++x # evaluates to 7`

A more interesting example, which shows the use of closures, currying and recursion to define addition, which is not built into the language

```
letrec my_add 
    fn x fn y if y then my_add(+x)(-y) else x
in 
my_add(5)(4) # evaluates to 9
```

`my_add(5)(4) # evaluates to 9`

### comments
`# hashtags >>>>>>>>>>>> //`

P.S.
> typedef struct X X Considered Harmful - idk, Linus Torvalds

> linked list >>>>>>>>>>>>>>> dynamic array - Lisp-er
