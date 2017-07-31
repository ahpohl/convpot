#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>
#include <sstream>

namespace util {

	void tokenize(const std::string& str,
		std::vector<std::string>& tokens,
		const std::string& delimiters);

	template<typename T>
	inline T stringTo( const std::string& s )
	{
		std::istringstream iss(s);
		T x;
		iss >> x;
		return x;
	};

	template<typename T>
	inline std::string toString( const T& x )
	{
		std::ostringstream o;
		o << x;
		return o.str();
	};

	class boxFIR
	{

	private:
	    int numCoeffs; // MUST be > 0

	public:
	    boxFIR(int _numCoeffs);
	    void filter(std::vector<double> &a);
	};
}

#endif // UTIL_H
