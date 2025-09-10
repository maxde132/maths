import ctypes

class ExprType(ctypes.c_int):
    Operation_type = 0
    RealNumber_type = 1
    ComplexNumber_type = 2
    Boolean_type = 3
    String_type = 4
    Vector_type = 5

class strbuf(ctypes.Structure):
    _fields_ = [
            ('s', ctypes.c_char_p),
            ('len', ctypes.c_size_t),
            ('allocd', ctypes.c_bool),
    ]

class Expr(ctypes.Structure):
    pass

class VecN(ctypes.Structure):
    _fields_ = [
            ('ptr', ctypes.POINTER(ctypes.POINTER(Expr))),
            ('n', ctypes.c_size_t),
    ]

class ComplexDouble(ctypes.Structure):
    _fields_ = [
            ('real', ctypes.c_double),
            ('imag', ctypes.c_double),
    ]

class EvalValue(ctypes.Union):
    _fields_ = [
            ('n', ctypes.c_double),
            ('cn', ComplexDouble),
            ('b', ctypes.c_bool),
            ('s', strbuf),
            ('v', VecN),
    ]

class TypedValue(ctypes.Structure):
    _fields_ = [
            ('type', ExprType),
            ('v', EvalValue),
    ]

mml = ctypes.CDLL('./libmml.so')

mml.parse_eval.argtypes = [ctypes.c_char_p]
mml.parse_eval.restype = TypedValue

mml.parse.argtypes = [ctypes.c_char_p]
mml.parse.restype = ctypes.POINTER(Expr)

mml.eval_mml.argtypes = [ctypes.POINTER(Expr)]
mml.eval_mml.restype = TypedValue

mml.print_val.argtypes = [TypedValue]
mml.print_val.restype = None

mml.assign_var.argtypes = [ctypes.c_char_p, ctypes.POINTER(Expr)]
mml.assign_var.restype = None

mml.free_expr.argtypes = [ctypes.POINTER(Expr)]
mml.free_expr.restype = None


mml.init_eval.argtypes = None
mml.init_eval.restype = None

mml.cleanup_eval.argtypes = None
mml.cleanup_eval.restype = None
