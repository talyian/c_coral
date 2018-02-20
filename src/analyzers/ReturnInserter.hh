namespace coral {
  namespace analyzers {

	// Turns the last expression in a function block into an implicit return:
	// :  func foobar(a):
	// :    a
	// for example, becomes
	// :  func foobar(a):
	// :    return a
	class ReturnInserter : public ast::ExprVisitor {
	public:
	  ast::Module * module = 0;
	  ast::BaseExpr * target;
	  std::string name;

	  ReturnInserter(ast::Module * m) : module(m) { visit(m); }
	  virtual std::string visitorName() { return "ReturnInserter"; }

	  void visit(ast::Module * m);
	  void visit(ast::Func * f);
	  void visit(ast::Block * m);
	  void visit(ast::Comment * e) { }
	  void visit(ast::Let * e) { }
	};
  }
}
