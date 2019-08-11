#include "util.hpp"

// split string into tokens at delimiters
void util::tokenize(const std::string& str,
					std::vector<std::string>& tokens,
					const std::string& delimiters)
{
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos = str.find_first_of(delimiters, lastPos);

	while ( (std::string::npos != pos) || (std::string::npos != lastPos) )
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}

// finite impulse response (FIR) filter that implements a boxcar window function
// http://stackoverflow.com/questions/12973255/algorithm-for-smoothing
util::boxFIR::boxFIR(int _numCoeffs) : numCoeffs(_numCoeffs)
{
	if (numCoeffs < 1) {
		// Must be > 0 or bad stuff happens
		numCoeffs = 1;
	}
}

// smooth vector with double numbers
void util::boxFIR::filter(std::vector<double> &a)
{
	double output;
    std::vector<double> b; // filter coefficients
    std::vector<double> m; // filter memories

	double val = 1. / numCoeffs;

	for (int ii = 0; ii < numCoeffs; ++ii) {
		b.push_back(val);
		m.push_back(0.);
    }

	// extend vector by order of filter
	int order = numCoeffs - 1;
	while ( order > 0) {
		a.push_back(a.back());
		order--;
	}

	// reverse (no phase distortion)
  for (int i = a.size(); i > 0; --i) {
    // Apply smoothing filter to signal
    output = 0;
    m[m.size() - 1] = a[i - 1];

    for (int j = numCoeffs; j > 0; --j) {
      output += b[j - 1] * m[j - 1];
    }

    // Reshuffle memories
    for (int j = 0; j != numCoeffs; ++j) {
      m[j] = m[j + 1];
    }

    a[i - 1] = output;
	}
}
