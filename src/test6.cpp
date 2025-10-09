#include "zketch.hpp"
using namespace zketch ;

class Calculator {
	private :
		std::vector<Button> buttons_ ;
		std::string expr_ ;

	public :
		Calculator(const Calculator&) = delete ;
		Calculator& operator=(const Calculator&) = delete ;
		Calculator(Calculator&&) = default ;
		Calculator& operator=(Calculator&&) = default ;
		Calculator() = default ;

	
} ;

int main() {
	zketch_init() ;
}