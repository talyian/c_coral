namespace coral {
  class Token {
  public:
	enum TokenValues {
	  EndOfFile = 0,
	  UNKNOWN = 1000,
	  STRING,
	  IDENTIFIER,
	  NEWLINE,
	  INDENT,
	  DEDENT,
	};
  };
}
