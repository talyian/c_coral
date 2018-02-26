class AllConstraints {
public:
  static std::set<Constraint *> of(TypeGraph * graph, TypeTerm * tt) {
    return AllConstraints(graph, tt).out;
  }
  TypeGraph * graph;
  TypeTerm * tt;
  std::set<Constraint *> out;
  AllConstraints(TypeGraph * graph, TypeTerm * tt);
};
