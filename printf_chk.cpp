#include "stdafx.h"

#include "c_core.h"

#define COMPILER c_compiler

#ifdef _MSC_VER
#define DLL_EXPORT __declspec(dllexport)
#else // _MSC_VER
#define DLL_EXPORT
#endif // _MSC_VER

extern "C" DLL_EXPORT
int generator_seed()
{
#ifdef _MSC_VER
  int r = _MSC_VER;
#ifndef CXX_GENERATOR
  r += 10000000;
#else // CXX_GENERATOR
  r += 20000000;
#endif // CXX_GENERATOR
#ifdef WIN32
  r += 100000;
#endif // WIN32
#endif // _MSC_VER
#ifdef __GNUC__
  int r = (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__);
#ifndef CXX_GENERATOR
  r += 30000000;
#else // CXX_GENERATOR
  r += 40000000;
#endif // CXX_GENERATOR
#endif // __GNUC__
  return r;
}

extern "C" DLL_EXPORT
int generator_sizeof(const COMPILER::type* T)
{
  using namespace COMPILER;
  switch(T->m_id) {
  case type::LONG:
  case type::POINTER:
    return 8;
  default:
    return T->size();
  }
}

extern "C" DLL_EXPORT
int generator_sizeof_type()
{
#ifdef GENERAL32BIT_SETTING
  return (int)c_compiler::type::UINT;
#else // GENERAL32BIT_SETTING  
  return (int)c_compiler::type::ULONG;
#endif // GENERAL32BIT_SETTING
}

std::string curr_func;

inline bool cmpid(COMPILER::tac* tac, COMPILER::tac::id_t id)
{
  return tac->id == id;
}

typedef std::vector<COMPILER::tac*>::const_iterator IT;
inline IT check(IT callp, IT begin);

extern "C" DLL_EXPORT
void generator_generate(const COMPILER::generator::interface_t* info)
{
  using namespace std;
  using namespace COMPILER;
  if (!info->m_func)
    return;
  curr_func = info->m_func->m_usr->m_name;
  const vector<tac*>& code = *info->m_code;

  IT begin = code.begin();
  while (begin != code.end()) {
    IT callp = find_if(begin, code.end(), bind2nd(ptr_fun(cmpid), tac::CALL));
    if (callp == code.end())
      break;
    begin = check(callp, begin);
  }
}

namespace printf_family {
  using namespace std;
  struct info {
    string m_name;
    int m_fmt;
    info(string name = "", int fmt = 0) : m_name(name), m_fmt(fmt) {}
  };
  struct ftable_t : map<string, info> {
    ftable_t()
    {
      (*this)["printf"] = info("printf", 0);
      (*this)["fprintf"] = info("fprintf", 1);
      (*this)["sprintf"] = info("sprintf", 1);
      (*this)["snprintf"] = info("snprintf", 2);
    }
  } ftable;
} // end of printf_family

namespace warning {
  using namespace std;
  using namespace COMPILER;
  void no_arg(const file_t& file, string func);
  void unknown_designator(const file_t& file, std::string designator);
  void invalid_designator(const file_t& file,
                          int nth, string text, const type* T);
}  // end of namespace warning

namespace error {
  using namespace std;
  using namespace COMPILER;
  void numof_arg(const file_t& file, string fmt, int require, int specified);
  void invalid_designator(const file_t& file, int nth,
                          string text, const type* T);
}  // end of namespace error

inline bool cmpx(COMPILER::tac* ptr, COMPILER::var* v)
{
  return ptr->x == v;
}

namespace devide {
  using namespace std;
  using namespace COMPILER;
  inline void entry(string fmt,
                    vector<pair<string, string> >& designators,
                    const file_t& file);
} // end of namespace devide

template<class InIt1, class InIt2, class Fn, class V> V
accumulate(InIt1 begin1, InIt1 end1, InIt2 begin2, V v, Fn op)
{
  for (; begin1 != end1; ++begin1, ++begin2)
    v = op(v, *begin1, *begin2);
  return v;
}

namespace printf_family {
  using namespace std;
  using namespace COMPILER;
  typedef void (*HANDLER)(int nth, string text, const type* T, const file_t& file);
  void h_int(int nth, string text, const type* T, const file_t& file);
  void h_int_uint(int nth, string text, const type* T, const file_t& file);
  void h_double(int nth, string text, const type* T, const file_t& file);
  void h_ptr(int nth, string text, const type* T, const file_t& file);
  void h_char_ptr(int nth, string text, const type* T, const file_t& file);
  void h_uint(int nth, string text, const type* T, const file_t& file);
  void h_long(int nth, string text, const type* T, const file_t& file);
  void h_long_ptr(int nth, string text, const type* T, const file_t& file);
  void h_ulong(int nth, string text, const type* T, const file_t& file);
  void h_longdouble(int nth, string text, const type* T, const file_t& file);
  void h_longlong(int nth, string text, const type* T, const file_t& file);
  void h_longlong_ptr(int nth, string text, const type* T, const file_t& file);
  void h_ulonglong(int nth, string text, const type* T, const file_t& file);
  struct htable_t : map<string, HANDLER> {
    htable_t()
    {
      (*this)["c"] =
      (*this)["d"] = h_int;
      (*this)["e"] =
      (*this)["f"] =
      (*this)["g"] = h_double;
      (*this)["i"] =
      (*this)["o"] = h_int;
      (*this)["p"] = h_ptr;
      (*this)["s"] = h_char_ptr;
      (*this)["u"] = h_uint;
      (*this)["x"] = h_int_uint;
      (*this)["E"] =
      (*this)["F"] =
      (*this)["G"] = h_double;
      (*this)["X"] = h_int_uint;
      (*this)["ld"] = h_long;
      (*this)["le"] =
      (*this)["lf"] =
      (*this)["lg"] = h_double;
      (*this)["li"] =
      (*this)["lo"] = h_long;
      (*this)["lu"] = h_ulong;
      (*this)["lx"] = h_long_ptr;
      (*this)["lE"] =
      (*this)["lF"] =
      (*this)["lG"] = h_double;
      (*this)["lX"] = h_long_ptr;
      (*this)["lld"] =
      (*this)["lli"] =
      (*this)["llo"] = h_longlong;
      (*this)["llu"] = h_ulonglong;
      (*this)["llx"] =
      (*this)["llX"] = h_longlong_ptr;
      (*this)["Le"] =
      (*this)["Lf"] =
      (*this)["Lg"] =
      (*this)["LE"] =
      (*this)["LF"] =
      (*this)["LG"] = h_longdouble;
#ifdef GENERAL32BIT_SETTING
      (*this)["zu"] =
      (*this)["zx"] =
      (*this)["zX"] = h_uint;
#else // GENERAL32BIT_SETTING
      (*this)["zu"] =
      (*this)["zx"] =
      (*this)["zX"] = h_ulong;
#endif // GENERAL32BIT_SETTING
    }
  } htable;
} // end of namespace printf_family

inline const COMPILER::type* simplify(const COMPILER::type* T)
{
  using namespace COMPILER;
  T = T->unqualified();
  int id = T->m_id;
  if (id == type::ENUM) {
    typedef const enum_type ET;
    ET* et = static_cast<ET*>(T);
    T = et->get_integer();
    T = T->unqualified();
  }
  else if (id == type::BIT_FIELD) {
    typedef const bit_field_type BT;
    BT* bt = static_cast<BT*>(T);
    T = bt->integer_type();
    T = T->unqualified();
  }
  return T;
}

void
printf_family::h_int(int nth, std::string text,
                     const COMPILER::type* T, const COMPILER::file_t& file)
{
  using namespace COMPILER;
  const type* org = T;
  T = simplify(T);
  int id = T->m_id;
  if (id == type::INT)
    return;
  if (id == type::UINT)
    warning::invalid_designator(file, nth, text, org);
  else
    error::invalid_designator(file, nth, text, org);
}

void
printf_family::h_int_uint(int nth, std::string text,
                          const COMPILER::type* T, const COMPILER::file_t& file)
{
  using namespace COMPILER;
  const type* org = T;
  T = simplify(T);
  int id = T->m_id;
  if (id == type::INT || id == type::UINT)
    return;
  error::invalid_designator(file, nth, text, org);
}

void
printf_family::h_double(int nth, std::string text,
                        const COMPILER::type* T, const COMPILER::file_t& file)
{
  using namespace COMPILER;
  const type* org = T;
  T = simplify(T);
  int id = T->m_id;
  if (id == type::DOUBLE)
    return;
  error::invalid_designator(file, nth, text, org);
}

void
printf_family::h_ptr(int nth, std::string text,
                     const COMPILER::type* T, const COMPILER::file_t& file)
{
  using namespace COMPILER;
  const type* org = T;
  T = simplify(T);
  if (T->m_id != type::POINTER)
    error::invalid_designator(file, nth, text, org);
}

void
printf_family::h_char_ptr(int nth, std::string text,
                          const COMPILER::type* T, const COMPILER::file_t& file)
{
  using namespace COMPILER;
  const type* org = T;
  T = T->unqualified();
  if (T->m_id != type::POINTER) {
    error::invalid_designator(file, nth, text, org);
    return;
  }
  typedef const pointer_type PT;
  PT* pt = static_cast<PT*>(T);
  T = pt->referenced_type();
  T = T->unqualified();
  if (T->size() != 1)
    error::invalid_designator(file, nth, text, org);
}

void
printf_family::h_uint(int nth, std::string text,
                      const COMPILER::type* T, const COMPILER::file_t& file)
{
  using namespace COMPILER;
  const type* org = T;
  T = simplify(T);
  int id = T->m_id;
  if (id == type::UINT)
    return;
  if (id == type::INT) {
    warning::invalid_designator(file, nth, text, org);
    return;
  }
  error::invalid_designator(file, nth, text, org);
}

void
printf_family::h_long(int nth, std::string text,
                      const COMPILER::type* T, const COMPILER::file_t& file)
{
  using namespace COMPILER;
  const type* org = T;
  T = simplify(T);
  int id = T->m_id;
  if (id == type::LONG)
    return;
  if (id == type::ULONG) {
    warning::invalid_designator(file, nth, text, org);
    return;
  }
  error::invalid_designator(file, nth, text, org);
}

void
printf_family::h_long_ptr(int nth, std::string text,
                          const COMPILER::type* T, const COMPILER::file_t& file)
{
  using namespace COMPILER;
  const type* org = T;
  T = simplify(T);
  int id = T->m_id;
  if (id == type::LONG || id == type::ULONG || id == type::POINTER)
    return;
  error::invalid_designator(file, nth, text, org);
}

void
printf_family::h_ulong(int nth, std::string text,
                       const COMPILER::type* T, const COMPILER::file_t& file)
{
  using namespace COMPILER;
  const type* org = T;
  T = simplify(T);
  int id = T->m_id;
  if (id == type::ULONG)
    return;
  if (id == type::LONG) {
    warning::invalid_designator(file, nth, text, org);
    return;
  }
  error::invalid_designator(file, nth, text, org);
}

void
printf_family::h_longdouble(int nth, std::string text,
                            const COMPILER::type* T,
                            const COMPILER::file_t& file)
{
  using namespace COMPILER;
  const type* org = T;
  T = simplify(T);
  int id = T->m_id;
  if (id == type::LONG_DOUBLE)
    return;
  error::invalid_designator(file, nth, text, org);
}

void
printf_family::h_longlong(int nth, std::string text,
                          const COMPILER::type* T, const COMPILER::file_t& file)
{
  using namespace COMPILER;
  const type* org = T;
  T = simplify(T);
  int id = T->m_id;
  if (id == type::LONGLONG)
    return;
  if (id == type::ULONGLONG)
    warning::invalid_designator(file, nth, text, org);
  else
    error::invalid_designator(file, nth, text, org);
}

void
printf_family::h_longlong_ptr(int nth, std::string text,
                              const COMPILER::type* T,
                              const COMPILER::file_t& file)
{
  using namespace COMPILER;
  const type* org = T;
  T = simplify(T);
  int id = T->m_id;
  if (id == type::LONGLONG || id == type::POINTER || id == type::ULONGLONG)
    return;
  error::invalid_designator(file, nth, text, org);
}

void
printf_family::h_ulonglong(int nth, std::string text,
                           const COMPILER::type* T,
                           const COMPILER::file_t& file)
{
  using namespace COMPILER;
  const type* org = T;
  T = simplify(T);
  int id = T->m_id;
  if (id == type::ULONGLONG)
    return;
  if (id == type::LONGLONG)
    warning::invalid_designator(file, nth, text, org);
  else
    error::invalid_designator(file, nth, text, org);
}

inline int check_type(int nth,
                      std::pair<std::string, std::string> designator,
                      COMPILER::tac* ptr)
{
  using namespace std;
  using namespace COMPILER;
  using namespace printf_family;
  string key = designator.second;
  htable_t::const_iterator p = htable.find(key);
  assert(p != htable.end());
  string text = designator.first;  
  var* y = ptr->y;
  const type* T = y->m_type;
  const file_t& file = ptr->m_file;
  (p->second)(nth, text, T, file);
  return nth + 1;
}

inline IT check(IT callp, IT begin)
{
  using namespace std;
  using namespace COMPILER;
  using namespace printf_family;
  usr* func = (*callp)->y->usr_cast();
  if (!func)
    return callp + 1;
  if (!(func->m_flag & usr::FUNCTION))
    return callp + 1;
  ftable_t::const_iterator q = ftable.find(func->m_name);
  if (q == ftable.end())
    return callp + 1;
  typedef vector<tac*>::const_reverse_iterator RIT;
  RIT s(callp);
  RIT t(begin);
  RIT u = find_if(s, t, bind2nd(not2(ptr_fun(cmpid)), tac::PARAM));
  file_t file = (*callp)->m_file;
  if (u == t) {
    warning::no_arg(file, func->m_name);
    return callp + 1;
  }
  IT uu(u.base());
  uu += q->second.m_fmt;
  tac* ptr = *uu;
  assert(ptr->id == tac::PARAM);
  RIT v = find_if(u, t, bind2nd(ptr_fun(cmpx), ptr->y));
  if (v == t)
    return callp + 1;
  tac* addr = *v;
  if (addr->id != tac::ADDR)
    return callp + 1;
  usr* str = addr->y->usr_cast();
  if (!str)
    return callp + 1;
  string fmt = str->m_name;
  if (!is_string(fmt))
    return callp + 1;
  fmt = fmt.substr(1, fmt.size() - 2);
  vector<pair<string, string> > designators;
  devide::entry(fmt, designators, file);
  int require = designators.size();
  int specified = distance(uu+1, callp);
  if (require != specified) {
    error::numof_arg(file, fmt, require, specified);
    return callp + 1;
  }
  accumulate(designators.begin(), designators.end(), uu+1, 0, check_type);
  return callp + 1;
}

namespace devide {
  using namespace std;
  using namespace COMPILER;
  inline string::size_type percent(string::size_type pos,
                                   string fmt,
                                   vector<pair<string, string> >& designators,
                                   const file_t& file);
} // end of namespace devide

inline void
devide::entry(std::string fmt,
              std::vector<std::pair<std::string, std::string> >& designators,
              const COMPILER::file_t& file)
{
  using namespace std;
  string::size_type pos = 0;
  while (pos != string::npos) {
    pos = fmt.find('%', pos);
    if (pos != string::npos)
      pos = percent(pos, fmt, designators, file);
  }
}

inline std::string::size_type
devide::percent(std::string::size_type pos,
                std::string fmt,
                std::vector<std::pair<std::string, std::string> >& designators,
                const COMPILER::file_t& file)
{
  string text;
  text += fmt[pos];
  char c = fmt[++pos];
  if (c == '%')
    return ++pos;

  // discard flag part
  while (c == '+' || c == '-' || c == ' ' || c == '0' || c == '#')
    ++pos, c = fmt[pos], text += c;
  
  // discard width part
  if (c == '*')
    ++pos, c = fmt[pos], text += c;
  else {
    while (isdigit(c))
      ++pos, c = fmt[pos], text += c;
  }

  // discard precision part
  if (c == '.') {
    ++pos, c = fmt[pos], text += c;
    while (isdigit(c))
      ++pos, c = fmt[pos], text += c;
  }

  switch (c) {
  case 'c': case 'd': case 'e': case 'f':
  case 'g': case 'i': case 'o': case 'p':
  case 's': case 'u': case 'x': case 'E':
  case 'F': case 'G': case 'X':
    {
      string key;
      key += c, text += c;
      designators.push_back(make_pair(text, key));
      return ++pos;
    }
  case 'l':
    {
      string key;
      key += c, text += c;
      c = fmt[++pos];
      key += c, text += c;
      if (c == 'd' || c == 'e' || c == 'f' || c == 'g' || c == 'i' ||
          c == 'o' || c == 'u' || c == 'x' || c == 'E' || c == 'F' ||
          c == 'G') {
        designators.push_back(make_pair(text, key));
        return ++pos;
      }
      if (c == 'l') {
        c = fmt[++pos];
        key += c, text += c;
        if (c == 'd' || c == 'i' || c == 'o' || c == 'u' || c == 'x' ||
            c == 'X') {
          designators.push_back(make_pair(text, key));
          return ++pos;
        }
        warning::unknown_designator(file, text);
        return ++pos;
      }
      warning::unknown_designator(file, text);
      return ++pos;
    }
  case 'L':
    {
      string key;
      key += c, text += c;
      c = fmt[++pos];
      key += c, text += c;
      if (c == 'e' || c == 'f' || c == 'g' || c == 'E' ||
          c == 'F' || c == 'G') {
        designators.push_back(make_pair(text, key));
        return ++pos;
      }
      warning::unknown_designator(file, text);
      return ++pos;
    }
  case 'z':
    {
      string key;
      key += c, text += c;
      c = fmt[++pos];
      key += c, text += c;
      if (c == 'u' || c == 'x' || c == 'X') {
        designators.push_back(make_pair(text, key));
        return ++pos;
      }
      warning::unknown_designator(file, text);
      return ++pos;
    }
  default:
    {
      text += c;
      warning::unknown_designator(file, text);
      return ++pos;
    }
  }
}

namespace warning {
  using namespace std;
  using namespace COMPILER;
  void common_header(string msg, const file_t& file)
  {
    static string prev;
    if (prev != curr_func) {
      cerr << file.m_name << ": in function `" << curr_func << "':" << '\n';
      prev = curr_func;
    }
    cerr << file.m_name << ':' << file.m_lineno << ": " << msg << ": ";
  }
  void header(const file_t& file) { common_header("warning", file); }
  int counter;
} // end of namespace warning

void warning::no_arg(const COMPILER::file_t& file, std::string func)
{
  using namespace std;
  header(file);
  cerr << "no argument specfied for `" << func << "'." << '\n';
  ++counter;
}

void warning::unknown_designator(const COMPILER::file_t& file, std::string s)
{
  using namespace std;
  header(file);
  cerr << "unknown designator `" << s << "'" << '\n';
  ++counter;
}

namespace warning {
  using namespace std;
  using namespace COMPILER;
  void invalid_designator_common(int nth, string text, const type* T)
  {
    cerr << "for ";
    switch (nth) {
    case 0: cerr << "1st"; break;
    case 1: cerr << "2nd"; break;
    case 2: cerr << "3rd"; break;
    default: cerr << nth + 1 << "th"; break;
    }

    cerr << " designator `" << text << "', ";
    T->decl(cerr, "");
    cerr << " is specified." << '\n';
  }
} // end of namespace warning

void
warning::invalid_designator(const COMPILER::file_t& file,
                            int nth,
                            std::string text,
                            const COMPILER::type* T)
{
  header(file);
  invalid_designator_common(nth, text, T);
  ++counter;
}

namespace error {
  using namespace COMPILER;
  void header(const file_t& file) { warning::common_header("error", file); }
  int counter;
} // end of namespace error

void error::numof_arg(const COMPILER::file_t& file,
                      std::string fmt, int require, int specified)
{
  using namespace std;
  error::header(file);
  cerr << "format string `" << fmt << "' requires " << require << " argument";
  if (require > 1)
    cerr << 's';
  cerr << " but specified " << specified << " argument.";
  if (specified > 1)
    cerr << 's';
  cerr << '\n';
  ++counter;
}

void
error::invalid_designator(const COMPILER::file_t& file,
                          int nth,
                          std::string text,
                          const COMPILER::type* T)
{
  header(file);
  warning::invalid_designator_common(nth, text, T);
  ++counter;
}

struct x {
  ~x()
  {
    using namespace std;
    if (!warning::counter && !error::counter)
      return;
    cerr << "printf_chk.dll: ";
    cerr << warning::counter << " warning";
    if (warning::counter > 1)
      cerr << 's';
    cerr << ", ";
    cerr << error::counter << " error";
    if (error::counter > 1)
      cerr << 's';
    cerr << '.' << '\n';
    if (error::counter)
      exit(1);
  }
} x;
