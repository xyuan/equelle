#include "EquelleRuntimeCUDA.hpp"
#include "EquelleRuntimeCUDA_cuda.hpp"
#include "EquelleRuntimeCUDA_havahol.hpp"

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <iterator>


//
//   CPP for implementation of non-cuda functions
//


void EquelleRuntimeCUDA::output(const String& tag, const CollOfScalar& coll, int dummy)
{
    // Get data back to host
    std::vector<double> host = coll.copyToHost();
    
    // BUG!
    // since copyToHost do not use any vector functions, just modify the raw data
    // host.size() returns 0 even though the values are there!
    // Go back to using malloc and free to make this clear?
    // Or fill with the right amount of zeros to make the vector the right size?
    
    if (output_to_file_) {
	// std::map<std::string, int> outputcount_;
	// set file name to tag-0000X.output
	int count = -1;
	auto it = outputcount_.find(tag);
	if ( it == outputcount_.end()) {
	    count = 0;
	    outputcount_[tag] = 1; // should contain the count to be used next time for same tag.
	} else {
	    count = outputcount_[tag];
	    ++outputcount_[tag];
	}
	std::ostringstream fname;
	fname << tag << "-" << std::setw(5) << std::setfill('0') << count << ".output";
	std::ofstream file(fname.str().c_str());
	if( !file ) {
	    OPM_THROW(std::runtime_error, "Failed to open " << fname.str());
	}
	file.precision(16);
	std::cout << "Printing to file...(host.size() = " << host.size() << " )\n";
	std::copy(host.data(), host.data() + coll.getSize(),
		  std::ostream_iterator<double>(file, "\n"));
    } else {
	std::cout << "\n";
	std::cout << "Values in " << tag << std::endl;
	for(int i = 0; i < coll.getSize(); ++i) {
	    std::cout << host[i] << "  ";
	}
	std::cout << std::endl;
    }
}
