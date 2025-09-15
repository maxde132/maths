# Syntax Guide for My Math Lang (MML)

### <span id="contents">Contents</span>:
1. [Concepts](#concepts)
2. [Basic Syntax](#basic-syntax)
3. [Advanced Syntax](#advanced-syntax)
4. [Built-ins](#built-ins)

## <span id="concepts">Concepts</span> [↩](#contents)
MML is a 'programming' language that evaluates mathematical expressions. I put 'programming' in quotes because it doesn't behave like most programming languages. In most programming languages, assigning an expression to a variable evaluates the expression, and assigns the output of that expression to the variable.
MML, more similar to mathematics than programming, instead literally assigns the expression to the variable. Rather than assigning the evaluated value of the expression to the variable and retrieving that value whenever the variable is used, MML simply reevaluates the expression associated with the variable each time it is used. This means that, while a warning may be displayed, it is not illegal to assign to a variable an expression containing an undefined value, given that the undefined value is defined before the variable is evaluated. A side effect of this is that recursive definitions are illegal. This means that something like `x = x + 1` is not allowed, as would be the case in mathematics (also because I can't be bothered to rework my entire program to allow it but that's irrelevant). Actually, I haven't added a check for this just yet, so it will probably cause a segmentation fault if you try this.
A variable/expression is 'evaluated' when it is used anywhere other than a variable definition or in a vector literal (see [Advanced Syntax](#advanced-syntax) for more on vectors).

## <span id="basic-syntax">Basic Syntax</span> [↩](#contents)
MML uses the most of the usual syntax for mathematical expressions, as well as a few operators taken from popular programming languages.
In this guide, the word 'expression' will be used to mean any valid operation or constant, where an operation may have as one of its operands another operation. The word `expr` in a specification (the leftmost part of an entry) will be used to represent any valid expression.
All expressions (equality or otherwise) found here evaluate to the boolean value `true`, for density of the explanation.
#### Some of the relevant operators:
- `A == B`	= equality operator							(ex. `3 == 3`)
- `A != B`	= inequality operator						(ex. `9 != 5`)
- `!A`		= unary Boolean NOT operator				(ex. `!false == true`)
- `A < B`	= less than operator						(ex. `9 < 5 == false`)
- `A <= B`	= less than or equal operator				(ex. `9 <= 9 == true`)
- `A > B`	= greater than operator						(ex. `9 > 5 == true`)
- `A >= B`	= greater than or equal operator			(ex. `9 >= 9 == true`)
- `A + B`	= add operation								(ex. `9 + 5 == 14`)
- `A - B`	= subtract operation						(ex. `9 - 5 == 4`)
- `-A`		= unary negate operation					(ex. `-9 + 5 == -4`)
- `+A`		= this does nothing but it's not invalid	(ex. `+9 + 5 == 14`)
- `A * B`	= multiply operation						(ex. `9 * 5 == 45`)
- `A / B`	= divide operation							(ex. `9 / 5 == 1.8`)
- `A ^ B`	= power operator							(ex. `9 ^ 5 ^ 2 == 9^25`)
- `A % B`	= modulo (remainder) operator				(ex. `9 % 5 == 4`)
- `|A|`		= absolute value operator					(ex. `|5-9| == 4`)
- `(expr)`	= parentheses are always evaluated first	(ex. `9*(2+3) == 45`)

Because it would be a massive pain otherwise, you may specify multiple statements/expressions by separating them with a semicolon (`;`).

## <span id="advanced-syntax">Advanced Syntax</span> [↩](#contents)
MML also supports the use of vectors of any length (it gets weird if the length is 0, though).
A vector may be created via this syntax for a vector literal:
`[A, B, C, ...]` where there may be a trailing comma following the last element. `[5, 15, 9.2]` is an example.
The values of a vector may be accessed through the `.` operator, in a zero-indexed fashion, like so: `[5, 15, 9.2].2 == 9.2`
There is also nothing stopping you from creating nested vectors, but they do not behave as a matrix would in mathematics.

A variable can be defined via the `=` operator, like so: `x = 0` <br/>
For more on the semantics of variable assignment, see [Concepts](#concepts).

MML allows you to implicitly multiply two distinct values in certain circumstances by placing them next to each other.
Example: `5pi == 5*pi` <br />

MML supports complex numbers as well. A complex number can be defined as: `z = 5 + 3i`, where `i` is a built-in constant (see [Built-ins](#built-ins) for other built-in constants) that is multiplied by the number `3` and added to `5`. This was not made possible via any special syntax---really, under the hood, MML just promotes a number to a complex number if it used in an expression with a complex number, so all the example really does is promote `3` to the complex number `3+0i`, multiply it by `i` to create `0+3i`, and add `5` to make `5+3i`, which is assigned to `z`.

Additionally, built-in functions are provided, though user-defined functions are not yet implemented. Functions are called via this syntax:
`function_name{argument1, argument2, ...}`. Most functions take just 1 argument, but some take 2 or more. For example: `cos{1.5pi} == 0` (due to rounding errors, this statement evaluates to `false` if the `--no-estimate-equality` argument is passed to the `mml` CLI).
A full catalogue of all provided built-in functions and their properties is available in [Built-ins](#built-ins).

## <span id="built-ins">Built-ins</span> [↩](#contents)
#### Built-in constants that can be used:
- `true` = the Boolean value `true` (value of `1 == 1`)
- `false` = the Boolean value `false` (value of `0 == 1`)
- `pi` = the mathematical constant `π` (the ratio of a circle's circumference to its diameter).
- `e` = the mathematical consant `e` (Euler's number).
- `phi` = the mathematical constant `φ` (the golden ratio)
- `i` = the imaginary unit `i` (the square root of -1)
- `nan` = NaN = not a number (error value)
- `inf` = infinity (can be made negative with `-inf`)

In the leftmost section of a function's entry in this list, `...` represents the possibility for any number of arguments to be passed to a particular function. All functions here take as arguments a real or complex number, unless otherwise specified.
#### Built-in functions (see [Advanced Syntax](#advanced-syntax) for function call syntax):
- `print{...}` = evaluates and prints the values of its arguments (which may be of any type), separated by spaces, with no newline following the final printed value.
- `println{...}` = evaluates and prints the value of its arguments (which may be of any type), separated by newlines, with a newline following the final printed value.
- `dbg{expr}` = prints the Abstract Syntax Tree (AST) construction of an expression `expr`. The expression is not evaluated.
- `atan2{y, x}` = performs the `atan2` function on its 2 _real_ arguments `y` and `x` (see [the wiki page](https://en.wikipedia.org/wiki/Atan2)).
- `sin{a}` = returns the trigonometric `sin` aka sine function of `a`.
- `cos{a}` = returns the trigonometric `cos` aka cosine function of `a`.
- `tan{a}` = returns the trigonometric `tan` aka tangent function of `a`.
- `asin{a}` = returns the trigonometric `arcsin` aka inverse sine function of `a`.
- `acos{a}` = returns the trigonometric `arccos` aka inverse cosine function of `a`.
- `atan{a}` = returns the trigonometric `arctan` aka inverse tangent function of `a`.
- `sinh{a}` = returns the hyperbolic trigonometric `sin` aka sine function of `a`.
- `cosh{a}` = returns the hyperbolic trigonometric `cos` aka cosine function of `a`.
- `tanh{a}` = returns the hyperbolic trigonometric `tan` aka tangent function of `a`.
- `asinh{a}` = returns the hyperbolic trigonometric `arcsin` aka inverse sine function of `a`.
- `acosh{a}` = returns the hyperbolic trigonometric `arccos` aka inverse cosine function of `a`.
- `atanh{a}` = returns the hyperbolic trigonometric `arctan` aka inverse tangent function of `a`.
- `ln{x}` = returns the natural logarithm (base e logarithm) of `x`.
- `log2{x}` = returns the base 2 logarithm of `x`.
- `log10{x}` = returns the base 10 logarithm of `x`.
- `csqrt{x}` = returns the square root of `x` regardless of its sign. 
- `sqrt{x}` = returns the square root of `x`. If `x` is a negative real number, returns `nan`; otherwise, behaves like `csqrt{x}`.
- `floor{x}` = returns the floor of the real argument `x` (the greatest integer less than or equal to `x`).
- `ceil{x}` = returns the ceiling of the real argument `x` (the least integer greater than or equal to `x`).
- `round{x}` = returns the closest integer to the real argument `x` (rounds away from 0).
- `conj{z}` = returns the complex conjugate of the complex argument `z` (see [the wiki page](https://en.wikipedia.org/wiki/Complex_conjugate)).
- `phase{z}` = returns the phase (argument) (as a real) of the complex argument `z` (see [the wiki page](https://en.wikipedia.org/wiki/Argument_(complex_analysis))).
- `real{z}` = returns the real component (as a real) of the complex argument `z`.
- `imag{z}` = returns the imaginary component (as a real) of the complex argument `z`.

#### Additional functions that may or may not be provided
- `max{...}` = returns the greatest of its arguments, where each of its arguments must be a real number or a Boolean value (the `max` function makes little sense on unordered values such as complex numbers).
- `min{...}` = returns the least of its arguments, where each of its arguments must be a real number or a Boolean value (the `min` function makes little sense on unordered values such as complex numbers).
