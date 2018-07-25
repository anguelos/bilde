namespace bilde{ template <typename T>
    struct Buffer{
        Buffer(const Buffer<T>& b){ /*nothing hapens;  */  }
        const int width;const int height;
        T* getRow(int row){return 0;}
        
    };
    typedef unsigned char t_uint8;
    typedef unsigned short int  t_uint16;
    typedef unsigned int t_uint32;
    typedef unsigned long int  t_uint64;
    typedef signed char t_sint8;
    typedef signed short int t_sint16;
    typedef signed int t_sint32;
    typedef signed long int t_sint64;
    typedef float t_real32;
    typedef double  t_real64;
}

//MACROS FOR INSTATIATING LISTS

//taken from Dave Dopson's solution http://stackoverflow.com/questions/1082192/how-to-generate-random-variable-names-in-c-using-macros
#define BILDE_PP_CAT(a, b) BILDE_PP_CAT_I(a, b)
#define BILDE_PP_CAT_I(a, b) BILDE_PP_CAT_II(~, a ## b)
#define BILDE_PP_CAT_II(p, res) res

#define BILDE_UNIQUENAMESPACE BILDE_PP_CAT( __NullNamespace__ , BILDE_PP_CAT(__COUNTER__, BILDE_PP_CAT(__,__LINE__) ) )

#define BILDE_INSTANTIATE_FOR(FUNCTION_NAME,...)namespace BILDE_UNIQUENAMESPACE {\
template <typename ...ARGS> struct Instantiate;\
template <typename T,typename ...ARGS> struct Instantiate<T,ARGS...>{\
    int function(){\
        auto ptr=&(FUNCTION_NAME<T>);\
        return Instantiate<ARGS...>::function();\
    }\
};\
template <> struct Instantiate<>{int function(){return 0;}};\
const Instantiate<__VA_ARGS__> var;}


//Fake STL namespace
namespace std{
    template <typename T> struct vector{
        T& operator[](int v){}
    };
    template <typename T> T min(T val1,T val2);
}





























































