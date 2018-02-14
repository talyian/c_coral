

Module x => x is ModuleType[x.body]
Comment x => x is Unit
Let x => x is Unit
Set x => x is Unit
While x => x is Unit
Return x => x is Unit

Var x           => ~x
BinOp x         => x is Call[~x.op, ~x.lhs, ~x.rhs]
Call x          => x is Call[~x.Callee, ...~x.Params]
Member x        => x is x.base, x.member | StructType s, f => s.FieldType f
IntLiteral x    => x is Int
StringLiteral x => x is String
Block x         => x is ~x.codeLines.last
Func x          => x is Func[...~x.Params, ~x.Body]
If x            => x is Union[~x.ifbody, ~x.Elsebody]
