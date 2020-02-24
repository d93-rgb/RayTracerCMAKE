#include "threads/dispatcher.h"

namespace rt
{

int Slice::get_index()
{
	if (idx == length || ++idx == length)
	{
		return -1;
	}
	else
	{
		return idx;
	}
}

} // namespace rt