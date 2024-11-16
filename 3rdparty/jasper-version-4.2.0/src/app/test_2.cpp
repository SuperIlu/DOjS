#include <iostream>
#include <jasper/jasper.h>

class jasper_library
{
public:
	jasper_library()
	{
		jas_conf_clear();
		jas_init_library();
	}
	~jasper_library()
	{
		jas_cleanup_library();
	}
};

int main()
{
	jasper_library lib;
}
