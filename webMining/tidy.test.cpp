/*#include "dom.hpp"
#include "tTPSFilter.h"

int main(int argc, char **argv )
{
	DOM dom(argv[1]);
	tTPSFilter filter;

	dom.traverse(dom.html());

	dom.printHTML();

	filter.SRDE(dom.body(), true);

	for (auto i:filter.getTagPathSequence())
		std::cout << i << std::endl;

	return 0;
}
*/
