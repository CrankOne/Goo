# ifndef H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H
# define H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H

namespace goo {
namespace dict {

class Dictionary;

class InsertionProxy {
private:
    Dictionary & _currentDictionary;
public:
    void bgn_sect( const char * );
    void end_sect( const char * = 0 );
};  // class InsertionProxy

}  // namespace goo
}  // namespace dict

# endif  // H_GOO_PARAMETERS_DICTIONARY_INSERTION_PROXY_H

