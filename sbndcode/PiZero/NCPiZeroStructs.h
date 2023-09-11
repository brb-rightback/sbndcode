enum EventType
  {
    kNCPiZero,
    kOtherNC,
    kCCNuMu,
    kCCNuE,
    kDirt,
    kNonFV,
    kCosmic,
    kBadRecoSignal,
    kUnknownEv = -1
  };

enum VarType
  {
    kBool,
    kInt,
    kDouble,
    kUnknownVar = -1
  };
    
class VecVar
{
  std::string name;

  public:
  
  VecVar(std::string n = "")
    : name(n)
  {}

  std::string Name() const
  {
    return name;
  }

  virtual VarType Identify() const
  {
    return kUnknownVar;
  }

  virtual void Resize(const int size) = 0;

  template<typename T>
  void SetVal(const int pos, const T val) {}
};

template<typename T>
class InhVecVar;

template<typename T>
class InhVecVar : public VecVar
{
  std::vector<T> var;

  public:

  InhVecVar(std::string n, std::vector<T> v)
    : VecVar(n),
      var(v)
  {}

  InhVecVar(std::string n = "")
    : VecVar(n),
      var(std::vector<T>())
  {}

  std::vector<T>& Var()
  {
    return var;
  }

  void Resize(const int size)
  {
    var.resize(size);
  }

  void SetVal(const int pos, const T value)
  {
    var[pos] = value;
  }

  virtual VarType Identify() const;
};

template<>
VarType InhVecVar<bool>::Identify() const { return kBool; }

template<>
VarType InhVecVar<int>::Identify() const { return kInt; }

template<>
VarType InhVecVar<double>::Identify() const { return kDouble; }

template <typename T,
  typename TIter = decltype(std::begin(std::declval<T>())),
  typename = decltype(std::end(std::declval<T>()))>
  constexpr auto enumerate(T && iterable)
{
    struct iterator
    {
      size_t i;
      TIter iter;
      bool operator != (const iterator & other) const { return iter != other.iter; }
      void operator ++ () { ++i; ++iter; }
      auto operator * () const { return std::tie(i, *iter); }
    };
    struct iterable_wrapper
    {
      T iterable;
      auto begin() { return iterator{ 0, std::begin(iterable) }; }
      auto end() { return iterator{ 0, std::end(iterable) }; }
    };
    return iterable_wrapper{ std::forward<T>(iterable) };
}
