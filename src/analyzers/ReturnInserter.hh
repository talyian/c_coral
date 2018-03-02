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

	  void visit(ast::Module *);
	  void visit(ast::Func *);
	  void visit(ast::Block *);
	  void visit(ast::Comment *) { }
	  void visit(ast::Let *) { }
	  void visit(ast::Extern *) { }
      void visit(ast::Tuple *) { }
	};
  }
}
