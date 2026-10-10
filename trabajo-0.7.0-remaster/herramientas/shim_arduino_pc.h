#include <string>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cctype>
struct String {
  std::string s;
  String(){} String(const char*c):s(c){} String(const std::string&x):s(x){}
  String(int v):s(std::to_string(v)){} String(unsigned v):s(std::to_string(v)){} String(uint8_t v):s(std::to_string((int)v)){}
  unsigned length() const {return s.size();}
  char operator[](int i) const {return s[i];}
  String& operator+=(char c){s+=c;return *this;} String& operator+=(const String&o){s+=o.s;return *this;} String& operator+=(const char*c){s+=c;return *this;}
  int indexOf(const String&o) const {size_t p=s.find(o.s);return p==std::string::npos?-1:(int)p;}
  int indexOf(char c,int from=0) const {size_t p=s.find(c,from);return p==std::string::npos?-1:(int)p;}
  String substring(int a,int b) const {return String(s.substr(a,b-a));} String substring(int a) const {return String(s.substr(a));}
  void trim(){size_t a=s.find_first_not_of(" \t\r\n");size_t b=s.find_last_not_of(" \t\r\n");s=(a==std::string::npos)?"":s.substr(a,b-a+1);}
  void replace(const char*a,const char*b){std::string f=a,r=b;size_t p=0;while((p=s.find(f,p))!=std::string::npos){s.replace(p,f.size(),r);p+=r.size();}}
  void toUpperCase(){for(auto&c:s)c=toupper(c);} bool startsWith(const char*p) const {return s.rfind(p,0)==0;}
  const char* c_str() const {return s.c_str();} void reserve(unsigned){}
  bool operator==(const String&o) const {return s==o.s;} bool operator!=(const String&o) const {return s!=o.s;}
  bool operator==(const char*o) const {return s==o;} bool operator!=(const char*o) const {return s!=o;}
};
inline String operator+(const String&a,const String&b){return String(a.s+b.s);}
inline String operator+(const char*a,const String&b){return String(std::string(a)+b.s);}
inline String operator+(const String&a,const char*b){return String(a.s+b);}
struct { void print(const char*c){printf("%s",c);} void println(const char*c){printf("%s\n",c);} void println(int v){printf("%d\n",v);} } Serial;
struct { void putString(const char*,const String&){} String getString(const char*,const char*){return "";} } dcPref;
enum DcPqEstado:uint8_t{PQ_LIBRE,PQ_PENDIENTE,PQ_ENVIADA,PQ_LISTA,PQ_TIMEOUT};
struct DcPuntual{String cmd;uint16_t timeout;String rx;DcPqEstado estado;};
static DcPuntual dcQ[4];
static int8_t dcPedir(const char*,uint16_t=0){return 0;} static DcPqEstado dcEstado(uint8_t){return PQ_LIBRE;}
static String dcRespuesta(uint8_t){return "";} static void dcLiberar(uint8_t){}
static String dcLimpia(const String&r){String o;for(unsigned i=0;i<r.length();i++){char c=r[i];if(c!=' '&&c!='\r'&&c!='\n'&&c!='\t')o+=c;}o.toUpperCase();return o;}
static bool dcHex(const String&s,int i,uint8_t&v){if(i<0||i+2>(int)s.length())return false;char b[3]={s[i],s[i+1],0};char*e=nullptr;long n=strtol(b,&e,16);if(!e||*e)return false;v=(uint8_t)n;return true;}
